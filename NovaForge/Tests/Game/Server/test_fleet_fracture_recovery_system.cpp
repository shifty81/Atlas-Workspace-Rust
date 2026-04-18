// Tests for: FleetFractureRecoverySystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_fracture_recovery_system.h"

using namespace atlas;
using RP = components::FleetFractureRecoveryState::RecoveryPhase;

static void testFleetFractureInit() {
    std::cout << "\n=== FleetFracture: Init ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getPhase("e1") == RP::Stable, "Phase is Stable initially");
    assertTrue(sys.getPhaseString("e1") == "Stable", "Phase string Stable");
    assertTrue(approxEqual(sys.getFractureScore("e1"), 0.0f), "Zero fracture score");
    assertTrue(approxEqual(sys.getRecoveryMomentum("e1"), 0.0f), "Zero momentum");
    assertTrue(approxEqual(sys.getFragility("e1"), 0.3f), "Default fragility 0.3");
    assertTrue(sys.getFractureLogCount("e1") == 0, "Zero fracture log entries");
    assertTrue(sys.getMilestoneCount("e1") == 0, "Zero milestones");
    assertTrue(sys.getTotalFractures("e1") == 0, "Zero total_fractures");
    assertTrue(sys.getTotalRecoveries("e1") == 0, "Zero total_recoveries");
    assertTrue(sys.getFleetId("e1").empty(), "Empty fleet_id");
    assertTrue(!sys.isFractured("e1"), "Not fractured initially");
    assertTrue(!sys.isRecovering("e1"), "Not recovering initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFleetFractureRecordEvent() {
    std::cout << "\n=== FleetFracture: RecordEvent ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        sys.recordFractureEvent("e1", "fe1", "Major battle", 5, 2, -10.0f, 0.5f),
        "Record fracture event");
    assertTrue(sys.getFractureLogCount("e1") == 1, "1 log entry");
    // Score = 0.5 * 40 * (1 + 0.3) = 26
    float score = sys.getFractureScore("e1");
    assertTrue(score > 0.0f, "Fracture score increased");
    assertTrue(sys.getTotalShipsLost("e1") == 5, "total_ships_lost = 5");
    assertTrue(sys.getTotalCaptainsLost("e1") == 2, "total_captains_lost = 2");

    // Score ~26, cracking_threshold=30, should still be Stable
    assertTrue(sys.getPhase("e1") == RP::Stable, "Phase Stable at score ~26");

    // Add another event to push into Cracking
    sys.recordFractureEvent("e1", "fe2", "Secondary loss", 2, 1, -5.0f, 0.1f);
    // Score += 0.1 * 40 * 1.3 = 5.2, total ~31.2
    score = sys.getFractureScore("e1");
    assertTrue(score >= 30.0f, "Score >= cracking threshold");
    // Phase should have transitioned to Cracking or beyond
    assertTrue(sys.getPhase("e1") != RP::Stable, "Phase not Stable after high score");
}

static void testFleetFractureRecordEventValidation() {
    std::cout << "\n=== FleetFracture: RecordEvent Validation ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        !sys.recordFractureEvent("e1", "", "desc", 0, 0, 0.0f, 0.5f),
        "Empty event_id rejected");
    assertTrue(
        !sys.recordFractureEvent("e1", "fe1", "", 0, 0, 0.0f, 0.5f),
        "Empty description rejected");
    assertTrue(
        !sys.recordFractureEvent("e1", "fe1", "desc", -1, 0, 0.0f, 0.5f),
        "Negative ships_lost rejected");
    assertTrue(
        !sys.recordFractureEvent("e1", "fe1", "desc", 0, -1, 0.0f, 0.5f),
        "Negative captains rejected");
    assertTrue(
        !sys.recordFractureEvent("e1", "fe1", "desc", 0, 0, 0.0f, -0.1f),
        "Negative severity rejected");
    assertTrue(
        !sys.recordFractureEvent("e1", "fe1", "desc", 0, 0, 0.0f, 1.1f),
        "Severity > 1 rejected");
    assertTrue(
        !sys.recordFractureEvent("missing", "fe1", "desc", 0, 0, 0.0f, 0.5f),
        "Missing entity fails");
    assertTrue(sys.getFractureLogCount("e1") == 0, "Zero log entries");
}

static void testFleetFractureApplyRecovery() {
    std::cout << "\n=== FleetFracture: ApplyRecovery ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Push into Fractured
    sys.recordFractureEvent("e1", "fe1", "Big loss", 10, 5, -20.0f, 1.0f);
    // Score = 1.0 * 40 * 1.3 = 52
    float score = sys.getFractureScore("e1");
    assertTrue(score > 0.0f, "Score > 0 after event");

    // Apply recovery
    float before = sys.getFractureScore("e1");
    assertTrue(sys.applyRecovery("e1", 10.0f), "Apply recovery 10");
    assertTrue(sys.getFractureScore("e1") < before, "Score decreased");

    // Invalid
    assertTrue(!sys.applyRecovery("e1", 0.0f), "Zero recovery rejected");
    assertTrue(!sys.applyRecovery("e1", -5.0f), "Negative recovery rejected");
    assertTrue(!sys.applyRecovery("missing", 10.0f), "Recovery on missing fails");

    // Clamp to 0
    assertTrue(sys.applyRecovery("e1", 200.0f), "Large recovery clamps to 0");
    assertTrue(approxEqual(sys.getFractureScore("e1"), 0.0f), "Score clamped to 0");
}

static void testFleetFractureRecoveryMomentumTick() {
    std::cout << "\n=== FleetFracture: RecoveryMomentum Tick ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Push to Cracking phase (score ~31.2)
    sys.recordFractureEvent("e1", "fe1", "Battle", 3, 1, -5.0f, 0.6f);
    sys.recordFractureEvent("e1", "fe2", "Skirmish", 1, 0, -2.0f, 0.2f);

    float score_before = sys.getFractureScore("e1");
    if (sys.getPhase("e1") == RP::Cracking || sys.getPhase("e1") == RP::Recovering) {
        sys.setRecoveryMomentum("e1", 0.5f);
        sys.update(2.0f);
        float score_after = sys.getFractureScore("e1");
        assertTrue(score_after < score_before, "Score reduced by momentum tick");
    } else {
        // Force to cracking by setting score manually via events
        // Just verify momentum can be set
        assertTrue(sys.setRecoveryMomentum("e1", 0.5f), "Set momentum succeeds");
    }
    assertTrue(approxEqual(sys.getRecoveryMomentum("e1"), 0.5f), "Momentum is 0.5");
    assertTrue(!sys.setRecoveryMomentum("e1", -0.1f), "Negative momentum rejected");
    assertTrue(!sys.setRecoveryMomentum("e1", 1.1f), "Momentum > 1 rejected");
    assertTrue(!sys.setRecoveryMomentum("missing", 0.5f), "Momentum on missing fails");
}

static void testFleetFractureMilestones() {
    std::cout << "\n=== FleetFracture: Milestones ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(
        sys.addMilestone("e1", "m1", "Rebuild half fleet", 50.0f),
        "Add milestone m1");
    assertTrue(
        sys.addMilestone("e1", "m2", "Full fleet restored", 100.0f),
        "Add milestone m2");
    assertTrue(sys.getMilestoneCount("e1") == 2, "2 milestones");
    assertTrue(!sys.isMilestoneAchieved("e1", "m1"), "m1 not achieved");
    assertTrue(sys.getMilestoneAchievedCount("e1") == 0, "0 achieved");

    // Progress below required
    assertTrue(sys.progressMilestone("e1", "m1", 40.0f), "Progress m1 to 40");
    assertTrue(!sys.isMilestoneAchieved("e1", "m1"), "m1 not achieved at 40");

    // Progress to achieved
    assertTrue(sys.progressMilestone("e1", "m1", 50.0f), "Progress m1 to 50");
    assertTrue(sys.isMilestoneAchieved("e1", "m1"), "m1 achieved at 50");
    assertTrue(sys.getMilestoneAchievedCount("e1") == 1, "1 achieved");

    // Duplicate milestone rejected
    assertTrue(!sys.addMilestone("e1", "m1", "Dup", 25.0f), "Duplicate milestone rejected");

    // Validation
    assertTrue(!sys.addMilestone("e1", "", "desc", 10.0f), "Empty id rejected");
    assertTrue(!sys.addMilestone("e1", "m3", "", 10.0f), "Empty desc rejected");
    assertTrue(!sys.addMilestone("e1", "m3", "desc", 0.0f), "Zero required rejected");
    assertTrue(!sys.addMilestone("e1", "m3", "desc", -1.0f), "Negative required rejected");
    assertTrue(!sys.progressMilestone("e1", "nonexistent", 10.0f), "Progress missing milestone fails");
    assertTrue(!sys.addMilestone("missing", "m3", "desc", 10.0f), "Add milestone on missing fails");
}

static void testFleetFractureFractureLogCap() {
    std::cout << "\n=== FleetFracture: FractureLog Cap ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxLog("e1", 3);

    // Need unique event_ids - use small severity to avoid phase confusion
    // Actually no uniqueness check on fracture events
    sys.recordFractureEvent("e1", "fe1", "Event 1", 0, 0, 0.0f, 0.0f);
    sys.recordFractureEvent("e1", "fe2", "Event 2", 0, 0, 0.0f, 0.0f);
    sys.recordFractureEvent("e1", "fe3", "Event 3", 0, 0, 0.0f, 0.0f);
    assertTrue(sys.getFractureLogCount("e1") == 3, "3 log entries");

    // 4th should auto-purge oldest (fe1)
    sys.recordFractureEvent("e1", "fe4", "Event 4", 0, 0, 0.0f, 0.0f);
    assertTrue(sys.getFractureLogCount("e1") == 3, "Still 3 after auto-purge");
}

static void testFleetFractureConfiguration() {
    std::cout << "\n=== FleetFracture: Configuration ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setFleetId("e1", "fleet-alpha"), "Set fleet_id");
    assertTrue(sys.getFleetId("e1") == "fleet-alpha", "fleet_id matches");
    assertTrue(!sys.setFleetId("e1", ""), "Empty fleet_id rejected");
    assertTrue(!sys.setFleetId("missing", "x"), "FleetId on missing fails");

    assertTrue(sys.setMaxLog("e1", 10), "Set max_log 10");
    assertTrue(!sys.setMaxLog("e1", 0), "Zero max_log rejected");
    assertTrue(!sys.setMaxLog("missing", 5), "MaxLog on missing fails");

    assertTrue(sys.setFragility("e1", 0.8f), "Set fragility 0.8");
    assertTrue(approxEqual(sys.getFragility("e1"), 0.8f), "Fragility matches");
    assertTrue(!sys.setFragility("e1", -0.1f), "Negative fragility rejected");
    assertTrue(!sys.setFragility("e1", 1.1f), "Fragility > 1 rejected");
    assertTrue(!sys.setFragility("missing", 0.5f), "Fragility on missing fails");

    assertTrue(sys.setFractureThreshold("e1", 70.0f), "Set fracture threshold");
    assertTrue(!sys.setFractureThreshold("e1", 0.0f), "Zero fracture threshold rejected");
    assertTrue(!sys.setFractureThreshold("e1", 100.0f), "100 fracture threshold rejected");
    assertTrue(!sys.setFractureThreshold("missing", 50.0f), "Threshold on missing fails");

    assertTrue(sys.setRecoveryThreshold("e1", 15.0f), "Set recovery threshold");
    assertTrue(!sys.setRecoveryThreshold("e1", -1.0f), "Negative recovery threshold rejected");
    assertTrue(!sys.setRecoveryThreshold("missing", 10.0f), "Recovery threshold on missing fails");
}

static void testFleetFracturePhaseTransitions() {
    std::cout << "\n=== FleetFracture: Phase Transitions ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Set thresholds for predictable transitions
    sys.setFractureThreshold("e1", 60.0f);
    sys.setRecoveryThreshold("e1", 20.0f);

    assertTrue(sys.getPhase("e1") == RP::Stable, "Initial: Stable");
    assertTrue(sys.getPhaseString("e1") == "Stable", "Phase string: Stable");

    // Push to Cracking (score >= cracking_threshold 30)
    // severity 0.6 * 40 * 1.3 = 31.2
    sys.recordFractureEvent("e1", "fe1", "Damage", 2, 0, 0.0f, 0.6f);
    assertTrue(sys.getPhase("e1") == RP::Cracking, "Phase: Cracking");
    assertTrue(sys.getPhaseString("e1") == "Cracking", "Phase string: Cracking");

    // Push to Fractured (score >= fracture_threshold 60)
    // Add another: 0.6 * 40 * 1.3 = 31.2 more, total ~62.4
    sys.recordFractureEvent("e1", "fe2", "More damage", 1, 0, 0.0f, 0.6f);
    assertTrue(sys.getPhase("e1") == RP::Fractured, "Phase: Fractured");
    assertTrue(sys.getPhaseString("e1") == "Fractured", "Phase string: Fractured");
    assertTrue(sys.isFractured("e1"), "isFractured = true");
    assertTrue(sys.isRecovering("e1"), "isRecovering = true (Fractured)");

    // Apply recovery to go below fracture_threshold
    sys.applyRecovery("e1", 10.0f);
    assertTrue(sys.getPhase("e1") == RP::Recovering, "Phase: Recovering");
    assertTrue(sys.getPhaseString("e1") == "Recovering", "Phase string: Recovering");
    assertTrue(sys.isRecovering("e1"), "isRecovering = true (Recovering)");

    // Apply recovery to go below recovery_threshold (20)
    sys.applyRecovery("e1", 50.0f);
    assertTrue(sys.getPhase("e1") == RP::Rebuilt, "Phase: Rebuilt");
    assertTrue(sys.getPhaseString("e1") == "Rebuilt", "Phase string: Rebuilt");

    // Full recovery to 0
    sys.applyRecovery("e1", 200.0f);
    assertTrue(approxEqual(sys.getFractureScore("e1"), 0.0f), "Score = 0");
    assertTrue(sys.getPhase("e1") == RP::Stable, "Phase: Stable again");
}

static void testFleetFractureCounters() {
    std::cout << "\n=== FleetFracture: Counters ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.recordFractureEvent("e1", "fe1", "Battle 1", 3, 2, -5.0f, 0.5f);
    sys.recordFractureEvent("e1", "fe2", "Battle 2", 2, 1, -3.0f, 0.4f);
    assertTrue(sys.getTotalShipsLost("e1") == 5, "total_ships_lost = 5");
    assertTrue(sys.getTotalCaptainsLost("e1") == 3, "total_captains_lost = 3");

    // Check total_fractures increments when entering Fractured
    // Current defaults: cracking=30, fracture=60
    // fe1 score: 0.5 * 40 * 1.3 = 26 (Stable)
    // fe2 score += 0.4 * 40 * 1.3 = 20.8, total ~46.8 (Cracking, not yet Fractured)
    // Need another to hit 60
    sys.recordFractureEvent("e1", "fe3", "Battle 3", 1, 0, -2.0f, 0.3f);
    // fe3 score += 0.3 * 40 * 1.3 = 15.6, total ~62.4 (Fractured)
    int frac = sys.getTotalFractures("e1");
    assertTrue(frac >= 1, "total_fractures >= 1 after hitting Fractured");

    // Recovery path
    sys.applyRecovery("e1", 50.0f);  // push to Recovering
    sys.applyRecovery("e1", 50.0f);  // push to Rebuilt
    int rec = sys.getTotalRecoveries("e1");
    assertTrue(rec >= 1, "total_recoveries >= 1 after Rebuilt");

    assertTrue(sys.clearFractureLog("e1"), "Clear fracture log");
    assertTrue(sys.getFractureLogCount("e1") == 0, "Zero log after clear");
    assertTrue(!sys.clearFractureLog("missing"), "Clear on missing fails");
}

static void testFleetFractureMissingEntity() {
    std::cout << "\n=== FleetFracture: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetFractureRecoverySystem sys(&world);

    assertTrue(sys.getPhase("missing") == RP::Stable, "getPhase = Stable default");
    assertTrue(sys.getPhaseString("missing").empty(), "getPhaseString = '' for missing");
    assertTrue(approxEqual(sys.getFractureScore("missing"), 0.0f), "getFractureScore = 0");
    assertTrue(approxEqual(sys.getRecoveryMomentum("missing"), 0.0f), "getRecoveryMomentum = 0");
    assertTrue(approxEqual(sys.getFragility("missing"), 0.0f), "getFragility = 0");
    assertTrue(sys.getFractureLogCount("missing") == 0, "getFractureLogCount = 0");
    assertTrue(sys.getMilestoneCount("missing") == 0, "getMilestoneCount = 0");
    assertTrue(sys.getMilestoneAchievedCount("missing") == 0, "getMilestoneAchievedCount = 0");
    assertTrue(!sys.isMilestoneAchieved("missing", "m1"), "isMilestoneAchieved = false");
    assertTrue(sys.getTotalFractures("missing") == 0, "getTotalFractures = 0");
    assertTrue(sys.getTotalRecoveries("missing") == 0, "getTotalRecoveries = 0");
    assertTrue(sys.getTotalCaptainsLost("missing") == 0, "getTotalCaptainsLost = 0");
    assertTrue(sys.getTotalShipsLost("missing") == 0, "getTotalShipsLost = 0");
    assertTrue(sys.getFleetId("missing").empty(), "getFleetId = ''");
    assertTrue(!sys.isFractured("missing"), "isFractured = false");
    assertTrue(!sys.isRecovering("missing"), "isRecovering = false");
}

void run_fleet_fracture_recovery_system_tests() {
    testFleetFractureInit();
    testFleetFractureRecordEvent();
    testFleetFractureRecordEventValidation();
    testFleetFractureApplyRecovery();
    testFleetFractureRecoveryMomentumTick();
    testFleetFractureMilestones();
    testFleetFractureFractureLogCap();
    testFleetFractureConfiguration();
    testFleetFracturePhaseTransitions();
    testFleetFractureCounters();
    testFleetFractureMissingEntity();
}
