#include "MarketDataFeed.h"
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <sstream>
#include <set>

namespace velocore {

MarketDataFeed::MarketDataFeed() 
    : config_(Configuration::getInstance()),
      ssl_context_(boost::asio::ssl::context::tlsv12_client),
      strand_(boost::asio::make_strand(io_context_)) {
    
    // Configure SSL context
    ssl_context_.set_default_verify_paths();
    // In MarketDataFeed constructor
    if (std::getenv("DISABLE_SSL_VERIFY") && std::string(std::getenv("DISABLE_SSL_VERIFY")) == "true") {
        ssl_context_.set_verify_mode(boost::asio::ssl::verify_none);
        std::cout << "SSL verification disabled for development" << std::endl;
    } else {
        ssl_context_.set_verify_mode(boost::asio::ssl::verify_peer);
    }
    
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
    
    // Check if already in pending subscriptions
    for (const auto& pending : pending_subscriptions_) {
        if (pending.symbol == symbol) {
            std::cout << "Subscription for " << symbol << " already pending" << std::endl;
            return;
        }
    }
    
    // Create subscription
    MarketSubscription subscription(symbol);
    subscription.trades = trades;
    subscription.quotes = quotes;
    subscription.bars = bars;
    
    pending_subscriptions_.push_back(subscription);
    
    // If connected and authenticated, send subscription immediately
    if (connected_ && authenticated_) {
        // Post the subscription message to the io_context thread
        boost::asio::post(strand_, [this]() {
            sendSubscriptionMessage();
        });
    }
    
    std::cout << "Queued subscription for " << symbol << std::endl;
}

void MarketDataFeed::unsubscribe(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(subscriptions_mutex_);
    
    // Check if actually subscribed
    if (subscribed_symbols_.find(symbol) == subscribed_symbols_.end()) {
        std::cout << "Not subscribed to " << symbol << std::endl;
        return;
    }
    
    // Remove from subscribed symbols
    subscribed_symbols_.erase(symbol);
    
    // Remove from pending subscriptions
    pending_subscriptions_.erase(
        std::remove_if(pending_subscriptions_.begin(), pending_subscriptions_.end(),
            [&symbol](const MarketSubscription& sub) { return sub.symbol == symbol; }),
        pending_subscriptions_.end()
    );
    
    // Send unsubscription message to Alpaca if connected and authenticated
    if (connected_ && authenticated_) {
        // Post the unsubscription message to the io_context thread
        boost::asio::post(strand_, [this, symbol]() {
            sendUnsubscriptionMessage(symbol);
        });
    }
    
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
        
        // Parse configured URL
        std::string url = config_.getAlpacaConfig().data_url;
        std::string host, port, path;
        bool is_secure;
        
        if (!parseWebSocketURL(url, host, port, path, is_secure)) {
            throw std::runtime_error("Invalid WebSocket URL: " + url);
        }
        
        std::cout << "Connecting to " << host << ":" << port << path << " (secure: " << is_secure << ")" << std::endl;
        
        // Create WebSocket stream
        ws_ = std::make_unique<boost::beast::websocket::stream<
            boost::asio::ssl::stream<boost::beast::tcp_stream>>>(strand_, ssl_context_);
        
        // Set SNI hostname for SSL
        if (is_secure) {
            if (!SSL_set_tlsext_host_name(ws_->next_layer().native_handle(), host.c_str())) {
                throw std::runtime_error("Failed to set SNI hostname");
            }
        }
        
        // Store connection details for later use
        connection_host_ = host;
        connection_port_ = port;
        connection_path_ = path;
        connection_is_secure_ = is_secure;
        
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
        connection_host_,
        connection_path_,
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
    
    // Post the write operation to the io_context to ensure thread safety
    boost::asio::post(strand_, [this, message]() {
        ws_->async_write(
            boost::asio::buffer(message),
            boost::beast::bind_front_handler(&MarketDataFeed::onWrite, this)
        );
    });
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
        
        if (json_msg.is_array()) {
            for (const auto& item : json_msg) {
                handleSingleMessage(item);
            }
        } else {
            // Fallback for non-array messages (shouldn't happen with Alpaca)
            handleSingleMessage(json_msg);
        }
        
    } catch (const std::exception& e) {
        std::cout << "Failed to parse message: " << e.what() << std::endl;
        std::cout << "Message: " << message << std::endl;
    }
}

void MarketDataFeed::handleSingleMessage(const nlohmann::json& msg) {
    if (!msg.contains("T")) {
        return;
    }
    
    // Update last heartbeat time when we receive any message
    last_heartbeat_ = std::chrono::steady_clock::now();
    
    std::string msg_type = msg["T"];
    
    if (msg_type == "success" && msg.contains("msg")) {
        std::string success_msg = msg["msg"];
        if (success_msg == "authenticated") {
            std::cout << "Successfully authenticated!" << std::endl;
            authenticated_ = true;
            // Send pending subscriptions now that we're authenticated
            sendSubscriptionMessage();
            // Start heartbeat monitoring
            startHeartbeat();
        } else if (success_msg == "connected") {
            std::cout << "Successfully connected to Alpaca WebSocket!" << std::endl;
            // Connection established, authentication should be sent automatically
        }
    } else if (msg_type == "subscription") {
        processSubscriptionAck(msg);
    } else if (msg_type == "error") {
        // Handle different error message formats
        std::string error_msg = "Unknown error";
        if (msg.contains("msg")) {
            error_msg = msg["msg"];
        } else if (msg.contains("message")) {
            error_msg = msg["message"];
        }
        
        // Include error code if available
        if (msg.contains("code")) {
            error_msg = "Error " + std::to_string(msg["code"].get<int>()) + ": " + error_msg;
        }
        
        reportError("Alpaca error: " + error_msg);
    } else if (msg_type == "t" || msg_type == "q" || msg_type == "b" || msg_type == "d" || msg_type == "u") {
        // Market data message types:
        // t = trade, q = quote, b = minute bar, d = daily bar, u = updated bar
        std::cout << "Received market data: " << msg_type << " for " << msg.value("S", "unknown") << std::endl;
        nlohmann::json array_msg = nlohmann::json::array({msg});
        parseMarketData(array_msg);
    } else {
        // Log unknown message types for debugging
        std::cout << "Received message type: " << msg_type << " - " << msg.dump() << std::endl;
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
    std::lock_guard<std::mutex> lock(subscriptions_mutex_);
    
    // Clear current subscribed symbols since we're getting the full state
    subscribed_symbols_.clear();
    
    // Parse the actual subscription acknowledgment according to Alpaca docs
    // Example: {"T":"subscription","trades":["AAPL"],"quotes":["AMD","CLDR"],"bars":["*"],"updatedBars":[],"dailyBars":["VOO"],"statuses":["*"],"lulds":[],"corrections":["AAPL"],"cancelErrors":["AAPL"]}
    
    std::set<std::string> unique_symbols;
    
    // Check all possible channels and collect subscribed symbols
    if (message.contains("trades") && message["trades"].is_array()) {
        for (const auto& symbol : message["trades"]) {
            if (symbol.is_string()) {
                std::string sym = symbol.get<std::string>();
                subscribed_symbols_.insert(sym);
                unique_symbols.insert(sym);
            }
        }
    }
    
    if (message.contains("quotes") && message["quotes"].is_array()) {
        for (const auto& symbol : message["quotes"]) {
            if (symbol.is_string()) {
                std::string sym = symbol.get<std::string>();
                subscribed_symbols_.insert(sym);
                unique_symbols.insert(sym);
            }
        }
    }
    
    if (message.contains("bars") && message["bars"].is_array()) {
        for (const auto& symbol : message["bars"]) {
            if (symbol.is_string()) {
                std::string sym = symbol.get<std::string>();
                subscribed_symbols_.insert(sym);
                unique_symbols.insert(sym);
            }
        }
    }
    
    // Also check other channels (dailyBars, updatedBars, etc.)
    if (message.contains("dailyBars") && message["dailyBars"].is_array()) {
        for (const auto& symbol : message["dailyBars"]) {
            if (symbol.is_string()) {
                std::string sym = symbol.get<std::string>();
                subscribed_symbols_.insert(sym);
                unique_symbols.insert(sym);
            }
        }
    }
    
    if (message.contains("updatedBars") && message["updatedBars"].is_array()) {
        for (const auto& symbol : message["updatedBars"]) {
            if (symbol.is_string()) {
                std::string sym = symbol.get<std::string>();
                subscribed_symbols_.insert(sym);
                unique_symbols.insert(sym);
            }
        }
    }
    
    std::cout << "Subscription acknowledged for symbols: ";
    for (const auto& sym : unique_symbols) {
        std::cout << sym << " ";
    }
    std::cout << std::endl;
    
    // Clear pending subscriptions since they've been processed
    pending_subscriptions_.clear();
}

void MarketDataFeed::parseMarketData(const nlohmann::json& message) {
    // Alpaca sends arrays of market data
    if (message.is_array()) {
        for (const auto& item : message) {
            if (item.contains("T")) {
                std::string msg_type = item["T"];
                
                try {
                    if (msg_type == "t") {
                        // Trade message
                        MarketTick tick = parseTradeMessage(item);
                        if (!tick.symbol.empty()) {
                            std::cout << "Broadcasting trade: " << tick.symbol << " @ $" << tick.trade_price << " x " << tick.trade_size << std::endl;
                            broadcastBookUpdate(tick.symbol, tick);
                        }
                    } else if (msg_type == "q") {
                        // Quote message
                        MarketTick tick = parseQuoteMessage(item);
                        if (!tick.symbol.empty()) {
                            std::cout << "Broadcasting quote: " << tick.symbol << " bid $" << tick.bid_price << " ask $" << tick.ask_price << std::endl;
                            broadcastBookUpdate(tick.symbol, tick);
                        }
                    } else if (msg_type == "b" || msg_type == "d" || msg_type == "u") {
                        // Bar message (minute bars, daily bars, updated bars)
                        MarketTick tick = parseBarMessage(item);
                        if (!tick.symbol.empty()) {
                            std::cout << "Broadcasting bar: " << tick.symbol << " close $" << tick.close << std::endl;
                            broadcastBookUpdate(tick.symbol, tick);
                        }
                    } else {
                        std::cout << "Unknown market data message type: " << msg_type << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cout << "Error parsing market data message: " << e.what() << std::endl;
                    std::cout << "Message: " << item.dump() << std::endl;
                }
            }
        }
    }
}

MarketTick MarketDataFeed::parseTradeMessage(const nlohmann::json& trade_data) {
    MarketTick tick;
    tick.type = MarketDataType::Trade;
    
    // Required fields
    tick.symbol = trade_data.value("S", "");
    tick.trade_price = trade_data.value("p", 0.0);
    tick.trade_size = trade_data.value("s", 0);
    tick.timestamp = std::chrono::steady_clock::now();
    
    return tick;
}

MarketTick MarketDataFeed::parseQuoteMessage(const nlohmann::json& quote_data) {
    MarketTick tick;
    tick.type = MarketDataType::Quote;
    
    // Required fields
    tick.symbol = quote_data.value("S", "");
    tick.bid_price = quote_data.value("bp", 0.0);
    tick.ask_price = quote_data.value("ap", 0.0);
    tick.bid_size = quote_data.value("bs", 0);
    tick.ask_size = quote_data.value("as", 0);
    tick.timestamp = std::chrono::steady_clock::now();
    
    return tick;
}

MarketTick MarketDataFeed::parseBarMessage(const nlohmann::json& bar_data) {
    MarketTick tick;
    tick.type = MarketDataType::Bar;
    
    tick.symbol = bar_data.value("S", "");
    tick.open = bar_data.value("o", 0.0);
    tick.high = bar_data.value("h", 0.0);
    tick.low = bar_data.value("l", 0.0);
    tick.close = bar_data.value("c", 0.0);
    tick.volume = bar_data.value("v", 0);
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

void MarketDataFeed::sendUnsubscriptionMessage(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(subscriptions_mutex_);
    
    nlohmann::json unsub_msg;
    unsub_msg["action"] = "unsubscribe";
    unsub_msg["trades"] = {symbol};
    unsub_msg["quotes"] = {symbol};
    unsub_msg["bars"] = {symbol};
    
    std::string unsub_str = unsub_msg.dump();
    std::cout << "Sending unsubscription for " << symbol << ": " << unsub_str << std::endl;
    
    sendMessage(unsub_str);
}

bool MarketDataFeed::parseWebSocketURL(const std::string& url, std::string& host, 
                                     std::string& port, std::string& path, bool& is_secure) {
    // Default values
    path = "/";
    port = "443";
    is_secure = true;
    
    // Check for wss:// or ws:// prefix
    if (url.find("wss://") == 0) {
        is_secure = true;
        port = "443";
        std::string remainder = url.substr(6); // Remove "wss://"
        
        // Find host and path separation
        size_t path_pos = remainder.find('/');
        if (path_pos != std::string::npos) {
            std::string host_port = remainder.substr(0, path_pos);
            path = remainder.substr(path_pos);
            
            // Check for port in host:port format
            size_t port_pos = host_port.find(':');
            if (port_pos != std::string::npos) {
                host = host_port.substr(0, port_pos);
                port = host_port.substr(port_pos + 1);
            } else {
                host = host_port;
            }
        } else {
            host = remainder;
        }
    } else if (url.find("ws://") == 0) {
        is_secure = false;
        port = "80";
        std::string remainder = url.substr(5); // Remove "ws://"
        
        // Find host and path separation
        size_t path_pos = remainder.find('/');
        if (path_pos != std::string::npos) {
            std::string host_port = remainder.substr(0, path_pos);
            path = remainder.substr(path_pos);
            
            // Check for port in host:port format
            size_t port_pos = host_port.find(':');
            if (port_pos != std::string::npos) {
                host = host_port.substr(0, port_pos);
                port = host_port.substr(port_pos + 1);
            } else {
                host = host_port;
            }
        } else {
            host = remainder;
        }
    } else {
        // Invalid URL format
        return false;
    }
    
    return !host.empty();
}

void MarketDataFeed::startHeartbeat() {
    if (!running_) {
        return;
    }
    
    const auto& md_config = config_.getMarketDataConfig();
    last_heartbeat_ = std::chrono::steady_clock::now();
    
    heartbeat_timer_->expires_after(std::chrono::milliseconds(md_config.heartbeat_interval_ms));
    heartbeat_timer_->async_wait([this](boost::beast::error_code ec) {
        if (!ec && running_) {
            handleHeartbeat();
        }
    });
}

void MarketDataFeed::handleHeartbeat() {
    if (!running_ || !connected_) {
        return;
    }
    
    const auto& md_config = config_.getMarketDataConfig();
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_message = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_heartbeat_).count();
    
    // Check if we haven't received any messages in too long
    if (time_since_last_message > (md_config.heartbeat_interval_ms * 2)) {
        std::cout << "Heartbeat timeout - no messages received for " << time_since_last_message << "ms" << std::endl;
        reportError("Heartbeat timeout - connection may be stale");
        scheduleReconnect();
        return;
    }
    
    // Continue monitoring
    startHeartbeat();
}

} // namespace velocore 