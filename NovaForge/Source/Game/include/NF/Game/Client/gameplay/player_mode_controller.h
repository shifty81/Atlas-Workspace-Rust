#pragma once
/**
 * @file player_mode_controller.h
 * @brief PlayerModeController — state machine for tri-modal gameplay.
 *
 * Manages all mode transitions between FPS, Flight, and Fleet Command.
 * Every transition strictly follows the 6-step protocol:
 *   1. Fade / dissolve control
 *   2. Transfer authority
 *   3. Swap camera
 *   4. Swap input map
 *   5. Swap UI stack
 *   6. Restore context on exit
 *
 * No shortcuts — all transitions go through this controller.
 */

#include "gameplay/player_mode.h"
#include <memory>
#include <functional>
#include <string>

namespace atlas::gameplay {

// ── Transition event ─────────────────────────────────────────────────────────

struct ModeTransition {
    PlayerMode from;
    PlayerMode to;
};

// ── PlayerModeController ─────────────────────────────────────────────────────

/**
 * Owns a set of concrete IPlayerMode instances and drives state transitions.
 *
 * Usage:
 *   controller.RegisterMode(std::make_unique<FPSMode>());
 *   controller.RegisterMode(std::make_unique<FlightMode>());
 *   controller.RegisterMode(std::make_unique<FleetCommandMode>());
 *   controller.TransitionTo(PlayerMode::FLIGHT);
 *   controller.Tick(deltaTime);
 */
class PlayerModeController {
public:
    using TransitionCallback =
        std::function<void(const ModeTransition&)>;

    PlayerModeController();
    ~PlayerModeController() = default;

    // Non-copyable
    PlayerModeController(const PlayerModeController&)            = delete;
    PlayerModeController& operator=(const PlayerModeController&) = delete;

    // ── Mode registration ─────────────────────────────────────────────

    /**
     * Register a concrete player mode.  Each PlayerMode value may only be
     * registered once.
     */
    void RegisterMode(std::unique_ptr<IPlayerMode> mode);

    // ── Transitions ───────────────────────────────────────────────────

    /**
     * Request a transition to a new mode.
     * Calls current->OnExit(), then next->OnEnter().
     * @return true if the transition was executed.
     */
    bool TransitionTo(PlayerMode next);

    // ── Per-frame ─────────────────────────────────────────────────────

    /**
     * Tick the active mode.
     * @param deltaTime Real-time seconds; the mode may apply its own time
     *                  scale internally.
     */
    void Tick(float deltaTime);

    // ── Queries ───────────────────────────────────────────────────────

    PlayerMode CurrentMode() const { return m_currentMode; }
    bool HasMode(PlayerMode mode) const;

    /** Effective time scale for the current mode. */
    float CurrentTimeScale() const;

    /** Ownership flags for the current mode. */
    ModeOwnership CurrentOwnership() const;

    /**
     * True if a transition is currently in progress (fade/dissolve step).
     * In this implementation the transition is instantaneous; subclasses
     * may override to add async fade support.
     */
    bool IsTransitioning() const { return m_transitioning; }

    // ── Callbacks ─────────────────────────────────────────────────────

    void SetTransitionCallback(TransitionCallback cb) {
        m_transitionCallback = std::move(cb);
    }

    // ── Transition count (for testing) ────────────────────────────────

    int TransitionCount() const { return m_transitionCount; }

private:
    IPlayerMode* FindMode(PlayerMode mode) const;

    PlayerMode  m_currentMode    = PlayerMode::FLIGHT;
    bool        m_initialized    = false;
    bool        m_transitioning  = false;
    int         m_transitionCount = 0;

    // Slot array indexed by PlayerMode (FPS=0, FLIGHT=1, FLEET_COMMAND=2)
    static constexpr int MODE_COUNT = 3;
    std::unique_ptr<IPlayerMode> m_modes[MODE_COUNT];

    TransitionCallback m_transitionCallback;
};

} // namespace atlas::gameplay
