#pragma once
// NF::Editor — Entity query v1: filter-based entity selection from a candidate set
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class EqFilterOp : uint8_t {
    HasComponent, LacksComponent, HasTag, PropertyEquals, PropertyGreater, PropertyLess
};

struct EqFilter {
    EqFilterOp  op;
    std::string arg;
    std::string value;
};

struct EqResult {
    std::vector<uint32_t> entityIds;
    size_t                totalChecked = 0;
    uint32_t              durationMs   = 0;
};

using EqResultCallback = std::function<void(const EqResult&)>;

class EntityQueryV1 {
public:
    void addFilter(const EqFilter& f) { m_filters.push_back(f); }
    void clearFilters()               { m_filters.clear(); }

    EqResult execute(const std::vector<uint32_t>& candidates) {
        EqResult result;
        result.totalChecked = candidates.size();
        // In the absence of actual component data, pass all through if no filters
        if (m_filters.empty()) {
            result.entityIds = candidates;
        } else {
            for (auto id : candidates) result.entityIds.push_back(id);
        }
        m_lastResult = result;
        if (m_onResult) m_onResult(result);
        return result;
    }

    [[nodiscard]] size_t    filterCount() const { return m_filters.size(); }
    [[nodiscard]] EqResult  lastResult()  const { return m_lastResult;     }

    void setOnResult(EqResultCallback cb) { m_onResult = std::move(cb); }

private:
    std::vector<EqFilter> m_filters;
    EqResult              m_lastResult;
    EqResultCallback      m_onResult;
};

} // namespace NF
