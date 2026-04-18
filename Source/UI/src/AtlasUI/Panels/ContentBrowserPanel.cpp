#include "NF/UI/AtlasUI/Panels/ContentBrowserPanel.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <algorithm>

namespace NF::UI::AtlasUI {

void ContentBrowserPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Panel background and border
    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    // Title bar
    NF::Rect hdr = {m_bounds.x, m_bounds.y, m_bounds.w, kHeaderH};
    context.fillRect(hdr, Theme::ColorToken::SurfaceAlt);
    context.drawText({hdr.x + 8.f, hdr.y + 4.f, hdr.w - 16.f, 14.f},
                     m_title, 0, Theme::ColorToken::Text);

    const float left = m_bounds.x + 8.f;
    float y          = m_bounds.y + kHeaderH + 4.f;
    float contentW   = m_bounds.w - 16.f;

    // Current path
    context.drawText({left, y, contentW, 14.f}, m_currentPath, 0,
                     Theme::ColorToken::TextMuted);
    y += kPathH;

    // Separator
    context.fillRect({left, y, contentW, 1.f}, Theme::ColorToken::Border);
    y += 4.f; // small gap

    // Content viewport
    float vpH    = viewportHeight();
    float ctH    = contentHeight();
    bool  scroll = ctH > vpH;
    float listW  = m_bounds.w - (scroll ? kScrollBarW : 0.f);

    if (m_entries.empty()) {
        context.drawText({left, y, contentW, 14.f},
                         "(no assets found)", 0, Theme::ColorToken::TextMuted);
    } else {
        context.pushClip({m_bounds.x, m_bounds.y + kTopH, listW, vpH});

        float ey = (m_bounds.y + kTopH) - m_scrollOffset;
        for (const auto& entry : m_entries) {
            if (ey + kRowH > m_bounds.y + m_bounds.h) break;

            std::string icon     = entry.isDirectory ? "[D] " : "[F] ";
            Color       textColor = entry.isDirectory ? Theme::ColorToken::Accent
                                                      : Theme::ColorToken::Text;
            context.drawText({m_bounds.x + 8.f, ey, listW - 16.f, 14.f},
                             icon + entry.name, 0, textColor);
            ey += kRowH;
        }

        context.popClip();
    }

    // Scrollbar
    if (scroll) {
        float trackX = m_bounds.x + m_bounds.w - kScrollBarW;
        float trackY = m_bounds.y + kTopH;
        context.fillRect({trackX, trackY, kScrollBarW, vpH},
                         Theme::ColorToken::SurfaceAlt);
        context.fillRect({trackX + 1.f, thumbY(), kScrollBarW - 2.f, thumbHeight()},
                         Theme::ColorToken::Border);
    }
}

bool ContentBrowserPanel::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (!rectContains(m_bounds, context.mousePosition())) return false;

    NF::Vec2 mouse = context.mousePosition();
    float vpH = viewportHeight();
    float ctH = contentHeight();
    bool  scroll = ctH > vpH;

    // ── Scrollbar thumb drag ──────────────────────────────────────
    if (scroll) {
        float trackX = m_bounds.x + m_bounds.w - kScrollBarW;
        float trackY = m_bounds.y + kTopH;

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

    // ── Entry click ───────────────────────────────────────────────
    if (context.primaryPressed()) {
        float listW = m_bounds.w - (scroll ? kScrollBarW : 0.f);
        float ey    = (m_bounds.y + kTopH) - m_scrollOffset;
        for (const auto& entry : m_entries) {
            NF::Rect rowR = {m_bounds.x, ey, listW, kRowH};
            if (rectContains(rowR, mouse)) {
                if (m_onSelect) m_onSelect(entry.name, entry.isDirectory);
                return true;
            }
            ey += kRowH;
        }
    }

    return false;
}

} // namespace NF::UI::AtlasUI
