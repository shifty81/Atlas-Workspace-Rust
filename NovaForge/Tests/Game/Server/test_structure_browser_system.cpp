// Tests for: StructureBrowserSystem
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/structure_browser_system.h"

using namespace atlas;

// ==================== StructureBrowserSystem Tests ====================

static void testStructureBrowserInit() {
    std::cout << "\n=== StructureBrowser: Init ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getStructureCount("e1") == 0, "Zero structures");
    assertTrue(sys.getSearchFilter("e1") == "", "Empty search filter");
    assertTrue(sys.getTotalSearches("e1") == 0, "Zero searches");
    assertTrue(sys.getTotalEntriesAdded("e1") == 0, "Zero added");
    assertTrue(sys.getTotalEntriesRemoved("e1") == 0, "Zero removed");
    assertTrue(sys.getFilteredCount("e1") == 0, "Zero filtered");
    assertTrue(sys.getPublicCount("e1") == 0, "Zero public");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testStructureBrowserAddRemove() {
    std::cout << "\n=== StructureBrowser: Add/Remove ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ST = components::StructureBrowserState::StructureType;

    assertTrue(sys.addStructure("e1", "s1", "Jita Trade Hub", ST::Keepstar, "Jita", "CalNavy"),
               "Add citadel");
    assertTrue(sys.getStructureCount("e1") == 1, "1 structure");
    assertTrue(sys.hasStructure("e1", "s1"), "Has s1");
    assertTrue(sys.getTotalEntriesAdded("e1") == 1, "1 added");

    assertTrue(sys.addStructure("e1", "s2", "Refinery Alpha", ST::Refinery, "Amarr", "CorpX"),
               "Add refinery");
    assertTrue(sys.getStructureCount("e1") == 2, "2 structures");

    // Duplicate rejected
    assertTrue(!sys.addStructure("e1", "s1", "Dupe", ST::Citadel, "X", "Y"),
               "Duplicate rejected");
    assertTrue(sys.getStructureCount("e1") == 2, "Still 2");

    // Empty ID rejected
    assertTrue(!sys.addStructure("e1", "", "No ID", ST::Citadel, "X", "Y"),
               "Empty ID rejected");

    // Empty name rejected
    assertTrue(!sys.addStructure("e1", "s3", "", ST::Citadel, "X", "Y"),
               "Empty name rejected");

    // Remove
    assertTrue(sys.removeStructure("e1", "s1"), "Remove s1");
    assertTrue(sys.getStructureCount("e1") == 1, "1 remaining");
    assertTrue(!sys.hasStructure("e1", "s1"), "s1 gone");
    assertTrue(sys.getTotalEntriesRemoved("e1") == 1, "1 removed");

    // Remove non-existent
    assertTrue(!sys.removeStructure("e1", "s1"), "Remove non-existent fails");

    // Missing entity
    assertTrue(!sys.addStructure("missing", "s9", "X", ST::Citadel, "X", "Y"),
               "Add on missing fails");
    assertTrue(!sys.removeStructure("missing", "s9"), "Remove on missing fails");
}

static void testStructureBrowserClear() {
    std::cout << "\n=== StructureBrowser: Clear ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ST = components::StructureBrowserState::StructureType;
    sys.addStructure("e1", "s1", "Hub A", ST::Citadel, "Jita", "Corp1");
    sys.addStructure("e1", "s2", "Hub B", ST::Fortizar, "Amarr", "Corp2");
    sys.addStructure("e1", "s3", "Ref C", ST::Refinery, "Rens", "Corp3");

    assertTrue(sys.clearStructures("e1"), "Clear");
    assertTrue(sys.getStructureCount("e1") == 0, "Zero after clear");
    assertTrue(sys.getTotalEntriesRemoved("e1") == 3, "3 removed total");
    assertTrue(!sys.clearStructures("missing"), "Clear on missing fails");
}

static void testStructureBrowserCapacity() {
    std::cout << "\n=== StructureBrowser: Capacity ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ST = components::StructureBrowserState::StructureType;

    // Manually set low capacity via component
    auto* ent = world.getEntity("e1");
    auto* comp = ent->getComponent<components::StructureBrowserState>();
    comp->max_entries = 3;

    sys.addStructure("e1", "s1", "A", ST::Citadel, "X", "Y");
    sys.addStructure("e1", "s2", "B", ST::Citadel, "X", "Y");
    sys.addStructure("e1", "s3", "C", ST::Citadel, "X", "Y");
    assertTrue(sys.getStructureCount("e1") == 3, "3 at cap");

    // Adding a 4th should fail
    assertTrue(!sys.addStructure("e1", "s4", "D", ST::Citadel, "X", "Y"),
               "At capacity - rejected");
    assertTrue(sys.getStructureCount("e1") == 3, "Still 3");
}

static void testStructureBrowserModification() {
    std::cout << "\n=== StructureBrowser: Modification ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ST = components::StructureBrowserState::StructureType;
    using SS = components::StructureBrowserState::StructureStatus;
    sys.addStructure("e1", "s1", "Test Fort", ST::Fortizar, "Jita", "Corp1");

    // Status
    assertTrue(sys.setStructureStatus("e1", "s1", SS::Reinforced), "Set reinforced");
    assertTrue(sys.getCountByStatus("e1", SS::Reinforced) == 1, "1 reinforced");
    assertTrue(sys.getCountByStatus("e1", SS::Online) == 0, "0 online now");

    // Fuel
    assertTrue(sys.setFuelRemaining("e1", "s1", 48.0f), "Set fuel 48h");
    assertTrue(approxEqual(sys.getFuelRemaining("e1", "s1"), 48.0f), "Fuel is 48");
    assertTrue(!sys.setFuelRemaining("e1", "s1", -1.0f), "Negative fuel rejected");

    // Public
    assertTrue(sys.setPublic("e1", "s1", true), "Set public");
    assertTrue(sys.getPublicCount("e1") == 1, "1 public");
    assertTrue(sys.setPublic("e1", "s1", false), "Set private");
    assertTrue(sys.getPublicCount("e1") == 0, "0 public");

    // Non-existent structure
    assertTrue(!sys.setStructureStatus("e1", "s9", SS::Online), "Status on missing struct");
    assertTrue(!sys.setFuelRemaining("e1", "s9", 10.0f), "Fuel on missing struct");
    assertTrue(!sys.setPublic("e1", "s9", true), "Public on missing struct");

    // Missing entity
    assertTrue(!sys.setStructureStatus("missing", "s1", SS::Online), "Status on missing entity");
    assertTrue(!sys.setFuelRemaining("missing", "s1", 10.0f), "Fuel on missing entity");
    assertTrue(!sys.setPublic("missing", "s1", true), "Public on missing entity");
}

static void testStructureBrowserServices() {
    std::cout << "\n=== StructureBrowser: Services ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ST = components::StructureBrowserState::StructureType;
    sys.addStructure("e1", "s1", "Eng Complex", ST::EngineeringComplex, "Jita", "Corp1");

    assertTrue(sys.getServiceCount("e1", "s1") == 0, "0 services initially");

    assertTrue(sys.addService("e1", "s1", "svc1", "Manufacturing"), "Add manufacturing");
    assertTrue(sys.getServiceCount("e1", "s1") == 1, "1 service");

    assertTrue(sys.addService("e1", "s1", "svc2", "Research"), "Add research");
    assertTrue(sys.getServiceCount("e1", "s1") == 2, "2 services");

    // Duplicate service
    assertTrue(!sys.addService("e1", "s1", "svc1", "Dupe"), "Duplicate service rejected");

    // Empty service ID
    assertTrue(!sys.addService("e1", "s1", "", "Empty"), "Empty service ID rejected");

    // Remove service
    assertTrue(sys.removeService("e1", "s1", "svc1"), "Remove svc1");
    assertTrue(sys.getServiceCount("e1", "s1") == 1, "1 service remaining");

    // Remove non-existent service
    assertTrue(!sys.removeService("e1", "s1", "svc1"), "Remove non-existent service");

    // Non-existent structure
    assertTrue(!sys.addService("e1", "s9", "svc1", "X"), "Service on missing struct");
    assertTrue(!sys.removeService("e1", "s9", "svc1"), "Remove service on missing struct");

    // Missing entity
    assertTrue(!sys.addService("missing", "s1", "svc1", "X"), "Service on missing entity");
    assertTrue(!sys.removeService("missing", "s1", "svc1"), "Remove svc on missing entity");
    assertTrue(sys.getServiceCount("missing", "s1") == 0, "ServiceCount on missing");
}

static void testStructureBrowserSearch() {
    std::cout << "\n=== StructureBrowser: Search/Filter ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ST = components::StructureBrowserState::StructureType;
    sys.addStructure("e1", "s1", "Jita Trade Hub", ST::Keepstar, "Jita", "CalNavy");
    sys.addStructure("e1", "s2", "Amarr Refinery", ST::Refinery, "Amarr", "AmarrCorp");
    sys.addStructure("e1", "s3", "Jita Eng Complex", ST::EngineeringComplex, "Jita", "Builders");
    sys.addStructure("e1", "s4", "Rens Fortizar", ST::Fortizar, "Rens", "MinCorp");

    // No filter = all
    assertTrue(sys.getFilteredCount("e1") == 4, "All 4 without filter");

    // Search by name
    assertTrue(sys.setSearchFilter("e1", "Jita"), "Set filter Jita");
    assertTrue(sys.getSearchFilter("e1") == "Jita", "Filter is Jita");
    assertTrue(sys.getFilteredCount("e1") == 3, "3 match Jita (2 name + 1 system)");
    assertTrue(sys.getTotalSearches("e1") == 1, "1 search");

    // Type filter
    assertTrue(sys.setTypeFilter("e1", ST::Keepstar, true), "Enable Keepstar filter");
    assertTrue(sys.getFilteredCount("e1") == 1, "1 Jita Keepstar");

    // Disable type filter
    assertTrue(sys.setTypeFilter("e1", ST::Keepstar, false), "Disable type filter");
    assertTrue(sys.getFilteredCount("e1") == 3, "Back to 3 Jita matches");

    // Clear search
    assertTrue(sys.setSearchFilter("e1", ""), "Clear search");
    assertTrue(sys.getFilteredCount("e1") == 4, "All 4 again");
    assertTrue(sys.getTotalSearches("e1") == 3, "3 total searches");

    // Missing entity
    assertTrue(!sys.setSearchFilter("missing", "X"), "Search on missing fails");
    assertTrue(!sys.setTypeFilter("missing", ST::Citadel, true), "TypeFilter on missing fails");
}

static void testStructureBrowserCountByType() {
    std::cout << "\n=== StructureBrowser: CountByType ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ST = components::StructureBrowserState::StructureType;
    sys.addStructure("e1", "s1", "K1", ST::Keepstar, "Jita", "C1");
    sys.addStructure("e1", "s2", "K2", ST::Keepstar, "Amarr", "C2");
    sys.addStructure("e1", "s3", "R1", ST::Refinery, "Rens", "C3");
    sys.addStructure("e1", "s4", "F1", ST::Fortizar, "Dodixie", "C4");

    assertTrue(sys.getCountByType("e1", ST::Keepstar) == 2, "2 Keepstars");
    assertTrue(sys.getCountByType("e1", ST::Refinery) == 1, "1 Refinery");
    assertTrue(sys.getCountByType("e1", ST::Fortizar) == 1, "1 Fortizar");
    assertTrue(sys.getCountByType("e1", ST::Citadel) == 0, "0 Citadels");

    assertTrue(sys.getCountByType("missing", ST::Keepstar) == 0, "CountByType on missing");
    assertTrue(sys.getCountByStatus("missing", components::StructureBrowserState::StructureStatus::Online) == 0, "CountByStatus on missing");
}

static void testStructureBrowserUpdate() {
    std::cout << "\n=== StructureBrowser: Update ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.update(5.0f);
    sys.update(10.0f);
    // No crash
    assertTrue(sys.getStructureCount("e1") == 0, "Still zero after update");
}

static void testStructureBrowserMissing() {
    std::cout << "\n=== StructureBrowser: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::StructureBrowserSystem sys(&world);

    using ST = components::StructureBrowserState::StructureType;
    using SS = components::StructureBrowserState::StructureStatus;

    assertTrue(!sys.initialize("m"), "Init fails");
    assertTrue(!sys.addStructure("m", "s", "X", ST::Citadel, "X", "Y"), "AddStructure fails");
    assertTrue(!sys.removeStructure("m", "s"), "RemoveStructure fails");
    assertTrue(!sys.clearStructures("m"), "ClearStructures fails");
    assertTrue(!sys.setStructureStatus("m", "s", SS::Online), "SetStatus fails");
    assertTrue(!sys.setFuelRemaining("m", "s", 10.0f), "SetFuel fails");
    assertTrue(!sys.setPublic("m", "s", true), "SetPublic fails");
    assertTrue(!sys.addService("m", "s", "x", "X"), "AddService fails");
    assertTrue(!sys.removeService("m", "s", "x"), "RemoveService fails");
    assertTrue(!sys.setSearchFilter("m", "x"), "SetSearchFilter fails");
    assertTrue(!sys.setTypeFilter("m", ST::Citadel, true), "SetTypeFilter fails");
    assertTrue(sys.getStructureCount("m") == 0, "getStructureCount 0");
    assertTrue(!sys.hasStructure("m", "s"), "hasStructure false");
    assertTrue(sys.getFilteredCount("m") == 0, "getFilteredCount 0");
    assertTrue(sys.getCountByType("m", ST::Citadel) == 0, "getCountByType 0");
    assertTrue(sys.getCountByStatus("m", SS::Online) == 0, "getCountByStatus 0");
    assertTrue(sys.getPublicCount("m") == 0, "getPublicCount 0");
    assertTrue(approxEqual(sys.getFuelRemaining("m", "s"), 0.0f), "getFuelRemaining 0");
    assertTrue(sys.getServiceCount("m", "s") == 0, "getServiceCount 0");
    assertTrue(sys.getSearchFilter("m") == "", "getSearchFilter empty");
    assertTrue(sys.getTotalSearches("m") == 0, "getTotalSearches 0");
    assertTrue(sys.getTotalEntriesAdded("m") == 0, "getTotalEntriesAdded 0");
    assertTrue(sys.getTotalEntriesRemoved("m") == 0, "getTotalEntriesRemoved 0");
}

void run_structure_browser_system_tests() {
    testStructureBrowserInit();
    testStructureBrowserAddRemove();
    testStructureBrowserClear();
    testStructureBrowserCapacity();
    testStructureBrowserModification();
    testStructureBrowserServices();
    testStructureBrowserSearch();
    testStructureBrowserCountByType();
    testStructureBrowserUpdate();
    testStructureBrowserMissing();
}
