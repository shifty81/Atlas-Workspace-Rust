#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

// Items 1–3, 5–7, 9 — Workspace headers (no Renderer dependency needed here)
#include "NF/Workspace/IViewportSurface.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include "NF/Workspace/ViewportFrameLoop.h"
#include "NF/Workspace/GizmoRenderer.h"
#include "NF/Workspace/ViewportCompositor.h"

// Item 7 — camera position/yaw/pitch lives in ViewportHostContract
#include "NF/Workspace/ViewportHostContract.h"

// Items 4–5 — AtlasUI viewport panel extensions
#include "NF/UI/AtlasUI/Panels/ViewportPanel.h"
#include "NF/UI/AtlasUI/Contexts.h"
#include "NF/UI/AtlasUI/DrawPrimitives.h"

using Catch::Matchers::WithinAbs;
using namespace NF;
using namespace NF::UI::AtlasUI;

// ─────────────────────────────────────────────────────────────────────────────
// Item 1 — IViewportSurface + ViewportSurfaceRegistry
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("NullViewportSurface default state is invalid", "[Phase64][Surface]") {
    NullViewportSurface s;
    CHECK_FALSE(s.isValid());
    CHECK(s.width() == 0);
    CHECK(s.height() == 0);
    CHECK(s.colorAttachment() == 0);
    CHECK_FALSE(s.isBound());
    CHECK(s.bindCount() == 0);
}

TEST_CASE("NullViewportSurface with dimensions is valid", "[Phase64][Surface]") {
    NullViewportSurface s(1280, 720);
    CHECK(s.isValid());
    CHECK(s.width() == 1280);
    CHECK(s.height() == 720);
}

TEST_CASE("NullViewportSurface resize updates dimensions", "[Phase64][Surface]") {
    NullViewportSurface s(640, 480);
    CHECK(s.isValid());
    s.resize(1920, 1080);
    CHECK(s.width() == 1920);
    CHECK(s.height() == 1080);
}

TEST_CASE("NullViewportSurface bind/unbind tracking", "[Phase64][Surface]") {
    NullViewportSurface s(800, 600);
    CHECK_FALSE(s.isBound());
    s.bind();
    CHECK(s.isBound());
    CHECK(s.bindCount() == 1);
    s.bind();
    CHECK(s.bindCount() == 2);
    s.unbind();
    CHECK_FALSE(s.isBound());
}

TEST_CASE("NullViewportSurface colorAttachment set/get", "[Phase64][Surface]") {
    NullViewportSurface s(800, 600);
    CHECK(s.colorAttachment() == 0);
    s.setColorAttachment(42);
    CHECK(s.colorAttachment() == 42);
}

TEST_CASE("ViewportSurfaceRegistry register and find", "[Phase64][Surface]") {
    ViewportSurfaceRegistry reg;
    NullViewportSurface s(800, 600);
    ViewportHandle h = 1;
    CHECK(reg.registerSurface(h, &s));
    CHECK(reg.surfaceCount() == 1);
    CHECK(reg.findSurface(h) == &s);
}

TEST_CASE("ViewportSurfaceRegistry rejects duplicate handle", "[Phase64][Surface]") {
    ViewportSurfaceRegistry reg;
    NullViewportSurface s1(800, 600), s2(800, 600);
    ViewportHandle h = 1;
    CHECK(reg.registerSurface(h, &s1));
    CHECK_FALSE(reg.registerSurface(h, &s2));
    CHECK(reg.surfaceCount() == 1);
}

TEST_CASE("ViewportSurfaceRegistry rejects null surface", "[Phase64][Surface]") {
    ViewportSurfaceRegistry reg;
    CHECK_FALSE(reg.registerSurface(1, nullptr));
    CHECK(reg.surfaceCount() == 0);
}

TEST_CASE("ViewportSurfaceRegistry rejects invalid handle", "[Phase64][Surface]") {
    ViewportSurfaceRegistry reg;
    NullViewportSurface s(800, 600);
    CHECK_FALSE(reg.registerSurface(kInvalidViewportHandle, &s));
}

TEST_CASE("ViewportSurfaceRegistry unregister", "[Phase64][Surface]") {
    ViewportSurfaceRegistry reg;
    NullViewportSurface s(800, 600);
    reg.registerSurface(1, &s);
    CHECK(reg.unregisterSurface(1));
    CHECK(reg.surfaceCount() == 0);
    CHECK_FALSE(reg.unregisterSurface(1));
}

TEST_CASE("ViewportSurfaceRegistry resize delegates", "[Phase64][Surface]") {
    ViewportSurfaceRegistry reg;
    NullViewportSurface s(800, 600);
    reg.registerSurface(1, &s);
    CHECK(reg.resize(1, 1920, 1080));
    CHECK(s.width() == 1920);
    CHECK(s.height() == 1080);
    CHECK_FALSE(reg.resize(99, 100, 100)); // unknown handle
}

TEST_CASE("ViewportSurfaceRegistry capacity limit", "[Phase64][Surface]") {
    ViewportSurfaceRegistry reg;
    std::vector<NullViewportSurface> surfaces(ViewportSurfaceRegistry::kMaxSurfaces + 1, NullViewportSurface(10, 10));
    for (size_t i = 0; i < ViewportSurfaceRegistry::kMaxSurfaces; ++i)
        CHECK(reg.registerSurface(static_cast<ViewportHandle>(i + 1), &surfaces[i]));
    CHECK_FALSE(reg.registerSurface(static_cast<ViewportHandle>(ViewportSurfaceRegistry::kMaxSurfaces + 1),
                                    &surfaces.back()));
}

TEST_CASE("ViewportSurfaceRegistry clear", "[Phase64][Surface]") {
    ViewportSurfaceRegistry reg;
    NullViewportSurface s(10, 10);
    reg.registerSurface(1, &s);
    reg.clear();
    CHECK(reg.surfaceCount() == 0);
    CHECK(reg.findSurface(1) == nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
// Item 2 — IViewportSceneProvider + ViewportSceneProviderRegistry
// ─────────────────────────────────────────────────────────────────────────────

// Minimal concrete provider for testing
struct TestSceneProvider : public IViewportSceneProvider {
    ViewportSceneState result;
    int callCount = 0;
    ViewportSceneState provideScene(ViewportHandle, const ViewportSlot&) override {
        ++callCount;
        return result;
    }
};

TEST_CASE("ViewportSceneProviderRegistry register and find", "[Phase64][SceneProvider]") {
    ViewportSceneProviderRegistry reg;
    TestSceneProvider p;
    CHECK(reg.registerProvider("scene_tool", &p));
    CHECK(reg.providerCount() == 1);
    CHECK(reg.findProvider("scene_tool") == &p);
}

TEST_CASE("ViewportSceneProviderRegistry rejects duplicate toolId", "[Phase64][SceneProvider]") {
    ViewportSceneProviderRegistry reg;
    TestSceneProvider p1, p2;
    CHECK(reg.registerProvider("tool", &p1));
    CHECK_FALSE(reg.registerProvider("tool", &p2));
}

TEST_CASE("ViewportSceneProviderRegistry rejects empty toolId", "[Phase64][SceneProvider]") {
    ViewportSceneProviderRegistry reg;
    TestSceneProvider p;
    CHECK_FALSE(reg.registerProvider("", &p));
}

TEST_CASE("ViewportSceneProviderRegistry rejects null provider", "[Phase64][SceneProvider]") {
    ViewportSceneProviderRegistry reg;
    CHECK_FALSE(reg.registerProvider("tool", nullptr));
}

TEST_CASE("ViewportSceneProviderRegistry unregister", "[Phase64][SceneProvider]") {
    ViewportSceneProviderRegistry reg;
    TestSceneProvider p;
    reg.registerProvider("tool", &p);
    CHECK(reg.unregisterProvider("tool"));
    CHECK(reg.providerCount() == 0);
    CHECK_FALSE(reg.unregisterProvider("tool"));
}

TEST_CASE("ViewportSceneProviderRegistry dispatchProvide finds correct provider", "[Phase64][SceneProvider]") {
    ViewportSceneProviderRegistry reg;
    TestSceneProvider pa, pb;
    pa.result.hasContent  = true;
    pa.result.entityCount = 5;
    pb.result.hasContent  = false;
    pb.result.entityCount = 0;

    reg.registerProvider("tool_a", &pa);
    reg.registerProvider("tool_b", &pb);

    ViewportSlot slotA; slotA.handle = 1; slotA.toolId = "tool_a";
    ViewportSlot slotB; slotB.handle = 2; slotB.toolId = "tool_b";

    auto stA = reg.dispatchProvide(slotA);
    CHECK(stA.hasContent);
    CHECK(stA.entityCount == 5);
    CHECK(pa.callCount == 1);
    CHECK(pb.callCount == 0);

    auto stB = reg.dispatchProvide(slotB);
    CHECK_FALSE(stB.hasContent);
    CHECK(pb.callCount == 1);
}

TEST_CASE("ViewportSceneProviderRegistry dispatchProvide returns empty state when no provider", "[Phase64][SceneProvider]") {
    ViewportSceneProviderRegistry reg;
    ViewportSlot slot; slot.handle = 1; slot.toolId = "unknown_tool";
    auto st = reg.dispatchProvide(slot);
    CHECK_FALSE(st.hasContent);
    CHECK(st.entityCount == 0);
}

TEST_CASE("ViewportSceneProviderRegistry capacity limit", "[Phase64][SceneProvider]") {
    ViewportSceneProviderRegistry reg;
    std::vector<TestSceneProvider> providers(ViewportSceneProviderRegistry::kMaxProviders + 1);
    for (size_t i = 0; i < ViewportSceneProviderRegistry::kMaxProviders; ++i)
        CHECK(reg.registerProvider("tool_" + std::to_string(i), &providers[i]));
    CHECK_FALSE(reg.registerProvider("overflow", &providers.back()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Item 3 — ViewportFrameLoop
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ViewportFrameLoop default state", "[Phase64][FrameLoop]") {
    ViewportFrameLoop loop;
    CHECK(loop.viewportRegistry() == nullptr);
    CHECK(loop.sceneRegistry() == nullptr);
    CHECK(loop.surfaceRegistry() == nullptr);
    CHECK(loop.stats().frameNumber == 0);
}

TEST_CASE("ViewportFrameLoop renderFrame with null registry returns empty", "[Phase64][FrameLoop]") {
    ViewportFrameLoop loop;
    auto results = loop.renderFrame();
    CHECK(results.empty());
    CHECK(loop.stats().frameNumber == 1);
}

TEST_CASE("ViewportFrameLoop renderFrame increments frameNumber", "[Phase64][FrameLoop]") {
    ViewportFrameLoop loop;
    ViewportHostRegistry reg;
    loop.setViewportRegistry(&reg);

    loop.renderFrame();
    loop.renderFrame();
    loop.renderFrame();
    CHECK(loop.stats().frameNumber == 3);
}

TEST_CASE("ViewportFrameLoop skips non-active slots", "[Phase64][FrameLoop]") {
    ViewportHostRegistry viewReg;
    ViewportFrameLoop loop;
    loop.setViewportRegistry(&viewReg);

    ViewportBounds b{0, 0, 800, 600};
    (void)viewReg.requestSlot("tool", b); // Idle, not activated

    auto results = loop.renderFrame();
    CHECK(results.empty());
    CHECK(loop.stats().activeSlots == 0);
}

TEST_CASE("ViewportFrameLoop renders active slot with valid surface", "[Phase64][FrameLoop]") {
    ViewportHostRegistry viewReg;
    ViewportSurfaceRegistry surfReg;
    ViewportSceneProviderRegistry sceneReg;
    TestSceneProvider provider;
    provider.result.hasContent  = true;
    provider.result.entityCount = 3;

    ViewportFrameLoop loop;
    loop.setViewportRegistry(&viewReg);
    loop.setSurfaceRegistry(&surfReg);
    loop.setSceneRegistry(&sceneReg);

    ViewportBounds b{0, 0, 800, 600};
    auto h = viewReg.requestSlot("my_tool", b);
    viewReg.activateSlot(h);

    NullViewportSurface surface(800, 600);
    surface.setColorAttachment(7);
    surfReg.registerSurface(h, &surface);
    sceneReg.registerProvider("my_tool", &provider);

    auto results = loop.renderFrame();
    REQUIRE(results.size() == 1);
    CHECK(results[0].handle == h);
    CHECK(results[0].rendered);
    CHECK(results[0].colorAttachmentId == 7);
    CHECK(results[0].sceneState.hasContent);
    CHECK(results[0].sceneState.entityCount == 3);
    CHECK(loop.stats().renderedSlots == 1);
    CHECK(loop.stats().skippedSlots == 0);
    CHECK(surface.bindCount() == 1);
    CHECK_FALSE(surface.isBound()); // unbind was called
}

TEST_CASE("ViewportFrameLoop skips active slot with no surface registered", "[Phase64][FrameLoop]") {
    ViewportHostRegistry viewReg;
    ViewportSurfaceRegistry surfReg;
    ViewportFrameLoop loop;
    loop.setViewportRegistry(&viewReg);
    loop.setSurfaceRegistry(&surfReg);

    ViewportBounds b{0, 0, 800, 600};
    auto h = viewReg.requestSlot("tool", b);
    viewReg.activateSlot(h);

    auto results = loop.renderFrame();
    REQUIRE(results.size() == 1);
    CHECK_FALSE(results[0].rendered);
    CHECK(results[0].colorAttachmentId == 0);
    CHECK(loop.stats().skippedSlots == 1);
}

TEST_CASE("ViewportFrameLoop onFrameRendered increments slot frameCount", "[Phase64][FrameLoop]") {
    ViewportHostRegistry viewReg;
    ViewportSurfaceRegistry surfReg;
    NullViewportSurface surface(800, 600);
    surface.setColorAttachment(1);

    ViewportFrameLoop loop;
    loop.setViewportRegistry(&viewReg);
    loop.setSurfaceRegistry(&surfReg);

    auto h = viewReg.requestSlot("tool", {0, 0, 800, 600});
    viewReg.activateSlot(h);
    surfReg.registerSurface(h, &surface);

    loop.renderFrame();
    loop.renderFrame();
    const auto* slot = viewReg.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK(slot->frameCount == 2);
}

TEST_CASE("ViewportFrameLoop resetStats preserves frameNumber", "[Phase64][FrameLoop]") {
    ViewportFrameLoop loop;
    loop.renderFrame();
    loop.renderFrame();
    REQUIRE(loop.stats().frameNumber == 2);
    loop.resetStats();
    CHECK(loop.stats().frameNumber == 2); // preserved
    CHECK(loop.stats().activeSlots == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Item 4 — DrawImageCmd + ViewportPanel::paint() with colorAttachment
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("DrawImageCmd stores rect, textureId, and tint", "[Phase64][DrawImage]") {
    DrawImageCmd cmd{{10, 20, 300, 200}, 42, 0xFFFFFFFF};
    CHECK(cmd.rect.x == 10.f);
    CHECK(cmd.textureId == 42u);
    CHECK(cmd.tint == 0xFFFFFFFFu);
}

TEST_CASE("DrawCommand variant holds DrawImageCmd", "[Phase64][DrawImage]") {
    DrawCommand cmd = DrawImageCmd{{0, 0, 100, 100}, 5, 0xFF0000FF};
    CHECK(std::holds_alternative<DrawImageCmd>(cmd));
    auto& img = std::get<DrawImageCmd>(cmd);
    CHECK(img.textureId == 5);
}

TEST_CASE("BasicPaintContext drawImage pushes DrawImageCmd to DrawList", "[Phase64][DrawImage]") {
    BasicPaintContext ctx;
    ctx.drawImage({0, 0, 100, 100}, 42, 0xFFFFFFFF);
    REQUIRE(ctx.drawList().size() == 1);
    REQUIRE(std::holds_alternative<DrawImageCmd>(ctx.drawList().commands()[0]));
    auto& cmd = std::get<DrawImageCmd>(ctx.drawList().commands()[0]);
    CHECK(cmd.textureId == 42u);
}

TEST_CASE("ViewportPanel default colorAttachment is 0", "[Phase64][ViewportPanel]") {
    ViewportPanel vp;
    CHECK(vp.colorAttachment() == 0);
}

TEST_CASE("ViewportPanel setColorAttachment/colorAttachment round-trip", "[Phase64][ViewportPanel]") {
    ViewportPanel vp;
    vp.setColorAttachment(99);
    CHECK(vp.colorAttachment() == 99);
}

TEST_CASE("ViewportPanel paint emits DrawImageCmd when colorAttachment is set", "[Phase64][ViewportPanel]") {
    ViewportPanel vp;
    vp.setVisible(true);
    vp.arrange({0, 0, 800, 600});
    vp.setColorAttachment(7);

    BasicPaintContext ctx;
    vp.paint(ctx);

    bool found = false;
    for (const auto& cmd : ctx.drawList().commands()) {
        if (std::holds_alternative<DrawImageCmd>(cmd)) {
            found = true;
            auto& img = std::get<DrawImageCmd>(cmd);
            CHECK(img.textureId == 7u);
        }
    }
    CHECK(found);
}

TEST_CASE("ViewportPanel paint shows grid when no colorAttachment and grid enabled", "[Phase64][ViewportPanel]") {
    ViewportPanel vp;
    vp.setVisible(true);
    vp.arrange({0, 0, 800, 600});
    vp.setGridEnabled(true);
    // colorAttachment = 0 (default)

    BasicPaintContext ctx;
    vp.paint(ctx);

    // Should not have a DrawImageCmd
    for (const auto& cmd : ctx.drawList().commands())
        CHECK_FALSE(std::holds_alternative<DrawImageCmd>(cmd));

    // Should have multiple FillRectCmds (grid lines)
    size_t fillCount = 0;
    for (const auto& cmd : ctx.drawList().commands())
        if (std::holds_alternative<FillRectCmd>(cmd)) ++fillCount;
    CHECK(fillCount > 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Item 5 — Resize callback
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ViewportPanel hasResizeCallback is false by default", "[Phase64][Resize]") {
    ViewportPanel vp;
    CHECK_FALSE(vp.hasResizeCallback());
}

TEST_CASE("ViewportPanel setResizeCallback installs callback", "[Phase64][Resize]") {
    ViewportPanel vp;
    bool fired = false;
    vp.setResizeCallback([&](float, float, float, float) { fired = true; });
    CHECK(vp.hasResizeCallback());
}

TEST_CASE("ViewportPanel resize callback fires on first paint", "[Phase64][Resize]") {
    ViewportPanel vp;
    float cbX = -1, cbY = -1, cbW = -1, cbH = -1;
    vp.setResizeCallback([&](float x, float y, float w, float h) {
        cbX = x; cbY = y; cbW = w; cbH = h;
    });
    vp.setVisible(true);
    vp.arrange({10, 20, 800, 600});

    BasicPaintContext ctx;
    vp.paint(ctx);

    CHECK_THAT(cbX, WithinAbs(10.f, 1e-4f));
    CHECK_THAT(cbY, WithinAbs(20.f, 1e-4f));
    CHECK_THAT(cbW, WithinAbs(800.f, 1e-4f));
    CHECK_THAT(cbH, WithinAbs(600.f, 1e-4f));
}

TEST_CASE("ViewportPanel resize callback does not fire when bounds unchanged", "[Phase64][Resize]") {
    ViewportPanel vp;
    int callCount = 0;
    vp.setResizeCallback([&](float, float, float, float) { ++callCount; });
    vp.setVisible(true);
    vp.arrange({0, 0, 800, 600});

    BasicPaintContext ctx;
    vp.paint(ctx); // fires once
    vp.paint(ctx); // same bounds → should not fire again
    CHECK(callCount == 1);
}

TEST_CASE("ViewportPanel resize callback fires again after bounds change", "[Phase64][Resize]") {
    ViewportPanel vp;
    int callCount = 0;
    vp.setResizeCallback([&](float, float, float, float) { ++callCount; });
    vp.setVisible(true);
    vp.arrange({0, 0, 800, 600});

    BasicPaintContext ctx;
    vp.paint(ctx);           // first paint → fires
    vp.arrange({0, 0, 1920, 1080});
    vp.paint(ctx);           // new bounds → fires again
    CHECK(callCount == 2);
}

// ─────────────────────────────────────────────────────────────────────────────
// Item 5 — Resize drives ViewportHostRegistry::updateBounds()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ViewportPanel resize callback wires into ViewportHostRegistry::updateBounds", "[Phase64][Resize]") {
    ViewportHostRegistry viewReg;
    ViewportBounds initialBounds{0, 0, 800, 600};
    auto h = viewReg.requestSlot("tool", initialBounds);
    viewReg.activateSlot(h);

    ViewportPanel vp;
    vp.setViewportHandle(h);
    vp.setResizeCallback([&](float x, float y, float w, float h_) {
        viewReg.updateBounds(h, {x, y, w, h_});
    });

    vp.setVisible(true);
    vp.arrange({0, 0, 1280, 720});

    BasicPaintContext ctx;
    vp.paint(ctx);

    const auto* slot = viewReg.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK_THAT(slot->bounds.width,  WithinAbs(1280.f, 1e-4f));
    CHECK_THAT(slot->bounds.height, WithinAbs(720.f,  1e-4f));
}

// ─────────────────────────────────────────────────────────────────────────────
// Item 5 — viewportHandle accessors
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ViewportPanel viewportHandle default is 0", "[Phase64][ViewportPanel]") {
    ViewportPanel vp;
    CHECK(vp.viewportHandle() == 0u);
}

TEST_CASE("ViewportPanel setViewportHandle/viewportHandle round-trip", "[Phase64][ViewportPanel]") {
    ViewportPanel vp;
    vp.setViewportHandle(3);
    CHECK(vp.viewportHandle() == 3u);
}

// ─────────────────────────────────────────────────────────────────────────────
// Item 6 — GizmoRenderer
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("GizmoAxis name helpers", "[Phase64][Gizmo]") {
    CHECK(std::string(gizmoAxisName(GizmoAxis::X))   == "X");
    CHECK(std::string(gizmoAxisName(GizmoAxis::Y))   == "Y");
    CHECK(std::string(gizmoAxisName(GizmoAxis::Z))   == "Z");
    CHECK(std::string(gizmoAxisName(GizmoAxis::XY))  == "XY");
    CHECK(std::string(gizmoAxisName(GizmoAxis::XZ))  == "XZ");
    CHECK(std::string(gizmoAxisName(GizmoAxis::YZ))  == "YZ");
    CHECK(std::string(gizmoAxisName(GizmoAxis::XYZ)) == "XYZ");
}

TEST_CASE("GizmoType name helpers", "[Phase64][Gizmo]") {
    CHECK(std::string(gizmoTypeName(GizmoType::Translate)) == "Translate");
    CHECK(std::string(gizmoTypeName(GizmoType::Rotate))    == "Rotate");
    CHECK(std::string(gizmoTypeName(GizmoType::Scale))     == "Scale");
}

TEST_CASE("GizmoRenderer default state", "[Phase64][Gizmo]") {
    GizmoRenderer gr;
    CHECK(gr.commandCount() == 0);
    CHECK_FALSE(gr.hasHighlightedGizmo());
}

TEST_CASE("GizmoRenderer addGizmo and commandCount", "[Phase64][Gizmo]") {
    GizmoRenderer gr;
    gr.addGizmo({GizmoAxis::X, GizmoType::Translate, {1, 2, 3}, 1.f, false});
    gr.addGizmo({GizmoAxis::Y, GizmoType::Rotate,    {0, 0, 0}, 0.5f, true});
    CHECK(gr.commandCount() == 2);
}

TEST_CASE("GizmoRenderer clear removes commands", "[Phase64][Gizmo]") {
    GizmoRenderer gr;
    gr.addGizmo({GizmoAxis::X, GizmoType::Scale, {}, 1.f, false});
    gr.clear();
    CHECK(gr.commandCount() == 0);
}

TEST_CASE("GizmoRenderer hasHighlightedGizmo detects highlight", "[Phase64][Gizmo]") {
    GizmoRenderer gr;
    gr.addGizmo({GizmoAxis::X, GizmoType::Translate, {}, 1.f, false});
    CHECK_FALSE(gr.hasHighlightedGizmo());
    gr.addGizmo({GizmoAxis::Y, GizmoType::Translate, {}, 1.f, true});
    CHECK(gr.hasHighlightedGizmo());
}

TEST_CASE("GizmoRenderer renderToSurface returns 0 for invalid surface", "[Phase64][Gizmo]") {
    GizmoRenderer gr;
    gr.addGizmo({GizmoAxis::XYZ, GizmoType::Translate, {}, 1.f, false});
    NullViewportSurface invalid; // 0×0 = invalid
    CHECK(gr.renderToSurface(invalid, 1) == 0);
}

TEST_CASE("GizmoRenderer renderToSurface returns command count for valid surface", "[Phase64][Gizmo]") {
    GizmoRenderer gr;
    gr.addGizmo({GizmoAxis::X, GizmoType::Translate, {}, 1.f, false});
    gr.addGizmo({GizmoAxis::Y, GizmoType::Scale,     {}, 1.f, false});
    NullViewportSurface surface(800, 600);
    CHECK(gr.renderToSurface(surface, 1) == 2);
}

TEST_CASE("GizmoRenderer capacity limit kMaxGizmos", "[Phase64][Gizmo]") {
    GizmoRenderer gr;
    for (size_t i = 0; i <= GizmoRenderer::kMaxGizmos + 5; ++i)
        gr.addGizmo({GizmoAxis::X, GizmoType::Translate, {}, 1.f, false});
    CHECK(gr.commandCount() == GizmoRenderer::kMaxGizmos);
}

// ─────────────────────────────────────────────────────────────────────────────
// Item 7 — ViewportCameraDescriptor extended with position/yaw/pitch
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ViewportCameraDescriptor has default position", "[Phase64][Camera]") {
    ViewportCameraDescriptor cam;
    CHECK_THAT(cam.position.x, WithinAbs(0.f, 1e-5f));
    CHECK_THAT(cam.position.y, WithinAbs(0.f, 1e-5f));
    CHECK_THAT(cam.position.z, WithinAbs(5.f, 1e-5f));
}

TEST_CASE("ViewportCameraDescriptor has default yaw and pitch", "[Phase64][Camera]") {
    ViewportCameraDescriptor cam;
    CHECK_THAT(cam.yaw,   WithinAbs(-90.f, 1e-5f));
    CHECK_THAT(cam.pitch, WithinAbs(0.f,   1e-5f));
}

TEST_CASE("ViewportCameraDescriptor position round-trip via slot", "[Phase64][Camera]") {
    ViewportHostRegistry reg;
    auto h = reg.requestSlot("tool", {0, 0, 800, 600});

    ViewportCameraDescriptor cam;
    cam.position = {10.f, 5.f, -3.f};
    cam.yaw   = 45.f;
    cam.pitch = -15.f;
    CHECK(reg.setCamera(h, cam));

    const auto* slot = reg.findSlot(h);
    REQUIRE(slot != nullptr);
    CHECK_THAT(slot->camera.position.x, WithinAbs(10.f,  1e-5f));
    CHECK_THAT(slot->camera.position.y, WithinAbs(5.f,   1e-5f));
    CHECK_THAT(slot->camera.position.z, WithinAbs(-3.f,  1e-5f));
    CHECK_THAT(slot->camera.yaw,        WithinAbs(45.f,  1e-5f));
    CHECK_THAT(slot->camera.pitch,      WithinAbs(-15.f, 1e-5f));
}

TEST_CASE("ViewportCameraDescriptor isValid unchanged by position", "[Phase64][Camera]") {
    ViewportCameraDescriptor cam;
    cam.position = {1000.f, -500.f, 200.f};
    cam.yaw   = 270.f;
    cam.pitch = -89.f;
    CHECK(cam.isValid()); // validity only checks FOV + near/far
}

// ─────────────────────────────────────────────────────────────────────────────
// Item 9 — ViewportCompositor + ViewportLayoutMode
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ViewportLayoutMode name helpers", "[Phase64][Compositor]") {
    CHECK(std::string(viewportLayoutModeName(ViewportLayoutMode::Single))     == "Single");
    CHECK(std::string(viewportLayoutModeName(ViewportLayoutMode::SideBySide)) == "SideBySide");
    CHECK(std::string(viewportLayoutModeName(ViewportLayoutMode::TwoByTwo))   == "TwoByTwo");
    CHECK(std::string(viewportLayoutModeName(ViewportLayoutMode::ThreeLeft))  == "ThreeLeft");
    CHECK(std::string(viewportLayoutModeName(ViewportLayoutMode::ThreeRight)) == "ThreeRight");
}

TEST_CASE("ViewportCompositor default layout is Single", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    CHECK(comp.layoutMode() == ViewportLayoutMode::Single);
    CHECK(comp.regionCount() == 1);
    CHECK(std::string(comp.layoutName()) == "Single");
}

TEST_CASE("ViewportCompositor regionCount per layout", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    comp.setLayoutMode(ViewportLayoutMode::Single);
    CHECK(comp.regionCount() == 1);
    comp.setLayoutMode(ViewportLayoutMode::SideBySide);
    CHECK(comp.regionCount() == 2);
    comp.setLayoutMode(ViewportLayoutMode::TwoByTwo);
    CHECK(comp.regionCount() == 4);
    comp.setLayoutMode(ViewportLayoutMode::ThreeLeft);
    CHECK(comp.regionCount() == 3);
    comp.setLayoutMode(ViewportLayoutMode::ThreeRight);
    CHECK(comp.regionCount() == 3);
}

TEST_CASE("ViewportCompositor Single layout fills full bounds", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    comp.setLayoutMode(ViewportLayoutMode::Single);
    ViewportBounds full{0, 0, 1280, 720};
    auto result = comp.computeSlotBounds(full, {1});
    REQUIRE(result.size() == 1);
    CHECK(result[0].handle == 1u);
    CHECK_THAT(result[0].bounds.width,  WithinAbs(1280.f, 1e-3f));
    CHECK_THAT(result[0].bounds.height, WithinAbs(720.f,  1e-3f));
}

TEST_CASE("ViewportCompositor SideBySide splits horizontally", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    comp.setLayoutMode(ViewportLayoutMode::SideBySide);
    ViewportBounds full{0, 0, 1000, 600};
    auto result = comp.computeSlotBounds(full, {1, 2});
    REQUIRE(result.size() == 2);
    CHECK_THAT(result[0].bounds.width, WithinAbs(500.f, 1e-3f));
    CHECK_THAT(result[1].bounds.width, WithinAbs(500.f, 1e-3f));
    CHECK_THAT(result[0].bounds.x, WithinAbs(0.f,   1e-3f));
    CHECK_THAT(result[1].bounds.x, WithinAbs(500.f, 1e-3f));
}

TEST_CASE("ViewportCompositor TwoByTwo produces quad regions", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    comp.setLayoutMode(ViewportLayoutMode::TwoByTwo);
    ViewportBounds full{0, 0, 1000, 800};
    auto result = comp.computeSlotBounds(full, {1, 2, 3, 4});
    REQUIRE(result.size() == 4);
    for (const auto& r : result) {
        CHECK_THAT(r.bounds.width,  WithinAbs(500.f, 1e-3f));
        CHECK_THAT(r.bounds.height, WithinAbs(400.f, 1e-3f));
    }
}

TEST_CASE("ViewportCompositor ThreeLeft layout proportions", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    comp.setLayoutMode(ViewportLayoutMode::ThreeLeft);
    ViewportBounds full{0, 0, 900, 600};
    auto result = comp.computeSlotBounds(full, {1, 2, 3});
    REQUIRE(result.size() == 3);
    // Left region = ⅔ of width
    CHECK_THAT(result[0].bounds.width, WithinAbs(600.f, 1e-3f));
    CHECK_THAT(result[0].bounds.height, WithinAbs(600.f, 1e-3f));
    // Right stacked = ⅓ of width, each ½ height
    CHECK_THAT(result[1].bounds.width, WithinAbs(300.f, 1e-3f));
    CHECK_THAT(result[1].bounds.height, WithinAbs(300.f, 1e-3f));
    CHECK_THAT(result[2].bounds.width, WithinAbs(300.f, 1e-3f));
    CHECK_THAT(result[2].bounds.height, WithinAbs(300.f, 1e-3f));
}

TEST_CASE("ViewportCompositor ThreeRight layout proportions", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    comp.setLayoutMode(ViewportLayoutMode::ThreeRight);
    ViewportBounds full{0, 0, 900, 600};
    auto result = comp.computeSlotBounds(full, {1, 2, 3});
    REQUIRE(result.size() == 3);
    // Left stacked = ⅓ width each ½ height
    CHECK_THAT(result[0].bounds.width, WithinAbs(300.f, 1e-3f));
    CHECK_THAT(result[1].bounds.width, WithinAbs(300.f, 1e-3f));
    // Right region = ⅔ width, full height
    CHECK_THAT(result[2].bounds.width,  WithinAbs(600.f, 1e-3f));
    CHECK_THAT(result[2].bounds.height, WithinAbs(600.f, 1e-3f));
}

TEST_CASE("ViewportCompositor fewer handles than regions clips result", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    comp.setLayoutMode(ViewportLayoutMode::TwoByTwo);
    ViewportBounds full{0, 0, 800, 600};
    auto result = comp.computeSlotBounds(full, {1, 2}); // only 2 handles for 4 regions
    CHECK(result.size() == 2);
}

TEST_CASE("ViewportCompositor more handles than regions ignores excess", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    comp.setLayoutMode(ViewportLayoutMode::SideBySide);
    ViewportBounds full{0, 0, 800, 600};
    auto result = comp.computeSlotBounds(full, {1, 2, 3, 4}); // 4 handles for 2 regions
    CHECK(result.size() == 2);
}

TEST_CASE("ViewportCompositor slot bounds handle IDs are preserved", "[Phase64][Compositor]") {
    ViewportCompositor comp;
    comp.setLayoutMode(ViewportLayoutMode::TwoByTwo);
    ViewportBounds full{0, 0, 800, 600};
    auto result = comp.computeSlotBounds(full, {10, 20, 30, 40});
    REQUIRE(result.size() == 4);
    CHECK(result[0].handle == 10u);
    CHECK(result[1].handle == 20u);
    CHECK(result[2].handle == 30u);
    CHECK(result[3].handle == 40u);
}
