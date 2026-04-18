#pragma once
// NF::UI — Custom 2D renderer (quad batching + stb_easy_font). No ImGui.
#include "NF/Core/Core.h"

namespace NF {

// Forward-declare backend so headers can be included independently.
class UIBackend;

struct UIVertex {
    Vec2 position;
    Vec2 uv;
    uint32_t color = 0xFFFFFFFF;
};

class UIRenderer {
public:
    void init() { NF_LOG_INFO("UI", "UI renderer initialized"); }
    void shutdown() { NF_LOG_INFO("UI", "UI renderer shutdown"); }

    // ── Backend management ───────────────────────────────────────
    void setBackend(UIBackend* backend) { m_backend = backend; }
    [[nodiscard]] UIBackend* backend() const { return m_backend; }

    // ── Frame lifecycle ──────────────────────────────────────────
    void beginFrame(float viewportW, float viewportH) {
        m_viewportW = viewportW;
        m_viewportH = viewportH;
        m_vertices.clear();
        m_indices.clear();
        m_quadCount = 0;
        m_textDrawCount = 0;
    }

    // ── Drawing primitives ───────────────────────────────────────

    void drawRect(const Rect& r, uint32_t color) {
        UIVertex topLeft     = {{r.x, r.y},             {0, 0}, color};
        UIVertex topRight    = {{r.x + r.w, r.y},       {1, 0}, color};
        UIVertex bottomRight = {{r.x + r.w, r.y + r.h}, {1, 1}, color};
        UIVertex bottomLeft  = {{r.x, r.y + r.h},       {0, 1}, color};

        uint32_t base = static_cast<uint32_t>(m_vertices.size());
        m_vertices.push_back(topLeft);
        m_vertices.push_back(topRight);
        m_vertices.push_back(bottomRight);
        m_vertices.push_back(bottomLeft);

        m_indices.push_back(base + 0);
        m_indices.push_back(base + 1);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 0);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 3);

        ++m_quadCount;
    }

    void drawText(float x, float y, std::string_view text, uint32_t color = 0xFFFFFFFF);

    void drawRectOutline(const Rect& r, uint32_t color, float thickness = 1.f) {
        drawRect({r.x, r.y, r.w, thickness}, color);                       // top
        drawRect({r.x, r.y + r.h - thickness, r.w, thickness}, color);     // bottom
        drawRect({r.x, r.y, thickness, r.h}, color);                       // left
        drawRect({r.x + r.w - thickness, r.y, thickness, r.h}, color);     // right
    }

    // Draw a textured image (scene render target) into the given rect.
    // textureId is a backend-specific handle (e.g. OpenGL texture name).
    // tint is applied as a color multiply (RRGGBBAA).
    // Falls back to a solid-color rect when no backend texture support is available.
    void drawImage(const Rect& r, uint32_t textureId, uint32_t tint = 0xFFFFFFFF) {
        // Track for query/testing purposes
        ++m_imageDrawCount;
        // Emit a colored rect as a fallback — real backends override via UIBackend::drawImageNative
        drawRect(r, tint);
        (void)textureId; // used by real backends
    }

    // Gradient rectangle — top→bottom colour interpolation
    void drawGradientRect(const Rect& r, uint32_t topColor, uint32_t bottomColor) {
        UIVertex tl = {{r.x, r.y},             {0, 0}, topColor};
        UIVertex tr = {{r.x + r.w, r.y},       {1, 0}, topColor};
        UIVertex br = {{r.x + r.w, r.y + r.h}, {1, 1}, bottomColor};
        UIVertex bl = {{r.x, r.y + r.h},       {0, 1}, bottomColor};

        uint32_t base = static_cast<uint32_t>(m_vertices.size());
        m_vertices.push_back(tl);
        m_vertices.push_back(tr);
        m_vertices.push_back(br);
        m_vertices.push_back(bl);

        m_indices.push_back(base + 0);
        m_indices.push_back(base + 1);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 0);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 3);

        ++m_quadCount;
    }

    // Horizontal line
    void drawLine(float x1, float y1, float x2, float y2, uint32_t color, float thickness = 1.f) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f) return;
        float nx = -dy / len * thickness * 0.5f;
        float ny =  dx / len * thickness * 0.5f;

        UIVertex v0 = {{x1 + nx, y1 + ny}, {0, 0}, color};
        UIVertex v1 = {{x1 - nx, y1 - ny}, {0, 1}, color};
        UIVertex v2 = {{x2 - nx, y2 - ny}, {1, 1}, color};
        UIVertex v3 = {{x2 + nx, y2 + ny}, {1, 0}, color};

        uint32_t base = static_cast<uint32_t>(m_vertices.size());
        m_vertices.push_back(v0);
        m_vertices.push_back(v1);
        m_vertices.push_back(v2);
        m_vertices.push_back(v3);

        m_indices.push_back(base + 0);
        m_indices.push_back(base + 1);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 0);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 3);

        ++m_quadCount;
    }

    // Circle (approximated with N segments)
    void drawCircle(float cx, float cy, float radius, uint32_t color, int segments = 16) {
        constexpr float PI = 3.14159265f;
        float angleStep = 2.f * PI / static_cast<float>(segments);

        // Centre vertex
        uint32_t centreIdx = static_cast<uint32_t>(m_vertices.size());
        m_vertices.push_back({{cx, cy}, {0.5f, 0.5f}, color});

        for (int i = 0; i <= segments; ++i) {
            float a = static_cast<float>(i) * angleStep;
            float px = cx + std::cos(a) * radius;
            float py = cy + std::sin(a) * radius;
            m_vertices.push_back({{px, py}, {0.5f + 0.5f * std::cos(a), 0.5f + 0.5f * std::sin(a)}, color});
        }

        for (int i = 0; i < segments; ++i) {
            m_indices.push_back(centreIdx);
            m_indices.push_back(centreIdx + 1 + static_cast<uint32_t>(i));
            m_indices.push_back(centreIdx + 2 + static_cast<uint32_t>(i));
        }
        ++m_quadCount;
    }

    // ── Mid-frame flush ──────────────────────────────────────────
    // Flushes all buffered geometry to the backend and clears the vertex
    // buffer.  Use this to ensure earlier draw calls (e.g. backgrounds) are
    // written to the GDI memDC before later draw calls (e.g. text) that must
    // appear on top, without ending the frame.
    void flush();

    // ── End frame — flush to backend ─────────────────────────────
    void endFrame();

    // ── Queries ──────────────────────────────────────────────────
    [[nodiscard]] const std::vector<UIVertex>& vertices() const { return m_vertices; }
    [[nodiscard]] const std::vector<uint32_t>& indices() const { return m_indices; }
    [[nodiscard]] size_t quadCount() const { return m_lastFrameQuadCount; }
    [[nodiscard]] size_t textDrawCount() const { return m_lastFrameTextCount; }
    [[nodiscard]] size_t imageDrawCount() const { return m_imageDrawCount; }
    [[nodiscard]] float viewportWidth() const { return m_viewportW; }
    [[nodiscard]] float viewportHeight() const { return m_viewportH; }

    // Text measurement helper (delegates to backend if available)
    [[nodiscard]] Vec2 measureText(std::string_view text, float fontSize = 14.f) const;

private:
    UIBackend* m_backend = nullptr;
    float m_viewportW = 0.f, m_viewportH = 0.f;
    std::vector<UIVertex> m_vertices;
    std::vector<uint32_t> m_indices;
    size_t m_quadCount = 0;
    size_t m_textDrawCount = 0;
    size_t m_imageDrawCount = 0;
    size_t m_lastFrameQuadCount = 0;
    size_t m_lastFrameTextCount = 0;
};

} // namespace NF
