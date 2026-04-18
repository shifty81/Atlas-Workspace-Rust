# Atlas Deterministic Scripting VM

> Version: 1.0 | Status: Implemented | Last updated: 2026-02-22

---

## Overview

The Atlas Scripting VM is a self-contained, sandboxed, stack-based bytecode
virtual machine designed for deterministic game logic scripting.  Scripts
execute inside the simulation tick and are fully replay-safe.

**Core Rule**: Scripts may influence simulation only through deterministic,
auditable channels.  Scripts are deterministic function evaluators inside
Atlas' contract system — not first-class authorities.

---

## Design Constraints

| Constraint | Enforcement |
|------------|-------------|
| No JIT | VM is pure interpreter |
| No native extensions | Sandboxed API surface only |
| No wall-clock time | `std::chrono` not included |
| No OS randomness | `std::random` not included |
| No IO | No file, network, or console access |
| Budget enforcement | Configurable step limits |

---

## Architecture

```
Simulation Tick
 └─ ScriptSystem::ExecuteTick(tick, seed)
     ├─ Sets atlas_tick, atlas_seed variables
     ├─ Executes each registered script (deterministic order)
     └─ Accumulates step counts and hashes
```

### Components

| Component | File | Purpose |
|-----------|------|---------|
| `ScriptVM` | `engine/script/ScriptVM.h/.cpp` | Stack-based bytecode interpreter |
| `ScriptSystem` | `engine/script/ScriptSystem.h/.cpp` | Tick-integrated script manager |
| `ScriptSandbox` | `engine/script/ScriptSandbox.h/.cpp` | Safe built-in functions |

---

## Opcodes

| Opcode | Description |
|--------|-------------|
| `PUSH_INT` | Push integer constant onto stack |
| `PUSH_FLOAT` | Push float constant onto stack |
| `PUSH_STRING` | Push string constant onto stack |
| `ADD` | Pop two values, push sum |
| `SUB` | Pop two values, push difference |
| `MUL` | Pop two values, push product |
| `DIV` | Pop two values, push quotient (div-by-zero returns 0) |
| `MOD` | Pop two values, push remainder (div-by-zero returns 0) |
| `NEG` | Negate top of stack |
| `EQ`, `NEQ` | Equality / inequality comparison |
| `LT`, `GT`, `LTE`, `GTE` | Ordered comparison |
| `AND`, `OR`, `NOT` | Logical operations |
| `LOAD_VAR` | Push named variable onto stack |
| `STORE_VAR` | Pop value, store into named variable |
| `CALL` | Call registered native function (next instruction = arg count) |
| `JMP` | Unconditional jump |
| `JMP_IF` | Conditional jump (pop, jump if truthy) |
| `RET` | Return top of stack |
| `HALT` | Stop execution |

---

## Script Values

Scripts operate on tagged values (`ScriptValue`):

| Type | Storage |
|------|---------|
| `None` | No value |
| `Int` | `int64_t` |
| `Float` | `double` |
| `Bool` | stored as `int64_t` (0/1) |
| `String` | `std::string` |

---

## Safe Built-in Functions

Registered by `ScriptSandbox::RegisterBuiltins()`:

| Function | Description |
|----------|-------------|
| `atlas_abs` | Absolute value (int or float) |
| `atlas_min` | Minimum of two values |
| `atlas_max` | Maximum of two values |
| `atlas_clamp` | Clamp value to [min, max] |
| `atlas_floor` | Floor of float value |
| `atlas_ceil` | Ceiling of float value |
| `atlas_sqrt` | Square root |
| `atlas_strlen` | String length |

---

## Script Contract

Every script must declare its contract:

```cpp
CompiledScript script;
script.deterministicDeclared = true;  // REQUIRED
script.replaySafe = true;             // REQUIRED
script.migrationSafe = true;          // Optional
```

`ScriptSystem::ValidateContracts()` rejects scripts that do not declare
`deterministicDeclared` and `replaySafe`.

---

## Budget Enforcement

```cpp
ScriptVM vm;
vm.SetMaxSteps(10000);  // halt after 10,000 instructions
auto result = vm.Execute(script);
if (vm.WasBudgetExceeded()) {
    // script exceeded its budget — result is partial
}
```

---

## Replay Safety

Script identity is tracked via `CompiledScript::sourceHash`.  During
replay, the hash must match the recording.  Mismatches block replay
execution.

The combined hash of all script states is available via
`ScriptSystem::CombinedHash()` for inclusion in the replay hash ladder.

---

## Usage Example

```cpp
#include "script/ScriptVM.h"
#include "script/ScriptSystem.h"
#include "script/ScriptSandbox.h"

atlas::script::ScriptSystem system;
atlas::script::ScriptSandbox::RegisterBuiltins(system.GetVM());

atlas::script::CompiledScript script;
script.name = "movement";
script.deterministicDeclared = true;
script.replaySafe = true;
// ... populate script.code and script.constants ...

system.RegisterScript(script);
system.ExecuteTick(currentTick, currentSeed);
```
