#include "NF/UI/AtlasUI/Widgets/NotificationCard.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

NotificationCard::NotificationCard(std::string message, NotificationLevel level)
    : m_message(std::move(message)), m_level(level) {}

void NotificationCard::paint(IPaintContext& context) {
    if (!m_visible || m_expired) return;

    Color accentColor = Theme::ColorToken::Accent;
    switch (m_level) {
        case NotificationLevel::Warning: accentColor = 0xFFD9A61E; break;
        case NotificationLevel::Error:   accentColor = 0xFFD94D4D; break;
        default: break;
    }

    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    // Accent bar on left side
    NF::Rect accentBar = {m_bounds.x, m_bounds.y, 3.f, m_bounds.h};
    context.fillRect(accentBar, accentColor);

    NF::Rect textArea = {m_bounds.x + 10.f, m_bounds.y + 8.f,
                         m_bounds.w - 20.f, m_bounds.h - 16.f};
    context.drawText(textArea, m_message, 0, Theme::ColorToken::Text);
}

} // namespace NF::UI::AtlasUI
