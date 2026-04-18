#pragma once
// NF::Editor — Scroll + virtualization system for large lists and trees
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── ScrollAxis ───────────────────────────────────────────────────

enum class ScrollAxis : uint8_t { Vertical, Horizontal, Both };

// ── ScrollState ──────────────────────────────────────────────────
// Tracks scroll position and visible extent for one or both axes.

struct ScrollState {
    float offsetX   = 0.f;
    float offsetY   = 0.f;
    float contentW  = 0.f;
    float contentH  = 0.f;
    float viewW     = 0.f;
    float viewH     = 0.f;

    [[nodiscard]] bool canScrollX() const { return contentW > viewW; }
    [[nodiscard]] bool canScrollY() const { return contentH > viewH; }

    [[nodiscard]] float maxOffsetX() const { return contentW > viewW ? contentW - viewW : 0.f; }
    [[nodiscard]] float maxOffsetY() const { return contentH > viewH ? contentH - viewH : 0.f; }

    void clamp() {
        if (offsetX < 0.f) offsetX = 0.f;
        if (offsetY < 0.f) offsetY = 0.f;
        float mx = maxOffsetX();
        float my = maxOffsetY();
        if (offsetX > mx) offsetX = mx;
        if (offsetY > my) offsetY = my;
    }

    void scrollBy(float dx, float dy) {
        offsetX += dx;
        offsetY += dy;
        clamp();
    }

    void scrollToTop()    { offsetY = 0.f; }
    void scrollToBottom() { offsetY = maxOffsetY(); }
    void scrollToLeft()   { offsetX = 0.f; }
    void scrollToRight()  { offsetX = maxOffsetX(); }

    [[nodiscard]] float normalizedY() const {
        return maxOffsetY() > 0.f ? offsetY / maxOffsetY() : 0.f;
    }
    [[nodiscard]] float normalizedX() const {
        return maxOffsetX() > 0.f ? offsetX / maxOffsetX() : 0.f;
    }
};

// ── VirtualItem ──────────────────────────────────────────────────
// Metadata for one item in a virtualized list.

struct VirtualItem {
    size_t   index    = 0;
    float    y        = 0.f;   // top position in content space
    float    height   = 20.f;
    bool     visible  = false;
    void*    userData = nullptr;

    [[nodiscard]] float bottom() const { return y + height; }
};

// ── VirtualList ──────────────────────────────────────────────────
// Manages a large list of items with visible-range culling.

class VirtualList {
public:
    static constexpr float DEFAULT_ITEM_HEIGHT = 20.f;

    explicit VirtualList(float itemHeight = DEFAULT_ITEM_HEIGHT)
        : m_itemHeight(itemHeight) {}

    void setItemCount(size_t count) {
        m_itemCount = count;
        m_scroll.contentH = static_cast<float>(count) * m_itemHeight;
    }

    [[nodiscard]] size_t itemCount() const { return m_itemCount; }

    void setViewHeight(float height) {
        m_scroll.viewH = height;
        m_scroll.clamp();
    }

    void setViewWidth(float width)  { m_scroll.viewW = width; }
    void setItemHeight(float h)     { m_itemHeight = h > 0.f ? h : DEFAULT_ITEM_HEIGHT; }
    [[nodiscard]] float itemHeight() const { return m_itemHeight; }

    [[nodiscard]] ScrollState& scroll() { return m_scroll; }
    [[nodiscard]] const ScrollState& scroll() const { return m_scroll; }

    // Returns the index of the first visible item
    [[nodiscard]] size_t firstVisibleIndex() const {
        if (m_itemCount == 0) return 0;
        size_t idx = static_cast<size_t>(m_scroll.offsetY / m_itemHeight);
        return idx < m_itemCount ? idx : m_itemCount - 1;
    }

    // Returns the number of items visible in the current view
    [[nodiscard]] size_t visibleItemCount() const {
        if (m_itemHeight <= 0.f || m_scroll.viewH <= 0.f) return 0;
        size_t count = static_cast<size_t>(m_scroll.viewH / m_itemHeight) + 2;
        size_t first = firstVisibleIndex();
        size_t remaining = m_itemCount > first ? m_itemCount - first : 0;
        return count < remaining ? count : remaining;
    }

    // Build VirtualItem metadata for the visible range
    [[nodiscard]] std::vector<VirtualItem> buildVisibleItems() const {
        std::vector<VirtualItem> items;
        size_t first = firstVisibleIndex();
        size_t count = visibleItemCount();
        items.reserve(count);
        for (size_t i = first; i < first + count && i < m_itemCount; ++i) {
            VirtualItem vi;
            vi.index   = i;
            vi.y       = static_cast<float>(i) * m_itemHeight - m_scroll.offsetY;
            vi.height  = m_itemHeight;
            vi.visible = vi.y < m_scroll.viewH && vi.bottom() > 0.f;
            items.push_back(vi);
        }
        return items;
    }

    // Scroll to make item at given index visible
    void ensureVisible(size_t index) {
        if (m_itemCount == 0) return;
        float itemTop    = static_cast<float>(index) * m_itemHeight;
        float itemBottom = itemTop + m_itemHeight;
        if (itemTop < m_scroll.offsetY)
            m_scroll.offsetY = itemTop;
        else if (itemBottom > m_scroll.offsetY + m_scroll.viewH)
            m_scroll.offsetY = itemBottom - m_scroll.viewH;
        m_scroll.clamp();
    }

private:
    size_t      m_itemCount  = 0;
    float       m_itemHeight;
    ScrollState m_scroll;
};

// ── ScrollableContainer ──────────────────────────────────────────
// Wraps a content area with independent scroll control.

class ScrollableContainer {
public:
    explicit ScrollableContainer(ScrollAxis axis = ScrollAxis::Vertical)
        : m_axis(axis) {}

    void setContentSize(float w, float h) {
        m_scroll.contentW = w;
        m_scroll.contentH = h;
        m_scroll.clamp();
    }

    void setViewSize(float w, float h) {
        m_scroll.viewW = w;
        m_scroll.viewH = h;
        m_scroll.clamp();
    }

    void scroll(float dx, float dy) {
        if (m_axis == ScrollAxis::Vertical   || m_axis == ScrollAxis::Both) m_scroll.offsetY += dy;
        if (m_axis == ScrollAxis::Horizontal || m_axis == ScrollAxis::Both) m_scroll.offsetX += dx;
        m_scroll.clamp();
    }

    [[nodiscard]] ScrollState& state() { return m_scroll; }
    [[nodiscard]] const ScrollState& state() const { return m_scroll; }
    [[nodiscard]] ScrollAxis axis() const { return m_axis; }

    [[nodiscard]] bool needsScrollbarY() const { return m_scroll.canScrollY(); }
    [[nodiscard]] bool needsScrollbarX() const { return m_scroll.canScrollX(); }

private:
    ScrollAxis  m_axis;
    ScrollState m_scroll;
};


} // namespace NF
