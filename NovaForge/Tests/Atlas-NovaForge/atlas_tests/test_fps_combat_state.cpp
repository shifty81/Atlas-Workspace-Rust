/**
 * Tests for FPSCombatStateMachine (Phase 23 — Tri-Modal Gameplay):
 *   - FPSCombatState enum names
 *   - Initial state (EXPLORING)
 *   - Threat detection, combat entry, damage, healing
 *   - State transitions: WOUNDED, BREACHED, DOWNED, REVIVED
 *   - Shield recharge, bleed-out tick
 *   - State callback
 */

#include <cassert>
#include <string>
#include <vector>
#include "../cpp_client/include/gameplay/fps_combat_state.h"

using namespace atlas::gameplay;

void test_fps_state_names() {
    assert(std::string(FPSCombatStateName(FPSCombatState::EXPLORING)) == "Exploring");
    assert(std::string(FPSCombatStateName(FPSCombatState::ALERT))     == "Alert");
    assert(std::string(FPSCombatStateName(FPSCombatState::ENGAGED))   == "Engaged");
    assert(std::string(FPSCombatStateName(FPSCombatState::WOUNDED))   == "Wounded");
    assert(std::string(FPSCombatStateName(FPSCombatState::BREACHED))  == "Breached");
    assert(std::string(FPSCombatStateName(FPSCombatState::DOWNED))    == "Downed");
}

void test_fps_initial_state() {
    FPSCombatStateMachine sm;
    assert(sm.State() == FPSCombatState::EXPLORING);
    assert(sm.IsAlive());
    assert(!sm.IsEngaged());
}

void test_fps_threat_detected_transition() {
    FPSCombatStateMachine sm;
    sm.OnThreatDetected();
    assert(sm.State() == FPSCombatState::ALERT);
}

void test_fps_threat_detected_from_non_exploring_noop() {
    FPSCombatStateMachine sm;
    sm.OnThreatDetected();   // EXPLORING → ALERT
    sm.OnCombatEntered();    // ALERT → ENGAGED
    sm.OnThreatDetected();   // already ENGAGED — should remain ENGAGED
    assert(sm.State() == FPSCombatState::ENGAGED);
}

void test_fps_combat_entered_from_exploring() {
    FPSCombatStateMachine sm;
    sm.OnCombatEntered();
    assert(sm.State() == FPSCombatState::ENGAGED);
    assert(sm.IsEngaged());
}

void test_fps_damage_drains_shield_first() {
    FPSCombatStateMachine sm;
    sm.Stats().shield    = 50.0f;
    sm.Stats().health    = 100.0f;
    sm.Stats().maxHealth = 100.0f;

    sm.OnDamageReceived(30.0f);
    // Shield absorbed all 30
    assert(sm.Stats().shield == 20.0f);
    assert(sm.Stats().health == 100.0f);
    assert(sm.State() == FPSCombatState::ENGAGED);
}

void test_fps_damage_spills_to_health() {
    FPSCombatStateMachine sm;
    sm.Stats().shield    = 10.0f;
    sm.Stats().health    = 100.0f;
    sm.Stats().maxHealth = 100.0f;

    sm.OnDamageReceived(30.0f);
    assert(sm.Stats().shield == 0.0f);
    assert(sm.Stats().health == 80.0f);
}

void test_fps_wounded_transition() {
    FPSCombatStateMachine sm;
    sm.Stats().shield    = 0.0f;
    sm.Stats().health    = 100.0f;
    sm.Stats().maxHealth = 100.0f;

    // Deal enough damage to drop below 50 %
    sm.OnDamageReceived(60.0f);
    assert(sm.State() == FPSCombatState::WOUNDED);

    // Speed penalty applied
    float base = 5.0f;
    float expected = base * (1.0f - FPSCombatStateMachine::WOUNDED_SPEED_PENALTY);
    assert(sm.Stats().moveSpeed == expected);
}

void test_fps_breached_transition() {
    FPSCombatStateMachine sm;
    sm.OnCombatEntered();
    sm.OnSuitBreached();
    assert(sm.State() == FPSCombatState::BREACHED);
    assert(sm.Stats().suitBreached);
    assert(sm.Stats().aimSway == FPSCombatStateMachine::BREACHED_AIM_SWAY);
}

void test_fps_downed_transition() {
    FPSCombatStateMachine sm;
    sm.Stats().shield    = 0.0f;
    sm.Stats().health    = 10.0f;
    sm.Stats().maxHealth = 100.0f;
    sm.OnDamageReceived(10.0f);
    assert(sm.State() == FPSCombatState::DOWNED);
    assert(!sm.IsAlive());
    assert(sm.Stats().moveSpeed == 0.0f);
}

void test_fps_revived_from_downed() {
    FPSCombatStateMachine sm;
    sm.Stats().shield    = 0.0f;
    sm.Stats().health    = 5.0f;
    sm.Stats().maxHealth = 100.0f;
    sm.OnDamageReceived(5.0f);
    assert(sm.State() == FPSCombatState::DOWNED);

    sm.OnRevived();
    assert(sm.State() == FPSCombatState::ALERT);
    assert(sm.IsAlive());
    assert(sm.Stats().health > 0.0f);
}

void test_fps_all_clear_returns_to_exploring() {
    FPSCombatStateMachine sm;
    sm.OnCombatEntered();
    sm.OnAllClearSignal();
    assert(sm.State() == FPSCombatState::EXPLORING);
}

void test_fps_injector_heals_in_engaged() {
    FPSCombatStateMachine sm;
    sm.Stats().shield    = 0.0f;
    sm.Stats().health    = 40.0f;
    sm.Stats().maxHealth = 100.0f;
    sm.OnCombatEntered();
    sm.OnInjectorUsed();
    assert(sm.Stats().health == 70.0f);  // 40 + 30% of 100
    assert(sm.Stats().injectorUsed);
}

void test_fps_injector_can_exit_wounded() {
    FPSCombatStateMachine sm;
    sm.Stats().shield    = 0.0f;
    sm.Stats().health    = 100.0f;
    sm.Stats().maxHealth = 100.0f;
    sm.OnDamageReceived(60.0f);
    assert(sm.State() == FPSCombatState::WOUNDED);

    sm.OnInjectorUsed();
    // 40 + 30 = 70, above 50% threshold
    assert(sm.State() == FPSCombatState::ENGAGED);
}

void test_fps_shield_recharges_after_delay() {
    FPSCombatStateMachine sm;
    sm.Stats().shield    = 0.0f;
    sm.Stats().maxShield = 50.0f;

    // Recharge delay must first expire
    float delay = FPSCombatStateMachine::SHIELD_RECHARGE_DELAY;
    sm.Tick(delay + 0.1f);  // exceed delay
    assert(sm.Stats().shield > 0.0f);
}

void test_fps_state_callback_fired() {
    FPSCombatStateMachine sm;

    std::vector<std::pair<FPSCombatState, FPSCombatState>> transitions;
    sm.SetStateCallback([&](FPSCombatState prev, FPSCombatState next) {
        transitions.push_back({ prev, next });
    });

    sm.OnThreatDetected();   // EXPLORING → ALERT
    sm.OnCombatEntered();    // ALERT → ENGAGED

    assert(transitions.size() == 2);
    assert(transitions[0].first  == FPSCombatState::EXPLORING);
    assert(transitions[0].second == FPSCombatState::ALERT);
    assert(transitions[1].first  == FPSCombatState::ALERT);
    assert(transitions[1].second == FPSCombatState::ENGAGED);
}

void test_fps_is_engaged_states() {
    FPSCombatStateMachine sm;
    sm.Stats().shield    = 0.0f;
    sm.Stats().health    = 100.0f;
    sm.Stats().maxHealth = 100.0f;

    assert(!sm.IsEngaged());
    sm.OnCombatEntered();
    assert(sm.IsEngaged());
    sm.OnDamageReceived(60.0f);
    assert(sm.IsEngaged());  // WOUNDED is still engaged
    sm.OnSuitBreached();
    assert(sm.IsEngaged());  // BREACHED is still engaged
}
