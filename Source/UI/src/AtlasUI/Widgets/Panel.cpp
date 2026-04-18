#include "NF/UI/AtlasUI/Widgets/Panel.h"

namespace NF::UI::AtlasUI {

void Panel::paint(IPaintContext& context) {
    if (!m_visible) return;

    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    NF::Rect titleRect = m_bounds;
    titleRect.x += 8.f;
    titleRect.y += 8.f;
    titleRect.h = 20.f;
    context.drawText(titleRect, m_title, 0, Theme::ColorToken::Text);
}

} // namespace NF::UI::AtlasUI
