#include <catch2/catch_test_macros.hpp>
#include "NF/UI/GLFWWindowProvider.h"
#include "NF/UI/GLFWInputAdapter.h"
#include "NF/UI/OpenGLBackend.h"
#include "NF/UI/UI.h"
#include "NF/Input/Input.h"

// ── GLFWWindowProvider ───────────────────────────────────────────

TEST_CASE("GLFWWindowProvider init and shutdown", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    REQUIRE_FALSE(window.isInitialized());

    NF::GLFWWindowProvider::WindowConfig cfg;
    REQUIRE(window.init(cfg));
    REQUIRE(window.isInitialized());
    REQUIRE(window.width() == 1280);
    REQUIRE(window.height() == 800);

    window.shutdown();
    REQUIRE_FALSE(window.isInitialized());
}

TEST_CASE("GLFWWindowProvider config defaults", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider::WindowConfig cfg;
    REQUIRE(cfg.width == 1280);
    REQUIRE(cfg.height == 800);
    REQUIRE(cfg.title == "Atlas Editor");
    REQUIRE(cfg.vsync == true);
    REQUIRE(cfg.maximized == false);
    REQUIRE(cfg.decorated == true);
}

TEST_CASE("GLFWWindowProvider custom config", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    cfg.width  = 800;
    cfg.height = 600;
    cfg.title  = "Test Window";
    cfg.vsync  = false;

    REQUIRE(window.init(cfg));
    REQUIRE(window.width() == 800);
    REQUIRE(window.height() == 600);
    window.shutdown();
}

TEST_CASE("GLFWWindowProvider shouldClose after pollEvents", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    window.init(cfg);

    REQUIRE_FALSE(window.shouldClose());
    window.pollEvents();
    REQUIRE(window.shouldClose());

    window.shutdown();
}

TEST_CASE("GLFWWindowProvider frame timing", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    window.init(cfg);

    REQUIRE(window.frameTime() > 0.0);
    REQUIRE(window.fps() > 0.0);

    window.shutdown();
}

TEST_CASE("GLFWWindowProvider setTitle", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    window.init(cfg);

    window.setTitle("New Title");
    // Stub: just verifies no crash
    REQUIRE(window.isInitialized());

    window.shutdown();
}

TEST_CASE("GLFWWindowProvider nativeHandle is null in stub", "[UI][M1][GLFW]") {
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig cfg;
    window.init(cfg);

    REQUIRE(window.nativeHandle() == nullptr);

    window.shutdown();
}

// ── OpenGLBackend ────────────────────────────────────────────────

TEST_CASE("OpenGLBackend as UIBackend", "[UI][M1][Backend]") {
    NF::OpenGLBackend backend;
    NF::UIBackend* base = &backend;

    REQUIRE(base->init(1280, 800));
    REQUIRE(std::string(base->backendName()) == "OpenGL");
    REQUIRE(base->isGPUAccelerated());

    base->beginFrame(1280, 800);
    base->endFrame();
    base->shutdown();
}

TEST_CASE("OpenGLBackend flush records stats", "[UI][M1][Backend]") {
    NF::OpenGLBackend backend;
    backend.init(800, 600);

    NF::UIVertex verts[4] = {};
    uint32_t indices[6] = {0, 1, 2, 2, 3, 0};

    backend.beginFrame(800, 600);
    backend.flush(verts, 4, indices, 6);
    REQUIRE(backend.lastVertexCount() == 4);
    REQUIRE(backend.lastIndexCount() == 6);
    backend.endFrame();

    backend.shutdown();
}

TEST_CASE("OpenGLBackend flush with zero vertices is no-op", "[UI][M1][Backend]") {
    NF::OpenGLBackend backend;
    backend.init(800, 600);

    backend.beginFrame(800, 600);
    backend.flush(nullptr, 0, nullptr, 0);
    REQUIRE(backend.lastVertexCount() == 0);
    REQUIRE(backend.lastIndexCount() == 0);
    backend.endFrame();

    backend.shutdown();
}

// ── GLFWInputAdapter ─────────────────────────────────────────────

TEST_CASE("GLFWInputAdapter event injection", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWInputAdapter adapter(input);
    REQUIRE_FALSE(adapter.isAttached());
    REQUIRE(adapter.eventCount() == 0);

    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    window.init(wCfg);

    adapter.attach(window);
    REQUIRE(adapter.isAttached());

    adapter.injectKeyEvent(NF::KeyCode::A, true);
    REQUIRE(adapter.eventCount() == 1);
    REQUIRE(input.isKeyDown(NF::KeyCode::A));

    adapter.injectKeyEvent(NF::KeyCode::A, false);
    REQUIRE(adapter.eventCount() == 2);
    REQUIRE_FALSE(input.isKeyDown(NF::KeyCode::A));

    adapter.detach();
    REQUIRE_FALSE(adapter.isAttached());

    input.shutdown();
    window.shutdown();
}

TEST_CASE("GLFWInputAdapter mouse move", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWInputAdapter adapter(input);
    adapter.injectMouseMove(100.f, 200.f);
    REQUIRE(adapter.eventCount() == 1);
    REQUIRE(input.state().mouse.x == 100.f);
    REQUIRE(input.state().mouse.y == 200.f);

    input.shutdown();
}

TEST_CASE("GLFWInputAdapter mouse button", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWInputAdapter adapter(input);
    adapter.injectMouseButton(0, true); // left button
    REQUIRE(input.isKeyDown(NF::KeyCode::Mouse1));
    REQUIRE(adapter.eventCount() == 1);

    adapter.injectMouseButton(0, false);
    REQUIRE_FALSE(input.isKeyDown(NF::KeyCode::Mouse1));

    input.shutdown();
}

TEST_CASE("GLFWInputAdapter scroll", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWInputAdapter adapter(input);
    adapter.injectScroll(0.f, 3.f);
    REQUIRE(adapter.eventCount() == 1);
    REQUIRE(input.state().mouse.scrollDelta == 3.f);

    input.shutdown();
}

TEST_CASE("GLFWInputAdapter attach requires initialized window", "[UI][M1][Input]") {
    NF::InputSystem input;
    input.init();

    NF::GLFWWindowProvider window; // not initialized
    NF::GLFWInputAdapter adapter(input);
    adapter.attach(window);
    REQUIRE_FALSE(adapter.isAttached());

    input.shutdown();
}

// ── Full M1 Integration (Atlas UI — no ImGui) ────────────────────

TEST_CASE("M1 integration: window, OpenGL backend, adapter", "[UI][M1][Integration]") {
    // 1. Window
    NF::GLFWWindowProvider window;
    NF::GLFWWindowProvider::WindowConfig wCfg;
    wCfg.width  = 1024;
    wCfg.height = 768;
    wCfg.title  = "Integration Test";
    REQUIRE(window.init(wCfg));

    // 2. OpenGL Backend (Atlas UI — no ImGui)
    NF::OpenGLBackend backend;
    REQUIRE(backend.init(wCfg.width, wCfg.height));

    // 3. UIRenderer with backend
    NF::UIRenderer renderer;
    renderer.init();
    renderer.setBackend(&backend);

    // 4. Input
    NF::InputSystem input;
    input.init();
    NF::GLFWInputAdapter adapter(input);
    adapter.attach(window);

    // Simulate one frame
    REQUIRE_FALSE(window.shouldClose());
    window.pollEvents();

    backend.beginFrame(window.width(), window.height());
    renderer.beginFrame(static_cast<float>(window.width()),
                        static_cast<float>(window.height()));
    renderer.drawRect({10.f, 10.f, 100.f, 50.f}, 0xFF0000FF);
    renderer.endFrame();
    backend.endFrame();

    REQUIRE(backend.lastVertexCount() == 4);
    REQUIRE(backend.lastIndexCount() == 6);

    // Inject input
    adapter.injectKeyEvent(NF::KeyCode::Escape, true);
    REQUIRE(input.isKeyDown(NF::KeyCode::Escape));

    // Window should close after pollEvents
    REQUIRE(window.shouldClose());

    // Cleanup
    adapter.detach();
    renderer.shutdown();
    backend.shutdown();
    window.shutdown();
    input.shutdown();
}
