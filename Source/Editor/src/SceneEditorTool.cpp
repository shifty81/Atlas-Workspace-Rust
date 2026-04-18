// NF::Editor — SceneEditorTool implementation.
//
// First real NF::IHostedTool from Phase 3 consolidation.
// Replaces the former pattern of header-only V1 stubs with a real
// lifecycle-managed tool registered with WorkspaceShell via ToolRegistry.

#include "NF/Editor/SceneEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/WorkspacePanelHost.h"
#include "NF/UI/AtlasUI/Panels/ViewportPanel.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <utility>

namespace NF {

SceneEditorTool::SceneEditorTool() {
    buildDescriptor();
}

void SceneEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Scene Editor";
    m_descriptor.category    = HostedToolCategory::SceneEditing;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = true;

    // Shared panels declared by this tool.
    // These are registered with PanelRegistry by WorkspaceShell.
    m_descriptor.supportedPanels = {
        "panel.viewport",
        "panel.outliner",
        "panel.inspector",
        "panel.console",
    };

    // Commands contributed by this tool.
    m_descriptor.commands = {
        "scene.create_entity",
        "scene.delete_entity",
        "scene.duplicate_entity",
        "scene.save_scene",
        "scene.enter_play",
        "scene.exit_play",
        "scene.set_mode.select",
        "scene.set_mode.translate",
        "scene.set_mode.rotate",
        "scene.set_mode.scale",
    };
}

bool SceneEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode = SceneEditMode::Select;
    m_stats    = {};

    // Seed default world-space transforms so the five starter entities are
    // immediately spread across the viewport rather than piled at the origin.
    // Values are in world units; the viewport renders them top-down at 6 px/unit.
    //
    //  Entity 0 — Camera_Main:      behind the scene, slightly above ground
    //  Entity 1 — DirectionalLight: high up, offset right/forward
    //  Entity 2 — Player:           scene origin
    //  Entity 3 — Environment:      left and forward
    //  Entity 4 — SkyDome:          far right and forward (large radius)

    m_entityTransforms[0] = { {  0.f,  5.f, -10.f }, { 0.f,  0.f, 0.f }, { 1.f, 1.f, 1.f } };
    m_entityTransforms[1] = { { 10.f, 20.f,   5.f }, { -45.f, 30.f, 0.f }, { 1.f, 1.f, 1.f } };
    m_entityTransforms[2] = { {  0.f,  0.f,   0.f }, { 0.f,  0.f, 0.f }, { 1.f, 1.f, 1.f } };
    m_entityTransforms[3] = { { -8.f,  0.f,   8.f }, { 0.f,  0.f, 0.f }, { 1.f, 1.f, 1.f } };
    m_entityTransforms[4] = { { 12.f, 30.f,  12.f }, { 0.f,  0.f, 0.f }, { 1.f, 1.f, 1.f } };

    m_state = HostedToolState::Ready;
    return true;
}

void SceneEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = SceneEditMode::Select;
    m_stats           = {};
    m_activeProjectId = {};
}

void SceneEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;

        // Seed a minimal default scene if no entities are present so the
        // viewport always shows content rather than an empty-state message.
        if (m_stats.entityCount == 0) {
            // Default scene: camera + directional light + environment root.
            m_stats.entityCount = 3;
        }

        // Request a viewport slot from the workspace viewport manager.
        // A default 1280×720 bounds is used here; the real bounds are applied
        // via the ViewportPanel resize callback (WorkspaceViewportBridge) on
        // the first paint cycle.
        if (m_viewportMgr && m_viewportHandle == kInvalidViewportHandle) {
            m_viewportHandle = m_viewportMgr->requestViewport(
                kToolId, {0.f, 0.f, 1280.f, 720.f});
            if (m_viewportHandle != kInvalidViewportHandle) {
                m_viewportMgr->registerSceneProvider(kToolId, this);
                m_viewportMgr->activateViewport(m_viewportHandle);
            }
        }
    }
}

void SceneEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;

        // Release the viewport slot so it can be reused by another tool.
        if (m_viewportMgr && m_viewportHandle != kInvalidViewportHandle) {
            m_viewportMgr->unregisterSceneProvider(kToolId);
            m_viewportMgr->releaseViewport(m_viewportHandle);
            m_viewportHandle = kInvalidViewportHandle;
        }
    }
}

void SceneEditorTool::update(float dt) {
    if (m_state != HostedToolState::Active) return;
    m_stats.lastFrameMs = dt * 1000.0f;

    // ── Fly-camera update ─────────────────────────────────────────────────────
    // Process WASD + mouse-look when an InputSystem is attached.
    // Only runs when no external scene provider owns the camera (i.e. the tool
    // drives the viewport camera directly rather than delegating to a runtime).
    if (m_input && !m_sceneProvider
            && m_viewportMgr && m_viewportHandle != kInvalidViewportHandle) {
        m_camController.update(dt, *m_input, m_camPos, m_camYaw, m_camPitch);

        // Push the updated camera descriptor to the viewport slot so the
        // frame loop and any GPU backend use the correct view transform.
        ViewportCameraDescriptor cam;
        cam.position    = m_camPos;
        cam.yaw         = m_camYaw;
        cam.pitch       = m_camPitch;
        cam.fovDegrees  = 60.f;
        cam.nearPlane   = 0.1f;
        cam.farPlane    = 10000.f;
        m_viewportMgr->setCamera(m_viewportHandle, cam);
    }

    // Submit gizmo commands for each selected entity when in a transform mode.
    // Position data is currently a centroid placeholder (0,0,0) until the scene
    // runtime feeds per-entity world positions via a future entity query API.
    if (m_viewportMgr && m_viewportHandle != kInvalidViewportHandle
            && m_stats.selectionCount > 0) {
        bool isTransformMode = (m_editMode == SceneEditMode::Translate
                             || m_editMode == SceneEditMode::Rotate
                             || m_editMode == SceneEditMode::Scale);
        if (isTransformMode) {
            GizmoType gizmoType = GizmoType::Translate;
            if (m_editMode == SceneEditMode::Rotate) gizmoType = GizmoType::Rotate;
            if (m_editMode == SceneEditMode::Scale)  gizmoType = GizmoType::Scale;

            GizmoDrawCommand cmd;
            cmd.axis     = GizmoAxis::XYZ;
            cmd.type     = gizmoType;
            cmd.position = {0.f, 0.f, 0.f}; // placeholder — scene runtime provides real pos
            cmd.size     = 1.f;
            m_viewportMgr->addGizmo(cmd);
        }
    }
}

ViewportSceneState SceneEditorTool::provideScene(ViewportHandle handle,
                                                  const ViewportSlot& slot) {
    // Delegate to the attached scene provider when one is wired (Phase D).
    if (m_sceneProvider)
        return m_sceneProvider->provideScene(handle, slot);

    ViewportSceneState state;
    state.hasContent  = (m_stats.entityCount > 0);
    state.entityCount = m_stats.entityCount;
    state.clearColor  = 0x1E1E1EFFu; // dark viewport background

    // The tool owns the camera when no external provider is attached.
    // Signal to the frame loop that it should use the tool's camera descriptor,
    // which is kept up-to-date by the fly-cam controller in update().
    state.overrideCamera = (m_viewportHandle != kInvalidViewportHandle);

    return state;
}

void SceneEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    // Seed a default starter scene with a camera, light, player, environment,
    // and sky so the viewport is populated immediately on project open.
    m_stats = {};
    m_stats.entityCount = 5;
}

void SceneEditorTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats = {};
}

void SceneEditorTool::setEditMode(SceneEditMode mode) {
    m_editMode = mode;
}

void SceneEditorTool::markDirty() const {
    m_stats.isDirty = true;
}

void SceneEditorTool::clearDirty() const {
    m_stats.isDirty = false;
}

void SceneEditorTool::setSelectionCount(uint32_t count) {
    m_stats.selectionCount = count;
}

void SceneEditorTool::setEntityCount(uint32_t count) {
    m_stats.entityCount = count;
}

void SceneEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Scene Editor panels (Hierarchy, Viewport, Inspector, ContentBrowser) are
    // owned and rendered by WorkspacePanelHost (see main.cpp).
    // renderToolView is intentionally a no-op for this tool so that the
    // AtlasUI panels are the sole source of visuals and hit-regions,
    // eliminating the click-offset mismatch caused by double-rendering.
    (void)ctx;
}
void SceneEditorTool::syncPanels(WorkspacePanelHost& host) const {
    // ── Hierarchy panel ────────────────────────────────────────────
    host.hierarchy().clearEntities();
    static const char* kEntityNames[] = {
        "Camera_Main", "DirectionalLight", "Player", "Environment", "SkyDome"
    };
    uint32_t count = m_stats.entityCount < kMaxEntities
                   ? m_stats.entityCount
                   : static_cast<uint32_t>(kMaxEntities);
    for (uint32_t i = 0; i < count; ++i) {
        bool selected = (static_cast<int>(i) == m_viewSelectedEntity);
        host.hierarchy().addEntity(static_cast<int>(i + 1),
                                   kEntityNames[i], selected, 0);
    }

    // ── Inspector panel ────────────────────────────────────────────
    if (m_viewSelectedEntity >= 0 &&
        static_cast<size_t>(m_viewSelectedEntity) < kMaxEntities) {
        const EntityTransform& t = m_entityTransforms[
            static_cast<size_t>(m_viewSelectedEntity)];
        host.inspector().setSelectedEntityId(m_viewSelectedEntity + 1);
        host.inspector().setTransform(t.pos[0], t.pos[1], t.pos[2]);
    } else {
        host.inspector().setSelectedEntityId(-1);
    }

    // ── Viewport panel ─────────────────────────────────────────────
    using VM = UI::AtlasUI::ViewportToolMode;
    VM vmode = VM::Select;
    switch (m_editMode) {
        case SceneEditMode::Translate: vmode = VM::Move;   break;
        case SceneEditMode::Rotate:    vmode = VM::Rotate; break;
        case SceneEditMode::Scale:     vmode = VM::Scale;  break;
        case SceneEditMode::Paint:     vmode = VM::Paint;  break;
        default:                       vmode = VM::Select; break;
    }
    host.viewport().setToolMode(vmode);
    // Sync fly-camera position so the overlay text shows current coordinates.
    host.viewport().setCameraPosition(m_camPos.x, m_camPos.y, m_camPos.z);
}

} // namespace NF
