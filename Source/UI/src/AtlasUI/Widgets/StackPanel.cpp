#include "NF/UI/AtlasUI/Widgets/StackPanel.h"

namespace NF::UI::AtlasUI {

void StackPanel::measure(ILayoutContext& context) {
    m_desiredSize = {};
    bool first = true;
    for (const auto& child : m_children) {
        if (!child || !child->isVisible()) continue;
        child->measure(context);
        const NF::Rect childBounds = child->bounds();
        if (m_orientation == StackOrientation::Vertical) {
            m_desiredSize.x = std::max(m_desiredSize.x, childBounds.w);
            m_desiredSize.y += childBounds.h;
            if (!first) m_desiredSize.y += m_spacing;
        } else {
            m_desiredSize.x += childBounds.w;
            if (!first) m_desiredSize.x += m_spacing;
            m_desiredSize.y = std::max(m_desiredSize.y, childBounds.h);
        }
        first = false;
    }
}

void StackPanel::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    float offset = 0.f;
    size_t visibleCount = 0;
    for (const auto& child : m_children) if (child && child->isVisible()) ++visibleCount;
    if (visibleCount == 0) return;
    const float totalSpacing = m_spacing * static_cast<float>(visibleCount - 1);
    const float slotExtent = m_orientation == StackOrientation::Vertical
        ? std::max(0.f, (bounds.h - totalSpacing) / static_cast<float>(visibleCount))
        : std::max(0.f, (bounds.w - totalSpacing) / static_cast<float>(visibleCount));

    for (const auto& child : m_children) {
        if (!child || !child->isVisible()) continue;
        NF::Rect childRect = bounds;
        if (m_orientation == StackOrientation::Vertical) {
            childRect.y += offset;
            childRect.h = slotExtent;
            offset += slotExtent + m_spacing;
        } else {
            childRect.x += offset;
            childRect.w = slotExtent;
            offset += slotExtent + m_spacing;
        }
        child->arrange(childRect);
    }
}

void StackPanel::paint(IPaintContext& context) {
    if (!m_visible) return;
    for (const auto& child : m_children) {
        if (child && child->isVisible()) child->paint(context);
    }
}

} // namespace NF::UI::AtlasUI
