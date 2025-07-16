#pragma once

#include "Types.h"
#include <atomic>
#include <chrono>
#include <string>
#include <crow/json.h>

namespace velocore {

struct Order {
    uint64_t id;
    uint64_t client_id;  // Client ID for latency simulation
    std::string symbol;
    Side side;
    OrderType type;
    double price;
    int quantity;
    int remaining_quantity;
    OrderStatus status;
    std::chrono::steady_clock::time_point timestamp;
    
    Order() = default;
    
    Order(uint64_t client_id, const std::string& symbol, Side side, OrderType type, double price, int quantity);
    
    static uint64_t generate_id();
    
private:
    static std::atomic<uint64_t> id_counter;
    
public:
    bool is_buy() const { return side == Side::Buy; }
    bool is_sell() const { return side == Side::Sell; }
    bool is_limit() const { return type == OrderType::Limit; }
    bool is_market() const { return type == OrderType::Market; }
    bool is_active() const { return status == OrderStatus::Active; }
    bool is_filled() const { return status == OrderStatus::Filled; }
    bool is_cancelled() const { return status == OrderStatus::Cancelled; }
    bool is_partially_filled() const { return status == OrderStatus::PartiallyFilled; }
    
    int filled_quantity() const { return quantity - remaining_quantity; }
    double fill_percentage() const { 
        return quantity > 0 ? (double)filled_quantity() / quantity * 100.0 : 0.0; 
    }
    
    void fill(int fill_qty);
    void cancel();
    
    crow::json::wvalue to_json() const;
    static Order from_json(const crow::json::rvalue& json);
};

bool operator<(const Order& lhs, const Order& rhs);

} 