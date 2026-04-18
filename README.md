# Atlas Workspace

Atlas Workspace is a **Rust + Vulkan** workspace platform for building games, tools, and procedural content generation pipelines. It provides a unified host for editors, build systems, AtlasAI workflows, and project orchestration.

Atlas Workspace is a **generic host environment**. Game projects such as NovaForge are developed inside it but do not define the workspace core.

---

## Rust / Vulkan Workspace (Primary Direction)

The repository has been reworked to **Rust** with a **Vulkan** rendering backend.  All core systems have been ported to idiomatic, safe Rust; the entire procedural content generation (PCG) pipeline is driven deterministically from a single universe seed.

### Quick Start

```bash
# Build everything
cargo build --workspace

# Run all tests
cargo test --workspace

# Run the workspace entry-point
cargo run --bin atlas-workspace
```

### Workspace Layout

```
Cargo.toml                    # Workspace manifest
crates/
  atlas-core/                 # Foundation types, logging, string IDs, versioning
  atlas-math/                 # Vec2/3/4, Mat4, Quat, AABB, Ray, Transform, Colour
  atlas-ecs/                  # Entity-Component-System (EntityManager, ComponentStore,
  |                           #   SystemRegistry, SceneGraph, DeltaEditStore)
  atlas-pcg/                  # PCG world-gen (deterministic RNG, domain manager,
  |                           #   constraint solver, mesh/material graph,
  |                           #   terrain, noise, LOD, planetary base, build queue)
  atlas-renderer/             # Vulkan rendering backend (ash, gpu-allocator,
  |                           #   context, swapchain, pipeline, shader, texture)
  atlas-world/                # Universe-scale generation (galaxies, star systems,
  |                           #   planets, asteroids, asset registry, world state)
  atlas-workspace/            # Main binary entry-point
NovaForge/                    # Hosted project (C++ reference implementation)
Source/                       # C++ workspace source (legacy reference)
Tests/                        # C++ Catch2 test suite (legacy)
Docs/                         # Canon docs, roadmap, inventory, archive
```

### Crate Overview

| Crate | Purpose |
|-------|---------|
| `atlas-core` | Logging, `StringId`, `AtlasError`, version constants |
| `atlas-math` | `glam`-backed math primitives, `Aabb`, `Ray`, `Transform`, `Color` |
| `atlas-ecs` | `EntityManager`, `ComponentStore`, `SceneGraph`, `SystemRegistry`, `DeltaEditStore` |
| `atlas-pcg` | `DeterministicRng`, `PcgManager`, `PcgDomain` (16 isolated streams), `ConstraintSolver`, `MeshGraph`, `MaterialGraph`, `LodBakingGraph`, `TerrainGenerator`, `PlanetaryBase`, `BuildQueue` |
| `atlas-renderer` | Vulkan context, swapchain, shader SPIR-V loading, graphics pipeline builder, camera/viewport, GPU buffer + texture descriptors |
| `atlas-world` | `Universe`, `Galaxy`, `StarSystem`, `Planet`, `AsteroidBelt`, `AssetRegistry`, `WorldState` |
| `atlas-workspace` | Binary entry-point; boots renderer and runs PCG demo |

### PCG Architecture

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

### Key Properties

- **Deterministic**: Same universe seed produces identical output on every platform and every run.
- **Isolated**: PCG domains never interfere with each other.
- **Layered**: Delta edits are recorded on top of the seed and replayed on load.
- **Parallel**: Terrain generation uses Rayon for parallel heightmap rows.
- **Safe**: 100% safe Rust; no `unsafe` in application code.

---

## Current Status

Active phase: **Rust/Vulkan Rework - PCG World-Gen Foundation**

### Done (Rust)
- Rust Cargo workspace with 7 crates
- `atlas-core`: logger, `StringId`, error types, version
- `atlas-math`: full math primitives via `glam`, `Aabb`, `Ray`, `Transform`, `Color`
- `atlas-ecs`: `EntityManager`, `ComponentStore`, `SceneGraph`, `SystemRegistry`, `DeltaEditStore`
- `atlas-pcg`: deterministic RNG, all 16 PCG domains, `PcgManager`, constraint solver (GA), mesh graph (Primitive/Subdivide/Noise/Transform/Merge/Output), material graph, LOD baking, FBM/ridged noise, terrain heightmap, planetary base, build queue
- `atlas-renderer`: Vulkan context, swapchain, shader SPIR-V loading, graphics pipeline builder, camera/viewport, GPU buffer + texture descriptors
- `atlas-world`: universe, galaxy (spiral arms), star systems, planets (with lazy heightmaps), asteroid belts, asset registry, world state
- `atlas-workspace`: main binary demo
- 64 passing unit tests

### In Progress
- Full Vulkan surface + swapchain (requires live display server)
- SPIR-V shader compilation pipeline (glslc/naga)
- GPU-side terrain mesh upload

### Planned
- Vulkan terrain rendering pass
- egui editor integration
- Ship fitting UI (constraint solver visualisation)
- Planet surface streaming (tile-based heightmap)
- Save/load via `DeltaEditStore` + JSON

---

## Legacy C++ Reference

The original C++ workspace lives in:
- `Source/` - workspace core (ECS, renderer, UI, editor, ...)
- `NovaForge/` - hosted game project
- `Tests/` - Catch2 test suite

```bash
# Build C++ (CMake)
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug
```

---

## Documentation Index

### Canon
- [Project Status](Docs/Canon/00_PROJECT_STATUS.md)
- [Locked Direction](Docs/Canon/01_LOCKED_DIRECTION.md)
- [Naming Canon](Docs/Canon/03_NAMING_CANON.md)
- [Module Boundaries](Docs/Canon/11_MODULE_BOUNDARIES.md)

### Roadmap
- [Master Roadmap](Docs/Roadmap/00_MASTER_ROADMAP.md)

### Inventory
- [Editor Tool Inventory](Docs/Inventory/EDITOR_TOOL_INVENTORY.md)
- [Panel and Service Matrix](Docs/Inventory/PANEL_AND_SERVICE_MATRIX.md)
