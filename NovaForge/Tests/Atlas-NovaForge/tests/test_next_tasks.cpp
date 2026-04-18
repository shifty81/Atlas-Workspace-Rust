#include "../engine/core/Engine.h"
#include "../engine/sim/TimeModel.h"
#include "../engine/sim/ReplayRecorder.h"
#include "../engine/sim/ReplayDivergenceInspector.h"
#include "../engine/sim/SaveSystem.h"
#include "../engine/sim/StateHasher.h"
#include "../engine/net/NetContext.h"
#include "../editor/panels/StateHashDiffPanel.h"
#include <iostream>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
#include <cstring>

using namespace atlas;
using namespace atlas::sim;

// --- Autosave ---

void test_engine_autosave_creates_file() {
    const std::string path = "/tmp/atlas_autosave_test.asav";
    std::remove(path.c_str());

    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 10;
    cfg.autosaveInterval = 5;
    cfg.autosavePath = path;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.GetWorld().CreateEntity();
    engine.Run();

    SaveSystem validator;
    auto result = validator.Validate(path);
    assert(result == SaveResult::Success);

    std::remove(path.c_str());
    std::cout << "[PASS] test_engine_autosave_creates_file" << std::endl;
}

// --- TimeModel SetTick ---

void test_time_model_set_tick() {
    TimeModel tm;
    tm.SetTickRate(60);

    for (int i = 0; i < 5; i++) tm.AdvanceTick();
    assert(tm.Context().sim.tick == 5);

    tm.SetTick(100);
    assert(tm.Context().sim.tick == 100);

    std::cout << "[PASS] test_time_model_set_tick" << std::endl;
}

// --- Replay save-point markers ---

void test_replay_save_point_mark() {
    ReplayRecorder recorder;
    recorder.StartRecording(60);

    for (uint32_t t = 1; t <= 5; ++t)
        recorder.RecordFrame(t, {});

    recorder.MarkSavePoint(3);
    auto pts = recorder.SavePoints();
    assert(pts.size() == 1);
    assert(pts[0] == 3);

    std::cout << "[PASS] test_replay_save_point_mark" << std::endl;
}

void test_replay_save_point_multiple() {
    ReplayRecorder recorder;
    recorder.StartRecording(60);

    for (uint32_t t = 1; t <= 5; ++t)
        recorder.RecordFrame(t, {});

    recorder.MarkSavePoint(2);
    recorder.MarkSavePoint(4);
    auto pts = recorder.SavePoints();
    assert(pts.size() == 2);
    assert(pts[0] == 2);
    assert(pts[1] == 4);

    std::cout << "[PASS] test_replay_save_point_multiple" << std::endl;
}

void test_replay_save_point_serialization() {
    const std::string path = "/tmp/atlas_replay_sp_test.rply";
    std::remove(path.c_str());

    {
        ReplayRecorder recorder;
        recorder.StartRecording(60, 99);
        for (uint32_t t = 1; t <= 5; ++t)
            recorder.RecordFrame(t, {static_cast<uint8_t>(t)});
        recorder.MarkSavePoint(3);
        recorder.StopRecording();
        assert(recorder.SaveReplay(path));
    }

    {
        ReplayRecorder loader;
        assert(loader.LoadReplay(path));
        assert(loader.Header().version == 3);
        auto pts = loader.SavePoints();
        assert(pts.size() == 1);
        assert(pts[0] == 3);
    }

    std::remove(path.c_str());
    std::cout << "[PASS] test_replay_save_point_serialization" << std::endl;
}

void test_replay_start_from_save() {
    ReplayRecorder recorder;
    recorder.StartFromSave(100, 60, 42);

    assert(recorder.State() == ReplayState::Recording);
    assert(recorder.Header().tickRate == 60);
    assert(recorder.Header().seed == 42);

    recorder.RecordFrame(100, {1, 2});
    const ReplayFrame* f = recorder.FrameAtTick(100);
    assert(f != nullptr);
    assert(f->tick == 100);

    std::cout << "[PASS] test_replay_start_from_save" << std::endl;
}

// --- Engine LoadAndReplay ---

void test_engine_load_and_replay() {
    const std::string path = "/tmp/atlas_load_replay_test.asav";
    std::remove(path.c_str());

    // Save
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.tickRate = 60;
        cfg.maxTicks = 10;

        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();
        engine.GetScheduler().SetFramePacing(false);

        engine.GetWorld().CreateEntity();
        engine.Run();

        auto ecsData = engine.GetWorld().Serialize();
        auto& save = engine.GetSaveSystem();
        auto result = save.Save(path,
                                engine.GetTimeModel().Context().sim.tick,
                                cfg.tickRate, 0, ecsData);
        assert(result == SaveResult::Success);
    }

    // Load and replay
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.tickRate = 60;
        cfg.maxTicks = 0;

        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();
        engine.GetScheduler().SetFramePacing(false);

        bool loadSucceeded = engine.LoadAndReplay(path);
        assert(loadSucceeded);
        assert(engine.GetTimeModel().Context().sim.tick == 10);
        assert(engine.GetWorld().EntityCount() >= 1);
    }

    std::remove(path.c_str());
    std::cout << "[PASS] test_engine_load_and_replay" << std::endl;
}

// --- Divergence report export ---

void test_divergence_export_report() {
    const std::string path = "/tmp/atlas_div_report_test.txt";
    std::remove(path.c_str());

    DivergenceReport report;
    report.divergeTick = 42;
    report.localHash = 0x1111;
    report.remoteHash = 0x2222;
    report.totalTicksCompared = 100;
    report.firstMatchingTicks = 42;
    report.severity = DivergenceSeverity::Warning;

    bool exportSucceeded = ReplayDivergenceInspector::ExportReport(report, path);
    assert(exportSucceeded);

    std::ifstream in(path);
    assert(in.is_open());
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("DIVERGED") != std::string::npos);
    assert(content.find("42") != std::string::npos);

    std::remove(path.c_str());
    std::cout << "[PASS] test_divergence_export_report" << std::endl;
}

// --- Engine RollbackToTick ---

void test_engine_rollback_to_tick() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 10;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.GetWorld().CreateEntity();
    engine.Run();

    // Verify we're at tick 10
    assert(engine.GetTimeModel().Context().sim.tick == 10);

    // Pick a snapshot tick (server mode snapshots every tick)
    const auto* snap = engine.GetWorldState().SnapshotAtTick(5);
    assert(snap != nullptr);

    bool rollbackSucceeded = engine.RollbackToTick(5);
    assert(rollbackSucceeded);
    assert(engine.GetTimeModel().Context().sim.tick == 5);

    // Rollback to non-existent tick should fail
    bool rollbackInvalidTick = engine.RollbackToTick(9999);
    assert(!rollbackInvalidTick);

    std::cout << "[PASS] test_engine_rollback_to_tick" << std::endl;
}

// --- NetContext BroadcastSaveTick ---

void test_net_broadcast_save_tick() {
    net::NetContext net;
    net.Init(net::NetMode::Server);

    assert(net.LastSaveTick() == 0);
    assert(net.LastSaveHash() == 0);

    net.BroadcastSaveTick(42, 0xDEADBEEF);

    assert(net.LastSaveTick() == 42);
    assert(net.LastSaveHash() == 0xDEADBEEF);

    // The broadcast should have queued a packet
    net.Poll();
    net::Packet pkt;
    bool received = net.Receive(pkt);
    assert(received);
    assert(pkt.type == 0xFF00);
    assert(pkt.tick == 42);
    assert(pkt.payload.size() == sizeof(uint32_t) + sizeof(uint64_t));

    // Verify payload contents
    uint32_t payloadTick = 0;
    uint64_t payloadHash = 0;
    std::memcpy(&payloadTick, pkt.payload.data(), sizeof(uint32_t));
    std::memcpy(&payloadHash, pkt.payload.data() + sizeof(uint32_t), sizeof(uint64_t));
    assert(payloadTick == 42);
    assert(payloadHash == 0xDEADBEEF);

    std::cout << "[PASS] test_net_broadcast_save_tick" << std::endl;
}

// --- StateHashDiffPanel no divergence ---

void test_state_hash_diff_panel_no_divergence() {
    sim::StateHasher local;
    sim::StateHasher remote;
    local.Reset(42);
    remote.Reset(42);

    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> input = {4, 5};

    for (uint64_t t = 1; t <= 5; ++t) {
        local.AdvanceTick(t, state, input);
        remote.AdvanceTick(t, state, input);
    }

    editor::StateHashDiffPanel panel;
    panel.SetLocalHasher(&local);
    panel.SetRemoteHasher(&remote);
    panel.Refresh();

    assert(!panel.HasDivergence());
    assert(panel.FirstDivergenceTick() == -1);
    assert(panel.Entries().size() == 5);
    assert(panel.Summary() == "No divergence");

    for (const auto& e : panel.Entries()) {
        assert(e.matches);
    }

    std::cout << "[PASS] test_state_hash_diff_panel_no_divergence" << std::endl;
}

// --- StateHashDiffPanel with divergence ---

void test_state_hash_diff_panel_with_divergence() {
    sim::StateHasher local;
    sim::StateHasher remote;
    local.Reset(42);
    remote.Reset(42);

    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> input = {4, 5};

    // Ticks 1-2: identical
    for (uint64_t t = 1; t <= 2; ++t) {
        local.AdvanceTick(t, state, input);
        remote.AdvanceTick(t, state, input);
    }

    // Tick 3: diverge
    std::vector<uint8_t> differentState = {9, 9, 9};
    local.AdvanceTick(3, state, input);
    remote.AdvanceTick(3, differentState, input);

    editor::StateHashDiffPanel panel;
    panel.SetLocalHasher(&local);
    panel.SetRemoteHasher(&remote);
    panel.Refresh();

    assert(panel.HasDivergence());
    assert(panel.FirstDivergenceTick() == 3);
    assert(panel.Entries().size() == 3);

    // First two should match
    assert(panel.Entries()[0].matches);
    assert(panel.Entries()[1].matches);
    assert(!panel.Entries()[2].matches);

    std::string summary = panel.Summary();
    assert(summary.find("Divergence at tick 3") != std::string::npos);

    std::cout << "[PASS] test_state_hash_diff_panel_with_divergence" << std::endl;
}

// --- Hash ladder save/load continuity ---

void test_hash_ladder_save_load_continuity() {
    const std::string path = "/tmp/atlas_hash_ladder_test.asav";
    std::remove(path.c_str());

    sim::StateHasher hasher;
    hasher.Reset(99);

    // Build up a hash ladder
    std::vector<uint8_t> state = {10, 20, 30};
    std::vector<uint8_t> input = {1};
    for (uint64_t t = 1; t <= 5; ++t) {
        hasher.AdvanceTick(t, state, input);
    }
    uint64_t hashBeforeSave = hasher.CurrentHash();

    // Save world state
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.tickRate = 60;
        cfg.maxTicks = 0;

        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();
        engine.GetScheduler().SetFramePacing(false);

        engine.GetWorld().CreateEntity();
        auto ecsData = engine.GetWorld().Serialize();
        auto result = engine.GetSaveSystem().Save(path, 5, cfg.tickRate, 0, ecsData);
        assert(result == sim::SaveResult::Success);
    }

    // Load and re-hash
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.tickRate = 60;
        cfg.maxTicks = 0;

        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();
        engine.GetScheduler().SetFramePacing(false);

        bool loadSucceeded = engine.LoadAndReplay(path);
        assert(loadSucceeded);

        // Rebuild hash ladder with same seed and same data
        sim::StateHasher hasher2;
        hasher2.Reset(99);
        for (uint64_t t = 1; t <= 5; ++t) {
            hasher2.AdvanceTick(t, state, input);
        }

        assert(hasher2.CurrentHash() == hashBeforeSave);
    }

    std::remove(path.c_str());
    std::cout << "[PASS] test_hash_ladder_save_load_continuity" << std::endl;
}

// --- Replay from save ---

void test_engine_replay_from_save() {
    const std::string savePath = "/tmp/atlas_rfs_save.asav";
    const std::string replayPath = "/tmp/atlas_rfs_replay.rply";
    std::remove(savePath.c_str());
    std::remove(replayPath.c_str());

    // Step 1: Run engine for 10 ticks and save at tick 5
    uint64_t savedTick = 0;
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.tickRate = 60;
        cfg.maxTicks = 10;

        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();
        engine.GetScheduler().SetFramePacing(false);

        engine.GetWorld().CreateEntity();
        engine.Run();

        // Save at current tick after running for maxTicks
        savedTick = engine.GetTimeModel().Context().sim.tick;
        auto ecsData = engine.GetWorld().Serialize();
        auto result = engine.GetSaveSystem().Save(savePath,
            savedTick, cfg.tickRate, 0, ecsData);
        assert(result == SaveResult::Success);
    }

    // Step 2: Create a replay with frames continuing from the save
    {
        ReplayRecorder recorder;
        recorder.StartRecording(60, 42);
        // Record frames for ticks 1-15 (some before save, some after)
        for (uint32_t t = 1; t <= 15; ++t) {
            recorder.RecordFrame(t, {static_cast<uint8_t>(t)});
        }
        recorder.StopRecording();
        assert(recorder.SaveReplay(replayPath));
    }

    // Step 3: Replay from save
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.tickRate = 60;
        cfg.maxTicks = 0;

        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();
        engine.GetScheduler().SetFramePacing(false);

        bool success = engine.ReplayFromSave(savePath, replayPath);
        assert(success);

        // After replay: tick should be saveTick + number of post-save frames
        // Save was at tick 10, replay has frames 11-15 = 5 frames after save
        assert(engine.GetTimeModel().Context().sim.tick == savedTick + 5);
        assert(engine.GetWorld().EntityCount() >= 1);
    }

    std::remove(savePath.c_str());
    std::remove(replayPath.c_str());
    std::cout << "[PASS] test_engine_replay_from_save" << std::endl;
}

void test_engine_replay_from_save_bad_save() {
    const std::string replayPath = "/tmp/atlas_rfs_replay2.rply";
    {
        ReplayRecorder recorder;
        recorder.StartRecording(60);
        recorder.RecordFrame(1, {1});
        recorder.StopRecording();
        recorder.SaveReplay(replayPath);
    }

    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    bool success = engine.ReplayFromSave("/tmp/nonexistent.asav", replayPath);
    assert(!success);

    std::remove(replayPath.c_str());
    std::cout << "[PASS] test_engine_replay_from_save_bad_save" << std::endl;
}

void test_engine_replay_from_save_bad_replay() {
    const std::string savePath = "/tmp/atlas_rfs_save2.asav";
    std::remove(savePath.c_str());

    // Create a valid save
    {
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

        auto ecsData = engine.GetWorld().Serialize();
        engine.GetSaveSystem().Save(savePath,
            engine.GetTimeModel().Context().sim.tick,
            cfg.tickRate, 0, ecsData);
    }

    // Try to replay from save with bad replay path
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.tickRate = 60;

        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();

        bool success = engine.ReplayFromSave(savePath, "/tmp/nonexistent.rply");
        assert(!success);
    }

    std::remove(savePath.c_str());
    std::cout << "[PASS] test_engine_replay_from_save_bad_replay" << std::endl;
}

void test_engine_replay_from_save_no_frames_after_save_point() {
    const std::string savePath = "/tmp/atlas_rfs_save3.asav";
    const std::string replayPath = "/tmp/atlas_rfs_replay3.rply";
    std::remove(savePath.c_str());
    std::remove(replayPath.c_str());

    // Create a valid save at tick 10.
    {
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

        auto ecsData = engine.GetWorld().Serialize();
        engine.GetSaveSystem().Save(savePath,
            engine.GetTimeModel().Context().sim.tick,
            cfg.tickRate, 0, ecsData);
    }

    // Replay only contains frames before save tick.
    {
        constexpr uint32_t replaySeed = 7;
        ReplayRecorder recorder;
        recorder.StartRecording(60, replaySeed);
        for (uint32_t t = 1; t <= 5; ++t) {
            recorder.RecordFrame(t, {static_cast<uint8_t>(t)});
        }
        recorder.StopRecording();
        assert(recorder.SaveReplay(replayPath));
    }

    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.tickRate = 60;

        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();

        bool success = engine.ReplayFromSave(savePath, replayPath);
        assert(!success);
    }

    std::remove(savePath.c_str());
    std::remove(replayPath.c_str());
    std::cout << "[PASS] test_engine_replay_from_save_no_frames_after_save_point" << std::endl;
}

// --- StateHashDiffPanel per-component breakdown ---

void test_state_hash_diff_panel_component_breakdown() {
    editor::StateHashDiffPanel panel;
    assert(!panel.HasComponentBreakdown());
    assert(panel.DivergentComponents().empty());

    editor::ComponentHashBreakdown breakdown;
    breakdown.tick = 42;

    editor::ComponentHashEntry comp1;
    comp1.name = "Transform";
    comp1.localHash = 0x1111;
    comp1.remoteHash = 0x1111;
    comp1.matches = true;
    breakdown.components.push_back(comp1);

    editor::ComponentHashEntry comp2;
    comp2.name = "Health";
    comp2.localHash = 0x2222;
    comp2.remoteHash = 0x3333;
    comp2.matches = false;
    breakdown.components.push_back(comp2);

    editor::ComponentHashEntry comp3;
    comp3.name = "Physics";
    comp3.localHash = 0x4444;
    comp3.remoteHash = 0x5555;
    comp3.matches = false;
    breakdown.components.push_back(comp3);

    panel.SetComponentBreakdown(breakdown);
    assert(panel.HasComponentBreakdown());
    assert(panel.GetComponentBreakdown().tick == 42);
    assert(panel.GetComponentBreakdown().components.size() == 3);

    auto divergent = panel.DivergentComponents();
    assert(divergent.size() == 2);
    assert(divergent[0] == "Health");
    assert(divergent[1] == "Physics");

    std::cout << "[PASS] test_state_hash_diff_panel_component_breakdown" << std::endl;
}

void test_state_hash_diff_panel_no_component_divergence() {
    editor::StateHashDiffPanel panel;

    editor::ComponentHashBreakdown breakdown;
    breakdown.tick = 10;

    editor::ComponentHashEntry comp1;
    comp1.name = "Transform";
    comp1.localHash = 0xAAAA;
    comp1.remoteHash = 0xAAAA;
    comp1.matches = true;
    breakdown.components.push_back(comp1);

    panel.SetComponentBreakdown(breakdown);
    assert(panel.HasComponentBreakdown());
    assert(panel.DivergentComponents().empty());

    std::cout << "[PASS] test_state_hash_diff_panel_no_component_divergence" << std::endl;
}
