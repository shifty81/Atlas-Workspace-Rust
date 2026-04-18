# Atlas Core Contract

**Status: RATIFIED**
**Ratification Date: 2026-02-16**

**This document defines the non-negotiable rules of the Atlas Engine.**
**Any system, tool, mod, or feature that violates this contract is considered invalid by design.**

All code merged into the Atlas repository is bound by this contract.
Violations block merge and are treated as P0 severity bugs.

---

## 1. Core Philosophy

Atlas is a deterministic simulation engine.

- The simulation is truth
- Rendering, UI, networking, and tools are views
- Replays are first-class citizens
- Time is explicit
- State is owned

**If two machines receive the same inputs, they must produce the same world state, bit-for-bit.**

---

## 2. Simulation Invariants (Hard Rules)

The following are NEVER allowed inside simulation code:

| Forbidden | Reason |
|-----------|--------|
| Wall-clock time | Non-deterministic |
| OS randomness | Non-deterministic |
| Thread timing dependence | Non-deterministic |
| Frame-rate dependence | Non-deterministic |
| Platform-specific math | Cross-platform divergence |
| Hidden global state | Untracked mutation |
| IO (filesystem, network, GPU) | Side effects |

Allowed:

- ✅ Deterministic inputs only
- ✅ Fixed-step execution
- ✅ Explicit ordering
- ✅ Explicit ownership

Violation = engine bug. Not a feature request. Not a TODO. A bug.

---

## 3. Authority & Ownership

- Only the simulation mutates world state
- Systems own domains, not each other's data
- Cross-system communication occurs via explicit commands, deterministic events, and structured data views
- No system may mutate another system's state directly

---

## 4. Deterministic Time

Atlas defines three time domains:

| Time Type | Description |
|-----------|-------------|
| Simulation Time | Fixed ticks, authoritative |
| World Time | Derived, serializable |
| Presentation Time | Interpolated, non-authoritative |

Only Simulation Time may influence logic.

---

## 5. Replay & Network Equivalence

- If it cannot be replayed, it cannot exist
- Network input streams == replay input streams
- Saves are structured replays + diffs
- Replays must survive engine versions (with rules)

---

## 6. Forbidden Third-Party UI Libraries

Atlas uses a **custom-built UI system**. Third-party immediate-mode or
retained-mode UI libraries are permanently banned from the entire codebase.

| Banned Library | Reason |
|----------------|--------|
| Dear ImGui | Violates determinism, self-hosting, and architectural ownership |
| Nuklear | Same category — external UI dependency |
| Any third-party UI framework | Atlas owns its rendering, layout, and event pipeline end-to-end |

The custom UI stack (`UILayoutSolver`, `GUIDSLParser`, `UISceneGraph`,
`WidgetDSL`, `FontBootstrap`, `TextRenderer`, `UIEventRouter`) is the
only permitted UI implementation. This is enforced at compile time, by
the contract scanner, and in CI.

**No exceptions. No "just for prototyping." No feature flags.**

---

## 7. Enforcement

This contract is enforced via:

- Compile-time bans (forbidden includes, macro guards, ImGui include guard)
- Runtime assertions (tick boundary checks, mutation guards)
- CI determinism checks (golden replay comparison)
- CI contract scanner (forbidden API and library detection)
- Replay hash validation (cross-machine)

No exceptions. No feature flags.

---

## 7. State Categories

### Simulation State (Authoritative)

Entity transforms, physics state, gameplay variables, AI memory, timers, cooldowns.

✅ Serialized. ✅ Deterministic. ✅ Replay-critical.

### Derived State (Non-Authoritative)

Path caches, visibility graphs, render proxies, UI summaries.

❌ Not serialized. ❌ Recomputable. ❌ Never authoritative.

### Metadata

Engine version, schema versions, determinism profile, platform hash.

---

## 8. Mutation Rules

- Only simulation systems mutate Simulation State
- Derived State may only read Simulation State
- State mutation occurs only at tick boundaries
- No mid-tick mutation. Ever.

---

## 9. Save / Load / Rollback

- Saves serialize Simulation State + Metadata
- Loads reset Derived State
- Rollback restores Simulation State only
- Replays rebuild Derived State deterministically
