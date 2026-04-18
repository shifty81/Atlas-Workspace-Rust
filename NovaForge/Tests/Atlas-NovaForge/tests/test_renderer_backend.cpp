#include <iostream>
#include <cassert>
#include <string>
#include <memory>

// Headers under test
#include "../engine/render/RenderAPI.h"
#include "../engine/render/RendererCapabilities.h"
#include "../engine/render/RendererBackend.h"
#include "../engine/render/NullRendererBackend.h"
#include "../engine/render/RendererFactory.h"
#include "../engine/ui/UIBackend.h"
#include "../engine/ui/UIDrawList.h"

static int s_passed = 0;
static int s_total = 0;

#define TEST_ASSERT(expr)                                                    \
    do {                                                                     \
        s_total++;                                                           \
        if (!(expr)) {                                                       \
            std::cerr << "  FAIL: " #expr " (" << __FILE__ << ":"            \
                      << __LINE__ << ")" << std::endl;                       \
        } else {                                                             \
            s_passed++;                                                      \
        }                                                                    \
    } while (0)

// =============================================================
// RenderAPI enum tests
// =============================================================

void test_render_api_enum_values() {
    using atlas::render::RenderAPI;
    TEST_ASSERT(static_cast<uint8_t>(RenderAPI::None) == 0);
    TEST_ASSERT(static_cast<uint8_t>(RenderAPI::OpenGL) == 1);
    TEST_ASSERT(static_cast<uint8_t>(RenderAPI::Vulkan) == 2);
    TEST_ASSERT(static_cast<uint8_t>(RenderAPI::DX11) == 3);
    TEST_ASSERT(static_cast<uint8_t>(RenderAPI::Null) == 4);
    std::cout << "[PASS] test_render_api_enum_values" << std::endl;
}

// =============================================================
// RendererCapabilities tests
// =============================================================

void test_renderer_capabilities_defaults() {
    atlas::render::RendererCapabilities caps;
    TEST_ASSERT(caps.bindlessTextures == false);
    TEST_ASSERT(caps.computeShaders == false);
    TEST_ASSERT(caps.rayTracing == false);
    TEST_ASSERT(caps.maxMSAASamples == 1);
    TEST_ASSERT(caps.hdrSwapchain == false);
    TEST_ASSERT(caps.maxTextureSize == 0);
    TEST_ASSERT(caps.maxUniformBuffers == 0);
    TEST_ASSERT(caps.deviceName.empty());
    TEST_ASSERT(caps.driverVersion.empty());
    std::cout << "[PASS] test_renderer_capabilities_defaults" << std::endl;
}

void test_renderer_capabilities_set_values() {
    atlas::render::RendererCapabilities caps;
    caps.bindlessTextures = true;
    caps.computeShaders = true;
    caps.maxMSAASamples = 8;
    caps.maxTextureSize = 16384;
    caps.deviceName = "TestGPU";
    caps.driverVersion = "1.2.3";
    TEST_ASSERT(caps.bindlessTextures == true);
    TEST_ASSERT(caps.computeShaders == true);
    TEST_ASSERT(caps.maxMSAASamples == 8);
    TEST_ASSERT(caps.maxTextureSize == 16384);
    TEST_ASSERT(caps.deviceName == "TestGPU");
    TEST_ASSERT(caps.driverVersion == "1.2.3");
    std::cout << "[PASS] test_renderer_capabilities_set_values" << std::endl;
}

// =============================================================
// NullRendererBackend tests
// =============================================================

void test_null_renderer_backend_init() {
    atlas::render::NullRendererBackend renderer;
    renderer.Init();
    TEST_ASSERT(renderer.GetAPI() == atlas::render::RenderAPI::Null);
    TEST_ASSERT(renderer.FrameCount() == 0);
    TEST_ASSERT(renderer.IsFrameActive() == false);
    const auto& caps = renderer.GetCapabilities();
    TEST_ASSERT(caps.deviceName == "NullRenderer");
    TEST_ASSERT(caps.driverVersion == "0.0.0");
    TEST_ASSERT(caps.bindlessTextures == false);
    TEST_ASSERT(caps.computeShaders == false);
    TEST_ASSERT(caps.maxTextureSize == 0);
    std::cout << "[PASS] test_null_renderer_backend_init" << std::endl;
}

void test_null_renderer_backend_frame_cycle() {
    atlas::render::NullRendererBackend renderer;
    renderer.Init();
    TEST_ASSERT(renderer.IsFrameActive() == false);
    renderer.BeginFrame();
    TEST_ASSERT(renderer.IsFrameActive() == true);
    renderer.EndFrame();
    TEST_ASSERT(renderer.IsFrameActive() == false);
    TEST_ASSERT(renderer.FrameCount() == 1);
    renderer.BeginFrame();
    renderer.EndFrame();
    TEST_ASSERT(renderer.FrameCount() == 2);
    std::cout << "[PASS] test_null_renderer_backend_frame_cycle" << std::endl;
}

void test_null_renderer_backend_viewport() {
    atlas::render::NullRendererBackend renderer;
    renderer.Init();
    renderer.SetViewport(1920, 1080);
    TEST_ASSERT(renderer.ViewportWidth() == 1920);
    TEST_ASSERT(renderer.ViewportHeight() == 1080);
    std::cout << "[PASS] test_null_renderer_backend_viewport" << std::endl;
}

void test_null_renderer_backend_shutdown() {
    atlas::render::NullRendererBackend renderer;
    renderer.Init();
    renderer.BeginFrame();
    renderer.EndFrame();
    renderer.Shutdown();
    TEST_ASSERT(renderer.IsFrameActive() == false);
    std::cout << "[PASS] test_null_renderer_backend_shutdown" << std::endl;
}

// =============================================================
// RendererFactory tests
// =============================================================

void test_renderer_factory_create_null() {
    auto backend = atlas::render::RendererFactory::Create(atlas::render::RenderAPI::Null);
    TEST_ASSERT(backend != nullptr);
    TEST_ASSERT(backend->GetAPI() == atlas::render::RenderAPI::Null);
    backend->Init();
    const auto& caps = backend->GetCapabilities();
    TEST_ASSERT(caps.deviceName == "NullRenderer");
    std::cout << "[PASS] test_renderer_factory_create_null" << std::endl;
}

void test_renderer_factory_create_none() {
    auto backend = atlas::render::RendererFactory::Create(atlas::render::RenderAPI::None);
    TEST_ASSERT(backend == nullptr);
    std::cout << "[PASS] test_renderer_factory_create_none" << std::endl;
}

void test_renderer_factory_gpu_backends_return_null_without_libs() {
    // Without GPU libraries linked, GPU backends return nullptr
    auto gl = atlas::render::RendererFactory::Create(atlas::render::RenderAPI::OpenGL);
    auto vk = atlas::render::RendererFactory::Create(atlas::render::RenderAPI::Vulkan);
    auto dx = atlas::render::RendererFactory::Create(atlas::render::RenderAPI::DX11);
    TEST_ASSERT(gl == nullptr);
    TEST_ASSERT(vk == nullptr);
    TEST_ASSERT(dx == nullptr);
    std::cout << "[PASS] test_renderer_factory_gpu_backends_return_null_without_libs" << std::endl;
}

// =============================================================
// UIBackend tests
// =============================================================

void test_null_ui_backend_frame_counting() {
    atlas::ui::NullUIBackend backend;
    TEST_ASSERT(backend.FrameCount() == 0);
    TEST_ASSERT(backend.DrawCalls() == 0);
    backend.BeginFrame();
    TEST_ASSERT(backend.FrameCount() == 1);
    backend.EndFrame();
    TEST_ASSERT(backend.FrameCount() == 1);
    backend.BeginFrame();
    TEST_ASSERT(backend.FrameCount() == 2);
    std::cout << "[PASS] test_null_ui_backend_frame_counting" << std::endl;
}

void test_null_ui_backend_draw_calls() {
    atlas::ui::NullUIBackend backend;
    atlas::ui::UIDrawList list;
    list.DrawRect({0, 0, 100, 100}, {255, 0, 0, 255});
    list.DrawText({0, 0, 100, 20}, "Hello", {255, 255, 255, 255});
    backend.BeginFrame();
    backend.Draw(list);
    backend.EndFrame();
    TEST_ASSERT(backend.DrawCalls() == 1);
    std::cout << "[PASS] test_null_ui_backend_draw_calls" << std::endl;
}

void test_null_ui_backend_name() {
    atlas::ui::NullUIBackend backend;
    TEST_ASSERT(std::string(backend.Name()) == "Null");
    std::cout << "[PASS] test_null_ui_backend_name" << std::endl;
}

void test_null_ui_backend_reset_counters() {
    atlas::ui::NullUIBackend backend;
    backend.BeginFrame();
    backend.EndFrame();
    TEST_ASSERT(backend.FrameCount() == 1);
    backend.ResetCounters();
    TEST_ASSERT(backend.FrameCount() == 0);
    TEST_ASSERT(backend.DrawCalls() == 0);
    std::cout << "[PASS] test_null_ui_backend_reset_counters" << std::endl;
}

// =============================================================
// RendererBackend capabilities access via base pointer
// =============================================================

void test_backend_capabilities_via_base_pointer() {
    std::unique_ptr<atlas::render::RendererBackend> backend =
        atlas::render::RendererFactory::Create(atlas::render::RenderAPI::Null);
    TEST_ASSERT(backend != nullptr);
    backend->Init();
    const auto& caps = backend->GetCapabilities();
    TEST_ASSERT(caps.deviceName == "NullRenderer");
    TEST_ASSERT(caps.computeShaders == false);
    backend->SetViewport(800, 600);
    backend->BeginFrame();
    backend->EndFrame();
    backend->Shutdown();
    std::cout << "[PASS] test_backend_capabilities_via_base_pointer" << std::endl;
}

// =============================================================
// Renderer parity test infrastructure stubs
// =============================================================

void test_renderer_parity_clear_color_stub() {
    // This test validates the pattern: create backend, run frame, query
    auto backend = atlas::render::RendererFactory::Create(atlas::render::RenderAPI::Null);
    TEST_ASSERT(backend != nullptr);
    backend->Init();
    backend->SetViewport(64, 64);
    backend->BeginFrame();
    // In a real parity test, we would: set render target, clear, readback pixels
    // For NullRenderer, we just verify the frame lifecycle works
    backend->EndFrame();

    auto* nullBackend = dynamic_cast<atlas::render::NullRendererBackend*>(backend.get());
    TEST_ASSERT(nullBackend != nullptr);
    TEST_ASSERT(nullBackend->FrameCount() == 1);
    TEST_ASSERT(nullBackend->ViewportWidth() == 64);
    TEST_ASSERT(nullBackend->ViewportHeight() == 64);
    std::cout << "[PASS] test_renderer_parity_clear_color_stub" << std::endl;
}

// =============================================================

void run_renderer_backend_tests() {
    std::cout << "\n--- RenderAPI Enum ---" << std::endl;
    test_render_api_enum_values();

    std::cout << "\n--- RendererCapabilities ---" << std::endl;
    test_renderer_capabilities_defaults();
    test_renderer_capabilities_set_values();

    std::cout << "\n--- NullRendererBackend ---" << std::endl;
    test_null_renderer_backend_init();
    test_null_renderer_backend_frame_cycle();
    test_null_renderer_backend_viewport();
    test_null_renderer_backend_shutdown();

    std::cout << "\n--- RendererFactory ---" << std::endl;
    test_renderer_factory_create_null();
    test_renderer_factory_create_none();
    test_renderer_factory_gpu_backends_return_null_without_libs();

    std::cout << "\n--- NullUIBackend ---" << std::endl;
    test_null_ui_backend_frame_counting();
    test_null_ui_backend_draw_calls();
    test_null_ui_backend_name();
    test_null_ui_backend_reset_counters();

    std::cout << "\n--- Backend Capabilities via Base Pointer ---" << std::endl;
    test_backend_capabilities_via_base_pointer();

    std::cout << "\n--- Renderer Parity Infrastructure ---" << std::endl;
    test_renderer_parity_clear_color_stub();

    std::cout << "\n  Renderer backend tests: " << s_passed << "/" << s_total
              << " passed" << std::endl;
}
