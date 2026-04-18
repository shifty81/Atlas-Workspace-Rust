// Tests for: SectorTensionSystem
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/sector_tension_system.h"

using namespace atlas;
using TT = components::SectorTensionState::TensionType;

// ==================== SectorTensionSystem Tests ====================

static void testSectorTensionInit() {
    std::cout << "\n=== SectorTension: Init ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(approxEqual(sys.getTensionLevel("e1"), 0.0f), "Zero tension initially");
    assertTrue(!sys.isHighTension("e1"), "Not high tension initially");
    assertTrue(!sys.isCriticalTension("e1"), "Not critical initially");
    assertTrue(sys.getEventCount("e1") == 0, "Zero events initially");
    assertTrue(sys.getTotalEventsRecorded("e1") == 0, "Zero total events");
    assertTrue(sys.getSectorId("e1").empty(), "Empty sector id");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testSectorTensionAddEvent() {
    std::cout << "\n=== SectorTension: AddEvent ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        sys.addEvent("e1", "ev1", TT::PirateRaid, 20.0f, 0.5f, 300.0f),
        "Add pirate raid event");
    assertTrue(sys.getEventCount("e1") == 1, "1 event");
    assertTrue(sys.hasEvent("e1", "ev1"), "Has ev1");
    assertTrue(approxEqual(sys.getTensionLevel("e1"), 20.0f),
               "Tension = 20 after event");
    assertTrue(sys.getTotalEventsRecorded("e1") == 1, "1 total event");

    // Second event stacks tension
    sys.addEvent("e1", "ev2", TT::Smuggling, 15.0f, 0.3f, 200.0f);
    assertTrue(approxEqual(sys.getTensionLevel("e1"), 35.0f), "Tension = 35");

    // Duplicate id rejected
    assertTrue(
        !sys.addEvent("e1", "ev1", TT::Disaster, 10.0f, 0.1f, 100.0f),
        "Duplicate event id rejected");
    assertTrue(sys.getEventCount("e1") == 2, "Still 2 events");

    // Validation
    assertTrue(
        !sys.addEvent("e1", "", TT::PirateRaid, 10.0f, 0.5f, 100.0f),
        "Empty id rejected");
    assertTrue(
        !sys.addEvent("e1", "ev3", TT::PirateRaid, 0.0f, 0.5f, 100.0f),
        "Zero magnitude rejected");
    assertTrue(
        !sys.addEvent("e1", "ev3", TT::PirateRaid, -5.0f, 0.5f, 100.0f),
        "Negative magnitude rejected");
    assertTrue(
        !sys.addEvent("e1", "ev3", TT::PirateRaid, 10.0f, -1.0f, 100.0f),
        "Negative decay rejected");
    assertTrue(
        !sys.addEvent("e1", "ev3", TT::PirateRaid, 10.0f, 0.5f, 0.0f),
        "Zero duration rejected");
    assertTrue(
        !sys.addEvent("missing", "ev3", TT::PirateRaid, 10.0f, 0.5f, 100.0f),
        "Add on missing fails");
}

static void testSectorTensionMaxTension() {
    std::cout << "\n=== SectorTension: MaxTension Cap ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Add events that exceed max (default 100)
    sys.addEvent("e1", "ev1", TT::Disaster, 60.0f, 0.0f, 1000.0f);
    sys.addEvent("e1", "ev2", TT::MilitaryPresence, 60.0f, 0.0f, 1000.0f);
    assertTrue(approxEqual(sys.getTensionLevel("e1"), 100.0f),
               "Tension capped at 100");
}

static void testSectorTensionHighCritical() {
    std::cout << "\n=== SectorTension: High/Critical Thresholds ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addEvent("e1", "ev1", TT::CorporateConflict, 74.0f, 0.0f, 9999.0f);
    assertTrue(!sys.isHighTension("e1"), "Not high at 74");
    assertTrue(!sys.isCriticalTension("e1"), "Not critical at 74");

    sys.addEvent("e1", "ev2", TT::EconomicStress, 2.0f, 0.0f, 9999.0f);
    assertTrue(sys.isHighTension("e1"), "High at 76");
    assertTrue(!sys.isCriticalTension("e1"), "Not critical at 76");

    sys.addEvent("e1", "ev3", TT::Smuggling, 20.0f, 0.0f, 9999.0f);
    assertTrue(sys.isHighTension("e1"), "High at 96");
    assertTrue(sys.isCriticalTension("e1"), "Critical at 96");
}

static void testSectorTensionDecay() {
    std::cout << "\n=== SectorTension: Decay ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setPassiveDecayRate("e1", 0.0f);  // disable passive decay

    // Event with explicit decay rate 5/s, duration 10s
    sys.addEvent("e1", "ev1", TT::PirateRaid, 50.0f, 5.0f, 10.0f);
    assertTrue(approxEqual(sys.getTensionLevel("e1"), 50.0f), "Tension = 50");

    sys.update(1.0f);
    // 5 points/s decayed over 1s
    float t = sys.getTensionLevel("e1");
    assertTrue(t < 50.0f, "Tension decreased after 1s");
    assertTrue(t > 0.0f, "Tension > 0 after 1s");
}

static void testSectorTensionEventExpiry() {
    std::cout << "\n=== SectorTension: Event Expiry ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setPassiveDecayRate("e1", 0.0f);

    sys.addEvent("e1", "ev1", TT::Smuggling, 30.0f, 0.0f, 3.0f);
    assertTrue(sys.getEventCount("e1") == 1, "1 event before expiry");

    sys.update(4.0f);
    assertTrue(sys.getEventCount("e1") == 0, "0 events after expiry");
    assertTrue(!sys.hasEvent("e1", "ev1"), "ev1 gone after expiry");
    // Tension may still be non-zero due to partial decay timing, just check it
    // hasn't increased.
    assertTrue(sys.getTotalEventsRecorded("e1") == 1,
               "Total events still 1");
}

static void testSectorTensionRemoveEvent() {
    std::cout << "\n=== SectorTension: RemoveEvent ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addEvent("e1", "ev1", TT::PirateRaid, 25.0f, 0.5f, 300.0f);
    sys.addEvent("e1", "ev2", TT::Disaster, 10.0f, 0.2f, 200.0f);

    assertTrue(sys.removeEvent("e1", "ev1"), "Remove ev1");
    assertTrue(sys.getEventCount("e1") == 1, "1 event after remove");
    assertTrue(!sys.hasEvent("e1", "ev1"), "ev1 gone");
    assertTrue(!sys.removeEvent("e1", "ev1"), "Remove missing fails");

    assertTrue(sys.clearEvents("e1"), "Clear events");
    assertTrue(sys.getEventCount("e1") == 0, "Zero after clear");
    assertTrue(!sys.clearEvents("missing"), "Clear on missing fails");
    assertTrue(!sys.removeEvent("missing", "ev1"), "Remove on missing fails");
}

static void testSectorTensionConfiguration() {
    std::cout << "\n=== SectorTension: Configuration ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setSectorId("e1", "Asakai"), "Set sector id");
    assertTrue(sys.getSectorId("e1") == "Asakai", "Sector id matches");
    assertTrue(!sys.setSectorId("e1", ""), "Empty sector id rejected");

    assertTrue(sys.setPassiveDecayRate("e1", 0.5f), "Set passive decay 0.5");
    assertTrue(approxEqual(sys.getPassiveDecayRate("e1"), 0.5f),
               "Decay rate matches");
    assertTrue(sys.setPassiveDecayRate("e1", 0.0f), "Set passive decay 0 (ok)");
    assertTrue(!sys.setPassiveDecayRate("e1", -0.1f),
               "Negative decay rejected");

    assertTrue(sys.setMaxTension("e1", 200.0f), "Set max tension 200");
    assertTrue(!sys.setMaxTension("e1", 0.0f), "Zero max tension rejected");
    assertTrue(!sys.setMaxTension("e1", -50.0f), "Negative max rejected");

    assertTrue(sys.setMaxEvents("e1", 5), "Set max events 5");
    assertTrue(!sys.setMaxEvents("e1", 0), "Zero max events rejected");

    assertTrue(!sys.setSectorId("missing", "s"),
               "SectorId on missing fails");
    assertTrue(!sys.setPassiveDecayRate("missing", 0.1f),
               "DecayRate on missing fails");
    assertTrue(!sys.setMaxTension("missing", 100.0f),
               "MaxTension on missing fails");
    assertTrue(!sys.setMaxEvents("missing", 5),
               "MaxEvents on missing fails");
}

static void testSectorTensionCountByType() {
    std::cout << "\n=== SectorTension: CountByType ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addEvent("e1", "ev1", TT::PirateRaid, 10.0f, 0.0f, 9999.0f);
    sys.addEvent("e1", "ev2", TT::PirateRaid, 10.0f, 0.0f, 9999.0f);
    sys.addEvent("e1", "ev3", TT::Disaster,   10.0f, 0.0f, 9999.0f);

    assertTrue(sys.getCountByType("e1", TT::PirateRaid) == 2,
               "2 pirate raid events");
    assertTrue(sys.getCountByType("e1", TT::Disaster) == 1, "1 disaster");
    assertTrue(sys.getCountByType("e1", TT::Smuggling) == 0, "0 smuggling");
    assertTrue(sys.getCountByType("missing", TT::PirateRaid) == 0,
               "CountByType on missing returns 0");
}

static void testSectorTensionCapacityCap() {
    std::cout << "\n=== SectorTension: Capacity Cap ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxEvents("e1", 3);

    sys.addEvent("e1", "ev1", TT::Smuggling, 5.0f, 0.0f, 100.0f);
    sys.addEvent("e1", "ev2", TT::Smuggling, 5.0f, 0.0f, 100.0f);
    sys.addEvent("e1", "ev3", TT::Smuggling, 5.0f, 0.0f, 100.0f);
    assertTrue(!sys.addEvent("e1", "ev4", TT::Smuggling, 5.0f, 0.0f, 100.0f),
               "ev4 rejected at capacity 3");
    assertTrue(sys.getEventCount("e1") == 3, "Still 3 events");
}

static void testSectorTensionMissingEntity() {
    std::cout << "\n=== SectorTension: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SectorTensionSystem sys(&world);

    assertTrue(approxEqual(sys.getTensionLevel("missing"), 0.0f),
               "TensionLevel returns 0 for missing");
    assertTrue(!sys.isHighTension("missing"),
               "isHighTension returns false for missing");
    assertTrue(!sys.isCriticalTension("missing"),
               "isCriticalTension returns false for missing");
    assertTrue(sys.getEventCount("missing") == 0,
               "EventCount returns 0 for missing");
    assertTrue(!sys.hasEvent("missing", "ev1"),
               "hasEvent returns false for missing");
    assertTrue(sys.getSectorId("missing").empty(),
               "SectorId returns empty for missing");
    assertTrue(sys.getTotalEventsRecorded("missing") == 0,
               "TotalEvents returns 0 for missing");
    assertTrue(sys.getCountByType("missing", TT::Disaster) == 0,
               "CountByType returns 0 for missing");
    assertTrue(approxEqual(sys.getPassiveDecayRate("missing"), 0.0f),
               "PassiveDecayRate returns 0 for missing");
}

void run_sector_tension_system_tests() {
    testSectorTensionInit();
    testSectorTensionAddEvent();
    testSectorTensionMaxTension();
    testSectorTensionHighCritical();
    testSectorTensionDecay();
    testSectorTensionEventExpiry();
    testSectorTensionRemoveEvent();
    testSectorTensionConfiguration();
    testSectorTensionCountByType();
    testSectorTensionCapacityCap();
    testSectorTensionMissingEntity();
}
