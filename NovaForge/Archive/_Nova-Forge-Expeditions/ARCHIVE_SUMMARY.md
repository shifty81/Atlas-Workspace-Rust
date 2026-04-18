# Archive: shifty81/Nova-Forge-Expeditions

**Archived:** 2026-04-05
**Source:** https://github.com/shifty81/Nova-Forge-Expeditions
**Merge Phase:** Phase 3
**Status:** ✅ Extracted — 1,708 files merged into tempnovaforge

## Description

Nova-Forge-Expeditions is the richest game codebase — a three-way merge of
AtlasForge (engine), NovaForge (game), and Atlas-NovaForge (combined) into
a unified ~5,900 file repository.

## Audit Summary

This repo represents a prior consolidation of 3 other repos:
- **Atlas-NovaForge** (primary — most complete engine+game merge): 3,090 files
- **NovaForge** (original standalone game): 2,566 files
- **AtlasForge** (original engine): 898 files

### Key Content Identified

| Area | Files | Source | Notes |
|------|-------|--------|-------|
| Engine core | ~700 | Atlas-NovaForge | PCG framework, rendering pipeline, tools, AtlasUI widgets |
| Game systems | 449 | NovaForge | Full server-side game systems (combat, economy, missions) |
| Game tests | 548 | NovaForge | Per-domain test suite for all game systems |
| Client | 311 | Atlas-NovaForge | Game client implementation |
| Server headers | 541 | NovaForge | Game server include files |
| Editor | ~190 | Atlas-NovaForge | 26 ToolingLayer tools, full editor implementation |
| Documentation | 379 | Mixed | Architecture docs, session history, design specs |
| Atlas tests | 51 | Atlas-NovaForge | Engine test suite |
| Engine tests | ~130 | Mixed | AtlasUI, editor tools, PCG, rendering, procedural tests |
| Tools | ~25 | Mixed | Blender addon with ship presets |
| Game data | ~400 | Atlas-NovaForge | JSON game data files |
| Modules | ~50 | Atlas-NovaForge | Gameplay modules (Faction, Combat, Economy) |
| Projects | ~30 | Atlas-NovaForge | Example game projects (eveoffline, arena2d, novaforge) |
| Schemas | ~20 | Atlas-NovaForge | Data schemas |

### Unique Content (not yet in tempnovaforge)

1. **449 game systems** (cpp_server/src/systems/) — abyssal, achievement, agent, alliance,
   ambient, ancient, etc. — the complete game server logic
2. **PCG framework** (engine/procedural/) — PCGManager, ConstraintSolver, HullMeshGenerator,
   DeterministicRNG, planetary base/interior generation
3. **Rendering pipeline** (engine/render/) — GBuffer, PBR, PostProcess, ShadowMap, InstancedRenderer
4. **ToolingLayer tools** (engine/tools/) — 16 editor tools with ITool interface
5. **AtlasUI widget set** (engine/ui/atlas/) — earlier AtlasUI implementation
6. **Blender addon** (tools/blender-addon/) — procedural spaceship mesh generator with presets
7. **Game modules** (modules/atlas_gameplay/) — FactionSystem, CombatFramework, EconomySystem
8. **Session archive** (docs/archive/sessions/) — 80+ development session summaries

### Known Overlaps Requiring Resolution

1. `engine/render/` — dual GL/standalone implementations need namespace separation
2. `cpp_server/src/systems/` — 192 ANF-refactored vs 449 NovaForge originals
3. `GameSession` — ANF domain-decomposed vs NovaForge `handlers/` pattern
4. `engine/tools/` vs `editor/` — duplicate tool implementations
5. `tests/` vs `atlas_tests/` — two test directories

## What Goes Where (Merge Map)

| Nova-Forge-Expeditions | tempnovaforge Target | Priority |
|------------------------|---------------------|----------|
| cpp_server/src/systems/ | Source/Game/src/Systems/ | High — game logic |
| cpp_server/include/ | Source/Game/include/ | High — game headers |
| cpp_server/tests/ | Tests/Game/ | High — game tests |
| engine/procedural/ | Source/Engine/src/PCG/ | Medium — PCG |
| engine/render/ (standalone) | Source/Renderer/ | Medium — rendering |
| engine/tools/ (ITool.h) | Source/Editor/ | Medium — tool interfaces |
| engine/ui/atlas/ | Archive (superseded by AtlasUI) | Low — reference only |
| tools/blender-addon/ | Tools/BlenderGenerator/ | Medium — merge with existing |
| modules/atlas_gameplay/ | Source/Game/src/Modules/ | Medium — gameplay modules |
| docs/ (unique) | Docs/ | Low — documentation |

## Migration Checklist

- [x] Audit source repo contents
- [x] Identify usable code, docs, and assets
- [x] Map content to canonical tempnovaforge locations
- [x] Extract game systems (449 systems) into Source/Game/src/Systems/
- [x] Extract server headers (541 files) into Source/Game/include/NF/Game/Server/
- [x] Extract game tests (548 tests) into Tests/Game/Server/
- [x] Extract PCG framework (36 files) into Source/Engine/src/PCG/
- [x] Extract rendering pipeline (28 files) into Source/Renderer/src/Pipeline/
- [x] Extract editor tools (40 files) into Source/Editor/src/Tools/
- [x] Extract gameplay modules (8 files) into Source/Game/src/Modules/
- [x] Merge Blender addon (8 files) into Tools/BlenderGenerator/blender-addon/
- [x] Merge unique docs (40 files) into Docs/Nova-Forge-Expeditions/
- [x] Archive superseded AtlasUI (7 files) → superseded/engine_ui_atlas/
- [x] Archive AUDIT.md + ROADMAP.md → docs_archive/
- [x] Update CONSOLIDATION_PLAN.md — Phase 3 marked extracted
- [x] Update TASKS.md
- [x] Verify no regressions (build + tests: 745/745 passing)

**Total extracted: 1,708 files**

## Original Repo

https://github.com/shifty81/Nova-Forge-Expeditions (to be archived once content extraction complete)
