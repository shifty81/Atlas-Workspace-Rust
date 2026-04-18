// Phase 21: Cleanup & Alignment — validate refactored Engine helpers
// and virtual SetViewport dispatch.

#include "../engine/core/Engine.h"
#include "../engine/ui/UIRenderer.h"
#include "../engine/render/VulkanRenderer.h"
#include <cassert>
#include <iostream>

// =============================================================
// Verify UIRenderer::SetViewport is virtual and callable
// =============================================================

namespace {

class TestRenderer : public atlas::ui::UIRenderer {
public:
    void BeginFrame() override {}
    void EndFrame() override {}
    void DrawRect(const atlas::ui::UIRect&, const atlas::ui::UIColor&) override {}
    void DrawText(const atlas::ui::UIRect&, const std::string&, const atlas::ui::UIColor&) override {}
    void DrawIcon(const atlas::ui::UIRect&, uint32_t, const atlas::ui::UIColor&) override {}
    void DrawBorder(const atlas::ui::UIRect&, int32_t, const atlas::ui::UIColor&) override {}
    void DrawImage(const atlas::ui::UIRect&, uint32_t, const atlas::ui::UIColor&) override {}

    void SetViewport(int32_t w, int32_t h) override {
        lastW = w;
        lastH = h;
    }

    int32_t lastW = 0;
    int32_t lastH = 0;
};

} // anonymous namespace

void test_p21_virtual_set_viewport() {
    TestRenderer tr;
    atlas::ui::UIRenderer* base = &tr;
    base->SetViewport(1920, 1080);
    assert(tr.lastW == 1920);
    assert(tr.lastH == 1080);
    std::cout << "  [PASS] test_p21_virtual_set_viewport" << std::endl;
}

void test_p21_null_renderer_set_viewport() {
    atlas::ui::NullUIRenderer nr;
    // NullUIRenderer inherits default no-op SetViewport — must not crash
    nr.SetViewport(800, 600);
    std::cout << "  [PASS] test_p21_null_renderer_set_viewport" << std::endl;
}

void test_p21_vulkan_set_viewport_override() {
    atlas::render::VulkanRenderer vk;
    atlas::ui::UIRenderer* base = &vk;
    // SetViewport should be callable through the base pointer
    base->SetViewport(2560, 1440);
    std::cout << "  [PASS] test_p21_vulkan_set_viewport_override" << std::endl;
}

// =============================================================
// Verify Engine helper methods exist and are callable
// =============================================================

void test_p21_engine_tick_simulation() {
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Server;
    cfg.headless = true;
    cfg.maxTicks = 1;
    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    // Engine must be constructible and initializable without crashing
    assert(engine.Running());
    std::cout << "  [PASS] test_p21_engine_tick_simulation" << std::endl;
}

void test_p21_engine_has_helper_methods() {
    // Compile-time check: the Engine class exposes TickSimulation,
    // UpdateUI, and RenderFrame as private helpers.  We verify indirectly
    // by confirming RunEditor / RunClient / RunServer still work.
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Server;
    cfg.headless = true;
    cfg.maxTicks = 2;
    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.Run();
    // After running with maxTicks=2, engine should have stopped
    assert(!engine.Running());
    std::cout << "  [PASS] test_p21_engine_has_helper_methods" << std::endl;
}

void test_p21_engine_client_mode_limited_ticks() {
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Client;
    cfg.headless = true;
    cfg.maxTicks = 3;
    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();  // headless → no window
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();
    engine.Run();
    assert(!engine.Running());
    std::cout << "  [PASS] test_p21_engine_client_mode_limited_ticks" << std::endl;
}

void test_p21_engine_editor_mode_limited_ticks() {
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Editor;
    cfg.headless = true;
    cfg.maxTicks = 3;
    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();  // headless → no window
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();
    engine.InitEditor();
    engine.Run();
    assert(!engine.Running());
    std::cout << "  [PASS] test_p21_engine_editor_mode_limited_ticks" << std::endl;
}

// =============================================================
// Registration
// =============================================================

void register_next_tasks_phase21() {
    std::cout << "\n--- Phase 21: Cleanup & Alignment ---" << std::endl;
    test_p21_virtual_set_viewport();
    test_p21_null_renderer_set_viewport();
    test_p21_vulkan_set_viewport_override();
    test_p21_engine_tick_simulation();
    test_p21_engine_has_helper_methods();
    test_p21_engine_client_mode_limited_ticks();
    test_p21_engine_editor_mode_limited_ticks();
    std::cout << "  Phase 21 tests: 7/7 passed\n";
}
