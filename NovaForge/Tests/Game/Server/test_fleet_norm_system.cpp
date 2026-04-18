// Tests for: FleetNormSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_norm_system.h"

using namespace atlas;

// ==================== FleetNormSystem Tests ====================

static void testFleetNormInit() {
    std::cout << "\n=== FleetNorm: Init ===" << std::endl;
    ecs::World world;
    systems::FleetNormSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getNormCount("e1") == 0, "Zero norms initially");
    assertTrue(sys.getActiveNormCount("e1") == 0, "Zero active norms");
    assertTrue(sys.getTotalNormsFormed("e1") == 0, "Zero norms formed");
    assertTrue(sys.getActivationThreshold("e1") == 5, "Default threshold = 5");
    assertTrue(sys.getFleetId("e1").empty(), "Empty fleet id initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFleetNormAddRemove() {
    std::cout << "\n=== FleetNorm: Add/Remove ===" << std::endl;
    ecs::World world;
    systems::FleetNormSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addNorm("e1", "n1", "Battle Salute", "combat_kill"),
               "Add norm n1");
    assertTrue(sys.addNorm("e1", "n2", "Mining Chant", "ore_mined"),
               "Add norm n2");
    assertTrue(sys.getNormCount("e1") == 2, "2 norms");
    assertTrue(sys.hasNorm("e1", "n1"), "Has n1");
    assertTrue(sys.hasNorm("e1", "n2"), "Has n2");
    assertTrue(!sys.hasNorm("e1", "n3"), "Does not have n3");

    // Duplicate id rejected
    assertTrue(!sys.addNorm("e1", "n1", "Other", "other_trigger"),
               "Duplicate id rejected");

    // Empty fields rejected
    assertTrue(!sys.addNorm("e1", "", "Name", "trigger"), "Empty id rejected");
    assertTrue(!sys.addNorm("e1", "n3", "", "trigger"), "Empty name rejected");
    assertTrue(!sys.addNorm("e1", "n3", "Name", ""), "Empty trigger rejected");

    assertTrue(sys.removeNorm("e1", "n1"), "Remove n1");
    assertTrue(sys.getNormCount("e1") == 1, "1 norm after remove");
    assertTrue(!sys.hasNorm("e1", "n1"), "n1 gone");
    assertTrue(!sys.removeNorm("e1", "n1"), "Remove missing fails");

    assertTrue(!sys.addNorm("missing", "n1", "Name", "trigger"),
               "Add on missing fails");
    assertTrue(!sys.removeNorm("missing", "n1"),
               "Remove on missing fails");
}

static void testFleetNormCapacity() {
    std::cout << "\n=== FleetNorm: Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetNormSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxNorms("e1", 3);

    assertTrue(sys.addNorm("e1", "n1", "N1", "t1"), "Add n1");
    assertTrue(sys.addNorm("e1", "n2", "N2", "t2"), "Add n2");
    assertTrue(sys.addNorm("e1", "n3", "N3", "t3"), "Add n3");
    assertTrue(!sys.addNorm("e1", "n4", "N4", "t4"), "n4 rejected at cap 3");
    assertTrue(sys.getNormCount("e1") == 3, "Count stays 3");
}

static void testFleetNormRecordAction() {
    std::cout << "\n=== FleetNorm: RecordAction ===" << std::endl;
    ecs::World world;
    systems::FleetNormSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setActivationThreshold("e1", 3);

    sys.addNorm("e1", "n1", "Battle Cry", "combat_kill");

    // Record 2 actions — not yet active
    sys.recordAction("e1", "combat_kill");
    sys.recordAction("e1", "combat_kill");
    assertTrue(!sys.isNormActive("e1", "n1"), "Not yet active at 2 records");
    assertTrue(sys.getNormStrength("e1", "n1") > 0.0f, "Strength > 0");
    assertTrue(sys.getNormStrength("e1", "n1") < 1.0f, "Strength < 1");
    assertTrue(sys.getTotalNormsFormed("e1") == 0, "No norms formed yet");

    // Third action activates
    sys.recordAction("e1", "combat_kill");
    assertTrue(sys.isNormActive("e1", "n1"), "Active at threshold");
    assertTrue(approxEqual(sys.getNormStrength("e1", "n1"), 1.0f),
               "Strength = 1 at threshold");
    assertTrue(sys.getTotalNormsFormed("e1") == 1, "1 norm formed");

    // Unrelated trigger has no effect on n1
    sys.recordAction("e1", "ore_mined");
    assertTrue(sys.getNormCount("e1") == 1, "Still 1 norm");

    // Empty trigger rejected
    assertTrue(!sys.recordAction("e1", ""), "Empty trigger rejected");

    assertTrue(!sys.recordAction("missing", "combat_kill"),
               "RecordAction on missing fails");
}

static void testFleetNormManualActivation() {
    std::cout << "\n=== FleetNorm: Manual Activation ===" << std::endl;
    ecs::World world;
    systems::FleetNormSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addNorm("e1", "n1", "Team Cheer", "warp_out");

    assertTrue(sys.activateNorm("e1", "n1"), "Manual activate n1");
    assertTrue(sys.isNormActive("e1", "n1"), "n1 is active");
    assertTrue(sys.getTotalNormsFormed("e1") == 1, "1 norm formed");
    assertTrue(!sys.activateNorm("e1", "n1"), "Re-activate blocked");

    assertTrue(sys.deactivateNorm("e1", "n1"), "Deactivate n1");
    assertTrue(!sys.isNormActive("e1", "n1"), "n1 not active");
    assertTrue(!sys.deactivateNorm("e1", "n1"), "Re-deactivate blocked");

    // Missing norm
    assertTrue(!sys.activateNorm("e1", "missing_norm"),
               "Activate missing norm fails");
    assertTrue(!sys.deactivateNorm("e1", "missing_norm"),
               "Deactivate missing norm fails");

    // Missing entity
    assertTrue(!sys.activateNorm("missing", "n1"),
               "Activate on missing entity fails");
    assertTrue(!sys.deactivateNorm("missing", "n1"),
               "Deactivate on missing entity fails");
}

static void testFleetNormClearNorms() {
    std::cout << "\n=== FleetNorm: ClearNorms ===" << std::endl;
    ecs::World world;
    systems::FleetNormSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addNorm("e1", "n1", "N1", "t1");
    sys.addNorm("e1", "n2", "N2", "t2");
    sys.activateNorm("e1", "n1");

    assertTrue(sys.clearNorms("e1"), "Clear succeeds");
    assertTrue(sys.getNormCount("e1") == 0, "Zero norms after clear");
    assertTrue(sys.getActiveNormCount("e1") == 0, "Zero active after clear");
    assertTrue(!sys.clearNorms("missing"), "Clear on missing fails");
}

static void testFleetNormConfiguration() {
    std::cout << "\n=== FleetNorm: Configuration ===" << std::endl;
    ecs::World world;
    systems::FleetNormSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setActivationThreshold("e1", 10), "Set threshold 10");
    assertTrue(sys.getActivationThreshold("e1") == 10, "Threshold = 10");
    assertTrue(!sys.setActivationThreshold("e1", 0), "Zero threshold rejected");
    assertTrue(!sys.setActivationThreshold("e1", -1), "Negative threshold rejected");

    assertTrue(sys.setMaxNorms("e1", 20), "Set max norms 20");
    assertTrue(!sys.setMaxNorms("e1", 0), "Zero max rejected");

    assertTrue(sys.setFleetId("e1", "fleet_alpha"), "Set fleet id");
    assertTrue(sys.getFleetId("e1") == "fleet_alpha", "Fleet id matches");
    assertTrue(!sys.setFleetId("e1", ""), "Empty fleet id rejected");

    assertTrue(!sys.setActivationThreshold("missing", 5),
               "Threshold on missing fails");
    assertTrue(!sys.setMaxNorms("missing", 10),
               "MaxNorms on missing fails");
    assertTrue(!sys.setFleetId("missing", "f"), "FleetId on missing fails");
}

static void testFleetNormCountByTrigger() {
    std::cout << "\n=== FleetNorm: CountByTrigger ===" << std::endl;
    ecs::World world;
    systems::FleetNormSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addNorm("e1", "n1", "N1", "combat_kill");
    sys.addNorm("e1", "n2", "N2", "combat_kill");
    sys.addNorm("e1", "n3", "N3", "ore_mined");

    assertTrue(sys.getCountByTrigger("e1", "combat_kill") == 2,
               "2 norms share combat_kill trigger");
    assertTrue(sys.getCountByTrigger("e1", "ore_mined") == 1,
               "1 norm has ore_mined trigger");
    assertTrue(sys.getCountByTrigger("e1", "unknown") == 0,
               "0 norms for unknown trigger");
    assertTrue(sys.getCountByTrigger("missing", "combat_kill") == 0,
               "CountByTrigger on missing returns 0");
}

static void testFleetNormMissingEntity() {
    std::cout << "\n=== FleetNorm: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetNormSystem sys(&world);

    assertTrue(sys.getNormCount("missing") == 0,
               "NormCount returns 0 for missing");
    assertTrue(sys.getActiveNormCount("missing") == 0,
               "ActiveNormCount returns 0 for missing");
    assertTrue(!sys.isNormActive("missing", "n1"),
               "isNormActive returns false for missing");
    assertTrue(approxEqual(sys.getNormStrength("missing", "n1"), 0.0f),
               "NormStrength returns 0 for missing");
    assertTrue(!sys.hasNorm("missing", "n1"),
               "hasNorm returns false for missing");
    assertTrue(sys.getTotalNormsFormed("missing") == 0,
               "TotalNormsFormed returns 0 for missing");
    assertTrue(sys.getCountByTrigger("missing", "t") == 0,
               "CountByTrigger returns 0 for missing");
    assertTrue(sys.getActivationThreshold("missing") == 0,
               "Threshold returns 0 for missing");
    assertTrue(sys.getFleetId("missing").empty(),
               "FleetId returns empty for missing");
}

void run_fleet_norm_system_tests() {
    testFleetNormInit();
    testFleetNormAddRemove();
    testFleetNormCapacity();
    testFleetNormRecordAction();
    testFleetNormManualActivation();
    testFleetNormClearNorms();
    testFleetNormConfiguration();
    testFleetNormCountByTrigger();
    testFleetNormMissingEntity();
}
