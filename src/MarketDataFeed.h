#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <chrono>
#include <mutex>

#include "Types.h"
#include "Config.h"

namespace velocore {

class MarketDataFeed {
public:
    using OnTickCallback = std::function<void(const MarketTick&)>;
    using OnConnectionCallback = std::function<void(bool connected)>;
    using OnErrorCallback = std::function<void(const std::string& error)>;

    MarketDataFeed();
    ~MarketDataFeed();

    // Main interface methods
    void start();
    void stop();
    void subscribe(const std::string& symbol, bool trades = true, bool quotes = true, bool bars = false);
    void unsubscribe(const std::string& symbol);
    
    // Callback registration
    void onTick(OnTickCallback callback);
    void onConnection(OnConnectionCallback callback);
    void onError(OnErrorCallback callback);
    
    // Status methods
    bool isConnected() const;
    std::vector<std::string> getSubscribedSymbols() const;
    
    // Broadcast method for system integration
    void broadcastBookUpdate(const std::string& symbol, const MarketTick& tick);

private:
    // WebSocket connection management
    void connectWebSocket();
    void runWebSocket();
    void reconnectWebSocket();
    void closeWebSocket();
    
    // Authentication and subscription
    void authenticateConnection();
    void sendSubscriptionMessage();
    void sendUnsubscriptionMessage(const std::string& symbol);
    void processSubscriptionAck(const nlohmann::json& message);
    
    // Message handling
    void handleMessage(const std::string& message);
    void handleSingleMessage(const nlohmann::json& msg);
    void parseMarketData(const nlohmann::json& message);
    MarketTick parseTradeMessage(const nlohmann::json& trade_data);
    MarketTick parseQuoteMessage(const nlohmann::json& quote_data);
    MarketTick parseBarMessage(const nlohmann::json& bar_data);
    
    // WebSocket operations
    void onConnect(boost::beast::error_code ec, boost::asio::ip::tcp::endpoint endpoint);
    void onHandshake(boost::beast::error_code ec);
    void onWrite(boost::beast::error_code ec, std::size_t bytes_transferred);
    void onRead(boost::beast::error_code ec, std::size_t bytes_transferred);
    void onClose(boost::beast::error_code ec);
    
    // Utility methods
    void sendMessage(const std::string& message);
    void scheduleReconnect();
    void updateConnectionStatus(bool connected);
    void reportError(const std::string& error);
    bool parseWebSocketURL(const std::string& url, std::string& host, 
                          std::string& port, std::string& path, bool& is_secure);
    void startHeartbeat();
    void handleHeartbeat();
    
    // Configuration and state
    const Configuration& config_;
    std::atomic<bool> running_{false};
    std::atomic<bool> connected_{false};
    std::atomic<bool> authenticated_{false};
    std::atomic<int> reconnect_attempts_{0};
    
    // Connection details
    std::string connection_host_;
    std::string connection_port_;
    std::string connection_path_;
    bool connection_is_secure_{true};
    
    // WebSocket components
    boost::asio::io_context io_context_;
    boost::asio::ssl::context ssl_context_;
    std::unique_ptr<boost::beast::websocket::stream<
        boost::asio::ssl::stream<boost::beast::tcp_stream>>> ws_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::beast::flat_buffer buffer_;
    std::thread worker_thread_;
    
    // Subscription management
    mutable std::mutex subscriptions_mutex_;
    std::unordered_set<std::string> subscribed_symbols_;
    std::vector<MarketSubscription> pending_subscriptions_;
    
    // Callbacks
    OnTickCallback tick_callback_;
    OnConnectionCallback connection_callback_;
    OnErrorCallback error_callback_;
    mutable std::mutex callback_mutex_;
    
    // Timing and heartbeat
    std::chrono::steady_clock::time_point last_heartbeat_;
    std::unique_ptr<boost::asio::steady_timer> heartbeat_timer_;
    std::unique_ptr<boost::asio::steady_timer> reconnect_timer_;
};

} // namespace velocore 