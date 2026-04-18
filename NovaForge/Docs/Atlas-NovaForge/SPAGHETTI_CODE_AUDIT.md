# Nova Forge — Spaghetti Code Audit & Remediation Plan

**Date**: March 2, 2026
**Scope**: C++ server (`cpp_server/`), build system, and test infrastructure
**Baseline**: 5263 server test assertions passing, 164 system files, 31K-line test file

---

## Executive Summary

The codebase is **functionally excellent** — 100% test pass rate, zero security vulnerabilities, consistent component registration. However, structural issues have accumulated as the system count grew from ~30 to 164. The five issues below, addressed in order, would reduce compile times by ~50%, eliminate ~60% of boilerplate, and make the codebase significantly easier to maintain and extend.

---

## Issue 1: Monolithic Test File (31,554 lines)

**Severity**: 🔴 Critical
**File**: `cpp_server/test_systems.cpp`
**Impact**: Compilation bottleneck, impossible to run targeted tests, merge conflicts

### Problem

All 1,644 test functions and 215 `#include` directives live in a single file. Any change to any system header forces a full recompilation of 31K lines. This also makes it impossible to run tests for a single system in isolation.

### Evidence

```
31,554 lines — test_systems.cpp
   215 — #include directives
 1,644 — test functions (void test*())
 5,263 — assertions
```

Every test function recreates `ecs::World` and a system instance from scratch:

```cpp
void testCapacitorRecharge() {
    std::cout << "\n=== Capacitor Recharge ===" << std::endl;
    ecs::World world;
    systems::CapacitorSystem capSys(&world);
    auto* entity = world.createEntity("test_ship");
    // ... test logic ...
}
```

This 5-line boilerplate is repeated 1,644 times.

### Remediation Plan (Phase 1 — 2–3 weeks)

| Step | Action | Effort |
|------|--------|--------|
| 1.1 | Create `cpp_server/tests/` directory with per-system test files | 2 days |
| 1.2 | Extract a `TestFixture` base class providing `World`, `setUp()`, `tearDown()` | 1 day |
| 1.3 | Move each system's tests into `tests/test_{system_name}.cpp` | 1 week |
| 1.4 | Update CMakeLists.txt to build individual test executables or a CTest suite | 2 days |
| 1.5 | Add CTest integration so `ctest -R capacitor` runs only capacitor tests | 1 day |
| 1.6 | Delete monolithic `test_systems.cpp` once all tests migrated and passing | 1 day |

**Expected outcome**: 164 test files averaging ~190 lines each. Incremental compilation drops from minutes to seconds.

---

## Issue 2: System Boilerplate Duplication

**Severity**: 🟠 High
**Files**: All 164 files in `cpp_server/src/systems/` and `cpp_server/include/systems/`
**Impact**: ~60% of system code is structural boilerplate, not unique logic

### Problem

Every system follows an identical skeleton:

**Header** (~40 lines each, 164 files):
```cpp
#ifndef NOVAFORGE_SYSTEMS_FOO_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FOO_SYSTEM_H
#include "ecs/system.h"
namespace atlas { namespace systems {
class FooSystem : public ecs::System {
public:
    explicit FooSystem(ecs::World* world);
    ~FooSystem() override = default;
    void update(float delta_time) override;
    std::string getName() const override { return "FooSystem"; }
    // API methods...
};
}}
#endif
```

**Source** (~20 lines of boilerplate per file):
```cpp
#include "systems/foo_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
namespace atlas { namespace systems {
FooSystem::FooSystem(ecs::World* world) : System(world) {}
void FooSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::Foo>();
    for (auto* entity : entities) {
        auto* foo = entity->getComponent<components::Foo>();
        if (!foo || !foo->active) continue;
        // actual logic...
    }
}
}}
```

Specific duplicated patterns:
- **Recharge systems** (`capacitor_system.cpp` lines 13–26 vs `shield_recharge_system.cpp` lines 13–26): identical `min(current + rate * dt, max)` loop
- **State machine systems** (`cloaking_system.cpp` lines 25–59 vs `jump_drive_system.cpp` lines 24–57): identical `switch(phase)` + timer tick pattern
- **FPS systems** (`fps_combat_system.cpp` lines 58–86 vs `fps_interaction_system.cpp` lines 21–49): identical entity creation and query patterns

### Remediation Plan (Phase 2 — 3–4 weeks)

| Step | Action | Effort |
|------|--------|--------|
| 2.1 | Create `SingleComponentSystem<C>` template base that provides entity query + null check loop | 2 days |
| 2.2 | Create `StateMachineSystem<C, PhaseEnum>` template for phase-driven systems | 3 days |
| 2.3 | Create `RechargeSystem<C>` template for recharge-pattern systems | 1 day |
| 2.4 | Migrate 20–30 simplest systems (capacitor, shield, etc.) to template bases | 1 week |
| 2.5 | Migrate remaining systems incrementally (batches of 10–15) | 2 weeks |

**Expected outcome**: Each system's unique logic shrinks from ~150 lines to ~50 lines. Template bases absorb repeated patterns.

---

## Issue 3: GameSession God Object (9 source files)

**Severity**: 🟠 High
**Files**: `cpp_server/src/game_session*.cpp` (9 files), `cpp_server/include/game_session.h`
**Impact**: Tight coupling between networking, ECS, combat, stations, scanning, missions — all through one class

### Problem

`GameSession` is a single class split across 9 `.cpp` files to manage its size. It holds direct pointers to 8+ system classes and acts as a dispatcher for all game operations. This means:

1. Adding any new protocol message requires touching `game_session.h` (adds dependency)
2. All 9 `.cpp` files share the same header, so changing one area recompiles all
3. Forward declarations in the header list ~15 system types

From `game_session_internal.h`: Shared constants and utilities are extracted, but the class itself remains monolithic.

### Remediation Plan (Phase 3 — 1–2 weeks)

| Step | Action | Effort |
|------|--------|--------|
| 3.1 | Define `IMessageHandler` interface with `handle(MessageType, payload)` | 1 day |
| 3.2 | Create domain-specific handlers: `CombatHandler`, `StationHandler`, `MovementHandler`, `MissionHandler`, `ScannerHandler` | 3 days |
| 3.3 | `GameSession` becomes a thin router that dispatches to handlers by message type | 2 days |
| 3.4 | Each handler owns its system pointer(s), removing them from `GameSession` | 1 day |
| 3.5 | Convert forward declarations to handler includes only | 1 day |

**Expected outcome**: `GameSession` shrinks from 9 files to 1 router + 5 focused handlers. Adding new features no longer requires modifying the central class.

---

## Issue 4: CMakeLists.txt Source Duplication

**Severity**: 🟡 Medium
**File**: `cpp_server/CMakeLists.txt` (814 lines)
**Impact**: Every new system requires adding source paths in two places (server binary + test binary)

### Problem

All ~200 source files are listed explicitly twice in CMakeLists.txt — once for the server executable and once for the test executable. They must be kept in sync manually. Example:

```cmake
# Server binary (lines 36-200)
set(SERVER_SOURCES
    src/systems/capacitor_system.cpp
    src/systems/shield_recharge_system.cpp
    ...200 more...
)

# Test binary (lines 700-814) — same files minus main.cpp
set(TEST_SOURCES
    src/systems/capacitor_system.cpp
    src/systems/shield_recharge_system.cpp
    ...same 200 files again...
)
```

### Remediation Plan (Phase 4 — 3–5 days)

| Step | Action | Effort |
|------|--------|--------|
| 4.1 | Extract shared sources into a static library target `novaforge_core` | 1 day |
| 4.2 | Server binary links `novaforge_core` + `src/main.cpp` | 0.5 day |
| 4.3 | Test binary links `novaforge_core` + `test_systems.cpp` | 0.5 day |
| 4.4 | Use `file(GLOB_RECURSE)` for system sources (with explicit exclusions) or maintain one list | 1 day |
| 4.5 | Add subdirectory CMakeLists.txt files for `src/systems/`, `src/data/`, etc. | 1 day |

**Expected outcome**: New systems need to be listed in only one place. CMakeLists.txt drops from 814 to ~200 lines.

---

## Issue 5: Data Layer JSON Parsing Duplication

**Severity**: 🟡 Medium
**Files**: `cpp_server/src/data/ship_database.cpp`, `cpp_server/src/data/npc_database.cpp`, `cpp_server/src/data/wormhole_database.cpp`
**Impact**: Copy-pasted JSON brace-counting parser logic across 3+ files

### Problem

Each database class implements its own manual JSON parsing with brace counting:

```cpp
// Identical pattern in ship_database.cpp and npc_database.cpp:
int braceDepth = 0;
for (char c : content) {
    if (c == '{') braceDepth++;
    if (c == '}') braceDepth--;
    // ... brace-counting logic ...
}
```

This is a custom JSON tokenizer duplicated across files instead of using a shared utility or the project's nlohmann-json dependency.

### Remediation Plan (Phase 5 — 2–3 days)

| Step | Action | Effort |
|------|--------|--------|
| 5.1 | Create `src/data/json_parser_utils.h` with shared `parseJsonArray()` and `parseJsonObject()` helpers | 1 day |
| 5.2 | Refactor `ship_database.cpp` to use shared parser | 0.5 day |
| 5.3 | Refactor `npc_database.cpp` and `wormhole_database.cpp` to use shared parser | 1 day |

**Expected outcome**: One JSON parsing implementation, used by all database classes.

---

## Recommended Execution Order

```
Phase 4: CMakeLists.txt cleanup          (3-5 days)  ✅ DONE (CORE_SOURCES extracted)
    ↓
Phase 1: Split test file                 (2-3 weeks) ✅ DONE (23 domain files, monolith deleted)
    ↓
Phase 5: Data layer dedup               (2-3 days)  ✅ DONE (json_helpers.h)
    ↓
Phase 2: System template bases          (3-4 weeks) 🔧 STARTED (SingleComponentSystem<C> template,
                                                       CapacitorSystem + ShieldRechargeSystem migrated)
    ↓
Phase 3: GameSession decomposition      (1-2 weeks) ← Coupling fix
```

**Total estimated effort**: 8–12 weeks
**Recommended approach**: Execute one phase per sprint, validate all 5263 tests pass after each phase.

---

## Metrics to Track

| Metric | Current | Target |
|--------|---------|--------|
| Largest file (lines) | ~500 (per-domain test files) | < 500 |
| Test recompile time (single system change) | ~5s | < 5s |
| CMakeLists.txt lines | 668 | < 250 |
| Source lists to update for new system | 1 | 1 |
| Average system boilerplate (lines) | ~80 (~20 for migrated) | ~20 |
| GameSession forward declarations | 15+ | 0 |

---

## What NOT to Change

- **Component registration macro** (`COMPONENT_TYPE`): Consistently applied across all 150+ components. No action needed.
- **ECS base classes** (`entity.h`, `world.h`, `system.h`, `component.h`): Lean and well-designed. No changes.
- **Component file organization**: Already split into 15 domain-specific headers. Good structure.
- **System naming convention**: While slightly inconsistent, renaming 164 files would be high-risk for low reward. Document the convention instead.

---

*Audit completed: March 2, 2026*
*Last updated: March 3, 2026 — Phases 1/4/5 done, Phase 2 started (SingleComponentSystem template)*
*Next review: After Phase 2 (system template migration) completion*
