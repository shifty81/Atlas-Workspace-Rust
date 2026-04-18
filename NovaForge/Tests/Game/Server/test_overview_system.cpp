// Tests for: OverviewSystem
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/overview_system.h"

using namespace atlas;

// ==================== OverviewSystem Tests ====================

static void testOverviewInit() {
    std::cout << "\n=== Overview: Init ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getEntryCount("e1") == 0, "Zero entries initially");
    assertTrue(sys.getProfileCount("e1") == 0, "Zero profiles initially");
    assertTrue(sys.getActiveProfileId("e1") == "", "No active profile");
    assertTrue(sys.getTotalEntriesTracked("e1") == 0, "Zero tracked");
    assertTrue(sys.getTotalEntriesRemoved("e1") == 0, "Zero removed");
    assertTrue(sys.getSortField("e1") == components::OverviewState::SortField::Distance,
               "Default sort is Distance");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testOverviewAddEntry() {
    std::cout << "\n=== Overview: AddEntry ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ET = components::OverviewState::EntryType;
    assertTrue(sys.addEntry("e1", "ship1", "Raven", ET::Ship, 15000.0f), "Add ship");
    assertTrue(sys.getEntryCount("e1") == 1, "1 entry");
    assertTrue(sys.hasEntry("e1", "ship1"), "Has ship1");
    assertTrue(sys.getEntryName("e1", "ship1") == "Raven", "Name is Raven");
    assertTrue(approxEqual(sys.getEntryDistance("e1", "ship1"), 15000.0f), "Distance 15km");
    assertTrue(sys.getTotalEntriesTracked("e1") == 1, "1 tracked");

    assertTrue(sys.addEntry("e1", "ast1", "Veldspar", ET::Asteroid, 5000.0f), "Add asteroid");
    assertTrue(sys.getEntryCount("e1") == 2, "2 entries");

    // Duplicate rejected
    assertTrue(!sys.addEntry("e1", "ship1", "X", ET::Ship, 1000.0f), "Duplicate rejected");
    assertTrue(sys.getEntryCount("e1") == 2, "Still 2 entries");

    // Empty ID rejected
    assertTrue(!sys.addEntry("e1", "", "X", ET::Ship, 1000.0f), "Empty ID rejected");

    // Negative distance rejected
    assertTrue(!sys.addEntry("e1", "x1", "X", ET::Ship, -5.0f), "Negative distance rejected");

    // Missing entity
    assertTrue(!sys.addEntry("missing", "x1", "X", ET::Ship, 100.0f), "Add fails on missing entity");
}

static void testOverviewRemoveEntry() {
    std::cout << "\n=== Overview: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ET = components::OverviewState::EntryType;
    sys.addEntry("e1", "ship1", "Raven", ET::Ship, 10000.0f);
    sys.addEntry("e1", "ast1", "Veldspar", ET::Asteroid, 5000.0f);

    assertTrue(sys.removeEntry("e1", "ship1"), "Remove ship1");
    assertTrue(sys.getEntryCount("e1") == 1, "1 entry remaining");
    assertTrue(!sys.hasEntry("e1", "ship1"), "ship1 gone");
    assertTrue(sys.getTotalEntriesRemoved("e1") == 1, "1 removed");

    assertTrue(!sys.removeEntry("e1", "ship1"), "Remove non-existent fails");
    assertTrue(!sys.removeEntry("missing", "ast1"), "Remove fails on missing entity");
}

static void testOverviewClearEntries() {
    std::cout << "\n=== Overview: ClearEntries ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ET = components::OverviewState::EntryType;
    sys.addEntry("e1", "s1", "A", ET::Ship, 1000.0f);
    sys.addEntry("e1", "s2", "B", ET::Ship, 2000.0f);

    assertTrue(sys.clearEntries("e1"), "Clear entries");
    assertTrue(sys.getEntryCount("e1") == 0, "Zero entries");
    assertTrue(sys.getTotalEntriesRemoved("e1") == 2, "2 removed");
    assertTrue(!sys.clearEntries("missing"), "Clear fails on missing entity");
}

static void testOverviewUpdateEntry() {
    std::cout << "\n=== Overview: UpdateEntry ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ET = components::OverviewState::EntryType;
    sys.addEntry("e1", "s1", "Raven", ET::Ship, 10000.0f);

    assertTrue(sys.updateEntryDistance("e1", "s1", 5000.0f), "Update distance");
    assertTrue(approxEqual(sys.getEntryDistance("e1", "s1"), 5000.0f), "Distance updated");

    assertTrue(sys.updateEntrySpeed("e1", "s1", 250.0f), "Update speed");
    assertTrue(!sys.updateEntryDistance("e1", "s1", -1.0f), "Negative distance rejected");
    assertTrue(!sys.updateEntrySpeed("e1", "s1", -1.0f), "Negative speed rejected");
    assertTrue(!sys.updateEntryDistance("e1", "sX", 100.0f), "Update non-existent fails");
    assertTrue(!sys.updateEntryDistance("missing", "s1", 100.0f), "Update fails on missing entity");
}

static void testOverviewEntryFlags() {
    std::cout << "\n=== Overview: EntryFlags ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ET = components::OverviewState::EntryType;
    sys.addEntry("e1", "s1", "Hostile Ship", ET::Ship, 10000.0f);

    assertTrue(!sys.isEntryHostile("e1", "s1"), "Not hostile initially");
    assertTrue(!sys.isEntryTargeted("e1", "s1"), "Not targeted initially");

    assertTrue(sys.setEntryHostile("e1", "s1", true), "Set hostile");
    assertTrue(sys.isEntryHostile("e1", "s1"), "Is hostile");

    assertTrue(sys.setEntryTargeted("e1", "s1", true), "Set targeted");
    assertTrue(sys.isEntryTargeted("e1", "s1"), "Is targeted");

    assertTrue(sys.setEntryHostile("e1", "s1", false), "Clear hostile");
    assertTrue(!sys.isEntryHostile("e1", "s1"), "No longer hostile");

    assertTrue(!sys.setEntryHostile("e1", "sX", true), "Hostile on non-existent fails");
    assertTrue(!sys.setEntryTargeted("missing", "s1", true), "Targeted on missing entity fails");
}

static void testOverviewProfile() {
    std::cout << "\n=== Overview: Profile ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.createProfile("e1", "pv1", "PvP"), "Create PvP profile");
    assertTrue(sys.getProfileCount("e1") == 1, "1 profile");

    assertTrue(sys.createProfile("e1", "pve", "PvE"), "Create PvE profile");
    assertTrue(sys.getProfileCount("e1") == 2, "2 profiles");

    // Duplicate profile rejected
    assertTrue(!sys.createProfile("e1", "pv1", "X"), "Duplicate profile rejected");

    // Empty profile ID rejected
    assertTrue(!sys.createProfile("e1", "", "X"), "Empty profile ID rejected");

    // Activate profile
    assertTrue(sys.activateProfile("e1", "pv1"), "Activate PvP profile");
    assertTrue(sys.getActiveProfileId("e1") == "pv1", "Active is pv1");

    // Activate non-existent
    assertTrue(!sys.activateProfile("e1", "noProfile"), "Activate non-existent fails");

    // Delete active profile clears active
    assertTrue(sys.deleteProfile("e1", "pv1"), "Delete PvP profile");
    assertTrue(sys.getActiveProfileId("e1") == "", "Active cleared after delete");
    assertTrue(sys.getProfileCount("e1") == 1, "1 profile remaining");

    assertTrue(!sys.deleteProfile("e1", "pv1"), "Delete non-existent fails");
    assertTrue(!sys.createProfile("missing", "x", "X"), "Create fails on missing entity");
}

static void testOverviewProfileFiltering() {
    std::cout << "\n=== Overview: ProfileFiltering ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ET = components::OverviewState::EntryType;

    sys.addEntry("e1", "s1", "Raven", ET::Ship, 10000.0f);
    sys.addEntry("e1", "s2", "Drake", ET::Ship, 20000.0f);
    sys.addEntry("e1", "a1", "Veldspar", ET::Asteroid, 5000.0f);
    sys.addEntry("e1", "w1", "Wreck", ET::Wreck, 8000.0f);

    // No profile active: all visible
    assertTrue(sys.getVisibleEntryCount("e1") == 4, "4 visible with no profile");

    // Create and activate a Ship-only profile
    sys.createProfile("e1", "ships", "Ships Only");
    assertTrue(sys.addTypeToProfile("e1", "ships", ET::Ship), "Add Ship type to profile");
    sys.activateProfile("e1", "ships");

    assertTrue(sys.getVisibleEntryCount("e1") == 2, "2 visible (ships only)");

    // Add wreck type
    assertTrue(sys.addTypeToProfile("e1", "ships", ET::Wreck), "Add Wreck type");
    assertTrue(sys.getVisibleEntryCount("e1") == 3, "3 visible (ships + wrecks)");

    // Duplicate type rejected
    assertTrue(!sys.addTypeToProfile("e1", "ships", ET::Ship), "Duplicate type rejected");

    // Remove type
    assertTrue(sys.removeTypeFromProfile("e1", "ships", ET::Wreck), "Remove Wreck type");
    assertTrue(sys.getVisibleEntryCount("e1") == 2, "2 visible again");

    assertTrue(!sys.removeTypeFromProfile("e1", "ships", ET::Wreck), "Remove non-existent type fails");
    assertTrue(!sys.addTypeToProfile("e1", "noProfile", ET::Ship), "Add to non-existent profile fails");
    assertTrue(!sys.removeTypeFromProfile("missing", "ships", ET::Ship), "Remove on missing entity fails");
}

static void testOverviewConfiguration() {
    std::cout << "\n=== Overview: Configuration ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using SF = components::OverviewState::SortField;

    assertTrue(sys.setSortField("e1", SF::Name), "Set sort to Name");
    assertTrue(sys.getSortField("e1") == SF::Name, "Sort is Name");

    assertTrue(sys.setSortAscending("e1", false), "Set descending");
    assertTrue(sys.setSortAscending("e1", true), "Set ascending");

    assertTrue(sys.setMaxRange("e1", 50000.0f), "Set max range");
    assertTrue(!sys.setMaxRange("e1", -1.0f), "Negative range rejected");

    assertTrue(!sys.setSortField("missing", SF::Speed), "Sort fails on missing entity");
    assertTrue(!sys.setSortAscending("missing", true), "Ascending fails on missing entity");
    assertTrue(!sys.setMaxRange("missing", 100.0f), "MaxRange fails on missing entity");
}

static void testOverviewMaxRange() {
    std::cout << "\n=== Overview: MaxRange ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ET = components::OverviewState::EntryType;
    sys.setMaxRange("e1", 10000.0f);

    assertTrue(sys.addEntry("e1", "s1", "Close", ET::Ship, 5000.0f), "Add within range");
    assertTrue(!sys.addEntry("e1", "s2", "Far", ET::Ship, 15000.0f), "Add beyond range rejected");
    assertTrue(sys.getEntryCount("e1") == 1, "Only 1 entry");
}

static void testOverviewUpdate() {
    std::cout << "\n=== Overview: Update ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.update(1.0f);
    sys.update(2.5f);
    assertTrue(sys.getEntryCount("e1") == 0, "Still zero after update");
}

static void testOverviewMissing() {
    std::cout << "\n=== Overview: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::OverviewSystem sys(&world);

    using ET = components::OverviewState::EntryType;
    using SF = components::OverviewState::SortField;

    assertTrue(!sys.initialize("m"), "Init fails");
    assertTrue(!sys.addEntry("m", "s1", "X", ET::Ship, 100.0f), "AddEntry fails");
    assertTrue(!sys.removeEntry("m", "s1"), "RemoveEntry fails");
    assertTrue(!sys.clearEntries("m"), "ClearEntries fails");
    assertTrue(!sys.updateEntryDistance("m", "s1", 1.0f), "UpdateDistance fails");
    assertTrue(!sys.updateEntrySpeed("m", "s1", 1.0f), "UpdateSpeed fails");
    assertTrue(!sys.setEntryHostile("m", "s1", true), "SetHostile fails");
    assertTrue(!sys.setEntryTargeted("m", "s1", true), "SetTargeted fails");
    assertTrue(!sys.createProfile("m", "p1", "X"), "CreateProfile fails");
    assertTrue(!sys.deleteProfile("m", "p1"), "DeleteProfile fails");
    assertTrue(!sys.activateProfile("m", "p1"), "ActivateProfile fails");
    assertTrue(!sys.addTypeToProfile("m", "p1", ET::Ship), "AddTypeToProfile fails");
    assertTrue(!sys.removeTypeFromProfile("m", "p1", ET::Ship), "RemoveTypeFromProfile fails");
    assertTrue(!sys.setSortField("m", SF::Name), "SetSortField fails");
    assertTrue(!sys.setSortAscending("m", true), "SetSortAscending fails");
    assertTrue(!sys.setMaxRange("m", 100.0f), "SetMaxRange fails");
    assertTrue(sys.getEntryCount("m") == 0, "getEntryCount returns 0");
    assertTrue(!sys.hasEntry("m", "s1"), "hasEntry returns false");
    assertTrue(approxEqual(sys.getEntryDistance("m", "s1"), 0.0f), "getEntryDistance returns 0");
    assertTrue(sys.getEntryName("m", "s1") == "", "getEntryName returns empty");
    assertTrue(!sys.isEntryHostile("m", "s1"), "isEntryHostile returns false");
    assertTrue(!sys.isEntryTargeted("m", "s1"), "isEntryTargeted returns false");
    assertTrue(sys.getActiveProfileId("m") == "", "getActiveProfileId returns empty");
    assertTrue(sys.getProfileCount("m") == 0, "getProfileCount returns 0");
    assertTrue(sys.getVisibleEntryCount("m") == 0, "getVisibleEntryCount returns 0");
    assertTrue(sys.getTotalEntriesTracked("m") == 0, "getTotalEntriesTracked returns 0");
    assertTrue(sys.getTotalEntriesRemoved("m") == 0, "getTotalEntriesRemoved returns 0");
    assertTrue(sys.getSortField("m") == SF::Distance, "getSortField returns Distance");
}

void run_overview_system_tests() {
    testOverviewInit();
    testOverviewAddEntry();
    testOverviewRemoveEntry();
    testOverviewClearEntries();
    testOverviewUpdateEntry();
    testOverviewEntryFlags();
    testOverviewProfile();
    testOverviewProfileFiltering();
    testOverviewConfiguration();
    testOverviewMaxRange();
    testOverviewUpdate();
    testOverviewMissing();
}
