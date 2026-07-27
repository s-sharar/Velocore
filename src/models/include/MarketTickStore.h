#pragma once

#include "Types.h"

#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace velocore {

struct SymbolTape {
    mutable std::mutex mu;
    MarketTick latest;
};

/**
 * Per-symbol latest-tick store. Map lock only for create/lookup;
 * each SymbolTape has its own mutex for the hot path.
 */
class MarketTickStore {
public:
    void upsert(const MarketTick& tick);
    std::optional<MarketTick> get(const std::string& symbol) const;
    std::vector<MarketTick> snapshotAll() const;

private:
    std::shared_ptr<SymbolTape> getOrCreateTape(const std::string& symbol);
    std::shared_ptr<SymbolTape> findTape(const std::string& symbol) const;

    mutable std::shared_mutex mapMu_;
    std::unordered_map<std::string, std::shared_ptr<SymbolTape>> tapes_;
};

} // namespace velocore
