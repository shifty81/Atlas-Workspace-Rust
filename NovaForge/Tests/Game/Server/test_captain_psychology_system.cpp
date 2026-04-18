// Tests for: CaptainPsychologySystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/captain_psychology_system.h"

using namespace atlas;

// ==================== Init Tests ====================

static void testInit() {
    std::cout << "\n=== CaptainPsychology: Init ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);

    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds on existing entity");

    // Check defaults
    assertTrue(approxEqual(sys.getAggression("e1"), 0.5f), "Default aggression is 0.5");
    assertTrue(approxEqual(sys.getCaution("e1"), 0.5f), "Default caution is 0.5");
    assertTrue(approxEqual(sys.getLoyalty("e1"), 0.5f), "Default loyalty is 0.5");
    assertTrue(approxEqual(sys.getGreed("e1"), 0.5f), "Default greed is 0.5");
    assertTrue(approxEqual(sys.getStress("e1"), 0.0f), "Default stress is 0");
    assertTrue(approxEqual(sys.getMood("e1"), 0.5f), "Default mood is 0.5");
    assertTrue(sys.getEventsProcessed("e1") == 0, "Default events_processed is 0");
    assertTrue(sys.getCaptainId("e1").empty(), "Default captain_id is empty");

    // Missing entity
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

// ==================== SetBaseline Tests ====================

static void testSetBaseline() {
    std::cout << "\n=== CaptainPsychology: SetBaseline ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setBaseline("e1", 0.8f, 0.3f, 0.9f, 0.1f), "Valid baselines accepted");

    // After resetToBaseline, current should match the new baselines
    sys.resetToBaseline("e1");
    assertTrue(approxEqual(sys.getAggression("e1"), 0.8f), "Aggression baseline applied");
    assertTrue(approxEqual(sys.getCaution("e1"), 0.3f), "Caution baseline applied");

    // Reject out-of-range
    assertTrue(!sys.setBaseline("e1", -0.1f, 0.5f, 0.5f, 0.5f), "Reject negative aggression");
    assertTrue(!sys.setBaseline("e1", 0.5f, 1.1f, 0.5f, 0.5f), "Reject caution > 1");
    assertTrue(!sys.setBaseline("missing", 0.5f, 0.5f, 0.5f, 0.5f), "Reject missing entity");
}

// ==================== SetCurrent Tests ====================

static void testSetCurrent() {
    std::cout << "\n=== CaptainPsychology: SetCurrent ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setCurrent("e1", 0.9f, 0.1f, 0.7f, 0.2f), "Valid current values accepted");
    assertTrue(approxEqual(sys.getAggression("e1"), 0.9f), "Aggression current set");
    assertTrue(approxEqual(sys.getCaution("e1"), 0.1f), "Caution current set");
    assertTrue(approxEqual(sys.getLoyalty("e1"), 0.7f), "Loyalty current set");

    // Reject out-of-range
    assertTrue(!sys.setCurrent("e1", 0.5f, 0.5f, -0.5f, 0.5f), "Reject negative loyalty");
    assertTrue(!sys.setCurrent("e1", 0.5f, 0.5f, 0.5f, 2.0f), "Reject greed > 1");
    assertTrue(!sys.setCurrent("missing", 0.5f, 0.5f, 0.5f, 0.5f), "Reject missing entity");
}

// ==================== ProcessEvent Tests ====================

static void testProcessEvent() {
    std::cout << "\n=== CaptainPsychology: ProcessEvent ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ET = components::CaptainPsychologyState::EventType;

    // Victory: aggression +0.05, caution -0.03
    float aggBefore = sys.getAggression("e1");
    float cautBefore = sys.getCaution("e1");
    assertTrue(sys.processEvent("e1", ET::Victory), "Victory event processed");
    assertTrue(sys.getAggression("e1") > aggBefore, "Victory increases aggression");
    assertTrue(sys.getCaution("e1") < cautBefore, "Victory decreases caution");

    // Defeat: caution +0.08, stress +0.15
    float cautBefore2 = sys.getCaution("e1");
    float stressBefore = sys.getStress("e1");
    assertTrue(sys.processEvent("e1", ET::Defeat), "Defeat event processed");
    assertTrue(sys.getCaution("e1") > cautBefore2, "Defeat increases caution");
    assertTrue(sys.getStress("e1") > stressBefore, "Defeat increases stress");

    // LootGained: greed +0.05
    float greedBefore = sys.getGreed("e1");
    sys.processEvent("e1", ET::LootGained);
    assertTrue(sys.getGreed("e1") > greedBefore, "LootGained increases greed");

    // AllyLost: loyalty +0.05, stress +0.1
    float loyalBefore = sys.getLoyalty("e1");
    float stressBefore2 = sys.getStress("e1");
    sys.processEvent("e1", ET::AllyLost);
    assertTrue(sys.getLoyalty("e1") > loyalBefore, "AllyLost increases loyalty");
    assertTrue(sys.getStress("e1") > stressBefore2, "AllyLost increases stress");

    // OrderGiven: loyalty +0.02
    float loyalBefore2 = sys.getLoyalty("e1");
    sys.processEvent("e1", ET::OrderGiven);
    assertTrue(sys.getLoyalty("e1") > loyalBefore2, "OrderGiven increases loyalty");

    // LongIdle: aggression -0.02
    float aggBefore2 = sys.getAggression("e1");
    sys.processEvent("e1", ET::LongIdle);
    assertTrue(sys.getAggression("e1") < aggBefore2, "LongIdle decreases aggression");

    assertTrue(sys.getEventsProcessed("e1") == 6, "6 events processed total");
    assertTrue(sys.getTotalShifts("e1") == 6, "6 total shifts applied");

    // Missing entity
    assertTrue(!sys.processEvent("missing", ET::Victory), "Event fails on missing entity");
}

// ==================== Stress Management Tests ====================

static void testStressManagement() {
    std::cout << "\n=== CaptainPsychology: StressManagement ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.applyStress("e1", 0.3f), "Apply stress succeeds");
    assertTrue(approxEqual(sys.getStress("e1"), 0.3f), "Stress is 0.3");

    // Clamp at 1.0
    assertTrue(sys.applyStress("e1", 0.9f), "Apply more stress");
    assertTrue(approxEqual(sys.getStress("e1"), 1.0f), "Stress clamped at 1.0");

    // Reset stress
    assertTrue(sys.resetStress("e1"), "Reset stress succeeds");
    assertTrue(approxEqual(sys.getStress("e1"), 0.0f), "Stress reset to 0");

    // Reject negative/zero
    assertTrue(!sys.applyStress("e1", -0.1f), "Reject negative stress");
    assertTrue(!sys.applyStress("e1", 0.0f), "Reject zero stress");
    assertTrue(!sys.applyStress("missing", 0.5f), "Apply stress fails on missing entity");
}

// ==================== Personality Drift Tests ====================

static void testPersonalityDrift() {
    std::cout << "\n=== CaptainPsychology: PersonalityDrift ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Set baseline to 0.5 (default), push current away
    sys.setCurrent("e1", 0.9f, 0.1f, 0.8f, 0.2f);
    sys.setDriftRate("e1", 0.1f);

    float aggBefore = sys.getAggression("e1");
    float cautBefore = sys.getCaution("e1");

    // Tick for 1 second: drift should pull toward baseline (0.5)
    sys.update(1.0f);

    assertTrue(sys.getAggression("e1") < aggBefore, "Aggression drifts down toward baseline");
    assertTrue(sys.getCaution("e1") > cautBefore, "Caution drifts up toward baseline");
    assertTrue(sys.getAggression("e1") > 0.5f, "Aggression still above baseline after 1s");
    assertTrue(sys.getCaution("e1") < 0.5f, "Caution still below baseline after 1s");

    // Drift a lot more — should converge toward baseline
    for (int i = 0; i < 50; ++i) sys.update(1.0f);
    assertTrue(approxEqual(sys.getAggression("e1"), 0.5f), "Aggression converged to baseline");
    assertTrue(approxEqual(sys.getCaution("e1"), 0.5f), "Caution converged to baseline");
}

// ==================== Mood Computation Tests ====================

static void testMoodComputation() {
    std::cout << "\n=== CaptainPsychology: MoodComputation ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Default mood
    assertTrue(approxEqual(sys.getMood("e1"), 0.5f), "Default mood is 0.5");

    // After Victory, mood increases
    using ET = components::CaptainPsychologyState::EventType;
    sys.processEvent("e1", ET::Victory);
    assertTrue(sys.getMood("e1") > 0.5f, "Mood increases after victory");

    // After Defeat, mood decreases
    float moodBefore = sys.getMood("e1");
    sys.processEvent("e1", ET::Defeat);
    assertTrue(sys.getMood("e1") < moodBefore, "Mood decreases after defeat");

    // Stress lowers mood on tick: apply stress, tick, check mood
    sys.applyStress("e1", 0.5f);
    sys.setStressDecay("e1", 0.001f); // very slow decay so stress stays
    sys.update(0.01f);
    float moodWithStress = sys.getMood("e1");
    sys.resetStress("e1");
    sys.update(0.01f);
    float moodNoStress = sys.getMood("e1");
    assertTrue(moodNoStress > moodWithStress, "Mood higher when stress is removed");
}

// ==================== Configuration Tests ====================

static void testConfiguration() {
    std::cout << "\n=== CaptainPsychology: Configuration ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // setDriftRate
    assertTrue(sys.setDriftRate("e1", 0.5f), "Valid drift rate accepted");
    assertTrue(!sys.setDriftRate("e1", 0.0f), "Reject zero drift rate");
    assertTrue(!sys.setDriftRate("e1", 1.5f), "Reject drift rate > 1");

    // setStressDecay
    assertTrue(sys.setStressDecay("e1", 0.2f), "Valid stress decay accepted");
    assertTrue(!sys.setStressDecay("e1", -0.1f), "Reject negative stress decay");

    // setCaptainId
    assertTrue(sys.setCaptainId("e1", "cpt_01"), "Valid captain id accepted");
    assertTrue(sys.getCaptainId("e1") == "cpt_01", "Captain id is cpt_01");
    assertTrue(!sys.setCaptainId("e1", ""), "Reject empty captain id");
    assertTrue(!sys.setCaptainId("missing", "cpt"), "Reject missing entity");
}

// ==================== Missing Entity Tests ====================

static void testMissingEntity() {
    std::cout << "\n=== CaptainPsychology: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);

    // All queries return safe defaults
    assertTrue(approxEqual(sys.getAggression("nope"), 0.0f), "Missing aggression returns 0");
    assertTrue(approxEqual(sys.getCaution("nope"), 0.0f), "Missing caution returns 0");
    assertTrue(approxEqual(sys.getLoyalty("nope"), 0.0f), "Missing loyalty returns 0");
    assertTrue(approxEqual(sys.getGreed("nope"), 0.0f), "Missing greed returns 0");
    assertTrue(approxEqual(sys.getMood("nope"), 0.0f), "Missing mood returns 0");
    assertTrue(approxEqual(sys.getStress("nope"), 0.0f), "Missing stress returns 0");
    assertTrue(sys.getEventsProcessed("nope") == 0, "Missing events_processed returns 0");
    assertTrue(sys.getTotalShifts("nope") == 0, "Missing total_shifts returns 0");
    assertTrue(!sys.isAggressive("nope"), "Missing isAggressive returns false");
    assertTrue(!sys.isCautious("nope"), "Missing isCautious returns false");
    assertTrue(!sys.isLoyal("nope"), "Missing isLoyal returns false");
    assertTrue(!sys.isGreedy("nope"), "Missing isGreedy returns false");
    assertTrue(!sys.isStressed("nope"), "Missing isStressed returns false");
    assertTrue(sys.getCaptainId("nope").empty(), "Missing captain_id returns empty");

    // Mutators fail on missing entity
    assertTrue(!sys.resetStress("nope"), "resetStress fails on missing entity");
    assertTrue(!sys.resetToBaseline("nope"), "resetToBaseline fails on missing entity");
}

// ==================== ResetToBaseline Tests ====================

static void testResetToBaseline() {
    std::cout << "\n=== CaptainPsychology: ResetToBaseline ===" << std::endl;
    ecs::World world;
    systems::CaptainPsychologySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Set custom baselines and push current away
    sys.setBaseline("e1", 0.3f, 0.7f, 0.4f, 0.6f);
    sys.setCurrent("e1", 0.9f, 0.1f, 0.8f, 0.2f);
    sys.applyStress("e1", 0.8f);

    assertTrue(sys.resetToBaseline("e1"), "resetToBaseline succeeds");
    assertTrue(approxEqual(sys.getAggression("e1"), 0.3f), "Aggression reset to baseline");
    assertTrue(approxEqual(sys.getCaution("e1"), 0.7f), "Caution reset to baseline");
    assertTrue(approxEqual(sys.getLoyalty("e1"), 0.4f), "Loyalty reset to baseline");
    assertTrue(approxEqual(sys.getGreed("e1"), 0.6f), "Greed reset to baseline");
    assertTrue(approxEqual(sys.getStress("e1"), 0.0f), "Stress reset to 0");
    assertTrue(approxEqual(sys.getMood("e1"), 0.5f), "Mood reset to 0.5");
}

// ==================== Test Runner ====================

void run_captain_psychology_system_tests() {
    testInit();
    testSetBaseline();
    testSetCurrent();
    testProcessEvent();
    testStressManagement();
    testPersonalityDrift();
    testMoodComputation();
    testConfiguration();
    testMissingEntity();
    testResetToBaseline();
}
