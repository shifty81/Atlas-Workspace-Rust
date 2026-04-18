// Tests for: FleetMoraleSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_morale_system.h"

using namespace atlas;

static void testFleetMoraleInit() {
    std::cout << "\n=== FleetMoraleSystem: Init ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("f1");

    assertTrue(sys.initialize("f1"), "Init succeeds");
    assertTrue(approxEqual(sys.getMorale("f1"), 0.5f), "Default morale is 0.5");
    assertTrue(approxEqual(sys.getCohesion("f1"), 0.5f), "Default cohesion is 0.5");
    assertTrue(sys.getEventCount("f1") == 0, "No events initially");
    assertTrue(sys.getTotalEvents("f1") == 0, "Total events is 0");
    assertTrue(sys.getVictories("f1") == 0, "Victories is 0");
    assertTrue(sys.getDefeats("f1") == 0, "Defeats is 0");
    assertTrue(sys.getFleetId("f1").empty(), "Fleet ID is empty");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testRecordEvents() {
    std::cout << "\n=== FleetMoraleSystem: RecordEvents ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using ME = components::FleetMoraleState::MoraleEvent;

    // Victory: morale +0.1, cohesion +0.05
    assertTrue(sys.recordEvent("f1", ME::Victory), "Record Victory succeeds");
    assertTrue(approxEqual(sys.getMorale("f1"), 0.6f), "Morale after Victory is 0.6");
    assertTrue(approxEqual(sys.getCohesion("f1"), 0.55f), "Cohesion after Victory is 0.55");
    assertTrue(sys.getVictories("f1") == 1, "Victories incremented");

    // Defeat: morale -0.15, cohesion -0.05
    assertTrue(sys.recordEvent("f1", ME::Defeat), "Record Defeat succeeds");
    assertTrue(approxEqual(sys.getMorale("f1"), 0.45f), "Morale after Defeat");
    assertTrue(sys.getDefeats("f1") == 1, "Defeats incremented");

    // AllyDestroyed: morale -0.2, cohesion -0.1
    assertTrue(sys.recordEvent("f1", ME::AllyDestroyed), "Record AllyDestroyed succeeds");
    assertTrue(approxEqual(sys.getMorale("f1"), 0.25f), "Morale after AllyDestroyed");
    assertTrue(approxEqual(sys.getCohesion("f1"), 0.4f), "Cohesion after AllyDestroyed");

    // LootShared: morale +0.08, cohesion +0.1
    assertTrue(sys.recordEvent("f1", ME::LootShared), "Record LootShared succeeds");
    assertTrue(approxEqual(sys.getMorale("f1"), 0.33f), "Morale after LootShared");
    assertTrue(approxEqual(sys.getCohesion("f1"), 0.5f), "Cohesion after LootShared");

    assertTrue(sys.getEventCount("f1") == 4, "Event log has 4 entries");
    assertTrue(sys.getTotalEvents("f1") == 4, "Total events is 4");

    assertTrue(!sys.recordEvent("ghost", ME::Victory), "Record on missing entity fails");
}

static void testMoraleManipulation() {
    std::cout << "\n=== FleetMoraleSystem: MoraleManipulation ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    assertTrue(sys.boostMorale("f1", 0.4f), "Boost morale succeeds");
    assertTrue(approxEqual(sys.getMorale("f1"), 0.9f), "Morale boosted to 0.9");

    assertTrue(sys.boostMorale("f1", 0.5f), "Boost morale clamps at 1.0");
    assertTrue(approxEqual(sys.getMorale("f1"), 1.0f), "Morale clamped to 1.0");

    assertTrue(sys.reduceMorale("f1", 1.5f), "Reduce morale clamps at 0.0");
    assertTrue(approxEqual(sys.getMorale("f1"), 0.0f), "Morale clamped to 0.0");

    assertTrue(sys.resetMorale("f1"), "Reset morale succeeds");
    assertTrue(approxEqual(sys.getMorale("f1"), 0.5f), "Morale reset to baseline");

    assertTrue(!sys.boostMorale("f1", -0.1f), "Reject negative boost amount");
    assertTrue(!sys.reduceMorale("f1", 0.0f), "Reject zero reduce amount");
}

static void testCohesionManipulation() {
    std::cout << "\n=== FleetMoraleSystem: CohesionManipulation ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    assertTrue(sys.boostCohesion("f1", 0.3f), "Boost cohesion succeeds");
    assertTrue(approxEqual(sys.getCohesion("f1"), 0.8f), "Cohesion boosted to 0.8");

    assertTrue(sys.boostCohesion("f1", 0.5f), "Boost cohesion clamps at 1.0");
    assertTrue(approxEqual(sys.getCohesion("f1"), 1.0f), "Cohesion clamped to 1.0");

    assertTrue(sys.reduceCohesion("f1", 1.5f), "Reduce cohesion clamps at 0.0");
    assertTrue(approxEqual(sys.getCohesion("f1"), 0.0f), "Cohesion clamped to 0.0");

    assertTrue(!sys.boostCohesion("f1", -0.1f), "Reject negative cohesion boost");
    assertTrue(!sys.reduceCohesion("f1", 0.0f), "Reject zero cohesion reduce");
    assertTrue(!sys.boostCohesion("ghost", 0.1f), "Boost on missing entity fails");
}

static void testMoraleDrift() {
    std::cout << "\n=== FleetMoraleSystem: MoraleDrift ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    // Boost morale high then let it drift down toward baseline
    sys.boostMorale("f1", 0.4f);
    float before = sys.getMorale("f1");
    assertTrue(approxEqual(before, 0.9f), "Morale starts at 0.9 before drift");

    sys.update(1.0f);
    float after = sys.getMorale("f1");
    assertTrue(after < before, "Morale drifts down toward baseline");
    assertTrue(after > 0.5f, "Morale still above baseline after 1s");

    // Reduce morale low then drift up
    sys.reduceMorale("f1", 0.8f);
    float low = sys.getMorale("f1");
    sys.update(1.0f);
    float afterUp = sys.getMorale("f1");
    assertTrue(afterUp > low, "Morale drifts up toward baseline");

    // Cohesion drift
    sys.boostCohesion("f1", 0.4f);
    float cBefore = sys.getCohesion("f1");
    sys.update(1.0f);
    float cAfter = sys.getCohesion("f1");
    assertTrue(cAfter < cBefore, "Cohesion drifts down toward baseline");
}

static void testEventLogTrimming() {
    std::cout << "\n=== FleetMoraleSystem: EventLogTrimming ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using ME = components::FleetMoraleState::MoraleEvent;

    sys.setMaxEventLog("f1", 5);

    for (int i = 0; i < 8; ++i) {
        sys.recordEvent("f1", ME::OrderFollowed);
    }

    assertTrue(sys.getEventCount("f1") == 5, "Event log trimmed to max 5");
    assertTrue(sys.getTotalEvents("f1") == 8, "Total events still counted as 8");

    assertTrue(sys.clearEventLog("f1"), "Clear event log succeeds");
    assertTrue(sys.getEventCount("f1") == 0, "Event log cleared");
}

static void testHighLowMoraleChecks() {
    std::cout << "\n=== FleetMoraleSystem: HighLowMoraleChecks ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    // Default 0.5 — neither high nor low
    assertTrue(!sys.isHighMorale("f1"), "Default morale not high");
    assertTrue(!sys.isLowMorale("f1"), "Default morale not low");
    assertTrue(!sys.isHighCohesion("f1"), "Default cohesion not high");
    assertTrue(!sys.isLowCohesion("f1"), "Default cohesion not low");

    sys.boostMorale("f1", 0.3f);
    assertTrue(sys.isHighMorale("f1"), "0.8 is high morale");

    sys.reduceMorale("f1", 0.7f);
    assertTrue(sys.isLowMorale("f1"), "0.1 is low morale");

    sys.boostCohesion("f1", 0.3f);
    assertTrue(sys.isHighCohesion("f1"), "0.8 is high cohesion");
}

static void testConfiguration() {
    std::cout << "\n=== FleetMoraleSystem: Configuration ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    assertTrue(sys.setFleetId("f1", "alpha-wing"), "Set fleet ID succeeds");
    assertTrue(sys.getFleetId("f1") == "alpha-wing", "Fleet ID set correctly");
    assertTrue(!sys.setFleetId("f1", ""), "Reject empty fleet ID");

    assertTrue(sys.setMoraleDecay("f1", 0.05f), "Set morale decay succeeds");
    assertTrue(!sys.setMoraleDecay("f1", 0.0f), "Reject zero morale decay");
    assertTrue(!sys.setMoraleDecay("f1", 1.5f), "Reject morale decay > 1");

    assertTrue(sys.setCohesionDecay("f1", 0.02f), "Set cohesion decay succeeds");
    assertTrue(!sys.setCohesionDecay("f1", -0.1f), "Reject negative cohesion decay");

    assertTrue(sys.setMoraleBaseline("f1", 0.6f), "Set morale baseline succeeds");
    assertTrue(!sys.setMoraleBaseline("f1", 1.5f), "Reject baseline > 1");

    assertTrue(sys.setCohesionBaseline("f1", 0.4f), "Set cohesion baseline succeeds");
    assertTrue(!sys.setCohesionBaseline("f1", -0.1f), "Reject negative baseline");

    assertTrue(sys.setMaxEventLog("f1", 10), "Set max event log succeeds");
    assertTrue(!sys.setMaxEventLog("f1", 0), "Reject max event log < 1");
}

static void testMissingEntity() {
    std::cout << "\n=== FleetMoraleSystem: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);

    assertTrue(approxEqual(sys.getMorale("ghost"), 0.0f), "getMorale returns 0 for missing");
    assertTrue(approxEqual(sys.getCohesion("ghost"), 0.0f), "getCohesion returns 0 for missing");
    assertTrue(sys.getEventCount("ghost") == 0, "getEventCount returns 0 for missing");
    assertTrue(sys.getTotalEvents("ghost") == 0, "getTotalEvents returns 0 for missing");
    assertTrue(sys.getVictories("ghost") == 0, "getVictories returns 0 for missing");
    assertTrue(sys.getDefeats("ghost") == 0, "getDefeats returns 0 for missing");
    assertTrue(!sys.isHighMorale("ghost"), "isHighMorale false for missing");
    assertTrue(!sys.isLowMorale("ghost"), "isLowMorale false for missing");
    assertTrue(!sys.isHighCohesion("ghost"), "isHighCohesion false for missing");
    assertTrue(!sys.isLowCohesion("ghost"), "isLowCohesion false for missing");
    assertTrue(sys.getFleetId("ghost").empty(), "getFleetId empty for missing");
    assertTrue(!sys.resetMorale("ghost"), "resetMorale fails for missing");
    assertTrue(!sys.clearEventLog("ghost"), "clearEventLog fails for missing");
}

static void testVictoryDefeatCounting() {
    std::cout << "\n=== FleetMoraleSystem: VictoryDefeatCounting ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("f1");
    sys.initialize("f1");

    using ME = components::FleetMoraleState::MoraleEvent;

    sys.recordEvent("f1", ME::Victory);
    sys.recordEvent("f1", ME::Victory);
    sys.recordEvent("f1", ME::Defeat);
    sys.recordEvent("f1", ME::Victory);
    sys.recordEvent("f1", ME::Defeat);

    assertTrue(sys.getVictories("f1") == 3, "Three victories counted");
    assertTrue(sys.getDefeats("f1") == 2, "Two defeats counted");
    assertTrue(sys.getTotalEvents("f1") == 5, "Five total events");
    assertTrue(sys.getEventCount("f1") == 5, "Five events in log");
}

void run_fleet_morale_system_tests() {
    testFleetMoraleInit();
    testRecordEvents();
    testMoraleManipulation();
    testCohesionManipulation();
    testMoraleDrift();
    testEventLogTrimming();
    testHighLowMoraleChecks();
    testConfiguration();
    testMissingEntity();
    testVictoryDefeatCounting();
}
