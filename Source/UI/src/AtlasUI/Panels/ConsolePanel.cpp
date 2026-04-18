#include "NF/UI/AtlasUI/Panels/ConsolePanel.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <algorithm>

namespace NF::UI::AtlasUI {

void ConsolePanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Panel background and border
    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    // Title bar
    NF::Rect hdr = {m_bounds.x, m_bounds.y, m_bounds.w, kHeaderH};
    context.fillRect(hdr, Theme::ColorToken::SurfaceAlt);
    context.drawText({hdr.x + 8.f, hdr.y + 4.f, hdr.w - 16.f, 14.f},
                     m_title, 0, Theme::ColorToken::Text);

    float vpH    = viewportHeight();
    float ctH    = contentHeight();
    bool  scroll = ctH > vpH;
    float listW  = m_bounds.w - (scroll ? kScrollBarW : 0.f);

    context.pushClip({m_bounds.x, m_bounds.y + kHeaderH, listW, vpH});

    float y    = (m_bounds.y + kHeaderH) - m_scrollOffset;
    float left = m_bounds.x + 8.f;

    for (const auto& msg : m_messages) {
        if (y + kRowH < m_bounds.y + kHeaderH) { y += kRowH; continue; } // above clip
        if (y > m_bounds.y + m_bounds.h) break;

        Color textColor = Theme::ColorToken::Text;
        if (msg.level == MessageLevel::Warning) textColor = 0xFFE8A435;
        else if (msg.level == MessageLevel::Error) textColor = 0xFFF44747;

        context.drawText({left, y, listW - 16.f, 14.f}, msg.text, 0, textColor);
        y += kRowH;
    }

    context.popClip();

    // Scrollbar
    if (scroll) {
        float trackX = m_bounds.x + m_bounds.w - kScrollBarW;
        float trackY = m_bounds.y + kHeaderH;
        context.fillRect({trackX, trackY, kScrollBarW, vpH},
                         Theme::ColorToken::SurfaceAlt);
        context.fillRect({trackX + 1.f, thumbY(), kScrollBarW - 2.f, thumbHeight()},
                         Theme::ColorToken::Border);
    }
}

bool ConsolePanel::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (!rectContains(m_bounds, context.mousePosition())) return false;

    NF::Vec2 mouse = context.mousePosition();
    float vpH = viewportHeight();
    float ctH = contentHeight();
    bool  scroll = ctH > vpH;

    // ── Scrollbar thumb drag ──────────────────────────────────────
    if (scroll) {
        float trackX = m_bounds.x + m_bounds.w - kScrollBarW;
        float trackY = m_bounds.y + kHeaderH;

        if (m_thumbDragging) {
            if (context.primaryDown()) {
                float dy    = mouse.y - m_dragStartY;
                float track = vpH - thumbHeight();
                if (track > 0.f) {
                    float ratio    = dy / track;
                    m_scrollOffset = std::max(0.f,
                        std::min(m_dragStartOff + ratio * maxScroll(), maxScroll()));
                }
                return true;
            }
            m_thumbDragging = false;
            context.releasePointer(static_cast<NF::UI::AtlasUI::IPanel*>(this));
        }

        NF::Rect thumbR = {trackX + 1.f, thumbY(), kScrollBarW - 2.f, thumbHeight()};
        if (rectContains(thumbR, mouse) && context.primaryDown()) {
            m_thumbDragging = true;
            m_dragStartY    = mouse.y;
            m_dragStartOff  = m_scrollOffset;
            context.capturePointer(static_cast<NF::UI::AtlasUI::IPanel*>(this));
            return true;
        }

        NF::Rect trackR = {trackX, trackY, kScrollBarW, vpH};
        if (rectContains(trackR, mouse) && context.primaryPressed()) {
            float mid = thumbY() + thumbHeight() * 0.5f;
            m_scrollOffset += (mouse.y < mid ? -vpH : vpH);
            clampScroll();
            return true;
        }
    }

    // ── Mouse wheel ───────────────────────────────────────────────
    float wheel = context.scrollDelta();
    if (wheel != 0.f) {
        m_scrollOffset -= wheel * 40.f;
        clampScroll();
        return true;
    }

    return false;
}

} // namespace NF::UI::AtlasUI
