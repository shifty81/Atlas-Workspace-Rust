#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// Phase 67 — Viewport camera fly-cam wiring and software renderer scanline write.
//
// Covers the two gaps corrected in this phase:
//
//  1. SoftwareViewportRenderer::renderGrid() writes pixels into the surface
//     via IViewportSurface::writeScanlines().
//  2. SceneEditorTool exposes camera state accessors and the fly-cam controller
//     is wired so update() + onAttachInput() form a complete pipeline.
//  3. provideScene() sets overrideCamera=true when the tool is the camera owner.

#include "NF/Workspace/IViewportSurface.h"
#include "NF/Workspace/SoftwareViewportRenderer.h"
#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/WorkspaceViewportManager.h"
#include "NF/Editor/SceneEditorTool.h"
#include "NF/Input/Input.h"
#include "NF/UI/GDIViewportSurface.h"

using Catch::Matchers::WithinAbs;
using namespace NF;

// ─────────────────────────────────────────────────────────────────────────────
// 1. IViewportSurface::writeScanlines — default no-op on NullViewportSurface
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("NullViewportSurface::writeScanlines is a no-op (compiles and does not crash)", "[Phase67][Scanlines]") {
    NullViewportSurface s(4, 4);
    uint32_t pixels[16] = {};
    // Default implementation is a no-op; must not crash.
    s.writeScanlines(pixels, 16);
}

TEST_CASE("NullViewportSurface::writeScanlines with null pointer is safe", "[Phase67][Scanlines]") {
    NullViewportSurface s(4, 4);
    // Default no-op; must not crash.
    s.writeScanlines(nullptr, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// 2. SoftwareViewportRenderer — writeScanlines is called after fillGrid
// ─────────────────────────────────────────────────────────────────────────────

// Test surface that records writeScanlines calls.
class TrackingViewportSurface final : public IViewportSurface {
public:
    explicit TrackingViewportSurface(uint32_t w, uint32_t h) : m_w(w), m_h(h) {}

    void resize(uint32_t w, uint32_t h) override { m_w = w; m_h = h; }
    void bind() override {}
    void unbind() override {}
    [[nodiscard]] uint32_t colorAttachment() const override { return 1u; }
    [[nodiscard]] bool     isValid()         const override { return m_w > 0 && m_h > 0; }
    [[nodiscard]] uint32_t width()           const override { return m_w; }
    [[nodiscard]] uint32_t height()          const override { return m_h; }

    void writeScanlines(const uint32_t* pixels, size_t count) override {
        ++m_writeCount;
        m_lastPixelCount = count;
        if (pixels && count > 0)
            m_firstPixel = pixels[0];
    }

    uint32_t writeCount()      const { return m_writeCount; }
    size_t   lastPixelCount()  const { return m_lastPixelCount; }
    uint32_t firstPixel()      const { return m_firstPixel; }

private:
    uint32_t m_w, m_h;
    uint32_t m_writeCount    = 0;
    size_t   m_lastPixelCount = 0;
    uint32_t m_firstPixel    = 0;
};

TEST_CASE("SoftwareViewportRenderer calls writeScanlines on the surface", "[Phase67][SoftRenderer]") {
    TrackingViewportSurface surf(64, 64);
    ViewportSlot slot;
    slot.handle = 1;
    slot.state  = ViewportState::Active;

    SoftwareViewportRenderer renderer;
    ViewportSceneState scene;
    renderer.renderGrid(surf, slot, scene);

    CHECK(surf.writeCount() == 1);
    CHECK(surf.lastPixelCount() == 64u * 64u);
}

TEST_CASE("SoftwareViewportRenderer writeScanlines pixel count matches surface dimensions", "[Phase67][SoftRenderer]") {
    TrackingViewportSurface surf(32, 16);
    ViewportSlot slot;
    slot.handle = 2;
    slot.state  = ViewportState::Active;

    SoftwareViewportRenderer renderer;
    ViewportSceneState scene;
    renderer.renderGrid(surf, slot, scene);

    CHECK(surf.lastPixelCount() == 32u * 16u);
    CHECK(renderer.pixelCount() == 32u * 16u);
}

TEST_CASE("SoftwareViewportRenderer internal buffer matches what is passed to writeScanlines", "[Phase67][SoftRenderer]") {
    TrackingViewportSurface surf(8, 8);
    ViewportSlot slot;
    slot.handle = 3;
    slot.state  = ViewportState::Active;

    SoftwareViewportRenderer renderer;
    ViewportSceneState scene;
    renderer.renderGrid(surf, slot, scene);

    // First pixel of internal buffer should match what writeScanlines received.
    REQUIRE(renderer.pixelCount() > 0);
    CHECK(surf.firstPixel() == renderer.pixels()[0]);
}

TEST_CASE("SoftwareViewportRenderer zero-size surface skips writeScanlines", "[Phase67][SoftRenderer]") {
    TrackingViewportSurface surf(0, 0);
    ViewportSlot slot;
    slot.handle = 4;

    SoftwareViewportRenderer renderer;
    ViewportSceneState scene;
    renderer.renderGrid(surf, slot, scene);

    // renderGrid() returns early for zero dimensions — no write should occur.
    CHECK(surf.writeCount() == 0);
}

TEST_CASE("SoftwareViewportRenderer multiple renderGrid calls write each time", "[Phase67][SoftRenderer]") {
    TrackingViewportSurface surf(16, 16);
    ViewportSlot slot;
    slot.handle = 5;
    slot.state  = ViewportState::Active;

    SoftwareViewportRenderer renderer;
    ViewportSceneState scene;
    renderer.renderGrid(surf, slot, scene);
    renderer.renderGrid(surf, slot, scene);
    renderer.renderGrid(surf, slot, scene);

    CHECK(surf.writeCount() == 3);
    CHECK(renderer.renderCount() == 3);
}

// ─────────────────────────────────────────────────────────────────────────────
// 3. GDIViewportSurface::writeScanlines copies pixels into the DIB buffer
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("GDIViewportSurface::writeScanlines is a valid IViewportSurface override", "[Phase67][GDI]") {
    GDIViewportSurface s;
    s.resize(4, 4);
    // On all platforms this must compile and not crash.
    uint32_t pixels[16] = {};
    for (uint32_t i = 0; i < 16; ++i) pixels[i] = 0xFF112233u;
    s.writeScanlines(pixels, 16);
    // Surface remains valid after writeScanlines.
    CHECK(s.isValid());
}

TEST_CASE("GDIViewportSurface::writeScanlines with nullptr does not crash", "[Phase67][GDI]") {
    GDIViewportSurface s;
    s.resize(8, 8);
    s.writeScanlines(nullptr, 0); // must not crash
}

TEST_CASE("GDIViewportSurface is still valid after write + resize cycle", "[Phase67][GDI]") {
    GDIViewportSurface s;
    s.resize(16, 16);
    uint32_t buf[256] = {};
    s.writeScanlines(buf, 256);
    s.resize(32, 32);
    CHECK(s.isValid());
    CHECK(s.width()  == 32);
    CHECK(s.height() == 32);
}

// ─────────────────────────────────────────────────────────────────────────────
// 4. SceneEditorTool camera state accessors
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SceneEditorTool has initial camera position at (0,0,5)", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();
    Vec3 pos = tool.cameraPosition();
    CHECK_THAT(pos.z, WithinAbs(5.f, 0.001f));
    tool.shutdown();
}

TEST_CASE("SceneEditorTool has initial camera yaw of -90 degrees", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();
    CHECK_THAT(tool.cameraYaw(), WithinAbs(-90.f, 0.001f));
    tool.shutdown();
}

TEST_CASE("SceneEditorTool camera position can be set and read back", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();
    tool.setCameraPosition({1.f, 2.f, 3.f});
    Vec3 p = tool.cameraPosition();
    CHECK_THAT(p.x, WithinAbs(1.f, 0.001f));
    CHECK_THAT(p.y, WithinAbs(2.f, 0.001f));
    CHECK_THAT(p.z, WithinAbs(3.f, 0.001f));
    tool.shutdown();
}

TEST_CASE("SceneEditorTool camera yaw and pitch setters round-trip", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();
    tool.setCameraYaw(45.f);
    tool.setCameraPitch(-15.f);
    CHECK_THAT(tool.cameraYaw(),   WithinAbs(45.f,  0.001f));
    CHECK_THAT(tool.cameraPitch(), WithinAbs(-15.f, 0.001f));
    tool.shutdown();
}

TEST_CASE("SceneEditorTool isFlyCamActive returns false when RMB not held", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();
    // No InputSystem attached — fly-cam is always inactive.
    CHECK_FALSE(tool.isFlyCamActive());
    tool.shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 5. SceneEditorTool::provideScene overrideCamera when active
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("provideScene sets overrideCamera=true when tool owns the viewport", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();

    ViewportHandle h = tool.viewportHandle();
    REQUIRE(h != kInvalidViewportHandle);

    // No external scene provider → tool owns the camera.
    const auto* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);
    ViewportSceneState state = tool.provideScene(h, *slot);
    CHECK(state.overrideCamera);

    tool.suspend();
    tool.shutdown();
}

TEST_CASE("provideScene does NOT set overrideCamera when external provider attached", "[Phase67][Camera]") {
    // A minimal scene provider that returns a known state.
    struct StubProvider final : IViewportSceneProvider {
        ViewportSceneState provideScene(ViewportHandle, const ViewportSlot&) override {
            ViewportSceneState s;
            s.hasContent     = true;
            s.overrideCamera = false;  // provider controls the camera itself
            return s;
        }
    } stub;

    SceneEditorTool tool;
    tool.initialize();
    tool.attachSceneProvider(&stub);

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();

    ViewportHandle h = tool.viewportHandle();
    REQUIRE(h != kInvalidViewportHandle);

    const auto* slot = mgr.findSlot(h);
    REQUIRE(slot != nullptr);
    // When a scene provider is attached, provideScene() delegates to it.
    ViewportSceneState state = tool.provideScene(h, *slot);
    // Provider said overrideCamera=false; our delegation must respect that.
    CHECK_FALSE(state.overrideCamera);

    tool.suspend();
    tool.shutdown();
}

// ─────────────────────────────────────────────────────────────────────────────
// 6. SceneEditorTool::update() pushes camera to viewport slot
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SceneEditorTool update with no input does not crash", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();

    // No onAttachInput — update must be safe.
    tool.update(0.016f);

    tool.suspend();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool update with attached InputSystem does not crash", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();

    InputSystem input;
    input.init();
    tool.onAttachInput(&input);

    // No RMB held in headless InputSystem → camera stays still but must not crash.
    tool.update(0.016f);
    tool.update(0.016f);

    tool.onDetachInput();
    input.shutdown();
    tool.suspend();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool update with no provider leaves camera position unchanged when no input key held", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();

    Vec3 startPos{0.f, 0.f, 5.f};
    tool.setCameraPosition(startPos);

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();

    InputSystem input;
    input.init();
    tool.onAttachInput(&input);

    // RMB not pressed in headless InputSystem → camera should not move.
    tool.update(0.016f);

    Vec3 endPos = tool.cameraPosition();
    CHECK_THAT(endPos.x, WithinAbs(startPos.x, 0.001f));
    CHECK_THAT(endPos.y, WithinAbs(startPos.y, 0.001f));
    CHECK_THAT(endPos.z, WithinAbs(startPos.z, 0.001f));

    tool.onDetachInput();
    input.shutdown();
    tool.suspend();
    tool.shutdown();
}

TEST_CASE("SceneEditorTool onDetachInput clears the input pointer safely", "[Phase67][Camera]") {
    SceneEditorTool tool;
    tool.initialize();

    InputSystem input;
    input.init();
    tool.onAttachInput(&input);
    tool.onDetachInput(); // must not crash; subsequent update must be safe

    WorkspaceViewportManager mgr;
    tool.attachViewportManager(&mgr);
    tool.activate();
    tool.update(0.016f); // m_input is null here — must not crash

    input.shutdown();
    tool.suspend();
    tool.shutdown();
}
