#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <memory>
#include <sstream>
#include <cstdlib>

#include "../src/models/include/Types.h"
#include "../src/Config.h"
#include "../src/MarketDataFeed.h"

using namespace velocore;
using json = nlohmann::json;

class WebSocketParsingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up environment variables for testing
        setenv("ALPACA_API_KEY", "test_key_123", 1);
        setenv("ALPACA_API_SECRET", "test_secret_456", 1);
        setenv("ALPACA_BASE_URL", "https://paper-api.alpaca.markets", 1);
        setenv("ALPACA_DATA_URL", "wss://stream.data.alpaca.markets/v2/iex", 1);
        setenv("ALPACA_PAPER_TRADING", "true", 1);
        
        Configuration& config = Configuration::getInstance();
        config.loadFromEnvironment();
    }
    
    void TearDown() override {
        // Clean up environment variables
        unsetenv("ALPACA_API_KEY");
        unsetenv("ALPACA_API_SECRET");
        unsetenv("ALPACA_BASE_URL");
        unsetenv("ALPACA_DATA_URL");
        unsetenv("ALPACA_PAPER_TRADING");
    }
    
    // Helper to create realistic Alpaca WebSocket messages
    std::string createAlpacaTradeMessage(const std::string& symbol, double price, int size) {
        json message;
        message["T"] = "t";  // Trade message type
        message["S"] = symbol;
        message["p"] = price;
        message["s"] = size;
        message["t"] = "2023-01-01T10:00:00Z";
        message["c"] = json::array({"@", "T"});
        message["i"] = 12345;
        message["x"] = "V";
        message["z"] = "C";
        
        return message.dump();
    }
    
    std::string createAlpacaQuoteMessage(const std::string& symbol, double bid, double ask, int bid_size, int ask_size) {
        json message;
        message["T"] = "q";  // Quote message type
        message["S"] = symbol;
        message["bp"] = bid;
        message["ap"] = ask;
        message["bs"] = bid_size;
        message["as"] = ask_size;
        message["t"] = "2023-01-01T10:00:00Z";
        message["c"] = json::array({"R"});
        message["bx"] = "V";
        message["ax"] = "V";
        
        return message.dump();
    }
    
    std::string createAlpacaBarMessage(const std::string& symbol, double open, double high, double low, double close, int volume) {
        json message;
        message["T"] = "b";  // Bar message type
        message["S"] = symbol;
        message["o"] = open;
        message["h"] = high;
        message["l"] = low;
        message["c"] = close;
        message["v"] = volume;
        message["t"] = "2023-01-01T10:00:00Z";
        message["n"] = 100;
        message["vw"] = close;
        
        return message.dump();
    }
    
    std::string createAlpacaAuthSuccessMessage() {
        json message;
        message["T"] = "success";
        message["msg"] = "authenticated";
        return message.dump();
    }
    
    std::string createAlpacaSubscriptionMessage() {
        json message;
        message["T"] = "subscription";
        message["trades"] = json::array({"AAPL", "GOOGL"});
        message["quotes"] = json::array({"AAPL"});
        message["bars"] = json::array({"GOOGL"});
        return message.dump();
    }
    
    std::string createAlpacaErrorMessage(const std::string& error_msg) {
        json message;
        message["T"] = "error";
        message["code"] = 400;
        message["msg"] = error_msg;
        return message.dump();
    }
};

// =====================================================
// Message Parsing Tests
// =====================================================

TEST_F(WebSocketParsingTest, ParseTradeMessageTest) {
    std::string trade_msg = createAlpacaTradeMessage("AAPL", 150.50, 100);
    
    // Test that the message parses correctly
    json parsed = json::parse(trade_msg);
    EXPECT_EQ(parsed["T"], "t");
    EXPECT_EQ(parsed["S"], "AAPL");
    EXPECT_DOUBLE_EQ(parsed["p"], 150.50);
    EXPECT_EQ(parsed["s"], 100);
}

TEST_F(WebSocketParsingTest, ParseQuoteMessageTest) {
    std::string quote_msg = createAlpacaQuoteMessage("AAPL", 150.25, 150.75, 200, 150);
    
    json parsed = json::parse(quote_msg);
    EXPECT_EQ(parsed["T"], "q");
    EXPECT_EQ(parsed["S"], "AAPL");
    EXPECT_DOUBLE_EQ(parsed["bp"], 150.25);
    EXPECT_DOUBLE_EQ(parsed["ap"], 150.75);
    EXPECT_EQ(parsed["bs"], 200);
    EXPECT_EQ(parsed["as"], 150);
}

TEST_F(WebSocketParsingTest, ParseBarMessageTest) {
    std::string bar_msg = createAlpacaBarMessage("AAPL", 150.0, 152.0, 149.5, 151.25, 10000);
    
    json parsed = json::parse(bar_msg);
    EXPECT_EQ(parsed["T"], "b");
    EXPECT_EQ(parsed["S"], "AAPL");
    EXPECT_DOUBLE_EQ(parsed["o"], 150.0);
    EXPECT_DOUBLE_EQ(parsed["h"], 152.0);
    EXPECT_DOUBLE_EQ(parsed["l"], 149.5);
    EXPECT_DOUBLE_EQ(parsed["c"], 151.25);
    EXPECT_EQ(parsed["v"], 10000);
}

TEST_F(WebSocketParsingTest, ParseAuthSuccessMessageTest) {
    std::string auth_msg = createAlpacaAuthSuccessMessage();
    
    json parsed = json::parse(auth_msg);
    EXPECT_EQ(parsed["T"], "success");
    EXPECT_EQ(parsed["msg"], "authenticated");
}

TEST_F(WebSocketParsingTest, ParseSubscriptionMessageTest) {
    std::string sub_msg = createAlpacaSubscriptionMessage();
    
    json parsed = json::parse(sub_msg);
    EXPECT_EQ(parsed["T"], "subscription");
    EXPECT_TRUE(parsed["trades"].is_array());
    EXPECT_TRUE(parsed["quotes"].is_array());
    EXPECT_TRUE(parsed["bars"].is_array());
}

TEST_F(WebSocketParsingTest, ParseErrorMessageTest) {
    std::string error_msg = createAlpacaErrorMessage("Invalid symbol");
    
    json parsed = json::parse(error_msg);
    EXPECT_EQ(parsed["T"], "error");
    EXPECT_EQ(parsed["code"], 400);
    EXPECT_EQ(parsed["msg"], "Invalid symbol");
}

// =====================================================
// MarketDataFeed Message Processing Tests
// =====================================================

class MarketDataFeedMessageTest : public WebSocketParsingTest {
protected:
    void SetUp() override {
        WebSocketParsingTest::SetUp();
        feed = std::make_unique<MarketDataFeed>();
        
        // Set up callback tracking
        tick_callbacks.clear();
        connection_callbacks.clear();
        error_callbacks.clear();
        
        feed->onTick([this](const MarketTick& tick) {
            tick_callbacks.push_back(tick);
        });
        
        feed->onConnection([this](bool connected) {
            connection_callbacks.push_back(connected);
        });
        
        feed->onError([this](const std::string& error) {
            error_callbacks.push_back(error);
        });
    }
    
    std::unique_ptr<MarketDataFeed> feed;
    std::vector<MarketTick> tick_callbacks;
    std::vector<bool> connection_callbacks;
    std::vector<std::string> error_callbacks;
};

TEST_F(MarketDataFeedMessageTest, CallbackRegistrationTest) {
    // Test that callbacks are properly registered
    EXPECT_EQ(tick_callbacks.size(), 0);
    EXPECT_EQ(connection_callbacks.size(), 0);
    EXPECT_EQ(error_callbacks.size(), 0);
    
    // Test broadcasting
    MarketTick test_tick("AAPL", MarketDataType::Trade);
    test_tick.trade_price = 150.50;
    test_tick.trade_size = 100;
    
    feed->broadcastBookUpdate("AAPL", test_tick);
    
    EXPECT_EQ(tick_callbacks.size(), 1);
    EXPECT_EQ(tick_callbacks[0].symbol, "AAPL");
    EXPECT_EQ(tick_callbacks[0].type, MarketDataType::Trade);
    EXPECT_DOUBLE_EQ(tick_callbacks[0].trade_price, 150.50);
    EXPECT_EQ(tick_callbacks[0].trade_size, 100);
}

TEST_F(MarketDataFeedMessageTest, MultipleTicksTest) {
    // Test multiple ticks
    MarketTick tick1("AAPL", MarketDataType::Trade);
    tick1.trade_price = 150.50;
    tick1.trade_size = 100;
    
    MarketTick tick2("GOOGL", MarketDataType::Quote);
    tick2.bid_price = 2800.25;
    tick2.ask_price = 2800.75;
    tick2.bid_size = 50;
    tick2.ask_size = 75;
    
    feed->broadcastBookUpdate("AAPL", tick1);
    feed->broadcastBookUpdate("GOOGL", tick2);
    
    EXPECT_EQ(tick_callbacks.size(), 2);
    
    // Check first tick
    EXPECT_EQ(tick_callbacks[0].symbol, "AAPL");
    EXPECT_EQ(tick_callbacks[0].type, MarketDataType::Trade);
    EXPECT_DOUBLE_EQ(tick_callbacks[0].trade_price, 150.50);
    
    // Check second tick
    EXPECT_EQ(tick_callbacks[1].symbol, "GOOGL");
    EXPECT_EQ(tick_callbacks[1].type, MarketDataType::Quote);
    EXPECT_DOUBLE_EQ(tick_callbacks[1].bid_price, 2800.25);
    EXPECT_DOUBLE_EQ(tick_callbacks[1].ask_price, 2800.75);
}

TEST_F(MarketDataFeedMessageTest, SubscriptionManagementTest) {
    // Test subscription management
    EXPECT_TRUE(feed->getSubscribedSymbols().empty());
    
    feed->subscribe("AAPL", true, true, false);
    auto symbols = feed->getSubscribedSymbols();
    EXPECT_EQ(symbols.size(), 1);
    EXPECT_EQ(symbols[0], "AAPL");
    
    feed->subscribe("GOOGL", true, false, true);
    symbols = feed->getSubscribedSymbols();
    EXPECT_EQ(symbols.size(), 2);
    
    feed->unsubscribe("AAPL");
    symbols = feed->getSubscribedSymbols();
    EXPECT_EQ(symbols.size(), 1);
    EXPECT_EQ(symbols[0], "GOOGL");
}

TEST_F(MarketDataFeedMessageTest, ConnectionStatusTest) {
    // Test initial connection status
    EXPECT_FALSE(feed->isConnected());
    
    // Note: Without actual WebSocket connection, 
    // we can't test the full connection flow
}

// =====================================================
// Error Handling Tests
// =====================================================

TEST_F(WebSocketParsingTest, InvalidJSONHandlingTest) {
    std::string invalid_json = "{ invalid json }";
    
    // Test that invalid JSON doesn't crash the parser
    EXPECT_THROW(json::parse(invalid_json), json::parse_error);
}

TEST_F(WebSocketParsingTest, MissingFieldsTest) {
    // Test message with missing required fields
    json incomplete_trade;
    incomplete_trade["T"] = "t";
    incomplete_trade["S"] = "AAPL";
    // Missing price and size fields
    
    std::string msg = incomplete_trade.dump();
    json parsed = json::parse(msg);
    
    EXPECT_EQ(parsed["T"], "t");
    EXPECT_EQ(parsed["S"], "AAPL");
    EXPECT_TRUE(parsed["p"].is_null());
    EXPECT_TRUE(parsed["s"].is_null());
}

TEST_F(WebSocketParsingTest, UnknownMessageTypeTest) {
    json unknown_message;
    unknown_message["T"] = "unknown";
    unknown_message["data"] = "test";
    
    std::string msg = unknown_message.dump();
    json parsed = json::parse(msg);
    
    EXPECT_EQ(parsed["T"], "unknown");
    EXPECT_EQ(parsed["data"], "test");
}

// =====================================================
// Performance Tests
// =====================================================

TEST_F(WebSocketParsingTest, MessageParsingPerformanceTest) {
    const int NUM_MESSAGES = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_MESSAGES; ++i) {
        std::string trade_msg = createAlpacaTradeMessage("AAPL", 150.0 + (i * 0.01), 100 + i);
        json parsed = json::parse(trade_msg);
        
        // Simulate basic field access
        std::string symbol = parsed["S"];
        double price = parsed["p"];
        int size = parsed["s"];
        
        // Prevent optimization
        (void)symbol;
        (void)price;
        (void)size;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should be able to parse 1000 messages in less than 50ms
    EXPECT_LT(duration.count(), 50000);
}

TEST_F(MarketDataFeedMessageTest, CallbackPerformanceTest) {
    const int NUM_TICKS = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_TICKS; ++i) {
        MarketTick tick("AAPL", MarketDataType::Trade);
        tick.trade_price = 150.0 + (i * 0.01);
        tick.trade_size = 100 + i;
        
        feed->broadcastBookUpdate("AAPL", tick);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should be able to process 1000 callbacks in less than 10ms
    EXPECT_LT(duration.count(), 10000);
    EXPECT_EQ(tick_callbacks.size(), NUM_TICKS);
}

// =====================================================
// Integration Tests
// =====================================================

TEST_F(WebSocketParsingTest, AlpacaMessageRoundTripTest) {
    // Test the complete flow from Alpaca message to MarketTick
    std::string trade_msg = createAlpacaTradeMessage("AAPL", 150.50, 100);
    json parsed = json::parse(trade_msg);
    
    // Create MarketTick from parsed data
    MarketTick tick("AAPL", MarketDataType::Trade);
    tick.trade_price = parsed["p"];
    tick.trade_size = parsed["s"];
    
    // Verify the data
    EXPECT_EQ(tick.symbol, "AAPL");
    EXPECT_EQ(tick.type, MarketDataType::Trade);
    EXPECT_DOUBLE_EQ(tick.trade_price, 150.50);
    EXPECT_EQ(tick.trade_size, 100);
    
    // Test JSON serialization
    crow::json::wvalue json_value = tick.to_json();
    std::string json_str = json_value.dump();
    json output = json::parse(json_str);
    
    EXPECT_EQ(output["symbol"], "AAPL");
    EXPECT_EQ(output["type"], "Trade");
    EXPECT_DOUBLE_EQ(output["trade_price"], 150.50);
    EXPECT_EQ(output["trade_size"], 100);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 