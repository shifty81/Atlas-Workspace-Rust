#include "../engine/ui/DiagnosticsOverlay.h"
#include "../engine/ui/UIRenderer.h"
#include "../engine/ui/UIGraph.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_diagnostics_overlay_default_off() {
    assert(!DiagnosticsOverlay::IsEnabled());
    std::cout << "[PASS] test_diagnostics_overlay_default_off" << std::endl;
}

void test_diagnostics_overlay_toggle() {
    DiagnosticsOverlay::SetEnabled(false);
    assert(!DiagnosticsOverlay::IsEnabled());

    DiagnosticsOverlay::Toggle();
    assert(DiagnosticsOverlay::IsEnabled());

    DiagnosticsOverlay::Toggle();
    assert(!DiagnosticsOverlay::IsEnabled());
    std::cout << "[PASS] test_diagnostics_overlay_toggle" << std::endl;
}

void test_diagnostics_overlay_set_enabled() {
    DiagnosticsOverlay::SetEnabled(true);
    assert(DiagnosticsOverlay::IsEnabled());

    DiagnosticsOverlay::SetEnabled(false);
    assert(!DiagnosticsOverlay::IsEnabled());
    std::cout << "[PASS] test_diagnostics_overlay_set_enabled" << std::endl;
}

void test_diagnostics_overlay_render_null_renderer() {
    DiagnosticsOverlay::SetEnabled(true);
    UIContext ctx{1920.0f, 1080.0f, 0.016f, 1};

    // Should not crash with null renderer
    DiagnosticsOverlay::Render(nullptr, ctx, 1.0f, 100, 200);

    DiagnosticsOverlay::SetEnabled(false);
    std::cout << "[PASS] test_diagnostics_overlay_render_null_renderer" << std::endl;
}

void test_diagnostics_overlay_render_when_disabled() {
    DiagnosticsOverlay::SetEnabled(false);
    NullUIRenderer renderer;
    UIContext ctx{1920.0f, 1080.0f, 0.016f, 1};

    // Should be a no-op when disabled
    DiagnosticsOverlay::Render(&renderer, ctx, 1.0f, 0, 0);
    std::cout << "[PASS] test_diagnostics_overlay_render_when_disabled" << std::endl;
}

void test_diagnostics_overlay_render_when_enabled() {
    DiagnosticsOverlay::SetEnabled(true);
    NullUIRenderer renderer;
    UIContext ctx{1920.0f, 1080.0f, 0.016f, 1};

    // Should not crash when enabled with a valid renderer
    DiagnosticsOverlay::Render(&renderer, ctx, 2.0f, 500, 300);

    DiagnosticsOverlay::SetEnabled(false);
    std::cout << "[PASS] test_diagnostics_overlay_render_when_enabled" << std::endl;
}
