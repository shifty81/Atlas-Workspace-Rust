// Tests for: CaptainMoodSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/captain_mood_system.h"

using namespace atlas;

static void testCaptainMoodInit() {
    std::cout << "\n=== CaptainMood: Init ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.isNeutral("e1"), "Default mood is Neutral");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 0.0f), "Default intensity 0");
    assertTrue(approxEqual(sys.getDecayRate("e1"), 0.05f), "Default decay rate 0.05");
    assertTrue(approxEqual(sys.getMoodThreshold("e1"), 0.1f), "Default threshold 0.1");
    assertTrue(sys.getMoodHistoryCount("e1") == 0, "Empty history");
    assertTrue(sys.getTotalEventsLogged("e1") == 0, "Zero events logged");
    assertTrue(sys.getCaptainId("e1").empty(), "Empty captain id");
    assertTrue(sys.getMoodLabel("e1") == "Neutral", "Label is Neutral");
    assertTrue(!sys.isPositiveMood("e1"), "Not positive initially");
    assertTrue(!sys.isNegativeMood("e1"), "Not negative initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testCaptainMoodApplyVictory() {
    std::cout << "\n=== CaptainMood: ApplyVictory ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.applyVictory("e1", 0.8f), "applyVictory succeeds");
    assertTrue(sys.getMood("e1") == components::CaptainMood::Confident,
               "Mood is Confident after victory");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 0.8f), "Intensity = 0.8");
    assertTrue(sys.isPositiveMood("e1"), "isPositiveMood = true");
    assertTrue(!sys.isNegativeMood("e1"), "isNegativeMood = false");
    assertTrue(!sys.isNeutral("e1"), "Not neutral");
    assertTrue(sys.getMoodLabel("e1") == "Confident", "Label = Confident");
    assertTrue(sys.getMoodHistoryCount("e1") == 1, "History has 1 entry");
    assertTrue(sys.getTotalEventsLogged("e1") == 1, "Total events = 1");

    // Invalid intensity
    assertTrue(!sys.applyVictory("e1", 0.0f), "Zero intensity rejected");
    assertTrue(!sys.applyVictory("e1", -0.1f), "Negative intensity rejected");
    assertTrue(!sys.applyVictory("e1", 1.1f), "Over-1 intensity rejected");

    assertTrue(!sys.applyVictory("missing", 0.5f), "Missing entity rejected");
}

static void testCaptainMoodApplySetback() {
    std::cout << "\n=== CaptainMood: ApplySetback ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.applySetback("e1", 0.6f), "applySetback succeeds");
    assertTrue(sys.getMood("e1") == components::CaptainMood::Frustrated,
               "Mood is Frustrated");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 0.6f), "Intensity = 0.6");
    assertTrue(!sys.isPositiveMood("e1"), "Not positive");
    assertTrue(sys.isNegativeMood("e1"), "isNegativeMood = true");
    assertTrue(sys.getMoodLabel("e1") == "Frustrated", "Label = Frustrated");

    assertTrue(!sys.applySetback("missing", 0.5f), "Missing entity rejected");
}

static void testCaptainMoodApplyNearDeath() {
    std::cout << "\n=== CaptainMood: ApplyNearDeath ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.applyNearDeath("e1"), "applyNearDeath succeeds");
    assertTrue(sys.getMood("e1") == components::CaptainMood::Anxious,
               "Mood is Anxious after near death");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 0.9f), "Near death intensity = 0.9");
    assertTrue(sys.isNegativeMood("e1"), "isNegativeMood = true");
    assertTrue(sys.getMoodLabel("e1") == "Anxious", "Label = Anxious");

    assertTrue(!sys.applyNearDeath("missing"), "Missing entity rejected");
}

static void testCaptainMoodApplyComradeship() {
    std::cout << "\n=== CaptainMood: ApplyComradeship ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.applyComradeship("e1", 0.7f), "applyComradeship succeeds");
    assertTrue(sys.getMood("e1") == components::CaptainMood::Confident,
               "Comradeship → Confident");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 0.7f), "Intensity = 0.7");
    assertTrue(sys.isPositiveMood("e1"), "Positive mood");

    assertTrue(!sys.applyComradeship("e1", -0.1f), "Negative intensity rejected");
    assertTrue(!sys.applyComradeship("missing", 0.5f), "Missing entity rejected");
}

static void testCaptainMoodApplyInsult() {
    std::cout << "\n=== CaptainMood: ApplyInsult ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.applyInsult("e1", 0.5f), "applyInsult succeeds");
    assertTrue(sys.getMood("e1") == components::CaptainMood::Frustrated,
               "Insult → Frustrated");
    assertTrue(sys.isNegativeMood("e1"), "isNegativeMood");

    assertTrue(!sys.applyInsult("e1", 0.0f), "Zero intensity rejected");
    assertTrue(!sys.applyInsult("missing", 0.5f), "Missing entity rejected");
}

static void testCaptainMoodApplyFocus() {
    std::cout << "\n=== CaptainMood: ApplyFocus ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.applyFocus("e1"), "applyFocus succeeds");
    assertTrue(sys.getMood("e1") == components::CaptainMood::Focused,
               "Focused mood set");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 0.8f), "Focus intensity = 0.8");
    assertTrue(sys.isPositiveMood("e1"), "Focus is positive");
    assertTrue(sys.getMoodLabel("e1") == "Focused", "Label = Focused");

    assertTrue(!sys.applyFocus("missing"), "Missing entity rejected");
}

static void testCaptainMoodApplyElation() {
    std::cout << "\n=== CaptainMood: ApplyElation ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.applyElation("e1", 1.0f), "applyElation succeeds");
    assertTrue(sys.getMood("e1") == components::CaptainMood::Elated,
               "Mood is Elated");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 1.0f), "Intensity = 1.0");
    assertTrue(sys.isPositiveMood("e1"), "Elated is positive");
    assertTrue(sys.getMoodLabel("e1") == "Elated", "Label = Elated");

    assertTrue(!sys.applyElation("e1", 0.0f), "Zero intensity rejected");
    assertTrue(!sys.applyElation("missing", 0.5f), "Missing entity rejected");
}

static void testCaptainMoodResetMood() {
    std::cout << "\n=== CaptainMood: ResetMood ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.applyElation("e1", 1.0f);

    assertTrue(!sys.isNeutral("e1"), "Not neutral before reset");
    assertTrue(sys.resetMood("e1"), "resetMood succeeds");
    assertTrue(sys.isNeutral("e1"), "Neutral after reset");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 0.0f), "Intensity = 0 after reset");

    // History preserved after reset
    assertTrue(sys.getMoodHistoryCount("e1") == 1, "History preserved after reset");

    assertTrue(!sys.resetMood("missing"), "Reset on missing entity fails");
}

static void testCaptainMoodDecay() {
    std::cout << "\n=== CaptainMood: Decay ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.applyVictory("e1", 0.8f);

    float before = sys.getMoodIntensity("e1");
    sys.update(2.0f);  // 2s × 0.05 = 0.1 reduction
    assertTrue(sys.getMoodIntensity("e1") < before, "Intensity decays over time");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 0.7f), "2s decay = 0.8 - 0.1 = 0.7");

    // Decay to threshold reverts to Neutral
    sys.update(100.0f);
    assertTrue(sys.isNeutral("e1"), "Reverts to Neutral after full decay");
    assertTrue(approxEqual(sys.getMoodIntensity("e1"), 0.0f), "Intensity = 0 at neutral");
}

static void testCaptainMoodHistory() {
    std::cout << "\n=== CaptainMood: History ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Fill to default max history (10)
    for (int i = 0; i < 10; ++i) {
        sys.applyVictory("e1", 0.5f);
    }
    assertTrue(sys.getMoodHistoryCount("e1") == 10, "History = 10");
    assertTrue(sys.getTotalEventsLogged("e1") == 10, "Total logged = 10");

    // One more should evict the oldest
    sys.applySetback("e1", 0.5f);
    assertTrue(sys.getMoodHistoryCount("e1") == 10, "History capped at 10");
    assertTrue(sys.getTotalEventsLogged("e1") == 11, "Total logged = 11 (uncapped)");
}

static void testCaptainMoodConfiguration() {
    std::cout << "\n=== CaptainMood: Configuration ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setDecayRate("e1", 0.1f), "setDecayRate succeeds");
    assertTrue(approxEqual(sys.getDecayRate("e1"), 0.1f), "DecayRate = 0.1");
    assertTrue(!sys.setDecayRate("e1", -0.1f), "Negative decay rejected");

    assertTrue(sys.setMoodThreshold("e1", 0.2f), "setMoodThreshold succeeds");
    assertTrue(approxEqual(sys.getMoodThreshold("e1"), 0.2f), "Threshold = 0.2");
    assertTrue(!sys.setMoodThreshold("e1", -0.1f), "Negative threshold rejected");
    assertTrue(!sys.setMoodThreshold("e1", 1.1f), "Over-1 threshold rejected");

    assertTrue(sys.setMaxHistory("e1", 5), "setMaxHistory succeeds");
    assertTrue(!sys.setMaxHistory("e1", 0), "Zero max history rejected");

    assertTrue(sys.setCaptainId("e1", "cap_bravo"), "setCaptainId succeeds");
    assertTrue(sys.getCaptainId("e1") == "cap_bravo", "CaptainId = cap_bravo");
    assertTrue(!sys.setCaptainId("e1", ""), "Empty captain id rejected");

    assertTrue(!sys.setDecayRate("missing", 0.1f), "setDecayRate on missing fails");
    assertTrue(!sys.setMoodThreshold("missing", 0.1f), "setThreshold on missing fails");
    assertTrue(!sys.setCaptainId("missing", "x"), "setCaptainId on missing fails");
}

static void testCaptainMoodMissingEntity() {
    std::cout << "\n=== CaptainMood: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CaptainMoodSystem sys(&world);

    assertTrue(sys.getMood("missing") == components::CaptainMood::Neutral,
               "getMood = Neutral on missing");
    assertTrue(approxEqual(sys.getMoodIntensity("missing"), 0.0f),
               "getMoodIntensity = 0 on missing");
    assertTrue(approxEqual(sys.getDecayRate("missing"), 0.0f),
               "getDecayRate = 0 on missing");
    assertTrue(approxEqual(sys.getMoodThreshold("missing"), 0.0f),
               "getMoodThreshold = 0 on missing");
    assertTrue(!sys.isPositiveMood("missing"), "isPositiveMood = false");
    assertTrue(!sys.isNegativeMood("missing"), "isNegativeMood = false");
    assertTrue(sys.isNeutral("missing"), "isNeutral = true on missing");
    assertTrue(sys.getMoodLabel("missing") == "Neutral", "getMoodLabel = Neutral");
    assertTrue(sys.getMoodHistoryCount("missing") == 0, "getMoodHistoryCount = 0");
    assertTrue(sys.getTotalEventsLogged("missing") == 0, "getTotalEventsLogged = 0");
    assertTrue(sys.getCaptainId("missing").empty(), "getCaptainId = ''");
    assertTrue(!sys.applyVictory("missing", 0.5f), "applyVictory = false");
    assertTrue(!sys.applySetback("missing", 0.5f), "applySetback = false");
    assertTrue(!sys.applyNearDeath("missing"), "applyNearDeath = false");
    assertTrue(!sys.applyComradeship("missing", 0.5f), "applyComradeship = false");
    assertTrue(!sys.applyInsult("missing", 0.5f), "applyInsult = false");
    assertTrue(!sys.applyFocus("missing"), "applyFocus = false");
    assertTrue(!sys.applyElation("missing", 0.5f), "applyElation = false");
    assertTrue(!sys.resetMood("missing"), "resetMood = false");
}

void run_captain_mood_system_tests() {
    testCaptainMoodInit();
    testCaptainMoodApplyVictory();
    testCaptainMoodApplySetback();
    testCaptainMoodApplyNearDeath();
    testCaptainMoodApplyComradeship();
    testCaptainMoodApplyInsult();
    testCaptainMoodApplyFocus();
    testCaptainMoodApplyElation();
    testCaptainMoodResetMood();
    testCaptainMoodDecay();
    testCaptainMoodHistory();
    testCaptainMoodConfiguration();
    testCaptainMoodMissingEntity();
}
