#include "NF/UI/AtlasUI/Widgets/StatusBar.h"

namespace NF::UI::AtlasUI {

void StatusBar::setText(const std::string& text) {
    m_sections.clear();
    if (!text.empty()) m_sections.push_back(text);
}

void StatusBar::addSection(const std::string& text) {
    m_sections.push_back(text);
}

void StatusBar::clearSections() {
    m_sections.clear();
}

void StatusBar::measure(ILayoutContext&) {
    m_bounds.h = kBarHeight;
}

void StatusBar::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
    m_bounds.h = kBarHeight;
}

void StatusBar::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Bar background
    context.fillRect(m_bounds, Theme::ColorToken::SurfaceAlt);

    // Top border
    context.fillRect({m_bounds.x, m_bounds.y, m_bounds.w, 1.f}, Theme::ColorToken::Border);

    float x = m_bounds.x + kSectionPadX;
    const float y = m_bounds.y + (m_bounds.h - 14.f) * 0.5f;

    for (size_t i = 0; i < m_sections.size(); ++i) {
        if (i > 0) {
            // Divider
            context.fillRect({x, m_bounds.y + 4.f, 1.f, m_bounds.h - 8.f},
                             Theme::ColorToken::Border);
            x += 1.f + kSectionPadX;
        }
        context.drawText({x, y, m_bounds.w - x, 14.f},
                         m_sections[i], 0, Theme::ColorToken::TextMuted);
        x += static_cast<float>(m_sections[i].size()) * 8.f + kSectionPadX;
    }
}

} // namespace NF::UI::AtlasUI
