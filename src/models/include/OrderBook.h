#pragma once

#include "Order.h"
#include "Trade.h"
#include <map>
#include <deque>
#include <vector>
#include <mutex>
#include <memory>

namespace velocore {

/**
 * OrderBook - Core matching engine that maintains separate buy and sell books
 * and executes trades based on price-time priority.
 * 
 */
class OrderBook {
private:
    // Buy book: price -> orders (highest price first)
    std::map<double, std::deque<Order>, std::greater<double>> buyBook;
    
    // Sell book: price -> orders (lowest price first)
    std::map<double, std::deque<Order>, std::less<double>> sellBook;
    
    // Trade tracking
    uint64_t nextTradeId;
    std::vector<Trade> tradeLog;
    
    // Thread safety
    mutable std::recursive_mutex bookMutex;
    
    // Internal helper methods
    /**
     * Attempts to match an incoming order against the opposite book
     * @param order The incoming order to match
     * @return Vector of trades generated from this matching attempt
     */
    std::vector<Trade> matchOrder(Order& order);
    
    /**
     * Matches a buy order against the sell book
     * @param buyOrder The incoming buy order
     * @return Vector of trades generated
     */
    std::vector<Trade> matchBuyOrder(Order& buyOrder);
    
    /**
     * Matches a sell order against the buy book
     * @param sellOrder The incoming sell order
     * @return Vector of trades generated
     */
    std::vector<Trade> matchSellOrder(Order& sellOrder);
    
    /**
     * Executes a trade between two orders
     * @param buyOrder The buy order
     * @param sellOrder The sell order
     * @param executionPrice The price at which the trade occurs
     * @param quantity The quantity to trade
     * @return The created Trade object
     */
    Trade executeTrade(Order& buyOrder, Order& sellOrder, double executionPrice, int quantity);
    
    /**
     * Adds an order to the appropriate book (buy or sell)
     * Only called for limit orders that have remaining quantity after matching
     * @param order The order to add to the book
     */
    void addToBook(const Order& order);
    
    /**
     * Removes an order from a price level queue
     * If the queue becomes empty, removes the entire price level
     * @param book Reference to the book (buy or sell)
     * @param price The price level to remove from
     */
    template<typename BookType>
    void removeFromPriceLevel(BookType& book, double price);
    
    /**
     * Checks if prices cross (can execute)
     * @param buyPrice The buy order price
     * @param sellPrice The sell order price
     * @return true if prices cross (buy >= sell)
     */
    bool pricesCross(double buyPrice, double sellPrice) const;

public:
    /**
     * Constructor - initializes empty order book
     */
    OrderBook();
    
    /**
     * Destructor
     */
    ~OrderBook() = default;
    
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;
    
    OrderBook(OrderBook&&) = delete;
    OrderBook& operator=(OrderBook&&) = delete;
    
    /**
     * Adds a new order and executes matches if possible
     * @param order The order to add/match
     * @return Vector of trades generated from this order
     */
    std::vector<Trade> addOrder(Order order);
    
    /**
     * Attempts to cancel an order by ID
     * @param orderId The ID of the order to cancel
     * @return true if order was found and cancelled, false otherwise
     */
    bool cancelOrder(uint64_t orderId);
    
    /**
     * Gets the current best bid price (highest buy price)
     * @return Best bid price, or 0.0 if no bids exist
     */
    double getBestBid() const;
    
    /**
     * Gets the current best ask price (lowest sell price)
     * @return Best ask price, or 0.0 if no asks exist
     */
    double getBestAsk() const;
    
    /**
     * Gets the current bid-ask spread
     * @return Spread (ask - bid), or 0.0 if either side is empty
     */
    double getSpread() const;
    
    /**
     * Gets the top N price levels for both sides
     * @param levels Number of levels to retrieve
     * @return JSON representation of the order book
     */
    crow::json::wvalue getBookSnapshot(size_t levels = 5) const;
    
    /**
     * Gets all trades executed so far
     * @return Reference to the trade log
     */
    const std::vector<Trade>& getTradeLog() const;
    
    /**
     * Gets the number of orders at each price level
     * @return JSON with bid/ask level counts
     */
    crow::json::wvalue getBookStatistics() const;
    
    /**
     * Clears all orders and trades
     */
    void clear();
    
    /**
     * Gets the total number of active orders in the book
     * @return Total order count
     */
    size_t getTotalOrders() const;
    
    /**
     * Checks if the order book is empty
     * @return true if both buy and sell books are empty
     */
    bool isEmpty() const;
};

} // namespace velocore 
