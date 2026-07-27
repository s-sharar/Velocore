#pragma once

#include "OrderBook.h"

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace velocore {

/**
 * Owns one OrderBook per symbol. Map lock is only for insert/lookup;
 * matching uses each book's own bookMutex. Books are never erased in v1,
 * so OrderBook& remains valid after the map lock is released.
 */
class OrderBookRegistry {
public:
    OrderBook& getOrCreate(const std::string& symbol);
    OrderBook* tryGet(const std::string& symbol);
    const OrderBook* tryGet(const std::string& symbol) const;

    std::vector<std::string> symbols() const;
    size_t totalOrders() const;
    size_t totalTrades() const;
    crow::json::wvalue aggregateStatistics() const;

    /** Snapshot of raw pointers valid for the process lifetime (no erase). */
    std::vector<std::pair<std::string, OrderBook*>> allBooks();

private:
    mutable std::shared_mutex mapMu_;
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> books_;
};

} // namespace velocore
