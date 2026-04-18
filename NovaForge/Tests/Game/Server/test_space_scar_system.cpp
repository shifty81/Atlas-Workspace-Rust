// Tests for: SpaceScarSystem
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/space_scar_system.h"

using namespace atlas;
using ST = components::SpaceScarState::ScarType;
using DS = components::SpaceScarState::DiscoverySource;

static void testSpaceScarInit() {
    std::cout << "\n=== SpaceScar: Init ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getScarCount("e1") == 0, "Zero scars initially");
    assertTrue(sys.getTotalDiscovered("e1") == 0, "Zero total_discovered");
    assertTrue(sys.getTotalMentions("e1") == 0, "Zero total_mentions");
    assertTrue(sys.getSystemId("e1").empty(), "Empty system_id");
    assertTrue(sys.getMaxScars("e1") == 50, "Default max_scars 50");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testSpaceScarAddScar() {
    std::cout << "\n=== SpaceScar: AddScar ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        sys.addScar("e1", "s1", "The Wreck of Eris", ST::WreckField, DS::Player,
                    "Sector 4A", "pilot_alpha"),
        "Add scar succeeds");
    assertTrue(sys.getScarCount("e1") == 1, "1 scar");
    assertTrue(sys.hasScar("e1", "s1"), "hasScar s1");
    assertTrue(sys.getScarName("e1", "s1") == "The Wreck of Eris", "Name matches");
    assertTrue(sys.getScarType("e1", "s1") == ST::WreckField, "Type WreckField");
    assertTrue(sys.getDiscoverySource("e1", "s1") == DS::Player, "Source Player");
    assertTrue(sys.getLocationLabel("e1", "s1") == "Sector 4A", "Location matches");
    assertTrue(sys.getFirstDiscoverer("e1", "s1") == "pilot_alpha", "Discoverer matches");
    assertTrue(sys.getMentionCount("e1", "s1") == 0, "Zero mentions");
    assertTrue(!sys.isOfficiallyNamed("e1", "s1"), "Not officially named");
    assertTrue(sys.getScarNotes("e1", "s1").empty(), "No notes");
    assertTrue(sys.getTotalDiscovered("e1") == 1, "total_discovered = 1");

    // Duplicate rejected
    assertTrue(
        !sys.addScar("e1", "s1", "Duplicate", ST::Battlefield, DS::AI, "loc", "p"),
        "Duplicate scar_id rejected");
    assertTrue(sys.getScarCount("e1") == 1, "Still 1 scar");

    // Second scar of different type
    assertTrue(
        sys.addScar("e1", "s2", "Burned Outpost", ST::BurnedStation, DS::AI,
                    "Sector 7B", "ai_scout"),
        "Add second scar");
    assertTrue(sys.getTotalDiscovered("e1") == 2, "total_discovered = 2");
    assertTrue(sys.getScarType("e1", "s2") == ST::BurnedStation, "Type BurnedStation");
    assertTrue(sys.getDiscoverySource("e1", "s2") == DS::AI, "Source AI");
}

static void testSpaceScarAddScarValidation() {
    std::cout << "\n=== SpaceScar: AddScar Validation ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        !sys.addScar("e1", "", "Name", ST::WreckField, DS::Player, "loc", "p"),
        "Empty scar_id rejected");
    assertTrue(
        !sys.addScar("e1", "s1", "", ST::WreckField, DS::Player, "loc", "p"),
        "Empty name rejected");
    assertTrue(
        !sys.addScar("missing", "s1", "Name", ST::WreckField, DS::Player, "loc", "p"),
        "Add on missing entity fails");
    assertTrue(sys.getScarCount("e1") == 0, "Still zero scars");
}

static void testSpaceScarRemoveScar() {
    std::cout << "\n=== SpaceScar: RemoveScar ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addScar("e1", "s1", "Scar1", ST::Battlefield, DS::Event, "loc1", "p1");
    sys.addScar("e1", "s2", "Scar2", ST::FailedColony, DS::Unknown, "loc2", "p2");

    assertTrue(sys.removeScar("e1", "s1"), "Remove s1");
    assertTrue(sys.getScarCount("e1") == 1, "1 scar remains");
    assertTrue(!sys.hasScar("e1", "s1"), "s1 gone");
    assertTrue(sys.hasScar("e1", "s2"), "s2 still exists");
    assertTrue(!sys.removeScar("e1", "s1"), "Remove missing fails");
    assertTrue(!sys.removeScar("missing", "s2"), "Remove on missing entity fails");

    assertTrue(sys.clearScars("e1"), "Clear scars");
    assertTrue(sys.getScarCount("e1") == 0, "Zero after clear");
    assertTrue(!sys.clearScars("missing"), "Clear on missing fails");
}

static void testSpaceScarNameScar() {
    std::cout << "\n=== SpaceScar: NameScar ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addScar("e1", "s1", "Old Name", ST::CelestialGraveyard, DS::Player, "loc", "p");
    assertTrue(!sys.isOfficiallyNamed("e1", "s1"), "Not official initially");

    assertTrue(sys.nameScar("e1", "s1", "New Name", false), "Rename unofficially");
    assertTrue(sys.getScarName("e1", "s1") == "New Name", "Name updated");
    assertTrue(!sys.isOfficiallyNamed("e1", "s1"), "Still not official");

    assertTrue(sys.nameScar("e1", "s1", "Official Name", true), "Rename officially");
    assertTrue(sys.getScarName("e1", "s1") == "Official Name", "Official name set");
    assertTrue(sys.isOfficiallyNamed("e1", "s1"), "Now official");

    assertTrue(!sys.nameScar("e1", "s1", "", true), "Empty name rejected");
    assertTrue(!sys.nameScar("e1", "nonexistent", "X", true), "Missing scar_id fails");
    assertTrue(!sys.nameScar("missing", "s1", "X", true), "Missing entity fails");
}

static void testSpaceScarRecordMention() {
    std::cout << "\n=== SpaceScar: RecordMention ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addScar("e1", "s1", "Scar One", ST::SignalAnomaly, DS::AI, "loc", "p");
    assertTrue(sys.getMentionCount("e1", "s1") == 0, "Zero mentions");

    assertTrue(sys.recordMention("e1", "s1"), "Record mention succeeds");
    assertTrue(sys.getMentionCount("e1", "s1") == 1, "1 mention");
    assertTrue(sys.getTotalMentions("e1") == 1, "total_mentions = 1");

    sys.recordMention("e1", "s1");
    sys.recordMention("e1", "s1");
    assertTrue(sys.getMentionCount("e1", "s1") == 3, "3 mentions");
    assertTrue(sys.getTotalMentions("e1") == 3, "total_mentions = 3");

    // Multiple scars
    sys.addScar("e1", "s2", "Scar Two", ST::WreckField, DS::Player, "loc2", "p2");
    sys.recordMention("e1", "s2");
    assertTrue(sys.getTotalMentions("e1") == 4, "total_mentions = 4");

    assertTrue(!sys.recordMention("e1", "nonexistent"), "Missing scar_id fails");
    assertTrue(!sys.recordMention("missing", "s1"), "Missing entity fails");
    assertTrue(sys.getTotalMentions("e1") == 4, "total_mentions unchanged");
}

static void testSpaceScarAddNote() {
    std::cout << "\n=== SpaceScar: AddNote ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addScar("e1", "s1", "Scar", ST::WreckField, DS::Player, "loc", "p");
    assertTrue(sys.getScarNotes("e1", "s1").empty(), "Notes empty initially");

    assertTrue(sys.addNote("e1", "s1", "First note"), "Add note succeeds");
    assertTrue(sys.getScarNotes("e1", "s1") == "First note", "Note matches");

    // Overwrite
    assertTrue(sys.addNote("e1", "s1", "Updated note"), "Overwrite note");
    assertTrue(sys.getScarNotes("e1", "s1") == "Updated note", "Note updated");

    assertTrue(!sys.addNote("e1", "s1", ""), "Empty note rejected");
    assertTrue(!sys.addNote("e1", "nonexistent", "note"), "Missing scar_id fails");
    assertTrue(!sys.addNote("missing", "s1", "note"), "Missing entity fails");
}

static void testSpaceScarConfiguration() {
    std::cout << "\n=== SpaceScar: Configuration ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setSystemId("e1", "sol-system"), "Set system_id");
    assertTrue(sys.getSystemId("e1") == "sol-system", "system_id matches");
    assertTrue(!sys.setSystemId("e1", ""), "Empty system_id rejected");
    assertTrue(!sys.setSystemId("missing", "sol"), "Set on missing fails");

    assertTrue(sys.setMaxScars("e1", 10), "Set max_scars 10");
    assertTrue(sys.getMaxScars("e1") == 10, "max_scars matches");
    assertTrue(!sys.setMaxScars("e1", 0), "Zero max_scars rejected");
    assertTrue(!sys.setMaxScars("e1", -1), "Negative max_scars rejected");
    assertTrue(!sys.setMaxScars("missing", 5), "Set max on missing fails");
}

static void testSpaceScarCountByType() {
    std::cout << "\n=== SpaceScar: CountByType ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addScar("e1", "s1", "W1", ST::WreckField, DS::Player, "", "");
    sys.addScar("e1", "s2", "W2", ST::WreckField, DS::AI, "", "");
    sys.addScar("e1", "s3", "B1", ST::Battlefield, DS::Event, "", "");
    sys.addScar("e1", "s4", "F1", ST::FailedColony, DS::Unknown, "", "");

    assertTrue(sys.getCountByType("e1", ST::WreckField) == 2, "2 WreckField");
    assertTrue(sys.getCountByType("e1", ST::Battlefield) == 1, "1 Battlefield");
    assertTrue(sys.getCountByType("e1", ST::FailedColony) == 1, "1 FailedColony");
    assertTrue(sys.getCountByType("e1", ST::BurnedStation) == 0, "0 BurnedStation");
    assertTrue(sys.getCountByType("e1", ST::CelestialGraveyard) == 0, "0 CelestialGraveyard");
    assertTrue(sys.getCountByType("e1", ST::SignalAnomaly) == 0, "0 SignalAnomaly");
    assertTrue(sys.getCountByType("missing", ST::WreckField) == 0, "CountByType on missing = 0");
}

static void testSpaceScarCapacityCap() {
    std::cout << "\n=== SpaceScar: Capacity Cap ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxScars("e1", 3);

    assertTrue(
        sys.addScar("e1", "s1", "Scar1", ST::WreckField, DS::Player, "", ""),
        "Add scar 1");
    assertTrue(
        sys.addScar("e1", "s2", "Scar2", ST::Battlefield, DS::AI, "", ""),
        "Add scar 2");
    assertTrue(
        sys.addScar("e1", "s3", "Scar3", ST::FailedColony, DS::Event, "", ""),
        "Add scar 3");
    assertTrue(
        !sys.addScar("e1", "s4", "Scar4", ST::SignalAnomaly, DS::Unknown, "", ""),
        "4th scar rejected at capacity 3");
    assertTrue(sys.getScarCount("e1") == 3, "Still 3 scars");
    assertTrue(sys.getTotalDiscovered("e1") == 3, "total_discovered = 3");
}

static void testSpaceScarMissingEntity() {
    std::cout << "\n=== SpaceScar: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SpaceScarSystem sys(&world);

    assertTrue(sys.getScarCount("missing") == 0, "getScarCount = 0");
    assertTrue(!sys.hasScar("missing", "s1"), "hasScar = false");
    assertTrue(sys.getScarName("missing", "s1").empty(), "getScarName = ''");
    assertTrue(sys.getScarType("missing", "s1") == ST::WreckField, "getScarType = WreckField default");
    assertTrue(sys.getDiscoverySource("missing", "s1") == DS::Unknown, "getDiscoverySource = Unknown default");
    assertTrue(sys.getLocationLabel("missing", "s1").empty(), "getLocationLabel = ''");
    assertTrue(sys.getFirstDiscoverer("missing", "s1").empty(), "getFirstDiscoverer = ''");
    assertTrue(sys.getMentionCount("missing", "s1") == 0, "getMentionCount = 0");
    assertTrue(!sys.isOfficiallyNamed("missing", "s1"), "isOfficiallyNamed = false");
    assertTrue(sys.getScarNotes("missing", "s1").empty(), "getScarNotes = ''");
    assertTrue(sys.getTotalDiscovered("missing") == 0, "getTotalDiscovered = 0");
    assertTrue(sys.getTotalMentions("missing") == 0, "getTotalMentions = 0");
    assertTrue(sys.getSystemId("missing").empty(), "getSystemId = ''");
    assertTrue(sys.getMaxScars("missing") == 0, "getMaxScars = 0");
    assertTrue(sys.getCountByType("missing", ST::Battlefield) == 0, "getCountByType = 0");
}

void run_space_scar_system_tests() {
    testSpaceScarInit();
    testSpaceScarAddScar();
    testSpaceScarAddScarValidation();
    testSpaceScarRemoveScar();
    testSpaceScarNameScar();
    testSpaceScarRecordMention();
    testSpaceScarAddNote();
    testSpaceScarConfiguration();
    testSpaceScarCountByType();
    testSpaceScarCapacityCap();
    testSpaceScarMissingEntity();
}
