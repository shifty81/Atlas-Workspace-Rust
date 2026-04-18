#include "NF/UI/AtlasUI/Panels/HierarchyPanel.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <algorithm>
#include <cstdio>

namespace NF::UI::AtlasUI {

float HierarchyPanel::contentHeight() const {
    float h = 0.f;
    if (!m_searchFilter.empty()) h += 18.f; // filter row
    for (const auto& e : m_entities) {
        if (!m_searchFilter.empty() && e.name.find(m_searchFilter) == std::string::npos)
            continue;
        h += kRowH;
    }
    return h;
}

float HierarchyPanel::thumbHeight() const {
    float vpH = viewportHeight();
    float ctH = contentHeight();
    if (ctH <= vpH) return vpH;
    float ratio = vpH / ctH;
    return std::max(20.f, vpH * std::min(1.f, ratio));
}

float HierarchyPanel::thumbY() const {
    float vpH   = viewportHeight();
    float track = vpH - thumbHeight();
    float mx    = maxScroll();
    float frac  = (mx > 0.f) ? (m_scrollOffset / mx) : 0.f;
    return (m_bounds.y + kHeaderH) + track * frac;
}

void HierarchyPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Panel background and border
    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    // Title bar
    NF::Rect hdr = {m_bounds.x, m_bounds.y, m_bounds.w, kHeaderH};
    context.fillRect(hdr, Theme::ColorToken::SurfaceAlt);
    context.drawText({hdr.x + 8.f, hdr.y + 4.f, hdr.w - 16.f, 14.f},
                     m_title, 0, Theme::ColorToken::Text);

    // Content area
    float vpH    = viewportHeight();
    float ctH    = contentHeight();
    bool  scroll = ctH > vpH;
    float contentW = m_bounds.w - (scroll ? kScrollBarW : 0.f);

    context.pushClip({m_bounds.x, m_bounds.y + kHeaderH, contentW, vpH});

    float y    = (m_bounds.y + kHeaderH) - m_scrollOffset;
    float left = m_bounds.x + 8.f;

    if (!m_searchFilter.empty()) {
        context.drawText({left, y, contentW - 16.f, 14.f},
                         "Filter: " + m_searchFilter, 0, Theme::ColorToken::TextMuted);
        y += 18.f;
    }

    for (const auto& entity : m_entities) {
        if (!m_searchFilter.empty() &&
            entity.name.find(m_searchFilter) == std::string::npos)
            continue;

        float indent = static_cast<float>(entity.depth) * 16.f;

        if (entity.selected) {
            context.fillRect({m_bounds.x, y, contentW, kRowH},
                             Theme::ColorToken::Selection);
        }

        char buf[64];
        if (entity.name.empty())
            std::snprintf(buf, sizeof(buf), "Entity #%d", entity.id);
        else
            std::snprintf(buf, sizeof(buf), "%s (#%d)", entity.name.c_str(), entity.id);

        context.drawText({left + indent, y + 2.f, contentW - 16.f - indent, 14.f},
                         buf, 0,
                         entity.selected ? Theme::ColorToken::Accent
                                         : Theme::ColorToken::Text);
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

bool HierarchyPanel::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (!rectContains(m_bounds, context.mousePosition())) {
        m_thumbDragging = false;
        return false;
    }

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
                    float ratio = dy / track;
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
            m_dragStartY   = mouse.y;
            m_dragStartOff = m_scrollOffset;
            context.capturePointer(static_cast<NF::UI::AtlasUI::IPanel*>(this));
            return true;
        }

        // Click on scrollbar track (page scroll)
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

    // ── Entity row click (select) ─────────────────────────────────
    if (context.primaryPressed()) {
        float y = (m_bounds.y + kHeaderH) - m_scrollOffset;
        if (!m_searchFilter.empty()) y += 18.f;

        float contentW = m_bounds.w - (scroll ? kScrollBarW : 0.f);

        for (const auto& entity : m_entities) {
            if (!m_searchFilter.empty() &&
                entity.name.find(m_searchFilter) == std::string::npos)
                continue;

            NF::Rect rowR = {m_bounds.x, y, contentW, kRowH};
            if (rectContains(rowR, mouse)) {
                if (m_onSelect) m_onSelect(entity.id);
                return true;
            }
            y += kRowH;
        }
    }

    return false;
}

} // namespace NF::UI::AtlasUI
