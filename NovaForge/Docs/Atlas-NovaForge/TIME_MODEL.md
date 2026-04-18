# Atlas Time Model

> Canonical reference for the three time layers used throughout Atlas.
>
> See also: `ATLAS_CORE_CONTRACT.md`, `ATLAS_SIMULATION_PHILOSOPHY.md`

---

## Overview

Atlas defines **three distinct time domains**. Mixing them is a contract
violation and will produce determinism bugs.

| Layer | Authority | Serialized? | Hashed? | Allowed In Simulation? |
|-------|-----------|-------------|---------|----------------------|
| **SimulationTime** | Tick counter | Yes | Yes | ✅ Required |
| **WorldTime** | In-universe clock | Yes | Yes | ✅ Read-only |
| **PresentationTime** | Wall-clock | No | No | ❌ Forbidden |

---

## SimulationTime

The authoritative clock for all game logic. Monotonically increasing,
fixed-step, and fully deterministic.

```cpp
struct SimulationTime {
    uint64_t tick;           // Current tick number (monotonic)
    float    fixedDeltaTime; // Seconds per tick (1 / tickRate)
    uint32_t tickRate;       // Ticks per second (e.g. 60)
};
```

### Rules

- `tick` advances by exactly 1 each frame. It never skips.
- `fixedDeltaTime` is constant for the lifetime of a session.
- All gameplay systems receive `fixedDeltaTime`, never a variable `dt`.
- `tickRate` is set once at engine startup via `EngineConfig::tickRate`.

---

## WorldTime

In-universe elapsed time. Supports dilation (slow-mo, fast-forward)
and pause without affecting the simulation tick counter.

```cpp
struct WorldTime {
    double elapsed;    // Total in-universe seconds
    float  dilation;   // Time scale (0 = frozen, 1 = normal, 2 = 2x)
    bool   paused;     // Explicit pause flag
};
```

### Rules

- WorldTime advances each tick by `fixedDeltaTime * dilation`.
- When `paused == true`, WorldTime does not advance.
- Systems may read `WorldTime::elapsed` but must not branch on it
  in ways that break determinism. Use it for cosmetic effects
  (day/night cycle, visual timers) only.

---

## PresentationTime

Wall-clock time used exclusively for rendering interpolation and
UI animation. **This time domain must never enter simulation code.**

```cpp
struct PresentationTime {
    double wallElapsed;  // Wall-clock seconds since start
    float  interpAlpha;  // Interpolation fraction [0,1] between ticks
};
```

### Rules

- Updated from the render thread only.
- Used for smooth camera interpolation and VFX timing.
- Never serialized, never hashed, never replicated.

---

## TimeContext

All three layers are bundled into a single `TimeContext` struct that is
passed through the engine each tick:

```cpp
struct TimeContext {
    SimulationTime   sim;
    WorldTime        world;
    PresentationTime presentation;
};
```

The `TimeModel` class manages advancement and provides the current
context via `TimeModel::Context()`.

---

## Engine Integration

The `Engine` class owns a `TimeModel` instance. During each tick:

1. `TimeModel::AdvanceTick()` increments `sim.tick` and advances `world.elapsed`.
2. The ECS world update receives `sim.fixedDeltaTime`.
3. The UI update receives `sim.tick` for synchronization.
4. Rendering code (outside the tick callback) may read `presentation`.

---

## Files

| File | Purpose |
|------|---------|
| `engine/sim/TimeModel.h` | Header with all time structs and `TimeModel` class |
| `engine/sim/TimeModel.cpp` | Implementation |
| `tests/test_time_model.cpp` | Unit tests (8 tests) |
| `tests/test_engine_integration.cpp` | Engine integration tests |
