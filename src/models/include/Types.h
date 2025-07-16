#pragma once

#include <string>
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

std::string to_string(Side side);
std::string to_string(OrderType type);
std::string to_string(OrderStatus status);

Side side_from_string(const std::string& str);
OrderType order_type_from_string(const std::string& str);

crow::json::wvalue to_json(Side side);
crow::json::wvalue to_json(OrderType type);
crow::json::wvalue to_json(OrderStatus status);

}
