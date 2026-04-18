# Atlas Workspace

> A **Rust + Vulkan** game development workspace — editor, PCG world-gen pipeline, and game runtime in a single Cargo workspace.

Atlas Workspace is a **generic host environment**. Game projects such as NovaForge are developed inside it but do not define the workspace core.

> **License note**: Atlas Workspace core crates (`atlas-*`) are dual-licensed **MIT OR Apache-2.0**.
> The `novaforge-game` crate and any code ported from Nova-Forge/Veloren is licensed **GPL v3.0**.
> See [`LICENSES/`](LICENSES/) and [`CREDITS.md`](CREDITS.md) for full attribution.

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
Cargo.toml                    # Workspace manifest (23 crates)
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
  atlas-renderer/             # 🔄 Vulkan rendering backend (context/pipeline/surface done;
  |                           #      TerrainMesh upload, headless integration test added)
  atlas-editor/               # 🔄 egui editor app (panels, commands, ViewportHost,
  |                           #      NotificationCenter, LayoutPersistence, PropertyGrid)
  atlas-game/                 # 🔄 Standalone game binary (GameRunner, GameModule trait)
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
  atlas-asset/                # 🔄 Asset registry (UUID handles, load pipeline,
  |                           #      NOVAFORGE_ASSETS_DIR env-var support)
  atlas-ui/                   # 🔄 egui widget extensions (ScrollList, TreeView, LogCapture)
  novaforge-game/             # 🔄 NovaForge game module (GPL v3.0) — implements GameModule,
                              #      NovaForgeAdapter for IGameProjectAdapter
Scripts/
  build_rust.sh               # Primary Rust build script
  test_rust.sh                # Rust test runner
  check_rust.sh               # Rust check + clippy
  build_shaders.sh            # SPIR-V shader compiler
  fetch_novaforge_assets.sh   # Download Nova-Forge LFS assets → novaforge-assets/
  build_cpp_legacy.sh         # ⚠ Legacy C++ CMake build (reference only)
novaforge-assets/             # ⚠ LOCAL ONLY — not committed. Run fetch_novaforge_assets.sh
  README.md                   #   See novaforge-assets/README.md for instructions
NovaForge/                    # C++ reference implementation (blueprint for Rust port)
Source/                       # C++ workspace source — reference/archive only
CREDITS.md                    # Full upstream attribution (Veloren, Nova-Forge, contributors)
LICENSES/                     # MIT, Apache-2.0, GPL-3.0 license texts
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
| `atlas-workspace` | Binary entry-point; boots editor and runs PCG demo | ✅ Implemented |
| `atlas-renderer` | Vulkan context+surface, swapchain, GBuffer, PBR material, shadow maps, post-process, instanced renderer, spatial hash, `TerrainMesh` | 🔄 In Progress |
| `atlas-editor` | egui editor app, 5 panels, CommandStack, SelectionState, ViewportHost, NotificationCenter, LayoutPersistence, PropertyGrid | 🔄 In Progress |
| `atlas-game` | Standalone game binary, `GameRunner`, `GameModule` trait | 🔄 In Progress |
| `atlas-asset` | Asset handle + UUID registry, load-from-disk pipeline, `NOVAFORGE_ASSETS_DIR` support | 🔄 In Progress |
| `atlas-ui` | egui widget extensions: `ScrollList`, `TreeView`, `UiLogCapture` | 🔄 In Progress |
| `novaforge-game` | NovaForge game module (**GPL v3.0**) — `NovaForgeGameModule`, `NovaForgeAdapter` | 🔄 In Progress |
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
> Active Phase: **Phase 0 — Rust Foundation Completion (M15)**

- **560 passing unit tests** across the Rust workspace (M14: +ScrollList, TreeView, ViewportHost, NotificationCenter)
- Vulkan context, surface creation, and full acquire→present pipeline implemented
- `atlas-renderer`: GBuffer, PBR material, shadow maps, post-process, instanced renderer, spatial hash, `TerrainMesh` — done
- `atlas-editor`: CommandStack, SelectionState, entity commands, 5 panels, ViewportHost, NotificationCenter, LayoutPersistence, PropertyGrid — in progress
- `atlas-game`: GameRunner, NullGameModule — scaffolded
- `atlas-ui`: ScrollList (virtual scroll), TreeView, UiLogCapture — implemented
- `atlas-asset`: AssetRegistry, AssetMeta, AssetGraph + NOVAFORGE_ASSETS_DIR load path — in progress
- `novaforge-game`: NovaForgeGameModule + NovaForgeAdapter stub — in progress (**GPL v3.0**)
- **11 stub crates** awaiting implementation (Phase 0 milestones 0.2–0.8)

See [Docs/Canon/00_PROJECT_STATUS.md](Docs/Canon/00_PROJECT_STATUS.md) for full per-crate detail.

---

## Build Scripts

| Script | Purpose |
|--------|---------|
| `Scripts/build_rust.sh` | **Primary** Rust build script with logging, test, clippy, fmt |
| `Scripts/test_rust.sh` | Rust test runner — per-crate pass/fail with log output |
| `Scripts/check_rust.sh` | Fast `cargo check` + `cargo clippy` |
| `Scripts/build_shaders.sh` | SPIR-V shader compiler for `atlas-renderer` |
| `Scripts/fetch_novaforge_assets.sh` | Download Nova-Forge LFS assets → `novaforge-assets/` |
| `Scripts/build_cpp_legacy.sh` | ⚠ Legacy C++ CMake build — **reference only** |

```bash
# Recommended: use the Makefile
make build          # cargo build --workspace
make test           # cargo test --workspace
make clippy         # cargo clippy --workspace
make shaders        # compile SPIR-V shaders
make fetch-assets   # fetch Nova-Forge assets into novaforge-assets/

# Or use scripts directly
bash Scripts/build_rust.sh               # debug build
bash Scripts/build_rust.sh release       # release build
bash Scripts/build_rust.sh --test        # build + test
bash Scripts/build_rust.sh --clippy      # build + clippy
bash Scripts/fetch_novaforge_assets.sh   # fetch game assets locally
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

## NovaForge Game Project

[Nova-Forge](https://github.com/shifty81/Nova-Forge) is a Rust fork of [Veloren](https://veloren.net) — an open-world voxel RPG. It is being adapted and integrated into this workspace as the `novaforge-game` crate.

### Architecture

```
crates/novaforge-game/          # GPL v3.0 — implements atlas-game::GameModule
  src/
    lib.rs                      # Public API
    module.rs                   # NovaForgeGameModule (game loop integration)
    adapter.rs                  # NovaForgeAdapter (IGameProjectAdapter for editor)
    systems/                    # Game systems (ported from Nova-Forge / Veloren)
```

### Game Assets

Nova-Forge assets (textures, voxel models, audio, terrain maps) are **not committed** to this repository. They are stored locally in `novaforge-assets/` after running:

```bash
bash Scripts/fetch_novaforge_assets.sh
```

Set `NOVAFORGE_ASSETS_DIR=/path/to/novaforge-assets` to override the default search path.

### Licensing

`novaforge-game` inherits the **GNU General Public License v3.0** from Veloren (via Nova-Forge). Atlas Workspace core crates (`atlas-*`) remain **MIT OR Apache-2.0** and must never depend on `novaforge-game`. All communication between the editor and game logic flows through the `IGameProjectAdapter` trait boundary.

See [`CREDITS.md`](CREDITS.md) for full upstream attribution.

---

## Documentation Index

### Canon
- [Project Status](Docs/Canon/00_PROJECT_STATUS.md)
- [Locked Direction](Docs/Canon/01_LOCKED_DIRECTION.md)
- [Naming Canon](Docs/Canon/03_NAMING_CANON.md)
- [Module Boundaries](Docs/Canon/11_MODULE_BOUNDARIES.md)

### Roadmap
- [Master Roadmap (Rust Phase 0–7)](Docs/Roadmap/00_MASTER_ROADMAP.md)
- [Legacy C++ Roadmap (Phases A–I)](Docs/Roadmap/00_MASTER_ROADMAP_LEGACY.md)

### Inventory
- [Editor Tool Inventory](Docs/Inventory/EDITOR_TOOL_INVENTORY.md)
- [Panel and Service Matrix](Docs/Inventory/PANEL_AND_SERVICE_MATRIX.md)

---

## Legal & Credits

See [`CREDITS.md`](CREDITS.md) for full upstream attribution, contributor lists, and third-party acknowledgements.

### License summary

| Component | License |
|-----------|---------|
| Atlas Workspace core (`atlas-*` crates) | MIT OR Apache-2.0 |
| `novaforge-game` crate (Veloren/Nova-Forge derived) | GNU General Public License v3.0 |

License texts are in [`LICENSES/`](LICENSES/):
- [`LICENSES/MIT`](LICENSES/MIT)
- [`LICENSES/Apache-2.0`](LICENSES/Apache-2.0)
- [`LICENSES/GPL-3.0`](LICENSES/GPL-3.0)
