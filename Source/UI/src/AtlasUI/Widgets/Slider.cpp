#include "NF/UI/AtlasUI/Widgets/Slider.h"

namespace NF::UI::AtlasUI {

void Slider::measure(ILayoutContext& context) {
    float h = kTrackHeight + kThumbSize;
    if (!m_label.empty()) h += kLabelHeight + Theme::Spacing::Small;
    auto avail = context.availableSize();
    m_desiredSize = {avail.x, h};
}

void Slider::arrange(const NF::Rect& bounds) {
    m_bounds = bounds;
}

void Slider::paint(IPaintContext& context) {
    if (!m_visible) return;

    float y = m_bounds.y;

    // Label
    if (!m_label.empty()) {
        context.drawText({m_bounds.x, y, m_bounds.w, kLabelHeight},
                         m_label, 0, Theme::ColorToken::Text);
        y += kLabelHeight + Theme::Spacing::Small;
    }

    // Track background
    NF::Rect tr = trackRect();
    context.fillRect(tr, Theme::ColorToken::SurfaceAlt);
    context.drawRect(tr, Theme::ColorToken::Border);

    // Filled portion
    float fillW = tr.w * fraction();
    if (fillW > 0.f) {
        context.fillRect({tr.x, tr.y, fillW, tr.h}, Theme::ColorToken::Accent);
    }

    // Thumb
    context.fillRect(thumbRect(), Theme::ColorToken::AccentHover);
    context.drawRect(thumbRect(), Theme::ColorToken::Border);
}

bool Slider::handleInput(IInputContext& context) {
    if (!m_visible) return false;

    NF::Vec2 mp = context.mousePosition();
    NF::Rect thumb = thumbRect();
    NF::Rect track = trackRect();

    if (m_dragging) {
        if (context.primaryDown()) {
            setValueFromMouseX(mp.x);
            return true;
        }
        m_dragging = false;
        context.releasePointer(this);
        return false;
    }

    if (rectContains(thumb, mp) && context.primaryDown()) {
        m_dragging = true;
        context.capturePointer(this);
        setValueFromMouseX(mp.x);
        return true;
    }

    // Click on track jumps to position
    if (rectContains(track, mp) && context.primaryDown()) {
        setValueFromMouseX(mp.x);
        return true;
    }

    return rectContains(m_bounds, mp);
}

NF::Rect Slider::trackRect() const {
    float trackY = m_bounds.y;
    if (!m_label.empty()) trackY += kLabelHeight + Theme::Spacing::Small;
    trackY += (kThumbSize - kTrackHeight) * 0.5f;
    return {m_bounds.x + kThumbSize * 0.5f,
            trackY,
            std::max(0.f, m_bounds.w - kThumbSize),
            kTrackHeight};
}

NF::Rect Slider::thumbRect() const {
    NF::Rect tr = trackRect();
    float thumbX = tr.x + tr.w * fraction() - kThumbSize * 0.5f;
    float thumbY = tr.y + (tr.h - kThumbSize) * 0.5f;
    return {thumbX, thumbY, kThumbSize, kThumbSize};
}

void Slider::setValueFromMouseX(float mouseX) {
    NF::Rect tr = trackRect();
    float range = m_max - m_min;
    if (tr.w <= 0.f || range <= 0.f) return;
    float frac = std::clamp((mouseX - tr.x) / tr.w, 0.f, 1.f);
    float newVal = m_min + frac * range;
    if (newVal != m_value) {
        m_value = newVal;
        if (m_onChange) m_onChange(m_value);
    }
}

} // namespace NF::UI::AtlasUI
