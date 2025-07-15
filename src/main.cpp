#include <crow.h>
#include <iostream>
#include <thread>

int main() {
    std::cout << "=== Velocore Trading Simulator ===" << std::endl;
    std::cout << "Initializing Crow web framework..." << std::endl;
    
    crow::SimpleApp app;
    
    CROW_ROUTE(app, "/ping")([](){
        std::cout << "Ping endpoint accessed" << std::endl;
        return crow::json::wvalue{{"message", "pong"}};  // returns JSON {"message": "pong"}
    });
    
    CROW_ROUTE(app, "/health")([](){
        std::cout << "Health check endpoint accessed" << std::endl;
        return crow::json::wvalue{
            {"status", "healthy"},
            {"service", "velocore"},
            {"threads", std::thread::hardware_concurrency()},
            {"timestamp", std::time(nullptr)}
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
    
    const int port = 18080;
    std::cout << "Starting server on port " << port << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  GET /ping        - Simple ping/pong test" << std::endl;
    std::cout << "  GET /health      - Detailed health check" << std::endl;
    std::cout << "  GET /architecture - System architecture overview" << std::endl;
    std::cout << std::endl;
    std::cout << "Server running with multithreading enabled..." << std::endl;
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads" << std::endl;
    
    app.port(port).multithreaded().run();
    
    return 0;
} 