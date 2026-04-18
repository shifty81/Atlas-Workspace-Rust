// Tests for: CaptainMentorshipSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/captain_mentorship_system.h"

using namespace atlas;

static void testCaptainMentorshipInit() {
    std::cout << "\n=== CaptainMentorship: Init ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.hasActiveStudent("e1"), "No active student initially");
    assertTrue(sys.getActiveStudentId("e1").empty(), "Active student id empty");
    assertTrue(sys.getSessionCount("e1") == 0, "Zero sessions");
    assertTrue(sys.getTotalStudentsMentored("e1") == 0, "Zero mentored");
    assertTrue(sys.getTotalGraduations("e1") == 0, "Zero graduations");
    assertTrue(sys.getTotalSkillTransfers("e1") == 0, "Zero skill transfers");
    assertTrue(sys.getMentorCaptainId("e1").empty(), "Mentor id empty");
    assertTrue(approxEqual(sys.getBondGrowthRate("e1"), 0.05f), "Default growth rate 0.05");
    assertTrue(approxEqual(sys.getSkillTransferThreshold("e1"), 0.5f), "Default skill threshold 0.5");
    assertTrue(approxEqual(sys.getGraduationThreshold("e1"), 0.9f), "Default graduation threshold 0.9");
    assertTrue(sys.getMaxSessions("e1") == 10, "Default max sessions 10");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testCaptainMentorshipStartMentorship() {
    std::cout << "\n=== CaptainMentorship: StartMentorship ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.startMentorship("e1", "s1", "cap_junior"), "Start mentorship succeeds");
    assertTrue(sys.hasActiveStudent("e1"), "Has active student");
    assertTrue(sys.getActiveStudentId("e1") == "cap_junior", "Active student is cap_junior");
    assertTrue(sys.getSessionCount("e1") == 1, "One session");
    assertTrue(sys.getTotalStudentsMentored("e1") == 1, "Total mentored = 1");
    assertTrue(sys.hasSession("e1", "s1"), "Has session s1");
    assertTrue(sys.isSessionActive("e1", "s1"), "Session s1 is active");
    assertTrue(approxEqual(sys.getBondStrength("e1", "s1"), 0.0f), "Bond starts at 0");
    assertTrue(!sys.isGraduated("e1", "s1"), "Not graduated yet");

    // Cannot start second mentorship while one is active
    assertTrue(!sys.startMentorship("e1", "s2", "cap_junior2"),
               "Cannot start while already mentoring");

    // Duplicate session id rejected after ending first
    sys.endMentorship("e1", "s1");
    assertTrue(!sys.startMentorship("e1", "s1", "cap_x"), "Duplicate session id rejected");

    // Empty inputs rejected
    assertTrue(!sys.startMentorship("e1", "", "cap_x"), "Empty session id rejected");
    assertTrue(!sys.startMentorship("e1", "s2", ""), "Empty student id rejected");
    assertTrue(!sys.startMentorship("missing", "s2", "cap_x"), "Missing entity rejected");
}

static void testCaptainMentorshipEndMentorship() {
    std::cout << "\n=== CaptainMentorship: EndMentorship ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.startMentorship("e1", "s1", "cap_junior");

    assertTrue(sys.endMentorship("e1", "s1"), "End mentorship succeeds");
    assertTrue(!sys.hasActiveStudent("e1"), "No active student after end");
    assertTrue(sys.getActiveStudentId("e1").empty(), "Active student id cleared");
    assertTrue(!sys.isSessionActive("e1", "s1"), "Session no longer active");
    assertTrue(sys.getSessionCount("e1") == 1, "Session still in history");

    // Ending already ended session fails
    assertTrue(!sys.endMentorship("e1", "s1"), "Double-end fails");
    // Non-existent session fails
    assertTrue(!sys.endMentorship("e1", "nonexistent"), "Nonexistent session fails");
    assertTrue(!sys.endMentorship("missing", "s1"), "Missing entity fails");
}

static void testCaptainMentorshipSharedEngagement() {
    std::cout << "\n=== CaptainMentorship: SharedEngagement ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.startMentorship("e1", "s1", "cap_junior");

    assertTrue(sys.recordSharedEngagement("e1", "s1"), "Shared engagement recorded");
    assertTrue(sys.getEngagementsShared("e1", "s1") == 1, "Engagements = 1");
    assertTrue(approxEqual(sys.getBondStrength("e1", "s1"), 0.05f), "Bond = 0.05 after one engagement");

    // Record more to grow bond
    for (int i = 0; i < 9; ++i) sys.recordSharedEngagement("e1", "s1");
    assertTrue(sys.getEngagementsShared("e1", "s1") == 10, "Engagements = 10");
    assertTrue(approxEqual(sys.getBondStrength("e1", "s1"), 0.5f), "Bond = 0.5 after 10 engagements");

    // Bond capped at 1.0
    for (int i = 0; i < 20; ++i) sys.recordSharedEngagement("e1", "s1");
    assertTrue(approxEqual(sys.getBondStrength("e1", "s1"), 1.0f), "Bond capped at 1.0");

    // Inactive session fails
    sys.endMentorship("e1", "s1");
    assertTrue(!sys.recordSharedEngagement("e1", "s1"), "Inactive session fails");
    assertTrue(!sys.recordSharedEngagement("missing", "s1"), "Missing entity fails");
}

static void testCaptainMentorshipSkillTransfer() {
    std::cout << "\n=== CaptainMentorship: SkillTransfer ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.startMentorship("e1", "s1", "cap_junior");

    // Bond too low for skill transfer
    assertTrue(!sys.applySkillTransfer("e1", "s1"), "Skill transfer fails when bond too low");

    // Grow bond to threshold
    for (int i = 0; i < 10; ++i) sys.recordSharedEngagement("e1", "s1"); // bond = 0.5
    assertTrue(sys.applySkillTransfer("e1", "s1"), "Skill transfer at threshold succeeds");
    assertTrue(sys.getSkillTransfers("e1", "s1") == 1, "Skill transfers = 1");
    assertTrue(sys.getTotalSkillTransfers("e1") == 1, "Total skill transfers = 1");

    assertTrue(sys.applySkillTransfer("e1", "s1"), "Second skill transfer succeeds");
    assertTrue(sys.getSkillTransfers("e1", "s1") == 2, "Skill transfers = 2");
    assertTrue(sys.getTotalSkillTransfers("e1") == 2, "Total skill transfers = 2");

    assertTrue(!sys.applySkillTransfer("missing", "s1"), "Missing entity fails");
}

static void testCaptainMentorshipGraduate() {
    std::cout << "\n=== CaptainMentorship: Graduate ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.startMentorship("e1", "s1", "cap_junior");

    // Bond too low for graduation
    assertTrue(!sys.graduateStudent("e1", "s1"), "Graduation fails when bond too low");

    // Grow bond past graduation threshold (0.9 = 18 engagements)
    for (int i = 0; i < 18; ++i) sys.recordSharedEngagement("e1", "s1");
    assertTrue(sys.getBondStrength("e1", "s1") >= 0.9f, "Bond >= 0.9");
    assertTrue(sys.graduateStudent("e1", "s1"), "Graduation succeeds");
    assertTrue(sys.isGraduated("e1", "s1"), "Session is graduated");
    assertTrue(!sys.isSessionActive("e1", "s1"), "Session closed after graduation");
    assertTrue(!sys.hasActiveStudent("e1"), "No active student after graduation");
    assertTrue(sys.getTotalGraduations("e1") == 1, "Total graduations = 1");

    // Double graduation fails
    assertTrue(!sys.graduateStudent("e1", "s1"), "Double graduation fails");
    assertTrue(!sys.graduateStudent("missing", "s1"), "Missing entity fails");
}

static void testCaptainMentorshipMultipleSessions() {
    std::cout << "\n=== CaptainMentorship: MultipleSessions ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Complete first mentorship
    sys.startMentorship("e1", "s1", "junior_a");
    sys.endMentorship("e1", "s1");

    // Start second
    assertTrue(sys.startMentorship("e1", "s2", "junior_b"), "Second mentorship starts");
    assertTrue(sys.getTotalStudentsMentored("e1") == 2, "Two total students mentored");
    assertTrue(sys.getSessionCount("e1") == 2, "Two sessions in history");
    assertTrue(sys.getActiveStudentId("e1") == "junior_b", "Active student is junior_b");
    sys.endMentorship("e1", "s2");

    // Third
    sys.startMentorship("e1", "s3", "junior_c");
    assertTrue(sys.getSessionCount("e1") == 3, "Three sessions");
    assertTrue(sys.getTotalStudentsMentored("e1") == 3, "Three mentored");
}

static void testCaptainMentorshipConfiguration() {
    std::cout << "\n=== CaptainMentorship: Configuration ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setMentorCaptainId("e1", "cap_vet"), "Set mentor captain id");
    assertTrue(sys.getMentorCaptainId("e1") == "cap_vet", "Mentor id = cap_vet");
    assertTrue(!sys.setMentorCaptainId("e1", ""), "Empty mentor id rejected");

    assertTrue(sys.setBondGrowthRate("e1", 0.1f), "Set bond growth rate");
    assertTrue(approxEqual(sys.getBondGrowthRate("e1"), 0.1f), "Growth rate = 0.1");
    assertTrue(!sys.setBondGrowthRate("e1", -0.1f), "Negative growth rate rejected");

    assertTrue(sys.setSkillTransferThreshold("e1", 0.6f), "Set skill transfer threshold");
    assertTrue(approxEqual(sys.getSkillTransferThreshold("e1"), 0.6f), "Threshold = 0.6");
    assertTrue(!sys.setSkillTransferThreshold("e1", 1.5f), "Over-1 threshold rejected");
    assertTrue(!sys.setSkillTransferThreshold("e1", -0.1f), "Negative threshold rejected");

    assertTrue(sys.setGraduationThreshold("e1", 0.95f), "Set graduation threshold");
    assertTrue(approxEqual(sys.getGraduationThreshold("e1"), 0.95f), "Threshold = 0.95");

    assertTrue(sys.setMaxSessions("e1", 5), "Set max sessions");
    assertTrue(sys.getMaxSessions("e1") == 5, "Max sessions = 5");
    assertTrue(!sys.setMaxSessions("e1", 0), "Zero max sessions rejected");
}

static void testCaptainMentorshipUpdate() {
    std::cout << "\n=== CaptainMentorship: Update ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.startMentorship("e1", "s1", "cap_junior");

    sys.update(10.0f); // Tick with active session

    // Session duration should have advanced
    // (No direct getter but system shouldn't crash)
    assertTrue(sys.isSessionActive("e1", "s1"), "Session still active after tick");
    assertTrue(!sys.isGraduated("e1", "s1"), "Not graduated after plain tick");
}

static void testCaptainMentorshipMissingEntity() {
    std::cout << "\n=== CaptainMentorship: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);

    assertTrue(!sys.initialize("ghost"), "Init fails");
    assertTrue(!sys.hasActiveStudent("ghost"), "hasActiveStudent false");
    assertTrue(sys.getActiveStudentId("ghost").empty(), "Active student empty");
    assertTrue(sys.getSessionCount("ghost") == 0, "Session count 0");
    assertTrue(sys.getTotalStudentsMentored("ghost") == 0, "Total mentored 0");
    assertTrue(sys.getTotalGraduations("ghost") == 0, "Total graduations 0");
    assertTrue(sys.getTotalSkillTransfers("ghost") == 0, "Total skill transfers 0");
    assertTrue(sys.getMentorCaptainId("ghost").empty(), "Mentor id empty");
    assertTrue(approxEqual(sys.getBondGrowthRate("ghost"), 0.0f), "Growth rate 0");
    assertTrue(!sys.startMentorship("ghost", "s1", "cap_x"), "startMentorship fails");
    assertTrue(!sys.endMentorship("ghost", "s1"), "endMentorship fails");
    assertTrue(!sys.graduateStudent("ghost", "s1"), "graduate fails");
    assertTrue(!sys.recordSharedEngagement("ghost", "s1"), "recordEngagement fails");
    assertTrue(!sys.applySkillTransfer("ghost", "s1"), "applySkillTransfer fails");
    assertTrue(!sys.setBondGrowthRate("ghost", 0.1f), "setBondGrowthRate fails");
    assertTrue(approxEqual(sys.getBondStrength("ghost", "s1"), 0.0f), "Bond strength 0");
    assertTrue(sys.getEngagementsShared("ghost", "s1") == 0, "Engagements 0");
    assertTrue(sys.getSkillTransfers("ghost", "s1") == 0, "Skill transfers 0");
    assertTrue(!sys.isSessionActive("ghost", "s1"), "Session not active");
    assertTrue(!sys.isGraduated("ghost", "s1"), "Not graduated");
    assertTrue(!sys.hasSession("ghost", "s1"), "No session");
    assertTrue(sys.getMaxSessions("ghost") == 0, "Max sessions 0");
}

static void testCaptainMentorshipCapacity() {
    std::cout << "\n=== CaptainMentorship: Capacity ===" << std::endl;
    ecs::World world;
    systems::CaptainMentorshipSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxSessions("e1", 3);

    // Fill 3 inactive sessions
    sys.startMentorship("e1", "s1", "j1"); sys.endMentorship("e1", "s1");
    sys.startMentorship("e1", "s2", "j2"); sys.endMentorship("e1", "s2");
    sys.startMentorship("e1", "s3", "j3"); sys.endMentorship("e1", "s3");
    assertTrue(sys.getSessionCount("e1") == 3, "Three sessions stored");

    // Starting another should evict an old inactive session
    assertTrue(sys.startMentorship("e1", "s4", "j4"), "Start beyond cap (evicts old)");
    assertTrue(sys.getSessionCount("e1") == 3, "Still at cap after eviction");
    assertTrue(sys.hasSession("e1", "s4"), "New session present");
}

void run_captain_mentorship_system_tests() {
    testCaptainMentorshipInit();
    testCaptainMentorshipStartMentorship();
    testCaptainMentorshipEndMentorship();
    testCaptainMentorshipSharedEngagement();
    testCaptainMentorshipSkillTransfer();
    testCaptainMentorshipGraduate();
    testCaptainMentorshipMultipleSessions();
    testCaptainMentorshipConfiguration();
    testCaptainMentorshipUpdate();
    testCaptainMentorshipMissingEntity();
    testCaptainMentorshipCapacity();
}
