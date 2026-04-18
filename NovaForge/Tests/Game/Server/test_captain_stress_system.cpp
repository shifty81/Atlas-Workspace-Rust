// Tests for: CaptainStressSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/captain_stress_system.h"

using namespace atlas;

static void testCaptainStressInit() {
    std::cout << "\n=== CaptainStress: Init ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(approxEqual(sys.getStressLevel("e1"), 0.0f), "Default stress 0.0");
    assertTrue(approxEqual(sys.getRecoveryRate("e1"), 1.0f), "Default recovery rate 1.0");
    assertTrue(approxEqual(sys.getStressThreshold("e1"), 70.0f), "Default threshold 70");
    assertTrue(approxEqual(sys.getCriticalLevel("e1"), 90.0f), "Default critical 90");
    assertTrue(!sys.isHighStress("e1"), "Not high stress initially");
    assertTrue(!sys.isCriticalStress("e1"), "Not critical stress initially");
    assertTrue(approxEqual(sys.getStressPercent("e1"), 0.0f), "Stress percent 0");
    assertTrue(sys.getTotalStressorsApplied("e1") == 0, "Zero stressors applied");
    assertTrue(sys.getTotalReliefEvents("e1") == 0, "Zero relief events");
    assertTrue(sys.getCaptainId("e1").empty(), "Empty captain_id");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testCaptainStressRecordCombat() {
    std::cout << "\n=== CaptainStress: RecordCombat ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.recordCombat("e1", 1.0f), "recordCombat(1.0) succeeds");
    assertTrue(sys.getStressLevel("e1") > 0.0f, "Stress > 0 after combat");
    assertTrue(approxEqual(sys.getStressLevel("e1"), 15.0f), "Full intensity = 15 stress");
    assertTrue(sys.getTotalStressorsApplied("e1") == 1, "Stressors = 1");

    assertTrue(sys.recordCombat("e1", 0.5f), "recordCombat(0.5) succeeds");
    assertTrue(approxEqual(sys.getStressLevel("e1"), 22.5f), "0.5 intensity = +7.5 stress");

    // Stress capped at 100
    for (int i = 0; i < 10; ++i) sys.recordCombat("e1", 1.0f);
    assertTrue(sys.getStressLevel("e1") <= 100.0f, "Stress capped at 100");

    // Invalid intensity rejected
    assertTrue(!sys.recordCombat("e1", -0.1f), "Negative intensity rejected");
    assertTrue(!sys.recordCombat("missing", 1.0f), "Missing entity rejected");
}

static void testCaptainStressRecordNearDeath() {
    std::cout << "\n=== CaptainStress: RecordNearDeath ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.recordNearDeath("e1"), "recordNearDeath succeeds");
    assertTrue(approxEqual(sys.getStressLevel("e1"), 25.0f), "NearDeath = 25 stress");
    assertTrue(sys.getTotalStressorsApplied("e1") == 1, "Stressors = 1");

    // Two near-deaths → high stress
    assertTrue(sys.recordNearDeath("e1"), "Second near-death");
    assertTrue(approxEqual(sys.getStressLevel("e1"), 50.0f), "50 stress after 2 near-deaths");

    // Four near-deaths → critical stress (at default 90)
    sys.recordNearDeath("e1");
    sys.recordNearDeath("e1");
    assertTrue(sys.isCriticalStress("e1"), "Critical stress after 4 near-deaths");

    assertTrue(!sys.recordNearDeath("missing"), "Missing entity fails");
}

static void testCaptainStressRecordMissionFailure() {
    std::cout << "\n=== CaptainStress: RecordMissionFailure ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.recordMissionFailure("e1"), "recordMissionFailure succeeds");
    assertTrue(approxEqual(sys.getStressLevel("e1"), 10.0f), "Failure = 10 stress");
    assertTrue(sys.getTotalStressorsApplied("e1") == 1, "Stressors = 1");

    assertTrue(!sys.recordMissionFailure("missing"), "Missing entity fails");
}

static void testCaptainStressLongDeployment() {
    std::cout << "\n=== CaptainStress: RecordLongDeployment ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.recordLongDeployment("e1", 16.0f), "16 hours deployment succeeds");
    // 16h / 8h = 2 stress points
    assertTrue(approxEqual(sys.getStressLevel("e1"), 2.0f), "16h = 2 stress");
    assertTrue(sys.getTotalStressorsApplied("e1") == 1, "Stressors = 1");

    // Negative hours rejected
    assertTrue(!sys.recordLongDeployment("e1", -1.0f), "Negative hours rejected");

    assertTrue(!sys.recordLongDeployment("missing", 8.0f), "Missing entity fails");
}

static void testCaptainStressRelief() {
    std::cout << "\n=== CaptainStress: Relief Events ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.recordNearDeath("e1");  // stress = 25

    // applyRest
    assertTrue(sys.applyRest("e1", 2.0f), "applyRest(2h) succeeds");
    // 2h × 3 = 6 stress reduction: 25 - 6 = 19
    assertTrue(approxEqual(sys.getStressLevel("e1"), 19.0f), "2h rest = -6 stress");
    assertTrue(sys.getTotalReliefEvents("e1") == 1, "Relief events = 1");

    // applyRelief
    assertTrue(sys.applyRelief("e1", 10.0f), "applyRelief(10) succeeds");
    assertTrue(approxEqual(sys.getStressLevel("e1"), 9.0f), "Relief -10 stress");

    // applyShoreleave
    assertTrue(sys.applyShoreleave("e1"), "applyShoreleave succeeds");
    assertTrue(approxEqual(sys.getStressLevel("e1"), 0.0f), "Shoreleave = 0 stress");
    assertTrue(sys.getTotalReliefEvents("e1") == 3, "Relief events = 3");

    // Invalid inputs
    assertTrue(!sys.applyRest("e1", -1.0f), "Negative rest hours rejected");
    assertTrue(!sys.applyRelief("e1", -1.0f), "Negative relief amount rejected");

    // Missing entity
    assertTrue(!sys.applyRest("missing", 2.0f), "applyRest on missing fails");
    assertTrue(!sys.applyRelief("missing", 5.0f), "applyRelief on missing fails");
    assertTrue(!sys.applyShoreleave("missing"), "applyShoreleave on missing fails");
}

static void testCaptainStressPassiveRecovery() {
    std::cout << "\n=== CaptainStress: Passive Recovery ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.recordNearDeath("e1");  // stress = 25

    float before = sys.getStressLevel("e1");
    sys.update(5.0f);  // 5s × 1.0 pts/s = 5 reduction
    assertTrue(sys.getStressLevel("e1") < before, "Passive recovery reduces stress");
    assertTrue(approxEqual(sys.getStressLevel("e1"), 20.0f), "5s passive recovery = -5");

    // Stress does not go below 0
    sys.update(100.0f);
    assertTrue(approxEqual(sys.getStressLevel("e1"), 0.0f), "Stress floors at 0");
}

static void testCaptainStressHighAndCritical() {
    std::cout << "\n=== CaptainStress: High and Critical Flags ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Below threshold
    sys.recordNearDeath("e1");  // 25
    assertTrue(!sys.isHighStress("e1"), "Not high stress at 25");

    // Above threshold (default 70)
    sys.recordNearDeath("e1");  // 50
    sys.recordNearDeath("e1");  // 75
    assertTrue(sys.isHighStress("e1"), "High stress at 75");
    assertTrue(!sys.isCriticalStress("e1"), "Not critical at 75");
    assertTrue(sys.getStressPercent("e1") > 0.7f, "Stress percent > 70%");

    // Critical (default 90)
    sys.recordNearDeath("e1");  // 100 (capped)
    assertTrue(sys.isCriticalStress("e1"), "Critical stress at 100");
}

static void testCaptainStressConfiguration() {
    std::cout << "\n=== CaptainStress: Configuration ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setRecoveryRate("e1", 2.0f), "setRecoveryRate(2) succeeds");
    assertTrue(approxEqual(sys.getRecoveryRate("e1"), 2.0f), "RecoveryRate = 2.0");
    assertTrue(!sys.setRecoveryRate("e1", -1.0f), "Negative rate rejected");

    assertTrue(sys.setStressThreshold("e1", 60.0f), "setStressThreshold(60) succeeds");
    assertTrue(approxEqual(sys.getStressThreshold("e1"), 60.0f), "Threshold = 60");
    assertTrue(!sys.setStressThreshold("e1", -1.0f), "Negative threshold rejected");
    assertTrue(!sys.setStressThreshold("e1", 110.0f), "Over-100 threshold rejected");

    assertTrue(sys.setCriticalLevel("e1", 80.0f), "setCriticalLevel(80) succeeds");
    assertTrue(approxEqual(sys.getCriticalLevel("e1"), 80.0f), "Critical = 80");

    assertTrue(sys.setCaptainId("e1", "cap_alpha"), "setCaptainId succeeds");
    assertTrue(sys.getCaptainId("e1") == "cap_alpha", "CaptainId = cap_alpha");
    assertTrue(!sys.setCaptainId("e1", ""), "Empty captainId rejected");

    assertTrue(!sys.setRecoveryRate("missing", 1.0f), "setRecoveryRate on missing fails");
    assertTrue(!sys.setStressThreshold("missing", 70.0f), "setThreshold on missing fails");
    assertTrue(!sys.setCaptainId("missing", "x"), "setCaptainId on missing fails");
}

static void testCaptainStressMissingEntity() {
    std::cout << "\n=== CaptainStress: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CaptainStressSystem sys(&world);

    assertTrue(approxEqual(sys.getStressLevel("missing"), 0.0f), "getStressLevel = 0");
    assertTrue(approxEqual(sys.getRecoveryRate("missing"), 0.0f), "getRecoveryRate = 0");
    assertTrue(approxEqual(sys.getStressThreshold("missing"), 0.0f), "getThreshold = 0");
    assertTrue(approxEqual(sys.getCriticalLevel("missing"), 0.0f), "getCritical = 0");
    assertTrue(!sys.isHighStress("missing"), "isHighStress = false");
    assertTrue(!sys.isCriticalStress("missing"), "isCriticalStress = false");
    assertTrue(approxEqual(sys.getStressPercent("missing"), 0.0f), "getStressPercent = 0");
    assertTrue(sys.getTotalStressorsApplied("missing") == 0, "getTotalStressors = 0");
    assertTrue(sys.getTotalReliefEvents("missing") == 0, "getTotalRelief = 0");
    assertTrue(sys.getCaptainId("missing").empty(), "getCaptainId = ''");
    assertTrue(!sys.recordCombat("missing", 1.0f), "recordCombat = false");
    assertTrue(!sys.recordNearDeath("missing"), "recordNearDeath = false");
    assertTrue(!sys.recordMissionFailure("missing"), "recordMissionFailure = false");
    assertTrue(!sys.recordLongDeployment("missing", 8.0f), "recordLongDeployment = false");
    assertTrue(!sys.applyRest("missing", 1.0f), "applyRest = false");
    assertTrue(!sys.applyRelief("missing", 5.0f), "applyRelief = false");
    assertTrue(!sys.applyShoreleave("missing"), "applyShoreleave = false");
}

void run_captain_stress_system_tests() {
    testCaptainStressInit();
    testCaptainStressRecordCombat();
    testCaptainStressRecordNearDeath();
    testCaptainStressRecordMissionFailure();
    testCaptainStressLongDeployment();
    testCaptainStressRelief();
    testCaptainStressPassiveRecovery();
    testCaptainStressHighAndCritical();
    testCaptainStressConfiguration();
    testCaptainStressMissingEntity();
}
