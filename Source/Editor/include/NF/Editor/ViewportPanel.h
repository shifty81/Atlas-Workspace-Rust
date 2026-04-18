#pragma once
// NF::Editor — Viewport panel + camera controller
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>
#include "NF/Editor/EditorPanel.h"

namespace NF {

enum class RenderMode : uint8_t { Shaded, Wireframe, Unlit };
enum class ToolMode   : uint8_t { Select, Move, Rotate, Scale, Paint, Erase };

// Controls a fly-camera inside the viewport when the right mouse button is held.
// Uses the same yaw/pitch math as FPSCamera but is self-contained here so the
// Editor module does not depend on Game logic directly.
struct ViewportCameraController {
    float moveSpeed      = 10.f;   // world units per second
    float mouseSensitivity = 0.15f; // degrees per pixel
    float sprintMultiplier = 3.f;  // held Shift boost

    // Call each frame; mutates cameraPos, yaw, pitch.
    // Only moves the camera when Mouse2 (right button) is held.
    void update(float dt, const InputSystem& input,
                Vec3& cameraPos, float& yaw, float& pitch) {
        bool rmb = input.isKeyDown(KeyCode::Mouse2);
        if (!rmb) {
            m_active = false;
            return;
        }
        m_active = true;

        // -- Mouse look --
        float dx = input.state().mouse.deltaX * mouseSensitivity;
        float dy = input.state().mouse.deltaY * mouseSensitivity;
        yaw   += dx;
        pitch -= dy;  // invert Y so dragging up looks up
        if (pitch >  89.f) pitch =  89.f;
        if (pitch < -89.f) pitch = -89.f;

        // -- Rebuild forward/right vectors from yaw & pitch --
        float yawRad   = yaw   * (3.14159265f / 180.f);
        float pitchRad = pitch * (3.14159265f / 180.f);
        Vec3 fwd{
            std::cos(yawRad) * std::cos(pitchRad),
            std::sin(pitchRad),
            std::sin(yawRad) * std::cos(pitchRad)
        };
        fwd = fwd.normalized();
        Vec3 worldUp{0.f, 1.f, 0.f};
        Vec3 right = fwd.cross(worldUp).normalized();

        // -- WASD movement --
        float speed = moveSpeed;
        if (input.isKeyDown(KeyCode::LShift) || input.isKeyDown(KeyCode::RShift))
            speed *= sprintMultiplier;

        Vec3 move{0.f, 0.f, 0.f};
        if (input.isKeyDown(KeyCode::W)) move = move + fwd  *  speed * dt;
        if (input.isKeyDown(KeyCode::S)) move = move + fwd  * -speed * dt;
        if (input.isKeyDown(KeyCode::D)) move = move + right *  speed * dt;
        if (input.isKeyDown(KeyCode::A)) move = move + right * -speed * dt;
        // Q/E for up/down in world space
        if (input.isKeyDown(KeyCode::E)) move = move + worldUp *  speed * dt;
        if (input.isKeyDown(KeyCode::Q)) move = move + worldUp * -speed * dt;

        cameraPos = cameraPos + move;
    }

    [[nodiscard]] bool isActive() const { return m_active; }

private:
    bool m_active = false;
};

class ViewportPanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Center; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.viewportBackground);
        // Grid overlay
        if (m_gridEnabled) {
            float step = 40.f;
            for (float gx = bounds.x; gx < bounds.x + bounds.w; gx += step)
                ui.drawRect({gx, bounds.y, 1.f, bounds.h}, theme.gridColor);
            for (float gy = bounds.y; gy < bounds.y + bounds.h; gy += step)
                ui.drawRect({bounds.x, gy, bounds.w, 1.f}, theme.gridColor);
        }
        // Camera info overlay
        char camText[64];
        std::snprintf(camText, sizeof(camText), "Cam: %.1f, %.1f, %.1f",
                      m_cameraPos.x, m_cameraPos.y, m_cameraPos.z);
        ui.drawText(bounds.x + 8.f, bounds.y + bounds.h - 20.f, camText, 0x888888FF);
        // Tool mode indicator
        const char* modes[] = {"Select", "Move", "Rotate", "Scale", "Paint", "Erase"};
        ui.drawText(bounds.x + 8.f, bounds.y + 8.f,
                    modes[static_cast<int>(m_toolMode)], 0xAAAAAAFF);
        // Render mode indicator
        const char* rmodes[] = {"Shaded", "Wireframe", "Unlit"};
        ui.drawText(bounds.x + bounds.w - 80.f, bounds.y + 8.f,
                    rmodes[static_cast<int>(m_renderMode)], 0x888888FF);
        // Center label
        const char* vpLabel = "[ 3D Viewport ]";
        float labelW = 15.f * 8.f;
        ui.drawText(bounds.x + (bounds.w - labelW) * 0.5f,
                    bounds.y + (bounds.h - 14.f) * 0.5f, vpLabel, 0x444444FF);
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        (void)bounds;
        // Viewport-specific UI overlays rendered through widget system
        ctx.beginHorizontal();
        if (ctx.button("Grid", 50.f)) { m_gridEnabled = !m_gridEnabled; }
        ctx.endHorizontal();
    }

    // Called by WorkspaceFrameController each frame.
    // Activates fly-camera only while right mouse button is held.
    void updateCamera(float dt, const InputSystem& input) {
        m_camController.update(dt, input, m_cameraPos, m_cameraYaw, m_cameraPitch);
    }

    [[nodiscard]] Vec3 cameraPosition() const { return m_cameraPos; }
    void setCameraPosition(Vec3 pos) { m_cameraPos = pos; }

    [[nodiscard]] float cameraYaw()   const { return m_cameraYaw; }
    [[nodiscard]] float cameraPitch() const { return m_cameraPitch; }
    void setCameraYaw(float y)   { m_cameraYaw   = y; }
    void setCameraPitch(float p) { m_cameraPitch = p; }

    [[nodiscard]] bool isFlyCamActive() const { return m_camController.isActive(); }

    [[nodiscard]] ViewportCameraController& cameraController() { return m_camController; }
    [[nodiscard]] const ViewportCameraController& cameraController() const { return m_camController; }

    [[nodiscard]] float cameraZoom() const { return m_cameraZoom; }
    void setCameraZoom(float z) { m_cameraZoom = z; }

    [[nodiscard]] bool gridEnabled() const { return m_gridEnabled; }
    void setGridEnabled(bool e) { m_gridEnabled = e; }

    [[nodiscard]] RenderMode renderMode() const { return m_renderMode; }
    void setRenderMode(RenderMode m) { m_renderMode = m; }

    [[nodiscard]] ToolMode toolMode() const { return m_toolMode; }
    void setToolMode(ToolMode m) { m_toolMode = m; }

private:
    std::string m_name = "Viewport";
    Vec3  m_cameraPos{0.f, 0.f, 0.f};
    float m_cameraYaw   = -90.f;  // degrees; -90 = look down -Z
    float m_cameraPitch =   0.f;  // degrees
    float m_cameraZoom  =   1.f;
    bool  m_gridEnabled = true;
    RenderMode m_renderMode = RenderMode::Shaded;
    ToolMode   m_toolMode   = ToolMode::Select;
    ViewportCameraController m_camController;
};

// ── InspectorPanel ───────────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::InspectorPanel instead (U1).


} // namespace NF
