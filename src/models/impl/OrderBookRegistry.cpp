#include "OrderBookRegistry.h"

namespace velocore {

OrderBook& OrderBookRegistry::getOrCreate(const std::string& symbol) {
    {
        std::shared_lock<std::shared_mutex> lock(mapMu_);
        auto it = books_.find(symbol);
        if (it != books_.end()) {
            return *it->second;
        }
    }

    std::unique_lock<std::shared_mutex> lock(mapMu_);
    auto it = books_.find(symbol);
    if (it != books_.end()) {
        return *it->second;
    }

    auto [inserted, _] = books_.emplace(symbol, std::make_unique<OrderBook>());
    return *inserted->second;
}

OrderBook* OrderBookRegistry::tryGet(const std::string& symbol) {
    std::shared_lock<std::shared_mutex> lock(mapMu_);
    auto it = books_.find(symbol);
    if (it == books_.end()) {
        return nullptr;
    }
    return it->second.get();
}

const OrderBook* OrderBookRegistry::tryGet(const std::string& symbol) const {
    std::shared_lock<std::shared_mutex> lock(mapMu_);
    auto it = books_.find(symbol);
    if (it == books_.end()) {
        return nullptr;
    }
    return it->second.get();
}

std::vector<std::string> OrderBookRegistry::symbols() const {
    std::shared_lock<std::shared_mutex> lock(mapMu_);
    std::vector<std::string> result;
    result.reserve(books_.size());
    for (const auto& [symbol, _] : books_) {
        result.push_back(symbol);
    }
    return result;
}

size_t OrderBookRegistry::totalOrders() const {
    std::vector<const OrderBook*> books;
    {
        std::shared_lock<std::shared_mutex> lock(mapMu_);
        books.reserve(books_.size());
        for (const auto& [_, book] : books_) {
            books.push_back(book.get());
        }
    }

    size_t total = 0;
    for (const OrderBook* book : books) {
        total += book->getTotalOrders();
    }
    return total;
}

size_t OrderBookRegistry::totalTrades() const {
    std::vector<const OrderBook*> books;
    {
        std::shared_lock<std::shared_mutex> lock(mapMu_);
        books.reserve(books_.size());
        for (const auto& [_, book] : books_) {
            books.push_back(book.get());
        }
    }

    size_t total = 0;
    for (const OrderBook* book : books) {
        total += book->getTradeCount();
    }
    return total;
}

crow::json::wvalue OrderBookRegistry::aggregateStatistics() const {
    std::vector<std::pair<std::string, const OrderBook*>> books;
    {
        std::shared_lock<std::shared_mutex> lock(mapMu_);
        books.reserve(books_.size());
        for (const auto& [symbol, book] : books_) {
            books.emplace_back(symbol, book.get());
        }
    }

    crow::json::wvalue result;
    for (const auto& [symbol, book] : books) {
        result[symbol] = book->getBookStatistics();
    }
    return result;
}

std::vector<std::pair<std::string, OrderBook*>> OrderBookRegistry::allBooks() {
    std::shared_lock<std::shared_mutex> lock(mapMu_);
    std::vector<std::pair<std::string, OrderBook*>> result;
    result.reserve(books_.size());
    for (auto& [symbol, book] : books_) {
        result.emplace_back(symbol, book.get());
    }
    return result;
}

} // namespace velocore
