# Atlas Template Contract

> Rules and guarantees for projects derived from AtlasForge.

**Status: RATIFIED**

**This document defines the contract between the Atlas template repository
and any project derived from it — whether forked, scaffolded with
`atlas init`, or maintained as a downstream consumer.**

Violations of this contract invalidate determinism guarantees, break
replay compatibility, and are treated as P0 severity bugs.

---

## 1. Purpose

This contract exists to ensure that every project derived from Atlas:

- Preserves deterministic execution semantics
- Maintains replay and network compatibility
- Respects the boundary between engine and game code
- Can safely pull template updates without breaking the game

**Who must follow it:**
- Any repository forked from AtlasForge
- Any project scaffolded with `atlas init`
- Any game team building on the Atlas template

---

## 2. Core Freeze

Engine core modules are frozen in template forks. These directories form
the determinism contract and must not be modified by game code:

| Frozen Directory | Contents |
|------------------|----------|
| `core/` | Deterministic scheduler, allocator, time model |
| `ecs/` | Entity-component-system framework |
| `sim/` | Simulation tick loop, state management |
| `graphvm/` | Graph VM interpreter, IR, compiler |

### Rules

- Only gameplay, assets, and project-specific code may be modified
- Bug fixes to frozen modules must be cherry-picked from upstream, not
  patched locally
- Local patches to frozen code invalidate all determinism guarantees
- CI enforces the freeze via read-only directory checks

---

## 3. Namespace Rules

Game code must live in its own namespace to avoid collisions with the
engine and other projects.

- Game code must use its own namespace (e.g., `mygame::`)
- The engine namespace `atlas::` must never be extended by game code
- The module interface boundary is `atlas::module::IGameModule` — game
  modules implement this interface, they do not modify it
- `atlas init` generates code in the correct namespace automatically

### Violations

```cpp
// ❌ WRONG — extending the engine namespace
namespace atlas {
    struct MyComponent { float x; };
}

// ✅ CORRECT — game namespace
namespace mygame {
    struct MyComponent { float x; };
}
```

---

## 4. Determinism Inheritance

Derived projects inherit the determinism contract defined in
[ATLAS_CORE_CONTRACT.md](ATLAS_CORE_CONTRACT.md).

- All simulation invariants apply to game code
- Replay compatibility must be maintained across template updates
- Golden replay tests must pass after every template sync
- If a template update changes determinism semantics, a version bump
  and migration guide are required

### Replay Compatibility

```
Template v1.0 replay → Game v1.x ✅ must work
Template v2.0 replay → Game v1.x ❌ may require migration
```

---

## 5. Asset Ownership

Assets are split between the template and the game project:

| Owner | Assets | Mutability |
|-------|--------|------------|
| Game project | Game assets (models, textures, audio, worlds) | Fully mutable |
| Template | Engine assets (default shaders, editor icons, core graphs) | Frozen |

- Game projects must not modify engine assets
- Asset hash stability must be maintained across template updates —
  engine asset hashes must not change within a minor version
- Game assets are exclusively owned by the game project and are never
  overwritten by template sync

---

## 6. Build System Contract

The Atlas build system provides two CMake packages that game projects
consume:

```cmake
find_package(AtlasEngine REQUIRED)
find_package(AtlasGameplay REQUIRED)
```

### Rules

- These packages must resolve correctly in any compliant project
- Game projects must not modify engine `CMakeLists.txt` files
- Platform targeting (Windows, Linux, macOS, Web) is inherited from the
  template and must not be overridden in game build files
- Game modules link against `Atlas::AtlasEngine` and
  `Atlas::AtlasGameplay` — no other engine targets are public API

---

## 7. Version Compatibility

Atlas template releases follow semantic versioning:

| Version Change | Meaning |
|----------------|---------|
| Patch (0.1.x) | Bug fixes, no determinism changes |
| Minor (0.x.0) | New features, backward-compatible determinism |
| Major (x.0.0) | Breaking changes, migration required |

- Breaking changes to determinism, replay format, or module interface
  occur only in major versions
- A migration guide is required for every major version update
- Minor versions guarantee that existing golden replays remain valid
- Game projects should pin to a minor version range

---

## 8. Enforcement

This contract is enforced through tooling, CI, and code review:

| Mechanism | What It Checks |
|-----------|----------------|
| `atlas init` | Generates a compliant project structure |
| `contract_scan.py` | Detects namespace violations and frozen-directory edits |
| CI determinism gate | Golden replay comparison after every merge |
| CI freeze check | Rejects PRs that modify frozen directories |
| Code review | Manual verification of contract compliance |

**No exceptions. No feature flags.**

---

## See Also

- [ATLAS_CORE_CONTRACT.md](ATLAS_CORE_CONTRACT.md) — Engine determinism rules
- [ATLAS_CONTRIBUTOR_RULES.md](ATLAS_CONTRIBUTOR_RULES.md) — Contribution guidelines
- [19_TEMPLATE_REPO.md](19_TEMPLATE_REPO.md) — Template repository system design
- [09_DEVELOPMENT_ROADMAP.md](09_DEVELOPMENT_ROADMAP.md) — Development roadmap
