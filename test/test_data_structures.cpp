#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "../src/models/include/Order.h"
#include "../src/models/include/Trade.h"
#include "../src/models/include/OrderBook.h"
#include "../src/models/include/Types.h"

using namespace velocore;
class DataModelsTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_time = std::chrono::steady_clock::now();
    }
    
    std::chrono::steady_clock::time_point test_time;
};

TEST_F(DataModelsTest, OrderStructureTest) {
    Order order;
    order.id = 12345;
    order.symbol = "TEST";
    order.side = Side::Buy;
    order.type = OrderType::Limit;
    order.price = 100.50;
    order.quantity = 100;
    order.timestamp = test_time;
    
    EXPECT_EQ(order.id, 12345);
    EXPECT_EQ(order.symbol, "TEST");
    EXPECT_EQ(order.side, Side::Buy);
    EXPECT_EQ(order.type, OrderType::Limit);
    EXPECT_DOUBLE_EQ(order.price, 100.50);
    EXPECT_EQ(order.quantity, 100);
    EXPECT_EQ(order.timestamp, test_time);
}

TEST_F(DataModelsTest, SideEnumTest) {
    Order buyOrder;
    buyOrder.side = Side::Buy;
    
    Order sellOrder;
    sellOrder.side = Side::Sell;
    
    EXPECT_EQ(buyOrder.side, Side::Buy);
    EXPECT_EQ(sellOrder.side, Side::Sell);
    EXPECT_NE(buyOrder.side, sellOrder.side);
}

TEST_F(DataModelsTest, OrderTypeEnumTest) {
    Order limitOrder;
    limitOrder.type = OrderType::Limit;
    
    Order marketOrder;
    marketOrder.type = OrderType::Market;
    
    EXPECT_EQ(limitOrder.type, OrderType::Limit);
    EXPECT_EQ(marketOrder.type, OrderType::Market);
    EXPECT_NE(limitOrder.type, marketOrder.type);
}

TEST_F(DataModelsTest, TradeStructureTest) {
    Trade trade;
    trade.trade_id = 67890;
    trade.buy_order_id = 12345;
    trade.sell_order_id = 54321;
    trade.symbol = "TEST";
    trade.price = 100.75;
    trade.quantity = 50;
    trade.timestamp = test_time;
    
    EXPECT_EQ(trade.trade_id, 67890);
    EXPECT_EQ(trade.buy_order_id, 12345);
    EXPECT_EQ(trade.sell_order_id, 54321);
    EXPECT_EQ(trade.symbol, "TEST");
    EXPECT_DOUBLE_EQ(trade.price, 100.75);
    EXPECT_EQ(trade.quantity, 50);
    EXPECT_EQ(trade.timestamp, test_time);
}

class MatchingEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        orderBook = std::make_unique<OrderBook>();
        nextOrderId = 1;
    }
    
    Order createOrder(Side side, OrderType type, double price, int quantity) {
        Order order;
        order.id = nextOrderId++;
        order.symbol = "TEST";
        order.side = side;
        order.type = type;
        order.price = price;
        order.quantity = quantity;
        order.remaining_quantity = quantity;
        order.status = OrderStatus::Active;
        order.timestamp = std::chrono::steady_clock::now();
        return order;
    }
    
    std::unique_ptr<OrderBook> orderBook;
    uint64_t nextOrderId;
};

TEST_F(MatchingEngineTest, OrderBookStructureTest) {
    EXPECT_NE(orderBook, nullptr);
    
    Order buyOrder = createOrder(Side::Buy, OrderType::Limit, 100.0, 50);
    Order sellOrder = createOrder(Side::Sell, OrderType::Limit, 101.0, 50);
    
    std::vector<Trade> buyTrades = orderBook->addOrder(buyOrder);
    std::vector<Trade> sellTrades = orderBook->addOrder(sellOrder);
    
    EXPECT_EQ(buyTrades.size(), 0);
    EXPECT_EQ(sellTrades.size(), 0);
}

TEST_F(MatchingEngineTest, PriceTimePriorityTest) {
    Order firstOrder = createOrder(Side::Buy, OrderType::Limit, 100.0, 30);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    Order secondOrder = createOrder(Side::Buy, OrderType::Limit, 100.0, 40);
    
    orderBook->addOrder(firstOrder);
    orderBook->addOrder(secondOrder);
    
    Order sellOrder = createOrder(Side::Sell, OrderType::Limit, 100.0, 30);
    std::vector<Trade> trades = orderBook->addOrder(sellOrder);
    
    EXPECT_EQ(trades.size(), 1);
    
    EXPECT_EQ(trades[0].buy_order_id, firstOrder.id);
    EXPECT_EQ(trades[0].sell_order_id, sellOrder.id);
    EXPECT_EQ(trades[0].quantity, 30);
    EXPECT_DOUBLE_EQ(trades[0].price, 100.0);
}

TEST_F(MatchingEngineTest, PricePriorityTest) {
    Order lowerPriceBuy = createOrder(Side::Buy, OrderType::Limit, 99.0, 50);
    Order higherPriceBuy = createOrder(Side::Buy, OrderType::Limit, 101.0, 50);
    
    orderBook->addOrder(lowerPriceBuy);
    orderBook->addOrder(higherPriceBuy);
    
    Order sellOrder = createOrder(Side::Sell, OrderType::Limit, 99.0, 50);
    std::vector<Trade> trades = orderBook->addOrder(sellOrder);
    
    EXPECT_EQ(trades.size(), 1);
    
    EXPECT_EQ(trades[0].buy_order_id, higherPriceBuy.id);
    EXPECT_EQ(trades[0].sell_order_id, sellOrder.id);
    EXPECT_EQ(trades[0].quantity, 50);
    EXPECT_DOUBLE_EQ(trades[0].price, 101.0);
}

TEST_F(MatchingEngineTest, PartialFillTest) {
    Order buyOrder = createOrder(Side::Buy, OrderType::Limit, 100.0, 100);
    orderBook->addOrder(buyOrder);
    
    Order sellOrder = createOrder(Side::Sell, OrderType::Limit, 100.0, 40);
    std::vector<Trade> trades = orderBook->addOrder(sellOrder);
    
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 40);
    EXPECT_DOUBLE_EQ(trades[0].price, 100.0);
}

TEST_F(MatchingEngineTest, MarketOrderTest) {
    Order sellOrder = createOrder(Side::Sell, OrderType::Limit, 105.0, 50);
    orderBook->addOrder(sellOrder);
    
    Order marketBuyOrder = createOrder(Side::Buy, OrderType::Market, 0.0, 50);
    std::vector<Trade> trades = orderBook->addOrder(marketBuyOrder);
    
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 50);
    EXPECT_DOUBLE_EQ(trades[0].price, 105.0);
}

TEST_F(MatchingEngineTest, MultipleMatchesTest) {
    Order buy1 = createOrder(Side::Buy, OrderType::Limit, 100.0, 20);
    Order buy2 = createOrder(Side::Buy, OrderType::Limit, 100.0, 30);
    Order buy3 = createOrder(Side::Buy, OrderType::Limit, 100.0, 25);
    
    orderBook->addOrder(buy1);
    orderBook->addOrder(buy2);
    orderBook->addOrder(buy3);
    
    Order sellOrder = createOrder(Side::Sell, OrderType::Limit, 100.0, 75);
    std::vector<Trade> trades = orderBook->addOrder(sellOrder);
    
    EXPECT_EQ(trades.size(), 3);
    
    EXPECT_EQ(trades[0].buy_order_id, buy1.id);
    EXPECT_EQ(trades[0].quantity, 20);
    
    EXPECT_EQ(trades[1].buy_order_id, buy2.id);
    EXPECT_EQ(trades[1].quantity, 30);
    
    EXPECT_EQ(trades[2].buy_order_id, buy3.id);
    EXPECT_EQ(trades[2].quantity, 25);
}

TEST_F(MatchingEngineTest, trade_idGenerationTest) {
    Order buy1 = createOrder(Side::Buy, OrderType::Limit, 100.0, 10);
    Order buy2 = createOrder(Side::Buy, OrderType::Limit, 100.0, 10);
    
    orderBook->addOrder(buy1);
    orderBook->addOrder(buy2);
    
    Order sell1 = createOrder(Side::Sell, OrderType::Limit, 100.0, 10);
    Order sell2 = createOrder(Side::Sell, OrderType::Limit, 100.0, 10);
    
    std::vector<Trade> trades1 = orderBook->addOrder(sell1);
    std::vector<Trade> trades2 = orderBook->addOrder(sell2);
    
    EXPECT_EQ(trades1.size(), 1);
    EXPECT_EQ(trades2.size(), 1);
    EXPECT_NE(trades1[0].trade_id, trades2[0].trade_id);
    
    EXPECT_EQ(trades2[0].trade_id, trades1[0].trade_id + 1);
}

TEST_F(MatchingEngineTest, OrderBookLevelsTest) {
    orderBook->addOrder(createOrder(Side::Buy, OrderType::Limit, 100.0, 10));
    orderBook->addOrder(createOrder(Side::Buy, OrderType::Limit, 99.0, 20));
    orderBook->addOrder(createOrder(Side::Buy, OrderType::Limit, 98.0, 30));
    
    orderBook->addOrder(createOrder(Side::Sell, OrderType::Limit, 101.0, 15));
    orderBook->addOrder(createOrder(Side::Sell, OrderType::Limit, 102.0, 25));
}

TEST_F(MatchingEngineTest, QuantityConservationTest) {
    Order buyOrder = createOrder(Side::Buy, OrderType::Limit, 100.0, 75);
    orderBook->addOrder(buyOrder);
    
    Order sellOrder = createOrder(Side::Sell, OrderType::Limit, 100.0, 75);
    std::vector<Trade> trades = orderBook->addOrder(sellOrder);
    
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 75);
    
    int totalTradeQuantity = 0;
    for (const auto& trade : trades) {
        totalTradeQuantity += trade.quantity;
    }
    EXPECT_EQ(totalTradeQuantity, 75);
}

TEST_F(MatchingEngineTest, PerformanceTest) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        Order order = createOrder(Side::Buy, OrderType::Limit, 100.0 + (i % 10), 10);
        orderBook->addOrder(order);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    EXPECT_LT(duration.count(), 100000);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}