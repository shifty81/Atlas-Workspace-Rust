// Tests for: PostEventAnalysisSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/post_event_analysis_system.h"

using namespace atlas;

static void testPostEventAnalysisInit() {
    std::cout << "\n=== PostEventAnalysis: Init ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.isAnalysisInProgress("e1"), "No analysis in progress initially");
    assertTrue(approxEqual(sys.getAnalysisProgress("e1"), 0.0f), "Progress 0 initially");
    assertTrue(sys.getCurrentEventId("e1").empty(), "No current event initially");
    assertTrue(sys.getAnalysisCount("e1") == 0, "0 completed analyses initially");
    assertTrue(sys.getTotalAnalysesCompleted("e1") == 0, "0 total analyses completed");
    assertTrue(sys.getPendingBlameCount("e1") == 0, "0 pending blame initially");
    assertTrue(sys.getLessonCount("e1") == 0, "0 lessons initially");
    assertTrue(sys.getTotalLessonsLearned("e1") == 0, "0 total lessons learned");
    assertTrue(sys.getFleetId("e1").empty(), "Fleet id empty initially");
    assertTrue(approxEqual(sys.getAnalysisDuration("e1"), 5.0f), "Default duration 5s");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testPostEventAnalysisStartAnalysis() {
    std::cout << "\n=== PostEventAnalysis: StartAnalysis ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Start analysis
    assertTrue(sys.startAnalysis("e1", "evt_001", "Battle at Asakai", 10.0f),
               "Start analysis succeeds");
    assertTrue(sys.isAnalysisInProgress("e1"), "Analysis in progress");
    assertTrue(sys.getCurrentEventId("e1") == "evt_001", "Current event id set");
    assertTrue(approxEqual(sys.getAnalysisProgress("e1"), 0.0f), "Progress 0 at start");

    // Cannot start while one is in progress
    assertTrue(!sys.startAnalysis("e1", "evt_002", "Another event", 5.0f),
               "Start rejected while analysis in progress");

    // Empty event_id rejected
    sys.dismissAnalysis("e1");
    assertTrue(!sys.startAnalysis("e1", "", "desc", 5.0f), "Empty event_id rejected");

    // Non-positive duration rejected
    assertTrue(!sys.startAnalysis("e1", "evt_003", "desc", 0.0f), "Zero duration rejected");
    assertTrue(!sys.startAnalysis("e1", "evt_003", "desc", -1.0f), "Negative duration rejected");

    // Missing entity
    assertTrue(!sys.startAnalysis("missing", "e1", "d", 5.0f), "Missing entity fails");
}

static void testPostEventAnalysisAddBlame() {
    std::cout << "\n=== PostEventAnalysis: AddBlame ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Can't add blame without active analysis
    assertTrue(!sys.addBlame("e1", "captain_a", "Broke formation", 0.5f),
               "addBlame fails without active analysis");

    sys.startAnalysis("e1", "evt_001", "Battle", 30.0f);

    // Valid blame entries
    assertTrue(sys.addBlame("e1", "captain_a", "Broke formation", 0.5f), "Blame 1 added");
    assertTrue(sys.addBlame("e1", "captain_b", "Delayed response", 0.3f), "Blame 2 added");
    assertTrue(sys.getPendingBlameCount("e1") == 2, "2 pending blame entries");

    // Weight edge cases
    assertTrue(sys.addBlame("e1", "captain_c", "Friendly fire", 0.0f), "Weight 0 valid");
    assertTrue(sys.addBlame("e1", "captain_d", "Full blame", 1.0f), "Weight 1.0 valid");

    // Invalid weight
    assertTrue(!sys.addBlame("e1", "captain_e", "reason", -0.1f), "Negative weight rejected");
    assertTrue(!sys.addBlame("e1", "captain_e", "reason", 1.1f), "Weight > 1 rejected");

    // Empty captain_id rejected
    assertTrue(!sys.addBlame("e1", "", "reason", 0.5f), "Empty captain_id rejected");

    // Empty reason rejected
    assertTrue(!sys.addBlame("e1", "captain_f", "", 0.5f), "Empty reason rejected");

    // Missing entity
    assertTrue(!sys.addBlame("missing", "x", "y", 0.5f), "Missing entity fails");
}

static void testPostEventAnalysisFinalizeAnalysis() {
    std::cout << "\n=== PostEventAnalysis: FinalizeAnalysis ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Can't finalize without active analysis
    assertTrue(!sys.finalizeAnalysis("e1", "We needed more DPS", 3, 1),
               "Finalize fails without active analysis");

    sys.startAnalysis("e1", "evt_001", "Battle at gate", 30.0f);
    sys.addBlame("e1", "captain_a", "Slow response", 0.6f);
    sys.addBlame("e1", "captain_b", "Off position", 0.4f);

    // Finalize
    assertTrue(sys.finalizeAnalysis("e1", "Positioning failure", 4, 1),
               "Finalize succeeds");
    assertTrue(!sys.isAnalysisInProgress("e1"), "No analysis in progress after finalize");
    assertTrue(sys.getCurrentEventId("e1").empty(), "Event id cleared after finalize");
    assertTrue(sys.getPendingBlameCount("e1") == 0, "Pending blame cleared after finalize");
    assertTrue(sys.getAnalysisCount("e1") == 1, "1 completed analysis");
    assertTrue(sys.getTotalAnalysesCompleted("e1") == 1, "Total completed 1");

    // Verify stored analysis
    assertTrue(sys.hasAnalysis("e1", "evt_001"), "Analysis evt_001 stored");
    assertTrue(sys.isAnalysisFinalized("e1", "evt_001"), "evt_001 is finalized");
    assertTrue(sys.getAnalysisConclusion("e1", "evt_001") == "Positioning failure",
               "Conclusion stored");
    assertTrue(sys.getAnalysisAgreementCount("e1", "evt_001") == 4, "Agreement count 4");
    assertTrue(sys.getAnalysisDisssentCount("e1", "evt_001") == 1, "Dissent count 1");
    assertTrue(sys.getCompletedBlameCount("e1", "evt_001") == 2, "2 blame entries stored");

    // Missing entity
    assertTrue(!sys.finalizeAnalysis("missing", "x", 1, 0), "Missing entity fails");
}

static void testPostEventAnalysisDismissAnalysis() {
    std::cout << "\n=== PostEventAnalysis: DismissAnalysis ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Can't dismiss without active analysis
    assertTrue(!sys.dismissAnalysis("e1"), "Dismiss fails without active analysis");

    sys.startAnalysis("e1", "evt_001", "Battle", 30.0f);
    sys.addBlame("e1", "captain_a", "Reason", 0.5f);

    // Dismiss clears everything without storing
    assertTrue(sys.dismissAnalysis("e1"), "Dismiss succeeds");
    assertTrue(!sys.isAnalysisInProgress("e1"), "No analysis after dismiss");
    assertTrue(sys.getCurrentEventId("e1").empty(), "Event id cleared");
    assertTrue(sys.getPendingBlameCount("e1") == 0, "Pending blame cleared");
    assertTrue(sys.getAnalysisCount("e1") == 0, "0 completed analyses (dismiss doesn't store)");
    assertTrue(sys.getTotalAnalysesCompleted("e1") == 0, "Total completed still 0");

    // Missing entity
    assertTrue(!sys.dismissAnalysis("missing"), "Missing entity fails");
}

static void testPostEventAnalysisAutoFinalize() {
    std::cout << "\n=== PostEventAnalysis: AutoFinalize ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.startAnalysis("e1", "evt_001", "Ambush", 3.0f);
    sys.addBlame("e1", "captain_a", "Scouting failure", 0.8f);

    // Advance to just before expiry
    sys.update(2.5f);
    assertTrue(sys.isAnalysisInProgress("e1"), "Still in progress at 2.5s");
    float prog = sys.getAnalysisProgress("e1");
    assertTrue(prog > 0.8f && prog <= 1.0f, "Progress > 0.8 at 2.5/3s");

    // Advance past expiry
    sys.update(1.0f);
    assertTrue(!sys.isAnalysisInProgress("e1"), "Analysis auto-finalized after timeout");
    assertTrue(sys.getAnalysisCount("e1") == 1, "1 analysis stored after auto-finalize");
    assertTrue(sys.getTotalAnalysesCompleted("e1") == 1, "Total 1 after auto-finalize");
    assertTrue(sys.hasAnalysis("e1", "evt_001"), "evt_001 stored after auto-finalize");
    assertTrue(sys.getCompletedBlameCount("e1", "evt_001") == 1, "Blame carried over");
    assertTrue(sys.getAnalysisConclusion("e1", "evt_001").empty(),
               "Empty conclusion on auto-finalize");
}

static void testPostEventAnalysisLessons() {
    std::cout << "\n=== PostEventAnalysis: Lessons ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Add lessons
    assertTrue(sys.addLesson("e1", "Always scout before engaging"), "Lesson 1 added");
    assertTrue(sys.addLesson("e1", "Maintain formation in gate camps"), "Lesson 2 added");
    assertTrue(sys.getLessonCount("e1") == 2, "2 lessons");
    assertTrue(sys.getTotalLessonsLearned("e1") == 2, "Total 2");

    // Empty lesson rejected
    assertTrue(!sys.addLesson("e1", ""), "Empty lesson rejected");

    // Capacity: set max to 2, adding 3rd should purge oldest
    sys.setMaxAnalyses("e1", 20); // doesn't affect lessons
    // Default max_lessons = 50, set it lower via component access
    // Since we can't directly set max_lessons via system, test overflow on full impl
    // Just test clearLessons
    assertTrue(sys.clearLessons("e1"), "clearLessons succeeds");
    assertTrue(sys.getLessonCount("e1") == 0, "0 lessons after clear");
    assertTrue(sys.getTotalLessonsLearned("e1") == 2, "Total preserved after clear");

    // Missing entity
    assertTrue(!sys.addLesson("missing", "lesson"), "addLesson missing fails");
    assertTrue(!sys.clearLessons("missing"), "clearLessons missing fails");
    assertTrue(sys.getLessonCount("missing") == 0, "getLessonCount missing returns 0");
    assertTrue(sys.getTotalLessonsLearned("missing") == 0, "getTotalLessonsLearned missing returns 0");
}

static void testPostEventAnalysisManageAnalyses() {
    std::cout << "\n=== PostEventAnalysis: ManageAnalyses ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Store two analyses
    sys.startAnalysis("e1", "evt_001", "Battle 1", 1.0f);
    sys.finalizeAnalysis("e1", "Need better range", 2, 0);
    sys.startAnalysis("e1", "evt_002", "Battle 2", 1.0f);
    sys.finalizeAnalysis("e1", "Kite more", 3, 1);
    assertTrue(sys.getAnalysisCount("e1") == 2, "2 analyses stored");

    // Remove one
    assertTrue(sys.removeAnalysis("e1", "evt_001"), "Remove evt_001 succeeds");
    assertTrue(!sys.hasAnalysis("e1", "evt_001"), "evt_001 gone");
    assertTrue(sys.getAnalysisCount("e1") == 1, "1 analysis remaining");

    // Remove non-existent
    assertTrue(!sys.removeAnalysis("e1", "nothere"), "Remove missing analysis fails");

    // clearAnalyses
    assertTrue(sys.clearAnalyses("e1"), "clearAnalyses succeeds");
    assertTrue(sys.getAnalysisCount("e1") == 0, "0 analyses after clear");
    // total_analyses_completed not reset by clearAnalyses
    assertTrue(sys.getTotalAnalysesCompleted("e1") == 2, "Total completed preserved after clear");

    // Missing entity
    assertTrue(!sys.removeAnalysis("missing", "x"), "removeAnalysis missing fails");
    assertTrue(!sys.clearAnalyses("missing"), "clearAnalyses missing fails");
}

static void testPostEventAnalysisCapacity() {
    std::cout << "\n=== PostEventAnalysis: Capacity ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxAnalyses("e1", 2);

    sys.startAnalysis("e1", "e1", "First battle", 1.0f);
    sys.finalizeAnalysis("e1", "Lesson A", 1, 0);
    sys.startAnalysis("e1", "e2", "Second battle", 1.0f);
    sys.finalizeAnalysis("e1", "Lesson B", 2, 0);
    assertTrue(sys.getAnalysisCount("e1") == 2, "2 analyses at capacity");

    // Adding third should purge oldest
    sys.startAnalysis("e1", "e3", "Third battle", 1.0f);
    sys.finalizeAnalysis("e1", "Lesson C", 1, 1);
    assertTrue(sys.getAnalysisCount("e1") == 2, "Still 2 after purge");
    assertTrue(!sys.hasAnalysis("e1", "e1"), "Oldest (e1) purged");
    assertTrue(sys.hasAnalysis("e1", "e3"), "e3 stored");
    assertTrue(sys.getTotalAnalysesCompleted("e1") == 3, "Total 3 after purge");
}

static void testPostEventAnalysisConfiguration() {
    std::cout << "\n=== PostEventAnalysis: Configuration ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // setFleetId valid
    assertTrue(sys.setFleetId("e1", "fleet_alpha"), "setFleetId succeeds");
    assertTrue(sys.getFleetId("e1") == "fleet_alpha", "Fleet id set");
    assertTrue(!sys.setFleetId("e1", ""), "setFleetId empty fails");

    // setMaxAnalyses valid
    assertTrue(sys.setMaxAnalyses("e1", 5), "setMaxAnalyses 5 succeeds");
    assertTrue(!sys.setMaxAnalyses("e1", 0), "setMaxAnalyses 0 fails");
    assertTrue(!sys.setMaxAnalyses("e1", -1), "setMaxAnalyses -1 fails");

    // setAnalysisDuration valid
    assertTrue(sys.setAnalysisDuration("e1", 60.0f), "setAnalysisDuration 60 succeeds");
    assertTrue(approxEqual(sys.getAnalysisDuration("e1"), 60.0f), "Duration is 60");
    assertTrue(!sys.setAnalysisDuration("e1", 0.0f), "Zero duration fails");
    assertTrue(!sys.setAnalysisDuration("e1", -1.0f), "Negative duration fails");

    // Missing entity
    assertTrue(!sys.setFleetId("missing", "x"), "setFleetId missing fails");
    assertTrue(!sys.setMaxAnalyses("missing", 5), "setMaxAnalyses missing fails");
    assertTrue(!sys.setAnalysisDuration("missing", 10.0f), "setAnalysisDuration missing fails");
}

static void testPostEventAnalysisMissingEntity() {
    std::cout << "\n=== PostEventAnalysis: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);

    assertTrue(!sys.isAnalysisInProgress("missing"), "isAnalysisInProgress missing returns false");
    assertTrue(approxEqual(sys.getAnalysisProgress("missing"), 0.0f), "getAnalysisProgress missing returns 0");
    assertTrue(sys.getCurrentEventId("missing").empty(), "getCurrentEventId missing returns empty");
    assertTrue(sys.getAnalysisCount("missing") == 0, "getAnalysisCount missing returns 0");
    assertTrue(!sys.hasAnalysis("missing", "e1"), "hasAnalysis missing returns false");
    assertTrue(!sys.isAnalysisFinalized("missing", "e1"), "isAnalysisFinalized missing returns false");
    assertTrue(sys.getAnalysisConclusion("missing", "e1").empty(), "getAnalysisConclusion missing returns empty");
    assertTrue(sys.getAnalysisAgreementCount("missing", "e1") == 0, "getAgreementCount missing returns 0");
    assertTrue(sys.getAnalysisDisssentCount("missing", "e1") == 0, "getDisssentCount missing returns 0");
    assertTrue(sys.getCompletedBlameCount("missing", "e1") == 0, "getCompletedBlameCount missing returns 0");
    assertTrue(sys.getTotalAnalysesCompleted("missing") == 0, "getTotalAnalysesCompleted missing returns 0");
    assertTrue(sys.getPendingBlameCount("missing") == 0, "getPendingBlameCount missing returns 0");
    assertTrue(sys.getLessonCount("missing") == 0, "getLessonCount missing returns 0");
    assertTrue(sys.getTotalLessonsLearned("missing") == 0, "getTotalLessonsLearned missing returns 0");
    assertTrue(sys.getFleetId("missing").empty(), "getFleetId missing returns empty");
    assertTrue(approxEqual(sys.getAnalysisDuration("missing"), 0.0f), "getAnalysisDuration missing returns 0");
    assertTrue(!sys.startAnalysis("missing", "x", "y", 5.0f), "startAnalysis missing fails");
    assertTrue(!sys.addBlame("missing", "x", "y", 0.5f), "addBlame missing fails");
    assertTrue(!sys.finalizeAnalysis("missing", "x", 1, 0), "finalizeAnalysis missing fails");
    assertTrue(!sys.dismissAnalysis("missing"), "dismissAnalysis missing fails");
    assertTrue(!sys.addLesson("missing", "lesson"), "addLesson missing fails");
    assertTrue(!sys.removeAnalysis("missing", "x"), "removeAnalysis missing fails");
    assertTrue(!sys.clearAnalyses("missing"), "clearAnalyses missing fails");
}

static void testPostEventAnalysisProgressTracking() {
    std::cout << "\n=== PostEventAnalysis: ProgressTracking ===" << std::endl;
    ecs::World world;
    systems::PostEventAnalysisSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.startAnalysis("e1", "evt_001", "Battle", 10.0f);

    // Progress at 0
    assertTrue(approxEqual(sys.getAnalysisProgress("e1"), 0.0f), "Progress 0 at start");

    sys.update(5.0f);
    float prog = sys.getAnalysisProgress("e1");
    assertTrue(prog > 0.4f && prog < 0.6f, "Progress ~0.5 at 5/10s");

    // Progress capped at 1 when near timeout
    sys.update(4.9f);
    prog = sys.getAnalysisProgress("e1");
    assertTrue(prog <= 1.0f, "Progress capped at 1.0");

    // After auto-finalize, progress resets to 0
    sys.update(1.0f);
    assertTrue(!sys.isAnalysisInProgress("e1"), "Analysis finalized");
    assertTrue(approxEqual(sys.getAnalysisProgress("e1"), 0.0f),
               "Progress 0 after finalize");
}

void run_post_event_analysis_system_tests() {
    testPostEventAnalysisInit();
    testPostEventAnalysisStartAnalysis();
    testPostEventAnalysisAddBlame();
    testPostEventAnalysisFinalizeAnalysis();
    testPostEventAnalysisDismissAnalysis();
    testPostEventAnalysisAutoFinalize();
    testPostEventAnalysisLessons();
    testPostEventAnalysisManageAnalyses();
    testPostEventAnalysisCapacity();
    testPostEventAnalysisConfiguration();
    testPostEventAnalysisMissingEntity();
    testPostEventAnalysisProgressTracking();
}
