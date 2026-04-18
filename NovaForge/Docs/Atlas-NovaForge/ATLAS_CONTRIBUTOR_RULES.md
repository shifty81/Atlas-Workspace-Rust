# Atlas Contributor Rules

## Welcome

Atlas is a deterministic simulation engine. This means every line of code must respect certain non-negotiable constraints. This document explains the rules you must follow when contributing.

**If your PR violates these rules, it will be rejected — not out of strictness, but out of necessity.**

---

## The Golden Rule

**If it can't be replayed, it can't exist.**

Every feature, every system, every line of simulation code must be 100% deterministic and replayable. No exceptions.

---

## What You Can Do

✅ **In Simulation Code:**
- Use fixed-step logic (ticks, not frames)
- Use `DeterministicRNG` for randomness (with explicit seeds)
- Read from `WorldState` (simulation state only)
- Write to `WorldState` (inside tick boundaries only)
- Use deterministic math (fixed-point or IEEE-754 with care)
- Call other simulation-only functions

✅ **In Presentation Code:**
- Render based on `WorldState` (read-only)
- Interpolate between ticks for smooth visuals
- Use wall-clock time for animations
- Handle UI input
- Play audio/visual effects

✅ **In Editor Code:**
- Inspect `WorldState` (read-only)
- Generate deterministic commands
- Visualize simulation data
- Profile and debug

---

## What You Cannot Do

❌ **Never in Simulation Code:**
- `std::chrono` or any wall-clock time
- `rand()` or `<random>` (use `DeterministicRNG` instead)
- File I/O or network I/O
- GPU operations
- Thread timing or `std::thread::sleep`
- Platform-specific math (`__m128`, SSE/AVX intrinsics)
- Hidden global state (untracked singletons)
- Frame-rate dependent logic
- Non-deterministic containers (`std::unordered_map` without ordered iteration)

❌ **Never Mutate State Outside Ticks:**
- All `WorldState` mutations must happen inside `ATLAS_SIM_TICK_BEGIN` / `ATLAS_SIM_TICK_END`
- Use `ATLAS_SIM_MUTATION_GUARD()` in mutation functions to enforce this

❌ **Never Use Third-Party UI Libraries:**
- Do not include, link, or reference Dear ImGui, Nuklear, or any external UI framework
- Atlas uses a fully custom UI stack: `UILayoutSolver`, `GUIDSLParser`, `UISceneGraph`, `WidgetDSL`, `FontBootstrap`, `TextRenderer`, `UIEventRouter`
- This applies to all code: engine, editor, client, server, tools, and tests
- The contract scanner and compile-time guards enforce this automatically

---

## Architecture Layers

Atlas has strict dependency rules:

```
┌─────────────────────────────────────┐
│          Editor / Tools             │  (Presentation + commands)
├─────────────────────────────────────┤
│       Client / Rendering            │  (Read-only views)
├─────────────────────────────────────┤
│    Replay / Networking              │  (Read simulation, send inputs)
├─────────────────────────────────────┤
│         Simulation                  │  (DETERMINISTIC, authoritative)
│    (ECS, Physics, AI, Gameplay)     │
├─────────────────────────────────────┤
│          Core / Assets              │  (No simulation logic)
└─────────────────────────────────────┘
```

### Dependency Rules

1. **Core** depends on nothing
2. **Simulation** depends only on **Core**
3. **Editor** never mutates **Simulation** directly (only via commands)
4. **Client/Rendering** reads **Simulation** but never writes

**If your module violates these dependencies, it will be rejected.**

---

## How to Contribute

### 1. Check the Contract

Before writing code, read:
- [`docs/ATLAS_CORE_CONTRACT.md`](ATLAS_CORE_CONTRACT.md)
- [`docs/ATLAS_DETERMINISM_ENFORCEMENT.md`](ATLAS_DETERMINISM_ENFORCEMENT.md)
- [`docs/ATLAS_LOCKDOWN_CHECKLIST.md`](ATLAS_LOCKDOWN_CHECKLIST.md)

### 2. Write Code with Guards

Use contract markers:

```cpp
#include <engine/core/contract/AtlasContract.h>
#include <engine/core/contract/SimulationGuard.h>

// In simulation code:
ATLAS_SIMULATION_ONLY
void UpdatePhysics(WorldState& state) {
    ATLAS_SIM_MUTATION_GUARD();  // Enforces tick boundary
    // ... mutation logic ...
}

// In presentation code:
ATLAS_PRESENTATION_ONLY
void RenderEntity(const Entity& e) {
    ATLAS_FORBID_IN_SIM("Rendering called in simulation");
    // ... rendering logic ...
}
```

### 3. Test Locally

Before submitting a PR:

```bash
# Build with strict determinism
./build.sh -b Release --clean

# Run contract scanner
python3 tools/contract_scan.py --path engine

# Run tests
./build.sh --test
```

### 4. Submit PR

- Use a clear, descriptive title
- Explain **why** the change is needed
- Explain **how** it maintains determinism
- Reference related issues/docs

---

## Common Mistakes

### ❌ Mistake: "I need current time for a cooldown"

**Wrong:**
```cpp
auto now = std::chrono::steady_clock::now();
if (now > lastCast + cooldown) { ... }
```

**Correct:**
```cpp
uint64_t currentTick = scheduler.CurrentTick();
if (currentTick > lastCastTick + cooldownTicks) { ... }
```

---

### ❌ Mistake: "I need random numbers"

**Wrong:**
```cpp
int damage = rand() % 10;
```

**Correct:**
```cpp
DeterministicRNG rng(entitySeed);
int damage = rng.next() % 10;
```

---

### ❌ Mistake: "I want to optimize with SIMD"

**Wrong:**
```cpp
__m128 v = _mm_load_ps(data);
```

**Correct:**
```cpp
// Use platform-agnostic math
// Or move optimization to presentation layer
```

---

### ❌ Mistake: "I need to cache expensive calculations"

**Wrong:**
```cpp
static std::map<Key, Value> cache;  // Hidden global state
```

**Correct:**
```cpp
// Store in DerivedState (rebuilt each tick/load)
worldState.RegisterBlock("PathCache", StateCategory::Derived);
```

---

### ❌ Mistake: "I'll use ImGui for quick editor UI"

**Wrong:**
```cpp
#include <imgui.h>
ImGui::Begin("My Panel");
```

**Correct:**
```cpp
// Use Atlas custom UI system
#include <engine/ui/UISceneGraph.h>
#include <engine/ui/WidgetDSL.h>
// Build UI with Atlas WidgetDSL or UISceneGraph
```

Atlas has a complete custom UI stack. Third-party UI libraries are
permanently banned — see `ATLAS_CORE_CONTRACT.md` §6.

---

## CI Will Reject Your PR If:

- ❌ Contract scanner detects forbidden APIs
- ❌ Contract scanner detects ImGui or other banned third-party UI libraries
- ❌ Golden replay hashes change unexpectedly
- ❌ Tests fail
- ❌ Determinism violations are logged
- ❌ Layer dependencies are violated

**This is not personal — it's how Atlas stays correct.**

---

## When in Doubt

1. **Ask first** — Open a discussion issue
2. **Read docs** — See the `docs/` directory
3. **Look at examples** — See `engine/sim/` for patterns
4. **Run the scanner** — `python3 tools/contract_scan.py`

---

## Final Words

Atlas is not "just another game engine."

It's a **deterministic simulation machine** with laws and teeth.

If you respect the contract, your code will be:
- ✅ Replayable
- ✅ Network-safe
- ✅ Debuggable
- ✅ Trusted

If you violate the contract, your code will be rejected.

**Thank you for respecting these rules. They exist to make Atlas unbreakable.**

---

## See Also

- [ATLAS_CORE_CONTRACT.md](ATLAS_CORE_CONTRACT.md) — The constitution
- [ATLAS_DETERMINISM_ENFORCEMENT.md](ATLAS_DETERMINISM_ENFORCEMENT.md) — How it's enforced
- [CONTRIBUTING.md](../CONTRIBUTING.md) — General contribution guidelines
- [BUILDING.md](BUILDING.md) — How to build and test
