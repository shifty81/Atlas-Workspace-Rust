/**
 * @file fps_combat_state.cpp
 * @brief FPSCombatStateMachine implementation.
 */

#include "gameplay/fps_combat_state.h"
#include <algorithm>

namespace atlas::gameplay {

void FPSCombatStateMachine::TransitionTo(FPSCombatState next) {
    if (m_state == next) return;
    FPSCombatState prev = m_state;
    m_state = next;
    ApplyStateModifiers();
    if (m_callback) m_callback(prev, next);
}

void FPSCombatStateMachine::ApplyStateModifiers() {
    // Reset modifiers first
    float baseSpeed = 5.0f;
    m_stats.moveSpeed = baseSpeed;
    m_stats.aimSway   = 0.0f;

    switch (m_state) {
        case FPSCombatState::WOUNDED:
            m_stats.moveSpeed = baseSpeed * (1.0f - WOUNDED_SPEED_PENALTY);
            m_stats.aimSway   = WOUNDED_AIM_SWAY;
            break;
        case FPSCombatState::BREACHED:
            m_stats.aimSway   = BREACHED_AIM_SWAY;
            m_stats.suitBreached = true;
            break;
        case FPSCombatState::DOWNED:
            m_stats.moveSpeed = 0.0f;
            break;
        default:
            break;
    }
}

void FPSCombatStateMachine::OnThreatDetected() {
    if (m_state == FPSCombatState::EXPLORING)
        TransitionTo(FPSCombatState::ALERT);
}

void FPSCombatStateMachine::OnCombatEntered() {
    if (m_state == FPSCombatState::EXPLORING ||
        m_state == FPSCombatState::ALERT) {
        TransitionTo(FPSCombatState::ENGAGED);
    }
}

void FPSCombatStateMachine::OnDamageReceived(float damage) {
    if (m_state == FPSCombatState::DOWNED) return;

    // Drain shield first
    if (m_stats.shield > 0.0f) {
        float absorbed = std::min(damage, m_stats.shield);
        m_stats.shield -= absorbed;
        damage -= absorbed;
    }

    m_stats.health = std::max(0.0f, m_stats.health - damage);
    m_shieldRechargeTimer = SHIELD_RECHARGE_DELAY;

    if (m_state == FPSCombatState::EXPLORING ||
        m_state == FPSCombatState::ALERT) {
        TransitionTo(FPSCombatState::ENGAGED);
    }

    float healthFraction = m_stats.health / m_stats.maxHealth;

    if (m_stats.health <= 0.0f) {
        OnDowned();
    } else if (healthFraction < WOUNDED_THRESHOLD &&
               m_state != FPSCombatState::WOUNDED &&
               m_state != FPSCombatState::BREACHED) {
        TransitionTo(FPSCombatState::WOUNDED);
    }
}

void FPSCombatStateMachine::OnSuitBreached() {
    if (m_state == FPSCombatState::DOWNED) return;
    TransitionTo(FPSCombatState::BREACHED);
}

void FPSCombatStateMachine::OnDowned() {
    TransitionTo(FPSCombatState::DOWNED);
    m_bleedOutTimer = BLEED_OUT_TIME;
}

void FPSCombatStateMachine::OnRevived() {
    if (m_state != FPSCombatState::DOWNED) return;
    m_stats.health      = m_stats.maxHealth * 0.25f;
    m_stats.suitBreached = false;
    m_bleedOutTimer     = 0.0f;
    TransitionTo(FPSCombatState::ALERT);
}

void FPSCombatStateMachine::OnAllClearSignal() {
    if (m_state == FPSCombatState::DOWNED) return;
    m_stats.suitBreached = false;
    TransitionTo(FPSCombatState::EXPLORING);
}

void FPSCombatStateMachine::OnInjectorUsed() {
    m_stats.injectorUsed = true;
    // Restore 30 % health immediately
    m_stats.health = std::min(m_stats.maxHealth,
                              m_stats.health + m_stats.maxHealth * 0.3f);
    // May exit WOUNDED if health is now above threshold
    float fraction = m_stats.health / m_stats.maxHealth;
    if (fraction >= WOUNDED_THRESHOLD &&
        m_state == FPSCombatState::WOUNDED) {
        TransitionTo(FPSCombatState::ENGAGED);
    }
}

void FPSCombatStateMachine::Tick(float deltaTime) {
    // Shield recharge (only when not in shield-down delay)
    if (m_shieldRechargeTimer > 0.0f) {
        m_shieldRechargeTimer -= deltaTime;
    } else if (m_stats.shield < m_stats.maxShield &&
               m_state != FPSCombatState::DOWNED) {
        m_stats.shield = std::min(m_stats.maxShield,
                                  m_stats.shield + SHIELD_RECHARGE_RATE * deltaTime);
    }

    // Bleed-out timer when downed
    if (m_state == FPSCombatState::DOWNED && m_bleedOutTimer > 0.0f) {
        m_bleedOutTimer -= deltaTime;
        if (m_bleedOutTimer <= 0.0f) {
            m_bleedOutTimer = 0.0f;
            m_stats.health  = 0.0f;
        }
    }
}

} // namespace atlas::gameplay
