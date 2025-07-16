#include "../include/Types.h"
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

Side side_from_string(const std::string& str) {
    if (str == "BUY") return Side::Buy;
    if (str == "SELL") return Side::Sell;
    throw std::invalid_argument("Invalid side: " + str);
}

OrderType order_type_from_string(const std::string& str) {
    if (str == "LIMIT") return OrderType::Limit;
    if (str == "MARKET") return OrderType::Market;
    throw std::invalid_argument("Invalid order type: " + str);
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

} 