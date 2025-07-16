#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <memory>
#include <sstream>
#include <cstdlib>
#include <nlohmann/json.hpp>

#include "../src/models/include/Types.h"
#include "../src/Config.h"
#include "../src/MarketDataFeed.h"

using namespace velocore;
using json = nlohmann::json;

class MarketDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_time = std::chrono::steady_clock::now();
        
        // Set up environment variables for testing
        setenv("ALPACA_API_KEY", "test_key_123", 1);
        setenv("ALPACA_API_SECRET", "test_secret_456", 1);
        setenv("ALPACA_BASE_URL", "https://paper-api.alpaca.markets", 1);
        setenv("ALPACA_DATA_URL", "wss://stream.data.alpaca.markets/v2/iex", 1);
        setenv("ALPACA_PAPER_TRADING", "true", 1);
    }
    
    void TearDown() override {
        // Clean up environment variables
        unsetenv("ALPACA_API_KEY");
        unsetenv("ALPACA_API_SECRET");
        unsetenv("ALPACA_BASE_URL");
        unsetenv("ALPACA_DATA_URL");
        unsetenv("ALPACA_PAPER_TRADING");
    }
    
    std::chrono::steady_clock::time_point test_time;
};

// =====================================================
// MarketTick Structure Tests
// =====================================================

TEST_F(MarketDataTest, MarketTickConstructorTest) {
    MarketTick tick("AAPL", MarketDataType::Trade);
    
    EXPECT_EQ(tick.symbol, "AAPL");
    EXPECT_EQ(tick.type, MarketDataType::Trade);
    EXPECT_EQ(tick.trade_price, 0.0);
    EXPECT_EQ(tick.trade_size, 0);
    EXPECT_EQ(tick.bid_price, 0.0);
    EXPECT_EQ(tick.ask_price, 0.0);
    EXPECT_EQ(tick.bid_size, 0);
    EXPECT_EQ(tick.ask_size, 0);
    EXPECT_EQ(tick.open, 0.0);
    EXPECT_EQ(tick.high, 0.0);
    EXPECT_EQ(tick.low, 0.0);
    EXPECT_EQ(tick.close, 0.0);
    EXPECT_EQ(tick.volume, 0);
}

TEST_F(MarketDataTest, MarketTickTradeDataTest) {
    MarketTick tick("AAPL", MarketDataType::Trade);
    tick.trade_price = 150.50;
    tick.trade_size = 100;
    tick.timestamp = test_time;
    
    EXPECT_EQ(tick.symbol, "AAPL");
    EXPECT_EQ(tick.type, MarketDataType::Trade);
    EXPECT_DOUBLE_EQ(tick.trade_price, 150.50);
    EXPECT_EQ(tick.trade_size, 100);
    EXPECT_EQ(tick.timestamp, test_time);
}

TEST_F(MarketDataTest, MarketTickQuoteDataTest) {
    MarketTick tick("AAPL", MarketDataType::Quote);
    tick.bid_price = 150.25;
    tick.ask_price = 150.75;
    tick.bid_size = 200;
    tick.ask_size = 150;
    tick.timestamp = test_time;
    
    EXPECT_EQ(tick.symbol, "AAPL");
    EXPECT_EQ(tick.type, MarketDataType::Quote);
    EXPECT_DOUBLE_EQ(tick.bid_price, 150.25);
    EXPECT_DOUBLE_EQ(tick.ask_price, 150.75);
    EXPECT_EQ(tick.bid_size, 200);
    EXPECT_EQ(tick.ask_size, 150);
    EXPECT_EQ(tick.timestamp, test_time);
}

TEST_F(MarketDataTest, MarketTickBarDataTest) {
    MarketTick tick("AAPL", MarketDataType::Bar);
    tick.open = 150.00;
    tick.high = 152.00;
    tick.low = 149.50;
    tick.close = 151.25;
    tick.volume = 10000;
    tick.timestamp = test_time;
    
    EXPECT_EQ(tick.symbol, "AAPL");
    EXPECT_EQ(tick.type, MarketDataType::Bar);
    EXPECT_DOUBLE_EQ(tick.open, 150.00);
    EXPECT_DOUBLE_EQ(tick.high, 152.00);
    EXPECT_DOUBLE_EQ(tick.low, 149.50);
    EXPECT_DOUBLE_EQ(tick.close, 151.25);
    EXPECT_EQ(tick.volume, 10000);
    EXPECT_EQ(tick.timestamp, test_time);
}

// =====================================================
// MarketSubscription Tests
// =====================================================

TEST_F(MarketDataTest, MarketSubscriptionConstructorTest) {
    MarketSubscription sub("AAPL");
    
    EXPECT_EQ(sub.symbol, "AAPL");
    EXPECT_EQ(sub.trades, false);
    EXPECT_EQ(sub.quotes, false);
    EXPECT_EQ(sub.bars, false);
}

TEST_F(MarketDataTest, MarketSubscriptionConfigurationTest) {
    MarketSubscription sub("AAPL");
    sub.trades = true;
    sub.quotes = true;
    sub.bars = false;
    
    EXPECT_EQ(sub.symbol, "AAPL");
    EXPECT_EQ(sub.trades, true);
    EXPECT_EQ(sub.quotes, true);
    EXPECT_EQ(sub.bars, false);
}

// =====================================================
// Enum String Conversion Tests
// =====================================================

TEST_F(MarketDataTest, MarketDataTypeToStringTest) {
    EXPECT_EQ(to_string(MarketDataType::Trade), "Trade");
    EXPECT_EQ(to_string(MarketDataType::Quote), "Quote");
    EXPECT_EQ(to_string(MarketDataType::Bar), "Bar");
}

TEST_F(MarketDataTest, MarketDataTypeFromStringTest) {
    EXPECT_EQ(market_data_type_from_string("Trade"), MarketDataType::Trade);
    EXPECT_EQ(market_data_type_from_string("Quote"), MarketDataType::Quote);
    EXPECT_EQ(market_data_type_from_string("Bar"), MarketDataType::Bar);
}

TEST_F(MarketDataTest, SideStringConversionTest) {
    EXPECT_EQ(to_string(Side::Buy), "Buy");
    EXPECT_EQ(to_string(Side::Sell), "Sell");
    
    EXPECT_EQ(side_from_string("Buy"), Side::Buy);
    EXPECT_EQ(side_from_string("Sell"), Side::Sell);
}

TEST_F(MarketDataTest, OrderTypeStringConversionTest) {
    EXPECT_EQ(to_string(OrderType::Limit), "Limit");
    EXPECT_EQ(to_string(OrderType::Market), "Market");
    
    EXPECT_EQ(order_type_from_string("Limit"), OrderType::Limit);
    EXPECT_EQ(order_type_from_string("Market"), OrderType::Market);
}

TEST_F(MarketDataTest, OrderStatusStringConversionTest) {
    EXPECT_EQ(to_string(OrderStatus::Active), "Active");
    EXPECT_EQ(to_string(OrderStatus::Filled), "Filled");
    EXPECT_EQ(to_string(OrderStatus::Cancelled), "Cancelled");
    EXPECT_EQ(to_string(OrderStatus::PartiallyFilled), "PartiallyFilled");
}

// =====================================================
// Configuration Tests
// =====================================================

TEST_F(MarketDataTest, ConfigurationSingletonTest) {
    Configuration& config1 = Configuration::getInstance();
    Configuration& config2 = Configuration::getInstance();
    
    EXPECT_EQ(&config1, &config2);
}

TEST_F(MarketDataTest, ConfigurationEnvironmentLoadingTest) {
    Configuration& config = Configuration::getInstance();
    config.loadFromEnvironment();
    
    const auto& alpaca_config = config.getAlpacaConfig();
    EXPECT_EQ(alpaca_config.api_key, "test_key_123");
    EXPECT_EQ(alpaca_config.api_secret, "test_secret_456");
    EXPECT_EQ(alpaca_config.base_url, "https://paper-api.alpaca.markets");
    EXPECT_EQ(alpaca_config.data_url, "wss://stream.data.alpaca.markets/v2/iex");
    EXPECT_EQ(alpaca_config.is_paper_trading, true);
}

TEST_F(MarketDataTest, ConfigurationValidationTest) {
    Configuration& config = Configuration::getInstance();
    config.loadFromEnvironment();
    
    // Should not throw with valid configuration
    EXPECT_NO_THROW(config.validateConfiguration());
}

TEST_F(MarketDataTest, ConfigurationMissingCredentialsTest) {
    // Clear environment variables
    unsetenv("ALPACA_API_KEY");
    unsetenv("ALPACA_API_SECRET");
    
    Configuration& config = Configuration::getInstance();
    
    // Should throw when trying to load missing credentials
    EXPECT_THROW(config.loadFromEnvironment(), std::runtime_error);
}

TEST_F(MarketDataTest, ConfigurationMarketDataConfigTest) {
    Configuration& config = Configuration::getInstance();
    const auto& market_config = config.getMarketDataConfig();
    
    EXPECT_EQ(market_config.reconnect_delay_ms, 5000);
    EXPECT_EQ(market_config.max_reconnect_attempts, 10);
    EXPECT_EQ(market_config.heartbeat_interval_ms, 30000);
    EXPECT_EQ(market_config.connection_timeout_ms, 30000);
}

TEST_F(MarketDataTest, ConfigurationGeneralConfigTest) {
    Configuration& config = Configuration::getInstance();
    const auto& general_config = config.getGeneralConfig();
    
    EXPECT_EQ(general_config.server_port, 8080);
    EXPECT_EQ(general_config.log_level, "INFO");
    EXPECT_EQ(general_config.debug_mode, false);
}

// =====================================================
// JSON Serialization Tests
// =====================================================

TEST_F(MarketDataTest, MarketTickJSONSerializationTest) {
    MarketTick tick("AAPL", MarketDataType::Trade);
    tick.trade_price = 150.50;
    tick.trade_size = 100;
    
    crow::json::wvalue json_value = tick.to_json();
    std::string json_str = json_value.dump();
    
    // Parse back to verify
    json parsed = json::parse(json_str);
    EXPECT_EQ(parsed["symbol"], "AAPL");
    EXPECT_EQ(parsed["type"], "Trade");
    EXPECT_DOUBLE_EQ(parsed["trade_price"], 150.50);
    EXPECT_EQ(parsed["trade_size"], 100);
}

TEST_F(MarketDataTest, SideJSONSerializationTest) {
    crow::json::wvalue buy_json = to_json(Side::Buy);
    crow::json::wvalue sell_json = to_json(Side::Sell);
    
    EXPECT_EQ(buy_json.dump(), "\"Buy\"");
    EXPECT_EQ(sell_json.dump(), "\"Sell\"");
}

TEST_F(MarketDataTest, OrderTypeJSONSerializationTest) {
    crow::json::wvalue limit_json = to_json(OrderType::Limit);
    crow::json::wvalue market_json = to_json(OrderType::Market);
    
    EXPECT_EQ(limit_json.dump(), "\"Limit\"");
    EXPECT_EQ(market_json.dump(), "\"Market\"");
}

TEST_F(MarketDataTest, OrderStatusJSONSerializationTest) {
    crow::json::wvalue active_json = to_json(OrderStatus::Active);
    crow::json::wvalue filled_json = to_json(OrderStatus::Filled);
    
    EXPECT_EQ(active_json.dump(), "\"Active\"");
    EXPECT_EQ(filled_json.dump(), "\"Filled\"");
}

TEST_F(MarketDataTest, MarketDataTypeJSONSerializationTest) {
    crow::json::wvalue trade_json = to_json(MarketDataType::Trade);
    crow::json::wvalue quote_json = to_json(MarketDataType::Quote);
    crow::json::wvalue bar_json = to_json(MarketDataType::Bar);
    
    EXPECT_EQ(trade_json.dump(), "\"Trade\"");
    EXPECT_EQ(quote_json.dump(), "\"Quote\"");
    EXPECT_EQ(bar_json.dump(), "\"Bar\"");
}

// =====================================================
// MarketDataFeed Basic Tests
// =====================================================

class MarketDataFeedTest : public MarketDataTest {
protected:
    void SetUp() override {
        MarketDataTest::SetUp();
        // Note: We can't easily test the full WebSocket functionality without a mock
        // But we can test the basic interface and callback mechanisms
    }
};

TEST_F(MarketDataFeedTest, MarketDataFeedConstructorTest) {
    Configuration& config = Configuration::getInstance();
    config.loadFromEnvironment();
    
    EXPECT_NO_THROW({
        MarketDataFeed feed;
    });
}

TEST_F(MarketDataFeedTest, MarketDataFeedCallbackRegistrationTest) {
    Configuration& config = Configuration::getInstance();
    config.loadFromEnvironment();
    
    MarketDataFeed feed;
    
    bool tick_callback_called = false;
    bool connection_callback_called = false;
    bool error_callback_called = false;
    
    feed.onTick([&tick_callback_called](const MarketTick& tick) {
        tick_callback_called = true;
    });
    
    feed.onConnection([&connection_callback_called](bool connected) {
        connection_callback_called = true;
    });
    
    feed.onError([&error_callback_called](const std::string& error) {
        error_callback_called = true;
    });
    
    // Test broadcasting (this will call the callbacks)
    MarketTick test_tick("AAPL", MarketDataType::Trade);
    feed.broadcastBookUpdate("AAPL", test_tick);
    
    EXPECT_TRUE(tick_callback_called);
}

TEST_F(MarketDataFeedTest, MarketDataFeedInitialStateTest) {
    Configuration& config = Configuration::getInstance();
    config.loadFromEnvironment();
    
    MarketDataFeed feed;
    
    // Initially not connected
    EXPECT_FALSE(feed.isConnected());
    
    // Initially no subscriptions
    auto symbols = feed.getSubscribedSymbols();
    EXPECT_TRUE(symbols.empty());
}

TEST_F(MarketDataFeedTest, MarketDataFeedSubscriptionTrackingTest) {
    Configuration& config = Configuration::getInstance();
    config.loadFromEnvironment();
    
    MarketDataFeed feed;
    
    // Test subscription tracking (without actual WebSocket connection)
    // This tests the internal subscription management
    feed.subscribe("AAPL", true, true, false);
    feed.subscribe("GOOGL", true, false, true);
    
    auto symbols = feed.getSubscribedSymbols();
    EXPECT_EQ(symbols.size(), 2);
    
    // Check that both symbols are in the list
    bool found_aapl = false, found_googl = false;
    for (const auto& symbol : symbols) {
        if (symbol == "AAPL") found_aapl = true;
        if (symbol == "GOOGL") found_googl = true;
    }
    EXPECT_TRUE(found_aapl);
    EXPECT_TRUE(found_googl);
}

// =====================================================
// Integration Tests
// =====================================================

TEST_F(MarketDataTest, MarketDataIntegrationTest) {
    // Test the complete flow from configuration to market data structures
    Configuration& config = Configuration::getInstance();
    config.loadFromEnvironment();
    
    // Create a market tick
    MarketTick tick("AAPL", MarketDataType::Trade);
    tick.trade_price = 150.50;
    tick.trade_size = 100;
    
    // Serialize to JSON
    crow::json::wvalue json_value = tick.to_json();
    std::string json_str = json_value.dump();
    
    // Parse back
    json parsed = json::parse(json_str);
    
    // Verify the round trip
    EXPECT_EQ(parsed["symbol"], "AAPL");
    EXPECT_EQ(parsed["type"], "Trade");
    EXPECT_DOUBLE_EQ(parsed["trade_price"], 150.50);
    EXPECT_EQ(parsed["trade_size"], 100);
}

TEST_F(MarketDataTest, MarketDataPerformanceTest) {
    const int NUM_TICKS = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_TICKS; ++i) {
        MarketTick tick("AAPL", MarketDataType::Trade);
        tick.trade_price = 150.0 + (i * 0.01);
        tick.trade_size = 100 + i;
        
        crow::json::wvalue json_value = tick.to_json();
        std::string json_str = json_value.dump();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should be able to process 1000 ticks in less than 100ms
    EXPECT_LT(duration.count(), 100000);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 