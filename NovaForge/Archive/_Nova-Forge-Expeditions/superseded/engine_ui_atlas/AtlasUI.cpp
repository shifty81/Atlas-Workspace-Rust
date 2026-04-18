#include "AtlasUI.h"
#include <algorithm>
#include <functional>

namespace atlas::ui::atlas {

// ---- Construction / destruction ----

AtlasUIContext::AtlasUIContext() {
    m_theme = AtlasTheme::Teal();
}

AtlasUIContext::~AtlasUIContext() = default;

// ---- Frame lifecycle ----

void AtlasUIContext::BeginFrame(const AtlasInputState& input,
                                float viewW, float viewH) {
    m_input  = input;
    m_viewW  = viewW;
    m_viewH  = viewH;
    m_inFrame = true;

    // Reset draw state for the new frame.
    m_vertices.clear();
    m_drawCmds.clear();

    // Reset layout cursor to top-left with padding.
    m_cursorX = static_cast<float>(m_theme.paddingMedium);
    m_cursorY = static_cast<float>(m_theme.paddingMedium);

    // Reset hot widget — it gets re-evaluated during widget calls.
    m_hotWidget = 0;
}

void AtlasUIContext::EndFrame() {
    // If mouse is not down, clear the active widget.
    if (!m_input.mouseDown) {
        m_activeWidget = 0;
    }
    m_inFrame = false;
}

// ---- Theme ----

void AtlasUIContext::SetTheme(const AtlasTheme& theme) {
    m_theme = theme;
}

const AtlasTheme& AtlasUIContext::GetTheme() const {
    return m_theme;
}

// ---- Draw output ----

const std::vector<AtlasVertex>& AtlasUIContext::GetVertices() const {
    return m_vertices;
}

const std::vector<AtlasDrawCmd>& AtlasUIContext::GetDrawCommands() const {
    return m_drawCmds;
}

// ---- ID system ----

uint32_t AtlasUIContext::GetID(const std::string& label) const {
    // FNV-1a hash for quick ID generation from label strings.
    uint32_t hash = 2166136261u;
    for (char c : label) {
        hash ^= static_cast<uint32_t>(static_cast<unsigned char>(c));
        hash *= 16777619u;
    }
    // Avoid 0 which means "no widget".
    return hash == 0 ? 1 : hash;
}

void AtlasUIContext::SetHotWidget(uint32_t id)    { m_hotWidget = id; }
void AtlasUIContext::SetActiveWidget(uint32_t id)  { m_activeWidget = id; }
void AtlasUIContext::ClearActiveWidget()            { m_activeWidget = 0; }

// ---- Primitives ----

void AtlasUIContext::AddFilledRect(const AtlasRect& rect,
                                   const AtlasColor& color) {
    uint32_t base = static_cast<uint32_t>(m_vertices.size());

    // Two-triangle quad (4 vertices, 6 indices packed as a triangle strip
    // stored as 6 vertices for simplicity — no index buffer needed).
    auto push = [&](float px, float py) {
        m_vertices.push_back({px, py, 0, 0, color.r, color.g, color.b, color.a});
    };
    float x0 = rect.x, y0 = rect.y;
    float x1 = rect.x + rect.w, y1 = rect.y + rect.h;

    push(x0, y0); push(x1, y0); push(x1, y1);
    push(x0, y0); push(x1, y1); push(x0, y1);

    AtlasDrawCmd cmd;
    cmd.textureId   = 0;
    cmd.vertexOffset = base;
    cmd.vertexCount  = 6;
    cmd.clipRect     = {0, 0, m_viewW, m_viewH};
    m_drawCmds.push_back(cmd);
}

void AtlasUIContext::AddBorderRect(const AtlasRect& rect,
                                   const AtlasColor& color,
                                   int32_t thickness) {
    float t = static_cast<float>(thickness);
    // Top
    AddFilledRect({rect.x, rect.y, rect.w, t}, color);
    // Bottom
    AddFilledRect({rect.x, rect.y + rect.h - t, rect.w, t}, color);
    // Left
    AddFilledRect({rect.x, rect.y + t, t, rect.h - 2 * t}, color);
    // Right
    AddFilledRect({rect.x + rect.w - t, rect.y + t, t, rect.h - 2 * t}, color);
}

void AtlasUIContext::AddText(const AtlasRect& rect,
                             const std::string& /*text*/,
                             const AtlasColor& color) {
    // Text rendering is abstracted; in a real GPU implementation this
    // would generate glyph quads.  For the batching layer we emit a
    // single placeholder quad covering the text bounds.
    AddFilledRect(rect, color);
}

// ---- Layout cursor ----

void AtlasUIContext::SetCursor(float x, float y) {
    m_cursorX = x;
    m_cursorY = y;
}

void AtlasUIContext::AdvanceCursor(float height) {
    m_cursorY += height + static_cast<float>(m_theme.itemSpacing);
}

} // namespace atlas::ui::atlas
