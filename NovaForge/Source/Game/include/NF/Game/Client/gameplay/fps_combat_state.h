#pragma once
/**
 * @file fps_combat_state.h
 * @brief Client-side FPS player combat state machine.
 *
 * State diagram:
 *
 *   EXPLORING → ALERT → ENGAGED → WOUNDED → DOWNED
 *                                   ↓
 *                               BREACHED  (hull breach / environmental hazard)
 *
 * Mechanical effects per state:
 *   WOUNDED  — movement speed −15%, breathing audio + vignette, no healing
 *              unless injector used
 *   BREACHED — aim sway increased, gravity may be unpredictable
 *   DOWNED   — no movement or actions; waiting for revive or bleed-out
 */

#include <functional>
#include <string>
#include <cstdint>

namespace atlas::gameplay {

// ── FPS player state ─────────────────────────────────────────────────────────

enum class FPSCombatState : uint8_t {
    EXPLORING,  ///< No known threats
    ALERT,      ///< Threats detected, not yet engaged
    ENGAGED,    ///< Actively in combat
    WOUNDED,    ///< Below 50 % health; mobility penalty
    BREACHED,   ///< Hull / suit breach; environmental hazard
    DOWNED      ///< Incapacitated; awaiting revive or bleed-out
};

inline const char* FPSCombatStateName(FPSCombatState s) {
    switch (s) {
        case FPSCombatState::EXPLORING: return "Exploring";
        case FPSCombatState::ALERT:     return "Alert";
        case FPSCombatState::ENGAGED:   return "Engaged";
        case FPSCombatState::WOUNDED:   return "Wounded";
        case FPSCombatState::BREACHED:  return "Breached";
        case FPSCombatState::DOWNED:    return "Downed";
    }
    return "Unknown";
}

// ── FPS player stats ─────────────────────────────────────────────────────────

struct FPSPlayerStats {
    float health         = 100.0f;
    float maxHealth      = 100.0f;
    float shield         = 50.0f;
    float maxShield      = 50.0f;
    float moveSpeed      = 5.0f;   ///< m/s — reduced in WOUNDED
    float aimSway        = 0.0f;   ///< 0 = steady, positive = unsteady
    bool  suitBreached   = false;
    bool  injectorUsed   = false;  ///< allows healing in ENGAGED if true
};

// ── FPSCombatStateMachine ────────────────────────────────────────────────────

/**
 * Manages client-side FPS player state transitions.
 *
 * Callers feed events (damage received, threat spotted, revived, etc.) and
 * the machine applies the correct state transition and mechanical modifiers.
 */
class FPSCombatStateMachine {
public:
    using StateCallback = std::function<void(FPSCombatState /*prev*/,
                                             FPSCombatState /*next*/)>;

    FPSCombatStateMachine() = default;

    // ── State queries ─────────────────────────────────────────────────

    FPSCombatState  State()  const { return m_state; }
    const FPSPlayerStats& Stats() const { return m_stats; }
    FPSPlayerStats&       Stats()       { return m_stats; }

    bool IsAlive()    const { return m_state != FPSCombatState::DOWNED; }
    bool IsEngaged()  const {
        return m_state == FPSCombatState::ENGAGED ||
               m_state == FPSCombatState::WOUNDED ||
               m_state == FPSCombatState::BREACHED;
    }

    // ── Event inputs ──────────────────────────────────────────────────

    /** A threat has been detected (enters ALERT if EXPLORING). */
    void OnThreatDetected();

    /** Player entered direct combat. */
    void OnCombatEntered();

    /** Damage was applied to the player. */
    void OnDamageReceived(float damage);

    /** Hull / suit breach event. */
    void OnSuitBreached();

    /** Player was downed. */
    void OnDowned();

    /** Player was revived (returns to ALERT). */
    void OnRevived();

    /** All threats cleared (returns to EXPLORING). */
    void OnAllClearSignal();

    /** Injector used — allows in-combat healing once per engagement. */
    void OnInjectorUsed();

    // ── Per-frame ─────────────────────────────────────────────────────

    /** Update shield recharge, bleed-out timer, etc. */
    void Tick(float deltaTime);

    // ── Callback ──────────────────────────────────────────────────────

    void SetStateCallback(StateCallback cb) { m_callback = std::move(cb); }

    // ── Constants ─────────────────────────────────────────────────────

    static constexpr float WOUNDED_THRESHOLD      = 0.5f;  ///< health fraction
    static constexpr float WOUNDED_SPEED_PENALTY  = 0.15f; ///< 15 % reduction
    static constexpr float WOUNDED_AIM_SWAY       = 1.5f;
    static constexpr float BREACHED_AIM_SWAY      = 3.0f;
    static constexpr float SHIELD_RECHARGE_RATE   = 5.0f;  ///< per second
    static constexpr float SHIELD_RECHARGE_DELAY  = 4.0f;  ///< seconds after last hit
    static constexpr float BLEED_OUT_TIME         = 30.0f; ///< seconds until death

private:
    void TransitionTo(FPSCombatState next);
    void ApplyStateModifiers();

    FPSCombatState m_state       = FPSCombatState::EXPLORING;
    FPSPlayerStats m_stats;
    StateCallback  m_callback;

    float m_shieldRechargeTimer = 0.0f;
    float m_bleedOutTimer       = 0.0f;
};

} // namespace atlas::gameplay
