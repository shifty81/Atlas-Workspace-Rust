#include "NF/UI/AtlasUI/Widgets/Checkbox.h"

namespace NF::UI::AtlasUI {

void Checkbox::measure(ILayoutContext& context) {
    auto textSz = context.measureText(m_label, 13.f);
    m_desiredSize = {kBoxSize + Theme::Spacing::Small + textSz.x,
                     std::max(kBoxSize, textSz.y)};
}

void Checkbox::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Box
    NF::Rect boxRect = {m_bounds.x, m_bounds.y + (m_bounds.h - kBoxSize) * 0.5f,
                        kBoxSize, kBoxSize};
    Color bg = m_checked  ? Theme::ColorToken::Accent
             : m_hovered  ? Theme::ColorToken::SurfaceAlt
                          : Theme::ColorToken::Surface;
    context.fillRect(boxRect, bg);
    context.drawRect(boxRect, Theme::ColorToken::Border);

    // Check mark (two filled rects forming a tick shape)
    if (m_checked) {
        float cx = boxRect.x + 3.f;
        float cy = boxRect.y + 6.f;
        context.fillRect({cx,       cy,     3.f, 5.f}, Theme::ColorToken::Text);
        context.fillRect({cx + 2.f, cy - 3.f, 8.f, 3.f}, Theme::ColorToken::Text);
    }

    // Label
    if (!m_label.empty()) {
        float lx = m_bounds.x + kBoxSize + Theme::Spacing::Small;
        float ly = m_bounds.y + (m_bounds.h - 14.f) * 0.5f;
        context.drawText({lx, ly, m_bounds.w - lx, 14.f},
                         m_label, 0, Theme::ColorToken::Text);
    }
}

bool Checkbox::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    const bool inside = rectContains(m_bounds, context.mousePosition());
    m_hovered = inside;
    if (inside && context.primaryDown()) {
        m_checked = !m_checked;
        if (m_onChange) m_onChange(m_checked);
        return true;
    }
    return inside;
}

} // namespace NF::UI::AtlasUI
