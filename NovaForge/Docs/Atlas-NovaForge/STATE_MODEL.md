# Atlas State Model

> Canonical reference for world state classification, mutation ownership,
> and snapshot/rollback rules.
>
> See also: `ATLAS_CORE_CONTRACT.md`, `TIME_MODEL.md`, `SAVE_SYSTEM.md`

---

## Overview

Atlas classifies all runtime state into four categories. Only
`Simulated` state is authoritative. All other categories are
either derived, cosmetic, or debug-only.

---

## State Categories

| Category | Hashed? | Serialized? | Replicated? | Survives Rollback? |
|----------|---------|-------------|-------------|-------------------|
| **Simulated** | ✅ Yes | ✅ Yes | ✅ Yes | ✅ Yes |
| **Derived** | ❌ No | ❌ No | ❌ No | ❌ Rebuilt |
| **Presentation** | ❌ No | ❌ No | ❌ No | ❌ Rebuilt |
| **Debug** | ❌ No | ❌ No | ❌ No | ❌ Stripped |

### Simulated State

Authoritative game state. Fully deterministic, serializable, and
replicable. This is the **only** state that participates in hashing,
save/load, and replay verification.

**Examples:** Entity transforms, physics state, gameplay variables,
AI memory, timers, cooldowns.

### Derived State

Computed from Simulated State each tick. Never serialized or hashed.
Rebuilt automatically on load, rollback, or reconnect.

**Examples:** Spatial query caches, pathfinding results, visibility
graphs, aggregated statistics.

### Presentation State

Rendering-only state for smooth visuals. Never enters simulation.

**Examples:** Interpolated positions, camera smoothing, particle
system state, VFX parameters.

### Debug State

Editor and debug overlays. Stripped in release builds.

**Examples:** Gizmo positions, inspector selections, profiler data.

---

## State Block Registration

Systems register their state blocks with the `WorldState` manager,
declaring name, category, and type tag:

```cpp
worldState.RegisterBlock("physics", StateCategory::Simulated, 0x01, 1024);
worldState.RegisterBlock("pathcache", StateCategory::Derived, 0x02, 512);
```

This allows the engine to:
- Hash only `Simulated` blocks.
- Skip `Derived` blocks during serialization.
- Rebuild `Derived` blocks after rollback.

---

## Mutation Rules

1. Only simulation systems may mutate `Simulated` state.
2. `Derived` state is rebuilt, never mutated directly.
3. State mutation occurs **only at tick boundaries** via `ATLAS_SIM_MUTATION_GUARD()`.
4. No mid-tick mutation. Ever.
5. Systems own their state domains and must not mutate another system's state.

---

## Snapshot / Rollback

### WorldSnapshot

```cpp
struct WorldSnapshot {
    uint64_t tick;           // Tick at which snapshot was taken
    uint64_t stateHash;      // Deterministic hash of all simulated state
    vector<uint8_t> ecsData; // Serialized ECS world
    vector<uint8_t> auxData; // Additional simulated state
};
```

### Snapshot Rules

- The server takes a snapshot every tick for rollback support.
- Snapshot history depth is configurable (`WorldState::SetMaxSnapshots()`).
- On rollback, the engine restores `Simulated` state from the snapshot
  and rebuilds all `Derived` state via the registered callback.

### Rollback Flow

```
1. Detect desync (hash mismatch)
2. Find last known-good snapshot
3. Restore ECS state from snapshot
4. Rebuild Derived state
5. Replay inputs from snapshot tick to current tick
6. Verify hash matches
```

---

## Files

| File | Purpose |
|------|---------|
| `engine/sim/WorldState.h` | Header with `StateCategory`, `WorldSnapshot`, `WorldState` |
| `engine/sim/WorldState.cpp` | Implementation |
| `tests/test_world_state.cpp` | Unit tests (9 tests) |
| `tests/test_engine_integration.cpp` | Engine integration tests |
