#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <cstdint>
#include <functional>
#include <string>

namespace NF::UI::AtlasUI {

/// Render modes for the viewport.
enum class ViewportRenderMode : uint8_t { Shaded, Wireframe, Unlit };

/// Tool modes for the viewport.
enum class ViewportToolMode : uint8_t { Select, Move, Rotate, Scale, Paint, Erase };

/// AtlasUI ViewportPanel — 3D viewport shell with camera info overlay.
/// Replaces the legacy NF::Editor::ViewportPanel for the AtlasUI framework.
/// This is a shell implementation; actual 3D rendering is handled by the
/// render backend. This panel provides the UI chrome and overlays.
class ViewportPanel final : public PanelBase {
public:
    ViewportPanel()
        : PanelBase("atlas.viewport", "Viewport") {}

    void paint(IPaintContext& context) override;

    // ── Scene texture (color attachment from render pass) ────────────────────
    // When non-zero the panel blits the texture via drawImage instead of
    // drawing the placeholder grid.  Set by ViewportFrameLoop each frame.
    void setColorAttachment(uint32_t textureId) { m_colorAttachment = textureId; }
    [[nodiscard]] uint32_t colorAttachment() const { return m_colorAttachment; }

    // ── Viewport slot handle ─────────────────────────────────────────────────
    // Associates this panel with a ViewportHostRegistry slot so the frame loop
    // can route scene content and resize events correctly.
    void setViewportHandle(uint32_t handle) { m_viewportHandle = handle; }
    [[nodiscard]] uint32_t viewportHandle() const { return m_viewportHandle; }

    // ── Resize callback ──────────────────────────────────────────────────────
    // Fired by paint() the first time the panel is rendered at a new size.
    // Signature: (x, y, width, height) in screen pixels.
    // Use to call ViewportHostRegistry::updateBounds() and trigger FBO resize.
    using ResizeCallback = std::function<void(float x, float y, float w, float h)>;
    void setResizeCallback(ResizeCallback cb) { m_resizeCallback = std::move(cb); }
    [[nodiscard]] bool hasResizeCallback() const { return static_cast<bool>(m_resizeCallback); }

    // ── Camera overlay ────────────────────────────────────────────────────────
    void setCameraPosition(float x, float y, float z) {
        m_camX = x; m_camY = y; m_camZ = z;
    }
    [[nodiscard]] float cameraX() const { return m_camX; }
    [[nodiscard]] float cameraY() const { return m_camY; }
    [[nodiscard]] float cameraZ() const { return m_camZ; }

    void setGridEnabled(bool enabled) { m_gridEnabled = enabled; }
    [[nodiscard]] bool gridEnabled() const { return m_gridEnabled; }

    void setRenderMode(ViewportRenderMode mode) { m_renderMode = mode; }
    [[nodiscard]] ViewportRenderMode renderMode() const { return m_renderMode; }

    void setToolMode(ViewportToolMode mode) { m_toolMode = mode; }
    [[nodiscard]] ViewportToolMode toolMode() const { return m_toolMode; }

private:
    uint32_t m_colorAttachment = 0;
    uint32_t m_viewportHandle  = 0;  // kInvalidViewportHandle = 0
    ResizeCallback m_resizeCallback;
    NF::Rect m_lastBounds{};

    float m_camX = 0.f, m_camY = 0.f, m_camZ = 0.f;
    bool m_gridEnabled = true;
    ViewportRenderMode m_renderMode = ViewportRenderMode::Shaded;
    ViewportToolMode m_toolMode = ViewportToolMode::Select;
};

} // namespace NF::UI::AtlasUI
