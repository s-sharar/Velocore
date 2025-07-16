#include "Trade.h"
#include <limits>

namespace velocore {

std::atomic<uint64_t> Trade::id_counter{1};

Trade::Trade(uint64_t buy_order_id, uint64_t sell_order_id, const std::string& symbol, 
             double price, int quantity)
    : trade_id(generate_id())
    , buy_order_id(buy_order_id)
    , sell_order_id(sell_order_id)
    , symbol(symbol)
    , price(price)
    , quantity(quantity)
    , timestamp(std::chrono::steady_clock::now()) {}

TradeStatistics::TradeStatistics() 
    : total_trades(0)
    , total_volume(0)
    , total_value(0.0)
    , avg_price(0.0)
    , min_price(std::numeric_limits<double>::max())
    , max_price(std::numeric_limits<double>::lowest()) {}

uint64_t Trade::generate_id() {
    return id_counter.fetch_add(1);
}

double Trade::total_value() const {
    return price * quantity;
}

crow::json::wvalue Trade::to_json() const {
    auto duration = timestamp.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    
    return crow::json::wvalue{
        {"trade_id", static_cast<int64_t>(trade_id)},
        {"buy_order_id", static_cast<int64_t>(buy_order_id)},
        {"sell_order_id", static_cast<int64_t>(sell_order_id)},
        {"symbol", symbol},
        {"price", price},
        {"quantity", quantity},
        {"total_value", total_value()},
        {"timestamp", millis}
    };
}

Trade Trade::from_json(const crow::json::rvalue& json) {
    Trade trade;
    trade.trade_id = generate_id();
    trade.buy_order_id = json["buy_order_id"].u();
    trade.sell_order_id = json["sell_order_id"].u();
    trade.symbol = json["symbol"].s();
    trade.price = json["price"].d();
    trade.quantity = json["quantity"].i();
    trade.timestamp = std::chrono::steady_clock::now();
    return trade;
}

bool operator<(const Trade& lhs, const Trade& rhs) {
    return lhs.timestamp < rhs.timestamp;
}

void TradeStatistics::update(const Trade& trade) {
    total_trades++;
    total_volume += trade.quantity;
    total_value += trade.total_value();
    avg_price = total_value / total_volume;
    
    if (trade.price < min_price) min_price = trade.price;
    if (trade.price > max_price) max_price = trade.price;
    
    last_trade_time = trade.timestamp;
}

crow::json::wvalue TradeStatistics::to_json() const {
    auto duration = last_trade_time.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    
    return crow::json::wvalue{
        {"total_trades", total_trades},
        {"total_volume", total_volume},
        {"total_value", total_value},
        {"avg_price", avg_price},
        {"min_price", min_price == std::numeric_limits<double>::max() ? 0.0 : min_price},
        {"max_price", max_price == std::numeric_limits<double>::lowest() ? 0.0 : max_price},
        {"last_trade_time", millis}
    };
}

} 