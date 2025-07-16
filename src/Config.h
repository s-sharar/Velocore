#pragma once

#include <string>
#include <stdexcept>
#include <cstdlib>

namespace velocore {

class Configuration {
public:
    // Alpaca API Configuration
    struct AlpacaConfig {
        std::string api_key;
        std::string api_secret;
        std::string base_url = "https://paper-api.alpaca.markets";
        std::string data_url = "wss://stream.data.alpaca.markets/v2/iex";
        bool is_paper_trading = true;
    };

    // Market Data Configuration
    struct MarketDataConfig {
        int reconnect_delay_ms = 5000;
        int max_reconnect_attempts = 10;
        int heartbeat_interval_ms = 30000;
        int connection_timeout_ms = 30000;
    };

    // General Configuration
    struct GeneralConfig {
        int server_port = 8080;
        std::string log_level = "INFO";
        bool debug_mode = false;
    };

    static Configuration& getInstance() {
        static Configuration instance;
        return instance;
    }

    void loadFromEnvironment() {
        // Load Alpaca configuration from environment variables
        alpaca_.api_key = getEnvVar("ALPACA_API_KEY");
        alpaca_.api_secret = getEnvVar("ALPACA_API_SECRET");
        
        // Optional overrides
        if (const char* base_url = std::getenv("ALPACA_BASE_URL")) {
            alpaca_.base_url = base_url;
        }
        
        if (const char* data_url = std::getenv("ALPACA_DATA_URL")) {
            alpaca_.data_url = data_url;
        }
        
        if (const char* is_paper = std::getenv("ALPACA_PAPER_TRADING")) {
            alpaca_.is_paper_trading = (std::string(is_paper) == "true");
        }
        
        // Load other configuration
        if (const char* port = std::getenv("SERVER_PORT")) {
            general_.server_port = std::stoi(port);
        }
        
        if (const char* log_level = std::getenv("LOG_LEVEL")) {
            general_.log_level = log_level;
        }
        
        if (const char* debug = std::getenv("DEBUG_MODE")) {
            general_.debug_mode = (std::string(debug) == "true");
        }
    }

    const AlpacaConfig& getAlpacaConfig() const { return alpaca_; }
    const MarketDataConfig& getMarketDataConfig() const { return market_data_; }
    const GeneralConfig& getGeneralConfig() const { return general_; }

    void validateConfiguration() const {
        if (alpaca_.api_key.empty() || alpaca_.api_secret.empty()) {
            throw std::runtime_error("Alpaca API credentials are required. Set ALPACA_API_KEY and ALPACA_API_SECRET environment variables.");
        }
        
        if (alpaca_.data_url.empty()) {
            throw std::runtime_error("Alpaca data URL is required.");
        }
    }

private:
    Configuration() = default;
    
    std::string getEnvVar(const std::string& key) const {
        const char* value = std::getenv(key.c_str());
        if (!value) {
            throw std::runtime_error("Environment variable " + key + " is required but not set.");
        }
        return std::string(value);
    }

    AlpacaConfig alpaca_;
    MarketDataConfig market_data_;
    GeneralConfig general_;
};

} // namespace velocore 