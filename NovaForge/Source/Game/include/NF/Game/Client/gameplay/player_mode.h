#pragma once
/**
 * @file player_mode.h
 * @brief IPlayerMode interface and PlayerMode enum for tri-modal gameplay.
 *
 * Nova Forge supports three hard-boundary player modes:
 *   FPS          — boots-on-deck, EVA, boarding
 *   FLIGHT       — direct ship piloting
 *   FLEET_COMMAND — RTS-style intent-based fleet orders (time scale 0.1–0.25x)
 *
 * Each mode owns its input map, camera, and UI stack.  Transitions are
 * always routed through PlayerModeController and follow the 6-step protocol:
 *   1. Fade / dissolve control
 *   2. Transfer authority
 *   3. Swap camera
 *   4. Swap input map
 *   5. Swap UI stack
 *   6. Restore context on exit
 */

#include <string>

namespace atlas::gameplay {

// ── Player mode enumeration ──────────────────────────────────────────────────

enum class PlayerMode {
    FPS,            ///< First-person boots-on-deck
    FLIGHT,         ///< Ship piloting
    FLEET_COMMAND   ///< RTS tactical overlay (0.1–0.25× time scale)
};

inline const char* PlayerModeName(PlayerMode mode) {
    switch (mode) {
        case PlayerMode::FPS:           return "FPS";
        case PlayerMode::FLIGHT:        return "Flight";
        case PlayerMode::FLEET_COMMAND: return "Fleet Command";
    }
    return "Unknown";
}

// ── Mode ownership flags ─────────────────────────────────────────────────────

struct ModeOwnership {
    bool ownsInput       = true;
    bool ownsCamera      = true;
    bool ownsUIStack     = true;
    float timeScale      = 1.0f;   ///< 0.1–0.25 in Fleet Command
    bool playerBodyPhysical = true; ///< false in Flight / Fleet Command
};

inline ModeOwnership DefaultOwnership(PlayerMode mode) {
    switch (mode) {
        case PlayerMode::FPS:
            return { true, true, true, 1.0f, true };
        case PlayerMode::FLIGHT:
            return { true, true, true, 1.0f, false };
        case PlayerMode::FLEET_COMMAND:
            return { true, true, true, 0.15f, false };
    }
    return {};
}

// ── IPlayerMode interface ────────────────────────────────────────────────────

/**
 * Base interface for a concrete player mode (FPS, Flight, Fleet Command).
 *
 * The PlayerModeController owns the lifecycle; concrete modes simply
 * implement OnEnter / OnExit / Tick and report their ownership mask.
 */
class IPlayerMode {
public:
    virtual ~IPlayerMode() = default;

    /** Called once when this mode becomes active. */
    virtual void OnEnter() = 0;

    /** Called once when this mode is being replaced by another. */
    virtual void OnExit() = 0;

    /**
     * Per-frame update while this mode is active.
     * @param deltaTime Seconds since last tick (may be scaled by timeScale).
     */
    virtual void Tick(float deltaTime) = 0;

    /** Identifier for logging / debug. */
    virtual PlayerMode Mode() const = 0;

    /** Ownership flags for this mode. */
    virtual ModeOwnership Ownership() const = 0;

    /** Human-readable name. */
    virtual const char* Name() const { return PlayerModeName(Mode()); }
};

} // namespace atlas::gameplay
