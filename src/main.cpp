#include <crow.h>
#include <iostream>
#include <thread>
#include <ctime>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <optional>
#include <string>

#include "Types.h"
#include "Order.h"
#include "Trade.h"
#include "OrderBook.h"
#include "OrderBookRegistry.h"
#include "MarketTickStore.h"
#include "Config.h"
#include "MarketDataFeed.h"
#include <nlohmann/json.hpp>
#include "brokers/impl/AlpacaPaperBroker.h"

using namespace velocore;

// CORS middleware
struct CORSMiddleware {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context&) {
        // Handle CORS preflight
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 204; // No Content
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.add_header("Access-Control-Max-Age", "86400");
            res.end();
        }
    }

    void after_handle(crow::request&, crow::response& res, context&) {
        // Add CORS headers to all responses
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    }
};

// Global instances
OrderBookRegistry orderBooks;
MarketTickStore tickStore;
TradeStatistics stats;
std::unique_ptr<MarketDataFeed> marketDataFeed;
std::unique_ptr<Broker> broker;

std::optional<std::string> querySymbol(const crow::request& req) {
    if (const char* symbol = req.url_params.get("symbol")) {
        if (symbol[0] != '\0') {
            return std::string(symbol);
        }
    }
    return std::nullopt;
}

// Order validation function
bool validateOrder(const std::string& symbol, Side side, OrderType type, double price, int quantity, std::string& errorMessage) {
    (void)side;
    
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
    tickStore.upsert(tick);

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
        // Initialize Alpaca paper broker
        std::cout << "Initializing Alpaca paper trading broker..." << std::endl;
        broker = std::make_unique<AlpacaPaperBroker>();
        
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
    
    crow::App<CORSMiddleware> app;
    
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
            
            // Process order through the matching engine (per-symbol book)
            std::vector<Trade> executedTrades = orderBooks.getOrCreate(order.symbol).addOrder(order);
            
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
    
    CROW_ROUTE(app, "/orders")([](const crow::request& req){
        auto symbol = querySymbol(req);
        if (symbol) {
            OrderBook* book = orderBooks.tryGet(*symbol);
            if (!book) {
                return crow::response(404, crow::json::wvalue{{"error", "No order book for symbol: " + *symbol}});
            }
            return crow::response(200, crow::json::wvalue{
                {"symbol", *symbol},
                {"message", "Use /orderbook?symbol= for current order book state"},
                {"active_orders", static_cast<int>(book->getTotalOrders())},
                {"book_statistics", book->getBookStatistics()}
            });
        }

        return crow::response(200, crow::json::wvalue{
            {"message", "Use /orderbook?symbol= for current order book state"},
            {"active_orders", static_cast<int>(orderBooks.totalOrders())},
            {"book_statistics", orderBooks.aggregateStatistics()}
        });
    });
    
    CROW_ROUTE(app, "/orderbook")([](const crow::request& req){
        auto symbol = querySymbol(req);
        if (!symbol) {
            return crow::response(400, crow::json::wvalue{{"error", "Missing required query parameter: symbol"}});
        }

        // Get number of levels to display (default: 5)
        int levels = 5;
        if (req.url_params.get("levels")) {
            levels = std::stoi(req.url_params.get("levels"));
            levels = std::max(1, std::min(levels, 20)); // Limit between 1 and 20
        }

        OrderBook* book = orderBooks.tryGet(*symbol);
        if (!book) {
            return crow::response(404, crow::json::wvalue{{"error", "No order book for symbol: " + *symbol}});
        }
        
        return crow::response(200, crow::json::wvalue{
            {"symbol", *symbol},
            {"orderbook", book->getBookSnapshot(levels)},
            {"statistics", book->getBookStatistics()}
        });
    });
    
    CROW_ROUTE(app, "/trades").methods("POST"_method)([](const crow::request& req){
        (void)req;
        return crow::response(405, crow::json::wvalue{
            {"error", "Manual trade creation not allowed"},
            {"message", "Trades are automatically created by the matching engine when orders are matched"}
        });
    });
    
    CROW_ROUTE(app, "/trades")([](const crow::request& req){
        crow::json::wvalue::list trade_list;
        auto symbol = querySymbol(req);
        if (symbol) {
            OrderBook* book = orderBooks.tryGet(*symbol);
            if (!book) {
                return crow::response(404, crow::json::wvalue{{"error", "No order book for symbol: " + *symbol}});
            }
            auto trades = book->getTradeLog();
            for (const auto& trade : trades) {
                trade_list.push_back(trade.to_json());
            }
            return crow::response(200, crow::json::wvalue{
                {"symbol", *symbol},
                {"trades", std::move(trade_list)},
                {"total_trades", static_cast<int>(trades.size())},
                {"statistics", stats.to_json()}
            });
        }

        auto books = orderBooks.allBooks();
        for (auto& [sym, book] : books) {
            (void)sym;
            for (const auto& trade : book->getTradeLog()) {
                trade_list.push_back(trade.to_json());
            }
        }

        return crow::response(200, crow::json::wvalue{
            {"trades", std::move(trade_list)},
            {"total_trades", static_cast<int>(orderBooks.totalTrades())},
            {"statistics", stats.to_json()}
        });
    });
    
    CROW_ROUTE(app, "/trades/<int>")([](const crow::request& req, int trade_id){
        auto symbol = querySymbol(req);
        if (symbol) {
            OrderBook* book = orderBooks.tryGet(*symbol);
            if (!book) {
                return crow::response(404, crow::json::wvalue{{"error", "No order book for symbol: " + *symbol}});
            }
            for (const auto& trade : book->getTradeLog()) {
                if (trade.trade_id == static_cast<uint64_t>(trade_id)) {
                    return crow::response(200, trade.to_json());
                }
            }
            return crow::response(404, crow::json::wvalue{{"error", "Trade not found"}});
        }

        for (auto& [sym, book] : orderBooks.allBooks()) {
            (void)sym;
            for (const auto& trade : book->getTradeLog()) {
                if (trade.trade_id == static_cast<uint64_t>(trade_id)) {
                    return crow::response(200, trade.to_json());
                }
            }
        }
        return crow::response(404, crow::json::wvalue{{"error", "Trade not found"}});
    });
    
    CROW_ROUTE(app, "/statistics")([](const crow::request& req){
        auto symbol = querySymbol(req);
        if (symbol) {
            OrderBook* book = orderBooks.tryGet(*symbol);
            if (!book) {
                return crow::response(404, crow::json::wvalue{{"error", "No order book for symbol: " + *symbol}});
            }
            return crow::response(200, crow::json::wvalue{
                {"symbol", *symbol},
                {"orderbook", book->getBookStatistics()},
                {"market_data", crow::json::wvalue{
                    {"best_bid", book->getBestBid()},
                    {"best_ask", book->getBestAsk()},
                    {"spread", book->getSpread()}
                }},
                {"trades", stats.to_json()}
            });
        }

        crow::json::wvalue market_data;
        for (auto& [sym, book] : orderBooks.allBooks()) {
            market_data[sym] = crow::json::wvalue{
                {"best_bid", book->getBestBid()},
                {"best_ask", book->getBestAsk()},
                {"spread", book->getSpread()}
            };
        }

        return crow::response(200, crow::json::wvalue{
            {"orderbook", orderBooks.aggregateStatistics()},
            {"market_data", std::move(market_data)},
            {"trades", stats.to_json()}
        });
    });
    
    CROW_ROUTE(app, "/orders/<int>/cancel").methods("POST"_method)([](const crow::request& req, int order_id){
        auto symbol = querySymbol(req);
        if (!symbol) {
            return crow::response(400, crow::json::wvalue{{"error", "Missing required query parameter: symbol"}});
        }

        OrderBook* book = orderBooks.tryGet(*symbol);
        if (!book) {
            return crow::response(404, crow::json::wvalue{
                {"error", "No order book for symbol: " + *symbol},
                {"order_id", order_id}
            });
        }

        bool cancelled = book->cancelOrder(static_cast<uint64_t>(order_id));
        
        if (cancelled) {
            return crow::response(200, crow::json::wvalue{
                {"message", "Order cancelled successfully"},
                {"order_id", order_id},
                {"symbol", *symbol}
            });
        } else {
            return crow::response(404, crow::json::wvalue{
                {"error", "Order not found or already executed"},
                {"order_id", order_id},
                {"symbol", *symbol}
            });
        }
    });
    
    CROW_ROUTE(app, "/market")([](const crow::request& req){
        auto symbol = querySymbol(req);
        if (symbol) {
            OrderBook* book = orderBooks.tryGet(*symbol);
            if (!book) {
                return crow::response(404, crow::json::wvalue{{"error", "No order book for symbol: " + *symbol}});
            }
            return crow::response(200, crow::json::wvalue{
                {"symbol", *symbol},
                {"best_bid", book->getBestBid()},
                {"best_ask", book->getBestAsk()},
                {"spread", book->getSpread()},
                {"total_active_orders", static_cast<int>(book->getTotalOrders())},
                {"total_trades", static_cast<int>(book->getTradeCount())},
                {"last_trade_stats", stats.to_json()}
            });
        }

        crow::json::wvalue by_symbol;
        for (auto& [sym, book] : orderBooks.allBooks()) {
            by_symbol[sym] = crow::json::wvalue{
                {"best_bid", book->getBestBid()},
                {"best_ask", book->getBestAsk()},
                {"spread", book->getSpread()},
                {"total_active_orders", static_cast<int>(book->getTotalOrders())},
                {"total_trades", static_cast<int>(book->getTradeCount())}
            };
        }

        return crow::response(200, crow::json::wvalue{
            {"symbols", std::move(by_symbol)},
            {"total_active_orders", static_cast<int>(orderBooks.totalOrders())},
            {"total_trades", static_cast<int>(orderBooks.totalTrades())},
            {"last_trade_stats", stats.to_json()}
        });
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
            OrderBook& simBook = orderBooks.getOrCreate("SIM");
            
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
                            std::vector<Trade> trades = simBook.addOrder(order);
                            
                            // Update counters
                            completed_orders.fetch_add(1);
                            total_trades.fetch_add(static_cast<int>(trades.size()));
                            
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
            double duration_ms = static_cast<double>(duration.count());
            if (duration_ms <= 0.0) {
                duration_ms = 1.0;
            }
            
            return crow::response(200, crow::json::wvalue{
                {"status", "completed"},
                {"symbol", "SIM"},
                {"orders_submitted", completed_orders.load()},
                {"trades_generated", total_trades.load()},
                {"duration_ms", static_cast<int>(duration.count())},
                {"threads_used", num_threads},
                {"orders_per_second", completed_orders.load() * 1000.0 / duration_ms},
                {"final_book_state", simBook.getBookSnapshot(3)},
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
            
            // Add debugging information
            std::cout << "Subscription request for symbol: " << symbol << std::endl;
            std::cout << "MarketDataFeed connected: " << (marketDataFeed->isConnected() ? "true" : "false") << std::endl;
            
            // Check if symbol is valid
            if (symbol.empty()) {
                return crow::response{400, "Symbol cannot be empty"};
            }
            
            // Attempt subscription
            try {
                marketDataFeed->subscribe(symbol, trades, quotes, bars);
                std::cout << "Successfully queued subscription for " << symbol << std::endl;
                return crow::response{200, "Subscribed to " + symbol};
            } catch (const std::exception& e) {
                std::cout << "Exception during subscription: " << e.what() << std::endl;
                return crow::response{500, "Internal error during subscription: " + std::string(e.what())};
            }
            
        } catch (const std::exception& e) {
            std::cout << "Exception in subscription endpoint: " << e.what() << std::endl;
            return crow::response{400, "Error: " + std::string(e.what())};
        }
    });
    
    CROW_ROUTE(app, "/market/data/<string>")([](const std::string& symbol){
        auto tick = tickStore.get(symbol);
        if (!tick) {
            return crow::response{404, "No data available for symbol: " + symbol};
        }
        return crow::response{200, tick->to_json()};
    });
    
    CROW_ROUTE(app, "/market/data")([](){
        auto ticks = tickStore.snapshotAll();
        crow::json::wvalue::list symbols_list;
        for (const auto& tick : ticks) {
            symbols_list.push_back(tick.to_json());
        }

        return crow::json::wvalue{
            {"ticks", std::move(symbols_list)},
            {"count", static_cast<int>(ticks.size())}
        };
    });

    // Alpaca paper trading REST endpoints
    CROW_ROUTE(app, "/alpaca/account")([](){
        if (!broker) return crow::response{400, "Broker not initialized"};
        nlohmann::json res = broker->getAccount();
        int status = res.value("_status", 200);
        res.erase("_status");
        return crow::response{status, res.dump()};
    });

    CROW_ROUTE(app, "/alpaca/positions")([](){
        if (!broker) return crow::response{400, "Broker not initialized"};
        nlohmann::json res = broker->getPositions();
        int status = res.value("_status", 200);
        res.erase("_status");
        return crow::response{status, res.dump()};
    });

    CROW_ROUTE(app, "/alpaca/orders")([](const crow::request& req){
        if (!broker) return crow::response{400, "Broker not initialized"};
        std::string status = "open";
        int limit = 50;
        std::string after;
        std::string until;
        if (req.url_params.get("status")) status = req.url_params.get("status");
        if (req.url_params.get("limit")) limit = std::stoi(req.url_params.get("limit"));
        if (req.url_params.get("after")) after = req.url_params.get("after");
        if (req.url_params.get("until")) until = req.url_params.get("until");
        nlohmann::json res = broker->listOrders(status, limit, after, until);
        int code = res.value("_status", 200);
        res.erase("_status");
        return crow::response{code, res.dump()};
    });

    CROW_ROUTE(app, "/alpaca/orders/<string>")([](const std::string& oid){
        if (!broker) return crow::response{400, "Broker not initialized"};
        nlohmann::json res = broker->getOrder(oid);
        int code = res.value("_status", 200);
        res.erase("_status");
        return crow::response{code, res.dump()};
    });

    CROW_ROUTE(app, "/alpaca/orders").methods("POST"_method)([](const crow::request& req){
        if (!broker) return crow::response{400, "Broker not initialized"};
        try {
            nlohmann::json body = nlohmann::json::parse(req.body);
            auto toLower = [](std::string s){
                std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
                return s;
            };
            nlohmann::json alp;
            bool passThrough = body.contains("symbol") && (body.contains("qty") || body.contains("notional")) && body.contains("side") && body.contains("type");
            if (passThrough) {
                alp = body;
            } else {
                alp["symbol"] = body.value("symbol", "");
                std::string side = body.value("side", "");
                std::string type = body.value("type", "");
                if (side.empty() && body.contains("Side")) side = body["Side"].get<std::string>();
                if (type.empty() && body.contains("OrderType")) type = body["OrderType"].get<std::string>();
                alp["side"] = toLower(side);
                alp["type"] = toLower(type);
                if (body.contains("qty")) alp["qty"] = body["qty"]; else alp["qty"] = body.value("quantity", 0);
                if (alp["type"] == "limit") {
                    if (body.contains("limit_price")) alp["limit_price"] = body["limit_price"]; else alp["limit_price"] = body.value("price", 0.0);
                }
                alp["time_in_force"] = body.value("time_in_force", std::string("day"));
            }
            if (!alp.contains("time_in_force")) alp["time_in_force"] = "day";
            nlohmann::json res = broker->placeOrder(alp);
            int code = res.value("_status", 200);
            res.erase("_status");
            return crow::response{code, res.dump()};
        } catch (const std::exception& e) {
            return crow::response{400, std::string("Invalid JSON: ") + e.what()};
        }
    });

    CROW_ROUTE(app, "/alpaca/orders/<string>").methods("DELETE"_method)([](const std::string& oid){
        if (!broker) return crow::response{400, "Broker not initialized"};
        bool ok = broker->cancelOrder(oid);
        return ok ? crow::response{204} : crow::response{500, "Failed to cancel"};
    });

    CROW_ROUTE(app, "/alpaca/orders").methods("DELETE"_method)([](){
        if (!broker) return crow::response{400, "Broker not initialized"};
        nlohmann::json res = broker->cancelAllOrders();
        int code = res.value("_status", 200);
        res.erase("_status");
        return crow::response{code, res.dump()};
    });
    
    const int port = 18080;
    std::cout << "Starting server on port " << port << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET  /ping               - Simple ping/pong test" << std::endl;
    std::cout << "  GET  /health             - Detailed health check" << std::endl;
    std::cout << "  GET  /architecture       - System architecture overview" << std::endl;
    std::cout << "  GET  /models/demo        - Data models demonstration" << std::endl;
    std::cout << "  POST /orders             - Submit new order (triggers matching engine)" << std::endl;
    std::cout << "  GET  /orders             - Order book summary (?symbol= optional)" << std::endl;
    std::cout << "  GET  /orderbook          - Order book snapshot (?symbol= required, levels=N)" << std::endl;
    std::cout << "  POST /orders/<id>/cancel - Cancel an active order (?symbol= required)" << std::endl;
    std::cout << "  GET  /trades             - List executed trades (?symbol= optional)" << std::endl;
    std::cout << "  GET  /trades/<id>        - Get specific trade (?symbol= optional)" << std::endl;
    std::cout << "  GET  /market             - Market summary (?symbol= optional)" << std::endl;
    std::cout << "  GET  /statistics         - Stats and book metrics (?symbol= optional)" << std::endl;
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