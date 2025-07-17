#include "TradeLogger.h"
#include <iostream>
#include <ctime>

namespace velocore {

TradeLogger::TradeLogger(const std::string& log_directory) 
    : log_directory_(log_directory) {
    // Create log directory if it doesn't exist
    std::filesystem::create_directories(log_directory_);
    current_date_ = getDateString();
    initializeLogs();
}

TradeLogger::~TradeLogger() {
    flush();
    closeAllLogs();
}

void TradeLogger::initializeLogs() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    // Close existing logs if any
    closeAllLogs();
    
    // Create log file paths with date
    std::string date_prefix = log_directory_ + "/" + current_date_;
    
    // Open log files in append mode
    order_log_ = std::make_unique<std::ofstream>(
        date_prefix + "_orders.jsonl", std::ios::app);
    trade_log_ = std::make_unique<std::ofstream>(
        date_prefix + "_trades.jsonl", std::ios::app);
    book_snapshot_log_ = std::make_unique<std::ofstream>(
        date_prefix + "_book_snapshots.jsonl", std::ios::app);
    market_data_log_ = std::make_unique<std::ofstream>(
        date_prefix + "_market_data.jsonl", std::ios::app);
    system_event_log_ = std::make_unique<std::ofstream>(
        date_prefix + "_system_events.jsonl", std::ios::app);
    
    // Log initialization event
    nlohmann::json init_event;
    init_event["event"] = "logger_initialized";
    init_event["timestamp"] = getTimestamp();
    init_event["log_directory"] = log_directory_;
    writeToLog(*system_event_log_, init_event);
}

void TradeLogger::closeAllLogs() {
    if (order_log_ && order_log_->is_open()) order_log_->close();
    if (trade_log_ && trade_log_->is_open()) trade_log_->close();
    if (book_snapshot_log_ && book_snapshot_log_->is_open()) book_snapshot_log_->close();
    if (market_data_log_ && market_data_log_->is_open()) market_data_log_->close();
    if (system_event_log_ && system_event_log_->is_open()) system_event_log_->close();
}

void TradeLogger::logOrder(const Order& order) {
    checkAndRotateLogs();
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (order_log_ && order_log_->is_open()) {
        writeToLog(*order_log_, formatOrder(order));
    }
}

void TradeLogger::logTrade(const Trade& trade) {
    checkAndRotateLogs();
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (trade_log_ && trade_log_->is_open()) {
        writeToLog(*trade_log_, formatTrade(trade));
    }
}

void TradeLogger::logBookSnapshot(const OrderBook& book) {
    checkAndRotateLogs();
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (book_snapshot_log_ && book_snapshot_log_->is_open()) {
        writeToLog(*book_snapshot_log_, formatBookSnapshot(book));
    }
}

void TradeLogger::logMarketData(const MarketTick& tick) {
    checkAndRotateLogs();
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (market_data_log_ && market_data_log_->is_open()) {
        writeToLog(*market_data_log_, formatMarketTick(tick));
    }
}

void TradeLogger::logPortfolioUpdate(const nlohmann::json& portfolio) {
    checkAndRotateLogs();
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (system_event_log_ && system_event_log_->is_open()) {
        nlohmann::json event;
        event["event"] = "portfolio_update";
        event["timestamp"] = getTimestamp();
        event["portfolio"] = portfolio;
        writeToLog(*system_event_log_, event);
    }
}

void TradeLogger::logOrderRejection(const Order& order, const std::string& reason) {
    checkAndRotateLogs();
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (system_event_log_ && system_event_log_->is_open()) {
        nlohmann::json event;
        event["event"] = "order_rejection";
        event["timestamp"] = getTimestamp();
        event["order"] = formatOrder(order);
        event["reason"] = reason;
        writeToLog(*system_event_log_, event);
    }
}

void TradeLogger::logSystemEvent(const std::string& event_type, const nlohmann::json& details) {
    checkAndRotateLogs();
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (system_event_log_ && system_event_log_->is_open()) {
        nlohmann::json event;
        event["event"] = event_type;
        event["timestamp"] = getTimestamp();
        event["details"] = details;
        writeToLog(*system_event_log_, event);
    }
}

void TradeLogger::rotateLogs() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::string new_date = getDateString();
    if (new_date != current_date_) {
        current_date_ = new_date;
        initializeLogs();
    }
}

std::string TradeLogger::getCurrentLogPath() const {
    return log_directory_ + "/" + current_date_;
}

void TradeLogger::flush() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    if (order_log_) order_log_->flush();
    if (trade_log_) trade_log_->flush();
    if (book_snapshot_log_) book_snapshot_log_->flush();
    if (market_data_log_) market_data_log_->flush();
    if (system_event_log_) system_event_log_->flush();
}

std::string TradeLogger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return ss.str();
}

std::string TradeLogger::getDateString() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y%m%d");
    return ss.str();
}

void TradeLogger::checkAndRotateLogs() {
    std::string new_date = getDateString();
    if (new_date != current_date_) {
        rotateLogs();
    }
}

void TradeLogger::writeToLog(std::ofstream& log, const nlohmann::json& data) {
    log << data.dump() << std::endl;
}

nlohmann::json TradeLogger::formatOrder(const Order& order) const {
    nlohmann::json j;
    j["timestamp"] = getTimestamp();
    j["order_id"] = order.order_id;
    j["symbol"] = order.symbol;
    j["side"] = to_string(order.side);
    j["type"] = to_string(order.type);
    j["price"] = order.price;
    j["quantity"] = order.quantity;
    j["remaining_quantity"] = order.remaining_quantity;
    j["status"] = to_string(order.status);
    j["trader_id"] = order.trader_id;
    return j;
}

nlohmann::json TradeLogger::formatTrade(const Trade& trade) const {
    nlohmann::json j;
    j["timestamp"] = getTimestamp();
    j["trade_id"] = trade.trade_id;
    j["symbol"] = trade.symbol;
    j["price"] = trade.price;
    j["quantity"] = trade.quantity;
    j["buyer_order_id"] = trade.buyer_order_id;
    j["seller_order_id"] = trade.seller_order_id;
    j["buyer_id"] = trade.buyer_id;
    j["seller_id"] = trade.seller_id;
    return j;
}

nlohmann::json TradeLogger::formatBookSnapshot(const OrderBook& book) const {
    nlohmann::json j;
    j["timestamp"] = getTimestamp();
    j["symbol"] = book.get_symbol();
    
    // Get bid levels
    auto bids = book.get_bid_levels();
    nlohmann::json bid_array = nlohmann::json::array();
    for (const auto& level : bids) {
        nlohmann::json bid_level;
        bid_level["price"] = level.price;
        bid_level["quantity"] = level.quantity;
        bid_level["order_count"] = level.order_count;
        bid_array.push_back(bid_level);
    }
    j["bids"] = bid_array;
    
    // Get ask levels
    auto asks = book.get_ask_levels();
    nlohmann::json ask_array = nlohmann::json::array();
    for (const auto& level : asks) {
        nlohmann::json ask_level;
        ask_level["price"] = level.price;
        ask_level["quantity"] = level.quantity;
        ask_level["order_count"] = level.order_count;
        ask_array.push_back(ask_level);
    }
    j["asks"] = ask_array;
    
    return j;
}

nlohmann::json TradeLogger::formatMarketTick(const MarketTick& tick) const {
    nlohmann::json j;
    j["timestamp"] = getTimestamp();
    j["symbol"] = tick.symbol;
    j["type"] = to_string(tick.type);
    
    switch (tick.type) {
        case MarketDataType::Trade:
            j["trade_price"] = tick.trade_price;
            j["trade_size"] = tick.trade_size;
            break;
            
        case MarketDataType::Quote:
            j["bid_price"] = tick.bid_price;
            j["ask_price"] = tick.ask_price;
            j["bid_size"] = tick.bid_size;
            j["ask_size"] = tick.ask_size;
            break;
            
        case MarketDataType::Bar:
            j["open"] = tick.open;
            j["high"] = tick.high;
            j["low"] = tick.low;
            j["close"] = tick.close;
            j["volume"] = tick.volume;
            break;
    }
    
    return j;
}

} // namespace velocore