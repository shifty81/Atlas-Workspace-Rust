#include <iostream>
#include <cassert>
#include <string>

#include "../engine/core/Engine.h"
#include "../editor/tools/PlayInEditor.h"

using namespace atlas;
using namespace atlas::editor;

void test_pie_initial_state() {
    PlayInEditor pie;
    assert(pie.Mode() == PIEMode::Stopped);
    assert(pie.PossessedEntity() == 0);
    assert(!pie.IsLoopbackActive());
    assert(pie.TicksSimulated() == 0);

    std::cout << "[PASS] test_pie_initial_state" << std::endl;
}

void test_pie_start_stop() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 0;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    PlayInEditor pie;
    assert(pie.StartSimulation(engine));
    assert(pie.Mode() == PIEMode::Simulating);

    assert(pie.StopSimulation(engine));
    assert(pie.Mode() == PIEMode::Stopped);

    std::cout << "[PASS] test_pie_start_stop" << std::endl;
}

void test_pie_pause_resume() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 0;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    PlayInEditor pie;
    pie.StartSimulation(engine);

    assert(pie.Pause());
    assert(pie.Mode() == PIEMode::Paused);

    assert(pie.Resume());
    assert(pie.Mode() == PIEMode::Simulating);

    pie.StopSimulation(engine);
    std::cout << "[PASS] test_pie_pause_resume" << std::endl;
}

void test_pie_step_tick() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 0;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    PlayInEditor pie;
    pie.StartSimulation(engine);
    pie.Pause();

    uint64_t tickBefore = engine.GetTimeModel().Context().sim.tick;
    assert(pie.StepTick(engine));
    assert(engine.GetTimeModel().Context().sim.tick == tickBefore + 1);
    assert(pie.TicksSimulated() == 1);

    pie.StopSimulation(engine);
    std::cout << "[PASS] test_pie_step_tick" << std::endl;
}

void test_pie_state_restore() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 0;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    // Create an entity before PIE
    engine.GetWorld().CreateEntity();
    uint32_t preCount = engine.GetWorld().EntityCount();
    uint64_t preTick = engine.GetTimeModel().Context().sim.tick;

    PlayInEditor pie;
    pie.StartSimulation(engine);

    // Modify state during simulation
    engine.GetWorld().CreateEntity();
    engine.GetTimeModel().AdvanceTick();

    assert(engine.GetWorld().EntityCount() > preCount);

    // Stop should restore state
    pie.StopSimulation(engine);
    assert(engine.GetWorld().EntityCount() == preCount);
    assert(engine.GetTimeModel().Context().sim.tick == preTick);

    std::cout << "[PASS] test_pie_state_restore" << std::endl;
}

void test_pie_possess_entity() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    PlayInEditor pie;
    pie.StartSimulation(engine);

    assert(pie.PossessEntity(42));
    assert(pie.PossessedEntity() == 42);
    assert(pie.Mode() == PIEMode::Possessed);

    assert(pie.Unpossess());
    assert(pie.PossessedEntity() == 0);
    assert(pie.Mode() == PIEMode::Simulating);

    pie.StopSimulation(engine);
    std::cout << "[PASS] test_pie_possess_entity" << std::endl;
}

void test_pie_possess_zero_fails() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    PlayInEditor pie;
    pie.StartSimulation(engine);

    assert(!pie.PossessEntity(0));
    assert(pie.PossessedEntity() == 0);

    pie.StopSimulation(engine);
    std::cout << "[PASS] test_pie_possess_zero_fails" << std::endl;
}

void test_pie_loopback() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    PlayInEditor pie;
    PIEConfig pieConfig;
    pieConfig.loopback = true;
    pie.StartSimulation(engine, pieConfig);

    assert(pie.IsLoopbackActive());
    assert(engine.GetNet().Mode() == net::NetMode::Server);

    pie.StopSimulation(engine);
    std::cout << "[PASS] test_pie_loopback" << std::endl;
}

void test_pie_mode_callback() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    PlayInEditor pie;
    std::vector<std::pair<PIEMode, PIEMode>> transitions;
    pie.SetModeCallback([&transitions](PIEMode oldMode, PIEMode newMode) {
        transitions.push_back({oldMode, newMode});
    });

    pie.StartSimulation(engine);
    assert(transitions.size() == 1);
    assert(transitions[0].first == PIEMode::Stopped);
    assert(transitions[0].second == PIEMode::Simulating);

    pie.Pause();
    assert(transitions.size() == 2);

    pie.Resume();
    assert(transitions.size() == 3);

    pie.StopSimulation(engine);
    assert(transitions.size() == 4);
    assert(transitions[3].second == PIEMode::Stopped);

    std::cout << "[PASS] test_pie_mode_callback" << std::endl;
}

void test_pie_double_start_fails() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    PlayInEditor pie;
    assert(pie.StartSimulation(engine));
    assert(!pie.StartSimulation(engine)); // already running

    pie.StopSimulation(engine);
    std::cout << "[PASS] test_pie_double_start_fails" << std::endl;
}

void test_pie_stop_when_stopped() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    PlayInEditor pie;
    assert(!pie.StopSimulation(engine)); // already stopped

    std::cout << "[PASS] test_pie_stop_when_stopped" << std::endl;
}

void test_pie_auto_possess() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    PlayInEditor pie;
    PIEConfig pieConfig;
    pieConfig.autoPossessEntity = 99;

    pie.StartSimulation(engine, pieConfig);
    assert(pie.PossessedEntity() == 99);

    pie.StopSimulation(engine);
    std::cout << "[PASS] test_pie_auto_possess" << std::endl;
}
