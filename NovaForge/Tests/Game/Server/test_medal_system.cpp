// Tests for: MedalSystem
#include "test_log.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/medal_system.h"

using namespace atlas;

static void testMedalInit() {
    std::cout << "\n=== MedalSystem: Init ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");

    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(sys.getMedalCount("e1") == 0, "No medals initially");
    assertTrue(sys.getAwardCount("e1") == 0, "No awards initially");
    assertTrue(sys.getCorpId("e1") == "", "Corp ID empty initially");
    assertTrue(sys.getTotalMedalsCreated("e1") == 0, "Total medals created 0");
    assertTrue(sys.getTotalAwardsGiven("e1") == 0, "Total awards given 0");
    assertTrue(sys.getTotalAwardsRevoked("e1") == 0, "Total awards revoked 0");
}

static void testMedalCreate() {
    std::cout << "\n=== MedalSystem: Create ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.createMedal("e1", "medal1", "Gold Star", "Outstanding performance", "ceo1"), "Create medal");
    assertTrue(sys.getMedalCount("e1") == 1, "1 medal");
    assertTrue(sys.hasMedal("e1", "medal1"), "Has medal1");
    assertTrue(sys.getMedalName("e1", "medal1") == "Gold Star", "Name matches");
    assertTrue(sys.getMedalDescription("e1", "medal1") == "Outstanding performance", "Description matches");
    assertTrue(sys.getTotalMedalsCreated("e1") == 1, "Total created 1");

    // Duplicate prevention
    assertTrue(!sys.createMedal("e1", "medal1", "Dup", "Dup", "ceo1"), "Reject duplicate ID");
    assertTrue(sys.getMedalCount("e1") == 1, "Still 1 medal");

    // Validation
    assertTrue(!sys.createMedal("e1", "", "Name", "Desc", "ceo1"), "Reject empty medal_id");
    assertTrue(!sys.createMedal("e1", "medal2", "", "Desc", "ceo1"), "Reject empty name");

    // Second medal
    assertTrue(sys.createMedal("e1", "medal2", "Silver Star", "Good work", "ceo1"), "Create second medal");
    assertTrue(sys.getMedalCount("e1") == 2, "2 medals");
    assertTrue(sys.getTotalMedalsCreated("e1") == 2, "Total created 2");
}

static void testMedalRemove() {
    std::cout << "\n=== MedalSystem: Remove ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.createMedal("e1", "medal1", "Gold", "Desc", "ceo1");
    sys.createMedal("e1", "medal2", "Silver", "Desc", "ceo1");

    // Award a medal before removing
    sys.awardMedal("e1", "a1", "medal1", "pilot1", "Great job");
    assertTrue(sys.getAwardCount("e1") == 1, "1 award before remove");

    assertTrue(sys.removeMedal("e1", "medal1"), "Remove medal1");
    assertTrue(sys.getMedalCount("e1") == 1, "1 medal left");
    assertTrue(!sys.hasMedal("e1", "medal1"), "medal1 removed");
    assertTrue(sys.getAwardCount("e1") == 0, "Awards for medal1 also removed");

    assertTrue(!sys.removeMedal("e1", "medal1"), "Cannot remove again");
    assertTrue(!sys.removeMedal("e1", "nonexist"), "Cannot remove nonexistent");
}

static void testMedalClear() {
    std::cout << "\n=== MedalSystem: Clear ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.createMedal("e1", "m1", "Gold", "Desc", "ceo1");
    sys.createMedal("e1", "m2", "Silver", "Desc", "ceo1");
    sys.awardMedal("e1", "a1", "m1", "pilot1", "Reason");

    assertTrue(sys.clearMedals("e1"), "Clear medals");
    assertTrue(sys.getMedalCount("e1") == 0, "0 medals after clear");
    assertTrue(sys.getAwardCount("e1") == 0, "0 awards after clear (cascaded)");
}

static void testMedalAward() {
    std::cout << "\n=== MedalSystem: Award ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.createMedal("e1", "medal1", "Gold Star", "Outstanding", "ceo1");
    sys.createMedal("e1", "medal2", "Silver Star", "Good", "ceo1");

    assertTrue(sys.awardMedal("e1", "a1", "medal1", "pilot1", "Top pilot"), "Award medal1 to pilot1");
    assertTrue(sys.getAwardCount("e1") == 1, "1 award");
    assertTrue(sys.hasAward("e1", "a1"), "Has award a1");
    assertTrue(sys.getAwardReason("e1", "a1") == "Top pilot", "Reason matches");
    assertTrue(sys.getTotalAwardsGiven("e1") == 1, "Total given 1");

    // Award same medal to another pilot
    assertTrue(sys.awardMedal("e1", "a2", "medal1", "pilot2", "Also great"), "Award medal1 to pilot2");
    assertTrue(sys.getAwardCount("e1") == 2, "2 awards");
    assertTrue(sys.getAwardCountForMedal("e1", "medal1") == 2, "2 awards for medal1");

    // Award different medal
    assertTrue(sys.awardMedal("e1", "a3", "medal2", "pilot1", "Consistent"), "Award medal2 to pilot1");
    assertTrue(sys.getAwardCountForRecipient("e1", "pilot1") == 2, "pilot1 has 2 awards");
    assertTrue(sys.getAwardCountForRecipient("e1", "pilot2") == 1, "pilot2 has 1 award");

    // Duplicate award_id prevention
    assertTrue(!sys.awardMedal("e1", "a1", "medal1", "pilot3", "Dup"), "Reject duplicate award_id");

    // Medal must exist
    assertTrue(!sys.awardMedal("e1", "a4", "nonexist", "pilot1", "Bad"), "Reject nonexistent medal");

    // Validation
    assertTrue(!sys.awardMedal("e1", "", "medal1", "pilot1", "R"), "Reject empty award_id");
    assertTrue(!sys.awardMedal("e1", "a4", "medal1", "", "R"), "Reject empty recipient_id");
}

static void testMedalRevoke() {
    std::cout << "\n=== MedalSystem: Revoke ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.createMedal("e1", "medal1", "Gold", "Desc", "ceo1");
    sys.awardMedal("e1", "a1", "medal1", "pilot1", "Reason1");
    sys.awardMedal("e1", "a2", "medal1", "pilot2", "Reason2");

    assertTrue(sys.revokeAward("e1", "a1"), "Revoke a1");
    assertTrue(sys.getAwardCount("e1") == 1, "1 award left");
    assertTrue(!sys.hasAward("e1", "a1"), "a1 removed");
    assertTrue(sys.getTotalAwardsRevoked("e1") == 1, "Total revoked 1");

    assertTrue(!sys.revokeAward("e1", "a1"), "Cannot revoke again");
    assertTrue(!sys.revokeAward("e1", "nonexist"), "Cannot revoke nonexistent");
}

static void testMedalClearAwards() {
    std::cout << "\n=== MedalSystem: Clear Awards ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.createMedal("e1", "medal1", "Gold", "Desc", "ceo1");
    sys.awardMedal("e1", "a1", "medal1", "pilot1", "R1");
    sys.awardMedal("e1", "a2", "medal1", "pilot2", "R2");

    assertTrue(sys.clearAwards("e1"), "Clear awards");
    assertTrue(sys.getAwardCount("e1") == 0, "0 awards after clear");
    assertTrue(sys.getMedalCount("e1") == 1, "Medals preserved");
}

static void testMedalConfiguration() {
    std::cout << "\n=== MedalSystem: Configuration ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setCorpId("e1", "corp_001"), "Set corp ID");
    assertTrue(sys.getCorpId("e1") == "corp_001", "Corp ID matches");

    assertTrue(sys.setMaxMedals("e1", 10), "Set max medals");
    assertTrue(!sys.setMaxMedals("e1", 0), "Reject zero max medals");
    assertTrue(!sys.setMaxMedals("e1", -1), "Reject negative max medals");

    assertTrue(sys.setMaxAwards("e1", 100), "Set max awards");
    assertTrue(!sys.setMaxAwards("e1", 0), "Reject zero max awards");
    assertTrue(!sys.setMaxAwards("e1", -1), "Reject negative max awards");
}

static void testMedalCapacity() {
    std::cout << "\n=== MedalSystem: Capacity ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.setMaxMedals("e1", 2);
    sys.setMaxAwards("e1", 3);

    assertTrue(sys.createMedal("e1", "m1", "Gold", "D", "c1"), "Create m1");
    assertTrue(sys.createMedal("e1", "m2", "Silver", "D", "c1"), "Create m2");
    assertTrue(!sys.createMedal("e1", "m3", "Bronze", "D", "c1"), "Reject m3 (at capacity)");
    assertTrue(sys.getMedalCount("e1") == 2, "2 medals at cap");

    sys.awardMedal("e1", "a1", "m1", "p1", "R");
    sys.awardMedal("e1", "a2", "m1", "p2", "R");
    sys.awardMedal("e1", "a3", "m2", "p1", "R");
    assertTrue(!sys.awardMedal("e1", "a4", "m2", "p3", "R"), "Reject a4 (at capacity)");
    assertTrue(sys.getAwardCount("e1") == 3, "3 awards at cap");
}

static void testMedalUpdate() {
    std::cout << "\n=== MedalSystem: Update ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.createMedal("e1", "m1", "Gold", "D", "c1");
    sys.awardMedal("e1", "a1", "m1", "p1", "R");
    sys.update(1.0f);
    // Update should not alter counts
    assertTrue(sys.getMedalCount("e1") == 1, "Medal count unchanged");
    assertTrue(sys.getAwardCount("e1") == 1, "Award count unchanged");
}

static void testMedalMissing() {
    std::cout << "\n=== MedalSystem: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::MedalSystem sys(&world);

    // Mutating methods return false
    assertTrue(!sys.initialize("none"), "Init fails");
    assertTrue(!sys.createMedal("none", "m1", "N", "D", "c1"), "createMedal fails");
    assertTrue(!sys.removeMedal("none", "m1"), "removeMedal fails");
    assertTrue(!sys.clearMedals("none"), "clearMedals fails");
    assertTrue(!sys.awardMedal("none", "a1", "m1", "p1", "R"), "awardMedal fails");
    assertTrue(!sys.revokeAward("none", "a1"), "revokeAward fails");
    assertTrue(!sys.clearAwards("none"), "clearAwards fails");
    assertTrue(!sys.setCorpId("none", "c1"), "setCorpId fails");
    assertTrue(!sys.setMaxMedals("none", 10), "setMaxMedals fails");
    assertTrue(!sys.setMaxAwards("none", 10), "setMaxAwards fails");

    // Queries return defaults
    assertTrue(sys.getMedalCount("none") == 0, "getMedalCount default");
    assertTrue(sys.getAwardCount("none") == 0, "getAwardCount default");
    assertTrue(!sys.hasMedal("none", "m1"), "hasMedal default");
    assertTrue(!sys.hasAward("none", "a1"), "hasAward default");
    assertTrue(sys.getCorpId("none") == "", "getCorpId default");
    assertTrue(sys.getMedalName("none", "m1") == "", "getMedalName default");
    assertTrue(sys.getMedalDescription("none", "m1") == "", "getMedalDescription default");
    assertTrue(sys.getTotalMedalsCreated("none") == 0, "getTotalMedalsCreated default");
    assertTrue(sys.getTotalAwardsGiven("none") == 0, "getTotalAwardsGiven default");
    assertTrue(sys.getTotalAwardsRevoked("none") == 0, "getTotalAwardsRevoked default");
    assertTrue(sys.getAwardCountForMedal("none", "m1") == 0, "getAwardCountForMedal default");
    assertTrue(sys.getAwardCountForRecipient("none", "p1") == 0, "getAwardCountForRecipient default");
    assertTrue(sys.getAwardReason("none", "a1") == "", "getAwardReason default");
}

void run_medal_system_tests() {
    testMedalInit();
    testMedalCreate();
    testMedalRemove();
    testMedalClear();
    testMedalAward();
    testMedalRevoke();
    testMedalClearAwards();
    testMedalConfiguration();
    testMedalCapacity();
    testMedalUpdate();
    testMedalMissing();
}
