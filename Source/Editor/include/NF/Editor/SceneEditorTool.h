#pragma once
// NF::Editor — SceneEditorTool: primary scene/world editing tool.
//
// Implements NF::IHostedTool for the Scene Editor, one of the ~10
// primary tools in the canonical workspace roster.
//
// The Scene Editor hosts:
//   Shared panels: viewport, outliner/hierarchy, inspector, console
//   Commands:      scene.create_entity, scene.delete_entity,
//                  scene.duplicate_entity, scene.save_scene,
//                  scene.enter_play, scene.exit_play
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for Phase 3 status.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/WorkspaceViewportManager.h"
#include <cmath>
#include <string>

namespace NF {

// ── Scene editing mode ────────────────────────────────────────────

enum class SceneEditMode : uint8_t {
    Select,    // selection / inspection
    Translate, // gizmo: move
    Rotate,    // gizmo: rotate
    Scale,     // gizmo: scale
    Paint,     // foliage / terrain paint
    Play,      // PIE — play-in-editor
};

inline const char* sceneEditModeName(SceneEditMode m) {
    switch (m) {
        case SceneEditMode::Select:    return "Select";
        case SceneEditMode::Translate: return "Translate";
        case SceneEditMode::Rotate:    return "Rotate";
        case SceneEditMode::Scale:     return "Scale";
        case SceneEditMode::Paint:     return "Paint";
        case SceneEditMode::Play:      return "Play";
    }
    return "Unknown";
}

// ── Scene Editor statistics ───────────────────────────────────────

struct SceneEditorStats {
    uint32_t entityCount      = 0; // entities in the active scene
    uint32_t selectionCount   = 0; // currently selected entities
    float    lastFrameMs      = 0.0f;
    mutable bool isDirty      = false; // unsaved changes (mutable: set from const renderToolView)
};

// ── SceneViewportCameraController ─────────────────────────────────
// Fly-camera controller for the scene viewport.
// Activates only while the right mouse button is held (Mouse2).
// Owns the camera world-space transform; caller pushes state to the
// ViewportCameraDescriptor after update() returns.

struct SceneViewportCameraController {
    // Pitch is clamped to ±kMaxPitchDegrees to prevent gimbal lock at the poles.
    static constexpr float kMaxPitchDegrees  = 89.f;
    static constexpr float kDegreesToRadians = 3.14159265f / 180.f;

    float moveSpeed         = 10.f;   // world units per second
    float mouseSensitivity  = 0.15f;  // degrees per pixel
    float sprintMultiplier  = 3.f;    // Shift-key speed boost

    /// Update camera state.  Mutates pos/yaw/pitch only while RMB is held.
    void update(float dt, const InputSystem& input,
                Vec3& pos, float& yaw, float& pitch) {
        if (!input.isKeyDown(KeyCode::Mouse2)) { m_active = false; return; }
        m_active = true;

        float dx = input.state().mouse.deltaX * mouseSensitivity;
        float dy = input.state().mouse.deltaY * mouseSensitivity;
        yaw   += dx;
        pitch -= dy;
        // Clamp pitch to ±89° to prevent gimbal lock at the poles.
        if (pitch >  kMaxPitchDegrees) pitch =  kMaxPitchDegrees;
        if (pitch < -kMaxPitchDegrees) pitch = -kMaxPitchDegrees;

        float yawRad   = yaw   * (kDegreesToRadians);
        float pitchRad = pitch * (kDegreesToRadians);
        Vec3 fwd{std::cos(yawRad) * std::cos(pitchRad),
                 std::sin(pitchRad),
                 std::sin(yawRad) * std::cos(pitchRad)};
        fwd = fwd.normalized();
        Vec3 worldUp{0.f, 1.f, 0.f};
        Vec3 right = fwd.cross(worldUp).normalized();

        float speed = moveSpeed;
        if (input.isKeyDown(KeyCode::LShift) || input.isKeyDown(KeyCode::RShift))
            speed *= sprintMultiplier;

        if (input.isKeyDown(KeyCode::W)) pos = pos + fwd   *  speed * dt;
        if (input.isKeyDown(KeyCode::S)) pos = pos + fwd   * -speed * dt;
        if (input.isKeyDown(KeyCode::D)) pos = pos + right *  speed * dt;
        if (input.isKeyDown(KeyCode::A)) pos = pos + right * -speed * dt;
        if (input.isKeyDown(KeyCode::E)) pos = pos + worldUp *  speed * dt;
        if (input.isKeyDown(KeyCode::Q)) pos = pos + worldUp * -speed * dt;
    }

    [[nodiscard]] bool isActive() const { return m_active; }

private:
    bool m_active = false;
};

// ── SceneEditorTool ───────────────────────────────────────────────

class SceneEditorTool final : public IHostedTool, public IViewportSceneProvider {
public:
    static constexpr const char* kToolId = "workspace.scene_editor";

    SceneEditorTool();
    ~SceneEditorTool() override = default;

    // ── IHostedTool identity ──────────────────────────────────────
    [[nodiscard]] const HostedToolDescriptor& descriptor() const override { return m_descriptor; }
    [[nodiscard]] const std::string& toolId()              const override { return m_descriptor.toolId; }

    // ── IHostedTool lifecycle ─────────────────────────────────────
    bool initialize() override;
    void shutdown()   override;
    void activate()   override;
    void suspend()    override;
    void update(float dt) override;

    [[nodiscard]] HostedToolState state() const override { return m_state; }

    // ── IHostedTool input hook ────────────────────────────────────
    void onAttachInput(const InputSystem* input) override { m_input = input; }
    void onDetachInput() override                        { m_input = nullptr; }

    // ── IViewportSceneProvider ────────────────────────────────────
    ViewportSceneState provideScene(ViewportHandle handle,
                                    const ViewportSlot& slot) override;

    // ── Viewport slot access ──────────────────────────────────────
    [[nodiscard]] ViewportHandle viewportHandle() const { return m_viewportHandle; }

    /// Attach the workspace viewport manager so this tool can request/release slots.
    /// Call before activate() (e.g. from WorkspaceShell::initialize()).
    void attachViewportManager(WorkspaceViewportManager* mgr) { m_viewportMgr = mgr; }

    /// Attach a scene provider so provideScene() delegates to it (Phase D wiring).
    /// Pass nullptr to detach and revert to the default stub scene state.
    /// NovaForgePreviewRuntime implements IViewportSceneProvider — pass it directly.
    void attachSceneProvider(IViewportSceneProvider* provider) {
        m_sceneProvider = provider;
    }

    [[nodiscard]] IViewportSceneProvider* sceneProvider() const {
        return m_sceneProvider;
    }

    // ── Camera state accessors (for tests and overlay display) ────
    [[nodiscard]] Vec3  cameraPosition() const { return m_camPos;   }
    [[nodiscard]] float cameraYaw()      const { return m_camYaw;   }
    [[nodiscard]] float cameraPitch()    const { return m_camPitch; }
    [[nodiscard]] bool  isFlyCamActive() const { return m_camController.isActive(); }

    void setCameraPosition(Vec3 pos)    { m_camPos   = pos; }
    void setCameraYaw(float yaw)        { m_camYaw   = yaw; }
    void setCameraPitch(float pitch)    { m_camPitch = pitch; }

    // ── Project adapter hooks ─────────────────────────────────────
    void onProjectLoaded(const std::string& projectId) override;
    void onProjectUnloaded() override;

    // ── Scene Editor interface ────────────────────────────────────

    [[nodiscard]] SceneEditMode    editMode() const { return m_editMode; }
    void                           setEditMode(SceneEditMode mode);

    [[nodiscard]] const SceneEditorStats& stats()   const { return m_stats; }
    [[nodiscard]] bool                    isDirty() const { return m_stats.isDirty; }
    void                                  markDirty() const;
    void                                  clearDirty() const;

    // Entity selection (counts only — scene data lives in the runtime)
    [[nodiscard]] uint32_t selectionCount() const { return m_stats.selectionCount; }
    void setSelectionCount(uint32_t count);

    // Scene entity count (updated each frame from scene runtime)
    void setEntityCount(uint32_t count);

    // Set which entity (0-based index) is selected in the view.
    // Called by the HierarchyPanel click callback so the Inspector updates
    // on the next syncPanels() call.  index = -1 means no selection.
    void setViewSelectedEntity(int index) { m_viewSelectedEntity = index; }

    // ── Render contract ───────────────────────────────────────────
    // Renders: Hierarchy | 3D Viewport | Inspector — three-column layout.
    // Status bar row shows mode, dirty flag, entity/selection counts.
    void renderToolView(const ToolViewRenderContext& ctx) const override;

    // ── Panel sync contract ───────────────────────────────────────
    // Pushes scene state into WorkspacePanelHost each frame so AtlasUI panels
    // show live entity/selection/transform data.
    void syncPanels(WorkspacePanelHost& host) const override;

private:
    HostedToolDescriptor     m_descriptor;
    HostedToolState          m_state      = HostedToolState::Unloaded;
    SceneEditorStats         m_stats;     // isDirty is mutable within the struct
    std::string              m_activeProjectId;

    // Viewport slot owned by this tool while Active
    ViewportHandle           m_viewportHandle = kInvalidViewportHandle;
    WorkspaceViewportManager* m_viewportMgr  = nullptr;

    // Optional scene provider — when set, provideScene() delegates to it (Phase D)
    IViewportSceneProvider*   m_sceneProvider = nullptr;

    // Input pointer injected via onAttachInput()
    const InputSystem*       m_input          = nullptr;

    // ── Fly-camera state ───────────────────────────────────────────
    // Updated each frame in update() when m_input is available.
    // Pushed to the viewport slot as a ViewportCameraDescriptor.
    Vec3  m_camPos   = {0.f, 0.f, 5.f};
    float m_camYaw   = -90.f;  // degrees; -90 = look down -Z
    float m_camPitch =   0.f;
    SceneViewportCameraController m_camController;

    // ── Mutable per-view UI state (safe from const renderToolView) ─
    // Hierarchy selection — index into the displayed entity list, -1 = none.
    mutable int           m_viewSelectedEntity = -1;
    // Current edit mode — set directly from renderToolView hit regions so that
    // button clicks take effect immediately without a command bus round-trip.
    mutable SceneEditMode m_editMode           = SceneEditMode::Select;

    // ── Mutable per-entity transform state ────────────────────────
    // Backing storage for interactive inspector sliders.  Each entity has
    // 9 transform floats (pos xyz, rot xyz, scale xyz) and type-specific
    // component floats.  Declared mutable so renderToolView() can update
    // them through slider interactions.
    static constexpr size_t kMaxEntities = 5;
    struct EntityTransform {
        float pos[3]   = {0.f, 0.f, 0.f};
        float rot[3]   = {0.f, 0.f, 0.f};
        float scale[3] = {1.f, 1.f, 1.f};
    };
    mutable EntityTransform m_entityTransforms[kMaxEntities];

    // Per-entity component state (Camera, Light, RigidBody, etc.)
    mutable float m_cameraFov = 60.f;
    mutable float m_cameraNear = 0.1f;
    mutable float m_cameraFar = 1000.f;
    mutable float m_lightIntensity = 1.f;
    mutable float m_lightAngle = 45.f;
    mutable float m_playerMass = 80.f;
    mutable float m_playerDrag = 0.05f;
    mutable float m_colliderRadius = 0.4f;
    mutable float m_colliderHeight = 1.8f;
    mutable float m_envAmbient = 0.05f;
    mutable float m_envFogStart = 50.f;

    void buildDescriptor();
};

} // namespace NF
