#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "models/include/Order.h"
#include "models/include/Trade.h"
#include "models/include/OrderBook.h"

namespace velocore {

class TradeLogger {
public:
    TradeLogger(const std::string& log_directory = "logs");
    ~TradeLogger();

    // Core logging functions
    void logOrder(const Order& order);
    void logTrade(const Trade& trade);
    void logBookSnapshot(const OrderBook& book);
    void logMarketData(const MarketTick& tick);
    
    // Additional logging for paper trading
    void logPortfolioUpdate(const nlohmann::json& portfolio);
    void logOrderRejection(const Order& order, const std::string& reason);
    void logSystemEvent(const std::string& event_type, const nlohmann::json& details);
    
    // Utility functions
    void rotateLogs();
    std::string getCurrentLogPath() const;
    void flush();

private:
    std::string log_directory_;
    std::string current_date_;
    
    // Separate log files for different types
    std::unique_ptr<std::ofstream> order_log_;
    std::unique_ptr<std::ofstream> trade_log_;
    std::unique_ptr<std::ofstream> book_snapshot_log_;
    std::unique_ptr<std::ofstream> market_data_log_;
    std::unique_ptr<std::ofstream> system_event_log_;
    
    mutable std::mutex log_mutex_;
    
    // Helper functions
    void initializeLogs();
    void closeAllLogs();
    std::string getTimestamp() const;
    std::string getDateString() const;
    void checkAndRotateLogs();
    void writeToLog(std::ofstream& log, const nlohmann::json& data);
    
    // Log formatting
    nlohmann::json formatOrder(const Order& order) const;
    nlohmann::json formatTrade(const Trade& trade) const;
    nlohmann::json formatBookSnapshot(const OrderBook& book) const;
    nlohmann::json formatMarketTick(const MarketTick& tick) const;
};

} // namespace velocore