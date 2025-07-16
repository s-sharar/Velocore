#include "Types.h"
#include <stdexcept>

namespace velocore {

std::string to_string(Side side) {
    switch (side) {
        case Side::Buy:  return "BUY";
        case Side::Sell: return "SELL";
        default:         return "UNKNOWN";
    }
}

std::string to_string(OrderType type) {
    switch (type) {
        case OrderType::Limit:  return "LIMIT";
        case OrderType::Market: return "MARKET";
        default:                return "UNKNOWN";
    }
}

std::string to_string(OrderStatus status) {
    switch (status) {
        case OrderStatus::Active:          return "ACTIVE";
        case OrderStatus::Filled:          return "FILLED";
        case OrderStatus::Cancelled:       return "CANCELLED";
        case OrderStatus::PartiallyFilled: return "PARTIALLY_FILLED";
        default:                           return "UNKNOWN";
    }
}

std::string to_string(MarketDataType type) {
    switch (type) {
        case MarketDataType::Trade: return "TRADE";
        case MarketDataType::Quote: return "QUOTE";
        case MarketDataType::Bar:   return "BAR";
        default:                    return "UNKNOWN";
    }
}

Side side_from_string(const std::string& str) {
    if (str == "BUY" || str == "buy") return Side::Buy;
    if (str == "SELL" || str == "sell") return Side::Sell;
    throw std::invalid_argument("Invalid side: " + str);
}

OrderType order_type_from_string(const std::string& str) {
    if (str == "LIMIT" || str == "limit") return OrderType::Limit;
    if (str == "MARKET" || str == "market") return OrderType::Market;
    throw std::invalid_argument("Invalid order type: " + str);
}

MarketDataType market_data_type_from_string(const std::string& str) {
    if (str == "TRADE" || str == "trade" || str == "t") return MarketDataType::Trade;
    if (str == "QUOTE" || str == "quote" || str == "q") return MarketDataType::Quote;
    if (str == "BAR" || str == "bar" || str == "b") return MarketDataType::Bar;
    throw std::invalid_argument("Invalid market data type: " + str);
}

crow::json::wvalue to_json(Side side) {
    return crow::json::wvalue{to_string(side)};
}

crow::json::wvalue to_json(OrderType type) {
    return crow::json::wvalue{to_string(type)};
}

crow::json::wvalue to_json(OrderStatus status) {
    return crow::json::wvalue{to_string(status)};
}

crow::json::wvalue to_json(MarketDataType type) {
    return crow::json::wvalue{to_string(type)};
}

crow::json::wvalue MarketTick::to_json() const {
    crow::json::wvalue json;
    json["symbol"] = symbol;
    json["type"] = to_string(type);
    json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()).count();
    
    if (type == MarketDataType::Trade) {
        json["trade_price"] = trade_price;
        json["trade_size"] = trade_size;
    } else if (type == MarketDataType::Quote) {
        json["bid_price"] = bid_price;
        json["ask_price"] = ask_price;
        json["bid_size"] = bid_size;
        json["ask_size"] = ask_size;
    } else if (type == MarketDataType::Bar) {
        json["open"] = open;
        json["high"] = high;
        json["low"] = low;
        json["close"] = close;
        json["volume"] = volume;
    }
    
    return json;
}

} 