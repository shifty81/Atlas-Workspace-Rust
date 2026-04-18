#include "NF/UI/AtlasUI/Panels/InspectorPanel.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <algorithm>
#include <cstdio>

namespace NF::UI::AtlasUI {

// ── rebuildGrid ──────────────────────────────────────────────────────────────

void InspectorPanel::rebuildGrid() {
    m_grid->clear();
    if (m_selectedEntityId < 0) return;

    // Transform group
    PropertyItem xformGroup;
    xformGroup.name     = "Transform";
    xformGroup.expanded = true;
    char buf[32];
    auto makeFloat = [&](const char* n, float v) {
        std::snprintf(buf, sizeof(buf), "%.3f", v);
        PropertyItem item;
        item.name     = n;
        item.value    = PropertyValue(std::string(buf));
        item.readOnly = false;
        return item;
    };
    xformGroup.children.push_back(makeFloat("X", m_transformX));
    xformGroup.children.push_back(makeFloat("Y", m_transformY));
    xformGroup.children.push_back(makeFloat("Z", m_transformZ));
    m_grid->addItem(std::move(xformGroup));

    // Custom properties group
    if (!m_properties.empty()) {
        PropertyItem propsGroup;
        propsGroup.name     = "Properties";
        propsGroup.expanded = true;
        for (const auto& p : m_properties) {
            PropertyItem item;
            item.name     = p.label;
            item.value    = PropertyValue(p.value);
            item.readOnly = false;
            propsGroup.children.push_back(std::move(item));
        }
        m_grid->addItem(std::move(propsGroup));
    }
}

// ── paint ────────────────────────────────────────────────────────────────────

void InspectorPanel::paint(IPaintContext& context) {
    if (!m_visible) return;

    // Panel background
    context.fillRect(m_bounds, Theme::ColorToken::Surface);
    context.drawRect(m_bounds, Theme::ColorToken::Border);

    // Title bar
    NF::Rect hdr = {m_bounds.x, m_bounds.y, m_bounds.w, 22.f};
    context.fillRect(hdr, Theme::ColorToken::SurfaceAlt);
    context.drawText({hdr.x + 8.f, hdr.y + 4.f, hdr.w - 16.f, 14.f},
                     m_title, 0, Theme::ColorToken::Text);

    float y           = m_bounds.y + 26.f;
    const float left  = m_bounds.x + 8.f;
    const float contentW = m_bounds.w - 16.f - kScrollW;

    if (m_selectedEntityId >= 0) {
        // Entity ID row
        char idBuf[32];
        std::snprintf(idBuf, sizeof(idBuf), "Entity #%d", m_selectedEntityId);
        context.drawText({left, y, contentW, 16.f}, idBuf, 0, Theme::ColorToken::Text);
        y += 20.f;

        // Separator
        context.fillRect({left, y, contentW, 1.f}, Theme::ColorToken::Border);
        y += Theme::Spacing::Small;

        // Transform header
        context.drawText({left, y, contentW, 16.f}, "Transform", 0,
                         Theme::ColorToken::Accent);

        // Clip the PropertyGrid to the content viewport and offset by scroll
        float vpH  = viewportHeight();
        float vpY  = m_bounds.y + kHeaderOff;
        context.pushClip({m_bounds.x, vpY, m_bounds.w - kScrollW, vpH});

        // Temporarily shift the grid's arrangement to account for scroll
        NF::Rect gridBounds = m_grid->bounds();
        NF::Rect scrolled   = gridBounds;
        scrolled.y         -= m_scrollOffset;
        m_grid->arrange(scrolled);
        m_grid->paint(context);
        // Restore original position
        m_grid->arrange(gridBounds);

        context.popClip();

        // Scrollbar
        float ctH = contentHeight();
        if (ctH > vpH) {
            float trackX = m_bounds.x + m_bounds.w - kScrollW;
            context.fillRect({trackX, vpY, kScrollW, vpH},
                             Theme::ColorToken::SurfaceAlt);
            context.fillRect({trackX + 1.f, thumbY(), kScrollW - 2.f, thumbHeight()},
                             Theme::ColorToken::Border);
        }
    } else {
        context.drawText({left, y, contentW, 16.f}, "No entity selected", 0,
                         Theme::ColorToken::TextMuted);
    }
}

// ── handleInput ──────────────────────────────────────────────────────────────

bool InspectorPanel::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    if (m_selectedEntityId < 0) return false;
    if (!rectContains(m_bounds, context.mousePosition())) return false;

    NF::Vec2 mouse = context.mousePosition();
    float vpH = viewportHeight();
    float ctH = contentHeight();

    // ── Scrollbar thumb drag ──────────────────────────────────────
    if (ctH > vpH) {
        float trackX = m_bounds.x + m_bounds.w - kScrollW;
        float trackY = m_bounds.y + kHeaderOff;

        if (m_thumbDragging) {
            if (context.primaryDown()) {
                float dy    = mouse.y - m_dragStartY;
                float track = vpH - thumbHeight();
                if (track > 0.f) {
                    float ratio  = dy / track;
                    m_scrollOffset = std::max(0.f,
                        std::min(m_dragStartOff + ratio * maxScroll(), maxScroll()));
                }
                return true;
            }
            m_thumbDragging = false;
            context.releasePointer(static_cast<NF::UI::AtlasUI::IPanel*>(this));
        }

        NF::Rect thumbR = {trackX + 1.f, thumbY(), kScrollW - 2.f, thumbHeight()};
        if (rectContains(thumbR, mouse) && context.primaryDown()) {
            m_thumbDragging = true;
            m_dragStartY    = mouse.y;
            m_dragStartOff  = m_scrollOffset;
            context.capturePointer(static_cast<NF::UI::AtlasUI::IPanel*>(this));
            return true;
        }

        NF::Rect trackR = {trackX, trackY, kScrollW, vpH};
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

    // ── Forward to property grid (offset mouse by scroll for hit-testing) ────
    return m_grid->handleInput(context);
}

} // namespace NF::UI::AtlasUI
