#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <functional>
#include <queue>
#include <chrono>
#include "models/include/Order.h"
#include "models/include/Trade.h"
#include "models/include/Types.h"
#include "persistence/TradeLogger.h"

namespace velocore {

// Portfolio position tracking
struct Position {
    std::string symbol;
    int quantity = 0;
    double average_price = 0.0;
    double realized_pnl = 0.0;
    double unrealized_pnl = 0.0;
    double market_value = 0.0;
    double cost_basis = 0.0;
    
    void update_market_value(double current_price) {
        market_value = quantity * current_price;
        unrealized_pnl = market_value - cost_basis;
    }
};

// Account portfolio
struct Portfolio {
    double cash_balance = 100000.0;  // Default starting balance
    double buying_power = 100000.0;
    double total_equity = 100000.0;
    double total_realized_pnl = 0.0;
    double total_unrealized_pnl = 0.0;
    std::unordered_map<std::string, Position> positions;
    
    nlohmann::json to_json() const;
};

// Order fill information
struct OrderFill {
    uint64_t order_id;
    std::string symbol;
    double fill_price;
    int fill_quantity;
    std::chrono::steady_clock::time_point fill_time;
    double commission = 0.0;
};

class PaperBroker {
public:
    using OnOrderUpdateCallback = std::function<void(const Order&)>;
    using OnFillCallback = std::function<void(const OrderFill&)>;
    using OnPortfolioUpdateCallback = std::function<void(const Portfolio&)>;
    
    PaperBroker(double initial_balance = 100000.0);
    ~PaperBroker();
    
    // Order management
    uint64_t submitOrder(const Order& order);
    bool cancelOrder(uint64_t order_id);
    bool modifyOrder(uint64_t order_id, double new_price, int new_quantity);
    
    // Portfolio queries
    Portfolio getPortfolio() const;
    Position getPosition(const std::string& symbol) const;
    std::vector<Order> getOpenOrders() const;
    std::vector<Order> getOrderHistory() const;
    
    // Market data updates
    void updateMarketPrice(const std::string& symbol, double bid, double ask, double last);
    void processMarketTick(const MarketTick& tick);
    
    // Callbacks
    void onOrderUpdate(OnOrderUpdateCallback callback);
    void onFill(OnFillCallback callback);
    void onPortfolioUpdate(OnPortfolioUpdateCallback callback);
    
    // Configuration
    void setCommissionRate(double rate);
    void setSlippageModel(std::function<double(const Order&, double market_price)> model);
    void enableLogging(std::shared_ptr<TradeLogger> logger);
    
    // Risk management
    void setMaxPositionSize(const std::string& symbol, int max_shares);
    void setMaxOrderValue(double max_value);
    void setDailyLossLimit(double limit);
    
private:
    // State management
    mutable std::mutex broker_mutex_;
    std::atomic<uint64_t> next_order_id_{1};
    std::atomic<uint64_t> next_trade_id_{1};
    
    // Portfolio and orders
    Portfolio portfolio_;
    std::unordered_map<uint64_t, Order> active_orders_;
    std::vector<Order> order_history_;
    std::unordered_map<std::string, std::vector<uint64_t>> symbol_orders_;
    
    // Market data cache
    struct MarketData {
        double bid = 0.0;
        double ask = 0.0;
        double last = 0.0;
        std::chrono::steady_clock::time_point last_update;
    };
    std::unordered_map<std::string, MarketData> market_data_;
    
    // Risk limits
    std::unordered_map<std::string, int> max_position_sizes_;
    double max_order_value_ = 1000000.0;
    double daily_loss_limit_ = 10000.0;
    double daily_realized_pnl_ = 0.0;
    std::chrono::system_clock::time_point last_pnl_reset_;
    
    // Execution configuration
    double commission_rate_ = 0.0;
    std::function<double(const Order&, double)> slippage_model_;
    
    // Callbacks
    OnOrderUpdateCallback order_update_callback_;
    OnFillCallback fill_callback_;
    OnPortfolioUpdateCallback portfolio_update_callback_;
    
    // Logging
    std::shared_ptr<TradeLogger> logger_;
    
    // Helper methods
    void processLimitOrder(Order& order);
    void processMarketOrder(Order& order);
    void executeOrder(Order& order, double fill_price);
    void updatePosition(const std::string& symbol, int quantity, double price, Side side);
    void updatePortfolioValue();
    bool validateOrder(const Order& order, std::string& rejection_reason);
    bool checkRiskLimits(const Order& order, std::string& rejection_reason);
    double calculateFillPrice(const Order& order, const MarketData& market) const;
    double applySlippage(const Order& order, double base_price) const;
    void resetDailyPnL();
    void notifyOrderUpdate(const Order& order);
    void notifyFill(const OrderFill& fill);
    void notifyPortfolioUpdate();
};

} // namespace velocore