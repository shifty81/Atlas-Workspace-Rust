// Tests for: SimSensorConfidenceSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/sim_sensor_confidence_system.h"

using namespace atlas;

static void testSensorConfidenceInit() {
    std::cout << "\n=== SensorConfidence: Init ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getEntryCount("e1") == 0, "No entries initially");
    assertTrue(sys.getActiveEntryCount("e1") == 0, "No active entries initially");
    assertTrue(sys.getTotalEntriesRecorded("e1") == 0, "Zero recorded total");
    assertTrue(sys.getTotalHighConfidence("e1") == 0, "Zero high confidence total");
    assertTrue(approxEqual(sys.getDecayRate("e1"), 0.02f), "Default decay rate 0.02");
    assertTrue(sys.getMaxEntries("e1") == 50, "Default max entries 50");
    assertTrue(sys.getScannerID("e1").empty(), "Default scanner id empty");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testSensorConfidenceAddEntry() {
    std::cout << "\n=== SensorConfidence: AddEntry ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Happy path
    assertTrue(sys.addEntry("e1", "entry1", "target1", "Unknown", 0.8f, 0.0f, 5.0f),
               "Add entry1 succeeds");
    assertTrue(sys.getEntryCount("e1") == 1, "1 entry after add");
    assertTrue(sys.hasEntry("e1", "entry1"), "entry1 exists");
    assertTrue(approxEqual(sys.getConfidence("e1", "entry1"), 0.8f), "Confidence 0.8");
    assertTrue(approxEqual(sys.getDistanceMin("e1", "entry1"), 0.0f), "dist_min 0");
    assertTrue(approxEqual(sys.getDistanceMax("e1", "entry1"), 5.0f), "dist_max 5");
    assertTrue(sys.getShipClassEstimate("e1", "entry1") == "Unknown", "Class estimate");
    assertTrue(sys.getTotalEntriesRecorded("e1") == 1, "1 recorded total");

    // Duplicate entry_id rejected
    assertTrue(!sys.addEntry("e1", "entry1", "target2", "Miner", 0.5f, 1.0f, 3.0f),
               "Duplicate entry_id rejected");
    assertTrue(sys.getEntryCount("e1") == 1, "Still 1 entry after duplicate");

    // Empty entry_id rejected
    assertTrue(!sys.addEntry("e1", "", "target2", "Miner", 0.5f, 1.0f, 3.0f),
               "Empty entry_id rejected");

    // Empty target_id rejected
    assertTrue(!sys.addEntry("e1", "entry2", "", "Miner", 0.5f, 1.0f, 3.0f),
               "Empty target_id rejected");

    // confidence > 1.0 rejected
    assertTrue(!sys.addEntry("e1", "entry2", "t2", "Miner", 1.5f, 1.0f, 3.0f),
               "Confidence > 1 rejected");

    // confidence < 0 rejected
    assertTrue(!sys.addEntry("e1", "entry2", "t2", "Miner", -0.1f, 1.0f, 3.0f),
               "Confidence < 0 rejected");

    // Capacity: set max to 2, fill, then reject
    sys.setMaxEntries("e1", 2);
    assertTrue(sys.addEntry("e1", "entry2", "t2", "Miner", 0.5f, 1.0f, 3.0f),
               "Add entry2 within capacity");
    assertTrue(!sys.addEntry("e1", "entry3", "t3", "Fighter", 0.5f, 1.0f, 3.0f),
               "Add rejected when at capacity");

    // Missing entity
    assertTrue(!sys.addEntry("missing", "x", "y", "z", 0.5f, 0.0f, 1.0f),
               "Add on missing entity fails");
}

static void testSensorConfidenceRemoveEntry() {
    std::cout << "\n=== SensorConfidence: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addEntry("e1", "e1", "t1", "Ship", 0.7f, 0.0f, 2.0f);
    sys.addEntry("e1", "e2", "t2", "Ship", 0.6f, 1.0f, 3.0f);
    assertTrue(sys.getEntryCount("e1") == 2, "2 entries");

    assertTrue(sys.removeEntry("e1", "e1"), "Remove e1 succeeds");
    assertTrue(!sys.hasEntry("e1", "e1"), "e1 gone");
    assertTrue(sys.getEntryCount("e1") == 1, "1 entry remaining");

    // Remove non-existent
    assertTrue(!sys.removeEntry("e1", "nothere"), "Remove missing entry fails");

    // Remove on missing entity
    assertTrue(!sys.removeEntry("missing", "e2"), "Remove on missing entity fails");
}

static void testSensorConfidenceClearEntries() {
    std::cout << "\n=== SensorConfidence: ClearEntries ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addEntry("e1", "e1", "t1", "Ship", 0.7f, 0.0f, 1.0f);
    sys.addEntry("e1", "e2", "t2", "Ship", 0.6f, 0.0f, 1.0f);
    sys.addEntry("e1", "e3", "t3", "Ship", 0.5f, 0.0f, 1.0f);
    assertTrue(sys.getEntryCount("e1") == 3, "3 entries before clear");

    assertTrue(sys.clearEntries("e1"), "Clear succeeds");
    assertTrue(sys.getEntryCount("e1") == 0, "0 entries after clear");
    assertTrue(sys.getActiveEntryCount("e1") == 0, "0 active after clear");

    // Missing entity
    assertTrue(!sys.clearEntries("missing"), "clearEntries on missing fails");
}

static void testSensorConfidenceDecay() {
    std::cout << "\n=== SensorConfidence: Decay ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setDecayRate("e1", 0.1f);

    sys.addEntry("e1", "e1", "t1", "Ship", 0.5f, 0.0f, 1.0f);
    assertTrue(approxEqual(sys.getEntryAge("e1", "e1"), 0.0f), "Age 0 initially");
    assertTrue(!sys.isDecayed("e1", "e1"), "Not decayed initially");

    // Advance 1 second: confidence 0.5 - 0.1 = 0.4, not decayed
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getConfidence("e1", "e1"), 0.4f, 0.01f), "Confidence 0.4 after 1s");
    assertTrue(approxEqual(sys.getEntryAge("e1", "e1"), 1.0f, 0.01f), "Age 1s after update");
    assertTrue(!sys.isDecayed("e1", "e1"), "Not decayed at 0.4");

    // Advance 4 more seconds: confidence near 0, should be decayed
    sys.update(4.0f);
    assertTrue(sys.getConfidence("e1", "e1") < 0.05f, "Confidence below 0.05 after decay");
    assertTrue(sys.isDecayed("e1", "e1"), "Entry marked decayed");
    assertTrue(sys.getActiveEntryCount("e1") == 0, "No active entries after decay");
}

static void testSensorConfidenceRefreshEntry() {
    std::cout << "\n=== SensorConfidence: RefreshEntry ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setDecayRate("e1", 0.1f);

    sys.addEntry("e1", "e1", "t1", "Ship", 0.3f, 0.0f, 1.0f);
    // Decay to near zero
    sys.update(3.0f);
    assertTrue(sys.isDecayed("e1", "e1"), "Entry decayed before refresh");

    // Refresh
    assertTrue(sys.refreshEntry("e1", "e1", 0.95f), "Refresh succeeds");
    assertTrue(approxEqual(sys.getConfidence("e1", "e1"), 0.95f), "Confidence 0.95 after refresh");
    assertTrue(approxEqual(sys.getEntryAge("e1", "e1"), 0.0f), "Age reset to 0");
    assertTrue(!sys.isDecayed("e1", "e1"), "Not decayed after refresh");
    assertTrue(sys.getActiveEntryCount("e1") == 1, "Active count 1 after refresh");

    // Invalid confidence
    assertTrue(!sys.refreshEntry("e1", "e1", 1.5f), "Refresh confidence > 1 rejected");
    assertTrue(!sys.refreshEntry("e1", "e1", -0.1f), "Refresh confidence < 0 rejected");

    // Missing entry
    assertTrue(!sys.refreshEntry("e1", "nothere", 0.5f), "Refresh missing entry fails");

    // Missing entity
    assertTrue(!sys.refreshEntry("missing", "e1", 0.5f), "Refresh missing entity fails");
}

static void testSensorConfidenceActiveCount() {
    std::cout << "\n=== SensorConfidence: ActiveCount ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setDecayRate("e1", 0.5f);

    sys.addEntry("e1", "e1", "t1", "Ship", 0.9f, 0.0f, 1.0f);
    sys.addEntry("e1", "e2", "t2", "Ship", 0.1f, 0.0f, 1.0f);
    sys.addEntry("e1", "e3", "t3", "Ship", 0.8f, 0.0f, 1.0f);

    assertTrue(sys.getActiveEntryCount("e1") == 3, "3 active before any decay");

    // Advance enough to decay e2 (0.1 - 0.5*dt)
    sys.update(0.5f);
    // e2 was 0.1, now 0.1 - 0.25 = -0.15 -> clamped 0.0 -> decayed
    assertTrue(sys.isDecayed("e1", "e2"), "e2 decayed");
    int active = sys.getActiveEntryCount("e1");
    assertTrue(active == 2, "2 active entries remaining");

    // Missing entity
    assertTrue(sys.getActiveEntryCount("missing") == 0, "Active count missing returns 0");
}

static void testSensorConfidenceHighConfidence() {
    std::cout << "\n=== SensorConfidence: HighConfidence ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Add entries with varying confidence
    sys.addEntry("e1", "e1", "t1", "Ship", 0.85f, 0.0f, 1.0f); // not high confidence
    assertTrue(sys.getTotalHighConfidence("e1") == 0, "0 high confidence entries");

    sys.addEntry("e1", "e2", "t2", "Ship", 0.9f, 0.0f, 1.0f); // exactly 0.9
    assertTrue(sys.getTotalHighConfidence("e1") == 1, "1 high confidence entry");

    sys.addEntry("e1", "e3", "t3", "Ship", 1.0f, 0.0f, 1.0f); // 1.0
    assertTrue(sys.getTotalHighConfidence("e1") == 2, "2 high confidence entries");

    sys.addEntry("e1", "e4", "t4", "Ship", 0.95f, 0.0f, 1.0f); // 0.95
    assertTrue(sys.getTotalHighConfidence("e1") == 3, "3 high confidence entries");

    // Total recorded should match number of added entries
    assertTrue(sys.getTotalEntriesRecorded("e1") == 4, "4 total recorded");

    // Decay/remove doesn't change total_high_confidence (it's a lifetime counter)
    sys.removeEntry("e1", "e2");
    assertTrue(sys.getTotalHighConfidence("e1") == 3, "High confidence unchanged after remove");
}

static void testSensorConfidenceConfiguration() {
    std::cout << "\n=== SensorConfidence: Configuration ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // setDecayRate valid
    assertTrue(sys.setDecayRate("e1", 0.0f), "setDecayRate 0 succeeds (no decay)");
    assertTrue(approxEqual(sys.getDecayRate("e1"), 0.0f), "Decay rate is 0");
    assertTrue(sys.setDecayRate("e1", 0.5f), "setDecayRate 0.5 succeeds");
    assertTrue(approxEqual(sys.getDecayRate("e1"), 0.5f), "Decay rate is 0.5");

    // setDecayRate invalid
    assertTrue(!sys.setDecayRate("e1", -0.1f), "setDecayRate negative fails");

    // setMaxEntries valid
    assertTrue(sys.setMaxEntries("e1", 1), "setMaxEntries 1 succeeds");
    assertTrue(sys.getMaxEntries("e1") == 1, "Max is 1");
    assertTrue(sys.setMaxEntries("e1", 100), "setMaxEntries 100 succeeds");
    assertTrue(sys.getMaxEntries("e1") == 100, "Max is 100");

    // setMaxEntries invalid
    assertTrue(!sys.setMaxEntries("e1", 0), "setMaxEntries 0 fails");
    assertTrue(!sys.setMaxEntries("e1", -5), "setMaxEntries -5 fails");

    // setScannerID valid
    assertTrue(sys.setScannerID("e1", "scanner_alpha"), "setScannerID succeeds");
    assertTrue(sys.getScannerID("e1") == "scanner_alpha", "Scanner id set");

    // setScannerID empty rejected
    assertTrue(!sys.setScannerID("e1", ""), "setScannerID empty fails");

    // Missing entity
    assertTrue(!sys.setDecayRate("missing", 0.1f), "setDecayRate missing fails");
    assertTrue(!sys.setMaxEntries("missing", 10), "setMaxEntries missing fails");
    assertTrue(!sys.setScannerID("missing", "s"), "setScannerID missing fails");
}

static void testSensorConfidenceMissingEntity() {
    std::cout << "\n=== SensorConfidence: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);

    assertTrue(sys.getEntryCount("missing") == 0, "getEntryCount missing returns 0");
    assertTrue(!sys.hasEntry("missing", "e1"), "hasEntry missing returns false");
    assertTrue(approxEqual(sys.getConfidence("missing", "e1"), 0.0f), "getConfidence missing returns 0");
    assertTrue(approxEqual(sys.getEntryAge("missing", "e1"), 0.0f), "getEntryAge missing returns 0");
    assertTrue(!sys.isDecayed("missing", "e1"), "isDecayed missing returns false");
    assertTrue(sys.getShipClassEstimate("missing", "e1").empty(), "getShipClass missing returns empty");
    assertTrue(approxEqual(sys.getDistanceMin("missing", "e1"), 0.0f), "getDistanceMin missing returns 0");
    assertTrue(approxEqual(sys.getDistanceMax("missing", "e1"), 0.0f), "getDistanceMax missing returns 0");
    assertTrue(sys.getActiveEntryCount("missing") == 0, "getActiveEntryCount missing returns 0");
    assertTrue(sys.getTotalEntriesRecorded("missing") == 0, "getTotalEntriesRecorded missing returns 0");
    assertTrue(sys.getTotalHighConfidence("missing") == 0, "getTotalHighConfidence missing returns 0");
    assertTrue(sys.getScannerID("missing").empty(), "getScannerID missing returns empty");
    assertTrue(approxEqual(sys.getDecayRate("missing"), 0.0f), "getDecayRate missing returns 0");
    assertTrue(sys.getMaxEntries("missing") == 0, "getMaxEntries missing returns 0");
    assertTrue(!sys.removeEntry("missing", "x"), "removeEntry missing returns false");
    assertTrue(!sys.clearEntries("missing"), "clearEntries missing returns false");
    assertTrue(!sys.refreshEntry("missing", "x", 0.5f), "refreshEntry missing returns false");
}

static void testSensorConfidenceEdgeCases() {
    std::cout << "\n=== SensorConfidence: EdgeCases ===" << std::endl;
    ecs::World world;
    systems::SimSensorConfidenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // dist_min > dist_max rejected
    assertTrue(!sys.addEntry("e1", "bad1", "t1", "Ship", 0.5f, 5.0f, 2.0f),
               "dist_min > dist_max rejected");

    // Negative dist_min rejected
    assertTrue(!sys.addEntry("e1", "bad2", "t1", "Ship", 0.5f, -1.0f, 2.0f),
               "Negative dist_min rejected");

    // Negative dist_max rejected
    assertTrue(!sys.addEntry("e1", "bad3", "t1", "Ship", 0.5f, 0.0f, -1.0f),
               "Negative dist_max rejected");

    // Zero confidence is valid (observed but not sure)
    assertTrue(sys.addEntry("e1", "zero", "t1", "Ship", 0.0f, 0.0f, 1.0f),
               "Zero confidence valid");
    assertTrue(approxEqual(sys.getConfidence("e1", "zero"), 0.0f), "Confidence is 0");

    // Confidence > 1.0 rejected
    assertTrue(!sys.addEntry("e1", "over", "t1", "Ship", 1.01f, 0.0f, 1.0f),
               "Confidence 1.01 rejected");

    // dist_min == dist_max is valid (exact distance)
    assertTrue(sys.addEntry("e1", "exact", "t2", "Ship", 0.5f, 3.0f, 3.0f),
               "Equal dist_min == dist_max valid");
    assertTrue(approxEqual(sys.getDistanceMin("e1", "exact"), 3.0f), "dist_min 3.0");
    assertTrue(approxEqual(sys.getDistanceMax("e1", "exact"), 3.0f), "dist_max 3.0");

    // Zero decay rate — confidence should not change
    sys.setDecayRate("e1", 0.0f);
    sys.addEntry("e1", "stable", "t3", "Ship", 0.8f, 0.0f, 1.0f);
    sys.update(100.0f);
    assertTrue(approxEqual(sys.getConfidence("e1", "stable"), 0.8f), "Confidence stable with 0 decay");
    assertTrue(!sys.isDecayed("e1", "stable"), "Not decayed with 0 decay rate");
}

void run_sim_sensor_confidence_system_tests() {
    testSensorConfidenceInit();
    testSensorConfidenceAddEntry();
    testSensorConfidenceRemoveEntry();
    testSensorConfidenceClearEntries();
    testSensorConfidenceDecay();
    testSensorConfidenceRefreshEntry();
    testSensorConfidenceActiveCount();
    testSensorConfidenceHighConfidence();
    testSensorConfidenceConfiguration();
    testSensorConfidenceMissingEntity();
    testSensorConfidenceEdgeCases();
}
