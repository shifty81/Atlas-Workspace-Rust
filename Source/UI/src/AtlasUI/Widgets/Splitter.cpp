#include "NF/UI/AtlasUI/Widgets/Splitter.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

void Splitter::setFirst(std::shared_ptr<IWidget> widget) {
    if (m_children.empty()) m_children.resize(2);
    m_children[0] = std::move(widget);
}

void Splitter::setSecond(std::shared_ptr<IWidget> widget) {
    if (m_children.size() < 2) m_children.resize(2);
    m_children[1] = std::move(widget);
}

void Splitter::measure(ILayoutContext& context) {
    for (const auto& child : m_children) {
        if (child && child->isVisible()) child->measure(context);
    }
}

void Splitter::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    if (m_children.size() < 2) return;

    NF::Rect first = bounds;
    NF::Rect second = bounds;
    if (m_orientation == SplitOrientation::Horizontal) {
        const float splitX = bounds.w * m_ratio;
        first.w = std::max(0.f, splitX - m_separatorThickness * 0.5f);
        second.x += splitX + m_separatorThickness * 0.5f;
        second.w = std::max(0.f, bounds.w - (second.x - bounds.x));
    } else {
        const float splitY = bounds.h * m_ratio;
        first.h = std::max(0.f, splitY - m_separatorThickness * 0.5f);
        second.y += splitY + m_separatorThickness * 0.5f;
        second.h = std::max(0.f, bounds.h - (second.y - bounds.y));
    }

    if (m_children[0]) m_children[0]->arrange(first);
    if (m_children[1]) m_children[1]->arrange(second);
}

void Splitter::paint(IPaintContext& context) {
    if (!m_visible) return;
    for (const auto& child : m_children) {
        if (child && child->isVisible()) child->paint(context);
    }
    context.fillRect(separatorRect(), Theme::ColorToken::Border);
}

bool Splitter::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    const bool onSeparator = rectContains(separatorRect(), context.mousePosition());
    if (onSeparator && context.primaryDown() && !m_dragging) {
        m_dragging = true;
        context.capturePointer(this);
        return true;
    }
    if (m_dragging) {
        if (context.primaryDown()) {
            updateRatioFromMouse(context.mousePosition());
        } else {
            m_dragging = false;
            context.releasePointer(this);
        }
        return true;
    }
    return Container::handleInput(context);
}

NF::Rect Splitter::separatorRect() const {
    if (m_orientation == SplitOrientation::Horizontal) {
        const float x = m_bounds.x + m_bounds.w * m_ratio - m_separatorThickness * 0.5f;
        return {x, m_bounds.y, m_separatorThickness, m_bounds.h};
    }
    const float y = m_bounds.y + m_bounds.h * m_ratio - m_separatorThickness * 0.5f;
    return {m_bounds.x, y, m_bounds.w, m_separatorThickness};
}

void Splitter::updateRatioFromMouse(NF::Vec2 mousePosition) {
    if (m_orientation == SplitOrientation::Horizontal && m_bounds.w > 0.f) {
        m_ratio = std::clamp((mousePosition.x - m_bounds.x) / m_bounds.w, 0.1f, 0.9f);
    } else if (m_orientation == SplitOrientation::Vertical && m_bounds.h > 0.f) {
        m_ratio = std::clamp((mousePosition.y - m_bounds.y) / m_bounds.h, 0.1f, 0.9f);
    }
}

} // namespace NF::UI::AtlasUI
