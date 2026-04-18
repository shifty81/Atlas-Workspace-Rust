#include "../engine/render/VulkanRenderer.h"
#include "../engine/render/RenderAPI.h"
#include "../engine/ui/UIRenderer.h"
#include "../engine/ui/UIManager.h"
#include "../engine/platform/PlatformWindow.h"
#include "../engine/core/Engine.h"
#include <iostream>
#include <cassert>

#if !defined(__linux__) || defined(ATLAS_HAS_X11)
#include "../engine/render/GLRenderer.h"
#endif

using namespace atlas;
using namespace atlas::ui;
using namespace atlas::render;

void test_render_api_enum() {
    RenderAPI gl = RenderAPI::OpenGL;
    RenderAPI vk = RenderAPI::Vulkan;
    assert(gl != vk);
    assert(static_cast<uint8_t>(gl) == 1);
    assert(static_cast<uint8_t>(vk) == 2);
    std::cout << "[PASS] test_render_api_enum" << std::endl;
}

void test_null_renderer() {
    NullUIRenderer renderer;
    UIRect rect{10, 20, 100, 50};
    UIColor color{255, 0, 0, 255};

    renderer.BeginFrame();
    renderer.DrawRect(rect, color);
    renderer.DrawText(rect, "test", color);
    renderer.DrawIcon(rect, 1, color);
    renderer.DrawBorder(rect, 2, color);
    renderer.DrawImage(rect, 1, color);
    renderer.EndFrame();

    std::cout << "[PASS] test_null_renderer" << std::endl;
}

void test_gl_renderer_viewport() {
#if !defined(__linux__) || defined(ATLAS_HAS_X11)
    GLRenderer renderer;
    renderer.SetViewport(1920, 1080);
    // GLRenderer stores viewport; no crash on set
    std::cout << "[PASS] test_gl_renderer_viewport" << std::endl;
#else
    std::cout << "[SKIP] test_gl_renderer_viewport (no GL support)" << std::endl;
#endif
}

void test_vulkan_renderer_viewport() {
    VulkanRenderer renderer;
    renderer.SetViewport(1920, 1080);
    // VulkanRenderer stores viewport; no crash on set
    std::cout << "[PASS] test_vulkan_renderer_viewport" << std::endl;
}

void test_ui_manager_set_renderer() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    assert(mgr.GetRenderer() == nullptr);

    NullUIRenderer renderer;
    mgr.SetRenderer(&renderer);
    assert(mgr.GetRenderer() == &renderer);

    mgr.Shutdown();
    assert(mgr.GetRenderer() == nullptr);

    std::cout << "[PASS] test_ui_manager_set_renderer" << std::endl;
}

void test_ui_manager_render_empty() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    NullUIRenderer renderer;
    mgr.SetRenderer(&renderer);

    // Render with no widgets should not crash
    mgr.Render(&renderer);

    std::cout << "[PASS] test_ui_manager_render_empty" << std::endl;
}

void test_ui_manager_render_widgets() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    NullUIRenderer renderer;
    mgr.SetRenderer(&renderer);

    auto& screen = mgr.GetScreen();
    uint32_t panel = screen.AddWidget(UIWidgetType::Panel, "TestPanel", 0, 0, 400, 300);
    uint32_t btn = screen.AddWidget(UIWidgetType::Button, "TestBtn", 10, 10, 100, 30);
    screen.SetParent(btn, panel);
    uint32_t text = screen.AddWidget(UIWidgetType::Text, "Hello", 10, 50, 200, 20);
    screen.SetParent(text, panel);
    uint32_t input = screen.AddWidget(UIWidgetType::InputField, "type here", 10, 80, 200, 25);
    screen.SetParent(input, panel);
    uint32_t list = screen.AddWidget(UIWidgetType::List, "Items", 10, 110, 200, 100);
    screen.SetParent(list, panel);

    // Render should traverse widget tree without crash
    mgr.Render(&renderer);

    std::cout << "[PASS] test_ui_manager_render_widgets" << std::endl;
}

void test_ui_manager_render_null_renderer() {
    UIManager mgr;
    mgr.Init(GUIContext::Editor);

    // Render with null renderer should not crash
    mgr.Render(nullptr);

    std::cout << "[PASS] test_ui_manager_render_null_renderer" << std::endl;
}

void test_engine_config_render_fields() {
    EngineConfig cfg;
    assert(cfg.renderAPI == RenderAPI::OpenGL);
    assert(cfg.windowWidth == 1280);
    assert(cfg.windowHeight == 720);
    assert(cfg.headless == false);

    cfg.renderAPI = RenderAPI::Vulkan;
    cfg.windowWidth = 1920;
    cfg.windowHeight = 1080;
    cfg.headless = true;
    assert(cfg.renderAPI == RenderAPI::Vulkan);
    assert(cfg.windowWidth == 1920);
    assert(cfg.windowHeight == 1080);
    assert(cfg.headless == true);

    std::cout << "[PASS] test_engine_config_render_fields" << std::endl;
}

void test_engine_headless_no_window() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Client;
    cfg.headless = true;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();

    // In headless mode, no window or renderer should be created
    assert(engine.GetWindow() == nullptr);
    assert(engine.GetRenderer() == nullptr);

    engine.GetScheduler().SetFramePacing(false);
    engine.Run();

    std::cout << "[PASS] test_engine_headless_no_window" << std::endl;
}

void test_engine_server_no_window() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();
    engine.InitECS();
    engine.InitNetworking();

    // Server mode should not create window
    assert(engine.GetWindow() == nullptr);
    assert(engine.GetRenderer() == nullptr);

    engine.GetScheduler().SetFramePacing(false);
    engine.Run();

    std::cout << "[PASS] test_engine_server_no_window" << std::endl;
}

void test_platform_window_event_type() {
    platform::WindowEvent event;
    assert(event.type == platform::WindowEvent::Type::NoEvent);
    event.type = platform::WindowEvent::Type::Close;
    assert(event.type == platform::WindowEvent::Type::Close);
    event.type = platform::WindowEvent::Type::Resize;
    event.width = 800;
    event.height = 600;
    assert(event.width == 800);
    assert(event.height == 600);

    std::cout << "[PASS] test_platform_window_event_type" << std::endl;
}

void test_platform_window_config() {
    platform::PlatformWindowConfig cfg;
    assert(cfg.title == "Atlas Engine");
    assert(cfg.width == 1280);
    assert(cfg.height == 720);
    assert(cfg.resizable == true);

    cfg.title = "Custom Title";
    cfg.width = 800;
    cfg.height = 600;
    cfg.resizable = false;
    assert(cfg.title == "Custom Title");
    assert(cfg.width == 800);
    assert(cfg.height == 600);
    assert(cfg.resizable == false);

    std::cout << "[PASS] test_platform_window_config" << std::endl;
}

void test_platform_has_window_implementation() {
    // At least one platform window implementation should be available
    // on any supported build target (Linux with X11 or Windows).
#if defined(ATLAS_HAS_X11) || defined(ATLAS_HAS_WIN32)
    std::cout << "[PASS] test_platform_has_window_implementation" << std::endl;
#else
    std::cout << "[SKIP] test_platform_has_window_implementation (no platform window)" << std::endl;
#endif
}

void test_engine_no_window_error_without_platform() {
    // When running headless, the engine should not attempt to create a window
    // and should not produce the "No platform window implementation" error
    EngineConfig cfg;
    cfg.mode = EngineMode::Editor;
    cfg.headless = true;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();

    // In headless mode, window and renderer should be null regardless of platform
    assert(engine.GetWindow() == nullptr);
    assert(engine.GetRenderer() == nullptr);

    std::cout << "[PASS] test_engine_no_window_error_without_platform" << std::endl;
}
