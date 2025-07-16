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
#include "OrderBook.h"

using namespace velocore;

// Global order book instance - the heart of the matching engine
OrderBook orderBook;
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
            
            // Create order from JSON
            Order order = Order::from_json(json_data);
            
            // Process order through the matching engine
            std::vector<Trade> executedTrades = orderBook.addOrder(order);
            
            // Update statistics with any executed trades
            for (const auto& trade : executedTrades) {
                stats.update(trade);
            }
            
            // Prepare response with order details and immediate executions
            crow::json::wvalue response;
            response["order"] = order.to_json();
            response["immediate_executions"] = static_cast<int>(executedTrades.size());
            
            if (!executedTrades.empty()) {
                crow::json::wvalue::list trade_list;
                for (const auto& trade : executedTrades) {
                    trade_list.push_back(trade.to_json());
                }
                response["trades"] = std::move(trade_list);
            }
            
            return crow::response(201, response);
        } catch (const std::exception& e) {
            return crow::response(400, crow::json::wvalue{{"error", e.what()}});
        }
    });
    
    CROW_ROUTE(app, "/orders")([](){
        return crow::json::wvalue{
            {"message", "Use /orderbook for current order book state"},
            {"active_orders", static_cast<int>(orderBook.getTotalOrders())},
            {"book_statistics", orderBook.getBookStatistics()}
        };
    });
    
    CROW_ROUTE(app, "/orderbook")([](const crow::request& req){
        // Get number of levels to display (default: 5)
        int levels = 5;
        if (req.url_params.get("levels")) {
            levels = std::stoi(req.url_params.get("levels"));
            levels = std::max(1, std::min(levels, 20)); // Limit between 1 and 20
        }
        
        return crow::json::wvalue{
            {"orderbook", orderBook.getBookSnapshot(levels)},
            {"statistics", orderBook.getBookStatistics()}
        };
    });
    
    CROW_ROUTE(app, "/trades").methods("POST"_method)([](const crow::request& req){
        return crow::response(405, crow::json::wvalue{
            {"error", "Manual trade creation not allowed"},
            {"message", "Trades are automatically created by the matching engine when orders are matched"}
        });
    });
    
    CROW_ROUTE(app, "/trades")([](){
        crow::json::wvalue::list trade_list;
        for (const auto& trade : orderBook.getTradeLog()) {
            trade_list.push_back(trade.to_json());
        }
        
        return crow::json::wvalue{
            {"trades", std::move(trade_list)},
            {"total_trades", static_cast<int>(orderBook.getTradeLog().size())},
            {"statistics", stats.to_json()}
        };
    });
    
    CROW_ROUTE(app, "/trades/<int>")([]( int trade_id){
        for (const auto& trade : orderBook.getTradeLog()) {
            if (trade.trade_id == static_cast<uint64_t>(trade_id)) {
                return crow::response(200, trade.to_json());
            }
        }
        return crow::response(404, crow::json::wvalue{{"error", "Trade not found"}});
    });
    
    CROW_ROUTE(app, "/statistics")([](){
        return crow::json::wvalue{
            {"orderbook", orderBook.getBookStatistics()},
            {"market_data", crow::json::wvalue{
                {"best_bid", orderBook.getBestBid()},
                {"best_ask", orderBook.getBestAsk()},
                {"spread", orderBook.getSpread()}
            }},
            {"trades", stats.to_json()}
        };
    });
    
    CROW_ROUTE(app, "/orders/<int>/cancel").methods("POST"_method)([](int order_id){
        bool cancelled = orderBook.cancelOrder(static_cast<uint64_t>(order_id));
        
        if (cancelled) {
            return crow::response(200, crow::json::wvalue{
                {"message", "Order cancelled successfully"},
                {"order_id", order_id}
            });
        } else {
            return crow::response(404, crow::json::wvalue{
                {"error", "Order not found or already executed"},
                {"order_id", order_id}
            });
        }
    });
    
    CROW_ROUTE(app, "/market")([](){
        return crow::json::wvalue{
            {"symbol", "SIM"},
            {"best_bid", orderBook.getBestBid()},
            {"best_ask", orderBook.getBestAsk()},
            {"spread", orderBook.getSpread()},
            {"total_active_orders", static_cast<int>(orderBook.getTotalOrders())},
            {"total_trades", static_cast<int>(orderBook.getTradeLog().size())},
            {"last_trade_stats", stats.to_json()}
        };
    });
    
    const int port = 18080;
    std::cout << "Starting server on port " << port << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  /ping               - Simple ping/pong test" << std::endl;
    std::cout << "  GET  /health             - Detailed health check" << std::endl;
    std::cout << "  GET  /architecture       - System architecture overview" << std::endl;
    std::cout << "  GET  /models/demo        - Data models demonstration" << std::endl;
    std::cout << "  POST /orders             - Submit new order (triggers matching engine)" << std::endl;
    std::cout << "  GET  /orders             - Order book summary" << std::endl;
    std::cout << "  GET  /orderbook          - Current order book snapshot (levels=N)" << std::endl;
    std::cout << "  POST /orders/<id>/cancel - Cancel an active order" << std::endl;
    std::cout << "  GET  /trades             - List all executed trades" << std::endl;
    std::cout << "  GET  /trades/<id>        - Get specific trade" << std::endl;
    std::cout << "  GET  /market             - Current market data summary" << std::endl;
    std::cout << "  GET  /statistics         - Market statistics and order book metrics" << std::endl;
    std::cout << std::endl;
    std::cout << "Server running with multithreading enabled..." << std::endl;
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads" << std::endl;
    
    app.port(port).multithreaded().run();
    
    return 0;
} 