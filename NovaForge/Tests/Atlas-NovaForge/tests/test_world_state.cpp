#include "../engine/sim/WorldState.h"
#include "../engine/core/contract/SimulationGuard.h"
#include <iostream>
#include <cassert>

using namespace atlas::sim;

void test_world_state_register_block() {
    WorldState ws;
    ws.RegisterBlock("ECS", StateCategory::Simulated, 1, 4096);
    ws.RegisterBlock("PathCache", StateCategory::Derived, 2, 1024);

    assert(ws.RegisteredBlocks().size() == 2);
    assert(ws.RegisteredBlocks()[0].name == "ECS");
    assert(ws.RegisteredBlocks()[0].category == StateCategory::Simulated);
    assert(ws.RegisteredBlocks()[1].name == "PathCache");
    assert(ws.RegisteredBlocks()[1].category == StateCategory::Derived);

    // Duplicate registration should be ignored
    ws.RegisterBlock("ECS", StateCategory::Simulated, 1, 4096);
    assert(ws.RegisteredBlocks().size() == 2);

    std::cout << "[PASS] test_world_state_register_block" << std::endl;
}

void test_world_state_find_block() {
    WorldState ws;
    ws.RegisterBlock("ECS", StateCategory::Simulated, 1);
    ws.RegisterBlock("VFX", StateCategory::Presentation, 2);

    const auto* ecs = ws.FindBlock("ECS");
    assert(ecs != nullptr);
    assert(ecs->typeTag == 1);

    const auto* missing = ws.FindBlock("Missing");
    assert(missing == nullptr);

    std::cout << "[PASS] test_world_state_find_block" << std::endl;
}

void test_world_state_snapshot() {
    WorldState ws;

    std::vector<uint8_t> ecsData = {1, 2, 3, 4};
    std::vector<uint8_t> auxData = {5, 6};

    WorldSnapshot snap = ws.TakeSnapshot(10, ecsData, auxData);
    assert(snap.tick == 10);
    assert(snap.stateHash != 0);
    assert(snap.ecsData == ecsData);
    assert(snap.auxiliaryData == auxData);

    std::cout << "[PASS] test_world_state_snapshot" << std::endl;
}

void test_world_state_snapshot_deterministic() {
    WorldState ws;

    std::vector<uint8_t> data = {10, 20, 30};
    WorldSnapshot s1 = ws.TakeSnapshot(1, data);
    WorldSnapshot s2 = ws.TakeSnapshot(1, data);
    assert(s1.stateHash == s2.stateHash);

    std::cout << "[PASS] test_world_state_snapshot_deterministic" << std::endl;
}

void test_world_state_push_and_retrieve() {
    WorldState ws;

    std::vector<uint8_t> data1 = {1};
    std::vector<uint8_t> data2 = {2};

    ATLAS_SIM_TICK_BEGIN();
    ws.PushSnapshot(ws.TakeSnapshot(1, data1));
    ws.PushSnapshot(ws.TakeSnapshot(2, data2));
    ATLAS_SIM_TICK_END();

    assert(ws.SnapshotCount() == 2);

    const auto* latest = ws.LatestSnapshot();
    assert(latest != nullptr);
    assert(latest->tick == 2);

    const auto* at1 = ws.SnapshotAtTick(1);
    assert(at1 != nullptr);
    assert(at1->tick == 1);

    const auto* missing = ws.SnapshotAtTick(99);
    assert(missing == nullptr);

    std::cout << "[PASS] test_world_state_push_and_retrieve" << std::endl;
}

void test_world_state_max_snapshots() {
    WorldState ws;
    ws.SetMaxSnapshots(3);
    assert(ws.MaxSnapshots() == 3);

    ATLAS_SIM_TICK_BEGIN();
    for (uint64_t i = 0; i < 5; ++i) {
        std::vector<uint8_t> data = {static_cast<uint8_t>(i)};
        ws.PushSnapshot(ws.TakeSnapshot(i, data));
    }
    ATLAS_SIM_TICK_END();

    // Should only retain the latest 3
    assert(ws.SnapshotCount() == 3);
    assert(ws.SnapshotAtTick(0) == nullptr);
    assert(ws.SnapshotAtTick(1) == nullptr);
    assert(ws.SnapshotAtTick(2) != nullptr);

    std::cout << "[PASS] test_world_state_max_snapshots" << std::endl;
}

void test_world_state_prune() {
    WorldState ws;

    ATLAS_SIM_TICK_BEGIN();
    for (uint64_t i = 0; i < 10; ++i) {
        std::vector<uint8_t> data = {static_cast<uint8_t>(i)};
        ws.PushSnapshot(ws.TakeSnapshot(i, data));
    }
    ATLAS_SIM_TICK_END();

    ws.PruneSnapshotsBefore(5);
    assert(ws.SnapshotCount() == 5);
    assert(ws.SnapshotAtTick(4) == nullptr);
    assert(ws.SnapshotAtTick(5) != nullptr);

    std::cout << "[PASS] test_world_state_prune" << std::endl;
}

void test_world_state_clear() {
    WorldState ws;
    std::vector<uint8_t> data = {1};
    ATLAS_SIM_TICK_BEGIN();
    ws.PushSnapshot(ws.TakeSnapshot(1, data));
    ATLAS_SIM_TICK_END();
    assert(ws.SnapshotCount() == 1);

    ws.ClearSnapshots();
    assert(ws.SnapshotCount() == 0);
    assert(ws.LatestSnapshot() == nullptr);

    std::cout << "[PASS] test_world_state_clear" << std::endl;
}

void test_world_state_derived_rebuild() {
    WorldState ws;

    bool rebuilt = false;
    uint64_t rebuildTick = 0;

    ws.SetDerivedRebuildCallback([&](const WorldSnapshot& snap) {
        rebuilt = true;
        rebuildTick = snap.tick;
    });

    // No snapshots, rebuild should be a no-op
    ws.RebuildDerived();
    assert(!rebuilt);

    std::vector<uint8_t> data = {1};
    ATLAS_SIM_TICK_BEGIN();
    ws.PushSnapshot(ws.TakeSnapshot(42, data));
    ATLAS_SIM_TICK_END();
    ws.RebuildDerived();
    assert(rebuilt);
    assert(rebuildTick == 42);

    std::cout << "[PASS] test_world_state_derived_rebuild" << std::endl;
}
