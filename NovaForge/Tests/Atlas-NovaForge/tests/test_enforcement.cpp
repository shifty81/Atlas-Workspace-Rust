#include "../engine/core/Engine.h"
#include "../engine/sim/TickScheduler.h"
#include "../engine/net/NetContext.h"
#include "../engine/assets/AssetValidator.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <filesystem>

using namespace atlas;

// ============================================================
// TickScheduler: Immutable tick rate after lock
// ============================================================

void test_tick_rate_lock() {
    sim::TickScheduler scheduler;
    scheduler.SetTickRate(60);
    assert(scheduler.TickRate() == 60);
    assert(!scheduler.IsTickRateLocked());

    scheduler.LockTickRate();
    assert(scheduler.IsTickRateLocked());

    // Attempt to change after lock should be ignored
    scheduler.SetTickRate(120);
    assert(scheduler.TickRate() == 60);

    std::cout << "[PASS] test_tick_rate_lock" << std::endl;
}

void test_tick_rate_set_before_lock() {
    sim::TickScheduler scheduler;
    scheduler.SetTickRate(30);
    assert(scheduler.TickRate() == 30);

    scheduler.SetTickRate(60);
    assert(scheduler.TickRate() == 60);

    scheduler.LockTickRate();
    scheduler.SetTickRate(90);
    assert(scheduler.TickRate() == 60);

    std::cout << "[PASS] test_tick_rate_set_before_lock" << std::endl;
}

void test_engine_locks_tick_rate_on_run() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    // Before Run, tick rate should not be locked
    assert(!engine.GetScheduler().IsTickRateLocked());

    engine.Run();

    // After Run, tick rate should be locked
    assert(engine.GetScheduler().IsTickRateLocked());
    assert(engine.GetScheduler().TickRate() == 60);

    // Attempt to change should be ignored
    engine.GetScheduler().SetTickRate(120);
    assert(engine.GetScheduler().TickRate() == 60);

    std::cout << "[PASS] test_engine_locks_tick_rate_on_run" << std::endl;
}

// ============================================================
// Engine: System execution order tracking
// ============================================================

void test_engine_system_order() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 0;

    Engine engine(cfg);
    assert(engine.SystemExecutionOrder().empty());

    engine.InitCore();
    assert(engine.SystemExecutionOrder().size() == 1);
    assert(engine.SystemExecutionOrder()[0] == "Core");

    engine.InitECS();
    assert(engine.SystemExecutionOrder().size() == 2);
    assert(engine.SystemExecutionOrder()[1] == "ECS");

    engine.InitNetworking();
    assert(engine.SystemExecutionOrder().size() == 3);
    assert(engine.SystemExecutionOrder()[2] == "Networking");

    std::cout << "[PASS] test_engine_system_order" << std::endl;
}

void test_engine_system_order_deterministic() {
    // Run init twice and verify order is identical
    std::vector<std::string> order1, order2;

    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();
        order1 = engine.SystemExecutionOrder();
    }

    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        Engine engine(cfg);
        engine.InitCore();
        engine.InitECS();
        engine.InitNetworking();
        order2 = engine.SystemExecutionOrder();
    }

    assert(order1.size() == order2.size());
    for (size_t i = 0; i < order1.size(); ++i) {
        assert(order1[i] == order2[i]);
    }

    std::cout << "[PASS] test_engine_system_order_deterministic" << std::endl;
}

// ============================================================
// NetContext: Packet schema version
// ============================================================

void test_net_packet_schema_version() {
    assert(net::NetContext::PacketSchemaVersion() == net::NET_PACKET_SCHEMA_VERSION);
    assert(net::NET_PACKET_SCHEMA_VERSION >= 1);

    std::cout << "[PASS] test_net_packet_schema_version" << std::endl;
}

// ============================================================
// AssetValidator: Asset immutability enforcement
// ============================================================

void test_asset_lock_and_check() {
    asset::AssetValidator validator;

    assert(!validator.IsAssetLocked("texture_stone"));
    assert(validator.LockedAssets().empty());

    bool locked = validator.LockAsset("texture_stone");
    assert(locked);
    assert(validator.IsAssetLocked("texture_stone"));
    assert(validator.LockedAssets().size() == 1);

    // Locking again should fail
    bool lockedAgain = validator.LockAsset("texture_stone");
    assert(!lockedAgain);
    assert(validator.LockedAssets().size() == 1);

    std::cout << "[PASS] test_asset_lock_and_check" << std::endl;
}

void test_asset_lock_multiple() {
    asset::AssetValidator validator;

    validator.LockAsset("mesh_wall");
    validator.LockAsset("texture_brick");
    validator.LockAsset("material_stone");

    auto locked = validator.LockedAssets();
    assert(locked.size() == 3);
    assert(validator.IsAssetLocked("mesh_wall"));
    assert(validator.IsAssetLocked("texture_brick"));
    assert(validator.IsAssetLocked("material_stone"));
    assert(!validator.IsAssetLocked("mesh_floor"));

    std::cout << "[PASS] test_asset_lock_multiple" << std::endl;
}

// ============================================================
// Determinism config file exists
// ============================================================

void test_determinism_config_exists() {
    auto path = std::filesystem::path(CMAKE_SOURCE_DIR) / "engine" / "core" / "contract" / "determinism.json";
    assert(std::filesystem::exists(path));

    std::ifstream f(path);
    assert(f.is_open());
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assert(content.find("\"version\"") != std::string::npos);
    assert(content.find("\"simulation\"") != std::string::npos);
    assert(content.find("\"replay\"") != std::string::npos);
    assert(content.find("\"networking\"") != std::string::npos);

    std::cout << "[PASS] test_determinism_config_exists" << std::endl;
}
