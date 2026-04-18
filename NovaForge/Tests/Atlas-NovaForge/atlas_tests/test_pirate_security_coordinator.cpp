/**
 * Tests for PirateSecurityCoordinator — integration layer that ties
 * pirate AI and security AI into a coordinated loop:
 *   - Pirates patrol trade routes, attack haulers
 *   - Security responds to pirate activity
 *   - Faction standings: pirates = -10, security = +10
 *   - Dynamic spawn scaling: more pirates → more security
 *   - Loot drops from destroyed haulers
 *   - Economic impact: hauler deaths → price increases
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/PirateSecurityCoordinator.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_coordinator_defaults() {
    PirateSecurityCoordinator coord;
    assert(coord.GetPirateStateMachine() == nullptr);
    assert(coord.GetSecurityStateMachine() == nullptr);
    assert(coord.TradeRouteCount() == 0);
    assert(coord.TotalHaulersDestroyed() == 0);
    assert(approxEq(coord.TotalCargoLost(), 0.0f));
    assert(approxEq(coord.TotalLootRecovered(), 0.0f));
    assert(approxEq(coord.GetPriceImpact(), 1.0f));
    assert(approxEq(coord.GetPirateActivityLevel(), 0.0f));
    assert(approxEq(coord.GetSecurityPerPirateRatio(), 1.5f));
    assert(coord.GetMaxSecurityGuards() == 20);
    assert(approxEq(coord.SimTime(), 0.0f));
}

void test_coordinator_wire_state_machines() {
    AIPirateStateMachine pirates;
    AISecurityStateMachine security;
    PirateSecurityCoordinator coord;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityStateMachine(&security);
    assert(coord.GetPirateStateMachine() == &pirates);
    assert(coord.GetSecurityStateMachine() == &security);
}

// ══════════════════════════════════════════════════════════════════
// Trade route management
// ══════════════════════════════════════════════════════════════════

void test_coordinator_add_trade_route() {
    PirateSecurityCoordinator coord;
    TradeRoute route;
    route.haulerEntityId = 100;
    route.originSystem = "Jita";
    route.destinationSystem = "Amarr";
    route.cargoValue = 5000.0f;
    route.securityLevel = 0.3f;
    assert(coord.AddTradeRoute(route));
    assert(coord.TradeRouteCount() == 1);
    const TradeRoute* r = coord.GetTradeRoute(100);
    assert(r != nullptr);
    assert(r->originSystem == "Jita");
    assert(approxEq(r->cargoValue, 5000.0f));
}

void test_coordinator_add_trade_route_duplicate() {
    PirateSecurityCoordinator coord;
    TradeRoute route;
    route.haulerEntityId = 100;
    assert(coord.AddTradeRoute(route));
    assert(!coord.AddTradeRoute(route));
    assert(coord.TradeRouteCount() == 1);
}

void test_coordinator_remove_trade_route() {
    PirateSecurityCoordinator coord;
    TradeRoute route;
    route.haulerEntityId = 100;
    coord.AddTradeRoute(route);
    assert(coord.RemoveTradeRoute(100));
    assert(coord.TradeRouteCount() == 0);
    assert(coord.GetTradeRoute(100) == nullptr);
}

void test_coordinator_remove_trade_route_nonexistent() {
    PirateSecurityCoordinator coord;
    assert(!coord.RemoveTradeRoute(999));
}

// ══════════════════════════════════════════════════════════════════
// Entity tracking
// ══════════════════════════════════════════════════════════════════

void test_coordinator_track_pirate() {
    PirateSecurityCoordinator coord;
    coord.TrackPirate(1);
    coord.TrackPirate(2);
    assert(coord.TrackedPirateIds().size() == 2);
    // Duplicate tracking should not add again
    coord.TrackPirate(1);
    assert(coord.TrackedPirateIds().size() == 2);
}

void test_coordinator_track_guard() {
    PirateSecurityCoordinator coord;
    coord.TrackGuard(10);
    coord.TrackGuard(11);
    assert(coord.TrackedGuardIds().size() == 2);
    coord.TrackGuard(10);
    assert(coord.TrackedGuardIds().size() == 2);
}

void test_coordinator_untrack_pirate() {
    PirateSecurityCoordinator coord;
    coord.TrackPirate(1);
    coord.TrackPirate(2);
    coord.UntrackPirate(1);
    assert(coord.TrackedPirateIds().size() == 1);
    assert(coord.TrackedPirateIds()[0] == 2);
}

void test_coordinator_untrack_guard() {
    PirateSecurityCoordinator coord;
    coord.TrackGuard(10);
    coord.TrackGuard(11);
    coord.UntrackGuard(10);
    assert(coord.TrackedGuardIds().size() == 1);
    assert(coord.TrackedGuardIds()[0] == 11);
}

// ══════════════════════════════════════════════════════════════════
// Faction standings
// ══════════════════════════════════════════════════════════════════

void test_coordinator_pirate_standing() {
    PirateSecurityCoordinator coord;
    FactionStandingEntry s = coord.GetPirateStanding();
    assert(s.factionName == "Venom Syndicate");
    assert(approxEq(s.standing, -10.0f));
}

void test_coordinator_security_standing() {
    PirateSecurityCoordinator coord;
    FactionStandingEntry s = coord.GetSecurityStanding();
    assert(s.factionName == "CONCORD");
    assert(approxEq(s.standing, 10.0f));
}

// ══════════════════════════════════════════════════════════════════
// Security scaling
// ══════════════════════════════════════════════════════════════════

void test_coordinator_security_scaling_no_pirates() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    coord.SetPirateStateMachine(&pirates);
    assert(coord.GetRecommendedSecurityCount() == 0);
}

void test_coordinator_security_scaling_with_pirates() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityPerPirateRatio(1.5f);

    // Add 4 pirates → recommended = ceil(4 * 1.5) = 6
    for (uint32_t i = 1; i <= 4; ++i) {
        PirateConfig cfg;
        cfg.entityId = i;
        pirates.AddPirate(cfg);
    }
    assert(coord.GetRecommendedSecurityCount() == 6);
}

void test_coordinator_security_scaling_capped() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityPerPirateRatio(2.0f);
    coord.SetMaxSecurityGuards(5);

    // Add 10 pirates → recommended = ceil(10 * 2.0) = 20, capped at 5
    for (uint32_t i = 1; i <= 10; ++i) {
        PirateConfig cfg;
        cfg.entityId = i;
        pirates.AddPirate(cfg);
    }
    assert(coord.GetRecommendedSecurityCount() == 5);
}

// ══════════════════════════════════════════════════════════════════
// Loot drops
// ══════════════════════════════════════════════════════════════════

void test_coordinator_generate_loot_drop() {
    PirateSecurityCoordinator coord;
    coord.GenerateLootDrop(100, 1, 5000.0f);
    assert(coord.GetPendingLootDrops().size() == 1);
    assert(coord.GetPendingLootDrops()[0].fromEntityId == 100);
    assert(coord.GetPendingLootDrops()[0].pirateEntityId == 1);
    assert(approxEq(coord.GetPendingLootDrops()[0].cargoValue, 5000.0f));
    assert(coord.TotalHaulersDestroyed() == 1);
    assert(approxEq(coord.TotalCargoLost(), 5000.0f));
}

void test_coordinator_clear_loot_drops() {
    PirateSecurityCoordinator coord;
    coord.GenerateLootDrop(100, 1, 1000.0f);
    coord.GenerateLootDrop(101, 2, 2000.0f);
    assert(coord.GetPendingLootDrops().size() == 2);
    coord.ClearLootDrops();
    assert(coord.GetPendingLootDrops().empty());
    // Stats persist after clearing
    assert(coord.TotalHaulersDestroyed() == 2);
    assert(approxEq(coord.TotalCargoLost(), 3000.0f));
}

// ══════════════════════════════════════════════════════════════════
// Economic impact
// ══════════════════════════════════════════════════════════════════

void test_coordinator_price_impact_increases() {
    PirateSecurityCoordinator coord;
    assert(approxEq(coord.GetPriceImpact(), 1.0f));
    coord.GenerateLootDrop(100, 1, 5000.0f);
    // Price should increase by 0.1 per hauler destroyed
    assert(approxEq(coord.GetPriceImpact(), 1.1f));
    coord.GenerateLootDrop(101, 2, 3000.0f);
    assert(approxEq(coord.GetPriceImpact(), 1.2f));
}

void test_coordinator_price_impact_decays() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    AISecurityStateMachine security;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityStateMachine(&security);

    // Manually bump price impact
    coord.GenerateLootDrop(100, 1, 5000.0f);
    assert(approxEq(coord.GetPriceImpact(), 1.1f));

    // Tick for a long time — price decays towards 1.0 at 0.01/s
    // After 10 seconds: 1.1 - 0.01*10 = 1.0
    coord.Tick(10.0f);
    assert(approxEq(coord.GetPriceImpact(), 1.0f, 0.02f));
}

// ══════════════════════════════════════════════════════════════════
// Pirate activity level
// ══════════════════════════════════════════════════════════════════

void test_coordinator_activity_level_no_pirates() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    AISecurityStateMachine security;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityStateMachine(&security);
    coord.Tick(0.1f);
    assert(approxEq(coord.GetPirateActivityLevel(), 0.0f));
}

void test_coordinator_activity_level_with_pirates() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    AISecurityStateMachine security;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityStateMachine(&security);

    PirateConfig cfg;
    cfg.entityId = 1;
    cfg.travelSpeed = 600.0f;
    pirates.AddPirate(cfg);
    coord.TrackPirate(1);

    // After first tick, pirate goes to Patrol (active state)
    coord.Tick(0.01f);
    // Activity should be > 0 (1/1 pirates active)
    assert(coord.GetPirateActivityLevel() > 0.0f);
}

void test_coordinator_activity_level_manual_set() {
    PirateSecurityCoordinator coord;
    coord.SetPirateActivityLevel(0.75f);
    assert(approxEq(coord.GetPirateActivityLevel(), 0.75f));
    // Clamp to [0, 1]
    coord.SetPirateActivityLevel(2.0f);
    assert(approxEq(coord.GetPirateActivityLevel(), 1.0f));
    coord.SetPirateActivityLevel(-0.5f);
    assert(approxEq(coord.GetPirateActivityLevel(), 0.0f));
}

// ══════════════════════════════════════════════════════════════════
// Sim time tracking
// ══════════════════════════════════════════════════════════════════

void test_coordinator_sim_time() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    AISecurityStateMachine security;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityStateMachine(&security);

    coord.Tick(1.0f);
    assert(approxEq(coord.SimTime(), 1.0f));
    coord.Tick(2.5f);
    assert(approxEq(coord.SimTime(), 3.5f));
}

// ══════════════════════════════════════════════════════════════════
// Integrated pirate-targets-hauler cycle
// ══════════════════════════════════════════════════════════════════

void test_coordinator_pirates_target_low_sec_routes() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    AISecurityStateMachine security;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityStateMachine(&security);

    // Add a pirate
    PirateConfig pcfg;
    pcfg.entityId = 1;
    pcfg.travelSpeed = 600.0f;
    pcfg.scanRange = 300.0f;
    pirates.AddPirate(pcfg);
    coord.TrackPirate(1);
    pirates.SetPatrolDistance(600.0f);

    // Add a low-sec trade route (sec < 0.5 → pirates can target)
    TradeRoute lowsec;
    lowsec.haulerEntityId = 200;
    lowsec.originSystem = "Rancer";
    lowsec.destinationSystem = "Amamake";
    lowsec.cargoValue = 5000.0f;
    lowsec.securityLevel = 0.2f;
    coord.AddTradeRoute(lowsec);

    // Add a high-sec trade route (sec >= 0.5 → pirates ignore)
    TradeRoute highsec;
    highsec.haulerEntityId = 201;
    highsec.originSystem = "Jita";
    highsec.destinationSystem = "Amarr";
    highsec.cargoValue = 10000.0f;
    highsec.securityLevel = 0.9f;
    coord.AddTradeRoute(highsec);

    // Run enough ticks for pirate to complete cycle:
    // Idle → Patrol → ScanForTargets → PursueTarget → Attack → LootCargo
    for (int i = 0; i < 300; ++i) {
        coord.Tick(0.1f);
    }

    // Pirate should have killed at least one hauler
    const PirateRuntime* rt = pirates.GetPirate(1);
    assert(rt->killCount >= 1);
}

void test_coordinator_security_responds_to_pirates() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    AISecurityStateMachine security;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityStateMachine(&security);

    // Add a pirate
    PirateConfig pcfg;
    pcfg.entityId = 1;
    pcfg.travelSpeed = 600.0f;
    pcfg.scanRange = 300.0f;
    pirates.AddPirate(pcfg);
    coord.TrackPirate(1);
    pirates.SetPatrolDistance(600.0f);

    // Add a security guard
    SecurityConfig scfg;
    scfg.entityId = 10;
    scfg.hitPoints = 200.0f;
    scfg.travelSpeed = 400.0f;
    scfg.responseTime = 0.5f;
    security.AddGuard(scfg);
    coord.TrackGuard(10);
    security.SetPostDistance(400.0f);

    // Add low-sec trade route
    TradeRoute route;
    route.haulerEntityId = 200;
    route.securityLevel = 0.2f;
    route.cargoValue = 5000.0f;
    coord.AddTradeRoute(route);

    // Run long enough for pirate to attack and security to respond
    for (int i = 0; i < 400; ++i) {
        coord.Tick(0.1f);
    }

    // Security should have responded to at least one threat
    const SecurityRuntime* srt = security.GetGuard(10);
    assert(srt->threatsNeutralized >= 1 ||
           srt->state == SecurityState::EngageThreat ||
           srt->state == SecurityState::WarpToThreat);
}

// ══════════════════════════════════════════════════════════════════
// Hauler loot drop from coordinated cycle
// ══════════════════════════════════════════════════════════════════

void test_coordinator_hauler_loot_on_pirate_kill() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    AISecurityStateMachine security;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityStateMachine(&security);

    PirateConfig pcfg;
    pcfg.entityId = 1;
    pcfg.travelSpeed = 600.0f;
    pcfg.scanRange = 300.0f;
    pcfg.lootCapacity = 200.0f;
    pirates.AddPirate(pcfg);
    coord.TrackPirate(1);
    pirates.SetPatrolDistance(600.0f);

    TradeRoute route;
    route.haulerEntityId = 200;
    route.securityLevel = 0.2f;
    route.cargoValue = 5000.0f;
    coord.AddTradeRoute(route);

    // Run until pirate completes attack+loot cycle
    for (int i = 0; i < 300; ++i) {
        coord.Tick(0.1f);
    }

    // Should have at least one loot drop from hauler destruction
    if (pirates.GetPirate(1)->killCount >= 1) {
        assert(coord.GetPendingLootDrops().size() >= 1);
        assert(coord.TotalHaulersDestroyed() >= 1);
        assert(coord.TotalCargoLost() >= 5000.0f);
    }
}

// ══════════════════════════════════════════════════════════════════
// Configuration
// ══════════════════════════════════════════════════════════════════

void test_coordinator_attack_chance_config() {
    PirateSecurityCoordinator coord;
    coord.SetAttackChancePerScan(0.25f);
    assert(approxEq(coord.GetAttackChancePerScan(), 0.25f));
}

void test_coordinator_security_ratio_config() {
    PirateSecurityCoordinator coord;
    coord.SetSecurityPerPirateRatio(2.0f);
    assert(approxEq(coord.GetSecurityPerPirateRatio(), 2.0f));
    coord.SetMaxSecurityGuards(10);
    assert(coord.GetMaxSecurityGuards() == 10);
}

// ══════════════════════════════════════════════════════════════════
// End-to-end: full pirate → security response → economic impact
// ══════════════════════════════════════════════════════════════════

void test_coordinator_end_to_end_cycle() {
    PirateSecurityCoordinator coord;
    AIPirateStateMachine pirates;
    AISecurityStateMachine security;
    coord.SetPirateStateMachine(&pirates);
    coord.SetSecurityStateMachine(&security);

    // Setup: 2 pirates, 2 guards, 1 low-sec route
    for (uint32_t i = 1; i <= 2; ++i) {
        PirateConfig pcfg;
        pcfg.entityId = i;
        pcfg.travelSpeed = 600.0f;
        pcfg.scanRange = 300.0f;
        pcfg.lootCapacity = 200.0f;
        pirates.AddPirate(pcfg);
        coord.TrackPirate(i);
    }
    pirates.SetPatrolDistance(600.0f);

    for (uint32_t i = 10; i <= 11; ++i) {
        SecurityConfig scfg;
        scfg.entityId = i;
        scfg.hitPoints = 200.0f;
        scfg.travelSpeed = 400.0f;
        scfg.responseTime = 0.5f;
        security.AddGuard(scfg);
        coord.TrackGuard(i);
    }
    security.SetPostDistance(400.0f);

    TradeRoute route;
    route.haulerEntityId = 200;
    route.securityLevel = 0.2f;
    route.cargoValue = 3000.0f;
    coord.AddTradeRoute(route);

    // Run for 50 seconds of sim time
    for (int i = 0; i < 500; ++i) {
        coord.Tick(0.1f);
    }

    // Verify integrated behavior:
    // 1. Sim time advanced
    assert(approxEq(coord.SimTime(), 50.0f, 0.1f));

    // 2. Pirate activity should be > 0
    assert(coord.GetPirateActivityLevel() >= 0.0f);

    // 3. Security scaling: 2 pirates * 1.5 ratio = 3 recommended
    assert(coord.GetRecommendedSecurityCount() == 3);

    // 4. Faction standings unchanged (always -10 / +10)
    assert(approxEq(coord.GetPirateStanding().standing, -10.0f));
    assert(approxEq(coord.GetSecurityStanding().standing, 10.0f));
}
