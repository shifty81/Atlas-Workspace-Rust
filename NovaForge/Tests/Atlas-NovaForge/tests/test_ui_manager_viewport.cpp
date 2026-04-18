#include "../engine/ui/UIManager.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_ui_manager_viewport_defaults() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);
    assert(mgr.GetViewportWidth() == 0.0f);
    assert(mgr.GetViewportHeight() == 0.0f);
    assert(mgr.GetDPIScale() == 1.0f);
    std::cout << "[PASS] test_ui_manager_viewport_defaults" << std::endl;
}

void test_ui_manager_set_viewport_size() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    mgr.SetViewportSize(1920.0f, 1080.0f);
    assert(mgr.GetViewportWidth() == 1920.0f);
    assert(mgr.GetViewportHeight() == 1080.0f);

    // Update viewport
    mgr.SetViewportSize(2560.0f, 1440.0f);
    assert(mgr.GetViewportWidth() == 2560.0f);
    assert(mgr.GetViewportHeight() == 1440.0f);
    std::cout << "[PASS] test_ui_manager_set_viewport_size" << std::endl;
}

void test_ui_manager_dpi_scale() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    mgr.SetDPIScale(2.0f);
    assert(mgr.GetDPIScale() == 2.0f);

    mgr.SetDPIScale(1.5f);
    assert(mgr.GetDPIScale() == 1.5f);
    std::cout << "[PASS] test_ui_manager_dpi_scale" << std::endl;
}

void test_ui_manager_event_router_access() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    auto& router = mgr.GetEventRouter();
    assert(router.TargetCount() == 0);
    std::cout << "[PASS] test_ui_manager_event_router_access" << std::endl;
}

void test_ui_manager_dispatch_event() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    struct TestTarget : public UIEventTarget {
        bool HitTest(int32_t, int32_t) const override { return true; }
        bool OnEvent(const UIEvent&) override { ++count; return true; }
        int count = 0;
    };

    TestTarget target;
    mgr.GetEventRouter().Register(&target);

    UIEvent evt;
    evt.type = UIEvent::Type::MouseDown;
    evt.x = 50; evt.y = 50;
    bool consumed = mgr.DispatchEvent(evt);
    assert(consumed);
    assert(target.count == 1);

    mgr.GetEventRouter().Unregister(&target);
    std::cout << "[PASS] test_ui_manager_dispatch_event" << std::endl;
}

void test_ui_manager_dispatch_not_initialized() {
    UIManager mgr;
    // Not initialized — dispatch should return false
    UIEvent evt;
    evt.type = UIEvent::Type::MouseDown;
    evt.x = 10; evt.y = 10;
    assert(!mgr.DispatchEvent(evt));
    std::cout << "[PASS] test_ui_manager_dispatch_not_initialized" << std::endl;
}

void test_ui_manager_font_bootstrap_access() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    assert(!mgr.IsFontReady());
    auto& fb = mgr.GetFontBootstrap();
    assert(fb.GetDefaultFont() == kInvalidFont);
    std::cout << "[PASS] test_ui_manager_font_bootstrap_access" << std::endl;
}

void test_ui_manager_shutdown_resets_viewport() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);
    mgr.SetViewportSize(1920.0f, 1080.0f);
    mgr.SetDPIScale(2.0f);

    struct TestTarget : public UIEventTarget {
        bool HitTest(int32_t, int32_t) const override { return true; }
        bool OnEvent(const UIEvent&) override { return true; }
    };
    TestTarget target;
    mgr.GetEventRouter().Register(&target);
    assert(mgr.GetEventRouter().TargetCount() == 1);

    mgr.Shutdown();

    assert(mgr.GetViewportWidth() == 0.0f);
    assert(mgr.GetViewportHeight() == 0.0f);
    assert(mgr.GetDPIScale() == 1.0f);
    assert(mgr.GetEventRouter().TargetCount() == 0);
    assert(!mgr.IsFontReady());
    std::cout << "[PASS] test_ui_manager_shutdown_resets_viewport" << std::endl;
}
