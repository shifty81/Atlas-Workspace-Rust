#include "NF/UI/AtlasUI/Widgets/ProgressBar.h"

namespace NF::UI::AtlasUI {

void ProgressBar::measure(ILayoutContext&) {
    float h = kBarHeight;
    if (!m_label.empty()) h += kLabelHeight + Theme::Spacing::Small;
    m_desiredSize = {m_bounds.w > 0.f ? m_bounds.w : 100.f, h};
}

void ProgressBar::paint(IPaintContext& context) {
    if (!m_visible) return;

    float y = m_bounds.y;

    // Label
    if (!m_label.empty()) {
        context.drawText({m_bounds.x, y, m_bounds.w, kLabelHeight},
                         m_label, 0, Theme::ColorToken::Text);
        y += kLabelHeight + Theme::Spacing::Small;
    }

    // Track
    NF::Rect track = {m_bounds.x, y, m_bounds.w, kBarHeight};
    context.fillRect(track, Theme::ColorToken::SurfaceAlt);
    context.drawRect(track, Theme::ColorToken::Border);

    // Fill
    float fillW = track.w * m_value;
    if (fillW > 0.f) {
        context.fillRect({track.x, track.y, fillW, track.h}, Theme::ColorToken::Accent);
    }
}

} // namespace NF::UI::AtlasUI
