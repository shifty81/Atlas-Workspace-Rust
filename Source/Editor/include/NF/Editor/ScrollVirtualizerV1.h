#pragma once
// NF::Editor — Scroll virtualizer v1: visible range tracking for large lists/trees
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

namespace NF {

// ── SvVirtualItem ─────────────────────────────────────────────────
// Lightweight descriptor for an item in a virtualized list.
// (Prefixed Sv to avoid collision with ScrollVirtualization.h VirtualItem)

struct SvVirtualItem {
    uint32_t id       = 0;
    float    heightPx = 20.f;  // item height in pixels
    bool     visible  = true;   // filtered-in status

    [[nodiscard]] bool isValid() const { return id != 0; }
};

// ── SvVisibleRange ────────────────────────────────────────────────
// Indices [firstIndex, lastIndex] inclusive.
// (Prefixed Sv to avoid collision with VisibleRange if defined elsewhere)

struct SvVisibleRange {
    size_t firstIndex = 0;
    size_t lastIndex  = 0;

    [[nodiscard]] bool isValid() const { return firstIndex <= lastIndex; }
    [[nodiscard]] size_t count() const { return lastIndex - firstIndex + 1; }
    [[nodiscard]] bool contains(size_t index) const {
        return index >= firstIndex && index <= lastIndex;
    }
};

// ── Scroll Virtualizer V1 ─────────────────────────────────────────
// Computes which items are visible given the current scroll offset.

class ScrollVirtualizerV1 {
public:
    static constexpr float DEFAULT_ITEM_HEIGHT  = 20.f;
    static constexpr float OVER_SCAN_ITEMS      = 3.f;  // buffer items above/below

    void setItems(std::vector<SvVirtualItem> items) {
        m_items = std::move(items);
        rebuildOffsets();
    }

    void setViewportHeight(float h) {
        m_viewportH = h > 0.f ? h : 0.f;
    }

    void setScrollOffset(float offset) {
        float maxOff = totalContentHeight() - m_viewportH;
        m_scrollOffset = offset < 0.f ? 0.f : (maxOff > 0.f && offset > maxOff ? maxOff : offset);
        if (m_scrollOffset < 0.f) m_scrollOffset = 0.f;
    }

    void scrollBy(float delta) {
        setScrollOffset(m_scrollOffset + delta);
    }

    void scrollToIndex(size_t index) {
        if (index >= m_offsets.size()) return;
        setScrollOffset(m_offsets[index]);
    }

    [[nodiscard]] SvVisibleRange computeVisibleRange() const {
        if (m_items.empty()) return {};

        size_t first = 0, last = m_items.size() - 1;

        // Find first visible (with over-scan above)
        for (size_t i = 0; i < m_items.size(); ++i) {
            if (!m_items[i].visible) continue;
            if (m_offsets[i] + m_items[i].heightPx >= m_scrollOffset - OVER_SCAN_ITEMS * DEFAULT_ITEM_HEIGHT) {
                first = i;
                break;
            }
        }

        // Find last visible (with over-scan below)
        float limit = m_scrollOffset + m_viewportH + OVER_SCAN_ITEMS * DEFAULT_ITEM_HEIGHT;
        for (size_t i = first; i < m_items.size(); ++i) {
            if (!m_items[i].visible) continue;
            if (m_offsets[i] > limit) {
                last = (i > 0) ? i - 1 : 0;
                break;
            }
            last = i;
        }

        return { first, last };
    }

    [[nodiscard]] float totalContentHeight() const {
        if (m_items.empty()) return 0.f;
        return m_offsets.back() + m_items.back().heightPx;
    }

    [[nodiscard]] float scrollOffset()   const { return m_scrollOffset;  }
    [[nodiscard]] float viewportHeight() const { return m_viewportH;     }
    [[nodiscard]] size_t itemCount()     const { return m_items.size();  }

    [[nodiscard]] float itemOffset(size_t index) const {
        return index < m_offsets.size() ? m_offsets[index] : 0.f;
    }

    [[nodiscard]] const std::vector<SvVirtualItem>& items() const { return m_items; }

    // Update single item height and rebuild offsets
    bool setItemHeight(uint32_t id, float h) {
        for (auto& item : m_items) {
            if (item.id == id) {
                item.heightPx = h > 1.f ? h : 1.f;
                rebuildOffsets();
                return true;
            }
        }
        return false;
    }

    bool setItemVisible(uint32_t id, bool visible) {
        for (auto& item : m_items) {
            if (item.id == id) { item.visible = visible; rebuildOffsets(); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t visibleItemCount() const {
        size_t n = 0;
        for (const auto& item : m_items) if (item.visible) ++n;
        return n;
    }

private:
    void rebuildOffsets() {
        m_offsets.resize(m_items.size());
        float y = 0.f;
        for (size_t i = 0; i < m_items.size(); ++i) {
            m_offsets[i] = y;
            if (m_items[i].visible) y += m_items[i].heightPx;
        }
    }

    std::vector<SvVirtualItem> m_items;
    std::vector<float>          m_offsets;
    float m_scrollOffset = 0.f;
    float m_viewportH    = 400.f;
};

} // namespace NF
