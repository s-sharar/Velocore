#include <crow.h>
#include <iostream>
#include <thread>
#include <ctime>
#include <vector>
#include <memory>
#include <algorithm>

#include "Types.h"
#include "Order.h"
#include "Trade.h"

using namespace velocore;

std::vector<std::unique_ptr<Order>> orders;
std::vector<std::unique_ptr<Trade>> trades;
TradeStatistics stats;

int main() {
    std::cout << "=== Velocore Trading Simulator ===" << std::endl;
    std::cout << "Initializing Crow web framework..." << std::endl;
    
    crow::SimpleApp app;
    
    CROW_ROUTE(app, "/ping")([](){
        std::cout << "Ping endpoint accessed" << std::endl;
        return crow::json::wvalue{{"message", "pong"}};
    });
    
    CROW_ROUTE(app, "/health")([](){
        std::cout << "Health check endpoint accessed" << std::endl;
        return crow::json::wvalue{
            {"status", "healthy"},
            {"service", "velocore"},
            {"threads", static_cast<std::int32_t>(std::thread::hardware_concurrency())},
            {"timestamp", static_cast<std::int64_t>(std::time(nullptr))}
        };
    });
    
    CROW_ROUTE(app, "/architecture")([](){
        return crow::json::wvalue{
            {"system", "Velocore Trading Simulator"},
            {"components", crow::json::wvalue::list{
                "Data Models (Order, Trade structs)",
                "Matching Engine (Order book with price-time priority)",
                "API Endpoints (REST API via Crow)",
                "Concurrency & Safety (Thread-safe operations)",
                "Latency Simulation (Network/processing delays)"
            }},
            {"separation_of_concerns", crow::json::wvalue{
                {"api_layer", "HTTP handling via Crow"},
                {"business_logic", "Matching engine operations"},
                {"data_layer", "Order and Trade data models"}
            }}
        };
    });
    
    CROW_ROUTE(app, "/models/demo")([](){
        Order sample_buy("SIM", Side::Buy, OrderType::Limit, 100.50, 100);
        Order sample_sell("SIM", Side::Sell, OrderType::Limit, 101.00, 50);
        Trade sample_trade(sample_buy.id, sample_sell.id, "SIM", 100.75, 50);
        
        return crow::json::wvalue{
            {"message", "Data Models Demonstration"},
            {"sample_buy_order", sample_buy.to_json()},
            {"sample_sell_order", sample_sell.to_json()},
            {"sample_trade", sample_trade.to_json()},
            {"enums", crow::json::wvalue{
                {"sides", crow::json::wvalue::list{"BUY", "SELL"}},
                {"order_types", crow::json::wvalue::list{"LIMIT", "MARKET"}},
                {"order_statuses", crow::json::wvalue::list{"ACTIVE", "FILLED", "CANCELLED", "PARTIALLY_FILLED"}}
            }}
        };
    });
    
    CROW_ROUTE(app, "/orders").methods("POST"_method)([](const crow::request& req){
        try {
            auto json_data = crow::json::load(req.body);
            if (!json_data) {
                return crow::response(400, "Invalid JSON");
            }
            
            auto order = std::make_unique<Order>(Order::from_json(json_data));
            auto response = order->to_json();
            
            orders.push_back(std::move(order));
            
            return crow::response(201, response);
        } catch (const std::exception& e) {
            return crow::response(400, crow::json::wvalue{{"error", e.what()}});
        }
    });
    
    CROW_ROUTE(app, "/orders")([](){
        crow::json::wvalue::list order_list;
        for (const auto& order : orders) {
            order_list.push_back(order->to_json());
        }
        
        return crow::json::wvalue{
            {"orders", std::move(order_list)},
            {"total_orders", static_cast<int>(orders.size())}
        };
    });
    
    CROW_ROUTE(app, "/orders/<int>")([]( int order_id){
        for (const auto& order : orders) {
            if (order->id == static_cast<uint64_t>(order_id)) {
                return crow::response(200, order->to_json());
            }
        }
        return crow::response(404, crow::json::wvalue{{"error", "Order not found"}});
    });
    
    CROW_ROUTE(app, "/trades").methods("POST"_method)([](const crow::request& req){
        try {
            auto json_data = crow::json::load(req.body);
            if (!json_data) {
                return crow::response(400, "Invalid JSON");
            }
            
            auto trade = std::make_unique<Trade>(Trade::from_json(json_data));
            stats.update(*trade);
            auto response = trade->to_json();
            
            trades.push_back(std::move(trade));
            
            return crow::response(201, response);
        } catch (const std::exception& e) {
            return crow::response(400, crow::json::wvalue{{"error", e.what()}});
        }
    });
    
    CROW_ROUTE(app, "/trades")([](){
        crow::json::wvalue::list trade_list;
        for (const auto& trade : trades) {
            trade_list.push_back(trade->to_json());
        }
        
        return crow::json::wvalue{
            {"trades", std::move(trade_list)},
            {"total_trades", static_cast<int>(trades.size())},
            {"statistics", stats.to_json()}
        };
    });
    
    CROW_ROUTE(app, "/trades/<int>")([]( int trade_id){
        for (const auto& trade : trades) {
            if (trade->trade_id == static_cast<uint64_t>(trade_id)) {
                return crow::response(200, trade->to_json());
            }
        }
        return crow::response(404, crow::json::wvalue{{"error", "Trade not found"}});
    });
    
    CROW_ROUTE(app, "/statistics")([](){
        return crow::json::wvalue{
            {"orders", crow::json::wvalue{
                {"total", static_cast<int>(orders.size())},
                {"active", static_cast<int>(std::count_if(orders.begin(), orders.end(), 
                    [](const auto& o) { return o->is_active(); }))},
                {"filled", static_cast<int>(std::count_if(orders.begin(), orders.end(), 
                    [](const auto& o) { return o->is_filled(); }))},
                {"cancelled", static_cast<int>(std::count_if(orders.begin(), orders.end(), 
                    [](const auto& o) { return o->is_cancelled(); }))}
            }},
            {"trades", stats.to_json()}
        };
    });
    
    const int port = 18080;
    std::cout << "Starting server on port " << port << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  /ping               - Simple ping/pong test" << std::endl;
    std::cout << "  GET  /health             - Detailed health check" << std::endl;
    std::cout << "  GET  /architecture       - System architecture overview" << std::endl;
    std::cout << "  GET  /models/demo        - Data models demonstration" << std::endl;
    std::cout << "  POST /orders             - Create new order" << std::endl;
    std::cout << "  GET  /orders             - List all orders" << std::endl;
    std::cout << "  GET  /orders/<id>        - Get specific order" << std::endl;
    std::cout << "  POST /trades             - Create new trade" << std::endl;
    std::cout << "  GET  /trades             - List all trades" << std::endl;
    std::cout << "  GET  /trades/<id>        - Get specific trade" << std::endl;
    std::cout << "  GET  /statistics         - System statistics" << std::endl;
    std::cout << std::endl;
    std::cout << "Server running with multithreading enabled..." << std::endl;
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads" << std::endl;
    
    app.port(port).multithreaded().run();
    
    return 0;
} 