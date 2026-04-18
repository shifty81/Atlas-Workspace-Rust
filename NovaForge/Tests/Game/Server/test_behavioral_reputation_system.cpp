// Tests for: BehavioralReputationSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/behavioral_reputation_system.h"

using namespace atlas;
using BType = components::BehavioralReputationState::BehaviorType;

static void testBehavioralReputationInit() {
    std::cout << "\n=== BehavioralReputation: Init ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getRecordCount("e1") == 0, "0 records initially");
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), 0.0f), "Generosity 0 initially");
    assertTrue(approxEqual(sys.getLoyaltyScore("e1"), 0.0f), "Loyalty 0 initially");
    assertTrue(approxEqual(sys.getSalvageScore("e1"), 0.0f), "Salvage 0 initially");
    assertTrue(approxEqual(sys.getDistressScore("e1"), 0.0f), "Distress 0 initially");
    assertTrue(approxEqual(sys.getOverallReputation("e1"), 0.0f), "Overall reputation 0 initially");
    assertTrue(sys.getTotalRecordsEver("e1") == 0, "0 total records ever");
    assertTrue(sys.getPlayerId("e1").empty(), "Player id empty initially");
    assertTrue(sys.getMaxRecords("e1") == 50, "Default max records 50");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testBehavioralReputationRecordBehavior() {
    std::cout << "\n=== BehavioralReputation: RecordBehavior ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Positive generosity: RescueShip
    assertTrue(sys.recordBehavior("e1", "r1", BType::RescueShip, 5.0f), "RescueShip recorded");
    assertTrue(sys.getRecordCount("e1") == 1, "1 record");
    assertTrue(sys.hasRecord("e1", "r1"), "r1 exists");
    assertTrue(approxEqual(sys.getImpact("e1", "r1"), 5.0f), "Impact 5.0 for r1");
    assertTrue(sys.getOccurrenceCount("e1", "r1") == 1, "Occurrence count 1");
    assertTrue(sys.getTotalRecordsEver("e1") == 1, "1 total ever");
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), 5.0f), "Generosity 5.0 after RescueShip");

    // Negative generosity: HoardResources
    assertTrue(sys.recordBehavior("e1", "r2", BType::HoardResources, -3.0f), "HoardResources recorded");
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), 2.0f), "Generosity 2.0 after HoardResources(-3)");

    // Duplicate record_id: increments occurrence count, no score re-add
    assertTrue(sys.recordBehavior("e1", "r1", BType::RescueShip, 5.0f), "Duplicate r1 increments count");
    assertTrue(sys.getOccurrenceCount("e1", "r1") == 2, "Occurrence count 2 after re-record");
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), 2.0f), "Generosity unchanged on duplicate");
    assertTrue(sys.getTotalRecordsEver("e1") == 3, "3 total ever (r1 twice, r2 once)");

    // Empty record_id rejected
    assertTrue(!sys.recordBehavior("e1", "", BType::HelpAlly, 1.0f), "Empty record_id rejected");

    // Missing entity
    assertTrue(!sys.recordBehavior("missing", "x", BType::HelpAlly, 1.0f), "Missing entity fails");
}

static void testBehavioralReputationScoreAxes() {
    std::cout << "\n=== BehavioralReputation: ScoreAxes ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Loyalty: HelpAlly(+), OvercommitAlly(-), FriendlyFire(-), TacticalRetreat(-)
    sys.recordBehavior("e1", "help1", BType::HelpAlly, 4.0f);
    // HelpAlly adds to both generosity and loyalty
    assertTrue(approxEqual(sys.getLoyaltyScore("e1"), 4.0f), "Loyalty 4 after HelpAlly");
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), 4.0f), "Generosity 4 after HelpAlly");

    sys.recordBehavior("e1", "oc1", BType::OvercommitAlly, -6.0f);
    assertTrue(approxEqual(sys.getLoyaltyScore("e1"), -2.0f), "Loyalty -2 after OvercommitAlly");
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), 4.0f), "Generosity unchanged");

    sys.recordBehavior("e1", "ff1", BType::FriendlyFire, -3.0f);
    assertTrue(approxEqual(sys.getLoyaltyScore("e1"), -5.0f), "Loyalty -5 after FriendlyFire");

    sys.recordBehavior("e1", "tr1", BType::TacticalRetreat, -1.0f);
    assertTrue(approxEqual(sys.getLoyaltyScore("e1"), -6.0f), "Loyalty -6 after TacticalRetreat");

    // Salvage: SalvageField(+), AbandonWreck(-)
    sys.recordBehavior("e1", "sf1", BType::SalvageField, 2.0f);
    assertTrue(approxEqual(sys.getSalvageScore("e1"), 2.0f), "Salvage 2 after SalvageField");

    sys.recordBehavior("e1", "aw1", BType::AbandonWreck, -4.0f);
    assertTrue(approxEqual(sys.getSalvageScore("e1"), -2.0f), "Salvage -2 after AbandonWreck");

    // Distress: RespondDistress(+), IgnoreDistress(-)
    sys.recordBehavior("e1", "rd1", BType::RespondDistress, 3.0f);
    assertTrue(approxEqual(sys.getDistressScore("e1"), 3.0f), "Distress 3 after RespondDistress");

    sys.recordBehavior("e1", "id1", BType::IgnoreDistress, -5.0f);
    assertTrue(approxEqual(sys.getDistressScore("e1"), -2.0f), "Distress -2 after IgnoreDistress");
}

static void testBehavioralReputationRemoveBehavior() {
    std::cout << "\n=== BehavioralReputation: RemoveBehavior ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.recordBehavior("e1", "r1", BType::RescueShip, 5.0f);
    sys.recordBehavior("e1", "r2", BType::HoardResources, -3.0f);
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), 2.0f), "Generosity 2 before remove");
    assertTrue(sys.getRecordCount("e1") == 2, "2 records before remove");

    // Remove r1
    assertTrue(sys.removeBehavior("e1", "r1"), "Remove r1 succeeds");
    assertTrue(!sys.hasRecord("e1", "r1"), "r1 gone");
    assertTrue(sys.getRecordCount("e1") == 1, "1 record after remove");
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), -3.0f), "Generosity -3 after removing RescueShip(+5)");

    // Remove non-existent
    assertTrue(!sys.removeBehavior("e1", "nothere"), "Remove missing fails");

    // Remove on missing entity
    assertTrue(!sys.removeBehavior("missing", "r2"), "Remove on missing entity fails");
}

static void testBehavioralReputationClearRecords() {
    std::cout << "\n=== BehavioralReputation: ClearRecords ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.recordBehavior("e1", "r1", BType::RescueShip, 5.0f);
    sys.recordBehavior("e1", "r2", BType::HelpAlly, 3.0f);
    sys.recordBehavior("e1", "r3", BType::SalvageField, 2.0f);
    assertTrue(sys.getRecordCount("e1") == 3, "3 records before clear");

    assertTrue(sys.clearRecords("e1"), "Clear succeeds");
    assertTrue(sys.getRecordCount("e1") == 0, "0 records after clear");
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), 0.0f), "Generosity 0 after clear");
    assertTrue(approxEqual(sys.getLoyaltyScore("e1"), 0.0f), "Loyalty 0 after clear");
    assertTrue(approxEqual(sys.getSalvageScore("e1"), 0.0f), "Salvage 0 after clear");
    assertTrue(approxEqual(sys.getDistressScore("e1"), 0.0f), "Distress 0 after clear");

    // total_records_ever is NOT reset
    assertTrue(sys.getTotalRecordsEver("e1") == 3, "total_records_ever preserved after clear");

    // Missing entity
    assertTrue(!sys.clearRecords("missing"), "clearRecords missing fails");
}

static void testBehavioralReputationCapacity() {
    std::cout << "\n=== BehavioralReputation: Capacity ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxRecords("e1", 3);

    sys.recordBehavior("e1", "r1", BType::RescueShip, 5.0f);
    sys.recordBehavior("e1", "r2", BType::HoardResources, -3.0f);
    sys.recordBehavior("e1", "r3", BType::SalvageField, 2.0f);
    assertTrue(sys.getRecordCount("e1") == 3, "3 records at capacity");

    // Adding r4 should auto-purge r1 (oldest)
    sys.recordBehavior("e1", "r4", BType::HelpAlly, 4.0f);
    assertTrue(sys.getRecordCount("e1") == 3, "Still 3 records after auto-purge");
    assertTrue(!sys.hasRecord("e1", "r1"), "r1 purged");
    assertTrue(sys.hasRecord("e1", "r4"), "r4 added");
    // r1 was RescueShip(+5) -> generosity should have r1 removed
    // Now: r2(-3)+r4(+4,generosity+loyalty)+r3(+2,salvage)
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), 1.0f, 0.01f),
               "Generosity after purge: -3+4=1");
}

static void testBehavioralReputationCountByType() {
    std::cout << "\n=== BehavioralReputation: CountByType ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.recordBehavior("e1", "r1", BType::RescueShip, 5.0f);
    sys.recordBehavior("e1", "r2", BType::RescueShip, 3.0f);
    sys.recordBehavior("e1", "r3", BType::IgnoreDistress, -2.0f);

    assertTrue(sys.getCountByType("e1", BType::RescueShip) == 2, "2 RescueShip records");
    assertTrue(sys.getCountByType("e1", BType::IgnoreDistress) == 1, "1 IgnoreDistress record");
    assertTrue(sys.getCountByType("e1", BType::HelpAlly) == 0, "0 HelpAlly records");

    // Missing entity
    assertTrue(sys.getCountByType("missing", BType::RescueShip) == 0,
               "countByType missing returns 0");
}

static void testBehavioralReputationDominantType() {
    std::cout << "\n=== BehavioralReputation: DominantType ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Empty: no dominant
    assertTrue(sys.getDominantBehaviorType("e1").empty(),
               "Dominant empty on no records");

    // Add small impact
    sys.recordBehavior("e1", "r1", BType::HelpAlly, 1.0f);
    assertTrue(sys.getDominantBehaviorType("e1") == "HelpAlly",
               "HelpAlly dominant with only record");

    // Add larger impact AbandonWreck
    sys.recordBehavior("e1", "r2", BType::AbandonWreck, -8.0f);
    assertTrue(sys.getDominantBehaviorType("e1") == "AbandonWreck",
               "AbandonWreck dominant with larger abs impact");

    // Missing entity
    assertTrue(sys.getDominantBehaviorType("missing").empty(),
               "getDominantBehaviorType missing returns empty");
}

static void testBehavioralReputationOverallReputation() {
    std::cout << "\n=== BehavioralReputation: OverallReputation ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Add behaviors to all axes
    sys.recordBehavior("e1", "r1", BType::RescueShip, 4.0f);   // generosity +4
    sys.recordBehavior("e1", "r2", BType::HelpAlly, 4.0f);     // generosity+loyalty +4
    sys.recordBehavior("e1", "r3", BType::SalvageField, 4.0f); // salvage +4
    sys.recordBehavior("e1", "r4", BType::RespondDistress, 4.0f); // distress +4
    // generosity = 4+4=8, loyalty=4, salvage=4, distress=4
    float overall = sys.getOverallReputation("e1");
    assertTrue(approxEqual(overall, 5.0f, 0.01f),
               "Overall reputation = (8+4+4+4)/4 = 5.0");

    // Missing entity
    assertTrue(approxEqual(sys.getOverallReputation("missing"), 0.0f),
               "getOverallReputation missing returns 0");
}

static void testBehavioralReputationConfiguration() {
    std::cout << "\n=== BehavioralReputation: Configuration ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // setPlayerId valid
    assertTrue(sys.setPlayerId("e1", "player_alpha"), "setPlayerId succeeds");
    assertTrue(sys.getPlayerId("e1") == "player_alpha", "Player id set");

    // setPlayerId empty rejected
    assertTrue(!sys.setPlayerId("e1", ""), "setPlayerId empty fails");

    // setMaxRecords valid
    assertTrue(sys.setMaxRecords("e1", 10), "setMaxRecords 10 succeeds");
    assertTrue(sys.getMaxRecords("e1") == 10, "Max records is 10");

    // setMaxRecords invalid
    assertTrue(!sys.setMaxRecords("e1", 0), "setMaxRecords 0 fails");
    assertTrue(!sys.setMaxRecords("e1", -1), "setMaxRecords -1 fails");

    // Missing entity
    assertTrue(!sys.setPlayerId("missing", "p1"), "setPlayerId missing fails");
    assertTrue(!sys.setMaxRecords("missing", 5), "setMaxRecords missing fails");
}

static void testBehavioralReputationMissingEntity() {
    std::cout << "\n=== BehavioralReputation: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);

    assertTrue(approxEqual(sys.getGenerosityScore("missing"), 0.0f), "getGenerosityScore missing returns 0");
    assertTrue(approxEqual(sys.getLoyaltyScore("missing"), 0.0f), "getLoyaltyScore missing returns 0");
    assertTrue(approxEqual(sys.getSalvageScore("missing"), 0.0f), "getSalvageScore missing returns 0");
    assertTrue(approxEqual(sys.getDistressScore("missing"), 0.0f), "getDistressScore missing returns 0");
    assertTrue(approxEqual(sys.getOverallReputation("missing"), 0.0f), "getOverallReputation missing returns 0");
    assertTrue(sys.getRecordCount("missing") == 0, "getRecordCount missing returns 0");
    assertTrue(!sys.hasRecord("missing", "r1"), "hasRecord missing returns false");
    assertTrue(sys.getOccurrenceCount("missing", "r1") == 0, "getOccurrenceCount missing returns 0");
    assertTrue(approxEqual(sys.getImpact("missing", "r1"), 0.0f), "getImpact missing returns 0");
    assertTrue(sys.getTotalRecordsEver("missing") == 0, "getTotalRecordsEver missing returns 0");
    assertTrue(sys.getCountByType("missing", BType::RescueShip) == 0, "getCountByType missing returns 0");
    assertTrue(sys.getDominantBehaviorType("missing").empty(), "getDominantBehaviorType missing returns empty");
    assertTrue(sys.getPlayerId("missing").empty(), "getPlayerId missing returns empty");
    assertTrue(sys.getMaxRecords("missing") == 0, "getMaxRecords missing returns 0");
    assertTrue(!sys.removeBehavior("missing", "r1"), "removeBehavior missing returns false");
    assertTrue(!sys.clearRecords("missing"), "clearRecords missing returns false");
}

static void testBehavioralReputationTickUpdate() {
    std::cout << "\n=== BehavioralReputation: TickUpdate ===" << std::endl;
    ecs::World world;
    systems::BehavioralReputationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // System tick should not change scores
    sys.recordBehavior("e1", "r1", BType::RescueShip, 5.0f);
    float gen_before = sys.getGenerosityScore("e1");
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getGenerosityScore("e1"), gen_before),
               "Scores unchanged after tick");
    assertTrue(sys.getRecordCount("e1") == 1, "Record count unchanged after tick");
}

void run_behavioral_reputation_system_tests() {
    testBehavioralReputationInit();
    testBehavioralReputationRecordBehavior();
    testBehavioralReputationScoreAxes();
    testBehavioralReputationRemoveBehavior();
    testBehavioralReputationClearRecords();
    testBehavioralReputationCapacity();
    testBehavioralReputationCountByType();
    testBehavioralReputationDominantType();
    testBehavioralReputationOverallReputation();
    testBehavioralReputationConfiguration();
    testBehavioralReputationMissingEntity();
    testBehavioralReputationTickUpdate();
}
