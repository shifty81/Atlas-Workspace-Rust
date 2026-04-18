#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// Phase 72 — Viewport entity projection and software renderer scene pass.
//
// Verifies:
//  1. ViewportEntityProxy is populated by NovaForgePreviewRuntime::provideScene()
//  2. ViewportSceneState carries entity list with correct positions
//  3. SoftwareViewportRenderer draws non-background pixels when entities are present
//  4. projectPoint() correctly projects world points to screen space
//  5. Behind-camera entities are culled (projectPoint returns false)
//  6. Selected entities are distinguishable from unselected ones
//  7. End-to-end: provideScene → renderGrid produces entity-coloured pixels

#include "NF/Workspace/IViewportSceneProvider.h"
#include "NF/Workspace/SoftwareViewportRenderer.h"
#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/IViewportSurface.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewRuntime.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewWorld.h"

using Catch::Matchers::WithinAbs;
using namespace NF;
using namespace NovaForge;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Build a ViewportSlot with a camera looking straight down -Z from (0,0,10).
static ViewportSlot makeFrontSlot(float w = 320.f, float h = 240.f)
{
    ViewportSlot slot;
    slot.handle = 1;
    slot.toolId = "test";
    slot.bounds = {0.f, 0.f, w, h};
    slot.state  = ViewportState::Active;
    // Camera at (0, 0, 10) looking along -Z: yaw = -90 + 180 = 90? 
    // Convention: yaw=-90 looks down -Z. So camera at z=10, looking toward z=0:
    // forward = (cos(-90°)*cos(0°), sin(0°), sin(-90°)*cos(0°))
    //         = (0, 0, -1)  ← correct: camera at z=10 looking toward origin
    slot.camera.position   = {0.f, 0.f, 10.f};
    slot.camera.yaw        = -90.f;
    slot.camera.pitch      =   0.f;
    slot.camera.fovDegrees =  60.f;
    slot.camera.nearPlane  =  0.1f;
    slot.camera.farPlane   = 1000.f;
    return slot;
}

// A surface that records the pixel buffer written to it.
class RecordingSurface final : public IViewportSurface {
public:
    explicit RecordingSurface(uint32_t w, uint32_t h) : m_w(w), m_h(h) {
        m_pixels.resize(static_cast<size_t>(w) * h, 0u);
    }
    void resize(uint32_t w, uint32_t h) override {
        m_w = w; m_h = h;
        m_pixels.assign(static_cast<size_t>(w) * h, 0u);
    }
    void bind()   override {}
    void unbind() override {}
    [[nodiscard]] uint32_t colorAttachment() const override { return 1u; }
    [[nodiscard]] bool     isValid()  const override { return m_w > 0 && m_h > 0; }
    [[nodiscard]] uint32_t width()    const override { return m_w; }
    [[nodiscard]] uint32_t height()   const override { return m_h; }

    void writeScanlines(const uint32_t* pixels, size_t count) override {
        if (!pixels) return;
        size_t n = std::min(count, m_pixels.size());
        std::copy(pixels, pixels + n, m_pixels.begin());
        ++m_writeCount;
    }

    [[nodiscard]] uint32_t pixel(uint32_t x, uint32_t y) const {
        if (x >= m_w || y >= m_h) return 0u;
        return m_pixels[y * m_w + x];
    }
    [[nodiscard]] uint32_t writeCount() const { return m_writeCount; }
    [[nodiscard]] const std::vector<uint32_t>& pixels() const { return m_pixels; }

    // Returns true if any pixel in the buffer matches the given colour.
    [[nodiscard]] bool hasPixelColor(uint32_t color) const {
        for (uint32_t p : m_pixels)
            if (p == color) return true;
        return false;
    }

private:
    uint32_t m_w, m_h;
    std::vector<uint32_t> m_pixels;
    uint32_t m_writeCount = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// 1. ViewportEntityProxy population from NovaForgePreviewRuntime
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("provideScene returns empty entity list when world is empty", "[Phase72][EntityProxy]") {
    NovaForgePreviewRuntime rt;
    ViewportSlot slot = makeFrontSlot();
    auto state = rt.provideScene(slot.handle, slot);
    CHECK_FALSE(state.hasContent);
    CHECK(state.entityCount == 0);
    CHECK(state.entities.empty());
}

TEST_CASE("provideScene populates one entity proxy per visible entity", "[Phase72][EntityProxy]") {
    NovaForgePreviewRuntime rt;
    auto id = rt.world().createEntity("Cube");
    rt.world().setTransform(id, {{1.f, 2.f, 3.f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}});

    ViewportSlot slot = makeFrontSlot();
    auto state = rt.provideScene(slot.handle, slot);

    REQUIRE(state.entities.size() == 1);
    CHECK(state.entities[0].x == Catch::Approx(1.f));
    CHECK(state.entities[0].y == Catch::Approx(2.f));
    CHECK(state.entities[0].z == Catch::Approx(3.f));
}

TEST_CASE("provideScene proxy halfW/H/D reflect entity scale * 0.5", "[Phase72][EntityProxy]") {
    NovaForgePreviewRuntime rt;
    auto id = rt.world().createEntity("Box");
    rt.world().setTransform(id, {{0.f, 0.f, 0.f}, {}, {4.f, 2.f, 6.f}});

    ViewportSlot slot = makeFrontSlot();
    auto state = rt.provideScene(slot.handle, slot);

    REQUIRE(state.entities.size() == 1);
    CHECK(state.entities[0].halfW == Catch::Approx(2.f));
    CHECK(state.entities[0].halfH == Catch::Approx(1.f));
    CHECK(state.entities[0].halfD == Catch::Approx(3.f));
}

TEST_CASE("provideScene marks selected entity with selected=true", "[Phase72][EntityProxy]") {
    NovaForgePreviewRuntime rt;
    auto id1 = rt.world().createEntity("A");
    auto id2 = rt.world().createEntity("B");
    rt.world().selectEntity(id2);

    ViewportSlot slot = makeFrontSlot();
    auto state = rt.provideScene(slot.handle, slot);

    REQUIRE(state.entities.size() == 2);
    // Order follows world entity list order
    bool foundSelected = false;
    for (const auto& proxy : state.entities) {
        if (proxy.selected) {
            foundSelected = true;
            CHECK(proxy.label != nullptr);
        }
    }
    CHECK(foundSelected);
}

TEST_CASE("provideScene skips hidden entities", "[Phase72][EntityProxy]") {
    NovaForgePreviewRuntime rt;
    auto id1 = rt.world().createEntity("Visible");
    auto id2 = rt.world().createEntity("Hidden");
    rt.world().setVisible(id2, false);

    ViewportSlot slot = makeFrontSlot();
    auto state = rt.provideScene(slot.handle, slot);

    CHECK(state.entityCount == 2);   // world count still 2
    CHECK(state.entities.size() == 1); // only visible one in proxy list
    CHECK(std::string(state.entities[0].label) == "Visible");
}

TEST_CASE("provideScene multiple entities all get proxy entries", "[Phase72][EntityProxy]") {
    NovaForgePreviewRuntime rt;
    for (int i = 0; i < 5; ++i)
        rt.world().createEntity("E" + std::to_string(i));

    ViewportSlot slot = makeFrontSlot();
    auto state = rt.provideScene(slot.handle, slot);

    CHECK(state.entities.size() == 5);
}

// ─────────────────────────────────────────────────────────────────────────────
// 2. SoftwareViewportRenderer::projectPoint — 3D→2D projection
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("projectPoint: entity at origin projects to screen centre", "[Phase72][Projection]") {
    SoftwareViewportRenderer r;
    ViewportSlot slot = makeFrontSlot(320.f, 240.f);
    // Camera at (0, 0, 10) looking at -Z, so origin (0,0,0) is directly in front.
    float sx = -1.f, sy = -1.f;
    bool vis = r.projectPoint(slot, 320, 240, 0.f, 0.f, 0.f, sx, sy);
    CHECK(vis);
    // Should land very close to the centre pixel
    CHECK_THAT(sx, WithinAbs(160.f, 2.f));
    CHECK_THAT(sy, WithinAbs(120.f, 2.f));
}

TEST_CASE("projectPoint: entity behind camera returns false", "[Phase72][Projection]") {
    SoftwareViewportRenderer r;
    ViewportSlot slot = makeFrontSlot();
    // Camera is at z=10 looking toward -Z. Entity at z=20 is behind the camera.
    float sx = 0.f, sy = 0.f;
    bool vis = r.projectPoint(slot, 320, 240, 0.f, 0.f, 20.f, sx, sy);
    CHECK_FALSE(vis);
}

TEST_CASE("projectPoint: entity to the left projects left of centre", "[Phase72][Projection]") {
    SoftwareViewportRenderer r;
    ViewportSlot slot = makeFrontSlot(320.f, 240.f);
    float sx = 0.f, sy = 0.f;
    bool vis = r.projectPoint(slot, 320, 240, -2.f, 0.f, 0.f, sx, sy);
    CHECK(vis);
    CHECK(sx < 160.f);
}

TEST_CASE("projectPoint: entity to the right projects right of centre", "[Phase72][Projection]") {
    SoftwareViewportRenderer r;
    ViewportSlot slot = makeFrontSlot(320.f, 240.f);
    float sx = 0.f, sy = 0.f;
    bool vis = r.projectPoint(slot, 320, 240, 2.f, 0.f, 0.f, sx, sy);
    CHECK(vis);
    CHECK(sx > 160.f);
}

TEST_CASE("projectPoint: entity above centre projects above centre (lower sy)", "[Phase72][Projection]") {
    SoftwareViewportRenderer r;
    ViewportSlot slot = makeFrontSlot(320.f, 240.f);
    float sx = 0.f, sy = 0.f;
    bool vis = r.projectPoint(slot, 320, 240, 0.f, 2.f, 0.f, sx, sy);
    CHECK(vis);
    // Screen Y is flipped: higher world Y → smaller screen Y (closer to top)
    CHECK(sy < 120.f);
}

TEST_CASE("projectPoint: entity below centre projects below centre (higher sy)", "[Phase72][Projection]") {
    SoftwareViewportRenderer r;
    ViewportSlot slot = makeFrontSlot(320.f, 240.f);
    float sx = 0.f, sy = 0.f;
    bool vis = r.projectPoint(slot, 320, 240, 0.f, -2.f, 0.f, sx, sy);
    CHECK(vis);
    CHECK(sy > 120.f);
}

TEST_CASE("projectPoint: closer entity has larger screen projection than farther one", "[Phase72][Projection]") {
    // A unit-offset entity that is closer to the camera should project farther
    // from the centre than the same offset from a farther position.
    SoftwareViewportRenderer r;
    ViewportSlot slot = makeFrontSlot(320.f, 240.f);

    float sx1, sy1, sx2, sy2;
    // Entity 1m to the right, 2 units from camera
    ViewportSlot close = slot;
    close.camera.position = {0.f, 0.f, 2.f};
    r.projectPoint(close, 320, 240, 1.f, 0.f, 0.f, sx1, sy1);

    // Same entity but camera is 10 units away
    r.projectPoint(slot, 320, 240, 1.f, 0.f, 0.f, sx2, sy2);

    // Closer camera → entity appears farther from centre (larger offset)
    float off1 = std::abs(sx1 - 160.f);
    float off2 = std::abs(sx2 - 160.f);
    CHECK(off1 > off2);
}

TEST_CASE("projectPoint: zero-size viewport returns false", "[Phase72][Projection]") {
    SoftwareViewportRenderer r;
    ViewportSlot slot = makeFrontSlot();
    float sx = 0.f, sy = 0.f;
    CHECK_FALSE(r.projectPoint(slot, 0, 0, 0.f, 0.f, 0.f, sx, sy));
}

// ─────────────────────────────────────────────────────────────────────────────
// 3. SoftwareViewportRenderer — entity pixels are written to surface
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("renderGrid with no entities writes only grid/bg pixels", "[Phase72][Render]") {
    SoftwareViewportRenderer r;
    RecordingSurface surf(64, 64);
    ViewportSlot slot = makeFrontSlot(64.f, 64.f);
    slot.bounds = {0.f, 0.f, 64.f, 64.f};

    ViewportSceneState empty;
    r.renderGrid(surf, slot, empty);

    CHECK(surf.writeCount() == 1);
    CHECK_FALSE(surf.hasPixelColor(r.entityColor));
    CHECK_FALSE(surf.hasPixelColor(r.selectColor));
}

TEST_CASE("renderGrid with entity in front of camera writes entity-coloured pixels", "[Phase72][Render]") {
    SoftwareViewportRenderer r;
    RecordingSurface surf(128, 128);
    ViewportSlot slot = makeFrontSlot(128.f, 128.f);
    slot.bounds = {0.f, 0.f, 128.f, 128.f};

    ViewportSceneState scene;
    scene.hasContent = true;
    ViewportEntityProxy proxy;
    proxy.x = 0.f; proxy.y = 0.f; proxy.z = 0.f; // at origin, in front of camera
    proxy.halfW = proxy.halfH = proxy.halfD = 0.5f;
    proxy.selected = false;
    scene.entities.push_back(proxy);

    r.renderGrid(surf, slot, scene);

    CHECK(surf.hasPixelColor(r.entityColor));
}

TEST_CASE("renderGrid selected entity writes select-coloured pixels", "[Phase72][Render]") {
    SoftwareViewportRenderer r;
    RecordingSurface surf(128, 128);
    ViewportSlot slot = makeFrontSlot(128.f, 128.f);
    slot.bounds = {0.f, 0.f, 128.f, 128.f};

    ViewportSceneState scene;
    scene.hasContent = true;
    ViewportEntityProxy proxy;
    proxy.x = 0.f; proxy.y = 0.f; proxy.z = 0.f;
    proxy.halfW = proxy.halfH = proxy.halfD = 0.5f;
    proxy.selected = true;
    scene.entities.push_back(proxy);

    r.renderGrid(surf, slot, scene);

    CHECK(surf.hasPixelColor(r.selectColor));
}

TEST_CASE("renderGrid behind-camera entity does not write entity pixels", "[Phase72][Render]") {
    SoftwareViewportRenderer r;
    RecordingSurface surf(64, 64);
    ViewportSlot slot = makeFrontSlot(64.f, 64.f);
    // Entity at z=50 is BEHIND camera at z=10 looking toward -Z
    ViewportSceneState scene;
    scene.hasContent = true;
    ViewportEntityProxy proxy;
    proxy.x = 0.f; proxy.y = 0.f; proxy.z = 50.f; // behind camera
    proxy.selected = false;
    scene.entities.push_back(proxy);

    r.renderGrid(surf, slot, scene);

    CHECK_FALSE(surf.hasPixelColor(r.entityColor));
    CHECK_FALSE(surf.hasPixelColor(r.selectColor));
}

// ─────────────────────────────────────────────────────────────────────────────
// 4. End-to-end: NovaForgePreviewRuntime → provideScene → renderGrid
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("E2E: entity in NovaForgePreviewWorld appears as entity pixel in render", "[Phase72][E2E]") {
    NovaForgePreviewRuntime rt;
    auto id = rt.world().createEntity("TestBox");
    // Place entity at origin, directly in front of the camera
    rt.world().setTransform(id, {{0.f, 0.f, 0.f}, {}, {1.f, 1.f, 1.f}});

    ViewportSlot slot = makeFrontSlot(128.f, 128.f);
    auto scene = rt.provideScene(slot.handle, slot);

    REQUIRE_FALSE(scene.entities.empty());

    SoftwareViewportRenderer renderer;
    RecordingSurface surf(128, 128);
    renderer.renderGrid(surf, slot, scene);

    CHECK(surf.hasPixelColor(renderer.entityColor));
}

TEST_CASE("E2E: selected entity in NovaForgePreviewWorld appears as select pixel", "[Phase72][E2E]") {
    NovaForgePreviewRuntime rt;
    auto id = rt.world().createEntity("Selected");
    rt.world().setTransform(id, {{0.f, 0.f, 0.f}, {}, {1.f, 1.f, 1.f}});
    rt.world().selectEntity(id);

    ViewportSlot slot = makeFrontSlot(128.f, 128.f);
    auto scene = rt.provideScene(slot.handle, slot);

    REQUIRE(scene.entities.size() == 1);
    CHECK(scene.entities[0].selected);

    SoftwareViewportRenderer renderer;
    RecordingSurface surf(128, 128);
    renderer.renderGrid(surf, slot, scene);

    CHECK(surf.hasPixelColor(renderer.selectColor));
}

TEST_CASE("E2E: hidden world entity does not produce entity pixel", "[Phase72][E2E]") {
    NovaForgePreviewRuntime rt;
    auto id = rt.world().createEntity("Invisible");
    rt.world().setTransform(id, {{0.f, 0.f, 0.f}, {}, {1.f, 1.f, 1.f}});
    rt.world().setVisible(id, false);

    ViewportSlot slot = makeFrontSlot(128.f, 128.f);
    auto scene = rt.provideScene(slot.handle, slot);

    CHECK(scene.entities.empty());

    SoftwareViewportRenderer renderer;
    RecordingSurface surf(128, 128);
    renderer.renderGrid(surf, slot, scene);

    CHECK_FALSE(surf.hasPixelColor(renderer.entityColor));
}

TEST_CASE("E2E: multiple entities all project and render", "[Phase72][E2E]") {
    NovaForgePreviewRuntime rt;
    // Three entities spread across the viewport
    for (float x : {-2.f, 0.f, 2.f}) {
        auto id = rt.world().createEntity("E");
        rt.world().setTransform(id, {{x, 0.f, 0.f}, {}, {0.5f, 0.5f, 0.5f}});
    }

    ViewportSlot slot = makeFrontSlot(256.f, 256.f);
    auto scene = rt.provideScene(slot.handle, slot);

    CHECK(scene.entities.size() == 3);

    SoftwareViewportRenderer renderer;
    RecordingSurface surf(256, 256);
    renderer.renderGrid(surf, slot, scene);

    CHECK(surf.hasPixelColor(renderer.entityColor));
}
