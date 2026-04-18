/**
 * Tests for FleetCommandSystem (Phase 23 — Tri-Modal Gameplay):
 *   - FleetIntent enum and name helpers
 *   - FleetOrder structure
 *   - FleetCommandSystem: IssueOrder, LastOrder, callbacks, counters
 */

#include <cassert>
#include <string>
#include <vector>
#include "../cpp_client/include/gameplay/fleet_command.h"

using namespace atlas::gameplay;

void test_fleet_intent_names() {
    assert(std::string(FleetIntentName(FleetIntent::ENGAGE))  == "Engage");
    assert(std::string(FleetIntentName(FleetIntent::DEFEND))  == "Defend");
    assert(std::string(FleetIntentName(FleetIntent::ESCORT))  == "Escort");
    assert(std::string(FleetIntentName(FleetIntent::RETREAT)) == "Retreat");
    assert(std::string(FleetIntentName(FleetIntent::HOLD))    == "Hold");
    assert(std::string(FleetIntentName(FleetIntent::REGROUP)) == "Regroup");
}

void test_fleet_command_initial_state() {
    FleetCommandSystem sys;
    assert(!sys.HasOrder());
    assert(sys.OrderCount() == 0);
    assert(!sys.IsFleetCommandModeActive());
}

void test_fleet_command_issue_engage() {
    FleetCommandSystem sys;
    sys.IssueOrder(FleetIntent::ENGAGE, "fleet_1", "enemy_42", 0.8f);
    assert(sys.HasOrder());
    assert(sys.OrderCount() == 1);
    assert(sys.LastOrder().intent     == FleetIntent::ENGAGE);
    assert(sys.LastOrder().fleetID    == "fleet_1");
    assert(sys.LastOrder().targetID   == "enemy_42");
    assert(sys.LastOrder().aggression == 0.8f);
}

void test_fleet_command_issue_hold() {
    FleetCommandSystem sys;
    sys.IssueOrder(FleetIntent::HOLD, "fleet_2");
    assert(sys.LastOrder().intent == FleetIntent::HOLD);
    assert(sys.LastOrder().targetID.empty());
}

void test_fleet_command_multiple_orders() {
    FleetCommandSystem sys;
    sys.IssueOrder(FleetIntent::ENGAGE,  "f1");
    sys.IssueOrder(FleetIntent::DEFEND,  "f1");
    sys.IssueOrder(FleetIntent::REGROUP, "f1");
    assert(sys.OrderCount() == 3);
    assert(sys.LastOrder().intent == FleetIntent::REGROUP);
}

void test_fleet_command_callback_fired() {
    FleetCommandSystem sys;

    std::vector<FleetOrder> received;
    sys.SetOrderCallback([&](const FleetOrder& o) {
        received.push_back(o);
    });

    sys.IssueOrder(FleetIntent::ESCORT, "f1", "ally_7", 0.5f);
    assert(received.size() == 1);
    assert(received[0].intent   == FleetIntent::ESCORT);
    assert(received[0].targetID == "ally_7");
}

void test_fleet_command_mode_toggle() {
    FleetCommandSystem sys;
    assert(!sys.IsFleetCommandModeActive());
    sys.SetFleetCommandModeActive(true);
    assert(sys.IsFleetCommandModeActive());
    sys.SetFleetCommandModeActive(false);
    assert(!sys.IsFleetCommandModeActive());
}

void test_fleet_command_time_scale_constant() {
    assert(FleetCommandSystem::FLEET_COMMAND_TIME_SCALE == 0.15f);
}

void test_fleet_command_struct_order() {
    FleetOrder o;
    o.intent     = FleetIntent::RETREAT;
    o.fleetID    = "main";
    o.wingID     = "alpha";
    o.aggression = 0.2f;
    assert(o.intent     == FleetIntent::RETREAT);
    assert(o.wingID     == "alpha");
    assert(o.aggression == 0.2f);
}

void test_fleet_command_no_callback() {
    // Issuing without callback must not crash
    FleetCommandSystem sys;
    sys.IssueOrder(FleetIntent::ENGAGE, "f1");
    assert(sys.HasOrder());
}
