# Atlas Engine Integration — Blender Spaceship Generator

This document describes how assets produced by the Blender Spaceship Generator
integrate into the Atlas Engine build and runtime pipeline.

## Export Pipeline

```
Blender (Generate) ──▶ .obj + .atlas_meta ──▶ AssetImporter ──▶ Engine Mesh + Metadata
```

### Step 1: Generate in Blender

The addon creates a mesh hierarchy:

- **Hull** — The outer shell mesh (convex hull from procedural profile)
- **Slots** — Empty objects marking module attachment points, named
  `slot_<type>_<index>` (e.g. `slot_weapon_0`, `slot_engine_1`)
- **Interior volumes** — Optional bounding boxes for interior module placement

### Step 2: Export

Click **Export for Atlas** in the sidebar panel. This writes:

1. **`<name>.obj`** — Standard Wavefront OBJ with hull geometry and slot
   positions encoded as vertices in a `slots` group.
2. **`<name>.atlas_meta`** — JSON sidecar:

```json
{
  "version": 1,
  "seed": 42,
  "hull_class": "cruiser",
  "tier": 3,
  "slots": [
    {
      "name": "slot_weapon_0",
      "type": "weapon",
      "position": [1.2, 0.0, 3.4],
      "size": "M"
    },
    {
      "name": "slot_engine_0",
      "type": "engine",
      "position": [-4.0, 0.0, 0.0],
      "size": "L"
    }
  ],
  "materials": {
    "hull": {
      "metallic": 0.8,
      "roughness": 0.35
    }
  },
  "bounding_box": {
    "min": [-5.0, -2.0, -2.0],
    "max": [5.0, 2.0, 2.0]
  }
}
```

### Step 3: Import into Atlas Engine

```cpp
#include "engine/assets/AssetImporter.h"

// Load the mesh
auto mesh = assetImporter.LoadMesh("ships/cruiser_01.obj");

// Load the metadata sidecar
auto meta = assetImporter.LoadJSON("ships/cruiser_01.atlas_meta");

// Create interior nodes from slot data
atlas::procedural::InteriorNode interior;
for (const auto& slot : meta["slots"]) {
    atlas::procedural::ModuleSlot ms;
    ms.size = ParseModuleSize(slot["size"]);
    ms.position = ParseVec3(slot["position"]);
    interior.AddSlot(ms);
}

// Apply PBR material from metadata
atlas::render::PBRMaterial mat;
mat.SetMetallic(meta["materials"]["hull"]["metallic"]);
mat.SetRoughness(meta["materials"]["hull"]["roughness"]);
```

### Step 4: Runtime

At runtime the engine uses:
- **InteriorNode** for module placement validation
- **HullMeshGenerator** for LOD generation / procedural refinement
- **PBRMaterial** for rendering
- **SpatialHash** for proximity queries against the ship's bounding box
- **ModuleTier** for visual tier progression

## Directory Conventions

Place exported assets under the project's asset directory:

```
projects/<game>/assets/
├── ships/
│   ├── cruiser_01.obj
│   ├── cruiser_01.atlas_meta
│   ├── fighter_01.obj
│   └── fighter_01.atlas_meta
└── textures/
    ├── hull_cruiser_albedo.png
    ├── hull_cruiser_normal.png
    └── ...
```

## Deterministic Regeneration

The seed stored in `.atlas_meta` allows bit-identical regeneration:

```cpp
atlas::procedural::DeterministicRNG rng(meta["seed"]);
// Regenerate the same ship from just the seed + hull class + tier
```

This aligns with the Atlas Engine's determinism-first philosophy and PCG
framework (see `engine/procedural/PCGManager.h`).
