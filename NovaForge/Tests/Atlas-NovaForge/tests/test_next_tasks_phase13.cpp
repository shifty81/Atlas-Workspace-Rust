#include "../engine/render/EditorViewportFramebuffer.h"
#include "../engine/render/GLViewportFramebuffer.h"
#include "../engine/ui/UIScreenGraph.h"
#include "../engine/ui/UIManager.h"
#include "../engine/core/Engine.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas;
using namespace atlas::render;
using namespace atlas::ui;

// ---------------------------------------------------------------
// GLViewportFramebuffer tests (unit-level, no GL context needed)
// ---------------------------------------------------------------

void test_gl_viewport_fb_default_state() {
    GLViewportFramebuffer fb;
    assert(fb.GetWidth() == 0);
    assert(fb.GetHeight() == 0);
    assert(fb.GetColorAttachment() == 0);
    assert(!fb.IsValid());
    std::cout << "[PASS] test_gl_viewport_fb_default_state" << std::endl;
}

void test_gl_viewport_fb_create_requires_nonzero() {
    GLViewportFramebuffer fb;
    // Without a GL context Create will fail, but zero-size must
    // return false immediately without touching GL.
    bool result = fb.Create(0, 0);
    assert(!result);
    assert(!fb.IsValid());
    std::cout << "[PASS] test_gl_viewport_fb_create_requires_nonzero" << std::endl;
}

void test_gl_viewport_fb_resize_to_zero() {
    GLViewportFramebuffer fb;
    fb.Resize(0, 0);
    assert(!fb.IsValid());
    assert(fb.GetWidth() == 0);
    assert(fb.GetHeight() == 0);
    std::cout << "[PASS] test_gl_viewport_fb_resize_to_zero" << std::endl;
}

void test_gl_viewport_fb_bind_unbind_safe() {
    // Bind/Unbind should not crash even without a GL context
    // (function pointers will be null, guarded by if-checks).
    GLViewportFramebuffer fb;
    fb.Bind();
    fb.Unbind();
    std::cout << "[PASS] test_gl_viewport_fb_bind_unbind_safe" << std::endl;
}

// ---------------------------------------------------------------
// UIScreen::ScaleLayout tests
// ---------------------------------------------------------------

void test_scale_layout_basic() {
    UIScreen screen;
    screen.Init("test");
    uint32_t w1 = screen.AddWidget(UIWidgetType::Panel, "P1",
                                     0.0f, 0.0f, 640.0f, 360.0f);
    uint32_t w2 = screen.AddWidget(UIWidgetType::Button, "B1",
                                     320.0f, 180.0f, 100.0f, 50.0f);

    // Double the viewport size
    screen.ScaleLayout(1280.0f, 720.0f, 2560.0f, 1440.0f);

    auto* p1 = screen.GetWidget(w1);
    auto* p2 = screen.GetWidget(w2);
    assert(p1 && p2);

    // All dimensions should double
    assert(std::abs(p1->width - 1280.0f) < 0.01f);
    assert(std::abs(p1->height - 720.0f) < 0.01f);
    assert(std::abs(p2->x - 640.0f) < 0.01f);
    assert(std::abs(p2->y - 360.0f) < 0.01f);
    assert(std::abs(p2->width - 200.0f) < 0.01f);
    assert(std::abs(p2->height - 100.0f) < 0.01f);

    std::cout << "[PASS] test_scale_layout_basic" << std::endl;
}

void test_scale_layout_identity() {
    UIScreen screen;
    screen.Init("test");
    uint32_t w = screen.AddWidget(UIWidgetType::Panel, "P",
                                    100.0f, 200.0f, 300.0f, 400.0f);

    // Same size → no change
    screen.ScaleLayout(1280.0f, 720.0f, 1280.0f, 720.0f);

    auto* p = screen.GetWidget(w);
    assert(p);
    assert(std::abs(p->x - 100.0f) < 0.01f);
    assert(std::abs(p->y - 200.0f) < 0.01f);
    assert(std::abs(p->width - 300.0f) < 0.01f);
    assert(std::abs(p->height - 400.0f) < 0.01f);

    std::cout << "[PASS] test_scale_layout_identity" << std::endl;
}

void test_scale_layout_zero_old_noop() {
    UIScreen screen;
    screen.Init("test");
    uint32_t w = screen.AddWidget(UIWidgetType::Panel, "P",
                                    10.0f, 20.0f, 30.0f, 40.0f);

    // Zero old dimensions → no change (avoids division by zero)
    screen.ScaleLayout(0.0f, 0.0f, 1920.0f, 1080.0f);

    auto* p = screen.GetWidget(w);
    assert(p);
    assert(std::abs(p->x - 10.0f) < 0.01f);
    assert(std::abs(p->width - 30.0f) < 0.01f);

    std::cout << "[PASS] test_scale_layout_zero_old_noop" << std::endl;
}

void test_scale_layout_zero_new_noop() {
    UIScreen screen;
    screen.Init("test");
    uint32_t w = screen.AddWidget(UIWidgetType::Panel, "P",
                                    10.0f, 20.0f, 30.0f, 40.0f);

    // Zero new dimensions → no change
    screen.ScaleLayout(1280.0f, 720.0f, 0.0f, 0.0f);

    auto* p = screen.GetWidget(w);
    assert(p);
    assert(std::abs(p->x - 10.0f) < 0.01f);

    std::cout << "[PASS] test_scale_layout_zero_new_noop" << std::endl;
}

void test_scale_layout_shrink() {
    UIScreen screen;
    screen.Init("test");
    uint32_t w = screen.AddWidget(UIWidgetType::Panel, "P",
                                    1280.0f, 720.0f, 1280.0f, 720.0f);

    // Halve
    screen.ScaleLayout(2560.0f, 1440.0f, 1280.0f, 720.0f);

    auto* p = screen.GetWidget(w);
    assert(p);
    assert(std::abs(p->x - 640.0f) < 0.01f);
    assert(std::abs(p->y - 360.0f) < 0.01f);
    assert(std::abs(p->width - 640.0f) < 0.01f);
    assert(std::abs(p->height - 360.0f) < 0.01f);

    std::cout << "[PASS] test_scale_layout_shrink" << std::endl;
}

// ---------------------------------------------------------------
// UIManager::SetViewportSize propagation tests
// ---------------------------------------------------------------

void test_ui_manager_viewport_propagates_layout() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t w = screen.AddWidget(UIWidgetType::Panel, "P",
                                    0.0f, 0.0f, 1280.0f, 720.0f);

    // Set initial viewport
    mgr.SetViewportSize(1280.0f, 720.0f);
    // Scale to double
    mgr.SetViewportSize(2560.0f, 1440.0f);

    auto* p = screen.GetWidget(w);
    assert(p);
    assert(std::abs(p->width - 2560.0f) < 0.01f);
    assert(std::abs(p->height - 1440.0f) < 0.01f);

    std::cout << "[PASS] test_ui_manager_viewport_propagates_layout" << std::endl;
}

void test_ui_manager_viewport_first_set_no_scale() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    auto& screen = mgr.GetScreen();
    uint32_t w = screen.AddWidget(UIWidgetType::Panel, "P",
                                    100.0f, 200.0f, 300.0f, 400.0f);

    // First call: old viewport is 0×0, so no scaling should occur
    mgr.SetViewportSize(1920.0f, 1080.0f);

    auto* p = screen.GetWidget(w);
    assert(p);
    assert(std::abs(p->x - 100.0f) < 0.01f);
    assert(std::abs(p->width - 300.0f) < 0.01f);

    std::cout << "[PASS] test_ui_manager_viewport_first_set_no_scale" << std::endl;
}

// ---------------------------------------------------------------
// Engine resize integration test
// ---------------------------------------------------------------

void test_engine_editor_sets_initial_viewport() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Editor;
    cfg.headless = true;
    cfg.maxTicks = 1;
    cfg.windowWidth = 1600;
    cfg.windowHeight = 900;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();
    engine.InitEditor();

    // InitEditor should set initial viewport size on UIManager
    auto& mgr = engine.GetUIManager();
    assert(std::abs(mgr.GetViewportWidth() - 1600.0f) < 0.01f);
    assert(std::abs(mgr.GetViewportHeight() - 900.0f) < 0.01f);

    std::cout << "[PASS] test_engine_editor_sets_initial_viewport" << std::endl;
}
