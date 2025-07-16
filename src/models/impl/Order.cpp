#include "Order.h"
#include <stdexcept>

namespace velocore {

std::atomic<uint64_t> Order::id_counter{1};

Order::Order(uint64_t client_id, const std::string& symbol, Side side, OrderType type, double price, int quantity)
    : id(generate_id())
    , client_id(client_id)
    , symbol(symbol)
    , side(side)
    , type(type)
    , price(price)
    , quantity(quantity)
    , remaining_quantity(quantity)
    , status(OrderStatus::Active)
    , timestamp(std::chrono::steady_clock::now()) {}

uint64_t Order::generate_id() {
    return id_counter.fetch_add(1);
}

void Order::fill(int fill_qty) {
    if (fill_qty <= 0 || fill_qty > remaining_quantity) {
        throw std::invalid_argument("Invalid fill quantity");
    }
    
    remaining_quantity -= fill_qty;
    
    if (remaining_quantity == 0) {
        status = OrderStatus::Filled;
    } else {
        status = OrderStatus::PartiallyFilled;
    }
}

void Order::cancel() {
    if (status == OrderStatus::Active || status == OrderStatus::PartiallyFilled) {
        status = OrderStatus::Cancelled;
    }
}

crow::json::wvalue Order::to_json() const {
    auto duration = timestamp.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    
    return crow::json::wvalue{
        {"id", static_cast<int64_t>(id)},
        {"client_id", static_cast<int64_t>(client_id)},
        {"symbol", symbol},
        {"side", to_string(side)},
        {"type", to_string(type)},
        {"price", price},
        {"quantity", quantity},
        {"remaining_quantity", remaining_quantity},
        {"filled_quantity", filled_quantity()},
        {"fill_percentage", fill_percentage()},
        {"status", to_string(status)},
        {"timestamp", millis}
    };
}

Order Order::from_json(const crow::json::rvalue& json) {
    Order order;
    order.client_id = json["client_id"].u();
    order.symbol = json["symbol"].s();
    order.side = side_from_string(json["side"].s());
    order.type = order_type_from_string(json["type"].s());
    order.price = json["price"].d();
    order.quantity = json["quantity"].i();
    order.remaining_quantity = order.quantity;
    order.status = OrderStatus::Active;
    order.timestamp = std::chrono::steady_clock::now();
    order.id = generate_id();
    return order;
}

bool operator<(const Order& lhs, const Order& rhs) {
    if (lhs.side != rhs.side) {
        return lhs.side < rhs.side;
    }
    
    if (lhs.is_buy()) {
        if (lhs.price != rhs.price) {
            return lhs.price > rhs.price;
        }
    } else {
        if (lhs.price != rhs.price) {
            return lhs.price < rhs.price;
        }
    }
    
    return lhs.timestamp < rhs.timestamp;
}

} 