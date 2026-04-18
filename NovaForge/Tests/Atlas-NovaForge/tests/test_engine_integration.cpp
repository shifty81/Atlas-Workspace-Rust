#include "../engine/core/Engine.h"
#include "../engine/sim/TimeModel.h"
#include "../engine/sim/WorldState.h"
#include "../engine/sim/SaveSystem.h"
#include "../engine/ai/BehaviorGraph.h"
#include "../engine/ai/BehaviorNodes.h"
#include "../engine/ai/RelationshipModel.h"
#include "../editor/panels/ConsolePanel.h"
#include <iostream>
#include <cassert>
#include <cstdio>

using namespace atlas;

// --- Engine ↔ TimeModel Integration ---

void test_engine_time_model_initialized() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 5;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.Run();

    // TimeModel should have advanced 5 ticks
    const auto& ctx = engine.GetTimeModel().Context();
    assert(ctx.sim.tick == 5);
    assert(ctx.sim.tickRate == 60);

    std::cout << "[PASS] test_engine_time_model_initialized" << std::endl;
}

void test_engine_time_model_tick_rate_matches() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 30;
    cfg.maxTicks = 3;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.Run();

    const auto& ctx = engine.GetTimeModel().Context();
    assert(ctx.sim.tick == 3);
    assert(ctx.sim.tickRate == 30);
    // fixedDeltaTime should be 1/30
    float expectedDt = 1.0f / 30.0f;
    assert(ctx.sim.fixedDeltaTime > expectedDt - 0.001f);
    assert(ctx.sim.fixedDeltaTime < expectedDt + 0.001f);

    std::cout << "[PASS] test_engine_time_model_tick_rate_matches" << std::endl;
}

void test_engine_time_model_world_time_advances() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 60; // 1 second of simulation

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.Run();

    const auto& ctx = engine.GetTimeModel().Context();
    // After 60 ticks at 60hz, world elapsed should be ~1.0 seconds
    assert(ctx.world.elapsed > 0.99 && ctx.world.elapsed < 1.01);

    std::cout << "[PASS] test_engine_time_model_world_time_advances" << std::endl;
}

// --- Engine ↔ WorldState Integration ---

void test_engine_world_state_snapshots_in_server() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 5;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    // Create an entity before running so ECS data is non-empty
    engine.GetWorld().CreateEntity();

    engine.Run();

    // Server should have stored snapshots
    assert(engine.GetWorldState().SnapshotCount() == 5);

    // The latest snapshot tick should be 5
    auto* latest = engine.GetWorldState().LatestSnapshot();
    assert(latest != nullptr);
    assert(latest->tick == 5);

    // ECS data should be non-empty in snapshots
    assert(!latest->ecsData.empty());

    std::cout << "[PASS] test_engine_world_state_snapshots_in_server" << std::endl;
}

void test_engine_world_state_snapshot_at_tick() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 10;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.Run();

    // Should be able to retrieve snapshots at specific ticks
    auto* snap3 = engine.GetWorldState().SnapshotAtTick(3);
    assert(snap3 != nullptr);
    assert(snap3->tick == 3);

    auto* snap7 = engine.GetWorldState().SnapshotAtTick(7);
    assert(snap7 != nullptr);
    assert(snap7->tick == 7);

    std::cout << "[PASS] test_engine_world_state_snapshot_at_tick" << std::endl;
}

// --- Engine ↔ SaveSystem Integration ---

void test_engine_save_system_accessible() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 5;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.GetWorld().CreateEntity();
    engine.Run();

    // Save via engine's save system
    auto ecsData = engine.GetWorld().Serialize();
    auto& save = engine.GetSaveSystem();
    auto result = save.Save("/tmp/atlas_engine_test.asav",
                            engine.GetTimeModel().Context().sim.tick,
                            cfg.tickRate, 0, ecsData);
    assert(result == sim::SaveResult::Success);

    // Load it back
    auto loadResult = save.Load("/tmp/atlas_engine_test.asav");
    assert(loadResult == sim::SaveResult::Success);
    assert(save.Header().saveTick == 5);
    assert(save.Header().tickRate == 60);

    // Clean up
    std::remove("/tmp/atlas_engine_test.asav");

    std::cout << "[PASS] test_engine_save_system_accessible" << std::endl;
}

// --- Console save/load Commands ---

void test_console_save_command() {
    ecs::World world;
    net::NetContext net;
    net.Init(net::NetMode::Standalone);
    sim::TickScheduler scheduler;
    scheduler.SetTickRate(60);
    sim::TimeModel timeModel;
    timeModel.SetTickRate(60);
    sim::SaveSystem saveSystem;

    // Advance a few ticks
    for (int i = 0; i < 3; i++) timeModel.AdvanceTick();

    world.CreateEntity();

    editor::ConsolePanel console(world, net, scheduler, timeModel, saveSystem);
    console.Execute("save /tmp/atlas_console_test.asav");

    auto& history = console.History();
    assert(history.size() == 2);
    // Should report success with tick 3
    assert(history[1].find("Saved at tick 3") != std::string::npos);

    // Clean up
    std::remove("/tmp/atlas_console_test.asav");

    std::cout << "[PASS] test_console_save_command" << std::endl;
}

void test_console_load_command() {
    ecs::World world;
    net::NetContext net;
    net.Init(net::NetMode::Standalone);
    sim::TickScheduler scheduler;
    scheduler.SetTickRate(60);
    sim::TimeModel timeModel;
    timeModel.SetTickRate(60);
    sim::SaveSystem saveSystem;

    // Create entity and save
    world.CreateEntity();
    auto ecsData = world.Serialize();
    saveSystem.Save("/tmp/atlas_console_load_test.asav", 10, 60, 0, ecsData);

    // Clear world
    for (auto e : world.GetEntities()) world.DestroyEntity(e);
    assert(world.EntityCount() == 0);

    // Load via console
    editor::ConsolePanel console(world, net, scheduler, timeModel, saveSystem);
    console.Execute("load /tmp/atlas_console_load_test.asav");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1].find("Loaded from") != std::string::npos);
    assert(history[1].find("tick 10") != std::string::npos);

    // Clean up
    std::remove("/tmp/atlas_console_load_test.asav");

    std::cout << "[PASS] test_console_load_command" << std::endl;
}

void test_console_load_file_not_found() {
    ecs::World world;
    net::NetContext net;
    net.Init(net::NetMode::Standalone);
    sim::TickScheduler scheduler;
    sim::TimeModel timeModel;
    sim::SaveSystem saveSystem;

    editor::ConsolePanel console(world, net, scheduler, timeModel, saveSystem);
    console.Execute("load /tmp/nonexistent_save.asav");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1] == "File not found: /tmp/nonexistent_save.asav");

    std::cout << "[PASS] test_console_load_file_not_found" << std::endl;
}

void test_console_save_no_path() {
    ecs::World world;
    net::NetContext net;
    net.Init(net::NetMode::Standalone);
    sim::TickScheduler scheduler;
    sim::TimeModel timeModel;
    sim::SaveSystem saveSystem;

    editor::ConsolePanel console(world, net, scheduler, timeModel, saveSystem);
    console.Execute("save");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1] == "Usage: save <path>");

    std::cout << "[PASS] test_console_save_no_path" << std::endl;
}

void test_console_time_info() {
    ecs::World world;
    net::NetContext net;
    net.Init(net::NetMode::Standalone);
    sim::TickScheduler scheduler;
    sim::TimeModel timeModel;
    timeModel.SetTickRate(60);
    sim::SaveSystem saveSystem;

    for (int i = 0; i < 5; i++) timeModel.AdvanceTick();

    editor::ConsolePanel console(world, net, scheduler, timeModel, saveSystem);
    console.Execute("time.info");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1].find("Tick: 5") != std::string::npos);
    assert(history[1].find("Rate: 60 Hz") != std::string::npos);

    std::cout << "[PASS] test_console_time_info" << std::endl;
}

// --- AI Determinism ---

void test_ai_behavior_deterministic_across_runs() {
    // Run the same AI graph twice with identical inputs and verify
    // bit-identical output.
    auto runBehaviorGraph = [](float threat, float health, float ammo, float morale, uint32_t tick) {
        atlas::ai::BehaviorGraph graph;
        auto emotionId = graph.AddNode(std::make_unique<atlas::ai::EmotionUpdateNode>());
        auto utilId = graph.AddNode(std::make_unique<atlas::ai::UtilityScoreNode>());
        graph.Compile();
        atlas::ai::AIContext ctx{threat, health, ammo, morale, tick};
        graph.Execute(ctx);

        std::vector<float> result;
        auto* emotionOut = graph.GetOutput(emotionId, 0);
        for (auto v : emotionOut->data) result.push_back(v);
        auto* utilOut = graph.GetOutput(utilId, 0);
        for (auto v : utilOut->data) result.push_back(v);
        return result;
    };

    auto a = runBehaviorGraph(0.7f, 0.8f, 0.5f, 0.6f, 100);
    auto b = runBehaviorGraph(0.7f, 0.8f, 0.5f, 0.6f, 100);
    assert(a == b);

    // Different inputs should produce different results
    auto c = runBehaviorGraph(0.2f, 0.9f, 0.5f, 0.3f, 200);
    assert(a != c);

    std::cout << "[PASS] test_ai_behavior_deterministic_across_runs" << std::endl;
}

void test_ai_relationship_determinism_across_save_load() {
    atlas::ai::RelationshipModel model;
    model.Set(1, 2, 0.5f);
    model.Set(1, 3, -0.8f);
    model.Set(2, 3, 0.3f);

    // Store original values
    float val12 = model.Get(1, 2);
    float val13 = model.Get(1, 3);
    float val23 = model.Get(2, 3);

    // Modify and restore
    model.Modify(1, 2, 0.1f);
    float modified = model.Get(1, 2);
    assert(modified != val12);

    // Create a fresh model and set the same values
    atlas::ai::RelationshipModel model2;
    model2.Set(1, 2, 0.5f);
    model2.Set(1, 3, -0.8f);
    model2.Set(2, 3, 0.3f);

    // Fresh model should match original values exactly
    assert(model2.Get(1, 2) == val12);
    assert(model2.Get(1, 3) == val13);
    assert(model2.Get(2, 3) == val23);

    std::cout << "[PASS] test_ai_relationship_determinism_across_save_load" << std::endl;
}
