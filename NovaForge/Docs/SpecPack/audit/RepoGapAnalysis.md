# Repo Gap Analysis

## Assessment Date: 2026-04-05

## Executive Summary
NovaForge has a **mature engine** with 860 tests, 449 game systems, 16 component types,
and 9 data schemas. The architecture is correct. Implementation is blocked by missing
**enforceable system contracts** — specifically material definitions, centrifuge processing,
interface port, and sand physics.

## What Exists (Strong)

### Engine & Editor (Phases 0-11) ✅
- Full C++20 build system with CMake presets
- Core math, ECS, input, rendering, physics, audio, animation
- Voxel runtime (Chunk, VoxelType, WorldGenerator)
- Editor with docking, 50+ panels, undo/redo, commands
- AtlasUI framework (15+ widgets, themes, services)
- Pipeline module (5 tools, event routing, CI gate)
- GLFW/ImGui window support (M1 stubs)

### Game Systems (G1-G21) ✅
- 21 game phases implemented with full test coverage
- Ship systems, fleet AI, economy, exploration
- Quest/mission, dialogue, save/load
- Tech tree, skills, crafting, inventory
- Status effects, contracts, companions, factions
- **449 server system headers** with component architecture

### Data Layer ✅
- 9 JSON schemas (module, ship, mission, skill, atlas configs)
- 18 data directories with game definitions
- Comprehensive ship/module/mission data files

### Documentation ✅
- 40+ architecture/design docs
- 14 SpecRollup specifications
- R.I.G. system spec, consolidation plan
- Feature gap matrix, audit checklist

## What Is Missing (Blockers)

### Tier 1 — Blocks Everything
| Gap | Impact | Schema Status |
|-----|--------|---------------|
| Voxel material properties | No mining yields, no collapse rules | ✅ Schema created |
| Centrifuge processing | No resource conversion, economy stalled | ✅ Schema created |
| Interface port system | No vehicle/terminal control | Schema N/A (code needed) |
| Sand/collapse physics | No subsurface tension | Schema N/A (code needed) |
| R.I.G. slot validation | No module install enforcement | ✅ Schema created |

### Tier 2 — Blocks Gameplay Depth
| Gap | Impact |
|-----|--------|
| Breach minigame | No hacking mechanic |
| Environmental simulation | No oxygen/hazard pressure |
| Injury persistence | Status effects are temporary only |
| Power consumption math | No fuel rates, load shedding values |
| R.I.G. upgrade costs | No progression numbers |

### Tier 3 — Blocks Workspace
| Gap | Impact |
|-----|--------|
| PropertyGrid widget | Can't edit entity properties in editor |
| TreeView widget | Can't browse hierarchies effectively |
| Layout persistence | Editor layout resets each session |
| Viewport host contract | No 3D rendering surface standard |
| AtlasAI panel host | No visible AI interface |
| File intake pipeline | No drag-and-drop asset import |

## Risk Assessment
- **High Risk**: Systems will drift without schema enforcement
- **Medium Risk**: Editor usability limited without workspace widgets
- **Low Risk**: Game content (missions, ships) can expand freely with existing schemas

## Recommendation
1. Implement Tier 1 schemas + C++ contracts first
2. Wire schemas into runtime validation
3. Then expand workspace widgets for editor
4. Game content expansion can proceed in parallel
