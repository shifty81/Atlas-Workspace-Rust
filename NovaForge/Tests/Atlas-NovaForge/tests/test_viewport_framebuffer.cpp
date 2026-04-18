#include "../engine/render/EditorViewportFramebuffer.h"
#include "../engine/core/Engine.h"
#include <iostream>
#include <cassert>

using namespace atlas;
using namespace atlas::render;

void test_null_viewport_framebuffer_defaults() {
    NullViewportFramebuffer fb;
    assert(fb.GetWidth() == 0);
    assert(fb.GetHeight() == 0);
    assert(fb.GetColorAttachment() == 0);
    assert(!fb.IsValid());
    std::cout << "[PASS] test_null_viewport_framebuffer_defaults" << std::endl;
}

void test_null_viewport_framebuffer_sized() {
    NullViewportFramebuffer fb(1280, 720);
    assert(fb.GetWidth() == 1280);
    assert(fb.GetHeight() == 720);
    assert(fb.IsValid());
    std::cout << "[PASS] test_null_viewport_framebuffer_sized" << std::endl;
}

void test_null_viewport_framebuffer_resize() {
    NullViewportFramebuffer fb(800, 600);
    assert(fb.GetWidth() == 800);
    assert(fb.GetHeight() == 600);
    assert(fb.IsValid());

    fb.Resize(1920, 1080);
    assert(fb.GetWidth() == 1920);
    assert(fb.GetHeight() == 1080);
    assert(fb.IsValid());

    // Resize to zero should make it invalid
    fb.Resize(0, 0);
    assert(!fb.IsValid());
    std::cout << "[PASS] test_null_viewport_framebuffer_resize" << std::endl;
}

void test_null_viewport_framebuffer_bind_unbind() {
    NullViewportFramebuffer fb(640, 480);
    // Bind/unbind are no-ops for NullViewportFramebuffer, should not crash
    fb.Bind();
    fb.Unbind();
    std::cout << "[PASS] test_null_viewport_framebuffer_bind_unbind" << std::endl;
}

void test_engine_editor_has_viewport_framebuffer() {
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

    // Editor mode should create a viewport framebuffer
    assert(engine.GetViewportFramebuffer() != nullptr);
    assert(engine.GetViewportFramebuffer()->IsValid());
    assert(engine.GetViewportFramebuffer()->GetWidth() == static_cast<uint32_t>(cfg.windowWidth));
    assert(engine.GetViewportFramebuffer()->GetHeight() == static_cast<uint32_t>(cfg.windowHeight));
    std::cout << "[PASS] test_engine_editor_has_viewport_framebuffer" << std::endl;
}

void test_engine_client_no_viewport_framebuffer() {
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
    engine.InitEditor();

    // Client mode should NOT create a viewport framebuffer
    assert(engine.GetViewportFramebuffer() == nullptr);
    std::cout << "[PASS] test_engine_client_no_viewport_framebuffer" << std::endl;
}

void test_engine_server_no_viewport_framebuffer() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.InitEditor();

    // Server mode should NOT create a viewport framebuffer
    assert(engine.GetViewportFramebuffer() == nullptr);
    std::cout << "[PASS] test_engine_server_no_viewport_framebuffer" << std::endl;
}
