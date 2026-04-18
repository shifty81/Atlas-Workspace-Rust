#include "DiagnosticsOverlay.h"
#include "UIRenderer.h"
#include "UIGraph.h" // UIContext
#include <string>
#include <cstdio>

namespace atlas::ui {

bool DiagnosticsOverlay::s_enabled = false;

void DiagnosticsOverlay::Toggle() {
    s_enabled = !s_enabled;
}

void DiagnosticsOverlay::SetEnabled(bool enabled) {
    s_enabled = enabled;
}

bool DiagnosticsOverlay::IsEnabled() {
    return s_enabled;
}

void DiagnosticsOverlay::Render(UIRenderer* renderer,
                                 const UIContext& ctx,
                                 float dpiScale,
                                 int32_t mouseX,
                                 int32_t mouseY) {
    if (!s_enabled || !renderer) return;

    // Semi-transparent background panel
    UIRect bgRect{10, 10, 320, 140};
    UIColor bgColor{0, 0, 0, 180};
    renderer->DrawRect(bgRect, bgColor);

    UIColor textColor{255, 255, 255, 255};
    int32_t y = 16;
    constexpr int32_t lineH = 20;

    char buf[128];

    // FPS
    float fps = (ctx.deltaTime > 0.0f) ? (1.0f / ctx.deltaTime) : 0.0f;
    std::snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
    renderer->DrawText({20, y, 300, lineH}, std::string(buf), textColor);
    y += lineH;

    // Frame time
    std::snprintf(buf, sizeof(buf), "Frame Time: %.3f ms", ctx.deltaTime * 1000.0f);
    renderer->DrawText({20, y, 300, lineH}, std::string(buf), textColor);
    y += lineH;

    // Viewport
    std::snprintf(buf, sizeof(buf), "Viewport: %.0f x %.0f",
                  ctx.screenWidth, ctx.screenHeight);
    renderer->DrawText({20, y, 300, lineH}, std::string(buf), textColor);
    y += lineH;

    // DPI scale
    std::snprintf(buf, sizeof(buf), "DPI Scale: %.2f", dpiScale);
    renderer->DrawText({20, y, 300, lineH}, std::string(buf), textColor);
    y += lineH;

    // Mouse position
    std::snprintf(buf, sizeof(buf), "Mouse: %d, %d", mouseX, mouseY);
    renderer->DrawText({20, y, 300, lineH}, std::string(buf), textColor);
    y += lineH;

    // Tick
    std::snprintf(buf, sizeof(buf), "Tick: %u", ctx.tick);
    renderer->DrawText({20, y, 300, lineH}, std::string(buf), textColor);
}

} // namespace atlas::ui
