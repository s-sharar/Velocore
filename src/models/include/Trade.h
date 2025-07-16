#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <limits>
#include <crow/json.h>

namespace velocore {

struct Trade {
    uint64_t trade_id;
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    std::string symbol;
    double price;
    int quantity;
    std::chrono::steady_clock::time_point timestamp;
    
    Trade() = default;
    
    Trade(uint64_t buy_order_id, uint64_t sell_order_id, const std::string& symbol, 
          double price, int quantity);
    
    static uint64_t generate_id();
    
private:
    static std::atomic<uint64_t> id_counter;
    
public:
    double total_value() const;
    
    crow::json::wvalue to_json() const;
    static Trade from_json(const crow::json::rvalue& json);
};

bool operator<(const Trade& lhs, const Trade& rhs);

struct TradeStatistics {
    int total_trades;
    int total_volume;
    double total_value;
    double avg_price;
    double min_price;
    double max_price;
    std::chrono::steady_clock::time_point last_trade_time;
    
    TradeStatistics();
    
    void update(const Trade& trade);
    crow::json::wvalue to_json() const;
};

} 