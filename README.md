# Atlas Workspace

> A **Rust + Vulkan** game development workspace — editor, PCG world-gen pipeline, and game runtime in a single Cargo workspace.

Atlas Workspace is a **generic host environment**. Game projects such as NovaForge are developed inside it but do not define the workspace core.

---

## Quick Start (Rust)

```bash
# Build everything
cargo build --workspace

# Run all tests
cargo test --workspace

# Run the workspace entry-point
cargo run --bin atlas-workspace

# Run the standalone game binary
cargo run --bin atlas-game
```

---

## Workspace Layout

```
Cargo.toml                    # Workspace manifest (22 crates)
crates/
  atlas-core/                 # ✅ Foundation types, logging, string IDs, versioning
  atlas-math/                 # ✅ Vec2/3/4, Mat4, Quat, AABB, Ray, Transform, Color
  atlas-ecs/                  # ✅ Entity-Component-System (EntityManager, ComponentStore,
  |                           #      SystemRegistry, SceneGraph, DeltaEditStore)
  atlas-pcg/                  # ✅ PCG world-gen (deterministic RNG, domain manager,
  |                           #      constraint solver, mesh/material graph,
  |                           #      terrain, noise, LOD, planetary base, build queue)
  atlas-world/                # ✅ Universe-scale generation (galaxies, star systems,
  |                           #      planets, asteroids, asset registry, world state)
  atlas-workspace/            # ✅ Binary entry-point (cargo run --bin atlas-workspace)
  atlas-renderer/             # 🔄 Vulkan rendering backend (context/pipeline done,
  |                           #      awaiting live surface)
  atlas-editor/               # 🔄 egui editor app (panels, commands, game_project_adapter
  |                           #      in progress)
  atlas-game/                 # 🔄 Standalone game binary (game loop scaffolding in progress)
  atlas-input/                # 🔲 Input system (winit events, action bindings)
  atlas-physics/              # 🔲 Physics (AABB, rigid body, ray-cast)
  atlas-sim/                  # 🔲 Simulation (fixed-timestep loop, system scheduler)
  atlas-script/               # 🔲 Scripting (Lua/Rhai integration, AutomationTask)
  atlas-animation/            # 🔲 Animation (clip, channel, keyframe, interpolation)
  atlas-ai/                   # 🔲 AtlasAI broker (request context, conversation, diff)
  atlas-sound/                # 🔲 Audio (source, mixer, SFX/music track)
  atlas-graphvm/              # 🔲 Node graph VM (pin types, execution engine)
  atlas-net/                  # 🔲 Networking (client/server, message framing)
  atlas-schema/               # 🔲 JSON schema types (property grid, validation)
  atlas-abi/                  # 🔲 Stable C ABI for plugin loading
  atlas-asset/                # 🔲 Asset registry (UUID handles, load pipeline)
  atlas-ui/                   # 🔲 egui widget extensions and panel framework
Scripts/
  build_rust.sh               # Primary Rust build script
  test_rust.sh                # Rust test runner
  check_rust.sh               # Rust check + clippy
  build_shaders.sh            # SPIR-V shader compiler
  build_cpp_legacy.sh         # ⚠ Legacy C++ CMake build (reference only)
NovaForge/                    # Hosted game project — C++ reference implementation
Source/                       # C++ workspace source — reference/archive only
Docs/                         # Canon docs, roadmap, inventory
```

---

## Crate Overview

| Crate | Purpose | Status |
|-------|---------|--------|
| `atlas-core` | Logging, `StringId`, `AtlasError`, version constants | ✅ Implemented |
| `atlas-math` | `glam`-backed math primitives, `Aabb`, `Ray`, `Transform`, `Color` | ✅ Implemented |
| `atlas-ecs` | `EntityManager`, `ComponentStore`, `SceneGraph`, `SystemRegistry`, `DeltaEditStore` | ✅ Implemented |
| `atlas-pcg` | `DeterministicRng`, `PcgManager`, 16 `PcgDomain`s, `ConstraintSolver`, `MeshGraph`, `MaterialGraph`, `TerrainGenerator`, `PlanetaryBase`, `BuildQueue` | ✅ Implemented |
| `atlas-world` | `Universe`, `Galaxy`, `StarSystem`, `Planet`, `AsteroidBelt`, `AssetRegistry`, `WorldState` | ✅ Implemented |
| `atlas-workspace` | Binary entry-point; boots renderer and runs PCG demo | ✅ Implemented |
| `atlas-renderer` | Vulkan context, swapchain, shader SPIR-V loading, graphics pipeline builder, camera/viewport, GPU buffer + texture descriptors | 🔄 In Progress |
| `atlas-editor` | egui editor app, 5 panels, CommandStack, SelectionState, game_project_adapter | 🔄 In Progress |
| `atlas-game` | Standalone game binary, `GameRunner`, `GameModule` trait | 🔄 In Progress |
| `atlas-input` | winit event → InputState, key/button/axis abstraction, action bindings | 🔲 Stub |
| `atlas-physics` | AABB collision, `RigidBody` + ECS integration, ray-cast query API | 🔲 Stub |
| `atlas-sim` | Fixed-timestep game loop, system scheduler, deterministic frame counter | 🔲 Stub |
| `atlas-script` | Lua or Rhai integration, `AutomationTask` | 🔲 Stub |
| `atlas-animation` | Clip, channel, keyframe, interpolation | 🔲 Stub |
| `atlas-ai` | `AtlasAIBroker`, `AIRequestContext`, conversation history, diff proposals | 🔲 Stub |
| `atlas-sound` | Audio source, mixer, SFX/music track | 🔲 Stub |
| `atlas-graphvm` | Node graph VM, pin types, execution engine | 🔲 Stub |
| `atlas-net` | Client/server, message framing | 🔲 Stub |
| `atlas-schema` | JSON schema definition types, property grid, validation | 🔲 Stub |
| `atlas-abi` | Stable `extern "C"` ABI for plugin loading, plugin descriptor | 🔲 Stub |
| `atlas-asset` | Asset handle + UUID registry, load-from-disk pipeline, hot-reload watcher | 🔲 Stub |
| `atlas-ui` | egui widget extensions, panel framework | 🔲 Stub |

Legend: ✅ Implemented | 🔄 In Progress | 🔲 Stub

---

## PCG Architecture

All procedural generation flows through a single seed authority: `PcgManager`. Each of the **16 isolated `PcgDomain`s** receives its own deterministic RNG stream derived from the universe seed.

```
Universe Seed (u64)
 +-- PcgManager::derive_all_domain_seeds()
     +-- Domain::Galaxy   -> galaxy layout (spiral arms, clusters)
     +-- Domain::System   -> star systems (star type, orbit, mass)
     +-- Domain::Planet   -> planet type, atmosphere, biome, heightmap
     +-- Domain::Asteroid -> asteroid belts (count, richness, metal)
     +-- Domain::Terrain  -> heightmap (FBM noise, ridged noise)
     +-- Domain::Ship     -> ship loadout (constraint solver)
     +-- Domain::Loot     -> loot tables
     +-- ... (16 domains total, all isolated)
```

---

## Key Properties

- **Deterministic**: Same universe seed produces identical output on every platform and every run.
- **Isolated**: PCG domains never interfere with each other.
- **Layered**: Delta edits are recorded on top of the seed and replayed on load.
- **Parallel**: Terrain generation uses Rayon for parallel heightmap rows.
- **Safe Rust**: 100% safe Rust; no `unsafe` in application code. Vulkan FFI is contained in `atlas-renderer`.

---

## Editor System

`atlas-editor` hosts game projects via the `IGameProjectAdapter` trait (game_project_adapter.rs). The editor provides:

- **egui-based panels**: OutlinerPanel, PropertiesPanel, ViewportPanel, AssetBrowserPanel, ConsolePanel
- **CommandStack**: undo/redo (SpawnEntityCommand, DeleteEntityCommand, RenameEntityCommand)
- **SelectionState**: multi-entity selection shared across panels
- **GameBuildSystem**: invokes `cargo build --bin atlas-game` as a subprocess
- **GameProjectAdapter trait**: the wall between editor and game logic — all communication flows through this interface

The editor shell (`WorkspaceShell`) is tool-agnostic. Game-specific logic **never** lives in workspace core.

---

## Game Hosting

`atlas-game` runs in two modes:

1. **Standalone** (`cargo run --bin atlas-game`) — boots `GameRunner`, loads `NovaForgeGameModule`, runs game loop independently.
2. **Play-In-Editor (PIE)** — `atlas-editor` instantiates `GameRunner` inside the editor process via `PIEService`, sharing the `atlas-renderer` Vulkan surface.

The boundary:
- `atlas-editor` owns: `WorkspaceShell`, panel registry, `EditorViewport`, `PIEService`
- `atlas-game` owns: `GameRunner`, `GameModule` trait, all NovaForge game systems
- `atlas-editor` **never** imports game logic directly — all communication through `IGameProjectAdapter`

---

## Current Status

> Reset Date: **2026-04-18** | Direction: **Rust + Vulkan** (primary). C++ in `Source/` is reference only.
> Active Phase: **Phase 0 — Rust Foundation Completion**

- **439 passing unit tests** across the Rust workspace
- Vulkan context and pipeline implemented; awaiting live Vulkan surface (requires display server)
- `atlas-renderer`: shader IR, spatial hash, PBR material, render config — done
- `atlas-editor`: CommandStack, SelectionState, entity commands, 5 panels — in progress
- `atlas-game`: GameRunner, NullGameModule — scaffolded
- **13 stub crates** awaiting implementation (Phase 0 milestones 0.2–0.8)

See [Docs/Canon/00_PROJECT_STATUS.md](Docs/Canon/00_PROJECT_STATUS.md) for full per-crate detail.

---

## Build Scripts

| Script | Purpose |
|--------|---------|
| `Scripts/build_rust.sh` | **Primary** Rust build script with logging, test, clippy, fmt |
| `Scripts/test_rust.sh` | Rust test runner — per-crate pass/fail with log output |
| `Scripts/check_rust.sh` | Fast `cargo check` + `cargo clippy` |
| `Scripts/build_shaders.sh` | SPIR-V shader compiler for `atlas-renderer` |
| `Scripts/build_cpp_legacy.sh` | ⚠ Legacy C++ CMake build — **reference only** |

```bash
# Recommended: use the Makefile
make build          # cargo build --workspace
make test           # cargo test --workspace
make clippy         # cargo clippy --workspace
make shaders        # compile SPIR-V shaders

# Or use scripts directly
bash Scripts/build_rust.sh               # debug build
bash Scripts/build_rust.sh release       # release build
bash Scripts/build_rust.sh --test        # build + test
bash Scripts/build_rust.sh --clippy      # build + clippy
```

---

## Legacy C++ Reference

The original C++ workspace lives in:
- `Source/` — workspace core (ECS, renderer, UI, editor, …) — **archive/reference only**
- `NovaForge/` — hosted game project — **C++ spec for Rust port**
- `Tests/` — Catch2 test suite — **legacy**

These are preserved as the **specification and blueprint** for the Rust port (Phases 2–4 of the roadmap). CMake is **not** the primary build system.

```bash
# C++ legacy build (reference only)
bash Scripts/build_cpp_legacy.sh Debug
```

---

## Documentation Index

### Canon
- [Project Status](Docs/Canon/00_PROJECT_STATUS.md)
- [Locked Direction](Docs/Canon/01_LOCKED_DIRECTION.md)
- [Naming Canon](Docs/Canon/03_NAMING_CANON.md)
- [Module Boundaries](Docs/Canon/11_MODULE_BOUNDARIES.md)

### Roadmap
- [Master Roadmap (Rust Phase 0–6)](Docs/Roadmap/00_MASTER_ROADMAP.md)
- [Legacy C++ Roadmap (Phases A–I)](Docs/Roadmap/00_MASTER_ROADMAP_LEGACY.md)

### Inventory
- [Editor Tool Inventory](Docs/Inventory/EDITOR_TOOL_INVENTORY.md)
- [Panel and Service Matrix](Docs/Inventory/PANEL_AND_SERVICE_MATRIX.md)
