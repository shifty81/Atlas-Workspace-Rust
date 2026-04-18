// Tests for: FleetCultureSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/fleet_culture_system.h"

using namespace atlas;
using CT = components::CultureElementType;

static void testFleetCultureInit() {
    std::cout << "\n=== FleetCulture: Init ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getElementCount("e1") == 0, "No elements initially");
    assertTrue(sys.getActiveElementCount("e1") == 0, "No active elements");
    assertTrue(approxEqual(sys.getCohesionBonus("e1"), 0.0f), "Cohesion bonus 0");
    assertTrue(approxEqual(sys.getTensionLevel("e1"), 0.0f), "No tension");
    assertTrue(!sys.isHighTension("e1"), "Not high tension");
    assertTrue(sys.getTotalElementsFormed("e1") == 0, "No elements formed");
    assertTrue(sys.getTotalReinforcements("e1") == 0, "No reinforcements");
    assertTrue(sys.getTotalViolations("e1") == 0, "No violations");
    assertTrue(sys.getFleetId("e1").empty(), "Fleet id empty");
    assertTrue(sys.getMaxElements("e1") == 15, "Default max elements 15");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFleetCultureAddElement() {
    std::cout << "\n=== FleetCulture: AddElement ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addElement("e1", "trad1", "We Always Salvage", CT::Tradition,
                              "The fleet never leaves wrecks behind."), "Add tradition");
    assertTrue(sys.getElementCount("e1") == 1, "One element");
    assertTrue(sys.hasElement("e1", "trad1"), "Has trad1");
    assertTrue(sys.isElementActive("e1", "trad1"), "Element is active");
    assertTrue(sys.getElementName("e1", "trad1") == "We Always Salvage", "Name correct");
    assertTrue(sys.getElementType("e1", "trad1") == CT::Tradition, "Type is Tradition");
    assertTrue(approxEqual(sys.getElementStrength("e1", "trad1"), 0.0f), "Strength starts 0");
    assertTrue(sys.getTotalElementsFormed("e1") == 1, "Total formed = 1");

    // Add different types
    assertTrue(sys.addElement("e1", "tab1", "No Retreat", CT::Taboo,
                              "We do not retreat without orders."), "Add taboo");
    assertTrue(sys.addElement("e1", "mot1", "Ever Forward", CT::Motto, ""), "Add motto");
    assertTrue(sys.addElement("e1", "rit1", "Pre-battle Silence", CT::Ritual, ""), "Add ritual");
    assertTrue(sys.getElementCount("e1") == 4, "Four elements");
    assertTrue(sys.getCountByType("e1", CT::Tradition) == 1, "One tradition");
    assertTrue(sys.getCountByType("e1", CT::Taboo) == 1, "One taboo");
    assertTrue(sys.getCountByType("e1", CT::Motto) == 1, "One motto");
    assertTrue(sys.getCountByType("e1", CT::Ritual) == 1, "One ritual");

    // Duplicate id rejected
    assertTrue(!sys.addElement("e1", "trad1", "Other", CT::Tradition, ""), "Duplicate rejected");

    // Empty id / name rejected
    assertTrue(!sys.addElement("e1", "", "Name", CT::Tradition, ""), "Empty id rejected");
    assertTrue(!sys.addElement("e1", "x", "", CT::Tradition, ""), "Empty name rejected");

    assertTrue(!sys.addElement("missing", "x", "y", CT::Tradition, ""), "Missing entity rejected");
}

static void testFleetCultureReinforce() {
    std::cout << "\n=== FleetCulture: Reinforce ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addElement("e1", "trad1", "We Always Salvage", CT::Tradition, "");

    assertTrue(sys.reinforce("e1", "trad1"), "Reinforce succeeds");
    assertTrue(sys.getReinforcementCount("e1", "trad1") == 1, "Reinforcement count = 1");
    assertTrue(approxEqual(sys.getElementStrength("e1", "trad1"), 0.1f), "Strength = 0.1");
    assertTrue(sys.getTotalReinforcements("e1") == 1, "Total reinforcements = 1");
    assertTrue(sys.getCohesionBonus("e1") > 0.0f, "Cohesion bonus > 0");

    // Reinforce more
    for (int i = 0; i < 9; ++i) sys.reinforce("e1", "trad1");
    assertTrue(sys.getReinforcementCount("e1", "trad1") == 10, "Reinforcements = 10");
    assertTrue(approxEqual(sys.getElementStrength("e1", "trad1"), 1.0f), "Strength capped at 1.0");

    // Non-existent element fails
    assertTrue(!sys.reinforce("e1", "nonexistent"), "Nonexistent element fails");
    assertTrue(!sys.reinforce("missing", "trad1"), "Missing entity fails");
}

static void testFleetCultureViolate() {
    std::cout << "\n=== FleetCulture: Violate ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addElement("e1", "tab1", "No Retreat", CT::Taboo, "");
    // First give it some strength
    for (int i = 0; i < 5; ++i) sys.reinforce("e1", "tab1"); // strength = 0.5

    assertTrue(sys.violate("e1", "tab1"), "Violate succeeds");
    assertTrue(sys.getViolationCount("e1", "tab1") == 1, "Violation count = 1");
    assertTrue(sys.getTotalViolations("e1") == 1, "Total violations = 1");
    // Strength should decrease
    float strength = sys.getElementStrength("e1", "tab1");
    assertTrue(strength < 0.5f, "Strength decreased after violation");
    // Tension should rise
    assertTrue(sys.getTensionLevel("e1") > 0.0f, "Tension > 0 after violation");

    // Multiple violations push tension high
    for (int i = 0; i < 5; ++i) sys.violate("e1", "tab1");
    assertTrue(sys.isHighTension("e1"), "High tension after many violations");

    assertTrue(!sys.violate("e1", "nonexistent"), "Nonexistent fails");
    assertTrue(!sys.violate("missing", "tab1"), "Missing entity fails");
}

static void testFleetCultureTensionDecay() {
    std::cout << "\n=== FleetCulture: TensionDecay ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addElement("e1", "tab1", "No Retreat", CT::Taboo, "");
    sys.violate("e1", "tab1"); // tension = 0.15
    float before = sys.getTensionLevel("e1");
    assertTrue(before > 0.0f, "Tension > 0 before decay");

    sys.update(5.0f); // decay = 0.01 * 5 = 0.05
    float after = sys.getTensionLevel("e1");
    assertTrue(after < before, "Tension decays over time");
    assertTrue(after >= 0.0f, "Tension not below 0");
}

static void testFleetCultureCohesionBonus() {
    std::cout << "\n=== FleetCulture: CohesionBonus ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addElement("e1", "t1", "Tradition A", CT::Tradition, "");
    sys.addElement("e1", "t2", "Tradition B", CT::Tradition, "");

    // Fully reinforce both elements
    for (int i = 0; i < 10; ++i) {
        sys.reinforce("e1", "t1");
        sys.reinforce("e1", "t2");
    }
    float bonus = sys.getCohesionBonus("e1");
    // Two elements at max strength = 2 * (1.0 * 0.1) = 0.2
    assertTrue(approxEqual(bonus, 0.2f), "Cohesion bonus = 0.2 with two max-strength elements");

    // Deactivating an element reduces cohesion
    sys.deactivateElement("e1", "t1");
    float after = sys.getCohesionBonus("e1");
    assertTrue(after < bonus, "Cohesion drops when element deactivated");
}

static void testFleetCultureDeactivate() {
    std::cout << "\n=== FleetCulture: Deactivate ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addElement("e1", "t1", "Tradition", CT::Tradition, "");
    assertTrue(sys.isElementActive("e1", "t1"), "Element active");
    assertTrue(sys.getActiveElementCount("e1") == 1, "One active");

    assertTrue(sys.deactivateElement("e1", "t1"), "Deactivate succeeds");
    assertTrue(!sys.isElementActive("e1", "t1"), "Element inactive");
    assertTrue(sys.getActiveElementCount("e1") == 0, "Zero active");
    assertTrue(sys.getElementCount("e1") == 1, "Still in list");

    // Double deactivate fails
    assertTrue(!sys.deactivateElement("e1", "t1"), "Double deactivate fails");
    assertTrue(!sys.deactivateElement("e1", "nonexistent"), "Nonexistent fails");
    assertTrue(!sys.deactivateElement("missing", "t1"), "Missing entity fails");

    // Reinforce on inactive element should fail
    assertTrue(!sys.reinforce("e1", "t1"), "Reinforce inactive fails");
    // Violate on inactive element should fail
    assertTrue(!sys.violate("e1", "t1"), "Violate inactive fails");
}

static void testFleetCultureRemoveAndClear() {
    std::cout << "\n=== FleetCulture: RemoveAndClear ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addElement("e1", "t1", "A", CT::Tradition, "");
    sys.addElement("e1", "t2", "B", CT::Taboo, "");

    assertTrue(sys.removeElement("e1", "t1"), "Remove t1");
    assertTrue(!sys.hasElement("e1", "t1"), "t1 gone");
    assertTrue(sys.getElementCount("e1") == 1, "One left");
    assertTrue(!sys.removeElement("e1", "t1"), "Already removed");
    assertTrue(!sys.removeElement("e1", "nonexistent"), "Nonexistent fails");

    assertTrue(sys.clearElements("e1"), "Clear all");
    assertTrue(sys.getElementCount("e1") == 0, "All cleared");
    assertTrue(approxEqual(sys.getCohesionBonus("e1"), 0.0f), "Cohesion 0 after clear");
    assertTrue(!sys.clearElements("missing"), "Clear missing entity fails");
}

static void testFleetCultureConfiguration() {
    std::cout << "\n=== FleetCulture: Configuration ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setFleetId("e1", "alpha-wing"), "Set fleet id");
    assertTrue(sys.getFleetId("e1") == "alpha-wing", "Fleet id correct");
    assertTrue(!sys.setFleetId("e1", ""), "Empty fleet id rejected");

    assertTrue(sys.setMaxElements("e1", 5), "Set max elements");
    assertTrue(sys.getMaxElements("e1") == 5, "Max elements = 5");
    assertTrue(!sys.setMaxElements("e1", 0), "Zero max rejected");

    assertTrue(sys.setTensionDecayRate("e1", 0.05f), "Set tension decay rate");
    assertTrue(!sys.setTensionDecayRate("e1", -0.1f), "Negative decay rate rejected");
}

static void testFleetCultureCapacity() {
    std::cout << "\n=== FleetCulture: Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxElements("e1", 3);

    assertTrue(sys.addElement("e1", "e1", "A", CT::Tradition, ""), "Add 1");
    assertTrue(sys.addElement("e1", "e2", "B", CT::Tradition, ""), "Add 2");
    assertTrue(sys.addElement("e1", "e3", "C", CT::Tradition, ""), "Add 3");
    assertTrue(!sys.addElement("e1", "e4", "D", CT::Tradition, ""), "Add 4 fails (at cap)");
    assertTrue(sys.getElementCount("e1") == 3, "Still 3 elements");
}

static void testFleetCultureMissingEntity() {
    std::cout << "\n=== FleetCulture: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::FleetCultureSystem sys(&world);

    assertTrue(!sys.initialize("ghost"), "Init fails");
    assertTrue(sys.getElementCount("ghost") == 0, "Element count 0");
    assertTrue(sys.getActiveElementCount("ghost") == 0, "Active count 0");
    assertTrue(!sys.hasElement("ghost", "x"), "No element");
    assertTrue(approxEqual(sys.getCohesionBonus("ghost"), 0.0f), "Cohesion 0");
    assertTrue(approxEqual(sys.getTensionLevel("ghost"), 0.0f), "Tension 0");
    assertTrue(!sys.isHighTension("ghost"), "Not high tension");
    assertTrue(sys.getTotalElementsFormed("ghost") == 0, "Total formed 0");
    assertTrue(sys.getTotalReinforcements("ghost") == 0, "Total reinforcements 0");
    assertTrue(sys.getTotalViolations("ghost") == 0, "Total violations 0");
    assertTrue(sys.getFleetId("ghost").empty(), "Fleet id empty");
    assertTrue(!sys.addElement("ghost", "x", "y", CT::Tradition, ""), "addElement fails");
    assertTrue(!sys.reinforce("ghost", "x"), "reinforce fails");
    assertTrue(!sys.violate("ghost", "x"), "violate fails");
    assertTrue(!sys.setFleetId("ghost", "x"), "setFleetId fails");
    assertTrue(sys.getCountByType("ghost", CT::Tradition) == 0, "Count by type 0");
    assertTrue(sys.getMaxElements("ghost") == 0, "Max elements 0");
}

void run_fleet_culture_system_tests() {
    testFleetCultureInit();
    testFleetCultureAddElement();
    testFleetCultureReinforce();
    testFleetCultureViolate();
    testFleetCultureTensionDecay();
    testFleetCultureCohesionBonus();
    testFleetCultureDeactivate();
    testFleetCultureRemoveAndClear();
    testFleetCultureConfiguration();
    testFleetCultureCapacity();
    testFleetCultureMissingEntity();
}
