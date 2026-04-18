# AtlasForge Generator Implementation Plan

Comprehensive implementation roadmap for evolving this addon
into the full **AtlasForge Generator** — a seed-deterministic, batch-ready
procedural content generation (PCG) pipeline that can be integrated into
any game project requiring procedural 3D assets.

> This plan represents the direction for the project as an engine-agnostic
> PCG tool that can be merged and adapted to specific projects.

---

## Table of Contents

1. [Vision](#1-vision)
2. [Current State](#2-current-state)
3. [Phase 1 — Full PCG Pipeline (Python + Blender)](#3-phase-1--full-pcg-pipeline-python--blender)
4. [Phase 2 — C++ Integration Layer](#4-phase-2--c-integration-layer)
5. [Phase 3 — Unified Material System](#5-phase-3--unified-material-system)
6. [Phase 4 — ECS Core & Simulation](#6-phase-4--ecs-core--simulation)
7. [Phase 5 — Multiplayer & Persistence](#7-phase-5--multiplayer--persistence)
8. [Phase 6 — Environmental Propagation & Debug Tools](#8-phase-6--environmental-propagation--debug-tools)
9. [Phase 7 — Addon Update System](#9-phase-7--addon-update-system)
10. [Directory Structure (Target)](#10-directory-structure-target)
11. [Integration Points with Projects](#11-integration-points-with-projects)

---

## 1. Vision

This addon already handles procedural ship, station, and
asteroid generation.  The next evolution turns it into the **AtlasForge
Generator** — a single-command pipeline that can produce an entire playable
universe of assets:

```
Galaxy → Systems → Planets → Stations → Ships → Characters
```

Every object is seed-deterministic and JSON-linked so any game engine can
place, simulate, and modify assets at runtime. The tool is designed to be
merged into specific projects and customized for their needs.

---

## 2. Current State

### ✅ Implemented (Blender Addon)

- 18 ship classes (Shuttle → Titan, Industrial, Mining, Exotic, etc.)
- 9-stage spine-first assembly pipeline
- Brick taxonomy (18 types, 5 categories) with Ship DNA export
- Engine archetypes, turret system, hull taper & cleanup passes
- Interior generation (cockpit, bridge, corridors, quarters, cargo, engine room)
- Module system (6 types, progressive availability)
- 4 faction styles (Solari, Veyren, Aurelian, Keldari) + game-inspired styles
- Procedural PBR textures with weathering
- Station generation (8 types, 4 faction architectures)
- Asteroid belt generation (16 ore types, 4 layouts)
- Project JSON import and OBJ export for game engines
- LOD generation, collision meshes, animation system
- Damage propagation, power flow simulation, build validator

### 🔲 Not Yet Implemented

- Galaxy / system / planet generators (Python)
- Character mesh generation
- C++ procedural hull meshing integration (pybind11)
- Blender ↔ C++ texture pipeline
- Unified material system (solid / liquid / gas)
- Terraforming engine framework
- ECS core implementation
- Multithreaded job scheduler
- Deterministic multiplayer sync
- Save / load state serialization
- Galaxy-wide environmental propagation
- Addon update system

## 3. Phase 1 — Full PCG Pipeline (Python + Blender)

**Goal:** Add Python generator modules so a single batch script can create an
entire galaxy of assets.

### New Modules

| Module | Purpose |
|--------|---------|
| `generators/galaxy_generator.py` | Seed-based galaxy with N systems |
| `generators/system_generator.py` | Stars, planets, station slots, ship slots per system |
| `generators/planet_generator.py` | Planet type, size, orbital params, biome |
| `generators/station_generator.py` | Station type, modules, hardpoints (wraps existing `station_generator.py`) |
| `generators/ship_generator_batch.py` | Headless Blender ship generation (wraps existing `ship_generator.py`) |
| `generators/character_generator.py` | Race, body type, cyber-limb procedural characters |

### Batch Script

`scripts/batch_generate.py` — single entry point:

```bash
python scripts/batch_generate.py --seed 987654 --systems 5
```

Produces:

```
build/
├── galaxy.json
├── systems/   (per-system JSON)
├── planets/   (per-planet JSON)
├── stations/  (OBJ + metadata JSON)
├── ships/     (OBJ + metadata JSON + LODs)
└── characters/ (metadata JSON)
```

### Data Flow

```
Universe Seed
    → Galaxy Generator (N systems)
        → System Generator (stars, planets, ships)
            → Planet Generator (type, size, biome, stations)
            → Station Generator → Blender headless → OBJ export
            → Ship Generator → Blender headless → OBJ export + LODs
```

### JSON Metadata Linking

Every asset references its parent via IDs:

```json
{
  "ship_id": "system_0_ship_2",
  "system_id": "system_0",
  "class": "Cruiser",
  "seed": 33333,
  "lod_files": ["..._LOD1.obj", "..._LOD2.obj"],
  "texture_params": "..._textures.json"
}
```

### Headless Blender Integration

```bash
blender --background --python scripts/generate_ship_blender.py -- \
  --ship_json build/ships/system_0_ship_2.json \
  --output_dir build/ships/
```

---

## 4. Phase 2 — C++ Integration Layer

**Goal:** Expose C++ procedural hull, texture, and style systems
to the Python pipeline via pybind11 for enhanced performance.

### Headers to Integrate

| Header | Purpose |
|--------|---------|
| `hull_mesher.h` | Bounding-box hull generation |
| `pcg_asset_style.h` | Shape deformation, control points, surface treatments |
| `procedural_texture_generator.h` | Faction colors, PBR params, hull markings, engine glow |
| `character_mesh_system.h` | Character race, body type, cyber-limb meshes |

### pybind11 Binding Module

`pcg_bindings.cpp` exposes:

- `create_hull(size, seed) → HullMesh`
- `apply_deformation(mesh, control_points) → HullMesh`
- `style_mesh(mesh, control_points) → HullMesh`
- `generate_texture(seed, faction) → TextureParams`

Compiled to `pcg_bindings.so` / `.pyd` and importable from Python.

### Pipeline Integration

```python
import pcg_bindings

hull = pcg_bindings.create_hull(size=1.0, seed=mesh_seed)
hull = pcg_bindings.apply_deformation(hull, control_points)
tex  = pcg_bindings.generate_texture(mesh_seed, "Solari")
```

The resulting mesh and texture params are then imported into Blender for final
LOD generation and OBJ/glTF export.

---

## 5. Phase 3 — Unified Material System

**Goal:** Define a single material struct that represents solids, liquids,
gases, and plasma — enabling terraforming, hazards, and environmental
simulation.

### Core Material Struct

```
Material {
  name, density, hardness, viscosity, compressibility,
  temperature, meltPoint, boilPoint, ignitionPoint,
  toxicity, reactivity, volatility, corrosionFactor,
  state: Solid | Liquid | Gas | Plasma
}
```

### Phase Logic

```
temperature < meltPoint       → Solid
meltPoint ≤ temperature < boilPoint → Liquid
temperature ≥ boilPoint       → Gas
temperature > plasmaThreshold → Plasma
```

### Planet Material Layers

```
Planet
 ├── CoreMaterial
 ├── CrustMaterial
 ├── SurfaceMaterial[]
 ├── LiquidBodies[]
 └── AtmosphereMaterial[]
```

### Blender → Engine Export

Materials annotated with custom properties (`nf_density`, `nf_meltPoint`,
`nf_toxicity`, etc.) are exported as JSON alongside meshes for engine import.

---

## 6. Phase 4 — ECS Core & Simulation

**Goal:** Architect a data-oriented ECS that powers runtime simulation in the
Atlas engine.

### Entity Model

- Entities are `uint32_t` IDs.
- Components are stored in Structure of Arrays (SoA) per archetype.
- Archetypes are indexed by `ComponentMask` bitmask.

### Core Components

| Component | Fields |
|-----------|--------|
| `CMaterial` | Unified material struct |
| `CTemperature` | `float value` |
| `CAtmosphere` | pressure, oxygen, toxicity, density |
| `CLiquidVolume` | amount, viscosity |
| `CPosition` | `Vec3 position` |
| `BrickComponent` | type, gridPos, rotation, hullWeight, hp, mass |
| `PowerComponent` | generation, consumption |
| `TurretComponent` | index, weaponType, trackingSpeed, limits, size |
| `ConnectionComponent` | parent, children, connectedToSpine |

### Key Systems

| System | Purpose |
|--------|---------|
| `PhaseTransitionSystem` | Update material state from temperature |
| `LiquidFlowSystem` | Distribute liquid volume to neighbours |
| `AtmosphereSimSystem` | Adjust pressure & composition over time |
| `PowerFlowSystem` | EVE-style capacitor charge / drain |
| `DamagePropagationSystem` | Brick HP, hull re-skin, structural cascade |
| `SalvageSystem` | Loot table roll on brick destruction |
| `BrickPlacementSystem` | Grid snap, hardpoint validation, placement |
| `HullRebuildSystem` | SDF re-mesh when bricks change |

### Multithreaded Job Scheduler

- Thread pool with lock-free work queue.
- Systems declare dependencies; independent systems run in parallel batches.
- Planet surface simulation uses chunk-based parallelism.
- Double-buffered state (Current / Next) prevents race conditions.

---

## 7. Phase 5 — Multiplayer & Persistence

### Deterministic Sync Model

- Fixed timestep (1/30 s) lockstep simulation.
- Clients sync inputs (PlayerCommand), not state.
- All randomness uses `DeterministicRNG(Hash(entityID, tick))`.
- Periodic state hash verification; mismatch triggers rewind + resim.

### Save / Load Serialization

- Binary snapshot: `SaveHeader` (version, tick, galaxySeed) + bulk SoA arrays.
- Archetype-based serialization for fast bulk read/write.
- Incremental delta saves between full snapshots.
- Seed-regeneratable: only store deltas from procedural baseline.

---

## 8. Phase 6 — Environmental Propagation & Debug Tools

### Galaxy-Wide Propagation

Each star system stores `SystemEnvironment { heat, pollution, radiation,
stability }`.  Changes propagate:

- **Upward:** planet → system (e.g., terraforming raises system stability).
- **Downward:** system → planets (e.g., war raises planetary temperature).

This creates emergent feedback loops (economy, warfare, migration).

### Heatmap Debug Overlays

| Scale | Overlay |
|-------|---------|
| Galaxy | System heat / toxicity / economic pressure / military destabilization |
| Planet | Oxygen %, pressure bands, storm density, toxic zones |
| Surface | Liquid volume, temperature, reactivity per chunk cell |

### Hazard & Survival

```
hazardScore = toxicity × 2 + temperatureDeviation × 1.5
            + pressureDelta × 1.2 + radiationLevel × 3
```

Ships gain `EnvironmentalResistance { heat, corrosion, pressure, radiation }`
enabling specialised deep-atmosphere, gas-mining, and lava-harvesting roles.

---

## 9. Phase 7 — Addon Update System

**Goal:** Keep the Blender addon evergreen as projects evolve.

| Feature | Description |
|---------|-------------|
| **Auto-import templates** | Load new JSON/YAML templates from a central directory or remote URL |
| **Versioned generators** | Each generator module carries a version; old projects specify version in metadata |
| **"Check for Updates" button** | Blender UI button fetches latest templates and merges into local addon |
| **Manual override protection** | Objects with `af_manual_override = True` are skipped by procedural regeneration |

---

## 10. Directory Structure (Target)

When fully implemented, the repository will look like:

```
AtlasForgeGenerator/
├── __init__.py                   # Blender addon entry point
├── ship_generator.py             # (existing) ship generation
├── ship_parts.py                 # (existing) ship components
├── interior_generator.py         # (existing) interior generation
├── module_system.py              # (existing) module system
├── brick_system.py               # (existing) brick taxonomy & Ship DNA
├── atlas_exporter.py             # (existing) JSON import + OBJ export
├── station_generator.py          # (existing) station generation
├── asteroid_generator.py         # (existing) asteroid belts
├── texture_generator.py          # (existing) procedural PBR materials
├── novaforge_importer.py         # (existing) project JSON import
├── render_setup.py               # (existing) catalog render setup
├── lod_generator.py              # (existing) LOD generation
├── collision_generator.py        # (existing) collision meshes
├── animation_system.py           # (existing) ship animations
├── damage_system.py              # (existing) damage propagation
├── power_system.py               # (existing) power flow simulation
├── build_validator.py            # (existing) build validation
├── generators/                   # NEW — batch PCG pipeline modules
│   ├── __init__.py
│   ├── galaxy_generator.py
│   ├── system_generator.py
│   ├── planet_generator.py
│   ├── character_generator.py
│   └── batch_pipeline.py
├── scripts/                      # NEW — CLI entry points
│   ├── batch_generate.py
│   ├── generate_ship_blender.py
│   └── generate_planet_blender.py
├── data/                         # NEW — template data
│   ├── ships/
│   ├── stations/
│   ├── planets/
│   └── characters/
├── schemas/                      # NEW — JSON schemas
│   ├── ship_dna.schema.json
│   └── galaxy.schema.json
├── README.md
├── USAGE.md
├── EXAMPLES.md
├── TECHNICAL.md
├── ENGINE_INTEGRATION.md
├── EVEOFFLINE_GUIDE.md
├── IMPLEMENTATION_SUMMARY.md
├── NOVAFORGE_PLAN.md             # This file
├── features.md
├── latest.txt                    # Original design discussion
├── test_validation.py
├── test_addon.py
└── .gitignore
```

---

## 11. Integration Points with Projects

This generator is designed to be merged into any game project requiring
procedural 3D assets.  Key integration touchpoints:

| Project Path | Generator Output |
|----------------|-----------------|
| `data/ships/*.json` | Ship metadata consumed by `project_importer.py` |
| `data/ships/obj_models/` | OBJ meshes exported by `atlas_exporter.py` |
| `include/pcg/hull_mesher.h` | C++ hull meshing (Phase 2 pybind11) |
| `include/pcg/pcg_asset_style.h` | Style & deformation hooks |
| `include/pcg/procedural_texture_generator.h` | Texture param generation |
| `include/pcg/character_mesh_system.h` | Character generation |
| `assets/reference_models/modules/` | Modular OBJ parts with hardpoints |
| `schemas/atlas.build.v1.json` | Build manifest validation |

### Workflow

1. Merge this repository as a tool into your project.
2. Define or update ship/station/planet JSON in your project's `data/` directory.
3. Run the batch generator to produce OBJ meshes, textures, LODs, and metadata.
4. Place outputs into your project's asset directories.
5. Your game engine loads assets at runtime using the JSON metadata for
   placement, simulation, and rendering.

---

*This plan is a living document.  Update it as phases are completed and new
requirements emerge.*
