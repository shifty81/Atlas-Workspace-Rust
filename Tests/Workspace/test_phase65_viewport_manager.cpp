#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// Phase 65 — WorkspaceViewportManager
#include "NF/Workspace/WorkspaceViewportManager.h"

using Catch::Matchers::WithinAbs;
using namespace NF;

// ─────────────────────────────────────────────────────────────────────────────
// Construction / initial state
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager default state has no slots", "[Phase65][Manager]") {
    WorkspaceViewportManager mgr;
    CHECK(mgr.slotCount()    == 0);
    CHECK(mgr.activeCount()  == 0);
    CHECK(mgr.surfaceCount() == 0);
    CHECK(mgr.providerCount() == 0);
}

TEST_CASE("WorkspaceViewportManager default layout is Single", "[Phase65][Manager]") {
    WorkspaceViewportManager mgr;
    CHECK(mgr.layoutMode() == ViewportLayoutMode::Single);
    CHECK(std::string(mgr.layoutName()) == "Single");
}

TEST_CASE("WorkspaceViewportManager sub-registry accessors are not null", "[Phase65][Manager]") {
    WorkspaceViewportManager mgr;
    // Just verify the references are reachable (compile + link smoke test)
    CHECK(mgr.slotCount() == mgr.viewportRegistry().slotCount());
    CHECK(mgr.surfaceCount() == mgr.surfaceRegistry().surfaceCount());
    CHECK(mgr.providerCount() == mgr.sceneRegistry().providerCount());
}

// ─────────────────────────────────────────────────────────────────────────────
// requestViewport / releaseViewport
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager requestViewport returns valid handle", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 1280, 720});
    CHECK(h != kInvalidViewportHandle);
    CHECK(mgr.slotCount() == 1);
}

TEST_CASE("WorkspaceViewportManager requestViewport invalid bounds returns invalid", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 0, 0}); // zero dimensions
    CHECK(h == kInvalidViewportHandle);
    CHECK(mgr.slotCount() == 0);
}

TEST_CASE("WorkspaceViewportManager requestViewport empty toolId returns invalid", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("", {0, 0, 800, 600});
    CHECK(h == kInvalidViewportHandle);
}

TEST_CASE("WorkspaceViewportManager requestViewport multiple slots", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h1 = mgr.requestViewport("tool.a", {0, 0, 800, 600});
    auto h2 = mgr.requestViewport("tool.b", {0, 0, 800, 600});
    CHECK(h1 != kInvalidViewportHandle);
    CHECK(h2 != kInvalidViewportHandle);
    CHECK(h1 != h2);
    CHECK(mgr.slotCount() == 2);
}

TEST_CASE("WorkspaceViewportManager releaseViewport removes slot", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    CHECK(mgr.releaseViewport(h));
    CHECK(mgr.slotCount() == 0);
}

TEST_CASE("WorkspaceViewportManager releaseViewport unknown handle returns false", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    CHECK_FALSE(mgr.releaseViewport(99));
}

TEST_CASE("WorkspaceViewportManager findSlot returns null for unknown handle", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    CHECK(mgr.findSlot(kInvalidViewportHandle) == nullptr);
    CHECK(mgr.findSlot(42) == nullptr);
}

TEST_CASE("WorkspaceViewportManager findSlot returns slot after request", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    const auto* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK(slot->toolId == "tool.scene");
    CHECK_THAT(slot->bounds.width, WithinAbs(800.f, 1e-3f));
}

// ─────────────────────────────────────────────────────────────────────────────
// activateViewport / pauseViewport / resumeViewport
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager slot starts Idle, activates to Active", "[Phase65][Lifecycle]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    const auto* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK(slot->state == ViewportState::Idle);
    CHECK(mgr.activateViewport(h));
    CHECK(slot->state == ViewportState::Active);
    CHECK(mgr.activeCount() == 1);
}

TEST_CASE("WorkspaceViewportManager activateViewport unknown returns false", "[Phase65][Lifecycle]") {
    WorkspaceViewportManager mgr;
    CHECK_FALSE(mgr.activateViewport(99));
}

TEST_CASE("WorkspaceViewportManager pauseViewport / resumeViewport", "[Phase65][Lifecycle]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    mgr.activateViewport(h);
    CHECK(mgr.pauseViewport(h));
    const auto* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK(slot->state == ViewportState::Paused);
    CHECK(mgr.resumeViewport(h));
    CHECK(slot->state == ViewportState::Active);
}

TEST_CASE("WorkspaceViewportManager pauseViewport unknown returns false", "[Phase65][Lifecycle]") {
    WorkspaceViewportManager mgr;
    CHECK_FALSE(mgr.pauseViewport(99));
}

TEST_CASE("WorkspaceViewportManager resumeViewport unknown returns false", "[Phase65][Lifecycle]") {
    WorkspaceViewportManager mgr;
    CHECK_FALSE(mgr.resumeViewport(99));
}

// ─────────────────────────────────────────────────────────────────────────────
// updateBounds / setRenderMode / setCamera
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager updateBounds updates slot bounds", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    CHECK(mgr.updateBounds(h, {0, 0, 1920, 1080}));
    const auto* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK_THAT(slot->bounds.width,  WithinAbs(1920.f, 1e-3f));
    CHECK_THAT(slot->bounds.height, WithinAbs(1080.f, 1e-3f));
}

TEST_CASE("WorkspaceViewportManager updateBounds invalid bounds returns false", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    CHECK_FALSE(mgr.updateBounds(h, {0, 0, 0, 0}));
}

TEST_CASE("WorkspaceViewportManager setRenderMode updates slot", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    CHECK(mgr.setRenderMode(h, ViewportRenderMode::Wireframe));
    const auto* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK(slot->renderMode == ViewportRenderMode::Wireframe);
}

TEST_CASE("WorkspaceViewportManager setCamera updates slot camera", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    ViewportCameraDescriptor cam;
    cam.position = {10.f, 5.f, -3.f};
    cam.yaw      = 45.f;
    cam.pitch    = -15.f;
    CHECK(mgr.setCamera(h, cam));
    const auto* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK_THAT(slot->camera.position.x, WithinAbs(10.f,  1e-5f));
    CHECK_THAT(slot->camera.yaw,        WithinAbs(45.f,  1e-5f));
    CHECK_THAT(slot->camera.pitch,      WithinAbs(-15.f, 1e-5f));
}

// ─────────────────────────────────────────────────────────────────────────────
// Surface registration
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager registerSurface increments surfaceCount", "[Phase65][Surface]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    NullViewportSurface surface(800, 600);
    CHECK(mgr.registerSurface(h, &surface));
    CHECK(mgr.surfaceCount() == 1);
}

TEST_CASE("WorkspaceViewportManager registerSurface null returns false", "[Phase65][Surface]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    CHECK_FALSE(mgr.registerSurface(h, nullptr));
}

TEST_CASE("WorkspaceViewportManager unregisterSurface decrements surfaceCount", "[Phase65][Surface]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    NullViewportSurface surface(800, 600);
    mgr.registerSurface(h, &surface);
    CHECK(mgr.unregisterSurface(h));
    CHECK(mgr.surfaceCount() == 0);
}

TEST_CASE("WorkspaceViewportManager releaseViewport also unregisters surface", "[Phase65][Surface]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    NullViewportSurface surface(800, 600);
    mgr.registerSurface(h, &surface);
    CHECK(mgr.surfaceCount() == 1);
    mgr.releaseViewport(h);
    CHECK(mgr.surfaceCount() == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Scene provider registration
// ─────────────────────────────────────────────────────────────────────────────

// Minimal concrete provider for testing
struct TestProvider : public IViewportSceneProvider {
    int callCount = 0;
    ViewportSceneState result;
    ViewportSceneState provideScene(ViewportHandle, const ViewportSlot&) override {
        ++callCount;
        return result;
    }
};

TEST_CASE("WorkspaceViewportManager registerSceneProvider increments providerCount", "[Phase65][Provider]") {
    WorkspaceViewportManager mgr;
    TestProvider p;
    CHECK(mgr.registerSceneProvider("tool.scene", &p));
    CHECK(mgr.providerCount() == 1);
}

TEST_CASE("WorkspaceViewportManager registerSceneProvider null returns false", "[Phase65][Provider]") {
    WorkspaceViewportManager mgr;
    CHECK_FALSE(mgr.registerSceneProvider("tool.scene", nullptr));
}

TEST_CASE("WorkspaceViewportManager registerSceneProvider empty toolId returns false", "[Phase65][Provider]") {
    WorkspaceViewportManager mgr;
    TestProvider p;
    CHECK_FALSE(mgr.registerSceneProvider("", &p));
}

TEST_CASE("WorkspaceViewportManager unregisterSceneProvider decrements providerCount", "[Phase65][Provider]") {
    WorkspaceViewportManager mgr;
    TestProvider p;
    mgr.registerSceneProvider("tool.scene", &p);
    CHECK(mgr.unregisterSceneProvider("tool.scene"));
    CHECK(mgr.providerCount() == 0);
}

TEST_CASE("WorkspaceViewportManager unregisterSceneProvider unknown returns false", "[Phase65][Provider]") {
    WorkspaceViewportManager mgr;
    CHECK_FALSE(mgr.unregisterSceneProvider("unknown"));
}

// ─────────────────────────────────────────────────────────────────────────────
// renderFrame
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager renderFrame with no active slots returns empty", "[Phase65][FrameLoop]") {
    WorkspaceViewportManager mgr;
    auto results = mgr.renderFrame();
    CHECK(results.empty());
    CHECK(mgr.frameStats().activeSlots == 0);
    CHECK(mgr.frameStats().renderedSlots == 0);
}

TEST_CASE("WorkspaceViewportManager renderFrame active slot without surface skips render", "[Phase65][FrameLoop]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    mgr.activateViewport(h);
    auto results = mgr.renderFrame();
    REQUIRE(results.size() == 1);
    CHECK(results[0].handle == h);
    CHECK_FALSE(results[0].rendered);
}

TEST_CASE("WorkspaceViewportManager renderFrame active slot with valid surface renders", "[Phase65][FrameLoop]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    mgr.activateViewport(h);
    NullViewportSurface surface(800, 600);
    surface.setColorAttachment(7);
    mgr.registerSurface(h, &surface);

    auto results = mgr.renderFrame();
    REQUIRE(results.size() == 1);
    CHECK(results[0].rendered);
    CHECK(results[0].colorAttachmentId == 7);
    CHECK(surface.isBound() == false); // unbind was called
    CHECK(surface.bindCount() == 1);
}

TEST_CASE("WorkspaceViewportManager renderFrame increments frameCount on slot", "[Phase65][FrameLoop]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    mgr.activateViewport(h);
    NullViewportSurface surface(800, 600);
    mgr.registerSurface(h, &surface);

    mgr.renderFrame();
    mgr.renderFrame();

    const auto* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK(slot->frameCount == 2);
}

TEST_CASE("WorkspaceViewportManager renderFrame paused slot skips", "[Phase65][FrameLoop]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    mgr.activateViewport(h);
    mgr.pauseViewport(h);
    NullViewportSurface surface(800, 600);
    mgr.registerSurface(h, &surface);

    auto results = mgr.renderFrame();
    CHECK(results.empty()); // paused slots not active
}

TEST_CASE("WorkspaceViewportManager renderFrame dispatches to scene provider", "[Phase65][FrameLoop]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    mgr.activateViewport(h);

    TestProvider p;
    p.result.hasContent  = true;
    p.result.entityCount = 10;
    mgr.registerSceneProvider("tool.scene", &p);

    NullViewportSurface surface(800, 600);
    mgr.registerSurface(h, &surface);

    auto results = mgr.renderFrame();
    REQUIRE(results.size() == 1);
    CHECK(results[0].sceneState.hasContent);
    CHECK(results[0].sceneState.entityCount == 10);
    CHECK(p.callCount == 1);
}

TEST_CASE("WorkspaceViewportManager frameStats track multiple frames", "[Phase65][FrameLoop]") {
    WorkspaceViewportManager mgr;
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    mgr.activateViewport(h);
    NullViewportSurface surface(800, 600);
    mgr.registerSurface(h, &surface);

    mgr.renderFrame();
    mgr.renderFrame();
    mgr.renderFrame();

    CHECK(mgr.frameStats().frameNumber == 3);
    CHECK(mgr.frameStats().renderedSlots == 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Layout / compositor
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager setLayoutMode changes mode", "[Phase65][Layout]") {
    WorkspaceViewportManager mgr;
    mgr.setLayoutMode(ViewportLayoutMode::TwoByTwo);
    CHECK(mgr.layoutMode() == ViewportLayoutMode::TwoByTwo);
    CHECK(std::string(mgr.layoutName()) == "TwoByTwo");
}

TEST_CASE("WorkspaceViewportManager computeLayout Single fills full bounds", "[Phase65][Layout]") {
    WorkspaceViewportManager mgr;
    mgr.setLayoutMode(ViewportLayoutMode::Single);
    auto h = mgr.requestViewport("tool.a", {0, 0, 1280, 720});
    auto regions = mgr.computeLayout({0, 0, 1280, 720}, {h});
    REQUIRE(regions.size() == 1);
    CHECK(regions[0].handle == h);
    CHECK_THAT(regions[0].bounds.width,  WithinAbs(1280.f, 1e-3f));
    CHECK_THAT(regions[0].bounds.height, WithinAbs(720.f,  1e-3f));
}

TEST_CASE("WorkspaceViewportManager computeLayout SideBySide splits in two", "[Phase65][Layout]") {
    WorkspaceViewportManager mgr;
    mgr.setLayoutMode(ViewportLayoutMode::SideBySide);
    auto h1 = mgr.requestViewport("tool.a", {0, 0, 1000, 600});
    auto h2 = mgr.requestViewport("tool.b", {0, 0, 1000, 600});
    auto regions = mgr.computeLayout({0, 0, 1000, 600}, {h1, h2});
    REQUIRE(regions.size() == 2);
    CHECK_THAT(regions[0].bounds.width, WithinAbs(500.f, 1e-3f));
    CHECK_THAT(regions[1].bounds.width, WithinAbs(500.f, 1e-3f));
}

TEST_CASE("WorkspaceViewportManager computeLayout TwoByTwo produces 4 regions", "[Phase65][Layout]") {
    WorkspaceViewportManager mgr;
    mgr.setLayoutMode(ViewportLayoutMode::TwoByTwo);
    std::vector<ViewportHandle> handles;
    for (int i = 0; i < 4; ++i)
        handles.push_back(mgr.requestViewport("tool." + std::to_string(i), {0, 0, 800, 600}));
    auto regions = mgr.computeLayout({0, 0, 800, 600}, handles);
    REQUIRE(regions.size() == 4);
    for (const auto& r : regions)
        CHECK_THAT(r.bounds.width, WithinAbs(400.f, 1e-3f));
}

// ─────────────────────────────────────────────────────────────────────────────
// Gizmo renderer
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager addGizmo/clearGizmos lifecycle", "[Phase65][Gizmo]") {
    WorkspaceViewportManager mgr;
    CHECK(mgr.gizmoRenderer().commandCount() == 0);
    mgr.addGizmo({GizmoAxis::X, GizmoType::Translate, {}, 1.f, false});
    CHECK(mgr.gizmoRenderer().commandCount() == 1);
    mgr.clearGizmos();
    CHECK(mgr.gizmoRenderer().commandCount() == 0);
}

TEST_CASE("WorkspaceViewportManager renderGizmos on valid surface returns count", "[Phase65][Gizmo]") {
    WorkspaceViewportManager mgr;
    mgr.addGizmo({GizmoAxis::X, GizmoType::Translate, {}, 1.f, false});
    mgr.addGizmo({GizmoAxis::Y, GizmoType::Scale,     {}, 1.f, true});
    NullViewportSurface surface(800, 600);
    auto h = mgr.requestViewport("tool.scene", {0, 0, 800, 600});
    CHECK(mgr.renderGizmos(surface, h) == 2);
}

TEST_CASE("WorkspaceViewportManager renderGizmos on invalid surface returns 0", "[Phase65][Gizmo]") {
    WorkspaceViewportManager mgr;
    mgr.addGizmo({GizmoAxis::X, GizmoType::Translate, {}, 1.f, false});
    NullViewportSurface invalid; // 0×0
    CHECK(mgr.renderGizmos(invalid, 1) == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// activeHandles
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager activeHandles returns only active slots", "[Phase65][Slot]") {
    WorkspaceViewportManager mgr;
    auto h1 = mgr.requestViewport("tool.a", {0, 0, 800, 600});
    auto h2 = mgr.requestViewport("tool.b", {0, 0, 800, 600});
    auto h3 = mgr.requestViewport("tool.c", {0, 0, 800, 600});
    mgr.activateViewport(h1);
    mgr.activateViewport(h3);
    // h2 remains Idle

    auto active = mgr.activeHandles();
    REQUIRE(active.size() == 2);
    CHECK(std::find(active.begin(), active.end(), h1) != active.end());
    CHECK(std::find(active.begin(), active.end(), h3) != active.end());
    CHECK(std::find(active.begin(), active.end(), h2) == active.end());
}

// ─────────────────────────────────────────────────────────────────────────────
// Integration: full single-viewport frame
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceViewportManager full single-viewport frame integration", "[Phase65][Integration]") {
    WorkspaceViewportManager mgr;

    // 1. Request and activate
    auto h = mgr.requestViewport("tool.scene", {0, 0, 1280, 720});
    mgr.activateViewport(h);

    // 2. Register a scene provider
    TestProvider provider;
    provider.result.hasContent  = true;
    provider.result.entityCount = 42;
    mgr.registerSceneProvider("tool.scene", &provider);

    // 3. Assign a surface
    NullViewportSurface surface(1280, 720);
    surface.setColorAttachment(100);
    mgr.registerSurface(h, &surface);

    // 4. Add gizmo
    mgr.addGizmo({GizmoAxis::XYZ, GizmoType::Translate, {}, 1.f, false});

    // 5. Render frame
    auto results = mgr.renderFrame();
    REQUIRE(results.size() == 1);
    CHECK(results[0].rendered);
    CHECK(results[0].colorAttachmentId == 100);
    CHECK(results[0].sceneState.hasContent);
    CHECK(results[0].sceneState.entityCount == 42);
    CHECK(provider.callCount == 1);

    // 6. Render gizmos
    CHECK(mgr.renderGizmos(surface, h) == 1);

    // 7. Layout round-trip
    mgr.setLayoutMode(ViewportLayoutMode::Single);
    auto layout = mgr.computeLayout({0, 0, 1280, 720}, mgr.activeHandles());
    REQUIRE(layout.size() == 1);
    CHECK_THAT(layout[0].bounds.width, WithinAbs(1280.f, 1e-3f));

    // 8. Clean up
    mgr.clearGizmos();
    CHECK(mgr.gizmoRenderer().commandCount() == 0);
    mgr.releaseViewport(h);
    CHECK(mgr.slotCount() == 0);
    CHECK(mgr.surfaceCount() == 0);
}

TEST_CASE("WorkspaceViewportManager multi-viewport frame integration", "[Phase65][Integration]") {
    WorkspaceViewportManager mgr;

    NullViewportSurface surf1(640, 720), surf2(640, 720);
    surf1.setColorAttachment(11);
    surf2.setColorAttachment(22);

    auto h1 = mgr.requestViewport("tool.a", {0,   0, 640, 720});
    auto h2 = mgr.requestViewport("tool.b", {640, 0, 640, 720});
    mgr.activateViewport(h1);
    mgr.activateViewport(h2);
    mgr.registerSurface(h1, &surf1);
    mgr.registerSurface(h2, &surf2);

    auto results = mgr.renderFrame();
    REQUIRE(results.size() == 2);

    bool found1 = false, found2 = false;
    for (const auto& r : results) {
        if (r.handle == h1) { CHECK(r.colorAttachmentId == 11); found1 = true; }
        if (r.handle == h2) { CHECK(r.colorAttachmentId == 22); found2 = true; }
    }
    CHECK(found1);
    CHECK(found2);

    CHECK(mgr.frameStats().renderedSlots == 2);
    CHECK(mgr.frameStats().activeSlots == 2);
}
