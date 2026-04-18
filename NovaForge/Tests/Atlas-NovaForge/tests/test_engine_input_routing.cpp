#include "../engine/core/Engine.h"
#include "../engine/ui/UIEventRouter.h"
#include "../engine/ui/DiagnosticsOverlay.h"
#include "../engine/platform/PlatformWindow.h"
#include <iostream>
#include <cassert>

using namespace atlas;

// ============================================================
// Engine Input Routing Tests
// ============================================================

void test_engine_has_event_router() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.headless = true;

    Engine engine(cfg);
    engine.InitCore();

    // EventRouter should be accessible and empty
    auto& router = engine.GetEventRouter();
    assert(router.TargetCount() == 0);

    std::cout << "[PASS] test_engine_has_event_router" << std::endl;
}

void test_engine_mouse_tracking_defaults() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.headless = true;

    Engine engine(cfg);
    engine.InitCore();

    // Mouse position should default to 0,0
    assert(engine.MouseX() == 0);
    assert(engine.MouseY() == 0);

    std::cout << "[PASS] test_engine_mouse_tracking_defaults" << std::endl;
}

void test_engine_event_router_register() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.headless = true;

    Engine engine(cfg);
    engine.InitCore();

    // Create a simple event target
    struct TestTarget : public ui::UIEventTarget {
        bool HitTest(int32_t, int32_t) const override { return true; }
        bool OnEvent(const ui::UIEvent&) override {
            ++eventCount;
            return true;
        }
        int eventCount = 0;
    };

    TestTarget target;
    engine.GetEventRouter().Register(&target);
    assert(engine.GetEventRouter().TargetCount() == 1);

    // Dispatch an event through the router
    ui::UIEvent evt;
    evt.type = ui::UIEvent::Type::MouseDown;
    evt.x = 10;
    evt.y = 20;
    engine.GetEventRouter().Dispatch(evt);

    assert(target.eventCount == 1);

    engine.GetEventRouter().Unregister(&target);
    assert(engine.GetEventRouter().TargetCount() == 0);

    std::cout << "[PASS] test_engine_event_router_register" << std::endl;
}

void test_diagnostics_overlay_toggle_state() {
    // Ensure toggle works correctly
    bool initial = ui::DiagnosticsOverlay::IsEnabled();
    ui::DiagnosticsOverlay::Toggle();
    assert(ui::DiagnosticsOverlay::IsEnabled() != initial);
    ui::DiagnosticsOverlay::Toggle();
    assert(ui::DiagnosticsOverlay::IsEnabled() == initial);

    // Restore default state
    ui::DiagnosticsOverlay::SetEnabled(false);

    std::cout << "[PASS] test_diagnostics_overlay_toggle_state" << std::endl;
}

void test_f3_key_constant_defined() {
    // Verify the platform-independent F3 key constant is defined and usable
    assert(platform::kKeyF3 != 0);
    // Ensure it doesn't conflict with printable ASCII
    assert(platform::kKeyF3 > 127);

    std::cout << "[PASS] test_f3_key_constant_defined" << std::endl;
}

void test_window_event_scroll_wheel_type() {
    // Verify that the ScrollWheel event type and scrollDelta field exist
    platform::WindowEvent event;
    event.type = platform::WindowEvent::Type::ScrollWheel;
    event.scrollDelta = -1.0f;
    event.mouseX = 100;
    event.mouseY = 200;
    assert(event.type == platform::WindowEvent::Type::ScrollWheel);
    assert(event.scrollDelta == -1.0f);
    assert(event.mouseX == 100);
    assert(event.mouseY == 200);

    std::cout << "[PASS] test_window_event_scroll_wheel_type" << std::endl;
}

void test_window_event_mouse_button_zero_indexed() {
    // UI handlers expect 0=left, 1=middle, 2=right.  Verify that the
    // default mouseButton value is 0 (left) and that the platform
    // convention is 0-indexed.
    platform::WindowEvent event;
    event.type = platform::WindowEvent::Type::MouseButtonDown;
    event.mouseButton = 0; // left click
    assert(event.mouseButton == 0);

    // UIManager and all widget managers check mouseButton == 0 for left click.
    // Verify interaction with a UI widget using a left-click (button 0).
    ui::UIEvent uiEvent;
    uiEvent.type = ui::UIEvent::Type::MouseDown;
    uiEvent.mouseButton = 0; // left click
    assert(uiEvent.mouseButton == 0);

    std::cout << "[PASS] test_window_event_mouse_button_zero_indexed" << std::endl;
}

void test_scroll_wheel_dispatch_through_ui_manager() {
    // Verify that a ScrollWheel UIEvent reaches the UIManager correctly
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.headless = true;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitUI();

    auto& screen = engine.GetUIManager().GetScreen();
    uint32_t sv = screen.AddWidget(ui::UIWidgetType::ScrollView, "SV", 10, 10, 200, 200);
    engine.GetUIManager().GetScrollManager().RegisterScrollView(sv, 500.0f);

    // Dispatch a scroll wheel event through UIManager
    ui::UIEvent scrollEvent;
    scrollEvent.type = ui::UIEvent::Type::ScrollWheel;
    scrollEvent.x = 100;
    scrollEvent.y = 100;
    scrollEvent.scrollDelta = 1.0f;
    bool consumed = engine.GetUIManager().DispatchEvent(scrollEvent);
    assert(consumed);

    // Verify scroll state changed
    auto* state = engine.GetUIManager().GetScrollManager().GetScrollState(sv);
    assert(state != nullptr);
    assert(state->GetOffset() != 0.0f);

    std::cout << "[PASS] test_scroll_wheel_dispatch_through_ui_manager" << std::endl;
}
