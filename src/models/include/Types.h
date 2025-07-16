#pragma once

#include <string>
#include <chrono>
#include <crow/json.h>

namespace velocore {

enum class Side {
    Buy,
    Sell
};

enum class OrderType {
    Limit,
    Market
};

enum class OrderStatus {
    Active,
    Filled,
    Cancelled,
    PartiallyFilled
};

// Market data types for Alpaca WebSocket feed
enum class MarketDataType {
    Trade,
    Quote,
    Bar
};

// Market tick structure for real-time market data
struct MarketTick {
    std::string symbol;
    MarketDataType type;
    std::chrono::steady_clock::time_point timestamp;
    
    // Trade data
    double trade_price = 0.0;
    int trade_size = 0;
    
    // Quote data
    double bid_price = 0.0;
    double ask_price = 0.0;
    int bid_size = 0;
    int ask_size = 0;
    
    // Bar data (for minute bars)
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    int volume = 0;
    
    MarketTick() = default;
    
    MarketTick(const std::string& sym, MarketDataType t) 
        : symbol(sym), type(t), timestamp(std::chrono::steady_clock::now()) {}
    
    crow::json::wvalue to_json() const;
};

// Market data subscription info
struct MarketSubscription {
    std::string symbol;
    bool trades = false;
    bool quotes = false;
    bool bars = false;
    
    MarketSubscription(const std::string& sym) : symbol(sym) {}
};

std::string to_string(Side side);
std::string to_string(OrderType type);
std::string to_string(OrderStatus status);
std::string to_string(MarketDataType type);

Side side_from_string(const std::string& str);
OrderType order_type_from_string(const std::string& str);
MarketDataType market_data_type_from_string(const std::string& str);

crow::json::wvalue to_json(Side side);
crow::json::wvalue to_json(OrderType type);
crow::json::wvalue to_json(OrderStatus status);
crow::json::wvalue to_json(MarketDataType type);

}
