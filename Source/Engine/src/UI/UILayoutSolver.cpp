#include "UILayoutSolver.h"
#include "../sim/StateHasher.h"
#include <algorithm>
#include <numeric>

namespace atlas::ui {

void UILayoutSolver::Clear() {
    m_entries.clear();
}

void UILayoutSolver::AddEntry(uint32_t widgetId, const UIConstraint& constraint) {
    LayoutEntry entry;
    entry.widgetId = widgetId;
    entry.constraint = constraint;
    m_entries.push_back(entry);
}

void UILayoutSolver::Solve(const UILayoutRect& bounds, LayoutDirection direction) {
    if (m_entries.empty()) return;

    if (m_entries.size() == 1) {
        auto& entry = m_entries[0];
        entry.resolved.x = bounds.x;
        entry.resolved.y = bounds.y;
        entry.resolved.w = std::clamp(bounds.w, entry.constraint.minWidth,
                                       entry.constraint.maxWidth);
        entry.resolved.h = std::clamp(bounds.h, entry.constraint.minHeight,
                                       entry.constraint.maxHeight);
        return;
    }

    bool isHorizontal = (direction == LayoutDirection::Horizontal);
    int32_t totalSpace = isHorizontal ? bounds.w : bounds.h;

    // First pass: allocate minimum sizes
    int32_t totalMin = 0;
    for (auto& entry : m_entries) {
        totalMin += isHorizontal ? entry.constraint.minWidth : entry.constraint.minHeight;
    }

    // Second pass: distribute remaining space by weight
    int32_t remaining = totalSpace - totalMin;
    if (remaining < 0) remaining = 0;

    float totalWeight = 0.0f;
    for (auto& entry : m_entries) {
        totalWeight += entry.constraint.weight;
    }

    int32_t offset = isHorizontal ? bounds.x : bounds.y;
    int32_t distributed = 0;

    for (size_t i = 0; i < m_entries.size(); ++i) {
        auto& entry = m_entries[i];
        int32_t minSize = isHorizontal ? entry.constraint.minWidth : entry.constraint.minHeight;
        int32_t maxSize = isHorizontal ? entry.constraint.maxWidth : entry.constraint.maxHeight;

        int32_t extra = 0;
        if (totalWeight > 0.0f) {
            if (i == m_entries.size() - 1) {
                // Last entry gets remaining to avoid rounding errors
                extra = remaining - distributed;
            } else {
                extra = static_cast<int32_t>(
                    (entry.constraint.weight / totalWeight) * static_cast<float>(remaining));
                distributed += extra;
            }
        }

        int32_t size = std::clamp(minSize + extra, minSize, maxSize);

        if (isHorizontal) {
            entry.resolved.x = offset;
            entry.resolved.y = bounds.y;
            entry.resolved.w = size;
            entry.resolved.h = std::clamp(bounds.h, entry.constraint.minHeight,
                                           entry.constraint.maxHeight);
            offset += size;
        } else {
            entry.resolved.x = bounds.x;
            entry.resolved.y = offset;
            entry.resolved.w = std::clamp(bounds.w, entry.constraint.minWidth,
                                           entry.constraint.maxWidth);
            entry.resolved.h = size;
            offset += size;
        }
    }
}

const UILayoutRect* UILayoutSolver::GetResolved(uint32_t widgetId) const {
    for (const auto& entry : m_entries) {
        if (entry.widgetId == widgetId) {
            return &entry.resolved;
        }
    }
    return nullptr;
}

const std::vector<LayoutEntry>& UILayoutSolver::Entries() const {
    return m_entries;
}

size_t UILayoutSolver::EntryCount() const {
    return m_entries.size();
}

uint64_t UILayoutSolver::LayoutHash() const {
    uint64_t hash = 0;
    for (const auto& entry : m_entries) {
        // Hash widgetId
        uint32_t id = entry.widgetId;
        hash = sim::StateHasher::HashCombine(
            hash, reinterpret_cast<const uint8_t*>(&id), sizeof(id));
        // Hash resolved rect fields (x, y, w, h) as int32_t
        int32_t vals[4] = { entry.resolved.x, entry.resolved.y,
                            entry.resolved.w, entry.resolved.h };
        hash = sim::StateHasher::HashCombine(
            hash, reinterpret_cast<const uint8_t*>(vals), sizeof(vals));
    }
    return hash;
}

} // namespace atlas::ui
