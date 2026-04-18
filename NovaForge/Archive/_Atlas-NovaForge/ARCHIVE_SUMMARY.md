# Archive: shifty81/Atlas-NovaForge

**Archived:** 2026-04-05
**Source:** https://github.com/shifty81/Atlas-NovaForge
**Merge Phase:** Phase 4
**Status:** ✅ Extracted (build verified — 745 tests pass)

## Description

Engine + game merge attempt — hybrid engine/game structure with C++ server, C++ client,
full engine module set, editor panels/tools, game data, Python tools, TLA+ formal specs,
sample projects, and BlenderSpaceshipGenerator.

## Extraction Summary

**1,264 files extracted** from Atlas-NovaForge into canonical tempnovaforge paths:

| Task | Source | Target | Files |
|------|--------|--------|-------|
| 4B | engine/ (40 module dirs) | Source/Engine/src/ | 241 |
| 4C | editor/ (panels/tools/ai/assistant/ui) | Source/Editor/src/ | 89 |
| 4D | cpp_client/src+include | Source/Game/src/Client/ + include/NF/Game/Client/ | 244 |
| 4E | data/ (20 game data dirs) | Data/ | 95 |
| 4F | schemas/ (5 JSON schemas) | Schemas/ | 5 |
| 4G | specs/ (3 TLA+ specs) | Docs/Atlas-NovaForge/specs/ | 3 |
| 4H | tools/ (Python + BlenderSpaceshipGenerator) | Tools/Atlas-NovaForge/ + Tools/BlenderGenerator/ | 52 |
| 4I | projects/ (4 sample projects) | Project/samples/ | 130 |
| 4J | docs/ (~102 docs) | Docs/Atlas-NovaForge/ | 104 |
| 4K | tests/ + atlas_tests/ | Tests/Atlas-NovaForge/ | 306 |
| 4L | superseded + usable snippets | Archive/_Atlas-NovaForge/ | 18 |

## Overlap with Phase 3 (Nova-Forge-Expeditions)

- **cpp_server/** — Same structure as NFE cpp_server (auth/config/data/ecs/network/pcg/systems/ui/utils).
  Phase 3 extracted the expanded systems (449 .cpp, 541 headers, 548 tests).
  Atlas-NovaForge cpp_server is the earlier iteration. Key files archived as usable_snippets.
- **engine/procedural/** and **engine/render/** — Overlap with Phase 3 engine extraction.
  Phase 4 extracted full engine module set (40 dirs) to Source/Engine/src/.
- **modules/atlas_gameplay/** — Same module extracted in Phase 3.
- **editor/tools/** — 40 tools already existed from Phase 3. New unique tools extracted alongside.

## Superseded Content (Not Extracted)

- cpp_server/ — Superseded by Phase 3 NFE extraction (server code is authoritative from NFE)
- testing/ — Binary test assets (zip/rar files, ~20MB total)
- assets/ — Binary assets (blend files, zips)
- legacy/ — Old planning text files (aiimp, gaps.txt, gui_issues.txt, etc.)
- archive/ — Original repo's own archive (planning_notes, legacy Python)
- scripts/ — Build scripts (build.bat, build.sh, build_all, build_vs, generate_solution)
- "From old repo/" — Data, docs, projects from an even older repo iteration

## Migration Checklist

- [x] Audit source repo contents (22 top-level dirs, ~2000 files)
- [x] Identify usable code, docs, and assets
- [x] Extract snippets to `Archive/_Atlas-NovaForge/usable_snippets/`
- [x] Archive original docs to `Archive/_Atlas-NovaForge/docs_archive/`
- [x] Merge relevant content into canonical locations in tempnovaforge
- [x] Update CONSOLIDATION_PLAN.md
- [x] Update TASKS.md
- [x] Verify no regressions (build + tests) — 745 tests pass, no breakage
- [x] Mark phase as ✅ Done

## Notes

- Extracted files use `atlas::` namespace with their own includes — not yet compiled.
  Future namespace migration (atlas:: → NF::) needed for integration.
- Engine modules provide the most significant new content: 40 subdirectories covering
  AI, animation, audio, camera, character, conversation, ECS, flow graphs, gameplay,
  graph VM, input, interaction, mod system, networking, physics, platform, plugin,
  procedural, production, project, rendering, rules, schema, scripting, simulation,
  sound, story, strategy graph, and tile systems.
- BlenderSpaceshipGenerator is a substantial Blender addon (~31 Python files) with
  ship generation, damage systems, LOD, interior generation, and PCG pipeline.

## Original Repo

https://github.com/shifty81/Atlas-NovaForge
