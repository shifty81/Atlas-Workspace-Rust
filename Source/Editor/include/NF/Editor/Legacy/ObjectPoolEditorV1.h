#pragma once
// NF::Editor — Object pool editor v1: prefab pool authoring with growth policy
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Opv1PoolState : uint8_t { Active, Inactive, Full };

inline const char* opv1PoolStateName(Opv1PoolState s) {
    switch (s) {
        case Opv1PoolState::Active:   return "Active";
        case Opv1PoolState::Inactive: return "Inactive";
        case Opv1PoolState::Full:     return "Full";
    }
    return "Unknown";
}

struct Opv1Pool {
    uint64_t      id          = 0;
    std::string   name;
    uint64_t      prefabId    = 0;
    int           initialSize = 1;
    int           maxSize     = 1;
    int           growBy      = 1;
    Opv1PoolState state       = Opv1PoolState::Active;

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive() const { return state == Opv1PoolState::Active; }
};

using Opv1ChangeCallback = std::function<void(uint64_t)>;

class ObjectPoolEditorV1 {
public:
    static constexpr size_t MAX_POOLS = 256;

    bool addPool(const Opv1Pool& pool) {
        if (!pool.isValid()) return false;
        for (const auto& p : m_pools) if (p.id == pool.id) return false;
        if (m_pools.size() >= MAX_POOLS) return false;
        m_pools.push_back(pool);
        if (m_onChange) m_onChange(pool.id);
        return true;
    }

    bool removePool(uint64_t id) {
        for (auto it = m_pools.begin(); it != m_pools.end(); ++it) {
            if (it->id == id) { m_pools.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Opv1Pool* findPool(uint64_t id) {
        for (auto& p : m_pools) if (p.id == id) return &p;
        return nullptr;
    }

    bool setState(uint64_t id, Opv1PoolState state) {
        auto* p = findPool(id);
        if (!p) return false;
        p->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setInitialSize(uint64_t id, int size) {
        auto* p = findPool(id);
        if (!p) return false;
        p->initialSize = std::max(1, size);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setMaxSize(uint64_t id, int size) {
        auto* p = findPool(id);
        if (!p) return false;
        p->maxSize = std::max(p->initialSize, size);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setGrowBy(uint64_t id, int growBy) {
        auto* p = findPool(id);
        if (!p) return false;
        p->growBy = std::max(1, growBy);
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t poolCount() const { return m_pools.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (const auto& p : m_pools) if (p.isActive()) ++c;
        return c;
    }

    [[nodiscard]] int totalCapacity() const {
        int total = 0;
        for (const auto& p : m_pools) total += p.maxSize;
        return total;
    }

    void setOnChange(Opv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Opv1Pool> m_pools;
    Opv1ChangeCallback    m_onChange;
};

} // namespace NF
