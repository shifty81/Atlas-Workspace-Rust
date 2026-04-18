#pragma once
// NovaForge::NovaForgePreviewRuntime — lightweight runtime for editor preview.
//
// Implements NF::IViewportSceneProvider so SceneEditorTool can delegate scene
// rendering to the preview runtime without a full game-boot sequence.
//
// Fly-camera state is maintained here and updated via processCameraInput().
// Gizmo state is derived from the world's selected entity transform.
// Inspector data and hierarchy order are also provided by this runtime.
//
// Phase D.1 — NovaForge Preview Runtime Bridge

#include "NF/Workspace/IViewportSceneProvider.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewWorld.h"
#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace NovaForge {

// ── FlyCameraState ────────────────────────────────────────────────────────────

struct FlyCameraState {
    float x           =   0.f;
    float y           =   2.f;
    float z           =  10.f;
    float yaw         = -90.f; ///< degrees; -90 looks down -Z
    float pitch       =   0.f; ///< degrees; clamped ±89
    float speed       =   5.f; ///< units per second
    float sensitivity =   0.1f;
};

// ── CameraInput ───────────────────────────────────────────────────────────────
// Simplified per-frame input state for fly-camera movement.

struct CameraInput {
    bool  moveForward    = false;
    bool  moveBack       = false;
    bool  moveLeft       = false;
    bool  moveRight      = false;
    bool  moveUp         = false;
    bool  moveDown       = false;
    float mouseDeltaX    = 0.f;
    float mouseDeltaY    = 0.f;
    bool  mouseButtonHeld = false; ///< right-mouse held to enable look
};

// ── GizmoMode / GizmoState ────────────────────────────────────────────────────

enum class GizmoMode : uint8_t {
    Translate,
    Rotate,
    Scale,
};

struct GizmoState {
    bool        visible  = false;
    GizmoMode   mode     = GizmoMode::Translate;
    EntityId    entityId = kInvalidEntityId;
    PreviewVec3 position;
};

// ── NovaForgePreviewRuntime ───────────────────────────────────────────────────

class NovaForgePreviewRuntime : public NF::IViewportSceneProvider {
public:
    NovaForgePreviewRuntime()  = default;
    ~NovaForgePreviewRuntime() override = default;

    // ── IViewportSceneProvider ────────────────────────────────────────────

    NF::ViewportSceneState provideScene(NF::ViewportHandle       /*handle*/,
                                        const NF::ViewportSlot&  /*slot*/) override {
        NF::ViewportSceneState st;
        const bool hasEntities = m_world.entityCount() > 0;
        // Content is available whenever there are entities in the preview world,
        // whether the runtime is running (play mode) or paused (static preview).
        st.hasContent     = hasEntities;
        st.entityCount    = m_world.entityCount();
        // The fly-camera is authoritative (overrides slot camera) only while running.
        st.overrideCamera = m_running;
        st.clearColor     = m_skyColor;

        // Populate entity proxies so the software renderer can project them.
        // Each visible entity contributes one proxy carrying its world position
        // and selection state.  Scale is used as a rough half-extent hint.
        st.entities.reserve(m_world.entityCount());
        for (const auto& e : m_world.entities()) {
            if (!e.visible) continue;
            NF::ViewportEntityProxy proxy;
            proxy.x        = e.transform.position.x;
            proxy.y        = e.transform.position.y;
            proxy.z        = e.transform.position.z;
            proxy.halfW    = e.transform.scale.x * 0.5f;
            proxy.halfH    = e.transform.scale.y * 0.5f;
            proxy.halfD    = e.transform.scale.z * 0.5f;
            proxy.selected = (e.id == m_world.selectedEntityId());
            proxy.label    = e.name.c_str();
            st.entities.push_back(proxy);
        }
        return st;
    }

    // ── Document binding ──────────────────────────────────────────────────
    // Stub implementations for the binder layer to call.
    // Full implementations will push document state into the preview world.

    void bindWorldDocument(const std::string& /*worldId*/)   { m_previewDirty = true; }
    void bindLevelDocument(const std::string& /*levelId*/)   { m_previewDirty = true; }
    void rebuildFromDocument()                                { m_previewDirty = false; }

    struct PreviewTransform {
        float px = 0.f, py = 0.f, pz = 0.f;
        float rx = 0.f, ry = 0.f, rz = 0.f;
        float sx = 1.f, sy = 1.f, sz = 1.f;
    };

    void applyEntityChange(EntityId /*id*/, const PreviewTransform& /*t*/) {}
    void applySelection(EntityId /*id*/)                                     {}

    // ── Scene world ───────────────────────────────────────────────────────

    [[nodiscard]]       NovaForgePreviewWorld& world()       { return m_world; }
    [[nodiscard]] const NovaForgePreviewWorld& world() const { return m_world; }

    // ── Tick ──────────────────────────────────────────────────────────────

    void tick(float dt) {
        if (!m_running) return;
        m_elapsed += dt;
    }

    // ── Fly-camera ────────────────────────────────────────────────────────

    void processCameraInput(const CameraInput& input, float dt) {
        if (input.mouseButtonHeld) {
            m_camera.yaw   += input.mouseDeltaX * m_camera.sensitivity;
            m_camera.pitch -= input.mouseDeltaY * m_camera.sensitivity;
            // Clamp pitch
            if (m_camera.pitch >  89.f) m_camera.pitch =  89.f;
            if (m_camera.pitch < -89.f) m_camera.pitch = -89.f;
        }

        // Compute forward direction from yaw/pitch
        const float yawR   = m_camera.yaw   * (3.14159265f / 180.f);
        const float pitchR = m_camera.pitch * (3.14159265f / 180.f);

        const float fwdX = std::cos(pitchR) * std::cos(yawR);
        const float fwdY = std::sin(pitchR);
        const float fwdZ = std::cos(pitchR) * std::sin(yawR);

        // Right = forward × world-up (0,1,0)
        const float rightX =  std::cos(yawR);
        const float rightZ = -std::sin(yawR);

        const float move = m_camera.speed * dt;

        if (input.moveForward) { m_camera.x += fwdX * move; m_camera.y += fwdY * move; m_camera.z += fwdZ * move; }
        if (input.moveBack)    { m_camera.x -= fwdX * move; m_camera.y -= fwdY * move; m_camera.z -= fwdZ * move; }
        if (input.moveLeft)    { m_camera.x -= rightX * move; m_camera.z -= rightZ * move; }
        if (input.moveRight)   { m_camera.x += rightX * move; m_camera.z += rightZ * move; }
        if (input.moveUp)      { m_camera.y += move; }
        if (input.moveDown)    { m_camera.y -= move; }
    }

    [[nodiscard]] const FlyCameraState& cameraState() const          { return m_camera; }
    void setCameraState(const FlyCameraState& s)                      { m_camera = s; }
    void setCameraSpeed(float speed)                                   { m_camera.speed = speed; }
    void setCameraSensitivity(float s)                                 { m_camera.sensitivity = s; }

    // ── Gizmo ─────────────────────────────────────────────────────────────

    [[nodiscard]] GizmoState gizmoState() const {
        GizmoState gs;
        gs.mode     = m_gizmoMode;
        if (const PreviewEntity* e = m_world.selectedEntity()) {
            gs.visible   = true;
            gs.entityId  = e->id;
            gs.position  = e->transform.position;
        }
        return gs;
    }

    void setGizmoMode(GizmoMode mode) { m_gizmoMode = mode; }
    [[nodiscard]] GizmoMode gizmoMode() const { return m_gizmoMode; }

    // ── Runtime lifecycle ─────────────────────────────────────────────────

    void  start()                                    { m_running = true; }
    void  stop()                                     { m_running = false; }
    [[nodiscard]] bool  isRunning()     const        { return m_running; }
    [[nodiscard]] float elapsedSeconds() const       { return m_elapsed; }

    // ── Inspector data ────────────────────────────────────────────────────
    // Returns flat property→value pairs for the selected entity.

    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
    selectedEntityProperties() const {
        std::vector<std::pair<std::string, std::string>> props;
        const PreviewEntity* e = m_world.selectedEntity();
        if (!e) return props;

        auto fmtF = [](float v) -> std::string {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.3f", static_cast<double>(v));
            return buf;
        };

        props.push_back({"name",        e->name});
        props.push_back({"position.x",  fmtF(e->transform.position.x)});
        props.push_back({"position.y",  fmtF(e->transform.position.y)});
        props.push_back({"position.z",  fmtF(e->transform.position.z)});
        props.push_back({"rotation.x",  fmtF(e->transform.rotation.x)});
        props.push_back({"rotation.y",  fmtF(e->transform.rotation.y)});
        props.push_back({"rotation.z",  fmtF(e->transform.rotation.z)});
        props.push_back({"scale.x",     fmtF(e->transform.scale.x)});
        props.push_back({"scale.y",     fmtF(e->transform.scale.y)});
        props.push_back({"scale.z",     fmtF(e->transform.scale.z)});
        props.push_back({"mesh",        e->meshTag});
        props.push_back({"material",    e->materialTag});
        props.push_back({"visible",     e->visible ? "true" : "false"});
        return props;
    }

    // ── Entity hierarchy ──────────────────────────────────────────────────
    // Returns entity IDs ordered so parent entities appear before children.
    // Root entities (parentId == kInvalidEntityId) come first.

    [[nodiscard]] std::vector<EntityId> hierarchyOrder() const {
        std::vector<EntityId> result;
        result.reserve(m_world.entityCount());

        // Collect roots first
        for (const auto& e : m_world.entities())
            if (e.parentId == kInvalidEntityId)
                result.push_back(e.id);

        // BFS over children
        size_t head = 0;
        while (head < result.size()) {
            EntityId parent = result[head++];
            for (const auto& e : m_world.entities())
                if (e.parentId == parent)
                    result.push_back(e.id);
        }
        return result;
    }

    void setSkyColor(uint32_t rrggbbaa) { m_skyColor = rrggbbaa; }
    [[nodiscard]] uint32_t skyColor() const { return m_skyColor; }

private:
    NovaForgePreviewWorld m_world;
    FlyCameraState        m_camera;
    GizmoMode             m_gizmoMode  = GizmoMode::Translate;
    bool                  m_running    = false;
    float                 m_elapsed    = 0.f;
    uint32_t              m_skyColor   = 0x1A1A2EFFu; ///< clear color (RRGGBBAA)
    bool                  m_previewDirty = false;
};

} // namespace NovaForge
