// Tests for: FittingValidationSystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/fitting_validation_system.h"

using namespace atlas;

// ==================== FittingValidationSystem Tests ====================

static void testFittingInit() {
    std::cout << "\n=== FittingValidation: Init ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getModuleCount("e1") == 0, "Zero modules initially");
    assertTrue(approxEqual(sys.getCpuUsed("e1"), 0.0f), "Zero CPU used");
    assertTrue(approxEqual(sys.getPowergridUsed("e1"), 0.0f), "Zero PG used");
    assertTrue(sys.getTotalValidations("e1") == 0, "Zero validations");
    assertTrue(sys.getTotalFitsApplied("e1") == 0, "Zero fits");
    assertTrue(sys.getTotalModulesRejected("e1") == 0, "Zero rejected");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFittingSetShipStats() {
    std::cout << "\n=== FittingValidation: SetShipStats ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using ST = components::FittingValidationState::SlotType;
    assertTrue(sys.setShipStats("e1", 400.0f, 1200.0f, 6, 5, 4, 3), "Set ship stats");
    assertTrue(sys.getSlotsTotal("e1", ST::High) == 6, "6 high slots");
    assertTrue(sys.getSlotsTotal("e1", ST::Medium) == 5, "5 medium slots");
    assertTrue(sys.getSlotsTotal("e1", ST::Low) == 4, "4 low slots");
    assertTrue(sys.getSlotsTotal("e1", ST::Rig) == 3, "3 rig slots");
    assertTrue(approxEqual(sys.getCpuRemaining("e1"), 400.0f), "400 CPU remaining");
    assertTrue(approxEqual(sys.getPowergridRemaining("e1"), 1200.0f), "1200 PG remaining");

    // Invalid stats rejected
    assertTrue(!sys.setShipStats("e1", -1.0f, 100.0f, 1, 1, 1, 1), "Negative CPU rejected");
    assertTrue(!sys.setShipStats("e1", 100.0f, -1.0f, 1, 1, 1, 1), "Negative PG rejected");
    assertTrue(!sys.setShipStats("e1", 100.0f, 100.0f, -1, 1, 1, 1), "Negative slots rejected");
    assertTrue(!sys.setShipStats("missing", 100.0f, 100.0f, 1, 1, 1, 1), "Missing entity fails");
}

static void testFittingShipTypeAndCalibration() {
    std::cout << "\n=== FittingValidation: ShipType & Calibration ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setShipTypeId("e1", "raven_navy"), "Set ship type");
    assertTrue(sys.setCalibrationTotal("e1", 350), "Set calibration to 350");
    assertTrue(sys.getCalibrationRemaining("e1") == 350, "350 calibration remaining");

    assertTrue(!sys.setCalibrationTotal("e1", -1), "Negative calibration rejected");
    assertTrue(!sys.setShipTypeId("missing", "x"), "ShipType on missing fails");
    assertTrue(!sys.setCalibrationTotal("missing", 100), "Calibration on missing fails");
}

static void testFittingFitModule() {
    std::cout << "\n=== FittingValidation: FitModule ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setShipStats("e1", 400.0f, 1200.0f, 3, 3, 3, 2);

    using ST = components::FittingValidationState::SlotType;

    assertTrue(sys.fitModule("e1", "m1", "425mm Autocannon", ST::High, 0, 30.0f, 160.0f), "Fit high module");
    assertTrue(sys.getModuleCount("e1") == 1, "1 module");
    assertTrue(sys.hasModule("e1", "m1"), "Has m1");
    assertTrue(approxEqual(sys.getCpuUsed("e1"), 30.0f), "30 CPU used");
    assertTrue(approxEqual(sys.getPowergridUsed("e1"), 160.0f), "160 PG used");
    assertTrue(approxEqual(sys.getCpuRemaining("e1"), 370.0f), "370 CPU remaining");
    assertTrue(approxEqual(sys.getPowergridRemaining("e1"), 1040.0f), "1040 PG remaining");
    assertTrue(sys.getTotalFitsApplied("e1") == 1, "1 fit applied");

    // Duplicate module rejected
    assertTrue(!sys.fitModule("e1", "m1", "X", ST::High, 1, 10.0f, 10.0f), "Duplicate rejected");
    assertTrue(sys.getTotalModulesRejected("e1") == 1, "1 rejected");

    // Empty module ID rejected
    assertTrue(!sys.fitModule("e1", "", "X", ST::High, 1, 10.0f, 10.0f), "Empty ID rejected");

    // Negative values rejected
    assertTrue(!sys.fitModule("e1", "m2", "X", ST::High, 1, -10.0f, 10.0f), "Negative CPU rejected");
    assertTrue(!sys.fitModule("e1", "m2", "X", ST::High, -1, 10.0f, 10.0f), "Negative slot rejected");

    assertTrue(!sys.fitModule("missing", "m2", "X", ST::High, 0, 10.0f, 10.0f), "Fit on missing fails");
}

static void testFittingSlotCapacity() {
    std::cout << "\n=== FittingValidation: SlotCapacity ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setShipStats("e1", 9999.0f, 9999.0f, 2, 1, 1, 1);

    using ST = components::FittingValidationState::SlotType;

    assertTrue(sys.fitModule("e1", "h1", "Gun1", ST::High, 0, 10.0f, 10.0f), "Fit high 0");
    assertTrue(sys.fitModule("e1", "h2", "Gun2", ST::High, 1, 10.0f, 10.0f), "Fit high 1");
    assertTrue(!sys.fitModule("e1", "h3", "Gun3", ST::High, 2, 10.0f, 10.0f), "High slot full");
    assertTrue(sys.getSlotsUsed("e1", ST::High) == 2, "2 high used");
    assertTrue(sys.getTotalModulesRejected("e1") == 1, "1 rejected from capacity");

    // Slot index collision
    assertTrue(sys.fitModule("e1", "m1", "Shield", ST::Medium, 0, 10.0f, 10.0f), "Fit med 0");
    assertTrue(!sys.fitModule("e1", "m2", "Shield2", ST::Medium, 0, 10.0f, 10.0f), "Slot 0 collision");

    // isSlotAvailable
    assertTrue(!sys.isSlotAvailable("e1", ST::High, 0), "High 0 not available");
    assertTrue(!sys.isSlotAvailable("e1", ST::High, 1), "High 1 not available");
    assertTrue(sys.isSlotAvailable("e1", ST::Low, 0), "Low 0 available");
}

static void testFittingUnfitModule() {
    std::cout << "\n=== FittingValidation: UnfitModule ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setShipStats("e1", 400.0f, 1200.0f, 3, 3, 3, 2);

    using ST = components::FittingValidationState::SlotType;
    sys.fitModule("e1", "m1", "Gun", ST::High, 0, 30.0f, 160.0f);
    sys.fitModule("e1", "m2", "Shield", ST::Medium, 0, 50.0f, 200.0f);

    assertTrue(sys.unfitModule("e1", "m1"), "Unfit m1");
    assertTrue(sys.getModuleCount("e1") == 1, "1 module remaining");
    assertTrue(!sys.hasModule("e1", "m1"), "m1 gone");
    assertTrue(approxEqual(sys.getCpuUsed("e1"), 50.0f), "50 CPU after unfit");

    assertTrue(!sys.unfitModule("e1", "m1"), "Unfit non-existent fails");
    assertTrue(!sys.unfitModule("missing", "m2"), "Unfit on missing fails");
}

static void testFittingClear() {
    std::cout << "\n=== FittingValidation: ClearFitting ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setShipStats("e1", 400.0f, 1200.0f, 3, 3, 3, 2);

    using ST = components::FittingValidationState::SlotType;
    sys.fitModule("e1", "m1", "Gun", ST::High, 0, 30.0f, 160.0f);
    sys.fitModule("e1", "m2", "Shield", ST::Medium, 0, 50.0f, 200.0f);

    assertTrue(sys.clearFitting("e1"), "Clear fitting");
    assertTrue(sys.getModuleCount("e1") == 0, "Zero modules");
    assertTrue(approxEqual(sys.getCpuUsed("e1"), 0.0f), "0 CPU");
    assertTrue(approxEqual(sys.getPowergridUsed("e1"), 0.0f), "0 PG");
    assertTrue(!sys.clearFitting("missing"), "Clear on missing fails");
}

static void testFittingMetaLevel() {
    std::cout << "\n=== FittingValidation: MetaLevel ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setShipStats("e1", 400.0f, 1200.0f, 3, 3, 3, 2);

    using ST = components::FittingValidationState::SlotType;
    sys.fitModule("e1", "m1", "Gun", ST::High, 0, 30.0f, 160.0f);

    assertTrue(sys.setModuleMetaLevel("e1", "m1", 5), "Set meta level 5");
    assertTrue(!sys.setModuleMetaLevel("e1", "m1", -1), "Negative meta rejected");
    assertTrue(!sys.setModuleMetaLevel("e1", "mX", 3), "Non-existent module fails");
    assertTrue(!sys.setModuleMetaLevel("missing", "m1", 3), "Missing entity fails");
}

static void testFittingSkillRequirement() {
    std::cout << "\n=== FittingValidation: SkillRequirement ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setShipStats("e1", 400.0f, 1200.0f, 3, 3, 3, 2);

    using ST = components::FittingValidationState::SlotType;
    sys.fitModule("e1", "m1", "Gun", ST::High, 0, 30.0f, 160.0f);

    assertTrue(sys.setModuleSkillRequirement("e1", "m1", "Gunnery", 3), "Set skill req");
    assertTrue(!sys.setModuleSkillRequirement("e1", "m1", "X", 0), "Level 0 rejected");
    assertTrue(!sys.setModuleSkillRequirement("e1", "m1", "X", 6), "Level 6 rejected");
    assertTrue(!sys.setModuleSkillRequirement("e1", "mX", "X", 3), "Non-existent module fails");
    assertTrue(!sys.setModuleSkillRequirement("missing", "m1", "X", 3), "Missing entity fails");
}

static void testFittingCalibrationUsage() {
    std::cout << "\n=== FittingValidation: CalibrationUsage ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setShipStats("e1", 400.0f, 1200.0f, 3, 3, 3, 2);
    sys.setCalibrationTotal("e1", 400);

    using ST = components::FittingValidationState::SlotType;
    sys.fitModule("e1", "r1", "Rig1", ST::Rig, 0, 0.0f, 0.0f);

    assertTrue(sys.addCalibrationUsage("e1", "r1", 200), "Add 200 calibration");
    assertTrue(sys.getCalibrationRemaining("e1") == 200, "200 remaining");

    assertTrue(sys.addCalibrationUsage("e1", "r1", 150), "Add 150 more");
    assertTrue(sys.getCalibrationRemaining("e1") == 50, "50 remaining");

    // Exceeds total
    assertTrue(!sys.addCalibrationUsage("e1", "r1", 100), "Exceeds total rejected");

    assertTrue(!sys.addCalibrationUsage("e1", "r1", -1), "Negative cost rejected");
    assertTrue(!sys.addCalibrationUsage("e1", "rX", 10), "Non-existent module fails");
    assertTrue(!sys.addCalibrationUsage("missing", "r1", 10), "Missing entity fails");
}

static void testFittingValidate() {
    std::cout << "\n=== FittingValidation: Validate ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setShipStats("e1", 100.0f, 500.0f, 2, 2, 2, 2);
    sys.setCalibrationTotal("e1", 400);

    using ST = components::FittingValidationState::SlotType;
    sys.fitModule("e1", "m1", "Gun", ST::High, 0, 40.0f, 200.0f);
    sys.fitModule("e1", "m2", "Shield", ST::Medium, 0, 30.0f, 100.0f);

    assertTrue(sys.validateFitting("e1"), "Valid fitting passes");
    assertTrue(sys.getTotalValidations("e1") == 1, "1 validation");

    // Overload CPU: add module that exceeds
    sys.fitModule("e1", "m3", "Heavy Gun", ST::High, 1, 50.0f, 150.0f);
    // Total CPU = 40 + 30 + 50 = 120 > 100
    assertTrue(!sys.validateFitting("e1"), "CPU overloaded fails");
    assertTrue(sys.getTotalValidations("e1") == 2, "2 validations");

    assertTrue(!sys.validateFitting("missing"), "Validate on missing fails");
}

static void testFittingUpdate() {
    std::cout << "\n=== FittingValidation: Update ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.update(1.0f);
    sys.update(2.0f);
    assertTrue(sys.getModuleCount("e1") == 0, "Still zero after update");
}

static void testFittingMissing() {
    std::cout << "\n=== FittingValidation: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FittingValidationSystem sys(&world);

    using ST = components::FittingValidationState::SlotType;

    assertTrue(!sys.initialize("m"), "Init fails");
    assertTrue(!sys.setShipStats("m", 100.0f, 100.0f, 1, 1, 1, 1), "SetShipStats fails");
    assertTrue(!sys.setShipTypeId("m", "x"), "SetShipTypeId fails");
    assertTrue(!sys.setCalibrationTotal("m", 400), "SetCalibrationTotal fails");
    assertTrue(!sys.fitModule("m", "m1", "X", ST::High, 0, 10.0f, 10.0f), "FitModule fails");
    assertTrue(!sys.unfitModule("m", "m1"), "UnfitModule fails");
    assertTrue(!sys.clearFitting("m"), "ClearFitting fails");
    assertTrue(!sys.setModuleMetaLevel("m", "m1", 5), "SetMetaLevel fails");
    assertTrue(!sys.setModuleSkillRequirement("m", "m1", "X", 3), "SetSkillReq fails");
    assertTrue(!sys.addCalibrationUsage("m", "m1", 100), "AddCalibration fails");
    assertTrue(!sys.validateFitting("m"), "Validate fails");
    assertTrue(approxEqual(sys.getCpuUsed("m"), 0.0f), "getCpuUsed returns 0");
    assertTrue(approxEqual(sys.getCpuRemaining("m"), 0.0f), "getCpuRemaining returns 0");
    assertTrue(approxEqual(sys.getPowergridUsed("m"), 0.0f), "getPgUsed returns 0");
    assertTrue(approxEqual(sys.getPowergridRemaining("m"), 0.0f), "getPgRemaining returns 0");
    assertTrue(sys.getModuleCount("m") == 0, "getModuleCount returns 0");
    assertTrue(!sys.hasModule("m", "m1"), "hasModule returns false");
    assertTrue(sys.getSlotsUsed("m", ST::High) == 0, "getSlotsUsed returns 0");
    assertTrue(sys.getSlotsTotal("m", ST::High) == 0, "getSlotsTotal returns 0");
    assertTrue(sys.getCalibrationRemaining("m") == 0, "getCalibRemaining returns 0");
    assertTrue(!sys.isSlotAvailable("m", ST::High, 0), "isSlotAvailable returns false");
    assertTrue(sys.getTotalValidations("m") == 0, "getTotalValidations returns 0");
    assertTrue(sys.getTotalFitsApplied("m") == 0, "getTotalFitsApplied returns 0");
    assertTrue(sys.getTotalModulesRejected("m") == 0, "getTotalModulesRejected returns 0");
}

void run_fitting_validation_system_tests() {
    testFittingInit();
    testFittingSetShipStats();
    testFittingShipTypeAndCalibration();
    testFittingFitModule();
    testFittingSlotCapacity();
    testFittingUnfitModule();
    testFittingClear();
    testFittingMetaLevel();
    testFittingSkillRequirement();
    testFittingCalibrationUsage();
    testFittingValidate();
    testFittingUpdate();
    testFittingMissing();
}
