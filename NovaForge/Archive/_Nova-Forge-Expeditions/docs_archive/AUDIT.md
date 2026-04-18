# Nova-Forge: Expeditions — Three-Way Repository Audit

> **Purpose**: This document is the canonical record of every decision made when merging
> `AtlasForge`, `NovaForge`, and `Atlas-NovaForge` into this single unified repository.
> Use it as the authoritative reference when resolving any future "where did this come from?" questions.

---

## Source Repositories

| Repo | URL | Role | Files |
|------|-----|------|-------|
| **Atlas-NovaForge** | `shifty81/Atlas-NovaForge` | Combined engine + game (canonical base) | 3,090 |
| **NovaForge** | `shifty81/NovaForge` | Original standalone game project | 2,566 |
| **AtlasForge** | `shifty81/AtlasForge` | Original standalone engine project | 898 |
| **Nova-Forge-Expeditions** | `shifty81/Nova-Forge-Expeditions` | **This repo — final unified product** | ~5,900 |

---

## Merge Strategy: Priority Order

```
1. Atlas-NovaForge  (PRIMARY — most complete, most recent merging of the other two)
2. NovaForge        (adds: more game systems, full test suite, cpp_common, handlers, docs/archive)
3. AtlasForge       (adds: engine/procedural, engine/render, engine/tools, engine/ui/atlas, Blender addon)
```

When the same file exists in multiple repos, **Atlas-NovaForge wins** unless explicitly noted below.
All decisions are additive — nothing is deleted from any source repo.

---

## Directory-by-Directory Decisions

### `engine/` — Core Engine Library

**Primary source**: Atlas-NovaForge  
**Additional content from AtlasForge** (87 files not in Atlas-NovaForge):

| AtlasForge Addition | Path | What It Is |
|--------------------|------|-----------|
| PCG Framework | `engine/procedural/` | `PCGManager`, `ConstraintSolver`, `HullMeshGenerator`, `BuildQueue`, `PCGVerify`, `DeterministicRNG`, `PCGDomain`, `InteriorNode`, `PlanetaryBase`, `ModuleTier`, `ModuleScaling` — standalone procedural generation framework |
| Rendering Pipeline | `engine/render/` | `GBuffer`, `PBRMaterial`, `PostProcess`, `ShadowMap`, `InstancedRenderer`, `SpatialHash` — full deferred rendering stack |
| ToolingLayer (original) | `engine/tools/` | `ITool`, `EditorEventBus`, `UndoableCommandBus`, `EditorToolRegistry`, `AnimationPreviewTool`, `BatchTransformTool`, `EntityInspectorTool`, `EnvironmentControlTool`, `LayerTagSystem`, `LiveEditTool`, `MaterialOverrideTool`, `NPCSpawnerTool`, `PCGSnapshotTool`, `PrefabPlacementTool`, `SceneBookmarkManager`, `SimulationStepTool` |
| AtlasUI Widget Set | `engine/ui/atlas/` | `AtlasUI`, `AtlasUIPanels`, `AtlasUITheme`, `AtlasUIWidgets` — Atlas-branded UI widget library |
| Asset Dependency Graph | `engine/assets/DependencyGraph.h/.cpp` | Asset dependency tracking and graph traversal |

**Why Atlas-NovaForge's `engine/` is canonical**:  
Atlas-NovaForge adds `engine/bootstrap/` (unified `RuntimeBootstrap` across all modes) which AtlasForge lacks.
Atlas-NovaForge's engine has been actively refactored with 14 more subdirectories than the original AtlasForge.

**Overlap note**: `engine/render/` in AtlasForge contains `GBuffer`, `ShadowMap`, `PBRMaterial`, etc.
Atlas-NovaForge's `engine/render/` contains OpenGL-specific renderer files. Both are kept — they target
different abstraction levels (AtlasForge's are standalone implementations; ANF's are integrated).

---

### `cpp_server/` — Nova Forge Dedicated Server

**Primary source**: Atlas-NovaForge (has decomposed `GameSession`)  
**Major additions from NovaForge**:

| Area | ANF | NovaForge | Decision |
|------|-----|-----------|---------|
| `cpp_server/src/systems/` | 192 files | **449 files** | **Use NovaForge** — 257 additional game systems |
| `cpp_server/tests/` | 25 files | **527 files** | **Use NovaForge** — full per-domain test suite |
| `cpp_server/include/` | 273 files | **541 files** | **Use NovaForge** — 268 additional headers |
| `cpp_server/src/handlers/` | ❌ absent | 6 files | **Use NovaForge** — combat/mission/movement/scanner/station/chat handlers |
| `GameSession` decomposition | ✅ 9 domain files | ❌ monolith | **Keep ANF** — `game_session_combat.cpp`, `_movement`, `_missions`, `_scanning`, `_stations` |

**Why this matters**: NovaForge's `cpp_server` has the *original* full game server codebase —
449 game systems, 527 test files, and 541 include headers vs Atlas-NovaForge's trimmed-down 192/25/273.
The merge gives us all systems + the cleaner ANF `GameSession` decomposition simultaneously.

**Game systems present in NovaForge but not Atlas-NovaForge** (sample):
`abyssal_escalation_system`, `abyssal_filament_system`, `achievement_system`,
`aegis_spawn_system`, `agent_mission_system`, `aggression_switching_system`,
`ai_economic_actor_system`, `alliance_management_system`, `ambient_event_system`,
`ancient_ai_remnant_system`, `ancient_module_discovery_system` ... (257 total)

---

### `cpp_client/` — Nova Forge Game Client

**Primary source**: Atlas-NovaForge (311 files vs NovaForge's 306 — marginally more complete)  
No unique files in NovaForge that aren't in Atlas-NovaForge for `cpp_client/`.

---

### `cpp_common/` — Shared Common Library

**Source**: NovaForge **only** — not present in Atlas-NovaForge or AtlasForge.  

| File | Contents |
|------|---------|
| `cpp_common/CMakeLists.txt` | CMake build for shared library |
| `cpp_common/include/novaforge/chat/chat_types.h` | Chat message type definitions for client/server |

**Action needed**: Wire `cpp_common` into the root `CMakeLists.txt` as a static library target.

---

### `client/` and `server/` — Atlas Engine Client/Server Wrappers

**Source**: Atlas-NovaForge **only** (AtlasForge has empty stubs; NovaForge doesn't have these).  
These are the newer unified Atlas engine client/server entry points that wrap the underlying
`cpp_client`/`cpp_server` game code via the `RuntimeBootstrap` system.

---

### `editor/` — Atlas Editor

**Primary source**: Atlas-NovaForge (all 26 ToolingLayer tools, complete migration).  
AtlasForge has an older `editor/` — Atlas-NovaForge's is a strict superset with all tools migrated.

---

### `docs/` — Documentation

**Primary source**: Atlas-NovaForge (275 files)  
**Additional from NovaForge** (unique not in ANF):

| Category | Path | What It Contains |
|----------|------|----------------|
| Session history | `docs/archive/sessions/` | 80+ session summaries from original NovaForge development |
| Phase history | `docs/archive/phases/` | Phase 4–7 design and implementation notes |
| Getting started | `docs/archive/getting-started/` | Original quickstart guides |
| Design specs | `docs/design/` | Chat system spec, editor tool layer design, server GUI design, test solar system |
| Feature plans | `docs/features/` | Creator/edit mode features |
| Editor tools | `docs/EDITOR_TOOLS.md` | Original editor tool documentation |
| Architecture | `docs/architecture/` | Directory structure proposals |

**Additional from AtlasForge** (unique not in ANF):

| File | What It Contains |
|------|----------------|
| `docs/BLENDER_ADDON_DESIGN.md` | Design doc for the Atlas Blender spaceship generator addon |
| `docs/NOVAFORGE_ENGINE_FEATURE_PLAN.md` | Original AtlasForge engine feature roadmap for NovaForge integration |

**Final docs count**: 379 files (up from ANF's 275)

---

### `tools/` — Development Tools

**Primary source**: Atlas-NovaForge  
**Additional from AtlasForge**:

| Tool | Path | What It Does |
|------|------|-------------|
| Blender Addon | `tools/blender-addon/` | Blender Python addon to generate Atlas-compatible spaceship meshes procedurally |
| Ship presets | `tools/blender-addon/presets/` | `fighter.json`, `frigate.json`, `cruiser.json`, `battleship.json`, `capital.json` |
| Integration guide | `tools/blender-addon/ENGINE_INTEGRATION.md` | How to connect generated meshes to the engine pipeline |

---

### `tests/` — Engine Unit Tests

**Primary source**: Atlas-NovaForge  
**Additional from AtlasForge** (5 unique test files):

| Test File | What It Tests |
|-----------|-------------|
| `tests/test_atlas_ui.cpp` | AtlasUI widget system (`engine/ui/atlas/`) |
| `tests/test_editor_tools.cpp` | Editor tool layer (`engine/tools/`) |
| `tests/test_pcg_framework.cpp` | PCG domain/manager framework (`engine/procedural/`) |
| `tests/test_procedural_pipeline.cpp` | Full procedural pipeline (HullMeshGenerator, ConstraintSolver) |
| `tests/test_rendering_pipeline.cpp` | Rendering pipeline (GBuffer, PBR, PostProcess, ShadowMap) |

---

### `data/` — Game Data (JSON)

**Source**: Atlas-NovaForge (has the complete set including `From old repo/` historical data)  
NovaForge's `data/` is a subset of Atlas-NovaForge's.

---

### `atlas_tests/` — Atlas Engine Test Suite

**Source**: Atlas-NovaForge (51 test files — superset of NovaForge's `atlas_tests/`)

---

### `modules/` — Engine Game Modules

**Source**: Atlas-NovaForge **only** — `modules/atlas_gameplay/` (FactionSystem, CombatFramework, EconomySystem).  
NovaForge and AtlasForge have no equivalent.

---

### `projects/` — Example Game Projects

**Source**: Atlas-NovaForge **only** — `projects/eveoffline/`, `projects/arena2d/`, `projects/novaforge/`.  
AtlasForge has older stubs; Atlas-NovaForge has fully wired `IGameModule` implementations.

---

### `archive/` and `legacy/`

**Source**: Atlas-NovaForge **only** — historical planning notes and pre-merge reference material.

---

### Design Documents

| File | Source | Notes |
|------|--------|-------|
| `Chat.md` | Nova-Forge-Expeditions (this repo) | Full planning chat log — canonical |
| `ROADMAP.md` | Nova-Forge-Expeditions (this repo) | 7-phase roadmap — canonical |
| `NovaForge Design ideas.rtf` | NovaForge | Original game design ideas document |
| `README.md` | **Rewritten** for Nova-Forge: Expeditions | See below |

---

## Final Repository File Counts

| Directory | Files | Primary Source | Augmented By |
|-----------|-------|---------------|-------------|
| `engine/` | ~700 | Atlas-NovaForge | AtlasForge (procedural, render, tools, ui/atlas) |
| `cpp_server/src/systems/` | 449 | NovaForge | — |
| `cpp_server/tests/` | 548 | NovaForge | — |
| `cpp_server/include/` | 541 | NovaForge | — |
| `cpp_server/src/` (other) | ~40 | Atlas-NovaForge | NovaForge (handlers/) |
| `cpp_client/` | 311 | Atlas-NovaForge | — |
| `cpp_common/` | 2 | NovaForge | — |
| `editor/` | ~190 | Atlas-NovaForge | — |
| `docs/` | 379 | Atlas-NovaForge | NovaForge (archive, design, features) + AtlasForge (2 docs) |
| `atlas_tests/` | 51 | Atlas-NovaForge | — |
| `tests/` | ~130 | Atlas-NovaForge | AtlasForge (5 test files) |
| `tools/` | ~25 | Atlas-NovaForge | AtlasForge (blender-addon) |
| `data/` | ~400 | Atlas-NovaForge | — |
| `modules/` | ~50 | Atlas-NovaForge | — |
| `projects/` | ~30 | Atlas-NovaForge | — |
| `schemas/` | ~20 | Atlas-NovaForge | — |
| **Total** | **~5,900** | | |

---

## Known Overlaps Requiring Future Resolution

### 1. `engine/render/` Dual Implementation

**Issue**: Atlas-NovaForge's `engine/render/` has OpenGL-integrated renderer files. AtlasForge's additions (`GBuffer.h/cpp`, `PBRMaterial`, `ShadowMap`, `PostProcess`, `InstancedRenderer`, `SpatialHash`) are standalone implementations.

**Current state**: Both sets coexist in `engine/render/`.  
**Resolution needed**: Audit which files share the same purpose and either merge them or clearly namespace them (`engine/render/gl/` for OpenGL backend, `engine/render/core/` for abstract pipeline).

---

### 2. `cpp_server/src/systems/` Version Divergence

**Issue**: Atlas-NovaForge's 192 system files are the *refactored* versions (some migrated to `SingleComponentSystem<C>` templates). NovaForge's 449 files include all 257 additional systems that ANF doesn't have.

**Current state**: NovaForge's full system set is present (including the ANF-refactored systems overwritten back to pre-refactor state for the 192 that overlapped).  
**Resolution needed**: 
1. Re-apply `SingleComponentSystem<C>` template migration to all systems (ROADMAP Phase A1)
2. Migrate the 257 new NovaForge systems to templates as part of Phase A1

---

### 3. `GameSession` Architecture Split

**Issue**: Atlas-NovaForge decomposed `GameSession` into `game_session_combat.cpp`, `game_session_movement.cpp`, etc. NovaForge's `handlers/` directory (`combat_handler.cpp`, `movement_handler.cpp`, etc.) represents a parallel decomposition attempt.

**Current state**: ANF's `game_session_*.cpp` files AND NovaForge's `handlers/*.cpp` coexist.  
**Resolution needed**: These both implement ROADMAP Phase A1's `GameSession` decomposition goal. Audit both implementations and choose one canonical approach. The `handlers/` pattern aligns better with the `IMessageHandler` interface design in `SPAGHETTI_CODE_AUDIT.md`.

---

### 4. `engine/tools/` vs `editor/` Tool Duplication

**Issue**: AtlasForge's `engine/tools/` contains `NPCSpawnerTool`, `AnimationPreviewTool`, `LayerTagSystem`, `SceneBookmarkManager`, etc. Atlas-NovaForge's `editor/` already has full implementations of all these as ToolingLayer tools.

**Current state**: Both sets exist — `engine/tools/` (AtlasForge originals) and `editor/tools/` (ANF fully integrated versions).  
**Resolution needed**: `engine/tools/` should be treated as the lightweight interface layer (`ITool.h`, `UndoableCommandBus`); the full implementations should live in `editor/`. Move any unique logic from `engine/tools/*.cpp` into the corresponding `editor/` files, then reduce `engine/tools/` to headers only.

---

### 5. `cpp_common/` Not Wired into Build

**Issue**: `cpp_common/` exists but the root `CMakeLists.txt` doesn't reference it.  
**Resolution needed**: Add `add_subdirectory(cpp_common)` to root CMakeLists and link `novaforge_common` into both `cpp_client` and `cpp_server`.

**✅ Resolved**: `add_subdirectory(cpp_common)` already present in root CMakeLists.txt. `novaforge_common` is now conditionally linked into both `atlas_dedicated_server` (cpp_server) and `atlas_client` (cpp_client). cpp_common CMake updated to C++20.

---

### 6. `tests/` vs `atlas_tests/` Test Suite Split

**Issue**: Two separate test directories exist — `tests/` (legacy + AtlasForge-specific) and `atlas_tests/` (newer Atlas Engine suite). The root `CMakeLists.txt` already has conditional logic for this but it needs review for the new test files added from AtlasForge.

**Resolution needed**: Register the 5 new AtlasForge test files (`test_atlas_ui`, `test_editor_tools`, `test_pcg_framework`, `test_procedural_pipeline`, `test_rendering_pipeline`) in `tests/CMakeLists.txt`.

**✅ Resolved**: All 5 test files now registered in `tests/CMakeLists.txt`.

---

### 7. `CMakeLists.txt` Not Yet Referencing All Components

**Issue**: The current root `CMakeLists.txt` is Atlas-NovaForge's and doesn't know about:
- `cpp_common/`
- `engine/procedural/` (new PCG framework)
- `engine/render/` additions (GBuffer, PBR, etc.)
- `engine/tools/` (new tool interfaces)
- `engine/ui/atlas/` (AtlasUI widgets)
- `tools/blender-addon/` (not a CMake target, but needs documentation)

**Resolution needed**: See updated `CMakeLists.txt` in this repo. The `engine/CMakeLists.txt` also needs updating to compile the new subdirectories.

---

## What Has NOT Been Merged

The following content was explicitly excluded — it exists in source repos but should not be part of the unified codebase:

| Item | Location | Why Excluded |
|------|---------|-------------|
| `From old repo/` | Atlas-NovaForge | Historical data snapshots; already present in `data/` in better form |
| `legacy/` | Atlas-NovaForge | Pre-migration reference; not compilable code |
| `archive/` | Atlas-NovaForge | Planning history; useful reference but not code |
| `NovaForge/atlas_tests/` | NovaForge | Subset of ANF's atlas_tests — no unique files |
| `NovaForge/CMakeLists.txt` | NovaForge | Older build system — ANF's is more complete |

---

## Immediate Next Actions (from ROADMAP Phase A)

Priority order based on what the audit reveals:

1. **Wire `cpp_common` into CMakeLists** (1 day) — it's stranded right now
2. **Audit `cpp_server/src/systems/` overlap** (2 days) — 192 ANF systems were overwritten with NF versions; re-identify which ANF had refactored and restore those refactors
3. **Reconcile `GameSession` vs `handlers/`** (2 days) — choose one decomposition pattern and delete the other
4. **Reduce `engine/tools/` to headers only** (1 day) — move implementations to `editor/`
5. **Register new test files in CMakeLists** (1 day) — 5 AtlasForge test files unregistered
6. **Audit `engine/render/` dual implementations** (3 days) — namespace or merge

---

*Audit completed: March 2026*  
*Three-way merge of: AtlasForge + NovaForge + Atlas-NovaForge → Nova-Forge: Expeditions*
