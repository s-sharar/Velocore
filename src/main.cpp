#include <crow.h>
#include <iostream>
#include <thread>
#include <ctime>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <mutex>

#include "Types.h"
#include "Order.h"
#include "Trade.h"
#include "OrderBook.h"
#include "Config.h"
#include "MarketDataFeed.h"

using namespace velocore;

// Global instances
OrderBook orderBook;
TradeStatistics stats;
std::unique_ptr<MarketDataFeed> marketDataFeed;

// Market data storage
std::unordered_map<std::string, MarketTick> latestTicks;
std::mutex ticksMutex;

// Order validation function
bool validateOrder(const std::string& symbol, Side side, OrderType type, double price, int quantity, std::string& errorMessage) {
    if (quantity <= 0) {
        errorMessage = "Quantity must be greater than 0";
        return false;
    }
    
    if (type == OrderType::Limit && price <= 0) {
        errorMessage = "Price must be greater than 0 for limit orders";
        return false;
    }
    
    if (symbol.empty()) {
        errorMessage = "Symbol cannot be empty";
        return false;
    }
    
    return true;
}

// Market data callback functions
void onMarketTick(const MarketTick& tick) {
    std::lock_guard<std::mutex> lock(ticksMutex);
    latestTicks[tick.symbol] = tick;
    
    std::cout << "Received " << to_string(tick.type) << " for " << tick.symbol;
    if (tick.type == MarketDataType::Trade) {
        std::cout << " - Price: $" << tick.trade_price << ", Size: " << tick.trade_size;
    } else if (tick.type == MarketDataType::Quote) {
        std::cout << " - Bid: $" << tick.bid_price << " x " << tick.bid_size
                  << ", Ask: $" << tick.ask_price << " x " << tick.ask_size;
    }
    std::cout << std::endl;
}

void onMarketConnection(bool connected) {
    std::cout << "Market data connection: " << (connected ? "CONNECTED" : "DISCONNECTED") << std::endl;
}

void onMarketError(const std::string& error) {
    std::cout << "Market data error: " << error << std::endl;
}

int main() {
    std::cout << "=== Velocore Trading Simulator ===" << std::endl;
    
    try {
        // Load configuration
        std::cout << "Loading configuration..." << std::endl;
        Configuration& config = Configuration::getInstance();
        config.loadFromEnvironment();
        config.validateConfiguration();
        std::cout << "Configuration loaded successfully!" << std::endl;
        
        // Initialize market data feed
        std::cout << "Initializing market data feed..." << std::endl;
        marketDataFeed = std::make_unique<MarketDataFeed>();
        
        // Register callbacks
        marketDataFeed->onTick(onMarketTick);
        marketDataFeed->onConnection(onMarketConnection);
        marketDataFeed->onError(onMarketError);
        
        // Start market data feed
        marketDataFeed->start();
        
    } catch (const std::exception& e) {
        std::cout << "Configuration error: " << e.what() << std::endl;
        std::cout << "Please set the required environment variables:" << std::endl;
        std::cout << "  ALPACA_API_KEY=your_api_key" << std::endl;
        std::cout << "  ALPACA_API_SECRET=your_api_secret" << std::endl;
        std::cout << "Continuing without market data feed..." << std::endl;
    }
    
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
        Order sample_buy(1, "SIM", Side::Buy, OrderType::Limit, 100.50, 100);
        Order sample_sell(2, "SIM", Side::Sell, OrderType::Limit, 101.00, 50);
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
            
            // Extract order data for validation
            std::string symbol = json_data["symbol"].s();
            Side side = side_from_string(json_data["side"].s());
            OrderType type = order_type_from_string(json_data["type"].s());
            double price = json_data["price"].d();
            int quantity = json_data["quantity"].i();
            
            // VALIDATE THE ORDER
            std::string errorMessage;
            if (!validateOrder(symbol, side, type, price, quantity, errorMessage)) {
                return crow::response(400, crow::json::wvalue{{"error", errorMessage}});
            }
            
            // Create order from JSON (validation passed)
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
        auto trades = orderBook.getTradeLog();  // Returns copy for thread safety
        for (const auto& trade : trades) {
            trade_list.push_back(trade.to_json());
        }
        
        return crow::json::wvalue{
            {"trades", std::move(trade_list)},
            {"total_trades", static_cast<int>(trades.size())},
            {"statistics", stats.to_json()}
        };
    });
    
    CROW_ROUTE(app, "/trades/<int>")([]( int trade_id){
        auto trades = orderBook.getTradeLog();  // Returns copy for thread safety
        for (const auto& trade : trades) {
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
            {"total_trades", static_cast<int>(orderBook.getTradeCount())},
            {"last_trade_stats", stats.to_json()}
        };
    });
    
    // Concurrency testing endpoint - creates multiple simultaneous orders
    CROW_ROUTE(app, "/test/concurrency").methods("POST"_method)([](const crow::request& req){
        try {
            auto json_data = crow::json::load(req.body);
            if (!json_data) {
                return crow::response(400, "Invalid JSON");
            }
            
            int num_orders = json_data["num_orders"].i();
            int num_threads = json_data.has("num_threads") ? json_data["num_threads"].i() : 4;
            
            if (num_orders <= 0 || num_orders > 1000) {
                return crow::response(400, "num_orders must be between 1 and 1000");
            }
            
            std::vector<std::thread> threads;
            std::atomic<int> completed_orders{0};
            std::atomic<int> total_trades{0};
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // Create worker threads
            for (int t = 0; t < num_threads; ++t) {
                threads.emplace_back([&, t]() {
                    for (int i = t; i < num_orders; i += num_threads) {
                        try {
                            // Create alternating buy/sell orders
                            Side side = (i % 2 == 0) ? Side::Buy : Side::Sell;
                            double price = (side == Side::Buy) ? 99.0 + (i % 10) : 101.0 + (i % 10);
                            int quantity = 10 + (i % 40);
                            
                            Order order(i + 1000, "SIM", side, OrderType::Limit, price, quantity);
                            
                            // Submit order to matching engine
                            std::vector<Trade> trades = orderBook.addOrder(order);
                            
                            // Update counters
                            completed_orders.fetch_add(1);
                            total_trades.fetch_add(trades.size());
                            
                            // Update statistics
                            for (const auto& trade : trades) {
                                stats.update(trade);
                            }
                            
                        } catch (const std::exception& e) {
                            // Continue on error
                        }
                    }
                });
            }
            
            // Wait for all threads to complete
            for (auto& thread : threads) {
                thread.join();
            }
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            return crow::response(200, crow::json::wvalue{
                {"status", "completed"},
                {"orders_submitted", completed_orders.load()},
                {"trades_generated", total_trades.load()},
                {"duration_ms", static_cast<int>(duration.count())},
                {"threads_used", num_threads},
                {"orders_per_second", completed_orders.load() * 1000.0 / duration.count()},
                {"final_book_state", orderBook.getBookSnapshot(3)},
                {"final_statistics", stats.to_json()}
            });
            
        } catch (const std::exception& e) {
            return crow::response(500, crow::json::wvalue{{"error", e.what()}});
        }
    });
    
    // Market data endpoints
    CROW_ROUTE(app, "/market/status")([](){
        crow::json::wvalue response;
        response["connected"] = marketDataFeed ? marketDataFeed->isConnected() : false;
        response["subscribed_symbols"] = crow::json::wvalue::list();
        
        if (marketDataFeed) {
            auto symbols = marketDataFeed->getSubscribedSymbols();
            auto symbols_list = crow::json::wvalue::list();
            for (const auto& symbol : symbols) {
                symbols_list.push_back(symbol);
            }
            response["subscribed_symbols"] = std::move(symbols_list);
        }
        
        return response;
    });
    
    CROW_ROUTE(app, "/market/subscribe").methods("POST"_method)([](const crow::request& req){
        if (!marketDataFeed) {
            return crow::response{400, "Market data feed not initialized"};
        }
        
        try {
            auto json_data = crow::json::load(req.body);
            if (!json_data) {
                return crow::response{400, "Invalid JSON"};
            }
            
            std::string symbol = json_data["symbol"].s();
            bool trades = json_data.has("trades") ? json_data["trades"].b() : true;
            bool quotes = json_data.has("quotes") ? json_data["quotes"].b() : true;
            bool bars = json_data.has("bars") ? json_data["bars"].b() : false;
            
            marketDataFeed->subscribe(symbol, trades, quotes, bars);
            
            return crow::response{200, "Subscribed to " + symbol};
            
        } catch (const std::exception& e) {
            return crow::response{400, "Error: " + std::string(e.what())};
        }
    });
    
    CROW_ROUTE(app, "/market/data/<string>")([]( const std::string& symbol){
        std::lock_guard<std::mutex> lock(ticksMutex);
        
        auto it = latestTicks.find(symbol);
        if (it == latestTicks.end()) {
            return crow::response{404, "No data available for symbol: " + symbol};
        }
        
        return crow::response{200, it->second.to_json().dump()};
    });
    
    CROW_ROUTE(app, "/market/data")([](){
        std::lock_guard<std::mutex> lock(ticksMutex);
        
        crow::json::wvalue response;
        response["symbols"] = crow::json::wvalue::list();
        
        auto symbols_list = crow::json::wvalue::list();
        for (const auto& [symbol, tick] : latestTicks) {
            symbols_list.push_back(tick.to_json());
        }
        
        response["ticks"] = std::move(symbols_list);
        response["count"] = latestTicks.size();
        
        return response;
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
    std::cout << "  POST /test/concurrency   - Test concurrent order submission (for testing thread safety)" << std::endl;
    std::cout << "  GET  /market/status      - Market data connection status" << std::endl;
    std::cout << "  POST /market/subscribe   - Subscribe to market data for symbol" << std::endl;
    std::cout << "  GET  /market/data        - Get all cached market data" << std::endl;
    std::cout << "  GET  /market/data/<sym>  - Get latest market data for specific symbol" << std::endl;
    std::cout << std::endl;
    std::cout << "Server running with multithreading enabled..." << std::endl;
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads" << std::endl;
    
    app.port(port).multithreaded().run();
    
    // Cleanup
    std::cout << "Shutting down..." << std::endl;
    if (marketDataFeed) {
        marketDataFeed->stop();
        marketDataFeed.reset();
    }
    
    return 0;
} 