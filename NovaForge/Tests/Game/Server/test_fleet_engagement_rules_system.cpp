// Tests for: FleetEngagementRulesSystem
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_engagement_rules_system.h"

using namespace atlas;
using RoeP = components::FleetEngagementRulesState::RoeProfile;
using TgtP = components::FleetEngagementRulesState::TargetPriority;

static void testFleetEngagementInit() {
    std::cout << "\n=== FleetEngagement: Init ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getRoe("e1") == RoeP::Defensive, "Default ROE is Defensive");
    assertTrue(sys.getRoeString("e1") == "Defensive", "ROE string Defensive");
    assertTrue(sys.getPrimaryTarget("e1") == TgtP::Any, "Default target Any");
    assertTrue(sys.isAutoEngageHostiles("e1"), "Auto-engage hostiles true by default");
    assertTrue(!sys.isAutoEngageNeutrals("e1"), "Auto-engage neutrals false by default");
    assertTrue(approxEqual(sys.getRangeLimit("e1"), 0.0f), "Range limit 0 by default");
    assertTrue(sys.getBroadcastTarget("e1").empty(), "No broadcast target by default");
    assertTrue(!sys.isAllHandsFire("e1"), "All hands fire false by default");
    assertTrue(!sys.isCeaseFire("e1"), "Cease fire false by default");
    assertTrue(!sys.isInCombat("e1"), "Not in combat by default");
    assertTrue(sys.getTotalEngagements("e1") == 0, "Zero engagements");
    assertTrue(sys.getTotalDisengages("e1") == 0, "Zero disengages");
    assertTrue(approxEqual(sys.getTimeSinceLastEngagement("e1"), 0.0f), "Zero time since last engagement");
    assertTrue(sys.getFleetId("e1").empty(), "Empty fleet_id");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFleetEngagementRoe() {
    std::cout << "\n=== FleetEngagement: ROE ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setRoe("e1", RoeP::Aggressive), "Set Aggressive");
    assertTrue(sys.getRoe("e1") == RoeP::Aggressive, "ROE is Aggressive");
    assertTrue(sys.getRoeString("e1") == "Aggressive", "ROE string Aggressive");

    assertTrue(sys.setRoe("e1", RoeP::Skirmish), "Set Skirmish");
    assertTrue(sys.getRoeString("e1") == "Skirmish", "ROE string Skirmish");

    assertTrue(sys.setRoe("e1", RoeP::Doctrine), "Set Doctrine");
    assertTrue(sys.getRoeString("e1") == "Doctrine", "ROE string Doctrine");

    // Passive forces cease_fire and clears all_hands_fire
    sys.allHandsFire("e1");
    assertTrue(sys.isAllHandsFire("e1"), "All hands fire set");
    assertTrue(sys.setRoe("e1", RoeP::Passive), "Set Passive");
    assertTrue(sys.getRoeString("e1") == "Passive", "ROE string Passive");
    assertTrue(sys.isCeaseFire("e1"), "Passive forces cease_fire");
    assertTrue(!sys.isAllHandsFire("e1"), "Passive clears all_hands_fire");

    assertTrue(!sys.setRoe("missing", RoeP::Defensive), "ROE on missing fails");
}

static void testFleetEngagementRoePassiveNeutrals() {
    std::cout << "\n=== FleetEngagement: Passive blocks neutrals ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.setRoe("e1", RoeP::Passive);
    assertTrue(!sys.setAutoEngageNeutrals("e1", true), "Passive blocks auto_engage_neutrals=true");
    assertTrue(!sys.isAutoEngageNeutrals("e1"), "Neutrals still false");

    sys.setRoe("e1", RoeP::Defensive);
    assertTrue(sys.setAutoEngageNeutrals("e1", true), "Defensive allows neutrals=true");
    assertTrue(sys.isAutoEngageNeutrals("e1"), "Neutrals now true");
    assertTrue(sys.setAutoEngageNeutrals("e1", false), "Can clear neutrals");
    assertTrue(!sys.isAutoEngageNeutrals("e1"), "Neutrals cleared");
}

static void testFleetEngagementTargetPriority() {
    std::cout << "\n=== FleetEngagement: Target Priority ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setPrimaryTarget("e1", TgtP::Structures), "Set Structures");
    assertTrue(sys.getPrimaryTarget("e1") == TgtP::Structures, "Target Structures");

    assertTrue(sys.setPrimaryTarget("e1", TgtP::Capitals), "Set Capitals");
    assertTrue(sys.getPrimaryTarget("e1") == TgtP::Capitals, "Target Capitals");

    assertTrue(sys.setPrimaryTarget("e1", TgtP::Battleships), "Set Battleships");
    assertTrue(sys.getPrimaryTarget("e1") == TgtP::Battleships, "Target Battleships");

    assertTrue(sys.setPrimaryTarget("e1", TgtP::Cruisers), "Set Cruisers");
    assertTrue(sys.getPrimaryTarget("e1") == TgtP::Cruisers, "Target Cruisers");

    assertTrue(sys.setPrimaryTarget("e1", TgtP::Frigates), "Set Frigates");
    assertTrue(sys.getPrimaryTarget("e1") == TgtP::Frigates, "Target Frigates");

    assertTrue(sys.setPrimaryTarget("e1", TgtP::Any), "Set Any");
    assertTrue(sys.getPrimaryTarget("e1") == TgtP::Any, "Target Any");

    assertTrue(!sys.setPrimaryTarget("missing", TgtP::Any), "Target on missing fails");
}

static void testFleetEngagementRangeLimit() {
    std::cout << "\n=== FleetEngagement: RangeLimit ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setRangeLimit("e1", 500.0f), "Set range 500");
    assertTrue(approxEqual(sys.getRangeLimit("e1"), 500.0f), "Range is 500");

    assertTrue(sys.setRangeLimit("e1", 0.0f), "Set range 0 (valid)");
    assertTrue(approxEqual(sys.getRangeLimit("e1"), 0.0f), "Range is 0");

    assertTrue(!sys.setRangeLimit("e1", -1.0f), "Negative range rejected");
    assertTrue(approxEqual(sys.getRangeLimit("e1"), 0.0f), "Range unchanged after rejection");

    assertTrue(!sys.setRangeLimit("missing", 100.0f), "Range on missing fails");
}

static void testFleetEngagementBroadcastTarget() {
    std::cout << "\n=== FleetEngagement: BroadcastTarget ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.broadcastTarget("e1", "target-ship-42"), "Set broadcast target");
    assertTrue(sys.getBroadcastTarget("e1") == "target-ship-42", "Broadcast target matches");

    assertTrue(sys.broadcastTarget("e1", ""), "Clear broadcast target (empty string valid)");
    assertTrue(sys.getBroadcastTarget("e1").empty(), "Broadcast target cleared");

    assertTrue(!sys.broadcastTarget("missing", "x"), "Broadcast on missing fails");
}

static void testFleetEngagementAllHandsFire() {
    std::cout << "\n=== FleetEngagement: AllHandsFire ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.ceaseFire("e1");
    assertTrue(sys.isCeaseFire("e1"), "Cease fire set before");

    assertTrue(sys.allHandsFire("e1"), "allHandsFire succeeds");
    assertTrue(sys.isAllHandsFire("e1"), "all_hands_fire = true");
    assertTrue(!sys.isCeaseFire("e1"), "cease_fire cleared");
    assertTrue(sys.isInCombat("e1"), "in_combat set");
    assertTrue(sys.getTotalEngagements("e1") == 1, "total_engagements incremented");

    assertTrue(!sys.allHandsFire("missing"), "allHandsFire on missing fails");
}

static void testFleetEngagementCeaseFire() {
    std::cout << "\n=== FleetEngagement: CeaseFire ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.allHandsFire("e1");
    assertTrue(sys.isAllHandsFire("e1"), "All hands fire before cease");

    assertTrue(sys.ceaseFire("e1"), "ceaseFire succeeds");
    assertTrue(sys.isCeaseFire("e1"), "cease_fire = true");
    assertTrue(!sys.isAllHandsFire("e1"), "all_hands_fire cleared");

    assertTrue(!sys.ceaseFire("missing"), "ceaseFire on missing fails");
}

static void testFleetEngagementCombatTracking() {
    std::cout << "\n=== FleetEngagement: CombatTracking ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setInCombat("e1", true), "setInCombat true");
    assertTrue(sys.isInCombat("e1"), "in_combat = true");
    assertTrue(sys.getTotalEngagements("e1") == 1, "1 engagement");
    assertTrue(approxEqual(sys.getTimeSinceLastEngagement("e1"), 0.0f), "Time reset on engage");

    assertTrue(sys.setInCombat("e1", false), "setInCombat false");
    assertTrue(!sys.isInCombat("e1"), "in_combat = false");
    assertTrue(sys.getTotalDisengages("e1") == 1, "1 disengage");

    assertTrue(sys.setInCombat("e1", true), "setInCombat true again");
    assertTrue(sys.getTotalEngagements("e1") == 2, "2 engagements");

    assertTrue(!sys.setInCombat("missing", true), "setInCombat on missing fails");
}

static void testFleetEngagementTimeTick() {
    std::cout << "\n=== FleetEngagement: TimeTick ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    // Not in combat: time advances
    sys.update(5.0f);
    assertTrue(sys.getTimeSinceLastEngagement("e1") > 0.0f, "Time advances when not in combat");

    float t = sys.getTimeSinceLastEngagement("e1");
    sys.update(3.0f);
    assertTrue(sys.getTimeSinceLastEngagement("e1") > t, "Time continues to advance");

    // In combat: time does not advance
    sys.setInCombat("e1", true);
    float t2 = sys.getTimeSinceLastEngagement("e1");
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getTimeSinceLastEngagement("e1"), t2), "Time pauses when in combat");
}

static void testFleetEngagementFleetId() {
    std::cout << "\n=== FleetEngagement: FleetId ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setFleetId("e1", "fleet-1"), "Set fleet_id");
    assertTrue(sys.getFleetId("e1") == "fleet-1", "fleet_id matches");
    assertTrue(!sys.setFleetId("e1", ""), "Empty fleet_id rejected");
    assertTrue(sys.getFleetId("e1") == "fleet-1", "fleet_id unchanged after rejection");
    assertTrue(!sys.setFleetId("missing", "x"), "FleetId on missing fails");
}

static void testFleetEngagementMissingEntity() {
    std::cout << "\n=== FleetEngagement: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetEngagementRulesSystem sys(&world);

    assertTrue(sys.getRoe("missing") == RoeP::Defensive, "getRoe = Defensive default");
    assertTrue(sys.getRoeString("missing").empty(), "getRoeString = '' for missing");
    assertTrue(sys.getPrimaryTarget("missing") == TgtP::Any, "getPrimaryTarget = Any default");
    assertTrue(!sys.isAutoEngageHostiles("missing"), "isAutoEngageHostiles = false");
    assertTrue(!sys.isAutoEngageNeutrals("missing"), "isAutoEngageNeutrals = false");
    assertTrue(approxEqual(sys.getRangeLimit("missing"), 0.0f), "getRangeLimit = 0");
    assertTrue(sys.getBroadcastTarget("missing").empty(), "getBroadcastTarget = ''");
    assertTrue(!sys.isAllHandsFire("missing"), "isAllHandsFire = false");
    assertTrue(!sys.isCeaseFire("missing"), "isCeaseFire = false");
    assertTrue(!sys.isInCombat("missing"), "isInCombat = false");
    assertTrue(sys.getTotalEngagements("missing") == 0, "getTotalEngagements = 0");
    assertTrue(sys.getTotalDisengages("missing") == 0, "getTotalDisengages = 0");
    assertTrue(approxEqual(sys.getTimeSinceLastEngagement("missing"), 0.0f), "getTimeSinceLastEngagement = 0");
    assertTrue(sys.getFleetId("missing").empty(), "getFleetId = ''");
    assertTrue(!sys.setRoe("missing", RoeP::Aggressive), "setRoe on missing fails");
    assertTrue(!sys.setPrimaryTarget("missing", TgtP::Capitals), "setPrimaryTarget on missing fails");
    assertTrue(!sys.setAutoEngageHostiles("missing", false), "setAutoEngageHostiles on missing fails");
    assertTrue(!sys.setAutoEngageNeutrals("missing", false), "setAutoEngageNeutrals on missing fails");
    assertTrue(!sys.allHandsFire("missing"), "allHandsFire on missing fails");
    assertTrue(!sys.ceaseFire("missing"), "ceaseFire on missing fails");
}

void run_fleet_engagement_rules_system_tests() {
    testFleetEngagementInit();
    testFleetEngagementRoe();
    testFleetEngagementRoePassiveNeutrals();
    testFleetEngagementTargetPriority();
    testFleetEngagementRangeLimit();
    testFleetEngagementBroadcastTarget();
    testFleetEngagementAllHandsFire();
    testFleetEngagementCeaseFire();
    testFleetEngagementCombatTracking();
    testFleetEngagementTimeTick();
    testFleetEngagementFleetId();
    testFleetEngagementMissingEntity();
}
