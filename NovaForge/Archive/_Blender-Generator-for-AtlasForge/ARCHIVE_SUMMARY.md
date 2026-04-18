# Archive: shifty81/Blender-Generator-for-AtlasForge

**Archived:** 2025-07-17
**Source:** https://github.com/shifty81/Blender-Generator-for-AtlasForge
**Source SHA:** `f2873cb9073cac2f09a06b2a885cb2d21d74b3fa`
**Merge Phase:** Phase 11
**Status:** ✅ Done

## Description

Comprehensive Blender addon for procedurally generating spaceships, stations,
and asteroids using an engine-based PCG pipeline. Features 18 ship classes,
brick taxonomy with Ship DNA, 4 faction styles, interior generation, PBR
materials, LOD/collision generation, damage propagation, and power simulation.

## Source Repository Structure

```
Blender-Generator-for-AtlasForge/
├── README.md                  ← Feature overview, installation, ship classes
├── TECHNICAL.md               ← Architecture, data flow, extension points
├── USAGE.md                   ← Installation and usage guide
├── EXAMPLES.md                ← Design philosophy, configuration examples
├── ENGINE_INTEGRATION.md      ← Ship DNA schemas, ECS mappings, hull pipeline
├── IMPLEMENTATION_SUMMARY.md  ← Implementation overview and metrics
├── NOVAFORGE_PLAN.md          ← Phased roadmap for full PCG pipeline
├── EVEOFFLINE_GUIDE.md        ← Project integration guide
├── features.md                ← Complete feature specification
├── __init__.py                ← Main addon entry (30K, UI, registration)
├── ship_generator.py          ← Orchestrates ship generation (19K)
├── ship_parts.py              ← Individual ship components (37K)
├── interior_generator.py      ← FPV-ready interior spaces (13K)
├── module_system.py           ← Attachable modules (10K)
├── brick_system.py            ← Brick taxonomy, grid, Ship DNA (11K)
├── atlas_exporter.py          ← JSON import + OBJ export (6K)
├── station_generator.py       ← Procedural station generation (11K)
├── asteroid_generator.py      ← Asteroid belt generation (8K)
├── texture_generator.py       ← Procedural PBR materials (17K)
├── novaforge_importer.py      ← Project JSON import (6K)
├── render_setup.py            ← Catalog render setup (6K)
├── lod_generator.py           ← LOD generation (5K)
├── collision_generator.py     ← Collision mesh generation (12K)
├── animation_system.py        ← Ship animations (12K)
├── damage_system.py           ← Damage propagation (15K)
├── power_system.py            ← Power flow simulation (9K)
├── build_validator.py         ← Build validation (11K)
├── test_addon.py              ← Functional tests (6K)
├── test_validation.py         ← Structure validation (54K)
├── latest.txt                 ← Original design discussion (251K)
└── .gitignore
```

## What Was Extracted

### All Files → `Archive/_Blender-Generator-for-AtlasForge/` (not Tools/)

Since `Tools/BlenderGenerator/` already contains Phase 4 content
(BlenderSpaceshipGenerator/, blender-addon/, novaforge_bridge.py, README.md),
this repo's content is archived separately to avoid conflicts.

#### Addon Source → `addon_source/` (22 files)

Complete Python addon preserved:
- `__init__.py` — Main addon entry, UI panel, properties (30K)
- `ship_generator.py` — Ship generation orchestrator (19K)
- `ship_parts.py` — Component generation: hull, cockpit, engines, wings, turrets (37K)
- `interior_generator.py` — FPV-ready interiors: bridge, corridors, quarters (13K)
- `module_system.py` — 6 module types with progressive availability (10K)
- `brick_system.py` — 18 brick types, Ship DNA JSON export (11K)
- `atlas_exporter.py` — JSON import + OBJ export pipeline (6K)
- `station_generator.py` — NPC stations and Upwell structures (11K)
- `asteroid_generator.py` — 16 ore types, 4 belt layouts (8K)
- `texture_generator.py` — Procedural PBR with weathering (17K)
- `novaforge_importer.py` — Project ship JSON import (6K)
- `render_setup.py` — 3/4-view camera, three-point lighting (6K)
- `lod_generator.py` — 4-level LOD pipeline (5K)
- `collision_generator.py` — Convex hull, box, multi-convex (12K)
- `animation_system.py` — Turret rotation, bay doors, landing gear (12K)
- `damage_system.py` — Brick HP, structural integrity, detachment cascade (15K)
- `power_system.py` — Capacitor charge/drain, system disable (9K)
- `build_validator.py` — Build validation checks (11K)
- `test_addon.py` — Functional tests (6K)
- `test_validation.py` — Structure validation tests (54K)
- `latest.txt` — Original design discussion (251K)
- `.gitignore`

#### Documentation → `docs/` (9 files)

- `README.md` — Feature overview, installation, usage
- `TECHNICAL.md` — Architecture, design patterns, API, extension points
- `USAGE.md` — Installation guide, configuration, troubleshooting
- `EXAMPLES.md` — Design philosophy, fleet building, export tips
- `ENGINE_INTEGRATION.md` — Ship DNA schemas, brick tables, ECS component mappings
- `IMPLEMENTATION_SUMMARY.md` — Implementation overview and metrics
- `NOVAFORGE_PLAN.md` — 7-phase roadmap: PCG pipeline → C++ integration → ECS
- `EVEOFFLINE_GUIDE.md` — Step-by-step project integration guide
- `features.md` — Complete feature specification (19 sections)

## What Was NOT Extracted (and why)

| Content | Reason |
|---------|--------|
| (nothing omitted) | All files from the repo were archived |

## Overlap Analysis

| Area | Overlap | Resolution |
|------|---------|------------|
| `Tools/BlenderGenerator/` | Phase 4 already placed `BlenderSpaceshipGenerator/`, `blender-addon/`, `novaforge_bridge.py`, `README.md` | Phase 11 content archived separately in `Archive/_Blender-Generator-for-AtlasForge/` |
| Addon concept | This repo is a significantly expanded version of the Phase 4 addon | Both preserved; this is the more complete/evolved version |
| `features.md` / `ENGINE_INTEGRATION.md` | Unique to this repo; no overlap with existing monorepo docs | Archived in `docs/` |

## Key Features (from source repo)

- **18 ship classes** (Shuttle → Titan + Industrial, Mining, Exhumer)
- **9-stage spine-first assembly** pipeline
- **Brick taxonomy** (18 types, 5 categories) with Ship DNA JSON
- **4 faction styles**: Solari (gold), Veyren (steel), Aurelian (green), Keldari (rust)
- **4 game-inspired styles**: X4, Elite Dangerous, Eve Online, NMS
- **Interior generation**: Human-scale FPV-ready rooms
- **Station generation**: 8 types with faction architecture
- **Asteroid belts**: 16 ore types, 4 layouts
- **PBR materials**: Procedural with weathering
- **LOD + collision**: 4-level LOD, convex hull meshes
- **Damage system**: Brick HP, structural integrity, salvage
- **Power simulation**: EVE-style capacitor charge/drain
- **OBJ export**: +Z-forward, +Y-up, 1 unit = 50m

## Migration Checklist

- [x] Audit source repo contents
- [x] Identify usable code, docs, and assets
- [x] Extract addon source to `Archive/_Blender-Generator-for-AtlasForge/addon_source/`
- [x] Archive docs to `Archive/_Blender-Generator-for-AtlasForge/docs/`
- [x] Verify no conflicts with `Tools/BlenderGenerator/` (Phase 4)
- [x] Update ARCHIVE_SUMMARY.md with full audit
- [x] Mark phase as ✅ Done
