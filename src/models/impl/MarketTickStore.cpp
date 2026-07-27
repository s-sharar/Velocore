#include "MarketTickStore.h"

namespace velocore {

std::shared_ptr<SymbolTape> MarketTickStore::getOrCreateTape(const std::string& symbol) {
    {
        std::shared_lock<std::shared_mutex> lock(mapMu_);
        auto it = tapes_.find(symbol);
        if (it != tapes_.end()) {
            return it->second;
        }
    }

    std::unique_lock<std::shared_mutex> lock(mapMu_);
    auto it = tapes_.find(symbol);
    if (it != tapes_.end()) {
        return it->second;
    }

    auto tape = std::make_shared<SymbolTape>();
    tapes_.emplace(symbol, tape);
    return tape;
}

std::shared_ptr<SymbolTape> MarketTickStore::findTape(const std::string& symbol) const {
    std::shared_lock<std::shared_mutex> lock(mapMu_);
    auto it = tapes_.find(symbol);
    if (it == tapes_.end()) {
        return nullptr;
    }
    return it->second;
}

void MarketTickStore::upsert(const MarketTick& tick) {
    auto tape = getOrCreateTape(tick.symbol);
    std::lock_guard<std::mutex> lock(tape->mu);
    tape->latest = tick;
}

std::optional<MarketTick> MarketTickStore::get(const std::string& symbol) const {
    auto tape = findTape(symbol);
    if (!tape) {
        return std::nullopt;
    }
    std::lock_guard<std::mutex> lock(tape->mu);
    return tape->latest;
}

std::vector<MarketTick> MarketTickStore::snapshotAll() const {
    std::vector<std::shared_ptr<SymbolTape>> tapes;
    {
        std::shared_lock<std::shared_mutex> lock(mapMu_);
        tapes.reserve(tapes_.size());
        for (const auto& [_, tape] : tapes_) {
            tapes.push_back(tape);
        }
    }

    std::vector<MarketTick> result;
    result.reserve(tapes.size());
    for (const auto& tape : tapes) {
        std::lock_guard<std::mutex> lock(tape->mu);
        result.push_back(tape->latest);
    }
    return result;
}

} // namespace velocore
