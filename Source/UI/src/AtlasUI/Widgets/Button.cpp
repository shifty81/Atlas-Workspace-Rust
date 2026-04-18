#include "NF/UI/AtlasUI/Widgets/Button.h"

namespace NF::UI::AtlasUI {

void Button::measure(ILayoutContext& context) {
    auto textSize = context.measureText(m_text, 14.f);
    m_desiredSize = {textSize.x + 20.f, textSize.y + 12.f};
}

void Button::paint(IPaintContext& context) {
    if (!m_visible) return;
    Color fill = Theme::ColorToken::SurfaceAlt;
    if (m_pressed) fill = Theme::ColorToken::Pressed;
    else if (m_hovered) fill = Theme::ColorToken::AccentHover;
    context.fillRect(m_bounds, fill);
    context.drawRect(m_bounds, Theme::ColorToken::Border);
    context.drawText(insetRect(m_bounds, Theme::Spacing::Medium, Theme::Spacing::Small),
                     m_text, 0, Theme::ColorToken::Text);
}

bool Button::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    const bool inside = rectContains(m_bounds, context.mousePosition());
    m_hovered = inside;
    if (inside && context.primaryDown() && !m_pressed) {
        m_pressed = true;
        context.capturePointer(this);
        return true;
    }
    if (m_pressed && !context.primaryDown()) {
        context.releasePointer(this);
        bool trigger = inside;
        m_pressed = false;
        if (trigger && m_onClick) m_onClick();
        return trigger;
    }
    return inside;
}

} // namespace NF::UI::AtlasUI
