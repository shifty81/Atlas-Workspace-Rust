# Archive: shifty81/AtlasForge

**Archived:** 2026-04-05
**Source:** https://github.com/shifty81/AtlasForge
**Merge Phase:** Phase 5
**Status:** ✅ Extracted

## Description

Original Atlas Game Engine — modular, data-driven C++20 game engine and simulation platform.
Core rendering, physics, ECS, graph VM, deterministic simulation, procedural generation,
AI, networking, scripting, editor with 28+ panels, replay/verification, TLA+ formal specs.
~718 files across 14 top-level directories.

AtlasForge is the **primary origin repo** from which Phases 3 (Nova-Forge-Expeditions)
and 4 (Atlas-NovaForge) already extracted the vast majority of content. Only files
not present from prior phases were extracted in Phase 5.

## Extraction Summary

**11 unique files extracted** + 12 archived references:

| Task | Source | Target | Files | Status |
|------|--------|--------|-------|--------|
| 5A | `engine/script/ScriptSandbox.*` | `Source/Engine/src/Script/` | 2 | ✅ Extracted |
| 5B | `engine/script/ScriptSystem.*` | `Source/Engine/src/Script/` | 2 | ✅ Extracted |
| 5C | `engine/abi/` (ABICapsule, ABIRegistry) | `Source/Engine/src/ABI/` | 4 | ✅ Extracted |
| 5D | `tileeditor/` (CMakeLists + main.cpp) | `Source/Programs/TileEditor/` | 2 | ✅ Extracted |
| 5E | `assets/fonts/builtin_fallback.json` | `Content/Assets/Fonts/` | 1 | ✅ Extracted |
| 5F | Key reference files | `Archive/_AtlasForge/usable_snippets/` | 10 | ✅ Archived |
| 5G | `README.md`, `CONTRIBUTING.md` | `Archive/_AtlasForge/docs_archive/` | 2 | ✅ Archived |

## Overlap with Prior Phases

- **engine/** (34 dirs, ~290 files) — Phase 4 extracted 40 engine module dirs to `Source/Engine/src/`. Nearly all engine content already present.
- **editor/** (~82 files) — Phase 4 extracted editor panels/tools to `Source/Editor/src/`.
- **server/client/runtime** — Phase 3/4 extracted server headers and client code.
- **tests/** (~197 files) — Phase 3/4 extracted 871+ test files to `Tests/`.
- **docs/** (~55 files) — Phase 4 extracted 104 docs to `Docs/Atlas-NovaForge/`.
- **schemas/** (5 files) — Already identical in `Schemas/`.
- **specs/** (3 files) — Phase 4 extracted TLA+ specs.
- **tools/** (~13 files) — Phase 4 extracted tools. crash_reporter, determinism_rules, verify_dependencies already present.
- **modules/atlas_gameplay/** — Phase 3 extracted gameplay modules.

## Superseded Content (Not Extracted — ~685 files)

All engine, editor, server, test, doc, schema, spec, and tool content was already
present in tempnovaforge from Phases 3–4. AtlasForge is the origin repo; later repos
expanded and evolved this content. Archived as usable snippets for reference.

## Unique Content Extracted

1. **ScriptSandbox** — Sandboxed script execution environment (memory/instruction limits, IO whitelist)
2. **ScriptSystem** — Script system integration layer (tick-driven, pause/resume, event subscriptions)
3. **ABICapsule** — ABI capsule system for plugin binary compatibility boundary
4. **ABIRegistry** — Versioned ABI interface registry for module loading
5. **TileEditor** — Standalone tile editor application entry point
6. **Font metadata** — Builtin fallback font definition (bitmap font data)

## Migration Checklist

- [x] Audit source repo contents (~718 files, 14 directories)
- [x] Identify usable code, docs, and assets
- [x] Extract snippets to `Archive/_AtlasForge/usable_snippets/`
- [x] Archive original docs to `Archive/_AtlasForge/docs_archive/`
- [x] Merge relevant content into canonical locations in tempnovaforge
- [x] Update CONSOLIDATION_PLAN.md
- [x] Update TASKS.md
- [x] Verify no regressions (build + tests) — 745 tests pass
- [x] Mark phase as ✅ Done

## Original Repo

https://github.com/shifty81/AtlasForge (to be archived once migration complete)
