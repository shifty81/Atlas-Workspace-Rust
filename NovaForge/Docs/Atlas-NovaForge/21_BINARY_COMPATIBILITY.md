# Atlas Binary Compatibility Layer

> Version: 1.0 | Status: Implemented | Last updated: 2026-02-22

---

## Overview

The Binary Compatibility Layer enables old Atlas projects to run on newer
engine versions without recompilation.  It uses versioned ABI capsules
that expose a frozen C function table, mapping old calling conventions
to current engine internals.

**Core Rule**: Binary compatibility is opt-in, scoped, and versioned.
Atlas does not promise ABI stability forever — it provides explicit
compatibility capsules.

---

## Architecture

```
Atlas Engine
 ├─ ABI v1 capsule (frozen)
 ├─ ABI v2 capsule (frozen)
 └─ Native (current)

Project Load Path:
  Load Project → Read ABI target → Find compatible capsule
  → Bind function table → Run project binary unchanged
```

### Components

| Component | File | Purpose |
|-----------|------|---------|
| `ABIVersion` | `engine/abi/ABICapsule.h` | Version identifier with compatibility checks |
| `ABIFunctionTable` | `engine/abi/ABICapsule.h` | Frozen C ABI function pointer table |
| `ABICapsule` | `engine/abi/ABICapsule.h/.cpp` | Versioned, sealable function table wrapper |
| `ABIRegistry` | `engine/abi/ABIRegistry.h/.cpp` | Capsule discovery, project binding |

---

## ABI Version Scheme

Versions follow `atlas_abi_v{major}_{minor}` format:

- **Same major** = compatible (minor changes only add, never remove)
- **Different major** = incompatible (requires project rebuild or migration)

```cpp
ABIVersion v1{1, 0};
ABIVersion v2{1, 2};
assert(v1.IsCompatibleWith(v2));  // same major → compatible
```

---

## Function Table

The `ABIFunctionTable` exposes 8 simulation-safe entry points:

| Function | Signature | Purpose |
|----------|-----------|---------|
| `RegisterSystem` | `bool(const char*)` | Register a named system |
| `StepSimulation` | `void()` | Advance one simulation tick |
| `GetCurrentTick` | `uint64_t()` | Query current tick number |
| `GetComponent` | `bool(entity, type, out, size)` | Read component data |
| `SetComponent` | `bool(entity, type, data, size)` | Write component data |
| `SubmitInput` | `void(data, size)` | Submit player input |
| `EmitEvent` | `void(type, data, size)` | Emit a simulation event |
| `GetWorldHash` | `uint64_t()` | Query world state hash |

---

## What Is Binary Compatible

| Area | Compatible? |
|------|-------------|
| Simulation entry points | ✅ Yes |
| Component registration | ✅ Yes |
| System lifecycle hooks | ✅ Yes |
| Script integration | ✅ Yes |
| Replay execution | ✅ Yes |
| Editor UI code | ❌ No (rebuilt) |
| Rendering backends | ❌ No (rebuilt) |
| Asset pipelines | ❌ No (rebuilt) |

---

## Capsule Lifecycle

```cpp
#include "abi/ABICapsule.h"
#include "abi/ABIRegistry.h"

// 1. Create and configure a capsule
auto capsule = std::make_shared<atlas::abi::ABICapsule>(
    ABIVersion{1, 0}, "Atlas ABI v1.0 — initial release"
);

// 2. Bind current engine implementations
capsule->Functions().RegisterSystem = [](const char* name) { ... };
capsule->Functions().StepSimulation = []() { ... };
// ... bind all 8 functions ...

// 3. Seal — no further modifications
capsule->Seal();

// 4. Register in the global registry
atlas::abi::ABIRegistry registry;
registry.RegisterCapsule(capsule);

// 5. Bind a project
atlas::abi::ProjectABITarget target;
target.projectName = "EveOffline";
target.targetABI = ABIVersion{1, 0};
registry.BindProject(target);
```

---

## Project Targeting

Projects declare their ABI target in the `.atlas` manifest:

```json
{
  "name": "EveOffline",
  "binary_abi": "atlas_abi_v1_0",
  "determinism_profile": "fixed32_v1"
}
```

The engine reads `binary_abi`, parses it via `ABIVersion::FromString()`,
and uses `ABIRegistry::FindCompatible()` to locate the best matching
capsule.

---

## Compatibility Resolution

When a project requests `atlas_abi_v1_2`:

1. Registry searches for capsules with major version 1
2. Finds all with minor version ≤ 2
3. Selects the highest minor version available
4. Binds the capsule to the project

If no compatible capsule is found, `BindProject()` returns `false` and
the project must be rebuilt against the current ABI.

---

## Replay Compatibility

Old replays recorded with ABI v1 capsules remain valid:

- Replay header stores ABI version
- Engine loads matching capsule before replay
- Simulation runs through the frozen function table
- Hash verification ensures bit-identical execution
