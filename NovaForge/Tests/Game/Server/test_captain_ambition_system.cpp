// Tests for: CaptainAmbitionSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/captain_ambition_system.h"

using namespace atlas;
using AT = components::AmbitionType;

static void testCaptainAmbitionInit() {
    std::cout << "\n=== CaptainAmbition: Init ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getAmbitionCount("e1") == 0, "No ambitions initially");
    assertTrue(approxEqual(sys.getDepartureRiskContrib("e1"), 0.0f), "No departure risk");
    assertTrue(sys.getAchievedCount("e1") == 0, "Zero achieved");
    assertTrue(sys.getBlockedCount("e1") == 0, "Zero blocked");
    assertTrue(sys.getTotalAmbitionsSet("e1") == 0, "Zero total set");
    assertTrue(sys.getTotalAchieved("e1") == 0, "Zero total achieved");
    assertTrue(sys.getTotalBlocked("e1") == 0, "Zero total blocked");
    assertTrue(sys.getCaptainId("e1").empty(), "Captain id empty");
    assertTrue(approxEqual(sys.getFrustrationDecayRate("e1"), 0.002f), "Default decay rate");
    assertTrue(sys.getMaxAmbitions("e1") == 5, "Default max ambitions 5");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testCaptainAmbitionAdd() {
    std::cout << "\n=== CaptainAmbition: Add ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addAmbition("e1", "a1", AT::LeadWing, "Lead a wing in battle", 1.0f),
               "Add lead wing ambition");
    assertTrue(sys.getAmbitionCount("e1") == 1, "One ambition");
    assertTrue(sys.hasAmbition("e1", "a1"), "Has a1");
    assertTrue(approxEqual(sys.getProgress("e1", "a1"), 0.0f), "Progress = 0");
    assertTrue(!sys.isAchieved("e1", "a1"), "Not achieved");
    assertTrue(!sys.isBlocked("e1", "a1"), "Not blocked");
    assertTrue(sys.getAmbitionType("e1", "a1") == AT::LeadWing, "Type is LeadWing");
    assertTrue(sys.getTotalAmbitionsSet("e1") == 1, "Total set = 1");
    assertTrue(approxEqual(sys.getFrustrationLevel("e1", "a1"), 0.0f), "No frustration");

    // Add different types
    assertTrue(sys.addAmbition("e1", "a2", AT::BecomeAce, "Get 50 kills", 50.0f),
               "Add BecomeAce with target 50");
    assertTrue(sys.addAmbition("e1", "a3", AT::CommandCapital, "Command a dreadnought", 1.0f),
               "Add CommandCapital");
    assertTrue(sys.getCountByType("e1", AT::LeadWing) == 1, "One LeadWing");
    assertTrue(sys.getCountByType("e1", AT::BecomeAce) == 1, "One BecomeAce");

    // Duplicate id rejected
    assertTrue(!sys.addAmbition("e1", "a1", AT::FleetCommander, "", 1.0f), "Duplicate rejected");
    // Empty id rejected
    assertTrue(!sys.addAmbition("e1", "", AT::LeadWing, "", 1.0f), "Empty id rejected");
    // Zero/negative target rejected
    assertTrue(!sys.addAmbition("e1", "a99", AT::LeadWing, "", 0.0f), "Zero target rejected");
    assertTrue(!sys.addAmbition("e1", "a99", AT::LeadWing, "", -1.0f), "Negative target rejected");
    assertTrue(!sys.addAmbition("missing", "x", AT::LeadWing, "", 1.0f), "Missing entity rejected");
}

static void testCaptainAmbitionAdvanceProgress() {
    std::cout << "\n=== CaptainAmbition: AdvanceProgress ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addAmbition("e1", "a1", AT::BecomeAce, "50 kills", 50.0f);

    assertTrue(sys.advanceProgress("e1", "a1", 10.0f), "Advance 10");
    assertTrue(approxEqual(sys.getProgress("e1", "a1"), 0.2f), "Progress = 0.2 (10/50)");

    assertTrue(sys.advanceProgress("e1", "a1", 30.0f), "Advance 30 more");
    assertTrue(approxEqual(sys.getProgress("e1", "a1"), 0.8f), "Progress = 0.8 (40/50)");

    // Advance past target caps at 1.0
    assertTrue(sys.advanceProgress("e1", "a1", 20.0f), "Advance past target");
    assertTrue(approxEqual(sys.getProgress("e1", "a1"), 1.0f), "Progress capped at 1.0");

    // Zero / negative amounts rejected
    assertTrue(!sys.advanceProgress("e1", "a1", 0.0f), "Zero amount rejected");
    assertTrue(!sys.advanceProgress("e1", "a1", -5.0f), "Negative amount rejected");

    // Advance on achieved ambition fails
    sys.markAchieved("e1", "a1");
    assertTrue(!sys.advanceProgress("e1", "a1", 1.0f), "Progress on achieved ambition fails");

    assertTrue(!sys.advanceProgress("missing", "a1", 1.0f), "Missing entity rejected");
}

static void testCaptainAmbitionMarkBlocked() {
    std::cout << "\n=== CaptainAmbition: MarkBlocked ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addAmbition("e1", "a1", AT::FleetCommander, "", 1.0f);

    assertTrue(sys.markBlocked("e1", "a1"), "Mark blocked");
    assertTrue(sys.isBlocked("e1", "a1"), "Is blocked");
    assertTrue(sys.getBlockedCount("e1") == 1, "Blocked count = 1");
    assertTrue(sys.getTotalBlocked("e1") == 1, "Total blocked = 1");

    // Double block fails
    assertTrue(!sys.markBlocked("e1", "a1"), "Double block fails");

    // Unblock
    assertTrue(sys.markUnblocked("e1", "a1"), "Mark unblocked");
    assertTrue(!sys.isBlocked("e1", "a1"), "No longer blocked");
    assertTrue(sys.getBlockedCount("e1") == 0, "Blocked count = 0");

    // Double unblock fails
    assertTrue(!sys.markUnblocked("e1", "a1"), "Double unblock fails");

    assertTrue(!sys.markBlocked("missing", "a1"), "Missing entity fails");
    assertTrue(!sys.markUnblocked("missing", "a1"), "Missing entity fails");
    assertTrue(!sys.markBlocked("e1", "nonexistent"), "Nonexistent ambition fails");
}

static void testCaptainAmbitionMarkAchieved() {
    std::cout << "\n=== CaptainAmbition: MarkAchieved ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addAmbition("e1", "a1", AT::RetireWithHonor, "", 1.0f);
    sys.markBlocked("e1", "a1"); // set blocked first

    assertTrue(sys.markAchieved("e1", "a1"), "Mark achieved");
    assertTrue(sys.isAchieved("e1", "a1"), "Is achieved");
    assertTrue(!sys.isBlocked("e1", "a1"), "No longer blocked when achieved");
    assertTrue(approxEqual(sys.getProgress("e1", "a1"), 1.0f), "Progress = 1.0 when achieved");
    assertTrue(approxEqual(sys.getFrustrationLevel("e1", "a1"), 0.0f), "Frustration = 0 when achieved");
    assertTrue(sys.getAchievedCount("e1") == 1, "Achieved count = 1");
    assertTrue(sys.getTotalAchieved("e1") == 1, "Total achieved = 1");
    // Departure risk should drop (frustration cleared)
    assertTrue(approxEqual(sys.getDepartureRiskContrib("e1"), 0.0f), "Departure risk 0 after achievement");

    // Double achieve fails
    assertTrue(!sys.markAchieved("e1", "a1"), "Double achieve fails");
    assertTrue(!sys.markAchieved("missing", "a1"), "Missing entity fails");
}

static void testCaptainAmbitionFrustrationAndDepartureRisk() {
    std::cout << "\n=== CaptainAmbition: FrustrationAndDepartureRisk ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addAmbition("e1", "a1", AT::LeadWing, "", 1.0f);
    sys.markBlocked("e1", "a1");

    // Tick to accumulate frustration
    sys.update(10.0f); // frustration += 0.01 * 10 = 0.1
    assertTrue(sys.getFrustrationLevel("e1", "a1") > 0.0f, "Frustration > 0 when blocked");
    assertTrue(sys.getDepartureRiskContrib("e1") > 0.0f, "Departure risk > 0");

    // Unblocking stops frustration accumulation
    sys.markUnblocked("e1", "a1");
    float beforeDecay = sys.getFrustrationLevel("e1", "a1");
    sys.update(100.0f); // frustration decays
    float afterDecay = sys.getFrustrationLevel("e1", "a1");
    assertTrue(afterDecay < beforeDecay, "Frustration decays when unblocked");
}

static void testCaptainAmbitionRemoveAndClear() {
    std::cout << "\n=== CaptainAmbition: RemoveAndClear ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.addAmbition("e1", "a1", AT::LeadWing, "", 1.0f);
    sys.addAmbition("e1", "a2", AT::BecomeAce, "", 50.0f);

    assertTrue(sys.removeAmbition("e1", "a1"), "Remove a1");
    assertTrue(!sys.hasAmbition("e1", "a1"), "a1 gone");
    assertTrue(sys.getAmbitionCount("e1") == 1, "One left");
    assertTrue(!sys.removeAmbition("e1", "a1"), "Already removed");
    assertTrue(!sys.removeAmbition("e1", "nonexistent"), "Nonexistent fails");

    assertTrue(sys.clearAmbitions("e1"), "Clear all");
    assertTrue(sys.getAmbitionCount("e1") == 0, "All cleared");
    assertTrue(approxEqual(sys.getDepartureRiskContrib("e1"), 0.0f), "Risk 0 after clear");
    assertTrue(!sys.clearAmbitions("missing"), "Clear missing fails");
}

static void testCaptainAmbitionCapacity() {
    std::cout << "\n=== CaptainAmbition: Capacity ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxAmbitions("e1", 3);

    assertTrue(sys.addAmbition("e1", "a1", AT::LeadWing, "", 1.0f), "Add 1");
    assertTrue(sys.addAmbition("e1", "a2", AT::BecomeAce, "", 50.0f), "Add 2");
    assertTrue(sys.addAmbition("e1", "a3", AT::FleetCommander, "", 1.0f), "Add 3");
    assertTrue(!sys.addAmbition("e1", "a4", AT::CommandCapital, "", 1.0f), "Add 4 fails");
    assertTrue(sys.getAmbitionCount("e1") == 3, "Still 3 ambitions");
}

static void testCaptainAmbitionConfiguration() {
    std::cout << "\n=== CaptainAmbition: Configuration ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setCaptainId("e1", "cap_01"), "Set captain id");
    assertTrue(sys.getCaptainId("e1") == "cap_01", "Captain id correct");
    assertTrue(!sys.setCaptainId("e1", ""), "Empty captain id rejected");

    assertTrue(sys.setFrustrationDecayRate("e1", 0.005f), "Set decay rate");
    assertTrue(approxEqual(sys.getFrustrationDecayRate("e1"), 0.005f), "Decay rate correct");
    assertTrue(!sys.setFrustrationDecayRate("e1", -0.1f), "Negative decay rejected");

    assertTrue(sys.setMaxAmbitions("e1", 3), "Set max ambitions");
    assertTrue(sys.getMaxAmbitions("e1") == 3, "Max ambitions = 3");
    assertTrue(!sys.setMaxAmbitions("e1", 0), "Zero max rejected");
}

static void testCaptainAmbitionMissingEntity() {
    std::cout << "\n=== CaptainAmbition: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);

    assertTrue(!sys.initialize("ghost"), "Init fails");
    assertTrue(sys.getAmbitionCount("ghost") == 0, "Count 0");
    assertTrue(approxEqual(sys.getDepartureRiskContrib("ghost"), 0.0f), "Risk 0");
    assertTrue(sys.getAchievedCount("ghost") == 0, "Achieved 0");
    assertTrue(sys.getBlockedCount("ghost") == 0, "Blocked 0");
    assertTrue(sys.getTotalAmbitionsSet("ghost") == 0, "Total set 0");
    assertTrue(sys.getTotalAchieved("ghost") == 0, "Total achieved 0");
    assertTrue(sys.getTotalBlocked("ghost") == 0, "Total blocked 0");
    assertTrue(sys.getCaptainId("ghost").empty(), "Captain id empty");
    assertTrue(!sys.hasAmbition("ghost", "a1"), "No ambition");
    assertTrue(approxEqual(sys.getProgress("ghost", "a1"), 0.0f), "Progress 0");
    assertTrue(!sys.isAchieved("ghost", "a1"), "Not achieved");
    assertTrue(!sys.isBlocked("ghost", "a1"), "Not blocked");
    assertTrue(!sys.addAmbition("ghost", "a1", AT::LeadWing, "", 1.0f), "addAmbition fails");
    assertTrue(!sys.advanceProgress("ghost", "a1", 1.0f), "advanceProgress fails");
    assertTrue(!sys.markBlocked("ghost", "a1"), "markBlocked fails");
    assertTrue(!sys.markAchieved("ghost", "a1"), "markAchieved fails");
    assertTrue(!sys.setCaptainId("ghost", "x"), "setCaptainId fails");
    assertTrue(sys.getCountByType("ghost", AT::LeadWing) == 0, "Count by type 0");
    assertTrue(approxEqual(sys.getFrustrationLevel("ghost", "a1"), 0.0f), "Frustration 0");
    assertTrue(sys.getMaxAmbitions("ghost") == 0, "Max ambitions 0");
    assertTrue(approxEqual(sys.getFrustrationDecayRate("ghost"), 0.0f), "Decay rate 0");
}

static void testCaptainAmbitionMultipleTypes() {
    std::cout << "\n=== CaptainAmbition: MultipleTypes ===" << std::endl;
    ecs::World world;
    systems::CaptainAmbitionSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addAmbition("e1", "a1", AT::CommandCapital, "", 1.0f);
    sys.addAmbition("e1", "a2", AT::LeadWing, "", 1.0f);
    sys.addAmbition("e1", "a3", AT::MentorJunior, "", 3.0f);
    sys.addAmbition("e1", "a4", AT::EarnMedal, "", 1.0f);
    sys.addAmbition("e1", "a5", AT::BuildReputation, "", 1000.0f);

    assertTrue(sys.getAmbitionCount("e1") == 5, "Five ambitions");
    assertTrue(sys.getTotalAmbitionsSet("e1") == 5, "Total set = 5");

    // Achieve one
    sys.markAchieved("e1", "a2");
    assertTrue(sys.getAchievedCount("e1") == 1, "One achieved");
    assertTrue(sys.getAmbitionCount("e1") == 5, "Still five ambitions");

    // Block two
    sys.markBlocked("e1", "a1");
    sys.markBlocked("e1", "a3");
    assertTrue(sys.getBlockedCount("e1") == 2, "Two blocked (non-achieved)");

    // Progress the reputation one
    sys.advanceProgress("e1", "a5", 500.0f);
    assertTrue(approxEqual(sys.getProgress("e1", "a5"), 0.5f), "Reputation at 50%");
}

void run_captain_ambition_system_tests() {
    testCaptainAmbitionInit();
    testCaptainAmbitionAdd();
    testCaptainAmbitionAdvanceProgress();
    testCaptainAmbitionMarkBlocked();
    testCaptainAmbitionMarkAchieved();
    testCaptainAmbitionFrustrationAndDepartureRisk();
    testCaptainAmbitionRemoveAndClear();
    testCaptainAmbitionCapacity();
    testCaptainAmbitionConfiguration();
    testCaptainAmbitionMissingEntity();
    testCaptainAmbitionMultipleTypes();
}
