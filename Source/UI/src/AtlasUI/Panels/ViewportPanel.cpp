#include "NF/UI/AtlasUI/Panels/ViewportPanel.h"
#include <cstdio>

namespace NF::UI::AtlasUI {

void ViewportPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Fire resize callback when bounds change (drives FBO resize and ViewportHostRegistry update).
    if (m_resizeCallback &&
        (m_lastBounds.x != m_bounds.x || m_lastBounds.y != m_bounds.y ||
         m_lastBounds.w != m_bounds.w || m_lastBounds.h != m_bounds.h)) {
        m_lastBounds = m_bounds;
        m_resizeCallback(m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h);
    }

    // Viewport background (darker than normal panels)
    context.fillRect(m_bounds, Theme::ColorToken::Background);

    // Scene texture — if a color attachment is bound, blit the rendered scene.
    // The grid overlay is suppressed when a real scene is rendered.
    if (m_colorAttachment != 0) {
        context.drawImage(m_bounds, m_colorAttachment, 0xFFFFFFFF);
    } else if (m_gridEnabled) {
        // Placeholder grid: drawn when no GPU scene texture is available.
        // When a viewport handle is wired this means "ready, awaiting first frame".
        const float step = 40.f;
        const Color gridColor = 0xFF333333;
        for (float gx = m_bounds.x; gx < m_bounds.x + m_bounds.w; gx += step)
            context.fillRect({gx, m_bounds.y, 1.f, m_bounds.h}, gridColor);
        for (float gy = m_bounds.y; gy < m_bounds.y + m_bounds.h; gy += step)
            context.fillRect({m_bounds.x, gy, m_bounds.w, 1.f}, gridColor);

        // Center label
        const char* label = m_viewportHandle != 0 ? "[ Viewport Ready ]" : "[ 3D Viewport ]";
        context.drawText({m_bounds.x + (m_bounds.w - 160.f) * 0.5f,
                          m_bounds.y + (m_bounds.h - 14.f) * 0.5f, 160.f, 14.f},
                         label, 0, 0xFF444444);
    }

    // Overlays — always rendered on top of scene or placeholder

    // Tool mode indicator (top-left)
    const char* toolNames[] = {"Select", "Move", "Rotate", "Scale", "Paint", "Erase"};
    context.drawText({m_bounds.x + 8.f, m_bounds.y + 8.f, 100.f, 14.f},
                     toolNames[static_cast<int>(m_toolMode)], 0, Theme::ColorToken::TextMuted);

    // Render mode indicator (top-right)
    const char* renderNames[] = {"Shaded", "Wireframe", "Unlit"};
    context.drawText({m_bounds.x + m_bounds.w - 80.f, m_bounds.y + 8.f, 72.f, 14.f},
                     renderNames[static_cast<int>(m_renderMode)], 0, Theme::ColorToken::TextMuted);

    // Camera info overlay (bottom-left)
    char camText[64];
    std::snprintf(camText, sizeof(camText), "Cam: %.1f, %.1f, %.1f", m_camX, m_camY, m_camZ);
    context.drawText({m_bounds.x + 8.f, m_bounds.y + m_bounds.h - 20.f, 200.f, 14.f},
                     camText, 0, Theme::ColorToken::TextMuted);
}

} // namespace NF::UI::AtlasUI
