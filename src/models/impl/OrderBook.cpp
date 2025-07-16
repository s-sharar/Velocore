#include "OrderBook.h"
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace velocore {

OrderBook::OrderBook() : nextTradeId(1) {}

std::vector<Trade> OrderBook::addOrder(Order order) {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    
    // Set order timestamp if not already set
    if (order.timestamp == std::chrono::steady_clock::time_point{}) {
        order.timestamp = std::chrono::steady_clock::now();
    }
    
    // Attempt to match the order
    std::vector<Trade> trades = matchOrder(order);
    
    // If it's a limit order and has remaining quantity, add it to the book
    if (order.is_limit() && order.remaining_quantity > 0) {
        addToBook(order);
    }
    
    return trades;
}

std::vector<Trade> OrderBook::matchOrder(Order& order) {
    if (order.is_buy()) {
        return matchBuyOrder(order);
    } else {
        return matchSellOrder(order);
    }
}

std::vector<Trade> OrderBook::matchBuyOrder(Order& buyOrder) {
    std::vector<Trade> trades;
    
    // Match against sell book
    while (buyOrder.remaining_quantity > 0 && !sellBook.empty()) {
        auto& [askPrice, askQueue] = *sellBook.begin();
        
        // Check if prices cross
        if (buyOrder.is_market() || pricesCross(buyOrder.price, askPrice)) {
            // Get the oldest order at this price level
            Order& sellOrder = askQueue.front();
            
            double executionPrice = askPrice;
            
            // Determine execution quantity
            int executeQty = std::min(buyOrder.remaining_quantity, sellOrder.remaining_quantity);
            
            // Execute the trade
            Trade trade = executeTrade(buyOrder, sellOrder, executionPrice, executeQty);
            trades.push_back(trade);
            tradeLog.push_back(trade);
            
            // Update order quantities
            buyOrder.remaining_quantity -= executeQty;
            sellOrder.remaining_quantity -= executeQty;
            
            // Update order statuses
            if (buyOrder.remaining_quantity == 0) {
                buyOrder.status = OrderStatus::Filled;
            } else if (buyOrder.remaining_quantity < buyOrder.quantity) {
                buyOrder.status = OrderStatus::PartiallyFilled;
            }
            
            if (sellOrder.remaining_quantity == 0) {
                sellOrder.status = OrderStatus::Filled;
                // Remove completely filled order from the book
                askQueue.pop_front();
                // If price level is now empty, remove it entirely
                if (askQueue.empty()) {
                    sellBook.erase(sellBook.begin());
                }
            } else {
                sellOrder.status = OrderStatus::PartiallyFilled;
            }
        } else {
            // Prices don't cross, no more matches possible
            break;
        }
    }
    
    return trades;
}

std::vector<Trade> OrderBook::matchSellOrder(Order& sellOrder) {
    std::vector<Trade> trades;
    
    // Match against buy book
    while (sellOrder.remaining_quantity > 0 && !buyBook.empty()) {
        auto& [bidPrice, bidQueue] = *buyBook.begin();
        
        // Check if prices cross
        if (sellOrder.is_market() || pricesCross(bidPrice, sellOrder.price)) {
            // Get the oldest order at this level
            Order& buyOrder = bidQueue.front();
            
            // Determine execution price
            double executionPrice = bidPrice;
            
            // Determine execution quantity
            int executeQty = std::min(sellOrder.remaining_quantity, buyOrder.remaining_quantity);
            
            // Execute the trade
            Trade trade = executeTrade(buyOrder, sellOrder, executionPrice, executeQty);
            trades.push_back(trade);
            tradeLog.push_back(trade);
            
            // Update order quantities
            sellOrder.remaining_quantity -= executeQty;
            buyOrder.remaining_quantity -= executeQty;
            
            // Update order statuses
            if (sellOrder.remaining_quantity == 0) {
                sellOrder.status = OrderStatus::Filled;
            } else if (sellOrder.remaining_quantity < sellOrder.quantity) {
                sellOrder.status = OrderStatus::PartiallyFilled;
            }
            
            if (buyOrder.remaining_quantity == 0) {
                buyOrder.status = OrderStatus::Filled;
                // Remove completely filled order from the book
                bidQueue.pop_front();
                // If price level is now empty, remove it entirely
                if (bidQueue.empty()) {
                    buyBook.erase(buyBook.begin());
                }
            } else {
                buyOrder.status = OrderStatus::PartiallyFilled;
            }
        } else {
            // Prices don't cross, no more matches possible
            break;
        }
    }
    
    return trades;
}

Trade OrderBook::executeTrade(Order& buyOrder, Order& sellOrder, double executionPrice, int quantity) {
    return Trade(
        buyOrder.id,
        sellOrder.id,
        buyOrder.symbol,  // Assuming both orders have the same symbol
        executionPrice,
        quantity
    );
}

void OrderBook::addToBook(const Order& order) {
    if (order.is_buy()) {
        buyBook[order.price].push_back(order);
    } else {
        sellBook[order.price].push_back(order);
    }
}

template<typename BookType>
void OrderBook::removeFromPriceLevel(BookType& book, double price) {
    auto it = book.find(price);
    if (it != book.end()) {
        it->second.pop_front();
        if (it->second.empty()) {
            book.erase(it);
        }
    }
}

bool OrderBook::pricesCross(double buyPrice, double sellPrice) const {
    return buyPrice >= sellPrice;
}

bool OrderBook::cancelOrder(uint64_t orderId) {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    
    // Search in buy book
    for (auto& [price, orders] : buyBook) {
        for (auto it = orders.begin(); it != orders.end(); ++it) {
            if (it->id == orderId) {
                it->cancel();
                orders.erase(it);
                if (orders.empty()) {
                    buyBook.erase(price);
                }
                return true;
            }
        }
    }
    
    // Search in sell book
    for (auto& [price, orders] : sellBook) {
        for (auto it = orders.begin(); it != orders.end(); ++it) {
            if (it->id == orderId) {
                it->cancel();
                orders.erase(it);
                if (orders.empty()) {
                    sellBook.erase(price);
                }
                return true;
            }
        }
    }
    
    return false;
}

double OrderBook::getBestBid() const {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    return buyBook.empty() ? 0.0 : buyBook.begin()->first;
}

double OrderBook::getBestAsk() const {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    return sellBook.empty() ? 0.0 : sellBook.begin()->first;
}

double OrderBook::getSpread() const {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    double bestBid = getBestBid();
    double bestAsk = getBestAsk();
    
    if (bestBid == 0.0 || bestAsk == 0.0) {
        return 0.0;
    }
    
    return bestAsk - bestBid;
}

crow::json::wvalue OrderBook::getBookSnapshot(size_t levels) const {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    
    crow::json::wvalue result;
    crow::json::wvalue::list bids, asks;
    
    // Get bid levels
    size_t bidCount = 0;
    for (const auto& [price, orders] : buyBook) {
        if (bidCount >= levels) break;
        
        int totalQty = 0;
        for (const auto& order : orders) {
            totalQty += order.remaining_quantity;
        }
        
        bids.push_back(crow::json::wvalue{
            {"price", price},
            {"quantity", totalQty},
            {"orders", static_cast<int>(orders.size())}
        });
        
        bidCount++;
    }
    
    // Get ask levels
    size_t askCount = 0;
    for (const auto& [price, orders] : sellBook) {
        if (askCount >= levels) break;
        
        int totalQty = 0;
        for (const auto& order : orders) {
            totalQty += order.remaining_quantity;
        }
        
        asks.push_back(crow::json::wvalue{
            {"price", price},
            {"quantity", totalQty},
            {"orders", static_cast<int>(orders.size())}
        });
        
        askCount++;
    }
    
    result["bids"] = std::move(bids);
    result["asks"] = std::move(asks);
    result["spread"] = getSpread();
    result["best_bid"] = getBestBid();
    result["best_ask"] = getBestAsk();
    
    return result;
}

const std::vector<Trade>& OrderBook::getTradeLog() const {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    return tradeLog;
}

crow::json::wvalue OrderBook::getBookStatistics() const {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    
    int totalBidLevels = buyBook.size();
    int totalAskLevels = sellBook.size();
    int totalBidOrders = 0;
    int totalAskOrders = 0;
    
    for (const auto& [price, orders] : buyBook) {
        totalBidOrders += orders.size();
    }
    
    for (const auto& [price, orders] : sellBook) {
        totalAskOrders += orders.size();
    }
    
    return crow::json::wvalue{
        {"bid_levels", totalBidLevels},
        {"ask_levels", totalAskLevels},
        {"bid_orders", totalBidOrders},
        {"ask_orders", totalAskOrders},
        {"total_orders", totalBidOrders + totalAskOrders},
        {"total_trades", static_cast<int>(tradeLog.size())}
    };
}

void OrderBook::clear() {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    buyBook.clear();
    sellBook.clear();
    tradeLog.clear();
    nextTradeId = 1;
}

size_t OrderBook::getTotalOrders() const {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    
    size_t total = 0;
    for (const auto& [price, orders] : buyBook) {
        total += orders.size();
    }
    for (const auto& [price, orders] : sellBook) {
        total += orders.size();
    }
    
    return total;
}

bool OrderBook::isEmpty() const {
    std::lock_guard<std::recursive_mutex> lock(bookMutex);
    return buyBook.empty() && sellBook.empty();
}

} // namespace velocore
