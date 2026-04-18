// Tests for: OperationalWearSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/operational_wear_system.h"

using namespace atlas;

static void testOperationalWearInit() {
    std::cout << "\n=== OperationalWear: Init ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(approxEqual(sys.getWearLevel("e1"), 0.0f), "Wear level 0 initially");
    assertTrue(approxEqual(sys.getFuelInefficiency("e1"), 0.0f), "Fuel inefficiency 0 initially");
    assertTrue(approxEqual(sys.getRepairDelayMult("e1"), 1.0f), "Repair mult 1.0 initially");
    assertTrue(approxEqual(sys.getCrewStress("e1"), 0.0f), "Crew stress 0 initially");
    assertTrue(!sys.isWorn("e1"), "Not worn initially");
    assertTrue(!sys.isCritical("e1"), "Not critical initially");
    assertTrue(!sys.hasHiddenPenalties("e1"), "No hidden penalties initially");
    assertTrue(approxEqual(sys.getDeploymentDuration("e1"), 0.0f), "Deployment 0 initially");
    assertTrue(sys.getTotalFieldRepairs("e1") == 0, "0 field repairs initially");
    assertTrue(sys.getTotalDockRepairs("e1") == 0, "0 dock repairs initially");
    assertTrue(sys.getShipId("e1").empty(), "Ship id empty initially");
    assertTrue(approxEqual(sys.getRotationThreshold("e1"), 86400.0f), "Default rotation threshold");
    assertTrue(approxEqual(sys.getPassiveWearRate("e1"), 0.005f), "Default passive wear rate");
    assertTrue(approxEqual(sys.getRecoveryRate("e1"), 5.0f), "Default recovery rate 5");
    assertTrue(sys.getMaxEvents("e1") == 20, "Default max events 20");
    assertTrue(sys.getEventCount("e1") == 0, "0 events initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testOperationalWearRecordWear() {
    std::cout << "\n=== OperationalWear: RecordWear ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Basic wear recording
    assertTrue(sys.recordWear("e1", "ev1", "Combat", 10.0f), "Record wear succeeds");
    assertTrue(approxEqual(sys.getWearLevel("e1"), 10.0f), "Wear level 10");
    assertTrue(sys.getEventCount("e1") == 1, "1 event logged");
    assertTrue(!sys.isWorn("e1"), "Not worn at 10");

    // Add more wear past worn threshold
    assertTrue(sys.recordWear("e1", "ev2", "Mining", 45.0f), "Record wear 45 succeeds");
    assertTrue(approxEqual(sys.getWearLevel("e1"), 55.0f), "Wear level 55");
    assertTrue(sys.isWorn("e1"), "Is worn at 55");
    assertTrue(!sys.isCritical("e1"), "Not critical at 55");

    // Push to critical
    assertTrue(sys.recordWear("e1", "ev3", "Transit", 35.0f), "Record wear 35 succeeds");
    assertTrue(approxEqual(sys.getWearLevel("e1"), 90.0f), "Wear level 90");
    assertTrue(sys.isCritical("e1"), "Is critical at 90");

    // Clamped at 100
    assertTrue(sys.recordWear("e1", "ev4", "Combat", 50.0f), "Record wear 50 succeeds");
    assertTrue(approxEqual(sys.getWearLevel("e1"), 100.0f), "Wear clamped at 100");

    // Invalid: empty event id
    assertTrue(!sys.recordWear("e1", "", "Combat", 5.0f), "Empty event_id rejected");

    // Invalid: zero amount
    assertTrue(!sys.recordWear("e1", "ev5", "Idle", 0.0f), "Zero amount rejected");

    // Invalid: negative amount
    assertTrue(!sys.recordWear("e1", "ev5", "Idle", -5.0f), "Negative amount rejected");

    // Missing entity
    assertTrue(!sys.recordWear("missing", "ev1", "Combat", 5.0f), "Missing entity fails");
}

static void testOperationalWearFieldRepair() {
    std::cout << "\n=== OperationalWear: FieldRepair ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.recordWear("e1", "ev1", "Combat", 80.0f);
    assertTrue(approxEqual(sys.getWearLevel("e1"), 80.0f), "Wear at 80 before repair");
    assertTrue(!sys.hasHiddenPenalties("e1"), "No hidden penalties before field repair");

    // Field repair reduces wear and sets hidden penalties
    assertTrue(sys.fieldRepair("e1", 30.0f), "Field repair succeeds");
    assertTrue(approxEqual(sys.getWearLevel("e1"), 50.0f), "Wear at 50 after field repair");
    assertTrue(sys.hasHiddenPenalties("e1"), "Has hidden penalties after field repair");
    assertTrue(sys.getTotalFieldRepairs("e1") == 1, "1 field repair counted");

    // Another field repair
    assertTrue(sys.fieldRepair("e1", 20.0f), "Second field repair succeeds");
    assertTrue(approxEqual(sys.getWearLevel("e1"), 30.0f), "Wear at 30");
    assertTrue(sys.getTotalFieldRepairs("e1") == 2, "2 field repairs counted");

    // Field repair clamped to 0, not negative
    assertTrue(sys.fieldRepair("e1", 100.0f), "Field repair beyond 0 succeeds (clamps)");
    assertTrue(approxEqual(sys.getWearLevel("e1"), 0.0f), "Wear clamped to 0");
    assertTrue(sys.hasHiddenPenalties("e1"), "Hidden penalties persist after field repair");
    assertTrue(sys.getTotalFieldRepairs("e1") == 3, "3 field repairs counted");

    // Invalid: zero amount
    assertTrue(!sys.fieldRepair("e1", 0.0f), "Zero amount rejected");

    // Invalid: negative amount
    assertTrue(!sys.fieldRepair("e1", -5.0f), "Negative amount rejected");

    // Missing entity
    assertTrue(!sys.fieldRepair("missing", 10.0f), "Missing entity fails");
}

static void testOperationalWearDockRepair() {
    std::cout << "\n=== OperationalWear: DockRepair ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.recordWear("e1", "ev1", "Combat", 90.0f);
    sys.fieldRepair("e1", 4.0f);
    assertTrue(approxEqual(sys.getWearLevel("e1"), 86.0f), "Wear at 86 before dock");
    assertTrue(sys.hasHiddenPenalties("e1"), "Has hidden penalties before dock");
    assertTrue(sys.isCritical("e1"), "Critical before dock");

    // Dock repair clears everything
    assertTrue(sys.dockRepair("e1"), "Dock repair succeeds");
    assertTrue(approxEqual(sys.getWearLevel("e1"), 0.0f), "Wear 0 after dock");
    assertTrue(!sys.hasHiddenPenalties("e1"), "No hidden penalties after dock");
    assertTrue(!sys.isWorn("e1"), "Not worn after dock");
    assertTrue(!sys.isCritical("e1"), "Not critical after dock");
    assertTrue(approxEqual(sys.getFuelInefficiency("e1"), 0.0f), "Fuel inefficiency 0 after dock");
    assertTrue(approxEqual(sys.getRepairDelayMult("e1"), 1.0f), "Repair mult 1.0 after dock");
    assertTrue(approxEqual(sys.getCrewStress("e1"), 0.0f), "Crew stress 0 after dock");
    assertTrue(sys.getTotalDockRepairs("e1") == 1, "1 dock repair counted");

    // Multiple dock repairs
    sys.recordWear("e1", "ev2", "Transit", 60.0f);
    assertTrue(sys.dockRepair("e1"), "Second dock repair succeeds");
    assertTrue(sys.getTotalDockRepairs("e1") == 2, "2 dock repairs counted");

    // Missing entity
    assertTrue(!sys.dockRepair("missing"), "Missing entity fails");
}

static void testOperationalWearPassiveWear() {
    std::cout << "\n=== OperationalWear: PassiveWear ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Set low rotation threshold for testing
    sys.setRotationThreshold("e1", 10.0f);  // 10 seconds
    sys.setPassiveWearRate("e1", 1.0f);     // 1 wear/second past threshold

    // Deploy for 5 seconds - below threshold, no passive wear
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getWearLevel("e1"), 0.0f), "No wear before threshold");
    assertTrue(approxEqual(sys.getDeploymentDuration("e1"), 5.0f, 0.1f),
               "Deployment 5s after 5s");

    // Deploy another 10s - past threshold, wear should accumulate
    sys.update(10.0f);
    float wear = sys.getWearLevel("e1");
    assertTrue(wear > 0.0f, "Passive wear accumulated past threshold");
    assertTrue(approxEqual(sys.getDeploymentDuration("e1"), 15.0f, 0.1f),
               "Deployment 15s total");

    // Missing entity
    assertTrue(approxEqual(sys.getDeploymentDuration("missing"), 0.0f),
               "Deployment duration missing returns 0");
}

static void testOperationalWearDockRecovery() {
    std::cout << "\n=== OperationalWear: DockRecovery ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.recordWear("e1", "ev1", "Combat", 60.0f);
    sys.setRecoveryRate("e1", 10.0f);  // 10 wear/second recovery
    sys.setDocked("e1", true);

    // Recover for 3 seconds
    sys.update(3.0f);
    float wear = sys.getWearLevel("e1");
    assertTrue(wear < 60.0f, "Wear reduced while docked");
    assertTrue(approxEqual(wear, 30.0f, 0.5f), "Wear ~30 after 3s at 10/s recovery");

    // Recover fully
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getWearLevel("e1"), 0.0f, 0.1f), "Wear recovers to 0");
    assertTrue(!sys.isWorn("e1"), "Not worn after recovery");
}

static void testOperationalWearDerivedValues() {
    std::cout << "\n=== OperationalWear: DerivedValues ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // At 0 wear: fuel_inefficiency=0, repair_mult=1.0, crew_stress=0
    assertTrue(approxEqual(sys.getFuelInefficiency("e1"), 0.0f), "Ineff 0 at 0 wear");
    assertTrue(approxEqual(sys.getRepairDelayMult("e1"), 1.0f), "Mult 1.0 at 0 wear");
    assertTrue(approxEqual(sys.getCrewStress("e1"), 0.0f), "Stress 0 at 0 wear");

    sys.recordWear("e1", "ev1", "Combat", 100.0f);

    // At 100 wear: fuel_inefficiency=0.5, repair_mult=2.0, crew_stress=100
    sys.update(0.001f); // trigger update to recompute
    assertTrue(approxEqual(sys.getFuelInefficiency("e1"), 0.5f, 0.01f), "Ineff 0.5 at 100 wear");
    assertTrue(approxEqual(sys.getRepairDelayMult("e1"), 2.0f, 0.01f), "Mult 2.0 at 100 wear");
    assertTrue(approxEqual(sys.getCrewStress("e1"), 100.0f, 0.1f), "Stress 100 at 100 wear");
}

static void testOperationalWearEventLog() {
    std::cout << "\n=== OperationalWear: EventLog ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.setMaxEvents("e1", 3);

    sys.recordWear("e1", "ev1", "Combat", 5.0f);
    sys.recordWear("e1", "ev2", "Mining", 5.0f);
    sys.recordWear("e1", "ev3", "Transit", 5.0f);
    assertTrue(sys.getEventCount("e1") == 3, "3 events at max");

    // Adding another should auto-purge oldest
    sys.recordWear("e1", "ev4", "Combat", 5.0f);
    assertTrue(sys.getEventCount("e1") == 3, "Still 3 events after auto-purge");

    // Invalid setMaxEvents
    assertTrue(!sys.setMaxEvents("e1", 0), "setMaxEvents 0 fails");
    assertTrue(!sys.setMaxEvents("e1", -1), "setMaxEvents -1 fails");

    // Missing entity
    assertTrue(sys.getEventCount("missing") == 0, "Event count missing returns 0");
}

static void testOperationalWearConfiguration() {
    std::cout << "\n=== OperationalWear: Configuration ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // setShipId valid
    assertTrue(sys.setShipId("e1", "Chimera-1"), "setShipId succeeds");
    assertTrue(sys.getShipId("e1") == "Chimera-1", "Ship id set");

    // setShipId empty rejected
    assertTrue(!sys.setShipId("e1", ""), "setShipId empty fails");

    // setRotationThreshold valid
    assertTrue(sys.setRotationThreshold("e1", 7200.0f), "setRotationThreshold succeeds");
    assertTrue(approxEqual(sys.getRotationThreshold("e1"), 7200.0f), "Threshold 7200");
    assertTrue(sys.setRotationThreshold("e1", 0.0f), "setRotationThreshold 0 valid");

    // setRotationThreshold negative rejected
    assertTrue(!sys.setRotationThreshold("e1", -1.0f), "Negative threshold rejected");

    // setPassiveWearRate valid
    assertTrue(sys.setPassiveWearRate("e1", 0.01f), "setPassiveWearRate succeeds");
    assertTrue(approxEqual(sys.getPassiveWearRate("e1"), 0.01f), "Passive rate 0.01");
    assertTrue(sys.setPassiveWearRate("e1", 0.0f), "setPassiveWearRate 0 valid");
    assertTrue(!sys.setPassiveWearRate("e1", -0.1f), "Negative passive rate rejected");

    // setRecoveryRate valid
    assertTrue(sys.setRecoveryRate("e1", 2.0f), "setRecoveryRate succeeds");
    assertTrue(approxEqual(sys.getRecoveryRate("e1"), 2.0f), "Recovery rate 2");
    assertTrue(sys.setRecoveryRate("e1", 0.0f), "setRecoveryRate 0 valid");
    assertTrue(!sys.setRecoveryRate("e1", -1.0f), "Negative recovery rate rejected");

    // setDocked valid
    assertTrue(sys.setDocked("e1", true), "setDocked true succeeds");
    assertTrue(sys.setDocked("e1", false), "setDocked false succeeds");

    // Missing entity
    assertTrue(!sys.setShipId("missing", "x"), "setShipId missing fails");
    assertTrue(!sys.setRotationThreshold("missing", 100.0f), "setRotationThreshold missing fails");
    assertTrue(!sys.setPassiveWearRate("missing", 0.1f), "setPassiveWearRate missing fails");
    assertTrue(!sys.setRecoveryRate("missing", 1.0f), "setRecoveryRate missing fails");
    assertTrue(!sys.setDocked("missing", true), "setDocked missing fails");
}

static void testOperationalWearMissingEntity() {
    std::cout << "\n=== OperationalWear: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);

    assertTrue(approxEqual(sys.getWearLevel("missing"), 0.0f), "getWearLevel missing returns 0");
    assertTrue(approxEqual(sys.getFuelInefficiency("missing"), 0.0f), "getFuelInefficiency missing returns 0");
    assertTrue(approxEqual(sys.getRepairDelayMult("missing"), 1.0f), "getRepairDelayMult missing returns 1.0");
    assertTrue(approxEqual(sys.getCrewStress("missing"), 0.0f), "getCrewStress missing returns 0");
    assertTrue(!sys.isWorn("missing"), "isWorn missing returns false");
    assertTrue(!sys.isCritical("missing"), "isCritical missing returns false");
    assertTrue(!sys.hasHiddenPenalties("missing"), "hasHiddenPenalties missing returns false");
    assertTrue(approxEqual(sys.getDeploymentDuration("missing"), 0.0f), "getDeploymentDuration missing returns 0");
    assertTrue(sys.getTotalFieldRepairs("missing") == 0, "getTotalFieldRepairs missing returns 0");
    assertTrue(sys.getTotalDockRepairs("missing") == 0, "getTotalDockRepairs missing returns 0");
    assertTrue(sys.getShipId("missing").empty(), "getShipId missing returns empty");
    assertTrue(approxEqual(sys.getRotationThreshold("missing"), 0.0f), "getRotationThreshold missing returns 0");
    assertTrue(approxEqual(sys.getPassiveWearRate("missing"), 0.0f), "getPassiveWearRate missing returns 0");
    assertTrue(approxEqual(sys.getRecoveryRate("missing"), 0.0f), "getRecoveryRate missing returns 0");
    assertTrue(sys.getMaxEvents("missing") == 0, "getMaxEvents missing returns 0");
    assertTrue(sys.getEventCount("missing") == 0, "getEventCount missing returns 0");
    assertTrue(!sys.recordWear("missing", "x", "y", 5.0f), "recordWear missing fails");
    assertTrue(!sys.fieldRepair("missing", 10.0f), "fieldRepair missing fails");
    assertTrue(!sys.dockRepair("missing"), "dockRepair missing fails");
}

static void testOperationalWearWornThresholds() {
    std::cout << "\n=== OperationalWear: WornThresholds ===" << std::endl;
    ecs::World world;
    systems::OperationalWearSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Exactly at 49.9: not worn
    sys.recordWear("e1", "ev1", "Combat", 49.9f);
    assertTrue(!sys.isWorn("e1"), "Not worn at 49.9");

    // Exactly at 50: worn
    sys.recordWear("e1", "ev2", "Combat", 0.1f);
    assertTrue(sys.isWorn("e1"), "Worn at exactly 50");
    assertTrue(!sys.isCritical("e1"), "Not critical at 50");

    // Exactly at 84.9: worn but not critical
    sys.dockRepair("e1");
    sys.recordWear("e1", "ev3", "Combat", 84.9f);
    assertTrue(sys.isWorn("e1"), "Worn at 84.9");
    assertTrue(!sys.isCritical("e1"), "Not critical at 84.9");

    // Exactly at 85: critical
    sys.recordWear("e1", "ev4", "Combat", 0.1f);
    assertTrue(sys.isCritical("e1"), "Critical at exactly 85");
}

void run_operational_wear_system_tests() {
    testOperationalWearInit();
    testOperationalWearRecordWear();
    testOperationalWearFieldRepair();
    testOperationalWearDockRepair();
    testOperationalWearPassiveWear();
    testOperationalWearDockRecovery();
    testOperationalWearDerivedValues();
    testOperationalWearEventLog();
    testOperationalWearConfiguration();
    testOperationalWearMissingEntity();
    testOperationalWearWornThresholds();
}
