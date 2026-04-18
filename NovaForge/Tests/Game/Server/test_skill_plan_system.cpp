// Tests for: SkillPlanSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/skill_plan_system.h"

using namespace atlas;

// ==================== SkillPlanSystem Tests ====================

static void testSkillPlanInit() {
    std::cout << "\n=== SkillPlan: Init ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getPlanCount("e1") == 0, "Zero plans initially");
    assertTrue(sys.getActivePlanId("e1") == "", "No active plan");
    assertTrue(sys.getTotalPlansCreated("e1") == 0, "Zero total created");
    assertTrue(sys.getTotalPlansDeleted("e1") == 0, "Zero total deleted");
    assertTrue(sys.getTotalSkillsPlanned("e1") == 0, "Zero total skills");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testSkillPlanCreate() {
    std::cout << "\n=== SkillPlan: CreatePlan ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.createPlan("e1", "p1", "Plan Alpha"), "Create plan p1");
    assertTrue(sys.getPlanCount("e1") == 1, "1 plan after create");
    assertTrue(sys.hasPlan("e1", "p1"), "Has plan p1");
    assertTrue(sys.getPlanName("e1", "p1") == "Plan Alpha", "Plan name correct");
    assertTrue(sys.getTotalPlansCreated("e1") == 1, "1 total created");

    assertTrue(sys.createPlan("e1", "p2", "Plan Beta"), "Create plan p2");
    assertTrue(sys.getPlanCount("e1") == 2, "2 plans after second create");

    // Duplicate rejected
    assertTrue(!sys.createPlan("e1", "p1", "Duplicate"), "Duplicate plan rejected");
    assertTrue(sys.getPlanCount("e1") == 2, "Still 2 plans");

    // Missing entity
    assertTrue(!sys.createPlan("nonexistent", "px", "X"), "Create fails on missing entity");
}

static void testSkillPlanDelete() {
    std::cout << "\n=== SkillPlan: DeletePlan ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.createPlan("e1", "p1", "Alpha");
    sys.createPlan("e1", "p2", "Beta");

    assertTrue(sys.deletePlan("e1", "p1"), "Delete plan p1");
    assertTrue(sys.getPlanCount("e1") == 1, "1 plan remaining");
    assertTrue(!sys.hasPlan("e1", "p1"), "p1 no longer exists");
    assertTrue(sys.getTotalPlansDeleted("e1") == 1, "1 total deleted");

    // Delete non-existent plan
    assertTrue(!sys.deletePlan("e1", "p1"), "Delete non-existent fails");

    // Delete on missing entity
    assertTrue(!sys.deletePlan("nonexistent", "p2"), "Delete fails on missing entity");
}

static void testSkillPlanRename() {
    std::cout << "\n=== SkillPlan: RenamePlan ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.createPlan("e1", "p1", "Alpha");

    assertTrue(sys.renamePlan("e1", "p1", "Alpha Renamed"), "Rename succeeds");
    assertTrue(sys.getPlanName("e1", "p1") == "Alpha Renamed", "Name updated");

    // Rename non-existent plan
    assertTrue(!sys.renamePlan("e1", "pX", "X"), "Rename non-existent fails");
    assertTrue(!sys.renamePlan("nonexistent", "p1", "X"), "Rename fails on missing entity");
}

static void testSkillPlanClear() {
    std::cout << "\n=== SkillPlan: ClearPlans ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.createPlan("e1", "p1", "Alpha");
    sys.createPlan("e1", "p2", "Beta");

    assertTrue(sys.clearPlans("e1"), "Clear plans succeeds");
    assertTrue(sys.getPlanCount("e1") == 0, "Zero plans after clear");
    assertTrue(!sys.clearPlans("nonexistent"), "Clear fails on missing entity");
}

static void testSkillPlanActivate() {
    std::cout << "\n=== SkillPlan: ActivatePlan ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.createPlan("e1", "p1", "Alpha");
    sys.createPlan("e1", "p2", "Beta");

    assertTrue(sys.activatePlan("e1", "p1"), "Activate p1");
    assertTrue(sys.getActivePlanId("e1") == "p1", "Active plan is p1");
    assertTrue(sys.isActivePlan("e1", "p1"), "p1 is active");
    assertTrue(!sys.isActivePlan("e1", "p2"), "p2 is not active");

    assertTrue(sys.activatePlan("e1", "p2"), "Activate p2");
    assertTrue(sys.getActivePlanId("e1") == "p2", "Active plan is p2");

    // Activate non-existent plan
    assertTrue(!sys.activatePlan("e1", "pX"), "Activate non-existent fails");
    assertTrue(!sys.activatePlan("nonexistent", "p1"), "Activate fails on missing entity");
}

static void testSkillPlanDeactivate() {
    std::cout << "\n=== SkillPlan: DeactivatePlan ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.createPlan("e1", "p1", "Alpha");
    sys.activatePlan("e1", "p1");

    assertTrue(sys.deactivatePlan("e1"), "Deactivate succeeds");
    assertTrue(sys.getActivePlanId("e1") == "", "No active plan");
    assertTrue(!sys.isActivePlan("e1", "p1"), "p1 no longer active");
    assertTrue(!sys.deactivatePlan("nonexistent"), "Deactivate fails on missing entity");
}

static void testSkillPlanAddSkill() {
    std::cout << "\n=== SkillPlan: AddSkill ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.createPlan("e1", "p1", "Alpha");

    assertTrue(sys.addSkill("e1", "p1", "s1", "Gunnery", 3, 3600.0f), "Add skill s1");
    assertTrue(sys.getSkillCount("e1", "p1") == 1, "1 skill in plan");
    assertTrue(sys.hasSkillInPlan("e1", "p1", "s1"), "Has skill s1");
    assertTrue(approxEqual(sys.getTotalTrainingTime("e1", "p1"), 3600.0f), "Training time 3600");
    assertTrue(sys.getTotalSkillsPlanned("e1") == 1, "1 total skills planned");

    assertTrue(sys.addSkill("e1", "p1", "s2", "Navigation", 5, 7200.0f), "Add skill s2");
    assertTrue(sys.getSkillCount("e1", "p1") == 2, "2 skills in plan");
    assertTrue(approxEqual(sys.getTotalTrainingTime("e1", "p1"), 10800.0f), "Combined time 10800");

    // Duplicate skill rejected
    assertTrue(!sys.addSkill("e1", "p1", "s1", "Gunnery", 4, 1000.0f), "Duplicate skill rejected");

    // Invalid target level
    assertTrue(!sys.addSkill("e1", "p1", "s3", "X", 0, 100.0f), "Level 0 rejected");
    assertTrue(!sys.addSkill("e1", "p1", "s3", "X", 6, 100.0f), "Level 6 rejected");

    // Negative training time
    assertTrue(!sys.addSkill("e1", "p1", "s3", "X", 3, -100.0f), "Negative time rejected");

    // Non-existent plan
    assertTrue(!sys.addSkill("e1", "pX", "s3", "X", 3, 100.0f), "Add to non-existent plan fails");
    assertTrue(!sys.addSkill("nonexistent", "p1", "s3", "X", 3, 100.0f), "Add fails on missing entity");
}

static void testSkillPlanRemoveSkill() {
    std::cout << "\n=== SkillPlan: RemoveSkill ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.createPlan("e1", "p1", "Alpha");
    sys.addSkill("e1", "p1", "s1", "Gunnery", 3, 3600.0f);
    sys.addSkill("e1", "p1", "s2", "Navigation", 5, 7200.0f);

    assertTrue(sys.removeSkill("e1", "p1", "s1"), "Remove skill s1");
    assertTrue(sys.getSkillCount("e1", "p1") == 1, "1 skill remaining");
    assertTrue(!sys.hasSkillInPlan("e1", "p1", "s1"), "s1 removed");
    assertTrue(sys.hasSkillInPlan("e1", "p1", "s2"), "s2 still there");

    // Remove non-existent skill
    assertTrue(!sys.removeSkill("e1", "p1", "s1"), "Remove non-existent fails");
    assertTrue(!sys.removeSkill("nonexistent", "p1", "s1"), "Remove fails on missing entity");
}

static void testSkillPlanClearSkills() {
    std::cout << "\n=== SkillPlan: ClearSkills ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.createPlan("e1", "p1", "Alpha");
    sys.addSkill("e1", "p1", "s1", "Gunnery", 3, 3600.0f);
    sys.addSkill("e1", "p1", "s2", "Navigation", 5, 7200.0f);

    assertTrue(sys.clearSkills("e1", "p1"), "Clear skills succeeds");
    assertTrue(sys.getSkillCount("e1", "p1") == 0, "Zero skills after clear");
    assertTrue(approxEqual(sys.getTotalTrainingTime("e1", "p1"), 0.0f), "Zero training time");

    assertTrue(!sys.clearSkills("e1", "pX"), "Clear skills on non-existent plan fails");
    assertTrue(!sys.clearSkills("nonexistent", "p1"), "Clear skills fails on missing entity");
}

static void testSkillPlanMoveSkill() {
    std::cout << "\n=== SkillPlan: MoveSkill ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.createPlan("e1", "p1", "Alpha");
    sys.addSkill("e1", "p1", "s1", "Gunnery", 3, 3600.0f);
    sys.addSkill("e1", "p1", "s2", "Navigation", 5, 7200.0f);
    sys.addSkill("e1", "p1", "s3", "Engineering", 2, 1800.0f);

    assertTrue(sys.moveSkill("e1", "p1", "s3", 0), "Move s3 to index 0");
    assertTrue(sys.getSkillCount("e1", "p1") == 3, "Still 3 skills");

    // Move non-existent skill
    assertTrue(!sys.moveSkill("e1", "p1", "sX", 0), "Move non-existent fails");

    // Invalid index
    assertTrue(!sys.moveSkill("e1", "p1", "s1", -1), "Negative index rejected");
    // Out-of-bounds index is clamped to last position (valid behavior)
    assertTrue(sys.moveSkill("e1", "p1", "s1", 99), "Out of bounds clamped to end");

    assertTrue(!sys.moveSkill("nonexistent", "p1", "s1", 0), "Move fails on missing entity");
}

static void testSkillPlanMaxPlans() {
    std::cout << "\n=== SkillPlan: MaxPlans ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setMaxPlans("e1", 2), "Set max plans to 2");
    assertTrue(sys.createPlan("e1", "p1", "A"), "Create plan 1");
    assertTrue(sys.createPlan("e1", "p2", "B"), "Create plan 2");
    assertTrue(!sys.createPlan("e1", "p3", "C"), "Plan 3 rejected (max reached)");
    assertTrue(sys.getPlanCount("e1") == 2, "Still 2 plans");

    assertTrue(!sys.setMaxPlans("nonexistent", 5), "SetMaxPlans fails on missing entity");
}

static void testSkillPlanUpdate() {
    std::cout << "\n=== SkillPlan: Update ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.update(1.0f);
    sys.update(2.0f);
    // Just ensure no crash; elapsed is internal
    assertTrue(sys.getPlanCount("e1") == 0, "Still zero plans after update");
}

static void testSkillPlanMissing() {
    std::cout << "\n=== SkillPlan: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SkillPlanSystem sys(&world);

    assertTrue(!sys.initialize("missing"), "Init fails");
    assertTrue(!sys.createPlan("missing", "p1", "X"), "CreatePlan fails");
    assertTrue(!sys.deletePlan("missing", "p1"), "DeletePlan fails");
    assertTrue(!sys.renamePlan("missing", "p1", "X"), "RenamePlan fails");
    assertTrue(!sys.clearPlans("missing"), "ClearPlans fails");
    assertTrue(!sys.activatePlan("missing", "p1"), "ActivatePlan fails");
    assertTrue(!sys.deactivatePlan("missing"), "DeactivatePlan fails");
    assertTrue(!sys.addSkill("missing", "p1", "s1", "X", 3, 100.0f), "AddSkill fails");
    assertTrue(!sys.removeSkill("missing", "p1", "s1"), "RemoveSkill fails");
    assertTrue(!sys.clearSkills("missing", "p1"), "ClearSkills fails");
    assertTrue(!sys.moveSkill("missing", "p1", "s1", 0), "MoveSkill fails");
    assertTrue(!sys.setMaxPlans("missing", 5), "SetMaxPlans fails");
    assertTrue(sys.getPlanCount("missing") == 0, "getPlanCount returns 0");
    assertTrue(!sys.hasPlan("missing", "p1"), "hasPlan returns false");
    assertTrue(sys.getPlanName("missing", "p1") == "", "getPlanName returns empty");
    assertTrue(sys.getSkillCount("missing", "p1") == 0, "getSkillCount returns 0");
    assertTrue(approxEqual(sys.getTotalTrainingTime("missing", "p1"), 0.0f), "getTotalTrainingTime returns 0");
    assertTrue(sys.getActivePlanId("missing") == "", "getActivePlanId returns empty");
    assertTrue(!sys.isActivePlan("missing", "p1"), "isActivePlan returns false");
    assertTrue(sys.getTotalPlansCreated("missing") == 0, "getTotalPlansCreated returns 0");
    assertTrue(sys.getTotalPlansDeleted("missing") == 0, "getTotalPlansDeleted returns 0");
    assertTrue(sys.getTotalSkillsPlanned("missing") == 0, "getTotalSkillsPlanned returns 0");
    assertTrue(!sys.hasSkillInPlan("missing", "p1", "s1"), "hasSkillInPlan returns false");
}

void run_skill_plan_system_tests() {
    testSkillPlanInit();
    testSkillPlanCreate();
    testSkillPlanDelete();
    testSkillPlanRename();
    testSkillPlanClear();
    testSkillPlanActivate();
    testSkillPlanDeactivate();
    testSkillPlanAddSkill();
    testSkillPlanRemoveSkill();
    testSkillPlanClearSkills();
    testSkillPlanMoveSkill();
    testSkillPlanMaxPlans();
    testSkillPlanUpdate();
    testSkillPlanMissing();
}
