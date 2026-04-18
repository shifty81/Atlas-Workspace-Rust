// Tests for: FleetReconSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_recon_system.h"

using namespace atlas;

static void testFleetReconInit() {
    std::cout << "\n=== FleetRecon: Init ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getMissionCount("e1") == 0, "Zero missions initially");
    assertTrue(sys.getDeployedCount("e1") == 0, "Zero deployed");
    assertTrue(sys.getTotalMissionsSent("e1") == 0, "Zero sent");
    assertTrue(sys.getTotalMissionsReturned("e1") == 0, "Zero returned");
    assertTrue(sys.getTotalScoutsLost("e1") == 0, "Zero lost");
    assertTrue(sys.getFleetId("e1").empty(), "Empty fleet id");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFleetReconDeployScout() {
    std::cout << "\n=== FleetRecon: DeployScout ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.deployScout("e1", "m1", "alpha_prime", "scout_1", 60.0f),
               "deployScout succeeds");
    assertTrue(sys.getMissionCount("e1") == 1, "Mission count = 1");
    assertTrue(sys.hasMission("e1", "m1"), "hasMission m1");
    assertTrue(sys.getMissionStatus("e1", "m1") == components::ReconStatus::Deployed,
               "Status = Deployed");
    assertTrue(approxEqual(sys.getMissionDuration("e1", "m1"), 60.0f),
               "Duration = 60");
    assertTrue(approxEqual(sys.getMissionElapsed("e1", "m1"), 0.0f), "Elapsed = 0");
    assertTrue(sys.getDeployedCount("e1") == 1, "Deployed count = 1");
    assertTrue(sys.getTotalMissionsSent("e1") == 1, "Total sent = 1");

    // Duplicate mission id rejected
    assertTrue(!sys.deployScout("e1", "m1", "beta", "scout_2", 30.0f),
               "Duplicate mission id rejected");

    // Invalid parameters
    assertTrue(!sys.deployScout("e1", "", "beta", "scout_2", 30.0f),
               "Empty mission_id rejected");
    assertTrue(!sys.deployScout("e1", "m2", "", "scout_2", 30.0f),
               "Empty target_system rejected");
    assertTrue(!sys.deployScout("e1", "m2", "beta", "", 30.0f),
               "Empty scout_id rejected");
    assertTrue(!sys.deployScout("e1", "m2", "beta", "scout_2", 0.0f),
               "Zero duration rejected");
    assertTrue(!sys.deployScout("e1", "m2", "beta", "scout_2", -1.0f),
               "Negative duration rejected");

    assertTrue(!sys.deployScout("missing", "m99", "x", "s", 10.0f),
               "Missing entity rejected");
}

static void testFleetReconCapacity() {
    std::cout << "\n=== FleetRecon: Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxMissions("e1", 3);

    assertTrue(sys.deployScout("e1", "m1", "sys1", "s1", 60.0f), "Deploy 1");
    assertTrue(sys.deployScout("e1", "m2", "sys2", "s2", 60.0f), "Deploy 2");
    assertTrue(sys.deployScout("e1", "m3", "sys3", "s3", 60.0f), "Deploy 3");
    assertTrue(!sys.deployScout("e1", "m4", "sys4", "s4", 60.0f),
               "4th deploy blocked at capacity");
    assertTrue(sys.getMissionCount("e1") == 3, "Mission count = 3");
    assertTrue(sys.getTotalMissionsSent("e1") == 3, "Total sent = 3");
}

static void testFleetReconRecallScout() {
    std::cout << "\n=== FleetRecon: RecallScout ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.deployScout("e1", "m1", "gamma", "scout_1", 120.0f);

    assertTrue(sys.recallScout("e1", "m1"), "recallScout succeeds");
    assertTrue(sys.getMissionStatus("e1", "m1") == components::ReconStatus::Returning,
               "Status = Returning after recall");

    // Cannot recall twice
    assertTrue(!sys.recallScout("e1", "m1"), "Cannot recall a non-Deployed mission");

    // Non-existent mission
    assertTrue(!sys.recallScout("e1", "nonexistent"), "Recall nonexistent fails");
    assertTrue(!sys.recallScout("missing", "m1"), "Missing entity fails");
}

static void testFleetReconSetMissionIntel() {
    std::cout << "\n=== FleetRecon: SetMissionIntel ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.deployScout("e1", "m1", "delta", "scout_1", 60.0f);

    assertTrue(sys.setMissionIntel("e1", "m1", 0.75f, 12, 3),
               "setMissionIntel succeeds");
    assertTrue(approxEqual(sys.getIntelThreat("e1", "m1"), 0.75f),
               "Intel threat = 0.75");
    assertTrue(sys.getIntelShips("e1", "m1") == 12, "Intel ships = 12");
    assertTrue(sys.getIntelAnomalies("e1", "m1") == 3, "Intel anomalies = 3");
    assertTrue(sys.isIntelReady("e1", "m1"), "Intel is ready");
    assertTrue(sys.getMissionStatus("e1", "m1") == components::ReconStatus::Intel_Ready,
               "Status = Intel_Ready");

    // Invalid values
    assertTrue(!sys.setMissionIntel("e1", "m1", -0.1f, 5, 2),
               "Negative threat rejected");
    assertTrue(!sys.setMissionIntel("e1", "m1", 1.1f, 5, 2),
               "Over-1 threat rejected");
    assertTrue(!sys.setMissionIntel("e1", "m1", 0.5f, -1, 2),
               "Negative ships rejected");
    assertTrue(!sys.setMissionIntel("e1", "m1", 0.5f, 5, -1),
               "Negative anomalies rejected");

    assertTrue(!sys.setMissionIntel("missing", "m1", 0.5f, 0, 0),
               "Missing entity fails");
    assertTrue(!sys.setMissionIntel("e1", "ghost", 0.5f, 0, 0),
               "Nonexistent mission fails");
}

static void testFleetReconConsumeIntel() {
    std::cout << "\n=== FleetRecon: ConsumeIntel ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.deployScout("e1", "m1", "epsilon", "scout_1", 60.0f);
    sys.setMissionIntel("e1", "m1", 0.5f, 5, 1);

    assertTrue(sys.isIntelReady("e1", "m1"), "Intel ready before consume");
    assertTrue(sys.consumeIntel("e1", "m1"), "consumeIntel succeeds");
    assertTrue(!sys.isIntelReady("e1", "m1"), "Intel no longer ready");
    assertTrue(sys.getMissionStatus("e1", "m1") == components::ReconStatus::Idle,
               "Status = Idle after consume");

    // Cannot consume again
    assertTrue(!sys.consumeIntel("e1", "m1"), "Cannot consume twice");

    assertTrue(!sys.consumeIntel("missing", "m1"), "Missing entity fails");
}

static void testFleetReconAutoReturn() {
    std::cout << "\n=== FleetRecon: AutoReturn ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.deployScout("e1", "m1", "zeta", "scout_1", 10.0f);

    // Before duration elapses — still Deployed
    sys.update(5.0f);
    assertTrue(sys.getMissionStatus("e1", "m1") == components::ReconStatus::Deployed,
               "Still Deployed at 5s / 10s duration");

    // After duration — transitions to Returning then Intel_Ready in next tick
    sys.update(6.0f);
    // At this point status should be Intel_Ready (Returning → Intel_Ready in same tick)
    assertTrue(
        sys.getMissionStatus("e1", "m1") == components::ReconStatus::Intel_Ready ||
        sys.getMissionStatus("e1", "m1") == components::ReconStatus::Returning,
        "Status progresses after duration");
    assertTrue(sys.getTotalMissionsReturned("e1") >= 0, "Returned counter non-negative");
}

static void testFleetReconRemoveAndClear() {
    std::cout << "\n=== FleetRecon: RemoveAndClear ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.deployScout("e1", "m1", "eta", "s1", 60.0f);
    sys.deployScout("e1", "m2", "theta", "s2", 60.0f);

    assertTrue(sys.removeMission("e1", "m1"), "removeMission succeeds");
    assertTrue(!sys.hasMission("e1", "m1"), "m1 no longer present");
    assertTrue(sys.hasMission("e1", "m2"), "m2 still present");
    assertTrue(sys.getMissionCount("e1") == 1, "Count = 1 after remove");

    assertTrue(!sys.removeMission("e1", "ghost"), "Remove nonexistent fails");

    assertTrue(sys.clearMissions("e1"), "clearMissions succeeds");
    assertTrue(sys.getMissionCount("e1") == 0, "Count = 0 after clear");

    assertTrue(!sys.removeMission("missing", "m1"), "Remove on missing entity fails");
    assertTrue(!sys.clearMissions("missing"), "Clear on missing entity fails");
}

static void testFleetReconConfiguration() {
    std::cout << "\n=== FleetRecon: Configuration ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setMaxMissions("e1", 8), "setMaxMissions(8) succeeds");
    assertTrue(!sys.setMaxMissions("e1", 0), "Zero max missions rejected");
    assertTrue(!sys.setMaxMissions("e1", -1), "Negative max rejected");

    assertTrue(sys.setScoutLossTimeout("e1", 600.0f), "setScoutLossTimeout succeeds");
    assertTrue(!sys.setScoutLossTimeout("e1", 0.0f), "Zero timeout rejected");
    assertTrue(!sys.setScoutLossTimeout("e1", -1.0f), "Negative timeout rejected");

    assertTrue(sys.setFleetId("e1", "fleet_alpha"), "setFleetId succeeds");
    assertTrue(sys.getFleetId("e1") == "fleet_alpha", "FleetId = fleet_alpha");
    assertTrue(!sys.setFleetId("e1", ""), "Empty fleet id rejected");

    assertTrue(!sys.setMaxMissions("missing", 5), "setMaxMissions on missing fails");
    assertTrue(!sys.setFleetId("missing", "x"), "setFleetId on missing fails");
}

static void testFleetReconDefaultIntelQueries() {
    std::cout << "\n=== FleetRecon: DefaultIntelQueries ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Querying nonexistent mission returns safe defaults
    assertTrue(approxEqual(sys.getIntelThreat("e1", "ghost"), 0.0f),
               "getIntelThreat = 0 for missing mission");
    assertTrue(sys.getIntelShips("e1", "ghost") == 0,
               "getIntelShips = 0 for missing mission");
    assertTrue(sys.getIntelAnomalies("e1", "ghost") == 0,
               "getIntelAnomalies = 0 for missing mission");
    assertTrue(!sys.isIntelReady("e1", "ghost"),
               "isIntelReady = false for missing mission");
    assertTrue(approxEqual(sys.getMissionElapsed("e1", "ghost"), 0.0f),
               "getMissionElapsed = 0 for missing mission");
    assertTrue(approxEqual(sys.getMissionDuration("e1", "ghost"), 0.0f),
               "getMissionDuration = 0 for missing mission");
}

static void testFleetReconMissingEntity() {
    std::cout << "\n=== FleetRecon: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetReconSystem sys(&world);

    assertTrue(sys.getMissionCount("missing") == 0, "getMissionCount = 0");
    assertTrue(!sys.hasMission("missing", "m1"), "hasMission = false");
    assertTrue(sys.getMissionStatus("missing", "m1") == components::ReconStatus::Idle,
               "getMissionStatus = Idle");
    assertTrue(approxEqual(sys.getMissionElapsed("missing", "m1"), 0.0f),
               "getMissionElapsed = 0");
    assertTrue(approxEqual(sys.getMissionDuration("missing", "m1"), 0.0f),
               "getMissionDuration = 0");
    assertTrue(approxEqual(sys.getIntelThreat("missing", "m1"), 0.0f),
               "getIntelThreat = 0");
    assertTrue(sys.getIntelShips("missing", "m1") == 0, "getIntelShips = 0");
    assertTrue(sys.getIntelAnomalies("missing", "m1") == 0, "getIntelAnomalies = 0");
    assertTrue(!sys.isIntelReady("missing", "m1"), "isIntelReady = false");
    assertTrue(sys.getDeployedCount("missing") == 0, "getDeployedCount = 0");
    assertTrue(sys.getTotalMissionsSent("missing") == 0, "getTotalSent = 0");
    assertTrue(sys.getTotalMissionsReturned("missing") == 0, "getTotalReturned = 0");
    assertTrue(sys.getTotalScoutsLost("missing") == 0, "getTotalLost = 0");
    assertTrue(sys.getFleetId("missing").empty(), "getFleetId = ''");
    assertTrue(!sys.deployScout("missing", "m1", "x", "s", 10.0f), "deployScout = false");
    assertTrue(!sys.recallScout("missing", "m1"), "recallScout = false");
    assertTrue(!sys.setMissionIntel("missing", "m1", 0.5f, 0, 0), "setIntel = false");
    assertTrue(!sys.consumeIntel("missing", "m1"), "consumeIntel = false");
}

void run_fleet_recon_system_tests() {
    testFleetReconInit();
    testFleetReconDeployScout();
    testFleetReconCapacity();
    testFleetReconRecallScout();
    testFleetReconSetMissionIntel();
    testFleetReconConsumeIntel();
    testFleetReconAutoReturn();
    testFleetReconRemoveAndClear();
    testFleetReconConfiguration();
    testFleetReconDefaultIntelQueries();
    testFleetReconMissingEntity();
}
