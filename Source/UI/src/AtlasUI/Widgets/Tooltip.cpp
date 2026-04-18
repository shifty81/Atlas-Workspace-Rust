#include "NF/UI/AtlasUI/Widgets/Tooltip.h"

namespace NF::UI::AtlasUI {

void Tooltip::show(const std::string& text, NF::Vec2 anchor) {
    m_text = text;
    m_anchor = anchor;
    m_visible = true;

    constexpr float kCharWidth = 8.f;
    constexpr float kPadding = 8.f;
    float w = static_cast<float>(m_text.size()) * kCharWidth + kPadding * 2.f;
    float h = 16.f + kPadding * 2.f;
    m_bounds = {m_anchor.x, m_anchor.y + 18.f, w, h};
}

void Tooltip::hide() {
    m_visible = false;
}

void Tooltip::paint(IPaintContext& context) {
    if (!m_visible) return;
    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);
    NF::Rect textRect = {m_bounds.x + 8.f, m_bounds.y + 8.f,
                         m_bounds.w - 16.f, m_bounds.h - 16.f};
    context.drawText(textRect, m_text, 0, Theme::ColorToken::Text);
}

} // namespace NF::UI::AtlasUI
