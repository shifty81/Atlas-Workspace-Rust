// Tests for: DirectionalScanSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/directional_scan_system.h"

using namespace atlas;

// ==================== DirectionalScanSystem Tests ====================

static void testDScanInit() {
    std::cout << "\n=== DirectionalScan: Init ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getResultCount("e1") == 0, "Zero results initially");
    assertTrue(approxEqual(sys.getScanAngle("e1"), 360.0f), "Default scan angle 360");
    assertTrue(approxEqual(sys.getScanRange("e1"), 14.3f), "Default scan range 14.3 AU");
    assertTrue(!sys.isScanning("e1"), "Not scanning initially");
    assertTrue(!sys.isScanComplete("e1"), "Not complete initially");
    assertTrue(!sys.isOnCooldown("e1"), "Not on cooldown initially");
    assertTrue(approxEqual(sys.getScanProgress("e1"), 0.0f), "Zero progress");
    assertTrue(approxEqual(sys.getCooldownRemaining("e1"), 0.0f), "Zero cooldown");
    assertTrue(sys.getTotalScans("e1") == 0, "Zero total scans");
    assertTrue(sys.getTotalObjectsFound("e1") == 0, "Zero total objects");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testDScanStartCancel() {
    std::cout << "\n=== DirectionalScan: Start/Cancel ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.startScan("e1"), "Start scan succeeds");
    assertTrue(sys.isScanning("e1"), "Is scanning");
    assertTrue(!sys.startScan("e1"), "Cannot start while scanning");

    assertTrue(sys.cancelScan("e1"), "Cancel scan");
    assertTrue(!sys.isScanning("e1"), "Not scanning after cancel");
    assertTrue(!sys.cancelScan("e1"), "Cannot cancel when not scanning");

    assertTrue(!sys.startScan("missing"), "Start on missing fails");
    assertTrue(!sys.cancelScan("missing"), "Cancel on missing fails");
}

static void testDScanCompletion() {
    std::cout << "\n=== DirectionalScan: Completion ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setScanDuration("e1", 4.0f);
    sys.setCooldownDuration("e1", 3.0f);

    assertTrue(sys.startScan("e1"), "Start scan");

    sys.update(2.0f);
    assertTrue(sys.isScanning("e1"), "Still scanning at 2s");
    assertTrue(approxEqual(sys.getScanProgress("e1"), 0.5f), "50% progress at 2s");

    sys.update(3.0f);
    assertTrue(sys.isScanComplete("e1"), "Scan complete after 5s total");
    assertTrue(approxEqual(sys.getScanProgress("e1"), 1.0f), "100% progress");
    assertTrue(sys.getTotalScans("e1") == 1, "1 total scan");
    assertTrue(sys.isOnCooldown("e1"), "On cooldown after scan");
    assertTrue(approxEqual(sys.getCooldownRemaining("e1"), 3.0f, 0.5f),
               "Cooldown ~3s");

    // Cannot start while on cooldown
    // First reset to idle by starting a new scan attempt
    assertTrue(!sys.startScan("e1"), "Cannot start on cooldown");

    sys.update(4.0f);
    assertTrue(!sys.isOnCooldown("e1"), "Cooldown expired");
    assertTrue(sys.startScan("e1"), "Start scan after cooldown");
}

static void testDScanConfiguration() {
    std::cout << "\n=== DirectionalScan: Configuration ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setScanAngle("e1", 60.0f), "Set angle 60");
    assertTrue(approxEqual(sys.getScanAngle("e1"), 60.0f), "Angle is 60");
    assertTrue(sys.setScanAngle("e1", 5.0f), "Set angle 5 (min)");
    assertTrue(sys.setScanAngle("e1", 360.0f), "Set angle 360 (max)");
    assertTrue(!sys.setScanAngle("e1", 4.0f), "Angle < 5 rejected");
    assertTrue(!sys.setScanAngle("e1", 361.0f), "Angle > 360 rejected");

    assertTrue(sys.setScanRange("e1", 10.0f), "Set range 10 AU");
    assertTrue(approxEqual(sys.getScanRange("e1"), 10.0f), "Range is 10");
    assertTrue(!sys.setScanRange("e1", 0.0f), "Zero range rejected");
    assertTrue(!sys.setScanRange("e1", -1.0f), "Negative range rejected");

    assertTrue(sys.setScanDuration("e1", 5.0f), "Set duration 5s");
    assertTrue(!sys.setScanDuration("e1", 0.0f), "Zero duration rejected");

    assertTrue(sys.setCooldownDuration("e1", 0.0f), "Set cooldown 0 (no cooldown)");
    assertTrue(!sys.setCooldownDuration("e1", -1.0f), "Negative cooldown rejected");

    assertTrue(sys.setMaxResults("e1", 100), "Set max results 100");
    assertTrue(!sys.setMaxResults("e1", 0), "Zero max results rejected");

    assertTrue(!sys.setScanAngle("missing", 60.0f), "Angle on missing fails");
    assertTrue(!sys.setScanRange("missing", 10.0f), "Range on missing fails");
    assertTrue(!sys.setScanDuration("missing", 5.0f), "Duration on missing fails");
    assertTrue(!sys.setCooldownDuration("missing", 3.0f), "Cooldown on missing fails");
    assertTrue(!sys.setMaxResults("missing", 10), "MaxResults on missing fails");
}

static void testDScanAddResult() {
    std::cout << "\n=== DirectionalScan: AddResult ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using OT = components::DirectionalScanState::ObjectType;

    assertTrue(sys.addResult("e1", "r1", "Enemy Ship", OT::Ship, 5000.0f, 45.0f),
               "Add ship result");
    assertTrue(sys.getResultCount("e1") == 1, "1 result");
    assertTrue(sys.hasResult("e1", "r1"), "Has r1");
    assertTrue(approxEqual(sys.getResultDistance("e1", "r1"), 5000.0f), "Distance 5000");
    assertTrue(approxEqual(sys.getResultBearing("e1", "r1"), 45.0f), "Bearing 45");
    assertTrue(sys.getTotalObjectsFound("e1") == 1, "1 total object");

    assertTrue(sys.addResult("e1", "r2", "Station", OT::Structure, 10000.0f, 180.0f),
               "Add structure result");
    assertTrue(sys.getResultCount("e1") == 2, "2 results");
    assertTrue(sys.getCountByType("e1", OT::Ship) == 1, "1 ship");
    assertTrue(sys.getCountByType("e1", OT::Structure) == 1, "1 structure");

    // Duplicate rejected
    assertTrue(!sys.addResult("e1", "r1", "Dupe", OT::Ship, 1000.0f, 0.0f),
               "Duplicate result rejected");

    // Empty ID rejected
    assertTrue(!sys.addResult("e1", "", "X", OT::Ship, 100.0f, 0.0f),
               "Empty ID rejected");

    // Negative distance rejected
    assertTrue(!sys.addResult("e1", "r3", "X", OT::Ship, -1.0f, 0.0f),
               "Negative distance rejected");

    // Invalid bearing rejected
    assertTrue(!sys.addResult("e1", "r4", "X", OT::Ship, 100.0f, -1.0f),
               "Negative bearing rejected");
    assertTrue(!sys.addResult("e1", "r5", "X", OT::Ship, 100.0f, 361.0f),
               "Bearing > 360 rejected");

    assertTrue(!sys.addResult("missing", "r9", "X", OT::Ship, 100.0f, 0.0f),
               "Add on missing fails");
}

static void testDScanCapacity() {
    std::cout << "\n=== DirectionalScan: Capacity ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxResults("e1", 3);

    using OT = components::DirectionalScanState::ObjectType;
    assertTrue(sys.addResult("e1", "r1", "A", OT::Ship, 100.0f, 0.0f), "Add r1");
    assertTrue(sys.addResult("e1", "r2", "B", OT::Ship, 200.0f, 90.0f), "Add r2");
    assertTrue(sys.addResult("e1", "r3", "C", OT::Ship, 300.0f, 180.0f), "Add r3");
    assertTrue(!sys.addResult("e1", "r4", "D", OT::Ship, 400.0f, 270.0f),
               "At capacity, rejected");
    assertTrue(sys.getResultCount("e1") == 3, "Still 3 results");
}

static void testDScanRemoveResult() {
    std::cout << "\n=== DirectionalScan: RemoveResult ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using OT = components::DirectionalScanState::ObjectType;
    sys.addResult("e1", "r1", "A", OT::Ship, 100.0f, 0.0f);
    sys.addResult("e1", "r2", "B", OT::Structure, 200.0f, 90.0f);

    assertTrue(sys.removeResult("e1", "r1"), "Remove r1");
    assertTrue(sys.getResultCount("e1") == 1, "1 result");
    assertTrue(!sys.hasResult("e1", "r1"), "r1 gone");
    assertTrue(sys.hasResult("e1", "r2"), "r2 still there");

    assertTrue(!sys.removeResult("e1", "r1"), "Remove non-existent fails");
    assertTrue(!sys.removeResult("missing", "r2"), "Remove on missing fails");
}

static void testDScanClearResults() {
    std::cout << "\n=== DirectionalScan: ClearResults ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using OT = components::DirectionalScanState::ObjectType;
    sys.addResult("e1", "r1", "A", OT::Ship, 100.0f, 0.0f);
    sys.addResult("e1", "r2", "B", OT::Drone, 200.0f, 90.0f);

    assertTrue(sys.clearResults("e1"), "Clear results");
    assertTrue(sys.getResultCount("e1") == 0, "Zero results");
    assertTrue(!sys.clearResults("missing"), "Clear on missing fails");
}

static void testDScanScanClearsOldResults() {
    std::cout << "\n=== DirectionalScan: ScanClearsResults ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setCooldownDuration("e1", 0.0f);  // no cooldown

    using OT = components::DirectionalScanState::ObjectType;
    sys.addResult("e1", "r1", "A", OT::Ship, 100.0f, 0.0f);
    assertTrue(sys.getResultCount("e1") == 1, "1 result before scan");

    assertTrue(sys.startScan("e1"), "Start scan clears old results");
    assertTrue(sys.getResultCount("e1") == 0, "Results cleared on new scan");
}

static void testDScanCountByType() {
    std::cout << "\n=== DirectionalScan: CountByType ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using OT = components::DirectionalScanState::ObjectType;
    sys.addResult("e1", "r1", "Ship1", OT::Ship, 100.0f, 0.0f);
    sys.addResult("e1", "r2", "Ship2", OT::Ship, 200.0f, 45.0f);
    sys.addResult("e1", "r3", "Station", OT::Structure, 300.0f, 90.0f);
    sys.addResult("e1", "r4", "Drone1", OT::Drone, 150.0f, 135.0f);
    sys.addResult("e1", "r5", "Wreck1", OT::Wreck, 250.0f, 180.0f);

    assertTrue(sys.getCountByType("e1", OT::Ship) == 2, "2 ships");
    assertTrue(sys.getCountByType("e1", OT::Structure) == 1, "1 structure");
    assertTrue(sys.getCountByType("e1", OT::Drone) == 1, "1 drone");
    assertTrue(sys.getCountByType("e1", OT::Wreck) == 1, "1 wreck");
    assertTrue(sys.getCountByType("e1", OT::Celestial) == 0, "0 celestials");
    assertTrue(sys.getCountByType("e1", OT::Anomaly) == 0, "0 anomalies");
    assertTrue(sys.getCountByType("missing", OT::Ship) == 0, "CountByType on missing");
}

static void testDScanMissing() {
    std::cout << "\n=== DirectionalScan: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::DirectionalScanSystem sys(&world);

    using OT = components::DirectionalScanState::ObjectType;

    assertTrue(!sys.initialize("m"), "Init fails");
    assertTrue(!sys.startScan("m"), "StartScan fails");
    assertTrue(!sys.cancelScan("m"), "CancelScan fails");
    assertTrue(!sys.setScanAngle("m", 60.0f), "SetScanAngle fails");
    assertTrue(!sys.setScanRange("m", 10.0f), "SetScanRange fails");
    assertTrue(!sys.setScanDuration("m", 5.0f), "SetScanDuration fails");
    assertTrue(!sys.setCooldownDuration("m", 3.0f), "SetCooldownDuration fails");
    assertTrue(!sys.setMaxResults("m", 10), "SetMaxResults fails");
    assertTrue(!sys.addResult("m", "r", "X", OT::Ship, 100.0f, 0.0f), "AddResult fails");
    assertTrue(!sys.removeResult("m", "r"), "RemoveResult fails");
    assertTrue(!sys.clearResults("m"), "ClearResults fails");
    assertTrue(sys.getResultCount("m") == 0, "getResultCount returns 0");
    assertTrue(!sys.hasResult("m", "r"), "hasResult returns false");
    assertTrue(approxEqual(sys.getScanAngle("m"), 0.0f), "getScanAngle returns 0");
    assertTrue(approxEqual(sys.getScanRange("m"), 0.0f), "getScanRange returns 0");
    assertTrue(approxEqual(sys.getScanProgress("m"), 0.0f), "getScanProgress returns 0");
    assertTrue(approxEqual(sys.getCooldownRemaining("m"), 0.0f), "getCooldown returns 0");
    assertTrue(!sys.isScanning("m"), "isScanning returns false");
    assertTrue(!sys.isScanComplete("m"), "isScanComplete returns false");
    assertTrue(!sys.isOnCooldown("m"), "isOnCooldown returns false");
    assertTrue(sys.getTotalScans("m") == 0, "getTotalScans returns 0");
    assertTrue(sys.getTotalObjectsFound("m") == 0, "getTotalObjectsFound returns 0");
    assertTrue(approxEqual(sys.getResultDistance("m", "r"), 0.0f), "getResultDistance returns 0");
    assertTrue(approxEqual(sys.getResultBearing("m", "r"), 0.0f), "getResultBearing returns 0");
    assertTrue(sys.getCountByType("m", OT::Ship) == 0, "getCountByType returns 0");
}

void run_directional_scan_system_tests() {
    testDScanInit();
    testDScanStartCancel();
    testDScanCompletion();
    testDScanConfiguration();
    testDScanAddResult();
    testDScanCapacity();
    testDScanRemoveResult();
    testDScanClearResults();
    testDScanScanClearsOldResults();
    testDScanCountByType();
    testDScanMissing();
}
