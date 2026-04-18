#include "NF/UI/AtlasUI/Widgets/RadioButton.h"

namespace NF::UI::AtlasUI {

void RadioButton::measure(ILayoutContext& context) {
    auto textSz = context.measureText(m_label, 13.f);
    m_desiredSize = {kDotSize + Theme::Spacing::Small + textSz.x,
                     std::max(kDotSize, textSz.y)};
}

void RadioButton::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Outer circle (approximated as filled rect with insets to suggest roundness)
    NF::Rect dotRect = {m_bounds.x, m_bounds.y + (m_bounds.h - kDotSize) * 0.5f,
                        kDotSize, kDotSize};
    Color bg = m_hovered ? Theme::ColorToken::SurfaceAlt : Theme::ColorToken::Surface;
    context.fillRect(dotRect, bg);
    context.drawRect(dotRect, m_selected ? Theme::ColorToken::Accent : Theme::ColorToken::Border);

    // Inner fill when selected
    if (m_selected) {
        float inset = 3.f;
        context.fillRect({dotRect.x + inset, dotRect.y + inset,
                          dotRect.w - inset * 2.f, dotRect.h - inset * 2.f},
                         Theme::ColorToken::Accent);
    }

    // Label
    if (!m_label.empty()) {
        float lx = m_bounds.x + kDotSize + Theme::Spacing::Small;
        float ly = m_bounds.y + (m_bounds.h - 14.f) * 0.5f;
        context.drawText({lx, ly, m_bounds.w - lx, 14.f},
                         m_label, 0, Theme::ColorToken::Text);
    }
}

bool RadioButton::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    const bool inside = rectContains(m_bounds, context.mousePosition());
    m_hovered = inside;
    if (inside && context.primaryDown() && !m_selected) {
        m_selected = true;
        if (m_onSelect) m_onSelect(m_value);
        return true;
    }
    return inside;
}

} // namespace NF::UI::AtlasUI
