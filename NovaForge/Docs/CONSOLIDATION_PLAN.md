# NovaForge — Repo Consolidation Plan

## Overview

All 12 source repos are being merged into `tempnovaforge` (this repo).
MasterRepo (v001) provides the structural skeleton.
Each repo is audited, refactored, and merged in phase order.
When a repo is fully merged, its contents are archived under `Archive/_RepoName/` and the source repo is retired.

## Phase Order

| Phase | Repo | Priority | Status |
|-------|------|----------|--------|
| 0 | MasterRepo | Structural seed | ✅ Done |
| 1 | MasterRepoRefactor | Structure + Atlas dirs | ✅ Done |
| 2 | AtlasToolingSuite | Tool suite | ✅ Done |
| 3 | Nova-Forge-Expeditions | Richest game codebase | ✅ Extracted |
| 4 | Atlas-NovaForge | Engine+game merge attempt | ✅ Extracted |
| 5 | AtlasForge | Original engine | ✅ Extracted |
| 6 | NovaForge-Project | Game project structure + rules | ✅ Extracted |
| 7 | SwissAgent | AI → Atlas_SwissAgent | ✅ Extracted |
| 8 | ArbiterAI | AI → Atlas_Arbiter | ✅ Extracted |
| 9 | Arbiter | AI → Atlas_Arbiter | ✅ Extracted |
| 10 | AtlasForge-EveOffline | Networking prototype | ✅ Extracted |
| 11 | Blender-Generator-for-AtlasForge | Tools/Atlas_BlenderGen | ✅ Extracted |

## Internal Consolidation (C3)

Phase C3 consolidates internal duplicate structures and integrates pending
content that was already present in the repo but not yet landed in the main tree.

| Task | Description | Status |
|------|-------------|--------|
| C3-1 | Archive C2 (AtlasToolingSuite) | ✅ Done — `Archive/_AtlasToolingSuite/` |
| C3-2 | Integrate Additions/ starter packs into Source/UI/ | ✅ Done — AtlasUI.h, SimplePanel, Label, Tabs, StackPanel, WidgetKit |
| C3-3 | Consolidate duplicate AtlasAI locations | ✅ Done — `Tools/AtlasAI/` → `AtlasAI/` (root) |
| C3-4 | Deprecate legacy tool paths | ✅ Done — `Tools/ArbiterAI/`, `Tools/SwissAgent/` marked deprecated |
| C3-5 | Scaffold archive stubs for phases 3–11 | ✅ Done — `Archive/_RepoName/ARCHIVE_SUMMARY.md` for each |
| C3-6 | Update CMakeLists.txt with new sources | ✅ Done — 3 new .cpp files |
| C3-7 | Update tracking docs | ✅ Done |

## Phase 3 Extraction (Nova-Forge-Expeditions)

Phase 3 performed a file-level extraction of unique content from `shifty81/Nova-Forge-Expeditions`
(~5,900 files, three-way merge of AtlasForge + NovaForge + Atlas-NovaForge).

| Task | Source Path | Target Path | Files | Status |
|------|------------|-------------|-------|--------|
| 3A | `cpp_server/src/systems/` | `Source/Game/src/Systems/` | 449 | ✅ Extracted |
| 3B | `cpp_server/include/` | `Source/Game/include/NF/Game/Server/` | 541 | ✅ Extracted |
| 3C | `cpp_server/tests/` | `Tests/Game/Server/` | 548 | ✅ Extracted |
| 3D | `engine/procedural/` | `Source/Engine/src/PCG/` | 36 | ✅ Extracted |
| 3E | `engine/render/` | `Source/Renderer/src/Pipeline/` | 28 | ✅ Extracted |
| 3F | `engine/tools/` | `Source/Editor/src/Tools/` | 40 | ✅ Extracted |
| 3G | `modules/atlas_gameplay/` | `Source/Game/src/Modules/` | 8 | ✅ Extracted |
| 3H | `tools/blender-addon/` | `Tools/BlenderGenerator/blender-addon/` | 8 | ✅ Extracted |
| 3I | `docs/` (unique) | `Docs/Nova-Forge-Expeditions/` | 40 | ✅ Extracted |
| 3J | `engine/ui/atlas/` | `Archive/_Nova-Forge-Expeditions/superseded/` | 7 | ✅ Archived |

**Total extracted: 1,708 files**

**Note:** Extracted files use `atlas::` namespace conventions from the source repo.
They are not yet compiled into the tempnovaforge build. Future refactoring work includes:
- Namespace migration: `atlas::` → `NF::` (for files that will be compiled)
- Include path adaptation to match tempnovaforge conventions
- CMakeLists.txt wiring for active compilation
- Game system template migration (`SingleComponentSystem<C>` per ROADMAP Phase A1)

## Phase 4 Extraction (Atlas-NovaForge)

Phase 4 performed a file-level extraction of unique content from `shifty81/Atlas-NovaForge`
(~2,000 files, engine+game merge attempt with C++ server/client, full engine module set,
editor panels, game data, Python tools, TLA+ specs, sample projects, and BlenderSpaceshipGenerator).

| Task | Source Path | Target Path | Files | Status |
|------|------------|-------------|-------|--------|
| 4B | `engine/` (40 module dirs) | `Source/Engine/src/` | 241 | ✅ Extracted |
| 4C | `editor/` (panels/tools/ai/assistant/ui) | `Source/Editor/src/` | 89 | ✅ Extracted |
| 4D | `cpp_client/src+include` | `Source/Game/src/Client/` + `include/NF/Game/Client/` | 244 | ✅ Extracted |
| 4E | `data/` (20 game data dirs) | `Data/` | 95 | ✅ Extracted |
| 4F | `schemas/` (5 JSON schemas) | `Schemas/` | 5 | ✅ Extracted |
| 4G | `specs/` (3 TLA+ specs) | `Docs/Atlas-NovaForge/specs/` | 3 | ✅ Extracted |
| 4H | `tools/` (Python + BlenderSpaceshipGenerator) | `Tools/Atlas-NovaForge/` + `Tools/BlenderGenerator/` | 52 | ✅ Extracted |
| 4I | `projects/` (4 sample projects) | `Project/samples/` | 130 | ✅ Extracted |
| 4J | `docs/` (~102 docs) | `Docs/Atlas-NovaForge/` | 104 | ✅ Extracted |
| 4K | `tests/` + `atlas_tests/` | `Tests/Atlas-NovaForge/` | 306 | ✅ Extracted |
| 4L | superseded + usable snippets | `Archive/_Atlas-NovaForge/` | 18 | ✅ Archived |

**Total extracted: 1,264 files** (after deduplication with Phase 3)

**Note:** Extracted files use `atlas::` namespace conventions. Not yet compiled into tempnovaforge build.
Build verification passed — 745 tests, no regressions.

## Phase 5 Extraction (AtlasForge)

Phase 5 performed a targeted extraction from `shifty81/AtlasForge` (~718 files, original engine repo).
AtlasForge is the primary origin repo already consumed by Phases 3–4. Only unique
files not present from prior phases were extracted to active paths.

| Task | Source Path | Target Path | Files | Status |
|------|------------|-------------|-------|--------|
| 5A | `engine/script/ScriptSandbox.*` | `Source/Engine/src/Script/` | 2 | ✅ Extracted |
| 5B | `engine/script/ScriptSystem.*` | `Source/Engine/src/Script/` | 2 | ✅ Extracted |
| 5C | `engine/abi/` | `Source/Engine/src/ABI/` | 4 | ✅ Extracted |
| 5D | `tileeditor/` | `Source/Programs/TileEditor/` | 2 | ✅ Extracted |
| 5E | `assets/fonts/builtin_fallback.json` | `Content/Assets/Fonts/` | 1 | ✅ Extracted |
| 5F | Full repo reference | `Archive/_AtlasForge/usable_snippets/` | 10 | ✅ Archived |
| 5G | `README.md`, `CONTRIBUTING.md` | `Archive/_AtlasForge/docs_archive/` | 2 | ✅ Archived |

**Total extracted: 11 unique files + 12 archived references**

Remaining ~685 files are superseded by prior phase extractions (engine, editor, server,
tests, docs, schemas, specs all already present from Phases 3–4).

## Phase 6 Extraction (NovaForge-Project)

Phase 6 extracted game project structure, governance docs, specs, scripts, and configs
from `shifty81/NovaForge-Project`.

| Task | Source Path | Target Path | Files | Status |
|------|------------|-------------|-------|--------|
| 6A | Root docs + specs | `Docs/NovaForge-Project/` | 21 | ✅ Extracted |
| 6B | Scripts | `Archive/_NovaForge-Project/scripts/` | 9 | ✅ Archived |
| 6C | Tests | `Archive/_NovaForge-Project/tests/` | 3 | ✅ Archived |
| 6D | Configs (manifest, CI, packaging) | `Archive/_NovaForge-Project/` | 7 | ✅ Archived |

**Total extracted: 21 docs + 19 archived references**

## Phases 7–9 Extraction (AI Repos)

Phases 7–9 extracted three AI repositories into canonical AtlasAI locations.

| Phase | Repo | Usable Snippets | Docs Archived | Status |
|-------|------|----------------|---------------|--------|
| 7 | SwissAgent | 30 → `AtlasAI/Atlas_SwissAgent/usable_snippets/` | 6 → `Archive/_SwissAgent/docs_archive/` | ✅ Extracted |
| 8 | ArbiterAI | 3 → `Archive/_ArbiterAI/usable_snippets/` | 1 → `Archive/_ArbiterAI/docs_archive/` | ✅ Extracted |
| 9 | Arbiter | 10 → `AtlasAI/Atlas_Arbiter/usable_snippets/` | 4 → `Archive/_Arbiter/docs_archive/` | ✅ Extracted |

## Phase 10 Extraction (AtlasForge-EveOffline)

Phase 10 extracted the networking prototype documentation and reference code.

| Task | Source Path | Target Path | Files | Status |
|------|------------|-------------|-------|--------|
| 10A | Docs (full tree) | `Docs/AtlasForge-EveOffline/` | 56 | ✅ Extracted |
| 10B | Code/config references | `Archive/_AtlasForge-EveOffline/` | 11 | ✅ Archived |

## Phase 11 Extraction (Blender-Generator-for-AtlasForge)

Phase 11 archived the original Blender addon source separately from Phase 4 content.

| Task | Source Path | Target Path | Files | Status |
|------|------------|-------------|-------|--------|
| 11A | Python addon + docs | `Archive/_Blender-Generator-for-AtlasForge/` | 31 | ✅ Archived |

## Archive Protocol

When a repo is fully merged:
1. Create `Archive/_RepoName/ARCHIVE_SUMMARY.md`
2. Create `Archive/_RepoName/usable_snippets/` with extracted code snippets
3. Create `Archive/_RepoName/docs_archive/` with all original docs
4. Push to tempnovaforge
5. User pulls locally then removes/archives the source repo on GitHub

## UI Panel Migration (U1–U8)

All legacy `EditorPanel`-derived panels have been migrated to the AtlasUI framework.
New implementations live under `Source/UI/include/NF/UI/AtlasUI/Panels/` and use
`NF::UI::AtlasUI::PanelBase` as their base class.

| Phase | Panel | AtlasUI Class | Panel ID | Status |
|-------|-------|---------------|----------|--------|
| U1 | InspectorPanel | `InspectorPanel` | `atlas.inspector` | ✅ Done |
| U2 | HierarchyPanel | `HierarchyPanel` | `atlas.hierarchy` | ✅ Done |
| U3 | ContentBrowserPanel | `ContentBrowserPanel` | `atlas.content_browser` | ✅ Done |
| U4 | ConsolePanel | `ConsolePanel` | `atlas.console` | ✅ Done |
| U5 | IDEPanel | `IDEPanel` | `atlas.ide` | ✅ Done |
| U6 | GraphEditorPanel | `GraphEditorPanel` | `atlas.graph_editor` | ✅ Done (shell) |
| U7 | ViewportPanel | `ViewportPanel` | `atlas.viewport` | ✅ Done (shell) |
| U8 | Legacy deprecation | — | — | ✅ Done |

Legacy `EditorPanel` classes in `Source/Editor/include/NF/Editor/Editor.h` are marked
as DEPRECATED with comments pointing to the new AtlasUI implementations.

## Archive Inventory

| Archive Directory | Phase | Status |
|-------------------|-------|--------|
| `Archive/_MasterRepo/` | C0 | ✅ Archived |
| `Archive/_MasterRepoRefactor/` | C1 | ✅ Archived |
| `Archive/_AtlasToolingSuite/` | C2 | ✅ Archived |
| `Archive/_Nova-Forge-Expeditions/` | 3 | ✅ Extracted |
| `Archive/_Atlas-NovaForge/` | 4 | ✅ Extracted |
| `Archive/_AtlasForge/` | 5 | ✅ Extracted |
| `Archive/_NovaForge-Project/` | 6 | ✅ Extracted |
| `Archive/_SwissAgent/` | 7 | ✅ Extracted |
| `Archive/_ArbiterAI/` | 8 | ✅ Extracted |
| `Archive/_Arbiter/` | 9 | ✅ Extracted |
| `Archive/_AtlasForge-EveOffline/` | 10 | ✅ Extracted |
| `Archive/_Blender-Generator-for-AtlasForge/` | 11 | ✅ Extracted |

## Naming Convention

All AI systems are renamed to `Atlas_` prefix and live under `AtlasAI/`:
- Arbiter → Atlas_Arbiter
- ArbiterAI → merged into Atlas_Arbiter  
- SwissAgent → Atlas_SwissAgent
- All AI functionality routes through the AtlasAI broker

## Canonical Directory Locations

| System | Canonical Path | Legacy Paths (deprecated) |
|--------|---------------|--------------------------|
| AtlasAI Broker | `AtlasAI/` | `Tools/AtlasAI/`, `Tools/ArbiterAI/`, `Tools/SwissAgent/` |
| Atlas_Arbiter | `AtlasAI/Atlas_Arbiter/` | `Tools/ArbiterAI/` |
| Atlas_SwissAgent | `AtlasAI/Atlas_SwissAgent/` | `Tools/SwissAgent/` |
| AtlasUI Framework | `Source/UI/include/NF/UI/AtlasUI/` | `Additions/atlasui_widget_kit_starter_patch/` |
| Spec Rollup | `Docs/SpecRollup/` | `Additions/atlas_workspace_novaforge_spec_rollup_pack/` |
| BlenderGenerator | `Tools/BlenderGenerator/` | (future: `Tools/Atlas_BlenderGen/`) |
| ContractScanner | `Tools/ContractScanner/` | — |
| ReplayMinimizer | `Tools/ReplayMinimizer/` | — |

## Spaghetti Audit Flags

- [x] Duplicate test dirs: `tests/`, `testing/`, `atlas_tests/` → consolidated to `Tests/`
- [x] Mixed case dirs: `tools/` vs `Tools/` → normalized to `Tools/`
- [x] `My Repos.rtf` at root → moved to `Archive/`
- [x] Inconsistent AI naming → all `Atlas_` prefix, `AtlasAI/` canonical location
- [x] Duplicate AtlasAI directories (`Tools/AtlasAI/` vs `AtlasAI/`) → consolidated to root `AtlasAI/`
- [x] Legacy tool READMEs → updated with deprecation markers
- [ ] Intake/DropBox at root → move to `Tools/Atlas_DevTools/` (deferred to workspace phase)
- [x] Loose oversized .md files at root (SwissAgent) → moved to `AtlasAI/Atlas_SwissAgent/` (Phase 7)
- [ ] Loose .zip at root (tempnovaforge) → move to `Archive/` (none found — resolved)
- [ ] `From old repo/` + `legacy/` + `archive/` in active dirs → move to `Archive/` (none found — resolved)
