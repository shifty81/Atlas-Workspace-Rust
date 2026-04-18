// Tests for: CaptainMilestoneSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/captain_milestone_system.h"

using namespace atlas;
using MT = components::CaptainMilestoneState::MilestoneType;

static void testCaptainMilestoneInit() {
    std::cout << "\n=== CaptainMilestone: Init ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getMilestoneCount("e1") == 0, "Zero milestones");
    assertTrue(sys.getAchievedCount("e1") == 0, "Zero achieved");
    assertTrue(sys.getCareerPoints("e1") == 0, "Zero career points");
    assertTrue(sys.getCaptainRank("e1") == "Recruit", "Default rank Recruit");
    assertTrue(sys.getCaptainId("e1").empty(), "Empty captain_id");
    assertTrue(sys.getTotalAchieved("e1") == 0, "Zero total_achieved");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testCaptainMilestoneAddMilestone() {
    std::cout << "\n=== CaptainMilestone: AddMilestone ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addMilestone("e1", "m1", MT::FirstKill, "First kill", 5), "Add m1");
    assertTrue(sys.hasMilestone("e1", "m1"), "hasMilestone m1");
    assertTrue(sys.getMilestoneCount("e1") == 1, "1 milestone");
    assertTrue(!sys.isMilestoneAchieved("e1", "m1"), "m1 not achieved");
    assertTrue(sys.getMilestoneCareerPoints("e1", "m1") == 5, "m1 career_points = 5");

    assertTrue(sys.addMilestone("e1", "m2", MT::SurvivedAmbush, "Survived", 10), "Add m2");
    assertTrue(sys.getMilestoneCount("e1") == 2, "2 milestones");
    assertTrue(!sys.hasMilestone("e1", "nonexistent"), "hasMilestone false for nonexistent");
}

static void testCaptainMilestoneAchieve() {
    std::cout << "\n=== CaptainMilestone: Achieve ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addMilestone("e1", "m1", MT::FirstKill, "First kill", 5);
    assertTrue(sys.achieveMilestone("e1", "m1"), "Achieve m1");
    assertTrue(sys.isMilestoneAchieved("e1", "m1"), "m1 is achieved");
    assertTrue(sys.getCareerPoints("e1") == 5, "Career points = 5");
    assertTrue(sys.getTotalAchieved("e1") == 1, "total_achieved = 1");
    assertTrue(sys.getCaptainRank("e1") == "Ensign", "Rank is Ensign at 5 points");
    assertTrue(sys.getAchievedCount("e1") == 1, "getAchievedCount = 1");

    // Double achieve should fail
    assertTrue(!sys.achieveMilestone("e1", "m1"), "Double achieve rejected");
    assertTrue(!sys.achieveMilestone("e1", "nonexistent"), "Achieve nonexistent fails");
    assertTrue(!sys.achieveMilestone("missing", "m1"), "Achieve on missing entity fails");
}

static void testCaptainMilestoneReset() {
    std::cout << "\n=== CaptainMilestone: Reset ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addMilestone("e1", "m1", MT::FirstKill, "First kill", 15);
    sys.achieveMilestone("e1", "m1");
    assertTrue(sys.getCareerPoints("e1") == 15, "Points = 15 after achieve");
    assertTrue(sys.getCaptainRank("e1") == "Lieutenant", "Rank Lieutenant at 15");

    assertTrue(sys.resetMilestone("e1", "m1"), "Reset m1");
    assertTrue(!sys.isMilestoneAchieved("e1", "m1"), "m1 not achieved after reset");
    assertTrue(sys.getCareerPoints("e1") == 0, "Points = 0 after reset");
    assertTrue(sys.getTotalAchieved("e1") == 0, "total_achieved = 0");
    assertTrue(sys.getCaptainRank("e1") == "Recruit", "Rank Recruit after reset");

    // Reset unachieved
    assertTrue(!sys.resetMilestone("e1", "m1"), "Reset unachieved fails");
    assertTrue(!sys.resetMilestone("e1", "nonexistent"), "Reset nonexistent fails");
    assertTrue(!sys.resetMilestone("missing", "m1"), "Reset on missing entity fails");
}

static void testCaptainMilestoneRemove() {
    std::cout << "\n=== CaptainMilestone: Remove ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addMilestone("e1", "m1", MT::FirstKill, "First kill", 5);
    sys.addMilestone("e1", "m2", MT::LeadWing, "Lead wing", 10);
    sys.achieveMilestone("e1", "m2");

    assertTrue(sys.removeMilestone("e1", "m1"), "Remove unachieved m1");
    assertTrue(!sys.hasMilestone("e1", "m1"), "m1 gone");
    assertTrue(sys.getMilestoneCount("e1") == 1, "1 milestone remains");

    assertTrue(!sys.removeMilestone("e1", "m2"), "Remove achieved m2 rejected");
    assertTrue(sys.hasMilestone("e1", "m2"), "m2 still present");

    assertTrue(!sys.removeMilestone("e1", "nonexistent"), "Remove nonexistent fails");
    assertTrue(!sys.removeMilestone("missing", "m1"), "Remove on missing entity fails");
}

static void testCaptainMilestoneClear() {
    std::cout << "\n=== CaptainMilestone: Clear ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addMilestone("e1", "m1", MT::FirstKill, "First kill", 5);
    sys.addMilestone("e1", "m2", MT::LeadWing, "Lead wing", 10);
    sys.addMilestone("e1", "m3", MT::SurvivedAmbush, "Survived", 3);
    sys.achieveMilestone("e1", "m2");

    assertTrue(sys.clearMilestones("e1"), "clearMilestones succeeds");
    // m2 was achieved, should remain; m1 and m3 removed
    assertTrue(sys.getMilestoneCount("e1") == 1, "1 achieved milestone remains");
    assertTrue(sys.hasMilestone("e1", "m2"), "m2 (achieved) remains");
    assertTrue(!sys.hasMilestone("e1", "m1"), "m1 (unachieved) removed");
    assertTrue(!sys.hasMilestone("e1", "m3"), "m3 (unachieved) removed");

    assertTrue(!sys.clearMilestones("missing"), "clearMilestones on missing fails");
}

static void testCaptainMilestoneRanks() {
    std::cout << "\n=== CaptainMilestone: Ranks ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.getCaptainRank("e1") == "Recruit", "Rank Recruit at 0");

    sys.addMilestone("e1", "m1", MT::FirstKill, "m1", 5);
    sys.achieveMilestone("e1", "m1");
    assertTrue(sys.getCaptainRank("e1") == "Ensign", "Rank Ensign at 5");

    sys.addMilestone("e1", "m2", MT::LeadWing, "m2", 10);
    sys.achieveMilestone("e1", "m2");
    assertTrue(sys.getCaptainRank("e1") == "Lieutenant", "Rank Lieutenant at 15");

    sys.addMilestone("e1", "m3", MT::FullCampaign, "m3", 15);
    sys.achieveMilestone("e1", "m3");
    assertTrue(sys.getCaptainRank("e1") == "Commander", "Rank Commander at 30");

    sys.addMilestone("e1", "m4", MT::HeroicCharge, "m4", 20);
    sys.achieveMilestone("e1", "m4");
    assertTrue(sys.getCaptainRank("e1") == "Captain", "Rank Captain at 50");

    sys.addMilestone("e1", "m5", MT::SeniorCaptain, "m5", 50);
    sys.achieveMilestone("e1", "m5");
    assertTrue(sys.getCaptainRank("e1") == "Admiral", "Rank Admiral at 100");
}

static void testCaptainMilestoneValidation() {
    std::cout << "\n=== CaptainMilestone: Validation ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.addMilestone("e1", "", MT::FirstKill, "desc", 5), "Empty id rejected");
    assertTrue(!sys.addMilestone("e1", "m1", MT::FirstKill, "", 5), "Empty description rejected");
    assertTrue(!sys.addMilestone("e1", "m1", MT::FirstKill, "desc", -1), "Negative points rejected");
    assertTrue(sys.addMilestone("e1", "m1", MT::FirstKill, "desc", 0), "Zero points allowed");
    assertTrue(!sys.addMilestone("e1", "m1", MT::FirstKill, "desc", 5), "Duplicate id rejected");
    assertTrue(!sys.addMilestone("missing", "m2", MT::FirstKill, "desc", 5), "Add on missing fails");

    // Capacity cap
    for (int i = 2; i <= 20; ++i) {
        sys.addMilestone("e1", "m" + std::to_string(i), MT::FirstKill, "desc", 1);
    }
    assertTrue(sys.getMilestoneCount("e1") == 20, "Capacity at 20");
    assertTrue(!sys.addMilestone("e1", "overflow", MT::FirstKill, "desc", 1), "Overflow rejected");
}

static void testCaptainMilestoneAllTypes() {
    std::cout << "\n=== CaptainMilestone: AllTypes ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addMilestone("e1", "t1", MT::FirstKill, "FirstKill", 1);
    sys.addMilestone("e1", "t2", MT::SurvivedAmbush, "SurvivedAmbush", 1);
    sys.addMilestone("e1", "t3", MT::LeadWing, "LeadWing", 1);
    sys.addMilestone("e1", "t4", MT::ActingFleetCommander, "ActingFleetCommander", 1);
    sys.addMilestone("e1", "t5", MT::FullCampaign, "FullCampaign", 1);
    sys.addMilestone("e1", "t6", MT::FirstSave, "FirstSave", 1);
    sys.addMilestone("e1", "t7", MT::EscapeArtist, "EscapeArtist", 1);
    sys.addMilestone("e1", "t8", MT::HeroicCharge, "HeroicCharge", 1);
    sys.addMilestone("e1", "t9", MT::LoyaltyTest, "LoyaltyTest", 1);
    sys.addMilestone("e1", "t10", MT::SeniorCaptain, "SeniorCaptain", 1);

    assertTrue(sys.getMilestoneCount("e1") == 10, "10 milestones of all types");
    assertTrue(sys.getCountByType("e1", MT::FirstKill) == 1, "1 FirstKill");
    assertTrue(sys.getCountByType("e1", MT::SurvivedAmbush) == 1, "1 SurvivedAmbush");
    assertTrue(sys.getCountByType("e1", MT::LeadWing) == 1, "1 LeadWing");
    assertTrue(sys.getCountByType("e1", MT::ActingFleetCommander) == 1, "1 ActingFleetCommander");
    assertTrue(sys.getCountByType("e1", MT::FullCampaign) == 1, "1 FullCampaign");
    assertTrue(sys.getCountByType("e1", MT::SeniorCaptain) == 1, "1 SeniorCaptain");
}

static void testCaptainMilestoneCaptainId() {
    std::cout << "\n=== CaptainMilestone: CaptainId ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setCaptainId("e1", "captain-007"), "Set captain_id");
    assertTrue(sys.getCaptainId("e1") == "captain-007", "captain_id matches");
    assertTrue(!sys.setCaptainId("e1", ""), "Empty captain_id rejected");
    assertTrue(sys.getCaptainId("e1") == "captain-007", "captain_id unchanged");
    assertTrue(!sys.setCaptainId("missing", "x"), "setCaptainId on missing fails");
}

static void testCaptainMilestoneCountByType() {
    std::cout << "\n=== CaptainMilestone: CountByType ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addMilestone("e1", "a1", MT::FirstKill, "fk1", 1);
    sys.addMilestone("e1", "a2", MT::FirstKill, "fk2", 1);
    sys.addMilestone("e1", "a3", MT::SurvivedAmbush, "sa1", 1);

    assertTrue(sys.getCountByType("e1", MT::FirstKill) == 2, "2 FirstKill");
    assertTrue(sys.getCountByType("e1", MT::SurvivedAmbush) == 1, "1 SurvivedAmbush");
    assertTrue(sys.getCountByType("e1", MT::LeadWing) == 0, "0 LeadWing");
    assertTrue(sys.getCountByType("missing", MT::FirstKill) == 0, "CountByType on missing = 0");
}

static void testCaptainMilestoneMissingEntity() {
    std::cout << "\n=== CaptainMilestone: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CaptainMilestoneSystem sys(&world);

    assertTrue(!sys.isMilestoneAchieved("missing", "m1"), "isMilestoneAchieved = false");
    assertTrue(sys.getMilestoneCount("missing") == 0, "getMilestoneCount = 0");
    assertTrue(sys.getAchievedCount("missing") == 0, "getAchievedCount = 0");
    assertTrue(sys.getCareerPoints("missing") == 0, "getCareerPoints = 0");
    assertTrue(sys.getCaptainRank("missing").empty(), "getCaptainRank = ''");
    assertTrue(!sys.hasMilestone("missing", "m1"), "hasMilestone = false");
    assertTrue(sys.getMilestoneCareerPoints("missing", "m1") == 0, "getMilestoneCareerPoints = 0");
    assertTrue(sys.getCaptainId("missing").empty(), "getCaptainId = ''");
    assertTrue(sys.getCountByType("missing", MT::FirstKill) == 0, "getCountByType = 0");
    assertTrue(sys.getTotalAchieved("missing") == 0, "getTotalAchieved = 0");
    assertTrue(!sys.achieveMilestone("missing", "m1"), "achieveMilestone on missing fails");
    assertTrue(!sys.resetMilestone("missing", "m1"), "resetMilestone on missing fails");
    assertTrue(!sys.removeMilestone("missing", "m1"), "removeMilestone on missing fails");
    assertTrue(!sys.clearMilestones("missing"), "clearMilestones on missing fails");
    assertTrue(!sys.setCaptainId("missing", "x"), "setCaptainId on missing fails");
}

void run_captain_milestone_system_tests() {
    testCaptainMilestoneInit();
    testCaptainMilestoneAddMilestone();
    testCaptainMilestoneAchieve();
    testCaptainMilestoneReset();
    testCaptainMilestoneRemove();
    testCaptainMilestoneClear();
    testCaptainMilestoneRanks();
    testCaptainMilestoneValidation();
    testCaptainMilestoneAllTypes();
    testCaptainMilestoneCaptainId();
    testCaptainMilestoneCountByType();
    testCaptainMilestoneMissingEntity();
}
