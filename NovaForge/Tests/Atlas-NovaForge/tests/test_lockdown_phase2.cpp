#include "../engine/core/Engine.h"
#include "../engine/assets/AssetValidator.h"
#include "../engine/ui/UILayoutSolver.h"
#include "../engine/sim/WorldState.h"
#include <iostream>
#include <cassert>

using namespace atlas;

// ============================================================
// Task 1: Rollback/resimulation hash verification
// ============================================================

void test_rollback_and_verify_success() {
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

    // Verify we ran 10 ticks and have snapshots
    assert(engine.GetTimeModel().Context().sim.tick == 10);
    assert(engine.GetWorldState().SnapshotAtTick(3) != nullptr);
    assert(engine.GetWorldState().SnapshotAtTick(8) != nullptr);

    // Rollback to tick 3, resimulate to tick 8, verify hash matches
    bool verified = engine.RollbackAndVerify(3, 8);
    assert(verified);

    std::cout << "[PASS] test_rollback_and_verify_success" << std::endl;
}

void test_rollback_and_verify_bad_ticks() {
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

    // snapshotTick >= targetTick should fail
    assert(!engine.RollbackAndVerify(5, 5));
    assert(!engine.RollbackAndVerify(5, 3));

    // Non-existent snapshot tick should fail
    assert(!engine.RollbackAndVerify(1, 9999));

    std::cout << "[PASS] test_rollback_and_verify_bad_ticks" << std::endl;
}

// ============================================================
// Task 2: Asset schema version locking
// ============================================================

void test_asset_schema_lock() {
    asset::AssetValidator validator;

    assert(!validator.IsSchemaLocked());
    assert(validator.LockedSchemaVersion() == 0);

    bool locked = validator.LockSchema(3);
    assert(locked);
    assert(validator.IsSchemaLocked());
    assert(validator.LockedSchemaVersion() == 3);

    // Locking again should fail
    bool lockedAgain = validator.LockSchema(4);
    assert(!lockedAgain);
    assert(validator.LockedSchemaVersion() == 3);

    std::cout << "[PASS] test_asset_schema_lock" << std::endl;
}

void test_asset_schema_lock_default() {
    asset::AssetValidator validator;

    assert(!validator.IsSchemaLocked());
    assert(validator.LockedSchemaVersion() == 0);

    std::cout << "[PASS] test_asset_schema_lock_default" << std::endl;
}

// ============================================================
// Task 3: Cross-platform layout hash
// ============================================================

void test_layout_hash_deterministic() {
    ui::UILayoutSolver solver;
    solver.AddEntry(1, {0, 0, 100, 50, INT32_MAX, INT32_MAX, 1.0f});
    solver.AddEntry(2, {0, 0, 200, 50, INT32_MAX, INT32_MAX, 1.0f});

    ui::UILayoutRect bounds{0, 0, 400, 100};
    solver.Solve(bounds, ui::LayoutDirection::Horizontal);

    uint64_t hash1 = solver.LayoutHash();
    assert(hash1 != 0);

    // Solve again with same inputs should produce same hash
    ui::UILayoutSolver solver2;
    solver2.AddEntry(1, {0, 0, 100, 50, INT32_MAX, INT32_MAX, 1.0f});
    solver2.AddEntry(2, {0, 0, 200, 50, INT32_MAX, INT32_MAX, 1.0f});
    solver2.Solve(bounds, ui::LayoutDirection::Horizontal);

    uint64_t hash2 = solver2.LayoutHash();
    assert(hash1 == hash2);

    std::cout << "[PASS] test_layout_hash_deterministic" << std::endl;
}

void test_layout_hash_differs_on_change() {
    ui::UILayoutRect bounds{0, 0, 400, 100};

    ui::UILayoutSolver solver1;
    solver1.AddEntry(1, {100, 50, 100, 50, INT32_MAX, INT32_MAX, 1.0f});
    solver1.AddEntry(2, {100, 50, 100, 50, INT32_MAX, INT32_MAX, 1.0f});
    solver1.Solve(bounds, ui::LayoutDirection::Horizontal);

    ui::UILayoutSolver solver2;
    solver2.AddEntry(1, {200, 50, 200, 50, INT32_MAX, INT32_MAX, 1.0f});
    solver2.AddEntry(2, {100, 50, 100, 50, INT32_MAX, INT32_MAX, 1.0f});
    solver2.Solve(bounds, ui::LayoutDirection::Horizontal);

    assert(solver1.LayoutHash() != solver2.LayoutHash());

    std::cout << "[PASS] test_layout_hash_differs_on_change" << std::endl;
}

void test_layout_hash_empty() {
    ui::UILayoutSolver solver;
    assert(solver.LayoutHash() == 0);

    std::cout << "[PASS] test_layout_hash_empty" << std::endl;
}

// ============================================================
// Task 4: System mutation ownership
// ============================================================

void test_mutation_ownership_register() {
    sim::WorldState ws;

    ws.RegisterOwnership("Physics", "Transform");
    ws.RegisterOwnership("Physics", "Velocity");

    assert(ws.OwnsComponent("Physics", "Transform"));
    assert(ws.OwnsComponent("Physics", "Velocity"));
    assert(!ws.OwnsComponent("AI", "Transform"));

    std::cout << "[PASS] test_mutation_ownership_register" << std::endl;
}

void test_mutation_ownership_exclusive() {
    sim::WorldState ws;

    ws.RegisterOwnership("Physics", "Transform");
    // Attempt by another system to claim same component is ignored
    ws.RegisterOwnership("AI", "Transform");

    assert(ws.OwnsComponent("Physics", "Transform"));
    assert(!ws.OwnsComponent("AI", "Transform"));
    assert(ws.OwnerOf("Transform") == "Physics");

    std::cout << "[PASS] test_mutation_ownership_exclusive" << std::endl;
}

void test_mutation_can_mutate() {
    sim::WorldState ws;

    ws.RegisterOwnership("Physics", "Transform");
    ws.RegisterOwnership("AI", "Memory");

    assert(ws.CanMutate("Physics", "Transform"));
    assert(!ws.CanMutate("AI", "Transform"));
    assert(ws.CanMutate("AI", "Memory"));
    assert(!ws.CanMutate("Physics", "Memory"));

    // Unowned component can be mutated by anyone
    assert(ws.CanMutate("Physics", "Unregistered"));
    assert(ws.CanMutate("AI", "Unregistered"));

    std::cout << "[PASS] test_mutation_can_mutate" << std::endl;
}

void test_mutation_owned_components() {
    sim::WorldState ws;

    ws.RegisterOwnership("Physics", "Transform");
    ws.RegisterOwnership("Physics", "Velocity");
    ws.RegisterOwnership("AI", "Memory");

    auto physicsComps = ws.OwnedComponents("Physics");
    assert(physicsComps.size() == 2);

    auto aiComps = ws.OwnedComponents("AI");
    assert(aiComps.size() == 1);
    assert(aiComps[0] == "Memory");

    auto emptyComps = ws.OwnedComponents("Render");
    assert(emptyComps.empty());

    std::cout << "[PASS] test_mutation_owned_components" << std::endl;
}

void test_mutation_owner_of_unregistered() {
    sim::WorldState ws;

    assert(ws.OwnerOf("Transform").empty());
    assert(ws.CanMutate("Anyone", "Transform"));

    std::cout << "[PASS] test_mutation_owner_of_unregistered" << std::endl;
}
