# NovaForge C++ Coding Guidelines

> **Effective**: 2026-03-01  
> **Applies to**: All C++ code in `engine/`, `editor/`, `cpp_server/`, `cpp_client/`

These guidelines establish the baseline quality gate for all changes to
the NovaForge codebase.  Every pull request **must** conform to these
rules; reviewers should reject code that violates them.

---

## Table of Contents

1. [File Organisation](#1-file-organisation)
2. [Naming Conventions](#2-naming-conventions)
3. [Header Files](#3-header-files)
4. [Include Ordering](#4-include-ordering)
5. [Namespaces](#5-namespaces)
6. [Classes & Structs](#6-classes--structs)
7. [Error Handling & Logging](#7-error-handling--logging)
8. [Constants & Magic Numbers](#8-constants--magic-numbers)
9. [Code Duplication](#9-code-duplication)
10. [File Size Limits](#10-file-size-limits)
11. [Testing](#11-testing)
12. [Documentation](#12-documentation)
13. [Commit & PR Conventions](#13-commit--pr-conventions)

---

## 1. File Organisation

### Directory layout

```
engine/          → Atlas core engine library
  core/          → Engine, Logger, EventBus
  ecs/           → Entity-Component-System
  sim/           → AI, missions, economy systems
  net/           → Networking primitives
  graphvm/       → Graph VM compiler / executor
  world/         → World layouts (CubeSphere, voxel)
  assets/        → Asset registry, serialisation

editor/          → Atlas Editor GUI tool

cpp_server/      → Dedicated server
  include/       → Public headers
  src/           → Implementation files (mirror include/ structure)

cpp_client/      → Game client (OpenGL)
  include/       → Public headers
  src/           → Implementation files (mirror include/ structure)
```

### File naming

| Module | Convention | Example |
|--------|-----------|---------|
| `engine/`, `editor/` | PascalCase | `EventBus.h`, `ECS.cpp` |
| `cpp_server/` | snake_case | `game_session.h`, `combat_system.cpp` |
| `cpp_client/` | snake_case | `ship_physics.h`, `audio_manager.cpp` |

New files **must** follow the convention of the module they belong to.

---

## 2. Naming Conventions

### Per-module rules

| Element | `engine/` / `editor/` | `cpp_server/` | `cpp_client/` |
|---------|----------------------|--------------|---------------|
| Classes | PascalCase | PascalCase | PascalCase |
| Methods | PascalCase | snake_case | snake_case |
| Member variables | `m_camelCase` | `snake_case_` (trailing `_`) | `m_camelCase` |
| Local variables | camelCase | snake_case | snake_case |
| Constants | `UPPER_SNAKE` or `constexpr` PascalCase | `UPPER_SNAKE` | `UPPER_SNAKE` |
| Enums / enum class | PascalCase values | PascalCase values | PascalCase values |
| Namespaces | `atlas`, `atlas::sub` | `atlas`, `atlas::sub` | `atlas`, `atlas::sub` |

> **Rule**: Match the dominant style of the module you are editing.
> Do **not** introduce a second style into an existing file.

### Examples

```cpp
// cpp_server style
class CombatSystem : public System {
public:
    void update(float delta_time);
private:
    float damage_multiplier_;
};

// engine style
class EventBus {
public:
    void Subscribe(EventType type, Callback cb);
private:
    std::vector<Callback> m_listeners;
};
```

---

## 3. Header Files

### Guard style

Use `#pragma once` for all new headers.

```cpp
#pragma once

#include <string>
// ...
```

Existing `#ifndef` guards may remain; do **not** bulk-convert in unrelated PRs.

### Forward declarations

Prefer forward declarations over `#include` in headers when the full
definition is not needed:

```cpp
// Good — header only needs a pointer
namespace atlas::systems { class CombatSystem; }

// Avoid — pulls in the entire CombatSystem header
#include "systems/combat_system.h"
```

---

## 4. Include Ordering

Group includes in this order, separated by blank lines:

1. **Own header** (the `.h` that matches this `.cpp`)
2. **Project headers** (`#include "..."`)
3. **Third-party headers** (`#include <glm/glm.hpp>`, `<imgui.h>`)
4. **Standard library** (`#include <string>`, `<vector>`)

```cpp
#include "game_session.h"          // 1. own header

#include "ecs/world.h"             // 2. project
#include "systems/combat_system.h"

#include <glm/glm.hpp>             // 3. third-party

#include <string>                  // 4. std
#include <vector>
```

---

## 5. Namespaces

All NovaForge code lives under `namespace atlas`.  Sub-namespaces are
`atlas::ecs`, `atlas::net`, `atlas::sim`, `atlas::systems`,
`atlas::components`, `atlas::pcg`, `atlas::data`, `atlas::network`.

```cpp
namespace atlas {
namespace systems {

class CombatSystem { /* ... */ };

} // namespace systems
} // namespace atlas
```

Do **not** use `using namespace` in headers.  In `.cpp` files, a single
`using namespace atlas;` at file scope is acceptable.

---

## 6. Classes & Structs

- One primary class per header (small helper structs may live alongside).
- Declare members in order: `public` → `protected` → `private`.
- Mark single-argument constructors `explicit`.
- Prefer `= default` / `= delete` for special member functions.
- Use `std::unique_ptr` for owned sub-objects; raw pointers for
  non-owning references.

---

## 7. Error Handling & Logging

### Logging

| Module | Logger to use |
|--------|---------------|
| `cpp_server/` | `atlas::Logger::instance()` (singleton, thread-safe) |
| `engine/` | `atlas::Logger` static methods |
| `cpp_client/` | `std::cout` / `std::cerr` (captured by `FileLogger`) |

**Never** use raw `std::cout` / `std::cerr` in `cpp_server/` code — use the
server Logger.

### Exception handling

- Catch specific exception types, not bare `catch (...)`.
- Every `catch` block **must** log the error before handling it.
- Use `try` / `catch` only at system boundaries (I/O, network, file
  parsing).  Do not use exceptions for normal control flow.

```cpp
// Good
try {
    value = std::stof(text);
} catch (const std::exception& e) {
    LOG_WARN("Failed to parse float: {}", e.what());
    value = fallback;
}

// Bad
try { value = std::stof(text); }
catch (...) { value = fallback; }   // silent failure
```

---

## 8. Constants & Magic Numbers

Every numeric literal that represents a game-play value, distance,
timer, or UI dimension **must** be a named constant:

```cpp
// Good
static constexpr float NPC_AWARENESS_RANGE = 50000.0f;

// Bad
if (distance < 50000.0f) { /* ... */ }
```

Group related constants at the top of the file or in a shared header
(e.g. `gameplay_constants.h`).

---

## 9. Code Duplication

### DRY — Don't Repeat Yourself

If the same logic appears in three or more places, extract it into a
shared utility or helper function.

### Shared utilities

| Utility | Location |
|---------|----------|
| JSON extraction helpers | `cpp_server/include/utils/json_helpers.h` |
| Protocol message builders | `cpp_server/include/network/protocol_handler.h` |
| Component helpers | `engine/ecs/` |

Before writing a new helper, check if one already exists.

---

## 10. File Size Limits

| Metric | Soft limit | Hard limit |
|--------|-----------|-----------|
| Lines per `.cpp` file | 400 | 600 |
| Lines per `.h` file | 200 | 400 |
| Methods per class | 15 | 25 |

If a file exceeds the **soft limit**, consider splitting.  Files
exceeding the **hard limit** must be split before the PR can merge.

When splitting a `.cpp` file:

1. Create domain-specific `.cpp` files (e.g. `foo_network.cpp`,
   `foo_rendering.cpp`).
2. Each new file includes the class header plus only the headers it
   needs.
3. Update `CMakeLists.txt` to list every new source file.
4. Keep shared `static` constants / helpers in a `_utils.cpp` or an
   internal header.

---

## 11. Testing

- Every new system or significant behaviour change must include test
  assertions.
- Tests use the project's custom test framework (`ATLAS_TEST_BEGIN`,
  `ATLAS_ASSERT`, `RUN_TEST`).
- Run `make test` (or `make test-server && make test-engine`) before
  submitting a PR.
- All **3 812+** existing tests must continue to pass.

---

## 12. Documentation

### Headers

Every public class and non-trivial public method should have a
Doxygen-style comment:

```cpp
/**
 * @brief Manages target locking and tracking for entities.
 *
 * Supports multiple simultaneous locks up to the ship's max_locked_targets.
 */
class TargetingSystem : public System {
public:
    /**
     * @brief Attempt to lock the given target.
     * @param source_id  Entity initiating the lock.
     * @param target_id  Entity to lock onto.
     * @return true if the lock was started successfully.
     */
    bool startLock(const std::string& source_id, const std::string& target_id);
};
```

### Commit messages

```
<type>: <short summary>

<optional body — what and why, not how>
```

Types: `feat`, `fix`, `refactor`, `docs`, `test`, `chore`.

---

## 13. Commit & PR Conventions

- Keep PRs focused — one concern per PR.
- Link related issues in the PR description.
- CI must be green before merge.
- At least one reviewer approval required.
- Squash-merge to keep history clean.

---

## Appendix — Checklist for Reviewers

```
- [ ] File size within limits (soft < 400, hard < 600 lines)
- [ ] No new magic numbers
- [ ] Includes ordered correctly
- [ ] Naming follows module convention
- [ ] No raw std::cout/cerr in cpp_server/
- [ ] No bare catch(...)
- [ ] Tests added or existing tests still pass
- [ ] Public API documented with @brief
```
