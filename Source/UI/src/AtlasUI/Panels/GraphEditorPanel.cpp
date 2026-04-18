#include "NF/UI/AtlasUI/Panels/GraphEditorPanel.h"
#include <string>

namespace NF::UI::AtlasUI {

void GraphEditorPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Panel background
    context.fillRect(m_bounds, Theme::ColorToken::Background);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    if (!hasOpenGraph()) {
        context.drawText({m_bounds.x + 8.f, m_bounds.y + 8.f, m_bounds.w - 16.f, 16.f},
                         "No graph open", 0, Theme::ColorToken::TextMuted);
        return;
    }

    // Graph name
    context.drawText({m_bounds.x + 8.f, m_bounds.y + 4.f, m_bounds.w - 16.f, 14.f},
                     m_graphName, 0, Theme::ColorToken::Text);

    // Draw nodes as labelled rectangles
    for (const auto& node : m_nodes) {
        NF::Rect nr{node.x + m_bounds.x, node.y + m_bounds.y + 20.f, 120.f, 60.f};
        Color bgColor = (node.id == m_selectedNodeId)
                        ? Theme::ColorToken::Selection
                        : Theme::ColorToken::SurfaceAlt;
        context.fillRect(nr, bgColor);
        context.drawRect(nr, Theme::ColorToken::Border);
        context.drawText({nr.x + 4.f, nr.y + 4.f, nr.w - 8.f, 14.f},
                         node.name, 0, Theme::ColorToken::Text);
    }

    // Link count annotation
    float ly = m_bounds.y + m_bounds.h - 20.f;
    context.drawText({m_bounds.x + 4.f, ly, m_bounds.w - 8.f, 14.f},
                     std::to_string(m_links.size()) + " link(s)", 0, Theme::ColorToken::TextMuted);
}

} // namespace NF::UI::AtlasUI
