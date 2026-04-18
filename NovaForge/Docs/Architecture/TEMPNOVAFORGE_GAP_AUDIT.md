# NovaForge — TempNovaForge Gap Audit

**Date:** 2026-04
**Auditor:** Architecture Review

---

## Purpose

This document catalogs the critical architecture and documentation gaps found in the
`tempnovaforge` repository after the MasterRepo consolidation phase (C0). It serves
as the authoritative action list for bringing the repo to a production-ready state.

---

## Current State Summary

- Engine phases 0–9: ✅ Complete
- Game phases G1–G20: ✅ Complete (641 Catch2 tests)
- Build system: ✅ CMake 3.20 with presets (NF_STANDALONE/NF_HOSTED)
- Test suite: ✅ Catch2 v3.5.2, 13 test executables
- Naming consistency: ✅ AtlasAI canonical name established; AtlasAI/ directory created
- Project manifest: ✅ `Project/project.atlas.json` exists and valid
- Hosted build mode: ✅ `NF_HOSTED` / `NF_STANDALONE` CMake options implemented
- Documentation: ✅ Architecture, Gameplay, Systems docs created
- Directory stubs: ✅ Logs/, Codex/, Content/Incoming/ exist with .gitkeep
- VS2022 test build: ✅ `catch_main.cpp` approach — uses `Catch2::Catch2` directly
- Program mains: ✅ Real Win32 window in Editor with GDI double-buffer rendering
- Season config: ✅ `Config/season.config.json` exists
- Project validator: ✅ `Scripts/validate_project.sh` — 56 checks, all pass

---

## Critical Architecture Gaps

### GAP 1 — Naming Inconsistency
**Severity:** HIGH
**Problem:** The codebase references `ArbiterAI`, `Arbiter`, and `SwissAgent` as separate
tool identities. The canonical name is `AtlasAI` for all AI broker functionality.
**Action:** Create `Docs/Architecture/NAMING_CANON.md`. Update all READMEs in
`Tools/ArbiterAI/` and `Tools/SwissAgent/`. Update README.md.
**Status:** ✅ Resolved — AtlasAI/ directory created; all READMEs updated; NAMING_CANON.md established
**Severity:** HIGH
**Problem:** `Project/project.atlas.json` does not exist. This is the authoritative
contract required for Atlas Workspace hosted loading.
**Action:** Create `Project/project.atlas.json` with full entrypoints, paths, build
config, pipeline config, and atlas_workspace contract block.
**Status:** ✅ Resolved — `Project/project.atlas.json` created with full contract

### GAP 3 — No Hosted Build Mode
**Severity:** HIGH
**Problem:** CMakeLists.txt has no `NF_HOSTED` or `NF_STANDALONE` options. When Atlas
Workspace controls the build, there is no way to disable executables by default.
**Action:** Add `NF_STANDALONE` (ON) and `NF_HOSTED` (OFF) options. Gate
`NF_BUILD_EDITOR`, `NF_BUILD_GAME`, `NF_BUILD_SERVER` defaults on `_NF_DEFAULT_BUILD_TARGETS`.
**Status:** ✅ Resolved — NF_STANDALONE/NF_HOSTED options in CMakeLists.txt

### GAP 4 — VS2022 Test Build Failure (LNK4098)
**Severity:** HIGH
**Problem:** `Catch2::Catch2WithMain` triggers MSVC static-lib extraction issues on the
Visual Studio generator, causing `LNK4098` linker warnings and potential test failures.
**Action:** Create `Tests/catch_main.cpp` with explicit `Catch::Session().run(argc, argv)`.
Update all 13 test targets in `Tests/CMakeLists.txt` to use `Catch2::Catch2` and include
`catch_main.cpp` directly.
**Status:** ✅ Resolved — Tests/catch_main.cpp with Catch2::Catch2 (not WithMain)

### GAP 5 — No Real Win32 Window in Editor/Game
**Severity:** MEDIUM
**Problem:** `Source/Programs/NovaForgeEditor/main.cpp` and `NovaForgeGame/main.cpp`
have no real Win32 window creation. The editor shows nothing on Windows — it is
effectively headless.
**Action:** Replace both mains with full Win32 window creation (double-buffered GDI),
proper WndProc, message loop, and frame timing. Editor renders the full panel layout.
**Status:** ✅ Resolved — Full Win32 window with GDI double-buffer, 7-panel layout

### GAP 6 — Game Phases G18–G20 Not Implemented
**Severity:** HIGH
**Problem:** G18 (Status Effects), G19 (Contracts & Bounties), and G20 (Companion System)
are tracked in the roadmap but have no code or tests.
**Action:** Implement G18, G19, G20 in `Source/Game/include/NF/Game/Game.h`.
Add comprehensive tests to `Tests/Game/test_game.cpp`.
**Status:** ✅ Resolved — G18 (Status Effects), G19 (Contracts & Bounties), G20 (Companion System) implemented with tests

### GAP 7 — Missing Architecture Documentation
**Severity:** MEDIUM
**Problem:** No canonical architecture documents exist in `Docs/Architecture/`.
The codebase has no single source of truth for naming, build modes, voxel pipeline,
hosted contract, or deferred workspace items.
**Action:** Create: NAMING_CANON.md, CURRENT_DIRECTION.md, HOSTED_PROJECT_CONTRACT.md,
BUILD_MODES.md, VOXEL_RENDER_PIPELINE.md, DEFERRED_TO_WORKSPACE.md.
**Status:** ✅ Resolved — All architecture docs created (NAMING_CANON, CURRENT_DIRECTION, HOSTED_PROJECT_CONTRACT, BUILD_MODES, VOXEL_RENDER_PIPELINE, DEFERRED_TO_WORKSPACE)

### GAP 8 — Missing Gameplay Documentation
**Severity:** MEDIUM
**Problem:** No `Docs/Gameplay/` directory or R.I.G. system specification exists.
**Action:** Create `Docs/Gameplay/RIG_SYSTEM.md`.
**Status:** ✅ Resolved — `Docs/Gameplay/RIG_SYSTEM.md` created

### GAP 9 — Missing Systems Documentation
**Severity:** MEDIUM
**Problem:** No `Docs/Systems/` directory or GraphVM visual scripting documentation exists.
**Action:** Create `Docs/Systems/VISUAL_SCRIPTING.md`.
**Status:** ✅ Resolved — `Docs/Systems/VISUAL_SCRIPTING.md` created

### GAP 10 — Missing Directory Stubs
**Severity:** LOW
**Problem:** `Logs/`, `Codex/Snippets/`, `Codex/Fixes/`, `Codex/Graphs/`,
and `Content/Incoming/` are referenced in contracts and configs but do not exist
as tracked directories.
**Action:** Create `.gitkeep` files in each directory.
**Status:** ✅ Resolved — All stubs created with .gitkeep

### GAP 11 — Missing Logger Format and Season Config
**Severity:** LOW
**Problem:** `Logs/build.logger` format is undocumented. `Config/season.config.json`
does not exist despite season system being part of the game design.
**Action:** Create `Logs/logger.format.md` and `Config/season.config.json`.
**Status:** ✅ Resolved — `Logs/logger.format.md` and `Config/season.config.json` created

### GAP 12 — AtlasToolingSuite Reference in README
**Severity:** LOW
**Problem:** README.md references `AtlasToolingSuite` as a separate entity. The
canonical name is `Atlas Workspace`. The consolidation table still shows deprecated
repo names without proper canonical mapping.
**Action:** Surgical README.md updates — replace deprecated names, add fleet note,
remove ImGui references.
**Status:** ✅ Resolved — README.md updated with canonical AtlasAI naming

---

## Documentation Reset — Complete

All required documents have been created:

| File | Status |
|---|---|
| `Docs/Architecture/NAMING_CANON.md` | ✅ Created |
| `Docs/Architecture/CURRENT_DIRECTION.md` | ✅ Created |
| `Docs/Architecture/HOSTED_PROJECT_CONTRACT.md` | ✅ Created |
| `Docs/Architecture/BUILD_MODES.md` | ✅ Created |
| `Docs/Architecture/VOXEL_RENDER_PIPELINE.md` | ✅ Created |
| `Docs/Architecture/DEFERRED_TO_WORKSPACE.md` | ✅ Created |
| `Docs/Gameplay/RIG_SYSTEM.md` | ✅ Created |
| `Docs/Systems/VISUAL_SCRIPTING.md` | ✅ Created |
| `Logs/logger.format.md` | ✅ Created |

The following documents need to be created (none existed before this audit):

| File | Priority |
|---|---|
| `Docs/Architecture/NAMING_CANON.md` | P0 |
| `Docs/Architecture/CURRENT_DIRECTION.md` | P0 |
| `Docs/Architecture/HOSTED_PROJECT_CONTRACT.md` | P0 |
| `Docs/Architecture/BUILD_MODES.md` | P1 |
| `Docs/Architecture/VOXEL_RENDER_PIPELINE.md` | P1 |
| `Docs/Architecture/DEFERRED_TO_WORKSPACE.md` | P1 |
| `Docs/Gameplay/RIG_SYSTEM.md` | P1 |
| `Docs/Systems/VISUAL_SCRIPTING.md` | P1 |
| `Logs/logger.format.md` | P2 |

---

## Implementation Priority

| Priority | Items |
|---|---|
| P0 — Blockers | GAP 1 (naming), GAP 2 (manifest), GAP 3 (hosted build), GAP 4 (VS2022 test) |
| P1 — High Value | GAP 5 (Win32 window), GAP 6 (G18-G20 game phases), GAP 7-9 (docs) |
| P2 — Housekeeping | GAP 10 (stubs), GAP 11 (logger/season), GAP 12 (README) |

---

## Success Condition

The audit is resolved when:
- [x] `cmake --preset debug -DNF_BUILD_TESTS=ON && cmake --build --preset debug` succeeds
- [x] `ctest --preset debug` reports 0 failures (641 tests)
- [x] No source file references ArbiterAI, SwissAgent, or ImGui outside Archive/
- [x] `Project/project.atlas.json` exists and is valid JSON
- [x] All 12 GAPs above are marked ✅
- [x] `Scripts/validate_project.sh` reports 56/56 checks pass
- [x] `AtlasAI/` directory structure established

---

## Final Note

This audit was triggered by the discovery that the repo had grown to G17 of the game
phases without establishing the foundational architecture contracts that Atlas Workspace
requires for hosted deployment. The C0 consolidation phase created a strong code base.
This audit ensures the documentation and build infrastructure match the code quality.
