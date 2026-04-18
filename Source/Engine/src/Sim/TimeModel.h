#pragma once
// ============================================================
// Atlas Time Model — Deterministic Time Management
// ============================================================
//
// Defines the canonical time layers used throughout Atlas:
//
//   SimulationTime — Fixed-step tick counter. This is the
//                    authoritative clock for all game logic.
//                    Monotonically increasing, never skips.
//
//   WorldTime      — In-universe elapsed time. May be dilated
//                    (slow-mo, fast-forward) or paused without
//                    affecting simulation ticks.
//
//   PresentationTime — Wall-clock-relative time used only for
//                      rendering interpolation and UI animation.
//                      Never enters simulation code.
//
// See: docs/ATLAS_CORE_CONTRACT.md
//      docs/ATLAS_SIMULATION_PHILOSOPHY.md

#include <cstdint>

namespace atlas::sim {

/// Simulation time: fixed-step tick counter (deterministic).
struct SimulationTime {
    uint64_t tick = 0;           ///< Current tick number (monotonic).
    float    fixedDeltaTime = 0; ///< Seconds per tick (1 / tickRate).
    uint32_t tickRate = 30;      ///< Ticks per second.

    /// Total elapsed simulation seconds (tick * fixedDeltaTime).
    double ElapsedSeconds() const {
        return static_cast<double>(tick) * static_cast<double>(fixedDeltaTime);
    }
};

/// World time: in-universe time with dilation support.
struct WorldTime {
    double elapsed = 0.0;        ///< Total in-universe seconds.
    float  dilation = 1.0f;      ///< Time scale (0 = paused, 1 = normal, 2 = 2x).
    bool   paused = false;       ///< Explicit pause flag.

    /// Advance world time by one simulation tick.
    void Advance(float fixedDeltaTime) {
        if (!paused && dilation > 0.0f) {
            elapsed += static_cast<double>(fixedDeltaTime) * static_cast<double>(dilation);
        }
    }
};

/// Presentation time: wall-clock interpolation for rendering only.
/// This struct must NEVER be read inside simulation code.
struct PresentationTime {
    double wallElapsed = 0.0;    ///< Wall-clock seconds since start.
    float  interpAlpha = 0.0f;   ///< Interpolation fraction [0,1] between ticks.
};

/// Unified time context passed to systems each tick.
struct TimeContext {
    SimulationTime   sim;
    WorldTime        world;
    PresentationTime presentation;  ///< Only valid in rendering context.
};

/// Manages the three time layers and their advancement.
class TimeModel {
public:
    /// Configure the tick rate. Must be called before first Advance().
    void SetTickRate(uint32_t hz);
    uint32_t TickRate() const;

    /// Advance simulation and world time by one tick.
    void AdvanceTick();

    /// Set world time dilation factor. 0 = paused, 1 = normal.
    void SetWorldDilation(float dilation);
    float WorldDilation() const;

    /// Pause / unpause world time (simulation ticks still advance).
    void SetWorldPaused(bool paused);
    bool IsWorldPaused() const;

    /// Update presentation interpolation alpha (call from render thread).
    void SetPresentationAlpha(float alpha);

    /// Update wall-clock elapsed (call from render thread).
    void SetWallElapsed(double elapsed);

    /// Get the current time context snapshot.
    const TimeContext& Context() const;

    /// Set the tick counter directly (used for save/load only).
    void SetTick(uint64_t tick);

    /// Reset all time state to zero.
    void Reset();

private:
    TimeContext m_ctx;
};

}  // namespace atlas::sim
