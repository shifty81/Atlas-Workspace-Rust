// Tests for: EmotionalArcSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/emotional_arc_system.h"

using namespace atlas;

static void testEmotionalArcInit() {
    std::cout << "\n=== EmotionalArc: Init ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(approxEqual(sys.getConfidence("e1"), 0.5f), "Default confidence 0.5");
    assertTrue(approxEqual(sys.getTrustInPlayer("e1"), 0.5f), "Default trust 0.5");
    assertTrue(approxEqual(sys.getFatigue("e1"), 0.0f), "Default fatigue 0.0");
    assertTrue(approxEqual(sys.getHope("e1"), 0.5f), "Default hope 0.5");
    assertTrue(sys.getWins("e1") == 0, "Zero wins initially");
    assertTrue(sys.getLosses("e1") == 0, "Zero losses initially");
    assertTrue(sys.getNearDeaths("e1") == 0, "Zero near-deaths initially");
    assertTrue(sys.getSavesByPlayer("e1") == 0, "Zero saves initially");
    assertTrue(sys.getTotalArcUpdates("e1") == 0, "Zero arc updates initially");
    assertTrue(sys.getCaptainId("e1").empty(), "Empty captain_id initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testEmotionalArcApplyWin() {
    std::cout << "\n=== EmotionalArc: ApplyWin ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    float c0 = sys.getConfidence("e1");
    float h0 = sys.getHope("e1");
    float f0 = sys.getFatigue("e1");

    assertTrue(sys.applyWin("e1"), "applyWin succeeds");
    assertTrue(sys.getConfidence("e1") > c0, "Confidence rose after win");
    assertTrue(sys.getHope("e1") > h0, "Hope rose after win");
    assertTrue(sys.getFatigue("e1") > f0, "Fatigue slightly rose after win");
    assertTrue(sys.getWins("e1") == 1, "Wins = 1");
    assertTrue(sys.getTotalArcUpdates("e1") == 1, "Arc updates = 1");

    // Multiple wins clamp at 1.0
    for (int i = 0; i < 20; ++i) sys.applyWin("e1");
    assertTrue(sys.getConfidence("e1") <= 1.0f, "Confidence capped at 1.0");
    assertTrue(sys.getHope("e1") <= 1.0f, "Hope capped at 1.0");

    assertTrue(!sys.applyWin("missing"), "applyWin on missing entity fails");
}

static void testEmotionalArcApplyLoss() {
    std::cout << "\n=== EmotionalArc: ApplyLoss ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    float c0 = sys.getConfidence("e1");
    float h0 = sys.getHope("e1");
    float f0 = sys.getFatigue("e1");

    assertTrue(sys.applyLoss("e1"), "applyLoss succeeds");
    assertTrue(sys.getConfidence("e1") < c0, "Confidence fell after loss");
    assertTrue(sys.getHope("e1") < h0, "Hope fell after loss");
    assertTrue(sys.getFatigue("e1") > f0, "Fatigue rose after loss");
    assertTrue(sys.getLosses("e1") == 1, "Losses = 1");
    assertTrue(sys.getTotalArcUpdates("e1") == 1, "Arc updates = 1");

    // Multiple losses clamp at 0.0
    for (int i = 0; i < 20; ++i) sys.applyLoss("e1");
    assertTrue(sys.getConfidence("e1") >= 0.0f, "Confidence clamped at 0.0");
    assertTrue(sys.getHope("e1") >= 0.0f, "Hope clamped at 0.0");

    assertTrue(!sys.applyLoss("missing"), "applyLoss on missing entity fails");
}

static void testEmotionalArcApplyNearDeath() {
    std::cout << "\n=== EmotionalArc: ApplyNearDeath ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    float t0 = sys.getTrustInPlayer("e1");
    float f0 = sys.getFatigue("e1");
    float c0 = sys.getConfidence("e1");

    assertTrue(sys.applyNearDeath("e1"), "applyNearDeath succeeds");
    assertTrue(sys.getTrustInPlayer("e1") < t0, "Trust fell after near-death");
    assertTrue(sys.getFatigue("e1") > f0, "Fatigue rose after near-death");
    assertTrue(sys.getConfidence("e1") < c0, "Confidence fell after near-death");
    assertTrue(sys.getNearDeaths("e1") == 1, "NearDeaths = 1");
    assertTrue(sys.getTotalArcUpdates("e1") == 1, "Arc updates = 1");

    // Multiple near-deaths clamp at 0
    for (int i = 0; i < 20; ++i) sys.applyNearDeath("e1");
    assertTrue(sys.getTrustInPlayer("e1") >= 0.0f, "Trust clamped at 0.0");
    assertTrue(sys.getFatigue("e1") <= 1.0f, "Fatigue clamped at 1.0");

    assertTrue(!sys.applyNearDeath("missing"), "applyNearDeath on missing entity fails");
}

static void testEmotionalArcApplySave() {
    std::cout << "\n=== EmotionalArc: ApplySave ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    float t0 = sys.getTrustInPlayer("e1");
    float h0 = sys.getHope("e1");

    assertTrue(sys.applySave("e1"), "applySave succeeds");
    assertTrue(sys.getTrustInPlayer("e1") > t0, "Trust rose after save");
    assertTrue(sys.getHope("e1") > h0, "Hope rose after save");
    assertTrue(sys.getSavesByPlayer("e1") == 1, "Saves = 1");
    assertTrue(sys.getTotalArcUpdates("e1") == 1, "Arc updates = 1");

    // Multiple saves clamp at 1.0
    for (int i = 0; i < 20; ++i) sys.applySave("e1");
    assertTrue(sys.getTrustInPlayer("e1") <= 1.0f, "Trust clamped at 1.0");

    assertTrue(!sys.applySave("missing"), "applySave on missing entity fails");
}

static void testEmotionalArcApplyRest() {
    std::cout << "\n=== EmotionalArc: ApplyRest ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setFatigue("e1", 0.8f);

    float f0 = sys.getFatigue("e1");
    assertTrue(sys.applyRest("e1", 4.0f), "applyRest succeeds");
    assertTrue(sys.getFatigue("e1") < f0, "Fatigue reduced after rest");
    assertTrue(sys.getTotalArcUpdates("e1") == 1, "Arc updates = 1");

    // Zero-hours rest still succeeds
    assertTrue(sys.applyRest("e1", 0.0f), "Zero-hours rest succeeds");

    // Negative hours rejected
    assertTrue(!sys.applyRest("e1", -1.0f), "Negative hours rejected");

    // Fatigue clamped at 0
    sys.applyRest("e1", 1000.0f);
    assertTrue(sys.getFatigue("e1") >= 0.0f, "Fatigue clamped at 0.0");

    assertTrue(!sys.applyRest("missing", 1.0f), "applyRest on missing entity fails");
}

static void testEmotionalArcApplyExploration() {
    std::cout << "\n=== EmotionalArc: ApplyExploration ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    float h0 = sys.getHope("e1");
    assertTrue(sys.applyExploration("e1"), "applyExploration succeeds");
    assertTrue(sys.getHope("e1") > h0, "Hope rose after exploration");
    assertTrue(sys.getTotalArcUpdates("e1") == 1, "Arc updates = 1");

    assertTrue(!sys.applyExploration("missing"), "applyExploration on missing entity fails");
}

static void testEmotionalArcSetters() {
    std::cout << "\n=== EmotionalArc: Setters ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setConfidence("e1", 0.9f), "setConfidence succeeds");
    assertTrue(approxEqual(sys.getConfidence("e1"), 0.9f), "Confidence = 0.9");

    assertTrue(sys.setTrustInPlayer("e1", 0.3f), "setTrustInPlayer succeeds");
    assertTrue(approxEqual(sys.getTrustInPlayer("e1"), 0.3f), "Trust = 0.3");

    assertTrue(sys.setFatigue("e1", 0.7f), "setFatigue succeeds");
    assertTrue(approxEqual(sys.getFatigue("e1"), 0.7f), "Fatigue = 0.7");

    assertTrue(sys.setHope("e1", 0.2f), "setHope succeeds");
    assertTrue(approxEqual(sys.getHope("e1"), 0.2f), "Hope = 0.2");

    // Clamping
    assertTrue(sys.setConfidence("e1", 1.5f), "setConfidence clamps high");
    assertTrue(approxEqual(sys.getConfidence("e1"), 1.0f), "Confidence clamped to 1.0");
    assertTrue(sys.setFatigue("e1", -0.5f), "setFatigue clamps low");
    assertTrue(approxEqual(sys.getFatigue("e1"), 0.0f), "Fatigue clamped to 0.0");

    // CaptainId
    assertTrue(sys.setCaptainId("e1", "cap_alpha"), "setCaptainId succeeds");
    assertTrue(sys.getCaptainId("e1") == "cap_alpha", "CaptainId = cap_alpha");
    assertTrue(!sys.setCaptainId("e1", ""), "Empty captain_id rejected");

    // Missing entity
    assertTrue(!sys.setConfidence("missing", 0.5f), "setConfidence on missing fails");
    assertTrue(!sys.setTrustInPlayer("missing", 0.5f), "setTrust on missing fails");
    assertTrue(!sys.setFatigue("missing", 0.5f), "setFatigue on missing fails");
    assertTrue(!sys.setHope("missing", 0.5f), "setHope on missing fails");
    assertTrue(!sys.setCaptainId("missing", "x"), "setCaptainId on missing fails");
}

static void testEmotionalArcArcLabel() {
    std::cout << "\n=== EmotionalArc: ArcLabel ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // The Optimist: confidence > 0.7, hope > 0.7
    sys.setConfidence("e1", 0.8f);
    sys.setHope("e1", 0.8f);
    sys.setFatigue("e1", 0.3f);
    sys.setTrustInPlayer("e1", 0.5f);
    assertTrue(sys.getArcLabel("e1") == "The Optimist", "Arc = The Optimist");

    // The Weary: fatigue > 0.8 (checked first)
    sys.setFatigue("e1", 0.9f);
    assertTrue(sys.getArcLabel("e1") == "The Weary", "Arc = The Weary");

    // The Survivor: confidence < 0.3, hope > 0.6
    sys.setFatigue("e1", 0.3f);
    sys.setConfidence("e1", 0.2f);
    sys.setHope("e1", 0.7f);
    assertTrue(sys.getArcLabel("e1") == "The Survivor", "Arc = The Survivor");

    // The Skeptic: trust < 0.3, confidence > 0.5
    sys.setFatigue("e1", 0.3f);
    sys.setConfidence("e1", 0.6f);
    sys.setHope("e1", 0.4f);
    sys.setTrustInPlayer("e1", 0.2f);
    assertTrue(sys.getArcLabel("e1") == "The Skeptic", "Arc = The Skeptic");

    // The Faithful: trust > 0.7, fatigue > 0.6
    sys.setFatigue("e1", 0.7f);
    sys.setTrustInPlayer("e1", 0.8f);
    sys.setConfidence("e1", 0.5f);
    sys.setHope("e1", 0.5f);
    assertTrue(sys.getArcLabel("e1") == "The Faithful", "Arc = The Faithful");

    // Undefined
    sys.setFatigue("e1", 0.3f);
    sys.setConfidence("e1", 0.5f);
    sys.setHope("e1", 0.5f);
    sys.setTrustInPlayer("e1", 0.5f);
    assertTrue(sys.getArcLabel("e1") == "Undefined", "Arc = Undefined");

    assertTrue(sys.getArcLabel("missing").empty(), "Missing entity arc label = ''");
}

static void testEmotionalArcStateChecks() {
    std::cout << "\n=== EmotionalArc: State Checks ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // isWornDown
    sys.setFatigue("e1", 0.5f);
    assertTrue(!sys.isWornDown("e1"), "Not worn down at 0.5");
    sys.setFatigue("e1", 0.9f);
    assertTrue(sys.isWornDown("e1"), "Worn down at 0.9");

    // isLoyalToPlayer
    sys.setTrustInPlayer("e1", 0.5f);
    assertTrue(!sys.isLoyalToPlayer("e1"), "Not loyal at 0.5");
    sys.setTrustInPlayer("e1", 0.8f);
    assertTrue(sys.isLoyalToPlayer("e1"), "Loyal at 0.8");

    // isOptimistic
    sys.setConfidence("e1", 0.5f);
    sys.setHope("e1", 0.5f);
    assertTrue(!sys.isOptimistic("e1"), "Not optimistic at 0.5/0.5");
    sys.setConfidence("e1", 0.7f);
    sys.setHope("e1", 0.7f);
    assertTrue(sys.isOptimistic("e1"), "Optimistic at 0.7/0.7");

    // Missing entity
    assertTrue(!sys.isWornDown("missing"), "isWornDown missing = false");
    assertTrue(!sys.isLoyalToPlayer("missing"), "isLoyalToPlayer missing = false");
    assertTrue(!sys.isOptimistic("missing"), "isOptimistic missing = false");
}

static void testEmotionalArcMissingEntity() {
    std::cout << "\n=== EmotionalArc: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);

    assertTrue(approxEqual(sys.getConfidence("missing"), 0.5f), "getConfidence = 0.5");
    assertTrue(approxEqual(sys.getTrustInPlayer("missing"), 0.5f), "getTrustInPlayer = 0.5");
    assertTrue(approxEqual(sys.getFatigue("missing"), 0.0f), "getFatigue = 0.0");
    assertTrue(approxEqual(sys.getHope("missing"), 0.5f), "getHope = 0.5");
    assertTrue(sys.getArcLabel("missing").empty(), "getArcLabel = ''");
    assertTrue(!sys.isWornDown("missing"), "isWornDown = false");
    assertTrue(!sys.isLoyalToPlayer("missing"), "isLoyalToPlayer = false");
    assertTrue(!sys.isOptimistic("missing"), "isOptimistic = false");
    assertTrue(sys.getWins("missing") == 0, "getWins = 0");
    assertTrue(sys.getLosses("missing") == 0, "getLosses = 0");
    assertTrue(sys.getNearDeaths("missing") == 0, "getNearDeaths = 0");
    assertTrue(sys.getSavesByPlayer("missing") == 0, "getSavesByPlayer = 0");
    assertTrue(sys.getTotalArcUpdates("missing") == 0, "getTotalArcUpdates = 0");
    assertTrue(sys.getCaptainId("missing").empty(), "getCaptainId = ''");
    assertTrue(!sys.applyWin("missing"), "applyWin = false");
    assertTrue(!sys.applyLoss("missing"), "applyLoss = false");
    assertTrue(!sys.applyNearDeath("missing"), "applyNearDeath = false");
    assertTrue(!sys.applySave("missing"), "applySave = false");
    assertTrue(!sys.applyRest("missing", 1.0f), "applyRest = false");
    assertTrue(!sys.applyExploration("missing"), "applyExploration = false");
}

void run_emotional_arc_system_tests() {
    testEmotionalArcInit();
    testEmotionalArcApplyWin();
    testEmotionalArcApplyLoss();
    testEmotionalArcApplyNearDeath();
    testEmotionalArcApplySave();
    testEmotionalArcApplyRest();
    testEmotionalArcApplyExploration();
    testEmotionalArcSetters();
    testEmotionalArcArcLabel();
    testEmotionalArcStateChecks();
    testEmotionalArcMissingEntity();
}
