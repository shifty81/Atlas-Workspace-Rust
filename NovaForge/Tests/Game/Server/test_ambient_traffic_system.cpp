// Tests for: AmbientTrafficSystem
#include "test_log.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/ambient_traffic_system.h"

using namespace atlas;

static void testAmbientTrafficDefaults() {
    std::cout << "\n=== AmbientTraffic: Defaults ===" << std::endl;
    ecs::World world;

    auto* sys = world.createEntity("traffic_sys");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);

    assertTrue(traffic->spawn_timer == 60.0f, "Default spawn timer is 60s");
    assertTrue(traffic->active_traffic_count == 0, "No active traffic initially");
    assertTrue(traffic->pending_spawns.empty(), "No pending spawns initially");
}

static void testAmbientTrafficSpawnOnTimer() {
    std::cout << "\n=== AmbientTraffic: Spawns On Timer ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 5.0f;

    auto* sys = world.createEntity("traffic_eco");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 1.0f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;
    state->resource_availability = 0.7f;
    state->pirate_activity = 0.5f;
    state->security_level = 0.6f;
    state->trade_volume = 0.6f;

    atSys.update(2.0f);
    auto spawns = atSys.getPendingSpawns("traffic_eco");
    assertTrue(!spawns.empty(), "Spawns generated after timer fires");
    assertTrue(traffic->active_traffic_count > 0, "Active traffic count increased");
}

static void testAmbientTrafficTraderSpawn() {
    std::cout << "\n=== AmbientTraffic: Trader Spawn ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_trader");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;
    state->resource_availability = 0.0f;
    state->pirate_activity = 0.0f;
    state->security_level = 0.0f;
    state->trade_volume = 0.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_trader");
    bool has_trader = false;
    for (auto& s : spawns) if (s == "trader") has_trader = true;
    assertTrue(has_trader, "Trader spawned in good economy");
    assertTrue(traffic->active_traffic_count >= 1, "Active count >= 1 for trader");
}

static void testAmbientTrafficMinerSpawn() {
    std::cout << "\n=== AmbientTraffic: Miner Spawn ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_miner");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.0f;
    state->resource_availability = 0.9f;
    state->pirate_activity = 0.0f;
    state->security_level = 0.0f;
    state->trade_volume = 0.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_miner");
    bool has_miner = false;
    for (auto& s : spawns) if (s == "miner") has_miner = true;
    assertTrue(has_miner, "Miner spawned in resource-rich system");
}

static void testAmbientTrafficPirateSpawn() {
    std::cout << "\n=== AmbientTraffic: Pirate Spawn ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_pirate");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.0f;
    state->resource_availability = 0.0f;
    state->pirate_activity = 0.8f;
    state->security_level = 0.0f;
    state->trade_volume = 0.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_pirate");
    bool has_pirate = false;
    for (auto& s : spawns) if (s == "pirate") has_pirate = true;
    assertTrue(has_pirate, "Pirate spawned in high pirate activity");
}

static void testAmbientTrafficPatrolSpawn() {
    std::cout << "\n=== AmbientTraffic: Patrol Spawn ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_patrol");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.0f;
    state->resource_availability = 0.0f;
    state->pirate_activity = 0.0f;
    state->security_level = 0.8f;
    state->trade_volume = 0.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_patrol");
    bool has_patrol = false;
    for (auto& s : spawns) if (s == "patrol") has_patrol = true;
    assertTrue(has_patrol, "Patrol spawned in high-security system");
}

static void testAmbientTrafficHaulerSpawn() {
    std::cout << "\n=== AmbientTraffic: Hauler Spawn ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_hauler");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.0f;
    state->resource_availability = 0.0f;
    state->pirate_activity = 0.0f;
    state->security_level = 0.0f;
    state->trade_volume = 0.7f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_hauler");
    bool has_hauler = false;
    for (auto& s : spawns) if (s == "hauler") has_hauler = true;
    assertTrue(has_hauler, "Hauler spawned on high trade volume");
}

static void testAmbientTrafficNoSpawnBelowThreshold() {
    std::cout << "\n=== AmbientTraffic: No Spawn Below Threshold ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_low");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    // All below thresholds
    state->economic_index = 0.1f;
    state->resource_availability = 0.1f;
    state->pirate_activity = 0.1f;
    state->security_level = 0.1f;
    state->trade_volume = 0.1f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_low");
    assertTrue(spawns.empty(), "No spawns when all values below thresholds");
    assertTrue(traffic->active_traffic_count == 0, "Zero active traffic");
}

static void testAmbientTrafficCapReached() {
    std::cout << "\n=== AmbientTraffic: Cap Reached ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;
    atSys.max_traffic_per_system = 5;

    auto* sys = world.createEntity("traffic_full");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    traffic->active_traffic_count = 5;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 1.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_full");
    assertTrue(spawns.empty(), "No spawns when at traffic cap");
    assertTrue(traffic->active_traffic_count == 5, "Traffic count unchanged at cap");
}

static void testAmbientTrafficClearPending() {
    std::cout << "\n=== AmbientTraffic: Clear Pending Spawns ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_clear");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;

    atSys.update(1.0f);
    assertTrue(!traffic->pending_spawns.empty(), "Pending spawns exist before clear");

    atSys.clearPendingSpawns("traffic_clear");
    assertTrue(traffic->pending_spawns.empty(), "Pending spawns cleared");
    auto spawns = atSys.getPendingSpawns("traffic_clear");
    assertTrue(spawns.empty(), "getPendingSpawns empty after clear");
}

static void testAmbientTrafficTimerReset() {
    std::cout << "\n=== AmbientTraffic: Timer Resets After Spawn ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 5.0f;

    auto* sys = world.createEntity("traffic_timer");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 1.0f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;

    atSys.update(2.0f);  // timer fires, resets to spawn_interval - leftover
    assertTrue(traffic->spawn_timer > 0.0f, "Timer reset after spawn evaluation");
}

static void testAmbientTrafficNoStateComponent() {
    std::cout << "\n=== AmbientTraffic: No SimStarSystemState ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_nostate");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    // No SimStarSystemState attached

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_nostate");
    assertTrue(spawns.empty(), "No spawns when SimStarSystemState absent");
    assertTrue(traffic->active_traffic_count == 0, "Zero active when no state");
}

static void testAmbientTrafficMissingEntity() {
    std::cout << "\n=== AmbientTraffic: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);

    auto spawns = atSys.getPendingSpawns("nobody");
    assertTrue(spawns.empty(), "No spawns for missing entity");
    assertTrue(atSys.getActiveTrafficCount("nobody") == 0, "Zero traffic for missing entity");
    // clearPendingSpawns on missing entity should not crash
    atSys.clearPendingSpawns("nobody");
    assertTrue(true, "clearPendingSpawns on missing entity does not crash");
}

void run_ambient_traffic_system_tests() {
    testAmbientTrafficDefaults();
    testAmbientTrafficSpawnOnTimer();
    testAmbientTrafficTraderSpawn();
    testAmbientTrafficMinerSpawn();
    testAmbientTrafficPirateSpawn();
    testAmbientTrafficPatrolSpawn();
    testAmbientTrafficHaulerSpawn();
    testAmbientTrafficNoSpawnBelowThreshold();
    testAmbientTrafficCapReached();
    testAmbientTrafficClearPending();
    testAmbientTrafficTimerReset();
    testAmbientTrafficNoStateComponent();
    testAmbientTrafficMissingEntity();
}
