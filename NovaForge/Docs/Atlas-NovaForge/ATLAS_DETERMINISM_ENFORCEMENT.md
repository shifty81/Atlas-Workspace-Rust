# Atlas Determinism Enforcement Layer

**The Atlas Core Contract is not aspirational — it is enforced at three levels:
compile-time, runtime, and CI. This document specifies the enforcement mechanisms.**

---

## 1. Overview

Determinism in Atlas is not a guideline. It is a structural guarantee, enforced
automatically at every stage of the development and deployment pipeline.

- **Compile-time**: forbidden APIs are caught before code ships
- **Runtime**: state mutations are guarded and hashed
- **CI**: replay baselines and contract scans block non-deterministic changes

---

## 2. Compile-Time Enforcement

### 2.1 Hard Bans

The following are banned in simulation translation units:

| Banned API / Pattern | Reason |
|----------------------|--------|
| `std::chrono` | Non-deterministic time |
| `rand()` | Non-deterministic randomness |
| `time()` | Wall-clock time |
| `<random>` | OS-dependent randomness |
| Platform-specific FP intrinsics | Cross-platform divergence |
| `unordered_map` / `unordered_set` | Non-deterministic iteration order (unless wrapped with deterministic hash) |

### 2.2 Required

- Fixed-point or deterministic FP wrappers where needed
- Explicit RNG streams (`DeterministicRNG`)
- Deterministic allocators (optional strict mode)

### 2.3 Enforcement Headers

- **`engine/core/contract/AtlasContract.h`** — included by every simulation TU
  - Build mode defines (`ATLAS_DETERMINISM_STRICT`)
  - Platform detection
  - Forbidden API macro overrides in strict mode
  - Contract attribute markers (`ATLAS_SIMULATION_ONLY`, `ATLAS_PRESENTATION_ONLY`, `ATLAS_EDITOR_ONLY`)
  - Static assertions for type sizes
  - Forbidden include detection

- **`engine/core/contract/DeterministicRNG.h`** — xorshift-based deterministic RNG
  - Seeded, reproducible
  - No OS dependencies

- **`engine/core/contract/SimulationGuard.h`** — tick boundary enforcement
  - `ATLAS_SIM_TICK_BEGIN()` / `ATLAS_SIM_TICK_END()` markers
  - `ATLAS_SIM_MUTATION_GUARD()` — traps if mutation occurs outside active tick

---

## 3. Runtime Enforcement

### 3.1 Assertions

- Tick order verification (systems execute in registered order)
- Job execution order hashing
- State mutation ownership checks (only owning system can mutate)

### 3.2 Hashing

- World state hash every N ticks
- Cross-machine comparison support
- Replay divergence detection on hash mismatch

### 3.3 Simulation Guard

- Global tick-active flag prevents state mutation outside tick boundaries
- Violations cause immediate trap (not a warning, not a log — a crash)

---

## 4. CI Enforcement

CI must fail if:

- Golden replay hashes differ from stored baseline
- Replay diverges when run on another platform
- Determinism violations are detected in build logs
- State schemas change without corresponding migration
- `determinism.json` changes without explicit version bump

### 4.1 Contract Bot (`tools/contract_scan.py`)

- Scans all simulation source files for forbidden API usage
- Reports specific file and violation
- Comments on PRs with violation details
- References `ATLAS_CORE_CONTRACT.md`

---

## 5. Namespace & Dependency Enforcement

### 5.1 Layer Rules (enforced in CMake)

```
core/          → no dependencies
simulation/    → depends only on core
networking/    → reads simulation, never mutates
assets/        → immutable definitions
replay/        → input streams + hashes
editor/        → presentation + commands
client/        → rendering, UI
```

### 5.2 Module Rules

Any new module must declare:

1. Which layer it belongs to
2. Which layers it may depend on
3. Whether it mutates World State

If it cannot answer those → PR rejected.

---

## 6. Determinism Profiles

| Profile | Enforcement | Notes |
|---------|-------------|-------|
| **Strict** | All enforcement active, zero tolerance | CI mode |
| **Development** | Warnings for violations, hot-reload enabled | Diagnostic logging |
| **Debug** | Relaxed enforcement, additional diagnostics | Performance instrumentation |
