#include "../engine/net/NetContext.h"
#include "../engine/ecs/ECS.h"
#include <iostream>
#include <cassert>

using namespace atlas::net;
using namespace atlas::ecs;

struct SnapPosition {
    float x = 0.0f;
    float y = 0.0f;
};

struct SnapHealth {
    int hp = 100;
};

void test_snapshot_saves_ecs_state() {
    World world;
    world.RegisterComponent<SnapPosition>(1);
    world.RegisterComponent<SnapHealth>(2);

    NetContext net;
    net.Init(NetMode::Server);
    net.SetWorld(&world);

    EntityID e = world.CreateEntity();
    world.AddComponent<SnapPosition>(e, {10.0f, 20.0f});
    world.AddComponent<SnapHealth>(e, {75});

    net.SaveSnapshot(1);

    auto& snaps = net.Snapshots();
    assert(snaps.size() == 1);
    assert(snaps[0].tick == 1);
    assert(!snaps[0].ecsState.empty());

    std::cout << "[PASS] test_snapshot_saves_ecs_state" << std::endl;
}

void test_rollback_restores_ecs_state() {
    World world;
    world.RegisterComponent<SnapPosition>(1);

    NetContext net;
    net.Init(NetMode::Server);
    net.SetWorld(&world);

    EntityID e = world.CreateEntity();
    world.AddComponent<SnapPosition>(e, {10.0f, 20.0f});

    // Save snapshot at tick 1
    net.SaveSnapshot(1);

    // Modify the world
    auto* pos = world.GetComponent<SnapPosition>(e);
    pos->x = 99.0f;
    pos->y = 99.0f;
    assert(world.GetComponent<SnapPosition>(e)->x == 99.0f);

    // Rollback to tick 1
    net.RollbackTo(1);

    // Verify state is restored
    auto* restored = world.GetComponent<SnapPosition>(e);
    assert(restored != nullptr);
    assert(restored->x == 10.0f);
    assert(restored->y == 20.0f);

    std::cout << "[PASS] test_rollback_restores_ecs_state" << std::endl;
}

void test_rollback_removes_future_snapshots() {
    World world;
    world.RegisterComponent<SnapPosition>(1);

    NetContext net;
    net.Init(NetMode::Server);
    net.SetWorld(&world);

    world.CreateEntity();

    net.SaveSnapshot(1);
    net.SaveSnapshot(2);
    net.SaveSnapshot(3);

    assert(net.Snapshots().size() == 3);

    net.RollbackTo(1);

    // Snapshots for tick 2 and 3 should be removed
    assert(net.Snapshots().size() == 1);
    assert(net.Snapshots()[0].tick == 1);

    std::cout << "[PASS] test_rollback_removes_future_snapshots" << std::endl;
}

void test_snapshot_without_world() {
    NetContext net;
    net.Init(NetMode::Server);
    // No world set

    net.SaveSnapshot(1);
    auto& snaps = net.Snapshots();
    assert(snaps.size() == 1);
    assert(snaps[0].ecsState.empty());

    std::cout << "[PASS] test_snapshot_without_world" << std::endl;
}

void test_rollback_with_multiple_entities() {
    World world;
    world.RegisterComponent<SnapPosition>(1);
    world.RegisterComponent<SnapHealth>(2);

    NetContext net;
    net.Init(NetMode::Server);
    net.SetWorld(&world);

    EntityID e1 = world.CreateEntity();
    EntityID e2 = world.CreateEntity();
    world.AddComponent<SnapPosition>(e1, {1.0f, 2.0f});
    world.AddComponent<SnapHealth>(e1, {100});
    world.AddComponent<SnapPosition>(e2, {3.0f, 4.0f});

    net.SaveSnapshot(5);

    // Destroy e2 and modify e1
    world.DestroyEntity(e2);
    auto* hp = world.GetComponent<SnapHealth>(e1);
    hp->hp = 50;

    assert(world.EntityCount() == 1);

    // Rollback
    net.RollbackTo(5);

    assert(world.EntityCount() == 2);
    assert(world.IsAlive(e1));
    assert(world.IsAlive(e2));

    auto* restoredHP = world.GetComponent<SnapHealth>(e1);
    assert(restoredHP != nullptr);
    assert(restoredHP->hp == 100);

    auto* restoredPos2 = world.GetComponent<SnapPosition>(e2);
    assert(restoredPos2 != nullptr);
    assert(restoredPos2->x == 3.0f);

    std::cout << "[PASS] test_rollback_with_multiple_entities" << std::endl;
}

void test_record_and_replay_input() {
    World world;
    int tickCount = 0;
    world.SetTickCallback([&](float) { tickCount++; });

    NetContext net;
    net.Init(NetMode::Server);
    net.SetWorld(&world);

    InputFrame f1{1, 1, 1.0f, 0.0f};
    InputFrame f2{2, 1, 0.0f, 1.0f};
    InputFrame f3{3, 1, -1.0f, 0.0f};
    net.RecordInput(f1);
    net.RecordInput(f2);
    net.RecordInput(f3);

    assert(net.RecordedInputs().size() == 3);

    // Replay from tick 2 onward (should apply 2 frames)
    net.ReplayFrom(2);
    assert(tickCount == 2);

    std::cout << "[PASS] test_record_and_replay_input" << std::endl;
}

void test_replay_applies_input_frames() {
    World world;
    world.RegisterComponent<SnapPosition>(1);

    EntityID e = world.CreateEntity();
    world.AddComponent<SnapPosition>(e, {0.0f, 0.0f});

    NetContext net;
    net.Init(NetMode::Server);
    net.SetWorld(&world);

    // Set up input apply callback that moves position by moveX/moveY
    net.SetInputApplyCallback([&](const InputFrame& frame) {
        auto* pos = world.GetComponent<SnapPosition>(e);
        if (pos) {
            pos->x += frame.moveX;
            pos->y += frame.moveY;
        }
    });

    InputFrame f1{1, 1, 5.0f, 0.0f};
    InputFrame f2{2, 1, 0.0f, 3.0f};
    InputFrame f3{3, 1, -2.0f, 1.0f};
    net.RecordInput(f1);
    net.RecordInput(f2);
    net.RecordInput(f3);

    // Replay all from tick 1
    net.ReplayFrom(1);

    auto* pos = world.GetComponent<SnapPosition>(e);
    assert(pos != nullptr);
    assert(pos->x == 3.0f);  // 5 + 0 + (-2) = 3
    assert(pos->y == 4.0f);  // 0 + 3 + 1 = 4

    std::cout << "[PASS] test_replay_applies_input_frames" << std::endl;
}
