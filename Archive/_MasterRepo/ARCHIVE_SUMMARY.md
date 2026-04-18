# Archive: MasterRepo (v001)

**Archived:** 2026-04-05  
**Source:** https://github.com/shifty81/MasterRepo  
**Merge Phase:** Phase 0 — Structural Seed  
**Status:** ✅ Merged

## What Was Taken

| Item | Destination in tempnovaforge |
|------|------------------------------|
| CMakeLists.txt (root) | Root CMakeLists.txt (updated) |
| CMakePresets.json | Root CMakePresets.json (merged — ci-debug, ci-release added) |
| vcpkg.json | Root vcpkg.json (updated) |
| Config/novaforge.project.json | Config/novaforge.project.json |
| Content/Definitions/DevWorld.json | Content/Definitions/DevWorld.json |
| Docs/Game/ROADMAP.md | Docs/Game/ROADMAP.md |
| Docs/Game/TASKS.md | Docs/Game/TASKS.md |
| Docs/Game/ARCHITECTURE.md | Docs/Game/ARCHITECTURE.md |
| Docs/Game/PROJECT_RULES.md | Docs/Game/PROJECT_RULES.md |
| Docs/Game/BUILD_RULES.md | Docs/Game/BUILD_RULES.md |
| GITHUB_COPILOT_IMPLEMENTATION_DIRECTIONS.md | Docs/GITHUB_COPILOT_IMPLEMENTATION_DIRECTIONS.md |
| README.md | Used as reference — tempnovaforge README retained (more complete) |

## What Was NOT Taken (and why)

| Item | Reason |
|------|--------|
| Source/ tree (all modules) | Source/ already exists in tempnovaforge with equal or better content. MasterRepo Source/ structure confirmed and used as the canonical module layout reference. |
| Tests/CMakeLists.txt | tempnovaforge has a more evolved per-module test infrastructure (614 tests). MasterRepo's single-executable NFTests structure is superseded. |
| Docs/Game/ACTION_MATRIX.md | Will be reviewed and absorbed in a follow-up pass |
| Docs/Game/GAP_MATRIX.md | Will be reviewed and absorbed in a follow-up pass |
| Docs/Game/DEFINITION_OF_DONE.md | Will be reviewed and absorbed in a follow-up pass |
| Docs/Game/PHASE_*.md | Will be reviewed and absorbed in a follow-up pass |
| Docs/Game/VOXEL_FIRST_DIRECTIVES.md | Will be reviewed and absorbed in a follow-up pass |

## Key Context

MasterRepo (v001) is the canonical structural reference for the entire consolidation.
- Phase 0–5 of the game loop were complete and validated (215 Catch2 tests at archive time)
- Phase 6 (Multiplayer Foundation) was the active phase in MasterRepo at time of archive
- The editor had a working 7-panel docking layout with Win32/OpenGL
- The Source/ layout (`Core/Engine/Renderer/Physics/Audio/Animation/Input/Networking/UI/Game/Editor/Programs/`) is the module graph all other repos will conform to
- `Docs/GITHUB_COPILOT_IMPLEMENTATION_DIRECTIONS.md` is the editor implementation baseline — all Copilot work should reference it

**tempnovaforge is ahead of MasterRepo:** by archive time tempnovaforge had completed
G1–G17 game phases (614 total Catch2 tests) and the full S0 pipeline core. MasterRepo
served as structural seed; tempnovaforge's implementation content is canonical going forward.

## Usable Code Snippets

See `usable_snippets/` subfolder.
