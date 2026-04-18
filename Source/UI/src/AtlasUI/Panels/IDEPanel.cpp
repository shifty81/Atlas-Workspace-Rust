#include "NF/UI/AtlasUI/Panels/IDEPanel.h"

namespace NF::UI::AtlasUI {

void IDEPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Panel background
    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    // Title bar
    NF::Rect hdr = m_bounds;
    hdr.h = 22.f;
    context.fillRect(hdr, Theme::ColorToken::SurfaceAlt);
    context.drawText({hdr.x + 8.f, hdr.y + 4.f, hdr.w - 16.f, 14.f},
                     m_title, 0, Theme::ColorToken::Text);

    float y = m_bounds.y + 30.f;
    const float left = m_bounds.x + 8.f;
    const float contentW = m_bounds.w - 16.f;

    // Search query
    if (!m_searchQuery.empty()) {
        context.drawText({left, y, contentW, 14.f},
                         "Search: " + m_searchQuery, 0, Theme::ColorToken::TextMuted);
        y += 18.f;

        // Separator
        context.fillRect({left, y, contentW, 1.f}, Theme::ColorToken::Border);
        y += Theme::Spacing::Small;
    }

    // Results
    for (const auto& result : m_results) {
        if (y > m_bounds.y + m_bounds.h - 4.f) break;

        std::string text = result.symbolName + " @ " + result.filePath;
        if (result.line > 0) {
            text += ":" + std::to_string(result.line);
        }
        context.drawText({left, y, contentW, 14.f}, text, 0, Theme::ColorToken::Text);
        y += 16.f;
    }
}

} // namespace NF::UI::AtlasUI
