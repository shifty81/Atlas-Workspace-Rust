#include "../engine/core/Engine.h"
#include <iostream>
#include <cassert>
#include <chrono>

using namespace atlas;

void test_engine_init_and_shutdown() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    assert(engine.Running());
    engine.Shutdown();
    assert(!engine.Running());

    std::cout << "[PASS] test_engine_init_and_shutdown" << std::endl;
}

void test_engine_run_loop_ticks() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 5;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    // Add a tick callback to count ticks
    int tickCount = 0;
    engine.GetWorld().SetTickCallback([&](float dt) {
        tickCount++;
        (void)dt;
    });

    engine.Run();

    assert(tickCount == 5);
    assert(!engine.Running());

    std::cout << "[PASS] test_engine_run_loop_ticks" << std::endl;
}

void test_engine_capabilities() {
    // Editor capabilities
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Editor;
        Engine engine(cfg);
        assert(engine.Can(Capability::AssetWrite));
        assert(engine.Can(Capability::Rendering));
        assert(engine.Can(Capability::GraphEdit));
        assert(engine.Can(Capability::HotReload));
        assert(!engine.Can(Capability::NetAuthority));
    }

    // Server capabilities
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        Engine engine(cfg);
        assert(!engine.Can(Capability::AssetWrite));
        assert(!engine.Can(Capability::Rendering));
        assert(!engine.Can(Capability::GraphEdit));
        assert(engine.Can(Capability::NetAuthority));
    }

    // Client capabilities
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Client;
        Engine engine(cfg);
        assert(!engine.Can(Capability::AssetWrite));
        assert(engine.Can(Capability::Rendering));
        assert(!engine.Can(Capability::GraphEdit));
        assert(!engine.Can(Capability::NetAuthority));
    }

    std::cout << "[PASS] test_engine_capabilities" << std::endl;
}

void test_engine_net_mode_from_config() {
    // Server config should init net in Server mode
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.maxTicks = 0;
        Engine engine(cfg);
        engine.InitCore();
        engine.InitNetworking();
        assert(engine.GetNet().Mode() == net::NetMode::Server);
        assert(engine.GetNet().IsAuthority());
    }

    // Client config should init net in Client mode
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Client;
        Engine engine(cfg);
        engine.InitCore();
        engine.InitNetworking();
        assert(engine.GetNet().Mode() == net::NetMode::Client);
        assert(!engine.GetNet().IsAuthority());
    }

    std::cout << "[PASS] test_engine_net_mode_from_config" << std::endl;
}

void test_engine_frame_pacing_default_enabled() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    Engine engine(cfg);

    // Frame pacing should be enabled by default
    assert(engine.GetScheduler().FramePacingEnabled());

    // Can disable and re-enable
    engine.GetScheduler().SetFramePacing(false);
    assert(!engine.GetScheduler().FramePacingEnabled());

    engine.GetScheduler().SetFramePacing(true);
    assert(engine.GetScheduler().FramePacingEnabled());

    std::cout << "[PASS] test_engine_frame_pacing_default_enabled" << std::endl;
}

void test_engine_frame_pacing_prevents_spin() {
    // With frame pacing on, 3 ticks at 30hz should take ~100ms, not instant
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 30;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    // Frame pacing is on by default

    auto start = std::chrono::steady_clock::now();
    engine.Run();
    auto elapsed = std::chrono::steady_clock::now() - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    // 3 ticks at 30hz = 100ms. Allow some tolerance but should be > 50ms
    // and under 300ms even on loaded CI systems
    assert(ms >= 50);
    assert(ms < 300);

    std::cout << "[PASS] test_engine_frame_pacing_prevents_spin" << std::endl;
}

void test_engine_ui_update_in_loop() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Client;
    cfg.tickRate = 60;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    // UIManager should be initialized after InitUI
    assert(engine.GetUIManager().IsInitialized());

    engine.Run();

    // Engine should stop after maxTicks
    assert(!engine.Running());

    std::cout << "[PASS] test_engine_ui_update_in_loop" << std::endl;
}
