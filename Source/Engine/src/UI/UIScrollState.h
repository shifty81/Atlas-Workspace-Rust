#pragma once
#include <cstdint>
#include <algorithm>

namespace atlas::ui {

/// Lightweight scroll state for panels with content that exceeds
/// the visible area.  Tracks scroll offset and provides clamping.
class UIScrollState {
public:
    /// Scroll by a delta (positive = scroll down, negative = scroll up).
    void Scroll(float delta) {
        m_offset = ClampOffset(m_offset + delta);
    }

    /// Set the scroll offset directly.
    void SetOffset(float offset) {
        m_offset = ClampOffset(offset);
    }

    /// Returns the current scroll offset.
    float GetOffset() const { return m_offset; }

    /// Set the total height of the content.
    void SetContentHeight(float height) {
        m_contentHeight = height;
        m_offset = ClampOffset(m_offset);
    }

    /// Returns the total content height.
    float GetContentHeight() const { return m_contentHeight; }

    /// Set the visible viewport height.
    void SetViewportHeight(float height) {
        m_viewportHeight = height;
        m_offset = ClampOffset(m_offset);
    }

    /// Returns the visible viewport height.
    float GetViewportHeight() const { return m_viewportHeight; }

    /// Returns the maximum scroll offset.
    float MaxOffset() const {
        float max = m_contentHeight - m_viewportHeight;
        return max > 0.0f ? max : 0.0f;
    }

    /// Returns true if the content is scrollable (content > viewport).
    bool IsScrollable() const {
        return m_contentHeight > m_viewportHeight;
    }

    /// Reset scroll offset to the top.
    void ScrollToTop() { m_offset = 0.0f; }

    /// Scroll to the bottom of the content.
    void ScrollToBottom() { m_offset = MaxOffset(); }

private:
    float m_offset = 0.0f;
    float m_contentHeight = 0.0f;
    float m_viewportHeight = 0.0f;

    float ClampOffset(float offset) const {
        float max = MaxOffset();
        if (offset < 0.0f) return 0.0f;
        if (offset > max) return max;
        return offset;
    }
};

} // namespace atlas::ui
