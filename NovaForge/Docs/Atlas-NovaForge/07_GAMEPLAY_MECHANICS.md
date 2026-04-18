# Atlas Engine — Gameplay Mechanics

## Core Principle

Gameplay mechanics are **interpreted, not compiled**.
The editor creates mechanic assets. The client loads and runs them.

## Mechanic Representation

| Mechanic Type | Representation          |
|---------------|------------------------|
| Movement      | Component + node graph |
| Combat        | Data + state machine   |
| AI            | Behavior graph         |
| Economy       | Formula graphs         |
| UI logic      | Event graph            |
| Proc-gen      | Generator graph        |

## The Gameplay Stack

```
[ Editor Tools ]
       ↓
[ Gameplay Assets ]
       ↓
[ Runtime Interpreter (Graph VM) ]
       ↓
[ ECS + Net Sync ]
```

## ECS Binding

Mechanics bind to components, not entities:

```cpp
struct WeaponComponent {
    MechanicHandle fireLogic;
};

// Engine system
for (auto e : ECS.View<WeaponComponent>()) {
    runtime.Execute(e.fireLogic, e);
}
```

## Editor → Client Workflow

1. Open Mechanics Editor
2. Create or modify a graph
3. Bind graph to a component
4. Press **Play In Client**
5. Client hot-reloads mechanic
6. Test live
7. Adjust values
8. Repeat

No restart. No rebuild.

## Composable Game Mechanics

Each mechanic is:
- A Graph (logic)
- ECS bindings (data)
- Input mapping (controls)
- Camera behavior (view)

### Supported Mechanics

- Vehicle movement
- Strategy overview
- State transitions
- First-person movement
- Zero-gravity movement
- Interior exploration
- Environmental construction
- Combat systems

## Network Safety

Because mechanics are assets, deterministic, and graph-based:
- Run on server only
- Mirror on client
- Lockstep in P2P
- Validate outputs

## Simulation-First Rules

All gameplay systems in Atlas are simulation-only. The following rules are
enforced by the Atlas Core Contract:

### Forbidden in Gameplay Code

| Forbidden | Why |
|-----------|-----|
| `std::chrono` | Wall-clock time breaks determinism |
| `rand()` / `<random>` | Non-deterministic RNG |
| OS time APIs | Platform-dependent |
| GPU/rendering calls | Presentation is not simulation |
| File I/O | Side effects break replay |
| Unordered containers | Iteration order is non-deterministic |

### Required Practices

- Use `DeterministicRNG` for all randomness (seeded, reproducible)
- Use `TimeModel::Context().sim` for simulation time, never wall-clock
- All gameplay state mutations occur only during simulation ticks
- Gameplay systems must be registered with explicit execution order
- All gameplay state must be categorized as `StateCategory::Simulated`

### AI Randomness Sources

AI systems must use **only** `DeterministicRNG` for any random decisions.
The RNG seed is stored in the save file and replay header, ensuring that
AI behavior is reproducible across:

- Different machines (cross-platform determinism)
- Save/load boundaries
- Replay playback
- Network rollback/resimulation

See `engine/core/contract/DeterministicRNG.h` for the implementation.
