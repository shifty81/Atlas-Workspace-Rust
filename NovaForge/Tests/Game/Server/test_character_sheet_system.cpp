// Tests for: CharacterSheetSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/character_sheet_system.h"

using namespace atlas;

static void testCharacterSheetInit() {
    std::cout << "\n=== CharacterSheetSystem: Init ===" << std::endl;
    ecs::World world;
    systems::CharacterSheetSystem sys(&world);
    world.createEntity("e1");

    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(sys.getCharacterName("e1") == "", "Name empty initially");
    assertTrue(sys.getRace("e1") == "Caldari", "Default race is Caldari");
    assertTrue(sys.getGender("e1") == "male", "Default gender is male");
    assertTrue(approxEqual(sys.getSecurityStatus("e1"), 0.0f), "Default sec status 0");
    assertTrue(sys.getImplantCount("e1") == 0, "No implants initially");
    assertTrue(sys.getEmploymentCount("e1") == 0, "No employment initially");
    assertTrue(sys.getCloneGrade("e1") == "foundry", "Default clone grade");
    assertTrue(sys.getCloneJumpCooldown("e1") == 0, "No cooldown initially");
    assertTrue(!sys.isOnCloneCooldown("e1"), "Not on cooldown initially");
}

static void testCharacterSheetIdentity() {
    std::cout << "\n=== CharacterSheetSystem: Identity ===" << std::endl;
    ecs::World world;
    systems::CharacterSheetSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setCharacterName("e1", "Pilot Alpha"), "Set name");
    assertTrue(sys.getCharacterName("e1") == "Pilot Alpha", "Name matches");
    assertTrue(!sys.setCharacterName("e1", ""), "Reject empty name");
    assertTrue(sys.setRace("e1", "Amarr"), "Set race");
    assertTrue(sys.getRace("e1") == "Amarr", "Race matches");
    assertTrue(!sys.setRace("e1", ""), "Reject empty race");
    assertTrue(sys.setBloodline("e1", "Khanid"), "Set bloodline");
    assertTrue(sys.getBloodline("e1") == "Khanid", "Bloodline matches");
    assertTrue(sys.setAncestry("e1", "Cyber Knights"), "Set ancestry");
    assertTrue(sys.getAncestry("e1") == "Cyber Knights", "Ancestry matches");
    assertTrue(sys.setGender("e1", "female"), "Set gender female");
    assertTrue(sys.getGender("e1") == "female", "Gender matches");
    assertTrue(!sys.setGender("e1", "other"), "Reject invalid gender");
    assertTrue(sys.getGender("e1") == "female", "Gender unchanged after reject");
    assertTrue(sys.setDateOfBirth("e1", 1000.0f), "Set DOB");
    assertTrue(approxEqual(sys.getDateOfBirth("e1"), 1000.0f), "DOB matches");
    assertTrue(!sys.setDateOfBirth("e1", -1.0f), "Reject negative DOB");
}

static void testCharacterSheetAttributes() {
    std::cout << "\n=== CharacterSheetSystem: Attributes ===" << std::endl;
    ecs::World world;
    systems::CharacterSheetSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.getBaseAttribute("e1", "intelligence") == 20, "Default intelligence");
    assertTrue(sys.getBaseAttribute("e1", "perception") == 20, "Default perception");
    assertTrue(sys.getBaseAttribute("e1", "charisma") == 19, "Default charisma");
    assertTrue(sys.getBaseAttribute("e1", "willpower") == 20, "Default willpower");
    assertTrue(sys.getBaseAttribute("e1", "memory") == 20, "Default memory");

    assertTrue(sys.setAttribute("e1", "intelligence", 25), "Set intelligence");
    assertTrue(sys.getBaseAttribute("e1", "intelligence") == 25, "Intelligence updated");
    assertTrue(!sys.setAttribute("e1", "intelligence", -1), "Reject negative attribute");
    assertTrue(sys.getBaseAttribute("e1", "intelligence") == 25, "Intelligence unchanged");
    assertTrue(!sys.setAttribute("e1", "foo", 10), "Reject unknown attribute");
    assertTrue(sys.getBaseAttribute("e1", "foo") == 0, "Unknown attribute returns 0");
}

static void testCharacterSheetSecurityStatus() {
    std::cout << "\n=== CharacterSheetSystem: Security Status ===" << std::endl;
    ecs::World world;
    systems::CharacterSheetSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setSecurityStatus("e1", 5.0f), "Set sec status 5");
    assertTrue(approxEqual(sys.getSecurityStatus("e1"), 5.0f), "Sec status is 5");
    assertTrue(sys.setSecurityStatus("e1", -10.0f), "Set sec status -10");
    assertTrue(approxEqual(sys.getSecurityStatus("e1"), -10.0f), "Sec status clamped to -10");
    assertTrue(sys.setSecurityStatus("e1", 15.0f), "Set sec status 15 (over max)");
    assertTrue(approxEqual(sys.getSecurityStatus("e1"), 10.0f), "Sec status clamped to 10");

    assertTrue(sys.setSecurityStatus("e1", 0.0f), "Reset sec status");
    assertTrue(sys.adjustSecurityStatus("e1", 3.5f), "Adjust +3.5");
    assertTrue(approxEqual(sys.getSecurityStatus("e1"), 3.5f), "Sec status is 3.5");
    assertTrue(sys.adjustSecurityStatus("e1", -5.0f), "Adjust -5");
    assertTrue(approxEqual(sys.getSecurityStatus("e1"), -1.5f), "Sec status is -1.5");
    assertTrue(sys.adjustSecurityStatus("e1", -20.0f), "Adjust -20 (clamp)");
    assertTrue(approxEqual(sys.getSecurityStatus("e1"), -10.0f), "Sec status clamped at -10");
}

static void testCharacterSheetClone() {
    std::cout << "\n=== CharacterSheetSystem: Clone ===" << std::endl;
    ecs::World world;
    systems::CharacterSheetSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setCloneGrade("e1", "apex"), "Set clone grade");
    assertTrue(sys.getCloneGrade("e1") == "apex", "Clone grade matches");
    assertTrue(!sys.setCloneGrade("e1", ""), "Reject empty grade");

    assertTrue(sys.setCloneLocation("e1", "station_01"), "Set clone location");
    assertTrue(sys.getCloneLocation("e1") == "station_01", "Clone location matches");

    assertTrue(sys.setCloneJumpCooldown("e1", 3600), "Set cooldown");
    assertTrue(sys.getCloneJumpCooldown("e1") == 3600, "Cooldown matches");
    assertTrue(sys.isOnCloneCooldown("e1"), "On cooldown");
    assertTrue(!sys.setCloneJumpCooldown("e1", -1), "Reject negative cooldown");
}

static void testCharacterSheetImplants() {
    std::cout << "\n=== CharacterSheetSystem: Implants ===" << std::endl;
    ecs::World world;
    systems::CharacterSheetSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addImplant("e1", "imp1", "Neural Boost", 1, "intelligence", 3), "Add implant slot 1");
    assertTrue(sys.getImplantCount("e1") == 1, "1 implant");
    assertTrue(sys.hasImplant("e1", "imp1"), "Has imp1");
    assertTrue(sys.getImplantSlot("e1", "imp1") == 1, "imp1 in slot 1");

    // Effective attribute check
    assertTrue(sys.getEffectiveAttribute("e1", "intelligence") == 23, "Effective intelligence with implant");
    assertTrue(sys.getBaseAttribute("e1", "intelligence") == 20, "Base intelligence unchanged");

    assertTrue(sys.addImplant("e1", "imp2", "Memory Augmentor", 2, "memory", 5), "Add implant slot 2");
    assertTrue(sys.getImplantCount("e1") == 2, "2 implants");
    assertTrue(sys.getEffectiveAttribute("e1", "memory") == 25, "Effective memory with implant");

    // Duplicate ID prevention
    assertTrue(!sys.addImplant("e1", "imp1", "Dup", 3, "charisma", 1), "Reject duplicate ID");
    // Slot occupancy
    assertTrue(!sys.addImplant("e1", "imp3", "Conflict", 1, "charisma", 1), "Reject occupied slot");
    // Invalid slot
    assertTrue(!sys.addImplant("e1", "imp3", "Bad", 0, "charisma", 1), "Reject slot 0");
    assertTrue(!sys.addImplant("e1", "imp3", "Bad", 11, "charisma", 1), "Reject slot 11");
    // Empty ID
    assertTrue(!sys.addImplant("e1", "", "Bad", 3, "charisma", 1), "Reject empty ID");
    // Negative bonus
    assertTrue(!sys.addImplant("e1", "imp3", "Bad", 3, "charisma", -1), "Reject negative bonus");

    assertTrue(sys.removeImplant("e1", "imp1"), "Remove imp1");
    assertTrue(sys.getImplantCount("e1") == 1, "1 implant after removal");
    assertTrue(!sys.hasImplant("e1", "imp1"), "imp1 removed");
    assertTrue(!sys.removeImplant("e1", "imp1"), "Cannot remove again");

    assertTrue(sys.clearImplants("e1"), "Clear implants");
    assertTrue(sys.getImplantCount("e1") == 0, "0 implants after clear");
}

static void testCharacterSheetEmployment() {
    std::cout << "\n=== CharacterSheetSystem: Employment ===" << std::endl;
    ecs::World world;
    systems::CharacterSheetSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addEmploymentRecord("e1", "corp1", "Brave Newbies", 100.0f), "Add employment");
    assertTrue(sys.getEmploymentCount("e1") == 1, "1 record");
    assertTrue(sys.addEmploymentRecord("e1", "corp2", "Pandemic Horde", 200.0f), "Add second");
    assertTrue(sys.getEmploymentCount("e1") == 2, "2 records");

    assertTrue(!sys.addEmploymentRecord("e1", "", "Bad", 300.0f), "Reject empty corp_id");
    assertTrue(!sys.addEmploymentRecord("e1", "corp3", "", 300.0f), "Reject empty corp_name");

    assertTrue(sys.endEmployment("e1", "corp1", 150.0f), "End employment");
    assertTrue(!sys.endEmployment("e1", "corp1", 160.0f), "Cannot end already ended");
    assertTrue(!sys.endEmployment("e1", "nonexist", 160.0f), "Cannot end nonexistent");

    assertTrue(sys.clearEmploymentHistory("e1"), "Clear history");
    assertTrue(sys.getEmploymentCount("e1") == 0, "0 records after clear");
}

static void testCharacterSheetUpdate() {
    std::cout << "\n=== CharacterSheetSystem: Update ===" << std::endl;
    ecs::World world;
    systems::CharacterSheetSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.setCloneJumpCooldown("e1", 100);
    assertTrue(sys.getCloneJumpCooldown("e1") == 100, "Cooldown is 100");

    sys.update(1.0f);
    assertTrue(sys.getCloneJumpCooldown("e1") == 99, "Cooldown decremented to 99");

    sys.update(50.0f);
    assertTrue(sys.getCloneJumpCooldown("e1") == 49, "Cooldown decremented to 49");

    sys.update(100.0f);
    assertTrue(sys.getCloneJumpCooldown("e1") == 0, "Cooldown clamped at 0");
    assertTrue(!sys.isOnCloneCooldown("e1"), "No longer on cooldown");

    // Should not go negative
    sys.update(10.0f);
    assertTrue(sys.getCloneJumpCooldown("e1") == 0, "Cooldown stays at 0");
}

static void testCharacterSheetMissing() {
    std::cout << "\n=== CharacterSheetSystem: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CharacterSheetSystem sys(&world);

    // All mutating methods return false
    assertTrue(!sys.initialize("none"), "Init fails");
    assertTrue(!sys.setCharacterName("none", "X"), "setCharacterName fails");
    assertTrue(!sys.setRace("none", "X"), "setRace fails");
    assertTrue(!sys.setBloodline("none", "X"), "setBloodline fails");
    assertTrue(!sys.setAncestry("none", "X"), "setAncestry fails");
    assertTrue(!sys.setGender("none", "male"), "setGender fails");
    assertTrue(!sys.setDateOfBirth("none", 1.0f), "setDateOfBirth fails");
    assertTrue(!sys.setAttribute("none", "intelligence", 10), "setAttribute fails");
    assertTrue(!sys.setSecurityStatus("none", 1.0f), "setSecurityStatus fails");
    assertTrue(!sys.adjustSecurityStatus("none", 1.0f), "adjustSecurityStatus fails");
    assertTrue(!sys.setCloneGrade("none", "apex"), "setCloneGrade fails");
    assertTrue(!sys.setCloneLocation("none", "s1"), "setCloneLocation fails");
    assertTrue(!sys.setCloneJumpCooldown("none", 10), "setCloneJumpCooldown fails");
    assertTrue(!sys.addImplant("none", "i1", "X", 1, "intel", 1), "addImplant fails");
    assertTrue(!sys.removeImplant("none", "i1"), "removeImplant fails");
    assertTrue(!sys.clearImplants("none"), "clearImplants fails");
    assertTrue(!sys.addEmploymentRecord("none", "c1", "C", 1.0f), "addEmploymentRecord fails");
    assertTrue(!sys.endEmployment("none", "c1", 2.0f), "endEmployment fails");
    assertTrue(!sys.clearEmploymentHistory("none"), "clearEmploymentHistory fails");

    // All queries return safe defaults
    assertTrue(sys.getCharacterName("none") == "", "getCharacterName default");
    assertTrue(sys.getRace("none") == "", "getRace default");
    assertTrue(sys.getBloodline("none") == "", "getBloodline default");
    assertTrue(sys.getAncestry("none") == "", "getAncestry default");
    assertTrue(sys.getGender("none") == "", "getGender default");
    assertTrue(approxEqual(sys.getDateOfBirth("none"), 0.0f), "getDateOfBirth default");
    assertTrue(approxEqual(sys.getSecurityStatus("none"), 0.0f), "getSecurityStatus default");
    assertTrue(sys.getBaseAttribute("none", "intelligence") == 0, "getBaseAttribute default");
    assertTrue(sys.getEffectiveAttribute("none", "intelligence") == 0, "getEffectiveAttribute default");
    assertTrue(sys.getCloneGrade("none") == "", "getCloneGrade default");
    assertTrue(sys.getCloneLocation("none") == "", "getCloneLocation default");
    assertTrue(sys.getCloneJumpCooldown("none") == 0, "getCloneJumpCooldown default");
    assertTrue(!sys.isOnCloneCooldown("none"), "isOnCloneCooldown default");
    assertTrue(sys.getImplantCount("none") == 0, "getImplantCount default");
    assertTrue(!sys.hasImplant("none", "i1"), "hasImplant default");
    assertTrue(sys.getImplantSlot("none", "i1") == 0, "getImplantSlot default");
    assertTrue(sys.getEmploymentCount("none") == 0, "getEmploymentCount default");
}

void run_character_sheet_system_tests() {
    testCharacterSheetInit();
    testCharacterSheetIdentity();
    testCharacterSheetAttributes();
    testCharacterSheetSecurityStatus();
    testCharacterSheetClone();
    testCharacterSheetImplants();
    testCharacterSheetEmployment();
    testCharacterSheetUpdate();
    testCharacterSheetMissing();
}
