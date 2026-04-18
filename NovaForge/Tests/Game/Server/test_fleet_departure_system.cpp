// Tests for: FleetDepartureSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_departure_system.h"

using namespace atlas;
using Stage = components::FleetDepartureState::DepartureStage;

static void testFleetDepartureInit() {
    std::cout << "\n=== FleetDeparture: Init ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getStage("e1") == Stage::Stable, "Default stage Stable");
    assertTrue(approxEqual(sys.getDepartureRisk("e1"), 0.0f), "Default risk 0.0");
    assertTrue(approxEqual(sys.getMorale("e1"), 0.0f), "Default morale 0.0");
    assertTrue(sys.getLosingStreak("e1") == 0, "Zero losing streak");
    assertTrue(sys.getConsecutiveNearDeaths("e1") == 0, "Zero near deaths");
    assertTrue(!sys.hasDepartureRequest("e1"), "No departure request");
    assertTrue(sys.getDepartureReason("e1").empty(), "Empty departure reason");
    assertTrue(approxEqual(sys.getTimeInStage("e1"), 0.0f), "Zero time in stage");
    assertTrue(sys.getTotalDepartures("e1") == 0, "Zero departures");
    assertTrue(sys.getTotalTransfers("e1") == 0, "Zero transfers");
    assertTrue(!sys.isDepartureRisk("e1"), "Not at departure risk");
    assertTrue(!sys.isGone("e1"), "Not gone");
    assertTrue(sys.getStageString("e1") == "Stable", "Stage string Stable");
    assertTrue(sys.getCaptainId("e1").empty(), "Empty captain_id");
    assertTrue(sys.getFleetId("e1").empty(), "Empty fleet_id");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFleetDepartureRecordNearDeath() {
    std::cout << "\n=== FleetDeparture: RecordNearDeath ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.recordNearDeath("e1"), "recordNearDeath succeeds");
    assertTrue(sys.getConsecutiveNearDeaths("e1") == 1, "NearDeaths = 1");
    assertTrue(sys.getDepartureRisk("e1") > 0.0f, "Risk > 0 after near-death");

    assertTrue(sys.recordNearDeath("e1"), "Second near-death succeeds");
    assertTrue(sys.getConsecutiveNearDeaths("e1") == 2, "NearDeaths = 2");
    assertTrue(sys.getDepartureRisk("e1") >= 0.4f, "Risk >= 0.4 after 2 near-deaths");

    assertTrue(!sys.recordNearDeath("missing"), "recordNearDeath on missing fails");
}

static void testFleetDepartureRecordLoss() {
    std::cout << "\n=== FleetDeparture: RecordLoss ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.recordLoss("e1"), "recordLoss succeeds");
    assertTrue(sys.getLosingStreak("e1") == 1, "Losing streak = 1");
    assertTrue(sys.getDepartureRisk("e1") > 0.0f, "Risk > 0 after loss");

    for (int i = 0; i < 9; ++i) sys.recordLoss("e1");
    assertTrue(sys.getLosingStreak("e1") == 10, "Streak = 10");
    assertTrue(sys.getDepartureRisk("e1") <= 1.0f, "Risk capped at 1.0");

    assertTrue(!sys.recordLoss("missing"), "recordLoss on missing fails");
}

static void testFleetDepartureRecordWin() {
    std::cout << "\n=== FleetDeparture: RecordWin ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.recordLoss("e1");
    sys.recordNearDeath("e1");
    assertTrue(sys.getDepartureRisk("e1") > 0.0f, "Risk > 0 before win");

    assertTrue(sys.recordWin("e1"), "recordWin succeeds");
    assertTrue(sys.getLosingStreak("e1") == 0, "Streak reset to 0");
    assertTrue(sys.getConsecutiveNearDeaths("e1") == 0, "NearDeaths reset to 0");
    assertTrue(approxEqual(sys.getDepartureRisk("e1"), 0.0f), "Risk reset to 0");

    assertTrue(!sys.recordWin("missing"), "recordWin on missing fails");
}

static void testFleetDepartureMorale() {
    std::cout << "\n=== FleetDeparture: Morale ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setMorale("e1", -80.0f), "setMorale(-80) succeeds");
    assertTrue(approxEqual(sys.getMorale("e1"), -80.0f), "Morale = -80");
    // Risk = max(0, (-(-80) - 30) / 100) = 50/100 = 0.5
    assertTrue(sys.getDepartureRisk("e1") > 0.3f, "Risk elevated from low morale");

    assertTrue(sys.setMorale("e1", 50.0f), "setMorale(50) succeeds");
    assertTrue(approxEqual(sys.getMorale("e1"), 50.0f), "Morale = 50");
    assertTrue(approxEqual(sys.getDepartureRisk("e1"), 0.0f), "Risk zero from high morale");

    // Clamping
    assertTrue(sys.setMorale("e1", 150.0f), "setMorale clamps high");
    assertTrue(approxEqual(sys.getMorale("e1"), 100.0f), "Morale clamped to 100");
    assertTrue(sys.setMorale("e1", -200.0f), "setMorale clamps low");
    assertTrue(approxEqual(sys.getMorale("e1"), -100.0f), "Morale clamped to -100");

    assertTrue(!sys.setMorale("missing", 0.0f), "setMorale on missing fails");
}

static void testFleetDepartureAutoTriggerArguing() {
    std::cout << "\n=== FleetDeparture: Auto-trigger Arguing ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setDepartureThreshold("e1", 0.4f);

    // Enough near-deaths to push risk above 0.4
    sys.recordNearDeath("e1");
    sys.recordNearDeath("e1");  // risk = 0.4

    assertTrue(sys.getStage("e1") == Stage::Stable, "Still Stable before update");
    sys.update(0.1f);  // tick triggers auto-advance
    assertTrue(sys.getStage("e1") == Stage::Arguing, "Advance to Arguing after tick");
    assertTrue(sys.isDepartureRisk("e1"), "isDepartureRisk = true");
    assertTrue(sys.getStageString("e1") == "Arguing", "Stage string Arguing");
}

static void testFleetDepartureRequestTransfer() {
    std::cout << "\n=== FleetDeparture: RequestTransfer ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Cannot request from Stable
    assertTrue(!sys.requestTransfer("e1", "Bad orders"), "Cannot request from Stable");

    // Advance to Arguing
    sys.setDepartureThreshold("e1", 0.3f);
    sys.recordNearDeath("e1");
    sys.recordNearDeath("e1");
    sys.update(0.1f);
    assertTrue(sys.getStage("e1") == Stage::Arguing, "Stage = Arguing");

    assertTrue(sys.requestTransfer("e1", "Repeated near-deaths"), "requestTransfer succeeds");
    assertTrue(sys.getStage("e1") == Stage::Requesting, "Stage = Requesting");
    assertTrue(sys.hasDepartureRequest("e1"), "Has departure request");
    assertTrue(sys.getDepartureReason("e1") == "Repeated near-deaths", "Reason matches");
    assertTrue(sys.isDepartureRisk("e1"), "Still at departure risk");

    assertTrue(!sys.requestTransfer("missing", "x"), "requestTransfer on missing fails");
}

static void testFleetDepartureAcceptTransfer() {
    std::cout << "\n=== FleetDeparture: AcceptTransfer ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Cannot accept from Stable
    assertTrue(!sys.acceptTransfer("e1"), "Cannot accept from Stable");

    // Bring to Requesting
    sys.setDepartureThreshold("e1", 0.2f);
    sys.recordNearDeath("e1");
    sys.update(0.1f);
    sys.requestTransfer("e1", "Morale collapse");

    assertTrue(sys.acceptTransfer("e1"), "acceptTransfer succeeds");
    assertTrue(sys.getStage("e1") == Stage::Departed, "Stage = Departed");
    assertTrue(sys.getTotalTransfers("e1") == 1, "Total transfers = 1");
    assertTrue(sys.isGone("e1"), "isGone = true");
    assertTrue(sys.getStageString("e1") == "Departed", "Stage string Departed");

    assertTrue(!sys.acceptTransfer("missing"), "acceptTransfer on missing fails");
}

static void testFleetDepartureResolveConflict() {
    std::cout << "\n=== FleetDeparture: ResolveConflict ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Cannot resolve from Stable
    assertTrue(!sys.resolveConflict("e1"), "Cannot resolve from Stable");

    // Advance to Arguing
    sys.setDepartureThreshold("e1", 0.2f);
    sys.recordNearDeath("e1");
    sys.update(0.1f);
    assertTrue(sys.getStage("e1") == Stage::Arguing, "Stage = Arguing");

    assertTrue(sys.resolveConflict("e1"), "resolveConflict from Arguing succeeds");
    assertTrue(sys.getStage("e1") == Stage::Stable, "Returned to Stable");
    assertTrue(approxEqual(sys.getDepartureRisk("e1"), 0.0f), "Risk cleared");
    assertTrue(!sys.hasDepartureRequest("e1"), "No departure request");
    assertTrue(sys.getLosingStreak("e1") == 0, "Streak cleared");
    assertTrue(sys.getConsecutiveNearDeaths("e1") == 0, "NearDeaths cleared");

    // Also works from Requesting
    sys.recordNearDeath("e1");
    sys.update(0.1f);
    sys.requestTransfer("e1", "Again");
    assertTrue(sys.resolveConflict("e1"), "resolveConflict from Requesting succeeds");
    assertTrue(sys.getStage("e1") == Stage::Stable, "Returned to Stable from Requesting");

    assertTrue(!sys.resolveConflict("missing"), "resolveConflict on missing fails");
}

static void testFleetDepartureConfiguration() {
    std::cout << "\n=== FleetDeparture: Configuration ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setDepartureThreshold("e1", 0.6f), "setThreshold(0.6) succeeds");
    assertTrue(!sys.setDepartureThreshold("e1", -0.1f), "Negative threshold rejected");
    assertTrue(!sys.setDepartureThreshold("e1", 1.1f), "Over-1.0 threshold rejected");

    assertTrue(sys.resetStreak("e1"), "resetStreak succeeds");

    assertTrue(sys.setCaptainId("e1", "cap_bravo"), "setCaptainId succeeds");
    assertTrue(sys.getCaptainId("e1") == "cap_bravo", "CaptainId = cap_bravo");
    assertTrue(!sys.setCaptainId("e1", ""), "Empty captainId rejected");

    assertTrue(sys.setFleetId("e1", "fleet_7"), "setFleetId succeeds");
    assertTrue(sys.getFleetId("e1") == "fleet_7", "FleetId = fleet_7");
    assertTrue(!sys.setFleetId("e1", ""), "Empty fleetId rejected");

    assertTrue(!sys.setDepartureThreshold("missing", 0.5f), "setThreshold on missing fails");
    assertTrue(!sys.resetStreak("missing"), "resetStreak on missing fails");
    assertTrue(!sys.setCaptainId("missing", "x"), "setCaptainId on missing fails");
    assertTrue(!sys.setFleetId("missing", "x"), "setFleetId on missing fails");
}

static void testFleetDepartureTimeInStage() {
    std::cout << "\n=== FleetDeparture: TimeInStage ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.update(5.0f);
    assertTrue(sys.getTimeInStage("e1") > 0.0f, "TimeInStage advances");

    // Advance to Arguing — time_in_stage resets
    sys.setDepartureThreshold("e1", 0.2f);
    sys.recordNearDeath("e1");
    sys.update(0.1f);
    assertTrue(sys.getStage("e1") == Stage::Arguing, "Stage = Arguing");
    assertTrue(sys.getTimeInStage("e1") < 1.0f, "TimeInStage reset on stage change");
}

static void testFleetDepartureMissingEntity() {
    std::cout << "\n=== FleetDeparture: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetDepartureSystem sys(&world);

    assertTrue(sys.getStage("missing") == Stage::Stable, "getStage = Stable");
    assertTrue(approxEqual(sys.getDepartureRisk("missing"), 0.0f), "getDepartureRisk = 0");
    assertTrue(approxEqual(sys.getMorale("missing"), 0.0f), "getMorale = 0");
    assertTrue(sys.getLosingStreak("missing") == 0, "getLosingStreak = 0");
    assertTrue(sys.getConsecutiveNearDeaths("missing") == 0, "getNearDeaths = 0");
    assertTrue(!sys.hasDepartureRequest("missing"), "hasDepartureRequest = false");
    assertTrue(sys.getDepartureReason("missing").empty(), "getDepartureReason = ''");
    assertTrue(approxEqual(sys.getTimeInStage("missing"), 0.0f), "getTimeInStage = 0");
    assertTrue(sys.getTotalDepartures("missing") == 0, "getTotalDepartures = 0");
    assertTrue(sys.getTotalTransfers("missing") == 0, "getTotalTransfers = 0");
    assertTrue(!sys.isDepartureRisk("missing"), "isDepartureRisk = false");
    assertTrue(!sys.isGone("missing"), "isGone = false");
    assertTrue(sys.getStageString("missing").empty(), "getStageString = ''");
    assertTrue(sys.getCaptainId("missing").empty(), "getCaptainId = ''");
    assertTrue(sys.getFleetId("missing").empty(), "getFleetId = ''");
    assertTrue(!sys.recordNearDeath("missing"), "recordNearDeath = false");
    assertTrue(!sys.recordLoss("missing"), "recordLoss = false");
    assertTrue(!sys.recordWin("missing"), "recordWin = false");
    assertTrue(!sys.setMorale("missing", 0.0f), "setMorale = false");
    assertTrue(!sys.requestTransfer("missing", "x"), "requestTransfer = false");
    assertTrue(!sys.acceptTransfer("missing"), "acceptTransfer = false");
    assertTrue(!sys.resolveConflict("missing"), "resolveConflict = false");
}

void run_fleet_departure_system_tests() {
    testFleetDepartureInit();
    testFleetDepartureRecordNearDeath();
    testFleetDepartureRecordLoss();
    testFleetDepartureRecordWin();
    testFleetDepartureMorale();
    testFleetDepartureAutoTriggerArguing();
    testFleetDepartureRequestTransfer();
    testFleetDepartureAcceptTransfer();
    testFleetDepartureResolveConflict();
    testFleetDepartureConfiguration();
    testFleetDepartureTimeInStage();
    testFleetDepartureMissingEntity();
}
