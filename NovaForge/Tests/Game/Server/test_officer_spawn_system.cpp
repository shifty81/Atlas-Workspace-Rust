// Tests for: OfficerSpawnSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/officer_spawn_system.h"

using namespace atlas;
using OR = components::OfficerSpawnState::OfficerRank;
using OF = components::OfficerSpawnState::OfficerFaction;

// ==================== OfficerSpawnSystem Tests ====================

static void testOfficerSpawnInit() {
    std::cout << "\n=== OfficerSpawn: Init ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getOfficerCount("e1") == 0, "Zero officers initially");
    assertTrue(sys.getTotalSpawned("e1") == 0, "Zero total spawned");
    assertTrue(sys.getTotalDefeated("e1") == 0, "Zero total defeated");
    assertTrue(sys.getSectorId("e1").empty(), "Empty sector id");
    assertTrue(approxEqual(sys.getBaseBounty("e1"), 10000.0f), "Default base bounty");
    assertTrue(approxEqual(sys.getDifficultyModifier("e1"), 1.0f), "Default difficulty");
    assertTrue(approxEqual(sys.getSpawnInterval("e1"), 600.0f), "Default spawn interval");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testOfficerSpawnSpawnOfficer() {
    std::cout << "\n=== OfficerSpawn: SpawnOfficer ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        sys.spawnOfficer("e1", "off1", OR::Commander, OF::Pirate, "loot_a"),
        "Spawn commander succeeds");
    assertTrue(sys.getOfficerCount("e1") == 1, "1 officer");
    assertTrue(sys.hasOfficer("e1", "off1"), "Has off1");
    assertTrue(sys.isOfficerActive("e1", "off1"), "off1 is active");
    assertTrue(sys.getTotalSpawned("e1") == 1, "1 total spawned");

    // Second officer
    assertTrue(
        sys.spawnOfficer("e1", "off2", OR::Admiral, OF::Military, "loot_b"),
        "Spawn admiral succeeds");
    assertTrue(sys.getOfficerCount("e1") == 2, "2 officers");

    // Duplicate id rejected
    assertTrue(
        !sys.spawnOfficer("e1", "off1", OR::Captain, OF::Rogue, "loot_c"),
        "Duplicate officer id rejected");
    assertTrue(sys.getOfficerCount("e1") == 2, "Still 2 officers");

    // Empty id rejected
    assertTrue(
        !sys.spawnOfficer("e1", "", OR::Lieutenant, OF::Elite, "loot_d"),
        "Empty id rejected");

    // Missing entity
    assertTrue(
        !sys.spawnOfficer("missing", "off3", OR::Captain, OF::Pirate, "loot_e"),
        "Spawn on missing entity fails");

    // Verify health for Commander (1000 * 1.0 difficulty)
    float bounty = sys.getOfficerBounty("e1", "off1");
    assertTrue(bounty > 0.0f, "off1 bounty > 0");
}

static void testOfficerSpawnDefeatOfficer() {
    std::cout << "\n=== OfficerSpawn: DefeatOfficer ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.spawnOfficer("e1", "off1", OR::Captain, OF::Mercenary, "loot_a");
    assertTrue(sys.defeatOfficer("e1", "off1"), "Defeat active officer");
    assertTrue(!sys.isOfficerActive("e1", "off1"), "off1 no longer active");
    assertTrue(sys.getTotalDefeated("e1") == 1, "1 total defeated");

    // Defeat already defeated
    assertTrue(!sys.defeatOfficer("e1", "off1"), "Defeat already-defeated fails");

    // Defeat missing officer
    assertTrue(!sys.defeatOfficer("e1", "nonexistent"), "Defeat missing officer fails");

    // Defeat on missing entity
    assertTrue(!sys.defeatOfficer("missing", "off1"), "Defeat on missing entity fails");

    // Defeat a despawned officer
    sys.spawnOfficer("e1", "off2", OR::Lieutenant, OF::Pirate, "loot_b");
    sys.despawnOfficer("e1", "off2");
    assertTrue(!sys.defeatOfficer("e1", "off2"), "Defeat despawned officer fails");

    assertTrue(sys.getTotalDefeated("e1") == 1, "Still 1 total defeated");
}

static void testOfficerSpawnDespawnOfficer() {
    std::cout << "\n=== OfficerSpawn: DespawnOfficer ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.spawnOfficer("e1", "off1", OR::Overlord, OF::Elite, "loot_a");
    assertTrue(sys.despawnOfficer("e1", "off1"), "Despawn active officer");
    assertTrue(!sys.isOfficerActive("e1", "off1"), "off1 no longer active");

    // Despawn already despawned
    assertTrue(!sys.despawnOfficer("e1", "off1"), "Despawn already-despawned fails");

    // Despawn defeated officer
    sys.spawnOfficer("e1", "off2", OR::Admiral, OF::Military, "loot_b");
    sys.defeatOfficer("e1", "off2");
    assertTrue(!sys.despawnOfficer("e1", "off2"), "Despawn defeated fails");

    // Despawn missing officer
    assertTrue(!sys.despawnOfficer("e1", "nonexistent"), "Despawn missing fails");

    // Despawn on missing entity
    assertTrue(!sys.despawnOfficer("missing", "off1"), "Despawn on missing entity fails");
}

static void testOfficerSpawnRemoveOfficer() {
    std::cout << "\n=== OfficerSpawn: RemoveOfficer ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.spawnOfficer("e1", "off1", OR::Commander, OF::Pirate, "loot_a");
    sys.spawnOfficer("e1", "off2", OR::Captain, OF::Rogue, "loot_b");

    assertTrue(sys.removeOfficer("e1", "off1"), "Remove off1");
    assertTrue(sys.getOfficerCount("e1") == 1, "1 officer after remove");
    assertTrue(!sys.hasOfficer("e1", "off1"), "off1 gone");
    assertTrue(!sys.removeOfficer("e1", "off1"), "Remove missing officer fails");

    assertTrue(sys.clearOfficers("e1"), "Clear officers");
    assertTrue(sys.getOfficerCount("e1") == 0, "Zero after clear");
    assertTrue(!sys.clearOfficers("missing"), "Clear on missing entity fails");
    assertTrue(!sys.removeOfficer("missing", "off1"), "Remove on missing entity fails");
}

static void testOfficerSpawnBountyCalc() {
    std::cout << "\n=== OfficerSpawn: BountyCalc ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // base_bounty=10000, difficulty=1.0, bounty_multiplier=2.0
    sys.spawnOfficer("e1", "off1", OR::Lieutenant, OF::Pirate, "loot_a");
    float expected = 2.0f * 10000.0f * 1.0f;
    assertTrue(approxEqual(sys.getOfficerBounty("e1", "off1"), expected),
               "Lieutenant bounty = 20000");

    // Change difficulty modifier
    sys.setDifficultyModifier("e1", 2.5f);
    expected = 2.0f * 10000.0f * 2.5f;
    assertTrue(approxEqual(sys.getOfficerBounty("e1", "off1"), expected),
               "Bounty after difficulty 2.5 = 50000");

    // Change base bounty
    sys.setBaseBounty("e1", 5000.0f);
    expected = 2.0f * 5000.0f * 2.5f;
    assertTrue(approxEqual(sys.getOfficerBounty("e1", "off1"), expected),
               "Bounty after base 5000 = 25000");

    // Bounty for missing officer
    assertTrue(approxEqual(sys.getOfficerBounty("e1", "nonexistent"), 0.0f),
               "Bounty for missing officer = 0");

    // Bounty on missing entity
    assertTrue(approxEqual(sys.getOfficerBounty("missing", "off1"), 0.0f),
               "Bounty on missing entity = 0");

    // Multiple officers different ranks
    sys.spawnOfficer("e1", "off2", OR::Overlord, OF::Elite, "loot_b");
    float b2 = sys.getOfficerBounty("e1", "off2");
    assertTrue(b2 > 0.0f, "Overlord bounty > 0");
    assertTrue(approxEqual(b2, 2.0f * 5000.0f * 2.5f),
               "Overlord same multiplier formula");

    // Defeated officer still has bounty (lookup by id)
    sys.defeatOfficer("e1", "off1");
    assertTrue(approxEqual(sys.getOfficerBounty("e1", "off1"),
               2.0f * 5000.0f * 2.5f), "Defeated officer still has bounty");
}

static void testOfficerSpawnConfiguration() {
    std::cout << "\n=== OfficerSpawn: Configuration ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setSectorId("e1", "Jita"), "Set sector id");
    assertTrue(sys.getSectorId("e1") == "Jita", "Sector id matches");
    assertTrue(!sys.setSectorId("e1", ""), "Empty sector id rejected");

    assertTrue(sys.setSpawnInterval("e1", 120.0f), "Set spawn interval 120");
    assertTrue(approxEqual(sys.getSpawnInterval("e1"), 120.0f), "Interval matches");
    assertTrue(!sys.setSpawnInterval("e1", 0.0f), "Zero interval rejected");
    assertTrue(!sys.setSpawnInterval("e1", -10.0f), "Negative interval rejected");

    assertTrue(sys.setMaxOfficers("e1", 10), "Set max officers 10");
    assertTrue(!sys.setMaxOfficers("e1", 0), "Zero max officers rejected");

    assertTrue(sys.setBaseBounty("e1", 50000.0f), "Set base bounty 50000");
    assertTrue(approxEqual(sys.getBaseBounty("e1"), 50000.0f), "Base bounty matches");
    assertTrue(!sys.setBaseBounty("e1", 0.0f), "Zero bounty rejected");
    assertTrue(!sys.setBaseBounty("e1", -100.0f), "Negative bounty rejected");

    assertTrue(sys.setDifficultyModifier("e1", 3.0f), "Set difficulty 3.0");
    assertTrue(approxEqual(sys.getDifficultyModifier("e1"), 3.0f), "Difficulty matches");
    assertTrue(!sys.setDifficultyModifier("e1", 0.0f), "Zero difficulty rejected");
    assertTrue(!sys.setDifficultyModifier("e1", -1.0f), "Negative difficulty rejected");

    // Missing entity
    assertTrue(!sys.setSectorId("missing", "s"), "SectorId on missing fails");
    assertTrue(!sys.setSpawnInterval("missing", 100.0f), "Interval on missing fails");
    assertTrue(!sys.setMaxOfficers("missing", 5), "MaxOfficers on missing fails");
    assertTrue(!sys.setBaseBounty("missing", 1000.0f), "BaseBounty on missing fails");
    assertTrue(!sys.setDifficultyModifier("missing", 1.0f), "Difficulty on missing fails");
}

static void testOfficerSpawnCountByRankFaction() {
    std::cout << "\n=== OfficerSpawn: CountByRank/Faction ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.spawnOfficer("e1", "off1", OR::Captain, OF::Pirate, "loot_a");
    sys.spawnOfficer("e1", "off2", OR::Captain, OF::Military, "loot_b");
    sys.spawnOfficer("e1", "off3", OR::Admiral, OF::Pirate, "loot_c");

    assertTrue(sys.getCountByRank("e1", OR::Captain) == 2, "2 captains");
    assertTrue(sys.getCountByRank("e1", OR::Admiral) == 1, "1 admiral");
    assertTrue(sys.getCountByRank("e1", OR::Overlord) == 0, "0 overlords");
    assertTrue(sys.getCountByFaction("e1", OF::Pirate) == 2, "2 pirates");
    assertTrue(sys.getCountByFaction("e1", OF::Military) == 1, "1 military");
    assertTrue(sys.getCountByRank("missing", OR::Captain) == 0,
               "CountByRank on missing returns 0");
    assertTrue(sys.getCountByFaction("missing", OF::Pirate) == 0,
               "CountByFaction on missing returns 0");
}

static void testOfficerSpawnCapacity() {
    std::cout << "\n=== OfficerSpawn: Capacity ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxOfficers("e1", 3);

    sys.spawnOfficer("e1", "off1", OR::Lieutenant, OF::Pirate, "loot_a");
    sys.spawnOfficer("e1", "off2", OR::Commander, OF::Mercenary, "loot_b");
    sys.spawnOfficer("e1", "off3", OR::Captain, OF::Rogue, "loot_c");
    assertTrue(sys.getOfficerCount("e1") == 3, "3 officers at capacity");

    assertTrue(
        !sys.spawnOfficer("e1", "off4", OR::Admiral, OF::Elite, "loot_d"),
        "off4 rejected at capacity 3");
    assertTrue(sys.getOfficerCount("e1") == 3, "Still 3 officers");

    // Remove one, then spawn should succeed again
    sys.removeOfficer("e1", "off1");
    assertTrue(
        sys.spawnOfficer("e1", "off4", OR::Admiral, OF::Elite, "loot_d"),
        "Spawn succeeds after removal");
    assertTrue(sys.getOfficerCount("e1") == 3, "Back to 3 officers");
}

static void testOfficerSpawnUpdate() {
    std::cout << "\n=== OfficerSpawn: Update ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.spawnOfficer("e1", "off1", OR::Commander, OF::Military, "loot_a");
    sys.update(10.0f);

    // Verify time_alive advanced
    assertTrue(sys.isOfficerActive("e1", "off1"), "off1 still active after 10s");

    // Large update but not quite 3600
    sys.update(3589.0f);
    assertTrue(sys.isOfficerActive("e1", "off1"), "off1 active at 3599s");

    // Push past 3600
    sys.update(2.0f);
    assertTrue(!sys.isOfficerActive("e1", "off1"), "off1 auto-despawned after 3601s");

    // Spawn another and verify elapsed increments
    sys.spawnOfficer("e1", "off2", OR::Lieutenant, OF::Pirate, "loot_b");
    sys.update(5.0f);
    assertTrue(sys.isOfficerActive("e1", "off2"), "off2 active after 5s");
    assertTrue(sys.hasOfficer("e1", "off2"), "off2 present");
}

static void testOfficerSpawnMissing() {
    std::cout << "\n=== OfficerSpawn: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::OfficerSpawnSystem sys(&world);

    assertTrue(sys.getOfficerCount("missing") == 0,
               "OfficerCount returns 0 for missing");
    assertTrue(!sys.hasOfficer("missing", "off1"),
               "hasOfficer returns false for missing");
    assertTrue(approxEqual(sys.getOfficerBounty("missing", "off1"), 0.0f),
               "OfficerBounty returns 0 for missing");
    assertTrue(!sys.isOfficerActive("missing", "off1"),
               "isOfficerActive returns false for missing");
    assertTrue(sys.getSectorId("missing").empty(),
               "SectorId returns empty for missing");
    assertTrue(sys.getTotalSpawned("missing") == 0,
               "TotalSpawned returns 0 for missing");
    assertTrue(sys.getTotalDefeated("missing") == 0,
               "TotalDefeated returns 0 for missing");
    assertTrue(sys.getCountByRank("missing", OR::Captain) == 0,
               "CountByRank returns 0 for missing");
    assertTrue(sys.getCountByFaction("missing", OF::Pirate) == 0,
               "CountByFaction returns 0 for missing");
    assertTrue(approxEqual(sys.getBaseBounty("missing"), 0.0f),
               "BaseBounty returns 0 for missing");
    assertTrue(approxEqual(sys.getDifficultyModifier("missing"), 0.0f),
               "DifficultyModifier returns 0 for missing");
    assertTrue(approxEqual(sys.getSpawnInterval("missing"), 0.0f),
               "SpawnInterval returns 0 for missing");
}

void run_officer_spawn_system_tests() {
    testOfficerSpawnInit();
    testOfficerSpawnSpawnOfficer();
    testOfficerSpawnDefeatOfficer();
    testOfficerSpawnDespawnOfficer();
    testOfficerSpawnRemoveOfficer();
    testOfficerSpawnBountyCalc();
    testOfficerSpawnConfiguration();
    testOfficerSpawnCountByRankFaction();
    testOfficerSpawnCapacity();
    testOfficerSpawnUpdate();
    testOfficerSpawnMissing();
}
