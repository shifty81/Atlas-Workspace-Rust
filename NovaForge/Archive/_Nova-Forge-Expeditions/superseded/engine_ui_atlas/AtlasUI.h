#pragma once
#include "AtlasUITheme.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::ui::atlas {

// ============================================================
// Atlas UI — Immediate-mode, GPU-batched UI framework
// ============================================================
// Designed to be self-contained (zero external dependencies
// beyond OpenGL 3.3) and usable in any Atlas Engine project.
//
// Usage:
//   AtlasUIContext ctx;
//   ctx.BeginFrame(mouseX, mouseY, mouseDown, viewW, viewH);
//   // ... widget calls ...
//   ctx.EndFrame();          // finalize draw commands
//   ctx.GetDrawCommands();   // retrieve batched commands
// ============================================================

/// Rectangle used for layout and hit-testing.
struct AtlasRect {
    float x = 0, y = 0, w = 0, h = 0;

    bool Contains(float px, float py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

/// Vertex format for GPU-batched rendering.
/// Position (x,y), texcoord (u,v), colour (r,g,b,a as uint8).
struct AtlasVertex {
    float x = 0, y = 0;
    float u = 0, v = 0;
    uint8_t r = 255, g = 255, b = 255, a = 255;
};

/// A single batched draw command representing a contiguous run
/// of vertices sharing the same texture.
struct AtlasDrawCmd {
    uint32_t textureId   = 0;   // 0 = untextured
    uint32_t vertexOffset = 0;
    uint32_t vertexCount  = 0;
    AtlasRect clipRect    = {};
};

/// Keyboard modifier state.
struct AtlasKeyState {
    bool shift = false;
    bool ctrl  = false;
    bool alt   = false;
};

/// Input state snapshot provided each frame.
struct AtlasInputState {
    float mouseX       = 0;
    float mouseY       = 0;
    bool  mouseDown    = false;
    bool  mouseClicked = false;   // true on the frame mouse went down
    bool  mouseUp      = false;   // true on the frame mouse went up
    float scrollDelta  = 0;
    AtlasKeyState keys;

    // Text input buffer (UTF-8 characters typed this frame).
    std::string textInput;
    bool backspace = false;
    bool enterKey  = false;
};

/// Immediate-mode UI context.  Create one per viewport and call
/// BeginFrame / EndFrame each frame.  All widget calls between
/// Begin/EndFrame produce batched draw commands that can be
/// rendered with a single GPU pass.
class AtlasUIContext {
public:
    AtlasUIContext();
    ~AtlasUIContext();

    // ---- Frame lifecycle ----
    void BeginFrame(const AtlasInputState& input, float viewW, float viewH);
    void EndFrame();

    // ---- Theme ----
    void SetTheme(const AtlasTheme& theme);
    const AtlasTheme& GetTheme() const;

    // ---- Draw output ----
    const std::vector<AtlasVertex>& GetVertices() const;
    const std::vector<AtlasDrawCmd>& GetDrawCommands() const;

    // ---- Viewport ----
    float ViewWidth()  const { return m_viewW; }
    float ViewHeight() const { return m_viewH; }

    // ---- Input state ----
    const AtlasInputState& Input() const { return m_input; }

    // ---- Immediate-mode ID system ----
    /// Generate a unique widget ID from a string label.
    uint32_t GetID(const std::string& label) const;

    /// Active/hot widget tracking for interaction state.
    uint32_t HotWidget()    const { return m_hotWidget; }
    uint32_t ActiveWidget() const { return m_activeWidget; }
    void SetHotWidget(uint32_t id);
    void SetActiveWidget(uint32_t id);
    void ClearActiveWidget();

    // ---- Primitive drawing (used internally by widgets) ----
    void AddFilledRect(const AtlasRect& rect, const AtlasColor& color);
    void AddBorderRect(const AtlasRect& rect, const AtlasColor& color, int32_t thickness = 1);
    void AddText(const AtlasRect& rect, const std::string& text, const AtlasColor& color);

    // ---- Layout cursor (simple top-to-bottom layout) ----
    float CursorX() const { return m_cursorX; }
    float CursorY() const { return m_cursorY; }
    void SetCursor(float x, float y);
    void AdvanceCursor(float height);

    // ---- Stats ----
    size_t VertexCount() const { return m_vertices.size(); }
    size_t DrawCommandCount() const { return m_drawCmds.size(); }

private:
    AtlasTheme m_theme;
    AtlasInputState m_input;
    float m_viewW = 0;
    float m_viewH = 0;

    // Vertex buffer + draw command list
    std::vector<AtlasVertex> m_vertices;
    std::vector<AtlasDrawCmd> m_drawCmds;

    // Widget interaction state
    uint32_t m_hotWidget    = 0;
    uint32_t m_activeWidget = 0;

    // Layout cursor
    float m_cursorX = 0;
    float m_cursorY = 0;

    // Frame state
    bool m_inFrame = false;
};

} // namespace atlas::ui::atlas
