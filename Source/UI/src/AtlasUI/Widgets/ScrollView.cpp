#include "NF/UI/AtlasUI/Widgets/ScrollView.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

void ScrollView::setScrollOffset(float offset) {
    float maxOff = maxScrollOffset();
    m_scrollOffset = std::max(0.f, std::min(offset, maxOff));
}

void ScrollView::scrollBy(float delta) {
    setScrollOffset(m_scrollOffset + delta);
}

void ScrollView::scrollToBottom() {
    setScrollOffset(maxScrollOffset());
}

float ScrollView::maxScrollOffset() const {
    return std::max(0.f, m_contentHeight - m_bounds.h);
}

float ScrollView::scrollFraction() const {
    float maxOff = maxScrollOffset();
    if (maxOff <= 0.f) return 0.f;
    return m_scrollOffset / maxOff;
}

float ScrollView::thumbHeight() const {
    if (m_contentHeight <= 0.f) return m_bounds.h;
    float ratio = m_bounds.h / m_contentHeight;
    return std::max(20.f, m_bounds.h * std::min(1.f, ratio));
}

float ScrollView::thumbY() const {
    float trackHeight = m_bounds.h - thumbHeight();
    return m_bounds.y + trackHeight * scrollFraction();
}

void ScrollView::measure(ILayoutContext& context) {
    if (m_content) {
        m_content->measure(context);
    }
}

void ScrollView::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    if (m_content) {
        float contentWidth = canScroll() ? (bounds.w - m_scrollBarWidth) : bounds.w;
        NF::Rect contentBounds = {bounds.x, bounds.y - m_scrollOffset, contentWidth, m_contentHeight};
        m_content->arrange(contentBounds);
    }
    // Clamp scroll offset to valid range after resize
    setScrollOffset(m_scrollOffset);
}

void ScrollView::paint(IPaintContext& context) {
    if (!m_visible) return;

    context.pushClip(m_bounds);

    if (m_content && m_content->isVisible()) {
        m_content->paint(context);
    }

    // Draw scroll bar
    if (canScroll()) {
        NF::Rect trackRect = {
            m_bounds.x + m_bounds.w - m_scrollBarWidth,
            m_bounds.y,
            m_scrollBarWidth,
            m_bounds.h
        };
        context.fillRect(trackRect, Theme::ColorToken::SurfaceAlt);

        NF::Rect thumbRect = {
            trackRect.x + 1.f,
            thumbY(),
            m_scrollBarWidth - 2.f,
            thumbHeight()
        };
        context.fillRect(thumbRect, Theme::ColorToken::Border);
    }

    context.popClip();
}

bool ScrollView::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (!rectContains(m_bounds, context.mousePosition())) return false;

    // Handle scroll bar thumb dragging
    if (canScroll()) {
        NF::Rect trackRect = {
            m_bounds.x + m_bounds.w - m_scrollBarWidth,
            m_bounds.y,
            m_scrollBarWidth,
            m_bounds.h
        };

        if (m_thumbDragging) {
            if (context.primaryDown()) {
                float dy = context.mousePosition().y - m_dragStartY;
                float trackRange = m_bounds.h - thumbHeight();
                if (trackRange > 0.f) {
                    float ratio = dy / trackRange;
                    setScrollOffset(m_dragStartOffset + ratio * maxScrollOffset());
                }
                return true;
            } else {
                m_thumbDragging = false;
                context.releasePointer(this);
            }
        }

        NF::Rect thumbR = {trackRect.x + 1.f, thumbY(), m_scrollBarWidth - 2.f, thumbHeight()};
        if (rectContains(thumbR, context.mousePosition()) && context.primaryDown()) {
            m_thumbDragging = true;
            m_dragStartY = context.mousePosition().y;
            m_dragStartOffset = m_scrollOffset;
            context.capturePointer(this);
            return true;
        }
    }

    // Forward to content
    if (m_content && m_content->isVisible()) {
        if (m_content->handleInput(context)) return true;
    }

    return false;
}

} // namespace NF::UI::AtlasUI
