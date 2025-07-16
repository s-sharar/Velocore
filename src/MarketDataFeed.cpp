#include "MarketDataFeed.h"
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <sstream>

namespace velocore {

MarketDataFeed::MarketDataFeed() 
    : config_(Configuration::getInstance()),
      ssl_context_(boost::asio::ssl::context::tlsv12_client),
      strand_(boost::asio::make_strand(io_context_)) {
    
    // Configure SSL context
    ssl_context_.set_default_verify_paths();
    ssl_context_.set_verify_mode(boost::asio::ssl::verify_peer);
    
    // Initialize timers
    heartbeat_timer_ = std::make_unique<boost::asio::steady_timer>(io_context_);
    reconnect_timer_ = std::make_unique<boost::asio::steady_timer>(io_context_);
}

MarketDataFeed::~MarketDataFeed() {
    stop();
}

void MarketDataFeed::start() {
    if (running_.exchange(true)) {
        return;
    }
    
    std::cout << "Starting MarketDataFeed..." << std::endl;
    
    // Start the I/O context in a separate thread
    worker_thread_ = std::thread([this]() {
        connectWebSocket();
        io_context_.run();
    });
}

void MarketDataFeed::stop() {
    if (!running_.exchange(false)) {
        return;
    }
    
    std::cout << "Stopping MarketDataFeed..." << std::endl;
    
    // Stop timers
    if (heartbeat_timer_) {
        heartbeat_timer_->cancel();
    }
    if (reconnect_timer_) {
        reconnect_timer_->cancel();
    }
    
    closeWebSocket();
    
    // Stop I/O context
    io_context_.stop();
    
    // Join worker thread
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    // Reset state
    connected_ = false;
    authenticated_ = false;
    reconnect_attempts_ = 0;
}

void MarketDataFeed::subscribe(const std::string& symbol, bool trades, bool quotes, bool bars) {
    std::lock_guard<std::mutex> lock(subscriptions_mutex_);
    
    // Check if already subscribed
    if (subscribed_symbols_.find(symbol) != subscribed_symbols_.end()) {
        std::cout << "Already subscribed to " << symbol << std::endl;
        return;
    }
    
    // Create subscription
    MarketSubscription subscription(symbol);
    subscription.trades = trades;
    subscription.quotes = quotes;
    subscription.bars = bars;
    
    pending_subscriptions_.push_back(subscription);
    
    // If connected and authenticated, send subscription immediately
    if (connected_ && authenticated_) {
        sendSubscriptionMessage();
    }
    
    std::cout << "Queued subscription for " << symbol << std::endl;
}

void MarketDataFeed::unsubscribe(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(subscriptions_mutex_);
    
    subscribed_symbols_.erase(symbol);
    
    // Remove from pending subscriptions
    pending_subscriptions_.erase(
        std::remove_if(pending_subscriptions_.begin(), pending_subscriptions_.end(),
            [&symbol](const MarketSubscription& sub) { return sub.symbol == symbol; }),
        pending_subscriptions_.end()
    );
    
    // TODO: Send unsubscription message to Alpaca
    std::cout << "Unsubscribed from " << symbol << std::endl;
}

void MarketDataFeed::onTick(OnTickCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    tick_callback_ = callback;
}

void MarketDataFeed::onConnection(OnConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    connection_callback_ = callback;
}

void MarketDataFeed::onError(OnErrorCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    error_callback_ = callback;
}

bool MarketDataFeed::isConnected() const {
    return connected_;
}

std::vector<std::string> MarketDataFeed::getSubscribedSymbols() const {
    std::lock_guard<std::mutex> lock(subscriptions_mutex_);
    return std::vector<std::string>(subscribed_symbols_.begin(), subscribed_symbols_.end());
}

void MarketDataFeed::broadcastBookUpdate(const std::string& symbol, const MarketTick& tick) {
    (void)symbol; // Mark as unused to suppress warning
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (tick_callback_) {
        tick_callback_(tick);
    }
}

void MarketDataFeed::connectWebSocket() {
    try {
        std::cout << "Connecting to Alpaca WebSocket..." << std::endl;
        
        // Parse URL
        std::string url = config_.getAlpacaConfig().data_url;
        std::string host = "stream.data.alpaca.markets";
        std::string port = "443";
        std::string path = "/v2/iex";
        
        // Create WebSocket stream
        ws_ = std::make_unique<boost::beast::websocket::stream<
            boost::asio::ssl::stream<boost::beast::tcp_stream>>>(strand_, ssl_context_);
        
        // Set SNI hostname
        if (!SSL_set_tlsext_host_name(ws_->next_layer().native_handle(), host.c_str())) {
            throw std::runtime_error("Failed to set SNI hostname");
        }
        
        // Resolve hostname
        boost::asio::ip::tcp::resolver resolver(io_context_);
        auto results = resolver.resolve(host, port);
        
        // Connect to server
        boost::beast::get_lowest_layer(*ws_).async_connect(
            results,
            boost::beast::bind_front_handler(&MarketDataFeed::onConnect, this)
        );
        
    } catch (const std::exception& e) {
        reportError("Failed to connect: " + std::string(e.what()));
        scheduleReconnect();
    }
}

void MarketDataFeed::onConnect(boost::beast::error_code ec, boost::asio::ip::tcp::endpoint endpoint) {
    (void)endpoint; // Mark as unused to suppress warning
    if (ec) {
        reportError("Connection failed: " + ec.message());
        scheduleReconnect();
        return;
    }
    
    std::cout << "TCP connection established, starting SSL handshake..." << std::endl;
    
    // Perform SSL handshake
    ws_->next_layer().async_handshake(
        boost::asio::ssl::stream_base::client,
        boost::beast::bind_front_handler(&MarketDataFeed::onHandshake, this)
    );
}

void MarketDataFeed::onHandshake(boost::beast::error_code ec) {
    if (ec) {
        reportError("SSL handshake failed: " + ec.message());
        scheduleReconnect();
        return;
    }
    
    std::cout << "SSL handshake successful, upgrading to WebSocket..." << std::endl;
    
    // Set WebSocket options
    ws_->set_option(boost::beast::websocket::stream_base::timeout::suggested(
        boost::beast::role_type::client));
    
    ws_->set_option(boost::beast::websocket::stream_base::decorator(
        [](boost::beast::websocket::request_type& req) {
            req.set(boost::beast::http::field::user_agent, "Velocore/1.0");
        }));
    
    // Perform WebSocket handshake
    ws_->async_handshake(
        "stream.data.alpaca.markets",
        "/v2/iex",
        [this](boost::beast::error_code ec) {
            if (ec) {
                reportError("WebSocket handshake failed: " + ec.message());
                scheduleReconnect();
                return;
            }
            
            std::cout << "WebSocket connection established!" << std::endl;
            
            updateConnectionStatus(true);
            reconnect_attempts_ = 0;
            
            // Start reading messages
            ws_->async_read(
                buffer_,
                boost::beast::bind_front_handler(&MarketDataFeed::onRead, this)
            );
            
            // Send authentication message
            authenticateConnection();
        }
    );
}



void MarketDataFeed::authenticateConnection() {
    const auto& alpaca_config = config_.getAlpacaConfig();
    
    nlohmann::json auth_msg;
    auth_msg["action"] = "auth";
    auth_msg["key"] = alpaca_config.api_key;
    auth_msg["secret"] = alpaca_config.api_secret;
    
    std::string auth_str = auth_msg.dump();
    std::cout << "Sending authentication..." << std::endl;
    
    sendMessage(auth_str);
}

void MarketDataFeed::sendMessage(const std::string& message) {
    if (!connected_) {
        std::cout << "Cannot send message: not connected" << std::endl;
        return;
    }
    
    ws_->async_write(
        boost::asio::buffer(message),
        boost::beast::bind_front_handler(&MarketDataFeed::onWrite, this)
    );
}

void MarketDataFeed::onWrite(boost::beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        reportError("Write failed: " + ec.message());
        scheduleReconnect();
        return;
    }
    
    std::cout << "Sent " << bytes_transferred << " bytes" << std::endl;
}

void MarketDataFeed::onRead(boost::beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        reportError("Read failed: " + ec.message());
        scheduleReconnect();
        return;
    }
    
    // Process the message
    std::string message = boost::beast::buffers_to_string(buffer_.data());
    buffer_.consume(bytes_transferred);
    
    handleMessage(message);
    
    // Continue reading
    ws_->async_read(
        buffer_,
        boost::beast::bind_front_handler(&MarketDataFeed::onRead, this)
    );
}

void MarketDataFeed::handleMessage(const std::string& message) {
    try {
        nlohmann::json json_msg = nlohmann::json::parse(message);
        
        // Handle different message types
        if (json_msg.contains("T")) {
            std::string msg_type = json_msg["T"];
            
            if (msg_type == "success" && json_msg.contains("msg")) {
                std::string success_msg = json_msg["msg"];
                if (success_msg == "authenticated") {
                    std::cout << "Successfully authenticated!" << std::endl;
                    authenticated_ = true;
                    sendSubscriptionMessage();
                }
            } else if (msg_type == "subscription") {
                processSubscriptionAck(json_msg);
            } else if (msg_type == "error") {
                std::string error_msg = json_msg.contains("msg") ? json_msg["msg"] : "Unknown error";
                reportError("Alpaca error: " + error_msg);
            }
        } else {
            // Handle market data messages
            parseMarketData(json_msg);
        }
        
    } catch (const std::exception& e) {
        std::cout << "Failed to parse message: " << e.what() << std::endl;
        std::cout << "Message: " << message << std::endl;
    }
}

void MarketDataFeed::sendSubscriptionMessage() {
    std::lock_guard<std::mutex> lock(subscriptions_mutex_);
    
    if (pending_subscriptions_.empty()) {
        return;
    }
    
    nlohmann::json sub_msg;
    sub_msg["action"] = "subscribe";
    
    std::vector<std::string> trades;
    std::vector<std::string> quotes;
    std::vector<std::string> bars;
    
    for (const auto& subscription : pending_subscriptions_) {
        if (subscription.trades) {
            trades.push_back(subscription.symbol);
        }
        if (subscription.quotes) {
            quotes.push_back(subscription.symbol);
        }
        if (subscription.bars) {
            bars.push_back(subscription.symbol);
        }
    }
    
    if (!trades.empty()) sub_msg["trades"] = trades;
    if (!quotes.empty()) sub_msg["quotes"] = quotes;
    if (!bars.empty()) sub_msg["bars"] = bars;
    
    std::string sub_str = sub_msg.dump();
    std::cout << "Sending subscription: " << sub_str << std::endl;
    
    sendMessage(sub_str);
}

void MarketDataFeed::processSubscriptionAck(const nlohmann::json& message) {
    (void)message; // Mark as unused to suppress warning
    std::lock_guard<std::mutex> lock(subscriptions_mutex_);
    
    // Add symbols to subscribed set
    for (const auto& subscription : pending_subscriptions_) {
        subscribed_symbols_.insert(subscription.symbol);
    }
    
    std::cout << "Subscription acknowledged for " << pending_subscriptions_.size() << " symbols" << std::endl;
    
    // Clear pending subscriptions
    pending_subscriptions_.clear();
}

void MarketDataFeed::parseMarketData(const nlohmann::json& message) {
    // Alpaca sends arrays of market data
    if (message.is_array()) {
        for (const auto& item : message) {
            if (item.contains("T")) {
                std::string msg_type = item["T"];
                
                if (msg_type == "t") {
                    // Trade message
                    MarketTick tick = parseTradeMessage(item);
                    broadcastBookUpdate(tick.symbol, tick);
                } else if (msg_type == "q") {
                    // Quote message
                    MarketTick tick = parseQuoteMessage(item);
                    broadcastBookUpdate(tick.symbol, tick);
                } else if (msg_type == "b") {
                    // Bar message
                    MarketTick tick = parseBarMessage(item);
                    broadcastBookUpdate(tick.symbol, tick);
                }
            }
        }
    }
}

MarketTick MarketDataFeed::parseTradeMessage(const nlohmann::json& trade_data) {
    MarketTick tick;
    tick.type = MarketDataType::Trade;
    tick.symbol = trade_data["S"];
    tick.trade_price = trade_data["p"];
    tick.trade_size = trade_data["s"];
    tick.timestamp = std::chrono::steady_clock::now();
    
    return tick;
}

MarketTick MarketDataFeed::parseQuoteMessage(const nlohmann::json& quote_data) {
    MarketTick tick;
    tick.type = MarketDataType::Quote;
    tick.symbol = quote_data["S"];
    tick.bid_price = quote_data["bp"];
    tick.ask_price = quote_data["ap"];
    tick.bid_size = quote_data["bs"];
    tick.ask_size = quote_data["as"];
    tick.timestamp = std::chrono::steady_clock::now();
    
    return tick;
}

MarketTick MarketDataFeed::parseBarMessage(const nlohmann::json& bar_data) {
    MarketTick tick;
    tick.type = MarketDataType::Bar;
    tick.symbol = bar_data["S"];
    tick.open = bar_data["o"];
    tick.high = bar_data["h"];
    tick.low = bar_data["l"];
    tick.close = bar_data["c"];
    tick.volume = bar_data["v"];
    tick.timestamp = std::chrono::steady_clock::now();
    
    return tick;
}

void MarketDataFeed::scheduleReconnect() {
    if (!running_) {
        return;
    }
    
    const auto& md_config = config_.getMarketDataConfig();
    
    if (reconnect_attempts_ >= md_config.max_reconnect_attempts) {
        reportError("Maximum reconnection attempts reached. Stopping.");
        running_ = false;
        return;
    }
    
    reconnect_attempts_++;
    int delay = md_config.reconnect_delay_ms * reconnect_attempts_;
    
    std::cout << "Scheduling reconnect in " << delay << "ms (attempt " 
              << reconnect_attempts_ << ")" << std::endl;
    
    reconnect_timer_->expires_after(std::chrono::milliseconds(delay));
    reconnect_timer_->async_wait([this](boost::beast::error_code ec) {
        if (!ec && running_) {
            connectWebSocket();
        }
    });
}

void MarketDataFeed::updateConnectionStatus(bool connected) {
    bool was_connected = connected_.exchange(connected);
    
    if (was_connected != connected) {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        if (connection_callback_) {
            connection_callback_(connected);
        }
    }
}

void MarketDataFeed::reportError(const std::string& error) {
    std::cout << "MarketDataFeed Error: " << error << std::endl;
    
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (error_callback_) {
        error_callback_(error);
    }
}

void MarketDataFeed::closeWebSocket() {
    if (ws_ && connected_) {
        ws_->async_close(
            boost::beast::websocket::close_code::normal,
            boost::beast::bind_front_handler(&MarketDataFeed::onClose, this)
        );
    }
}

void MarketDataFeed::onClose(boost::beast::error_code ec) {
    if (ec) {
        std::cout << "Close failed: " << ec.message() << std::endl;
    } else {
        std::cout << "WebSocket connection closed" << std::endl;
    }
    
    updateConnectionStatus(false);
    authenticated_ = false;
}

} // namespace velocore 