// Tests for: PlayerPresenceSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/player_presence_system.h"

using namespace atlas;

// ==================== PlayerPresenceSystem Tests ====================

static void testPlayerPresenceInit() {
    std::cout << "\n=== PlayerPresence: Init ===" << std::endl;
    ecs::World world;
    systems::PlayerPresenceSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(approxEqual(sys.getTimeSinceLastAction("e1"), 0.0f), "Zero time since action");
    assertTrue(!sys.isSilent("e1"), "Not silent initially");
    assertTrue(approxEqual(sys.getEngagementScore("e1"), 0.0f), "Zero engagement score");
    assertTrue(sys.getTotalCommandsIssued("e1") == 0, "Zero commands issued");
    assertTrue(sys.getSilenceStreak("e1") == 0, "Zero silence streak");
    assertTrue(sys.getPlayerId("e1").empty(), "Empty player id initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testPlayerPresenceRecordActivity() {
    std::cout << "\n=== PlayerPresence: RecordActivity ===" << std::endl;
    ecs::World world;
    systems::PlayerPresenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Advance time so silence timer has something to reset
    sys.update(50.0f);
    assertTrue(approxEqual(sys.getTimeSinceLastAction("e1"), 50.0f, 1.0f),
               "Timer advanced to 50s");

    assertTrue(
        sys.recordActivity("e1",
            components::PlayerPresenceState::ActivityType::Command),
        "Record command succeeds");
    assertTrue(approxEqual(sys.getTimeSinceLastAction("e1"), 0.0f),
               "Timer reset after activity");
    assertTrue(sys.getTotalCommandsIssued("e1") == 1, "1 command recorded");
    assertTrue(!sys.isSilent("e1"), "Not silent after activity");

    // Multiple activity types
    sys.recordActivity("e1", components::PlayerPresenceState::ActivityType::Combat);
    sys.recordActivity("e1", components::PlayerPresenceState::ActivityType::Trade);
    sys.recordActivity("e1", components::PlayerPresenceState::ActivityType::Navigation);
    assertTrue(sys.getTotalCommandsIssued("e1") == 4, "4 commands total");

    assertTrue(
        !sys.recordActivity("missing",
            components::PlayerPresenceState::ActivityType::Command),
        "Record on missing fails");
}

static void testPlayerPresenceSilenceDetection() {
    std::cout << "\n=== PlayerPresence: Silence Detection ===" << std::endl;
    ecs::World world;
    systems::PlayerPresenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setSilenceThreshold("e1", 10.0f);

    sys.update(5.0f);
    assertTrue(!sys.isSilent("e1"), "Not silent at 5s");

    sys.update(6.0f);
    assertTrue(sys.isSilent("e1"), "Silent after 11s total");
    assertTrue(sys.getSilenceStreak("e1") == 1, "Silence streak = 1");

    // Activity resets silence
    sys.recordActivity("e1", components::PlayerPresenceState::ActivityType::Chat);
    assertTrue(!sys.isSilent("e1"), "Not silent after activity");

    // Silence again
    sys.update(15.0f);
    assertTrue(sys.isSilent("e1"), "Silent again after 15s");
    assertTrue(sys.getSilenceStreak("e1") == 2, "Silence streak = 2");
}

static void testPlayerPresenceResetActivity() {
    std::cout << "\n=== PlayerPresence: ResetActivity ===" << std::endl;
    ecs::World world;
    systems::PlayerPresenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setSilenceThreshold("e1", 5.0f);

    sys.update(10.0f);
    assertTrue(sys.isSilent("e1"), "Silent before reset");
    sys.recordActivity("e1", components::PlayerPresenceState::ActivityType::Command);
    sys.update(10.0f);
    assertTrue(sys.getSilenceStreak("e1") >= 1, "Streak accumulated");

    assertTrue(sys.resetActivity("e1"), "Reset succeeds");
    assertTrue(!sys.isSilent("e1"), "Not silent after reset");
    assertTrue(sys.getSilenceStreak("e1") == 0, "Streak cleared");
    assertTrue(approxEqual(sys.getTimeSinceLastAction("e1"), 0.0f),
               "Timer reset");
    assertTrue(approxEqual(sys.getEngagementScore("e1"), 0.0f),
               "Engagement cleared");

    assertTrue(!sys.resetActivity("missing"), "Reset on missing fails");
}

static void testPlayerPresenceConfiguration() {
    std::cout << "\n=== PlayerPresence: Configuration ===" << std::endl;
    ecs::World world;
    systems::PlayerPresenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setSilenceThreshold("e1", 120.0f), "Set threshold 120s");
    assertTrue(sys.setSilenceThreshold("e1", 1.0f), "Set threshold 1s");
    assertTrue(!sys.setSilenceThreshold("e1", 0.0f), "Zero threshold rejected");
    assertTrue(!sys.setSilenceThreshold("e1", -5.0f), "Negative threshold rejected");

    assertTrue(sys.setEngagementWindow("e1", 30.0f), "Set window 30s");
    assertTrue(!sys.setEngagementWindow("e1", 0.0f), "Zero window rejected");
    assertTrue(!sys.setEngagementWindow("e1", -1.0f), "Negative window rejected");

    assertTrue(sys.setPlayerId("e1", "player_01"), "Set player id");
    assertTrue(sys.getPlayerId("e1") == "player_01", "Player id matches");
    assertTrue(!sys.setPlayerId("e1", ""), "Empty player id rejected");

    assertTrue(!sys.setSilenceThreshold("missing", 60.0f), "Threshold on missing fails");
    assertTrue(!sys.setEngagementWindow("missing", 30.0f), "Window on missing fails");
    assertTrue(!sys.setPlayerId("missing", "p"), "PlayerId on missing fails");
}

static void testPlayerPresenceEngagementScore() {
    std::cout << "\n=== PlayerPresence: EngagementScore ===" << std::endl;
    ecs::World world;
    systems::PlayerPresenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setEngagementWindow("e1", 10.0f);

    // Record 5 actions within window
    for (int i = 0; i < 5; ++i)
        sys.recordActivity("e1",
            components::PlayerPresenceState::ActivityType::Command);
    sys.update(0.1f);
    float score = sys.getEngagementScore("e1");
    assertTrue(score > 0.0f, "Engagement score > 0 after 5 commands");
    assertTrue(score <= 1.0f, "Engagement score <= 1");

    // Record many more to saturate
    for (int i = 0; i < 100; ++i)
        sys.recordActivity("e1",
            components::PlayerPresenceState::ActivityType::Combat);
    sys.update(0.1f);
    assertTrue(approxEqual(sys.getEngagementScore("e1"), 1.0f),
               "Engagement score capped at 1.0");
}

static void testPlayerPresenceMissingEntity() {
    std::cout << "\n=== PlayerPresence: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::PlayerPresenceSystem sys(&world);

    assertTrue(approxEqual(sys.getTimeSinceLastAction("missing"), 0.0f),
               "TimeSinceLastAction returns 0 for missing");
    assertTrue(!sys.isSilent("missing"),
               "isSilent returns false for missing");
    assertTrue(approxEqual(sys.getEngagementScore("missing"), 0.0f),
               "EngagementScore returns 0 for missing");
    assertTrue(sys.getTotalCommandsIssued("missing") == 0,
               "TotalCommands returns 0 for missing");
    assertTrue(sys.getSilenceStreak("missing") == 0,
               "SilenceStreak returns 0 for missing");
    assertTrue(sys.getPlayerId("missing").empty(),
               "PlayerId returns empty for missing");
    assertTrue(
        !sys.recordActivity("missing",
            components::PlayerPresenceState::ActivityType::Command),
        "RecordActivity fails for missing");
    assertTrue(!sys.resetActivity("missing"),
               "ResetActivity fails for missing");
}

static void testPlayerPresenceTimerAdvancement() {
    std::cout << "\n=== PlayerPresence: Timer Advancement ===" << std::endl;
    ecs::World world;
    systems::PlayerPresenceSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.update(1.0f);
    assertTrue(approxEqual(sys.getTimeSinceLastAction("e1"), 1.0f),
               "Timer at 1s");
    sys.update(2.5f);
    assertTrue(approxEqual(sys.getTimeSinceLastAction("e1"), 3.5f),
               "Timer at 3.5s");

    // Record activity and check reset
    sys.recordActivity("e1",
        components::PlayerPresenceState::ActivityType::Crafting);
    sys.update(0.5f);
    assertTrue(approxEqual(sys.getTimeSinceLastAction("e1"), 0.5f),
               "Timer at 0.5s after activity");
}

void run_player_presence_system_tests() {
    testPlayerPresenceInit();
    testPlayerPresenceRecordActivity();
    testPlayerPresenceSilenceDetection();
    testPlayerPresenceResetActivity();
    testPlayerPresenceConfiguration();
    testPlayerPresenceEngagementScore();
    testPlayerPresenceMissingEntity();
    testPlayerPresenceTimerAdvancement();
}
