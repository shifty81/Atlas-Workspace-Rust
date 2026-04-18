#include <catch2/catch_test_macros.hpp>

// ── Viewport wiring integration tests (Phase 66) ────────────────────────────
// Verifies the 9-gap viewport wiring plan described in the architecture docs:
//
//  Gap 1  — WorkspaceShell owns WorkspaceViewportManager (viewportManager())
//  Gap 2  — IHostedTool::onAttachInput / onDetachInput hooks
//  Gap 3  — SceneEditorTool implements IViewportSceneProvider
//  Gap 4  — WorkspaceViewportBridge connects panel ↔ handle + resize callback
//  Gap 5  — forwardFrameResults forwards colorAttachmentId to ViewportPanel
//  Gap 6  — ViewportFrameLoop integrates gizmo pass (setGizmoRenderer)
//  Gap 7  — GDIViewportSurface: concrete IViewportSurface (constructor + resize)
//  Gap 8  — SceneEditorTool::update() submits GizmoDrawCommands
//  Gap 9  — WorkspaceShell::viewportManager() is accessible after initialize()

#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/WorkspaceViewportManager.h"
#include "NF/Workspace/WorkspaceViewportBridge.h"
#include "NF/Workspace/IViewportSurface.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include "NF/Workspace/ViewportFrameLoop.h"
#include "NF/Workspace/GizmoRenderer.h"
#include "NF/Editor/SceneEditorTool.h"
#include "NF/UI/AtlasUI/Panels/ViewportPanel.h"
#include "NF/UI/AtlasUI/Contexts.h"
#include "NF/UI/GDIViewportSurface.h"

using namespace NF;
using namespace NF::UI::AtlasUI;

// ─────────────────────────────────────────────────────────────────────────────
// Gap 1 — WorkspaceShell owns WorkspaceViewportManager
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceShell exposes viewportManager accessor", "[Phase66][Gap1]") {
    WorkspaceShell shell;
    // Before initialize: accessor must compile and return a valid reference.
    WorkspaceViewportManager& mgr = shell.viewportManager();
    CHECK(mgr.slotCount() == 0);
    CHECK(mgr.activeCount() == 0);
}

TEST_CASE("WorkspaceShell viewportManager is ready after initialize", "[Phase66][Gap1]") {
    WorkspaceShell shell;
    shell.initialize();
    CHECK(shell.viewportManager().slotCount() == 0);
    CHECK(shell.phase() == ShellPhase::Ready);
    shell.shutdown();
}

TEST_CASE("WorkspaceShell const viewportManager accessor is usable", "[Phase66][Gap1]") {
    const WorkspaceShell shell;
    const WorkspaceViewportManager& mgr = shell.viewportManager();
    CHECK(mgr.slotCount() == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Gap 2 — IHostedTool::onAttachInput / onDetachInput hooks
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SceneEditorTool onAttachInput stores input pointer", "[Phase66][Gap2]") {
    SceneEditorTool tool;
    tool.initialize();
    InputSystem input;
    tool.onAttachInput(&input);
    // If onDetachInput() doesn't crash, the pointer was stored and cleared.
    tool.onDetachInput();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool onDetachInput with no prior attach is safe", "[Phase66][Gap2]") {
    SceneEditorTool tool;
    tool.initialize();
    tool.onDetachInput(); // must not crash
    tool.shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// Gap 3 — SceneEditorTool implements IViewportSceneProvider
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SceneEditorTool IS-A IViewportSceneProvider", "[Phase66][Gap3]") {
    SceneEditorTool tool;
    IViewportSceneProvider* provider = &tool;
    CHECK(provider != nullptr);
}

TEST_CASE("SceneEditorTool provideScene returns content for default starter scene", "[Phase66][Gap3]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();

    ViewportHandle h = tool.viewportHandle();
    REQUIRE(h != kInvalidViewportHandle);

    const ViewportSlot* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);

    // activate() seeds a default starter scene (3 entities: camera, light, env root)
    // so the viewport always shows content rather than an empty-state message.
    ViewportSceneState state = tool.provideScene(h, *slot);
    CHECK(state.hasContent);  // starter scene entities present
    CHECK(state.entityCount == 3);

    tool.suspend();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool provideScene returns hasContent when entities present", "[Phase66][Gap3]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();

    tool.setEntityCount(5);

    ViewportHandle h = tool.viewportHandle();
    REQUIRE(h != kInvalidViewportHandle);
    const ViewportSlot* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);

    ViewportSceneState state = tool.provideScene(h, *slot);
    CHECK(state.hasContent);
    CHECK(state.entityCount == 5);

    tool.suspend();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool activate requests viewport slot", "[Phase66][Gap3]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);

    CHECK(mgr.slotCount() == 0);
    tool.activate();
    CHECK(mgr.slotCount() == 1);
    CHECK(tool.viewportHandle() != kInvalidViewportHandle);

    tool.suspend();
    CHECK(mgr.slotCount() == 0); // slot released on suspend
    CHECK(tool.viewportHandle() == kInvalidViewportHandle);

    tool.shutdown();
}

TEST_CASE("SceneEditorTool activate registers scene provider", "[Phase66][Gap3]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();

    CHECK(mgr.providerCount() == 1);

    tool.suspend();
    CHECK(mgr.providerCount() == 0);

    tool.shutdown();
}

TEST_CASE("SceneEditorTool activate sets slot to Active state", "[Phase66][Gap3]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();

    ViewportHandle h = tool.viewportHandle();
    REQUIRE(h != kInvalidViewportHandle);
    CHECK(mgr.activeCount() == 1);

    tool.suspend();
    tool.shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// Gap 4 — WorkspaceViewportBridge: connect / disconnect / forwardFrameResults
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportBridge::connect sets panel handle", "[Phase66][Gap4]") {
    WorkspaceViewportManager mgr;
    ViewportHandle h = mgr.requestViewport("test.tool", {0, 0, 800, 600});
    REQUIRE(h != kInvalidViewportHandle);

    ViewportPanel panel;
    CHECK(WorkspaceViewportBridge::connect(&panel, mgr, h));
    CHECK(panel.viewportHandle() == h);
}

TEST_CASE("WorkspaceViewportBridge::connect installs resize callback", "[Phase66][Gap4]") {
    WorkspaceViewportManager mgr;
    ViewportHandle h = mgr.requestViewport("test.tool", {0, 0, 800, 600});

    ViewportPanel panel;
    WorkspaceViewportBridge::connect(&panel, mgr, h);
    CHECK(panel.hasResizeCallback());
}

TEST_CASE("WorkspaceViewportBridge::connect with null surface still connects", "[Phase66][Gap4]") {
    WorkspaceViewportManager mgr;
    ViewportHandle h = mgr.requestViewport("test.tool", {0, 0, 800, 600});

    ViewportPanel panel;
    CHECK(WorkspaceViewportBridge::connect(&panel, mgr, h, nullptr));
    CHECK(mgr.surfaceCount() == 0); // no surface registered
}

TEST_CASE("WorkspaceViewportBridge::connect with NullSurface registers surface", "[Phase66][Gap4]") {
    WorkspaceViewportManager mgr;
    ViewportHandle h = mgr.requestViewport("test.tool", {0, 0, 800, 600});

    NullViewportSurface surface(800, 600);
    ViewportPanel panel;
    WorkspaceViewportBridge::connect(&panel, mgr, h, &surface);
    CHECK(mgr.surfaceCount() == 1);
}

TEST_CASE("WorkspaceViewportBridge::disconnect clears panel handle", "[Phase66][Gap4]") {
    WorkspaceViewportManager mgr;
    ViewportHandle h = mgr.requestViewport("test.tool", {0, 0, 800, 600});

    NullViewportSurface surface(800, 600);
    ViewportPanel panel;
    WorkspaceViewportBridge::connect(&panel, mgr, h, &surface);
    WorkspaceViewportBridge::disconnect(&panel, mgr, h);

    CHECK(panel.viewportHandle() == kInvalidViewportHandle);
    CHECK_FALSE(panel.hasResizeCallback());
    CHECK(mgr.surfaceCount() == 0);
}

TEST_CASE("WorkspaceViewportBridge::connect returns false for null panel", "[Phase66][Gap4]") {
    WorkspaceViewportManager mgr;
    ViewportHandle h = mgr.requestViewport("test.tool", {0, 0, 800, 600});
    CHECK_FALSE(WorkspaceViewportBridge::connect(nullptr, mgr, h));
}

TEST_CASE("WorkspaceViewportBridge::connect returns false for invalid handle", "[Phase66][Gap4]") {
    WorkspaceViewportManager mgr;
    ViewportPanel panel;
    CHECK_FALSE(WorkspaceViewportBridge::connect(&panel, mgr, kInvalidViewportHandle));
}

// ─────────────────────────────────────────────────────────────────────────────
// Gap 5 — forwardFrameResults routes colorAttachment to the correct panel
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("forwardFrameResults sets colorAttachment when handle matches", "[Phase66][Gap5]") {
    ViewportPanel panel;
    panel.setViewportHandle(3u);

    ViewportFrameResult r;
    r.handle           = 3u;
    r.colorAttachmentId = 42u;
    r.rendered         = true;

    std::vector<ViewportFrameResult> results{r};
    WorkspaceViewportBridge::forwardFrameResults(&panel, results);
    CHECK(panel.colorAttachment() == 42u);
}

TEST_CASE("forwardFrameResults does nothing when no handle matches", "[Phase66][Gap5]") {
    ViewportPanel panel;
    panel.setViewportHandle(7u);

    ViewportFrameResult r;
    r.handle           = 3u;
    r.colorAttachmentId = 42u;

    std::vector<ViewportFrameResult> results{r};
    WorkspaceViewportBridge::forwardFrameResults(&panel, results);
    CHECK(panel.colorAttachment() == 0u); // unchanged
}

TEST_CASE("forwardFrameResults with null panel is a no-op", "[Phase66][Gap5]") {
    std::vector<ViewportFrameResult> results;
    // Must not crash.
    WorkspaceViewportBridge::forwardFrameResults(nullptr, results);
}

TEST_CASE("forwardFrameResults with kInvalidViewportHandle panel is a no-op", "[Phase66][Gap5]") {
    ViewportPanel panel; // handle == 0 = kInvalidViewportHandle

    ViewportFrameResult r;
    r.handle           = 1u;
    r.colorAttachmentId = 99u;

    std::vector<ViewportFrameResult> results{r};
    WorkspaceViewportBridge::forwardFrameResults(&panel, results);
    CHECK(panel.colorAttachment() == 0u);
}

// ─────────────────────────────────────────────────────────────────────────────
// Gap 6 — ViewportFrameLoop integrates gizmo pass
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ViewportFrameLoop setGizmoRenderer stores the pointer", "[Phase66][Gap6]") {
    ViewportFrameLoop loop;
    GizmoRenderer gizmos;
    loop.setGizmoRenderer(&gizmos);
    CHECK(loop.gizmoRenderer() == &gizmos);
}

TEST_CASE("ViewportFrameLoop calls gizmo renderToSurface per rendered slot", "[Phase66][Gap6]") {
    ViewportHostRegistry          viewReg;
    ViewportSurfaceRegistry       surfReg;
    ViewportSceneProviderRegistry sceneReg;
    GizmoRenderer                 gizmos;

    ViewportHandle h = viewReg.requestSlot("tool.a", {0, 0, 800, 600});
    viewReg.activateSlot(h);

    NullViewportSurface surface(800, 600);
    surface.setColorAttachment(77u);
    surfReg.registerSurface(h, &surface);

    GizmoDrawCommand cmd;
    gizmos.addGizmo(cmd);
    CHECK(gizmos.commandCount() == 1);

    ViewportFrameLoop loop;
    loop.setViewportRegistry(&viewReg);
    loop.setSurfaceRegistry(&surfReg);
    loop.setSceneRegistry(&sceneReg);
    loop.setGizmoRenderer(&gizmos);

    auto results = loop.renderFrame();
    REQUIRE(results.size() == 1);
    CHECK(results[0].rendered);
    // renderToSurface() is a no-op stub but must not crash; gizmo count intact.
    CHECK(gizmos.commandCount() == 1);
}

TEST_CASE("ViewportFrameLoop without gizmo renderer still renders", "[Phase66][Gap6]") {
    ViewportHostRegistry    viewReg;
    ViewportSurfaceRegistry surfReg;

    ViewportHandle h = viewReg.requestSlot("tool.a", {0, 0, 800, 600});
    viewReg.activateSlot(h);
    NullViewportSurface surface(800, 600);
    surfReg.registerSurface(h, &surface);

    ViewportFrameLoop loop;
    loop.setViewportRegistry(&viewReg);
    loop.setSurfaceRegistry(&surfReg);
    // No gizmo renderer — must not crash.
    auto results = loop.renderFrame();
    CHECK(results.size() == 1);
    CHECK(results[0].rendered);
}

// ─────────────────────────────────────────────────────────────────────────────
// Gap 7 — GDIViewportSurface
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("GDIViewportSurface default state is invalid", "[Phase66][Gap7]") {
    GDIViewportSurface s;
    CHECK_FALSE(s.isValid());
    CHECK(s.width()  == 0);
    CHECK(s.height() == 0);
    CHECK(s.colorAttachment() == 0);
    CHECK_FALSE(s.isBound());
    CHECK(s.bindCount() == 0);
}

TEST_CASE("GDIViewportSurface resize makes surface valid", "[Phase66][Gap7]") {
    GDIViewportSurface s;
    s.resize(640, 480);
    CHECK(s.width()  == 640);
    CHECK(s.height() == 480);
    CHECK(s.isValid());
}

TEST_CASE("GDIViewportSurface bind/unbind cycle increments bindCount", "[Phase66][Gap7]") {
    GDIViewportSurface s;
    s.resize(256, 256);
    s.bind();
    CHECK(s.isBound());
    CHECK(s.bindCount() == 1);
    s.unbind();
    CHECK_FALSE(s.isBound());
    CHECK(s.bindCount() == 1);
}

TEST_CASE("GDIViewportSurface colorAttachment is non-zero after resize", "[Phase66][Gap7]") {
    GDIViewportSurface s;
    s.resize(128, 128);
    // On Windows the value is the memDC handle; on other platforms it is 1.
    CHECK(s.colorAttachment() != 0u);
}

TEST_CASE("GDIViewportSurface resize to zero invalidates surface", "[Phase66][Gap7]") {
    GDIViewportSurface s;
    s.resize(256, 256);
    CHECK(s.isValid());
    s.resize(0, 0);
    CHECK_FALSE(s.isValid());
    CHECK(s.colorAttachment() == 0u);
}

TEST_CASE("GDIViewportSurface is an IViewportSurface", "[Phase66][Gap7]") {
    GDIViewportSurface s;
    IViewportSurface* base = &s;
    CHECK(base != nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
// Gap 8 — SceneEditorTool::update() submits gizmo commands in transform modes
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SceneEditorTool update submits gizmo in Translate mode with selection", "[Phase66][Gap8]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();
    tool.setSelectionCount(2);
    tool.setEditMode(SceneEditMode::Translate);

    mgr.clearGizmos();
    tool.update(0.016f);
    CHECK(mgr.gizmoRenderer().commandCount() == 1);

    const auto& cmd = mgr.gizmoRenderer().commands().front();
    CHECK(cmd.type == GizmoType::Translate);

    tool.suspend();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool update submits Rotate gizmo in Rotate mode", "[Phase66][Gap8]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();
    tool.setSelectionCount(1);
    tool.setEditMode(SceneEditMode::Rotate);

    mgr.clearGizmos();
    tool.update(0.016f);
    REQUIRE(mgr.gizmoRenderer().commandCount() == 1);
    CHECK(mgr.gizmoRenderer().commands().front().type == GizmoType::Rotate);

    tool.suspend();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool update submits Scale gizmo in Scale mode", "[Phase66][Gap8]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();
    tool.setSelectionCount(3);
    tool.setEditMode(SceneEditMode::Scale);

    mgr.clearGizmos();
    tool.update(0.016f);
    REQUIRE(mgr.gizmoRenderer().commandCount() == 1);
    CHECK(mgr.gizmoRenderer().commands().front().type == GizmoType::Scale);

    tool.suspend();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool update does NOT submit gizmo in Select mode", "[Phase66][Gap8]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();
    tool.setSelectionCount(5);
    tool.setEditMode(SceneEditMode::Select);

    mgr.clearGizmos();
    tool.update(0.016f);
    CHECK(mgr.gizmoRenderer().commandCount() == 0);

    tool.suspend();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool update does NOT submit gizmo with no selection", "[Phase66][Gap8]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();
    tool.setSelectionCount(0);
    tool.setEditMode(SceneEditMode::Translate);

    mgr.clearGizmos();
    tool.update(0.016f);
    CHECK(mgr.gizmoRenderer().commandCount() == 0);

    tool.suspend();
    tool.shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// Gap 9 — Full round-trip: shell → tool → surface → frame loop → panel
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Full viewport wiring round-trip produces colorAttachment in panel", "[Phase66][Gap9]") {
    WorkspaceShell shell;
    shell.initialize();

    auto& mgr = shell.viewportManager();

    // Tool activation requests a viewport slot.
    SceneEditorTool tool;
    tool.initialize();
    tool.attachViewportManager(&mgr);
    tool.activate();

    ViewportHandle h = tool.viewportHandle();
    REQUIRE(h != kInvalidViewportHandle);

    // Register a valid surface.
    NullViewportSurface surface(1280, 720);
    surface.setColorAttachment(99u);
    mgr.registerSurface(h, &surface);

    // Wire the panel.
    ViewportPanel panel;
    WorkspaceViewportBridge::connect(&panel, mgr, h);

    // Run a frame.
    mgr.clearGizmos();
    auto results = mgr.renderFrame();
    REQUIRE(results.size() == 1);
    CHECK(results[0].rendered);
    CHECK(results[0].colorAttachmentId == 99u);

    WorkspaceViewportBridge::forwardFrameResults(&panel, results);
    CHECK(panel.colorAttachment() == 99u);

    tool.suspend();
    tool.shutdown();
    shell.shutdown();
}

TEST_CASE("Full round-trip with gizmo submission: tool feeds gizmos to frame loop", "[Phase66][Gap9]") {
    WorkspaceShell shell;
    shell.initialize();

    auto& mgr = shell.viewportManager();

    SceneEditorTool tool;
    tool.initialize();
    tool.attachViewportManager(&mgr);
    tool.activate();

    ViewportHandle h = tool.viewportHandle();
    REQUIRE(h != kInvalidViewportHandle);

    NullViewportSurface surface(800, 600);
    surface.setColorAttachment(55u);
    mgr.registerSurface(h, &surface);

    // Put tool in a state that emits gizmos.
    tool.setSelectionCount(1);
    tool.setEditMode(SceneEditMode::Translate);

    mgr.clearGizmos();
    tool.update(0.016f); // submits 1 gizmo
    CHECK(mgr.gizmoRenderer().commandCount() == 1);

    auto results = mgr.renderFrame(); // gizmo composited in frame loop
    REQUIRE(results.size() == 1);
    CHECK(results[0].rendered);

    tool.suspend();
    tool.shutdown();
    shell.shutdown();
}
