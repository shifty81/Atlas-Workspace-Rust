# NovaForge — Master Convergence Roadmap
**Last Updated:** 2026-04-05
**Active Repo:** `shifty81/tempnovaforge`
**Strategy:** All repos converged into `tempnovaforge` (C3 complete). Active work is now implementation — editor systems, workspace pipeline, game loop, and AI integration. Standalone repos re-extracted post Phase 5.

---

## Naming Canon

| Old Name         | New Name               | Canonical Location        |
|------------------|------------------------|---------------------------|
| ArbiterAI        | Atlas_Arbiter          | `AtlasAI/Atlas_Arbiter/`  |
| Arbiter          | Atlas_Arbiter          | `AtlasAI/Atlas_Arbiter/`  |
| SwissAgent       | Atlas_SwissAgent       | `AtlasAI/Atlas_SwissAgent/` |
| BlenderGenerator | Atlas_BlenderGenerator | `Tools/BlenderGenerator/` |
| ContractScanner  | Atlas_ContractScanner  | `Tools/ContractScanner/`  |
| ReplayMinimizer  | Atlas_ReplayMinimizer  | `Tools/ReplayMinimizer/`  |

> ⚠️ `Tools/AtlasAI/`, `Tools/ArbiterAI/`, `Tools/SwissAgent/` are **deprecated** legacy paths.
> Canonical AI broker location is `AtlasAI/` at repo root.

All AI features: prefix `Atlas_` or namespace `AtlasAI::`
All engine C++ classes: prefix `NF` (NovaForge) or `Atlas`
All workspace runtime subdirs: use canonical names (`Atlas_Arbiter`, `Atlas_SwissAgent`)

---

## Phase 0 — Bootstrap & Convergence Setup ✅ COMPLETE

- [x] `tempnovaforge` base structure established
- [x] CMakeLists.txt + CMakePresets.json scaffolded
- [x] vcpkg.json base created
- [x] Docs structure created (Architecture, AI, Engine, Editor, Game, Systems, Design)
- [x] Tools stubs created (ArbiterAI→AtlasAI, SwissAgent, BlenderGenerator, ContractScanner, ReplayMinimizer)
- [x] All 12 source repos extracted and archived (Phases C0–C3, 3–11)
- [x] AtlasUI panel framework established (PanelBase + 7 migrated panel shells)
- [x] Programs structure defined (NovaForgeEditor, NovaForgeGame, NovaForgeServer, TileEditor)
- [x] AtlasAI canonical location consolidated to `AtlasAI/` root
- [x] Legacy tool paths deprecated (Tools/AtlasAI, Tools/ArbiterAI, Tools/SwissAgent)

---

## Phase 1 — Repo Alignment & Naming Cleanup ✅ COMPLETE (with outstanding items)

All source repos migrated. Outstanding cleanup items carried forward to Phase 1-B.

### Phase 1-A — Source Migration ✅ COMPLETE
- [x] Source/Core migrated and spaghetti-audited
- [x] Source/Engine migrated (40 module dirs, ~290 files)
- [x] Source/Renderer migrated
- [x] Source/Audio migrated
- [x] Source/Physics migrated
- [x] Source/Input migrated
- [x] Source/Networking migrated
- [x] Source/Animation migrated
- [x] Source/UI migrated (AtlasUI framework + widget kit)
- [x] Source/AI stubs created
- [x] Source/Editor migrated (panels, tools, AI, assistant, UI subdirs)
- [x] Source/Programs migrated (NovaForgeEditor, NovaForgeGame, NovaForgeServer, TileEditor)
- [x] Source/GraphVM migrated
- [x] Tools/AtlasAI/Atlas_Arbiter source migrated → `AtlasAI/Atlas_Arbiter/`
- [x] Tools/AtlasAI/Atlas_SwissAgent source migrated → `AtlasAI/Atlas_SwissAgent/`
- [x] Tools/BlenderGenerator migrated
- [x] Editor build target present in CMakeLists.txt

### Phase 1-B — Naming & Stale Reference Cleanup 🔲 TODO
- [ ] Rename `Atlas/Workspace/Arbiter/` → `Atlas/Workspace/Atlas_Arbiter/`
- [ ] Rename `Atlas/Workspace/SwissAgent/` → `Atlas/Workspace/Atlas_SwissAgent/`
- [ ] Fix `Tools/AtlasAI/ATLAS_AI_OVERVIEW.md` — update stale path references to point to `AtlasAI/`
- [ ] Update Naming Canon table above (already done in this file)
- [ ] Remove ImGui dep from CMakeLists.txt (editor uses custom NF::UIRenderer, no ImGui)
- [ ] Consolidate `AssetBrowserPanel` into `ContentBrowserPanel` (duplicate)
- [ ] Consolidate `InteractionDebugger` into `InteractionDebugPanel` (duplicate)
- [ ] Expand vcpkg.json with all engine deps (Vulkan, OpenAL, Bullet3, Catch2, nlohmann-json, stb)

---

## Phase 2 — Refactor & Spaghetti Audit

- [ ] Full codebase audit: identify circular dependencies
- [ ] Enforce layering: Engine → Game → Editor (no reverse deps)
- [ ] Naming pass: all AI symbols renamed to `AtlasAI::` / `Atlas_` prefix throughout Source/
- [ ] Namespace migration: `atlas::` → `NF::` for all extracted Phase 3–4 files compiled into build
- [ ] Remove dead code identified in audit
- [ ] CMake module graph cleaned up
- [ ] All Python tooling confirmed in Tools/ (none in Source/)
- [ ] GLSL shaders moved to Content/Shaders/

---

## 🎯 Milestone 1 — Editor-Game Loop Operational

> **Goal:** Open workspace → select NovaForge project → use any editor → save changes →
> relaunch editor OR see changes live in the running game client.
> This is the MasterRepoV001 checkpoint brought fully operational in tempnovaforge.

### M1-A — Repo Alignment (prerequisite, feeds into Phase 1-B above)
- [ ] All naming cleanup from Phase 1-B complete
- [ ] `Editor.cpp` (currently 62 bytes) expanded to full `NF::EditorApp` init
- [ ] `Source/Programs/NovaForgeEditor/main.cpp` wired: Win32 window → WGL → `NF::EditorApp` loop

### M1-B — Core Editor Panels (7-panel layout fully rendering)
- [ ] `ViewportPanel` — wire `EditorViewportFramebuffer` + `ViewportCameraController`
  - FPS camera: hold right-click to activate, WASD move, Q/E vertical, Shift sprint
  - Gizmo overlay (translate/rotate/scale)
  - This is the **standard rendering viewport** — all 3D editors inherit `NFRenderViewport` base
- [ ] `NFRenderViewport` base class created — all rendering editors inherit for standardized FPS cam
- [ ] `HierarchyPanel` — scene entity tree, select/rename/reparent entities
- [ ] `InspectorPanel` — property editor wired to `PropertyEditor` + `CommandStack`
- [ ] `ContentBrowserPanel` — directory scan of `Content/`, drag-drop to viewport/inspector
- [ ] `ConsolePanel` — already implemented ✅, verify wiring to engine Logger
- [ ] `GraphEditorPanel` — node/pin rendering, drag-to-connect, compile → `GraphVM::compile()`
- [ ] `IDEPanel` — `ProjectIndexer` scan of `Source/`, symbol search, go-to-definition

### M1-C — Asset Editors (full 3D viewport via NFRenderViewport) ✅
- [x] `NFRenderViewport` base class — standardized FPS cam, gizmo, render modes for all 3D editors
- [x] `MeshViewerPanel` — orbit + FPS cam, wireframe/solid/lit toggle
- [x] `MaterialEditorPanel` — PBR property sliders + live preview in `NFRenderViewport`
- [x] `SkeletalEditorPanel` — **NEW** — FPS viewport + bone hierarchy tree + weight paint mode
  - Opening a skeletal asset from workspace prompt opens this panel with the file pre-loaded
- [x] `AnimationEditorPanel` — **NEW** — timeline, keyframe editor, blend tree view + `NFRenderViewport`
- [x] `ShipEditorPanel` — wired to `NFRenderViewport` base
- [x] `CharacterEditorPanel` — wired to `NFRenderViewport` base
- [x] `PrefabEditorPanel` — scene-in-a-box, wired to `NFRenderViewport`

### M1-D — Project Workspace & Selector
- [ ] `ProjectPickerPanel` — scan `Project/` dir, list `.atlas.json` projects, select to load
- [ ] `ProjectPathService` wired — `contentPath()`, `dataPath()`, `configPath()`, `savePath()`
- [ ] NovaForge selectable as a project in the workspace launcher
- [ ] Opening workspace auto-shows project picker, selecting project loads all editor state

### M1-E — Play In Editor (PIE) Mode
- [ ] `PlayInEditor` tool implemented
  - Launch `NovaForgeGame` process from editor toolbar button
  - Feed current scene state to the game client on launch
  - Stop PIE terminates the game process, restores editor state
- [ ] PIE toolbar button wired in `EditorCommandRegistry` (`pie.play`, `pie.stop`, `pie.pause`)
- [ ] Scene serialization to temp file on PIE launch (used by game client to load editor scene)

### M1-F — Live Game Mirror (dev solar system)
- [ ] `LiveUpdateService` implemented
  - File-watch on `Content/` and `Data/` directories
  - On save: serialize asset diff → push to running game client via named pipe / local socket
  - Game client in dev mode connects on startup and applies incoming diffs in real-time
- [ ] Game client dev mode: on launch checks for `--dev` flag, connects to `LiveUpdateService`
- [ ] Dev solar system: game client mirrors the active world scene live during development

### M1-G — AI Workspace Workflow (Atlas Assistant → Editor routing)
- [ ] `.novaforge/pipeline/changes/` directory scaffolded with `manifest.json`
- [ ] `AtlasAssistantPanel` wired to `AtlasAI::SwissAgent` via pipeline ChangeEvents
- [ ] Prompt routing: user types in `AtlasAssistantPanel` → pipeline event → SwissAgent classifies → opens correct editor
  - Example: "modify skeletal of ShipHull_01" → `SkeletalEditorPanel::openAsset("ShipHull_01")`
  - Example: "edit material on AsteroidRock" → `MaterialEditorPanel::openAsset("AsteroidRock")`
  - Example: "open animation for PlayerRun" → `AnimationEditorPanel::openAsset("PlayerRun")`
- [ ] `Atlas_Arbiter` rules wired: asset-type → editor-type routing table
- [ ] Save in any editor → pipeline ChangeEvent → `LiveUpdateService` pushes to game client

---

## Phase 3 — PCG & Voxel Runtime

- [ ] Voxel chunk system implemented (`Source/World/`)
- [ ] Heightmap + biome pass
- [ ] Structure generator
- [ ] PCG loot tables
- [ ] Integration with `Atlas_BlenderGenerator` for spaceship spawning
- [ ] `TerrainEditorPanel` — **NEW** — heightmap paint, biome brush, in editor viewport
- [ ] `WorldGraphPanel` — fully implemented (currently 1.2KB stub)

---

## Phase 4 — AtlasAI Runtime Integration

- [ ] `Source/AI/` C++ stubs created for all planned modules
- [ ] `Atlas_NPCController` wired into game loop
- [ ] `Atlas_PathfindingSystem` (navmesh or flow field)
- [ ] `Atlas_BehaviourTree`
- [ ] `Atlas_Dialogue` (LLM-assisted, optional)
- [ ] `AtlasAI/` broker connected to engine via IPC or embedded Python
- [ ] `AIDebuggerPanel` fully wired to live runtime AI state ✅ (already 7.6KB — verify wiring)

---

## Phase 4-B — Visual Scripting & Graph System (fleshed out)

> The GraphVM and 14 graph types are extracted. This phase wires them all through the editor.

- [ ] `GraphEditorPanel` supports all 14 graph types with per-type node palettes:
  - World gen graph, Animation graph, AI graph, Audio graph, UI graph
  - Character graph, Weapon graph, Tile graph, Story graph, Strategy graph
  - Flow graph, Interaction graph, Sound graph, Procedural material graph
- [ ] Per-graph-type routing: asset type determines which graph palette loads in `GraphEditorPanel`
- [ ] `GraphVM::compile()` → `GraphVM::execute()` round-trip tested end-to-end
- [ ] Hot-reload: recompile graph → push bytecode to running PIE/live session without restart
- [ ] `RuleGraphEditorPanel` — fully wired to `Atlas_Arbiter` rule engine
- [ ] `WorldGraphPanel` — fully wired to world gen graph pipeline
- [ ] Node visual polish: typed pins, color-coded node categories, minimap

---

## Phase 4-C — Additional Editor Systems

- [ ] `SoundEditorPanel` — **NEW** — audio asset editor, waveform view, loop/trim
- [ ] `ParticleEditorPanel` — **NEW** — VFX / particle system editor with live preview
- [ ] `QuestEditorPanel` — fully implemented (currently 4.5KB partial)
- [ ] `InventoryEditorPanel` — fully implemented (currently 4KB partial)
- [ ] `TilePalettePanel` — fully implemented (currently 4.3KB partial)

---

## Phase 5 — Archive & Cleanup ✅ IN PROGRESS

All source repos extracted. Archive stubs created. Final archive steps remain.

### Archive Checklist

| Repo                                      | Migrated | Archived |
|-------------------------------------------|----------|----------|
| shifty81/Atlas-NovaForge                  | [x]      | [x]      |
| shifty81/AtlasForge                       | [x]      | [x]      |
| shifty81/MasterRepo                       | [x]      | [x]      |
| shifty81/AtlasToolingSuite                | [x]      | [x]      |
| shifty81/MasterRepoRefactor               | [x]      | [x]      |
| shifty81/SwissAgent                       | [x]      | [x]      |
| shifty81/Nova-Forge-Expeditions           | [x]      | [x]      |
| shifty81/NovaForge-Project                | [x]      | [x]      |
| shifty81/AtlasForge-EveOffline            | [x]      | [x]      |
| shifty81/Blender-Generator-for-AtlasForge | [x]      | [x]      |
| shifty81/ArbiterAI                        | [x]      | [x]      |
| shifty81/Arbiter                          | [x]      | [x]      |

### Remaining Archive Tasks
- [ ] Update each source repo's GitHub description to "ARCHIVED — merged into shifty81/tempnovaforge"
- [ ] Remove local clones once description updated

---

## Phase 6 — Standalone Extraction (Post-Convergence)

Once tempnovaforge is complete, consider extracting standalone repos for:
- `AtlasEngine` — pure engine, no game
- `AtlasAI` — standalone AI tooling suite
- `NovaForge` — the game itself (uses AtlasEngine)

These are **future decisions only**. Do not create these repos until Phase 5 archive tasks are done.

---

## Gap Analysis — Missing Systems

| Gap                                    | Priority | Phase    | Notes                                                        |
|----------------------------------------|----------|----------|--------------------------------------------------------------|
| `Editor.cpp` is 62-byte stub           | HIGH     | M1-A     | Full `NF::EditorApp` init needed                             |
| `ViewportPanel` is shell only          | HIGH     | M1-B     | Needs framebuffer + FPS cam wiring                           |
| `NFRenderViewport` base class done   | ✅       | M1-B/C   | All 3D editors now inherit standardized FPS viewport         |
| FPS camera not universal               | HIGH     | M1-B     | Right-click+WASD must be standard for all rendering editors  |
| PIE mode not implemented               | HIGH     | M1-E     | PlayInEditor tool exists but not wired                       |
| Live game mirror missing               | HIGH     | M1-F     | No LiveUpdateService or file-watch mechanism                 |
| `SkeletalEditorPanel` done           | ✅       | M1-C     | Implemented with NFRenderViewport base                       |
| `AnimationEditorPanel` done          | ✅       | M1-C     | Implemented with NFRenderViewport base                       |
| `GraphEditorPanel` is shell            | HIGH     | M1-B     | No node/pin rendering or GraphVM compile wiring              |
| `ProjectPickerPanel` is stub           | HIGH     | M1-D     | 1.4KB, not functional                                        |
| `.novaforge/pipeline/` empty           | HIGH     | M1-G     | No ChangeEvent infrastructure                                |
| `AtlasAssistantPanel` not wired        | HIGH     | M1-G     | No SwissAgent routing                                        |
| `Source/AI/` completely empty          | MED      | Phase 4  | No C++ AI runtime stubs                                      |
| `WorldGraphPanel` is stub              | MED      | Phase 3  | 1.2KB, not functional                                        |
| `MaterialEditorPanel` enhanced       | ✅       | M1-C     | Now includes NFRenderViewport + live preview + PBR sliders   |
| `SoundEditorPanel` missing             | MED      | Phase 4-C| Not yet created                                              |
| `ParticleEditorPanel` missing          | MED      | Phase 4-C| Not yet created                                              |
| `TerrainEditorPanel` missing           | MED      | Phase 3  | Not yet created                                              |
| 14-graph-type routing missing          | MED      | Phase 4-B| GraphEditorPanel has no per-type palette system              |
| Save/Load system                       | HIGH     | M1-F     | Serialization needed for PIE + LiveUpdate                    |
| Asset pipeline (cook + package)        | HIGH     | Phase 5  | No asset cooker, raw assets only                             |
| vcpkg.json not fully expanded          | HIGH     | Phase 1-B| Missing Vulkan, Bullet3, OpenAL, Catch2, stb, nlohmann-json  |
| ImGui in CMakeLists (wrong)            | HIGH     | Phase 1-B| Editor uses custom NF::UIRenderer — remove ImGui dep         |
| Workspace/Arbiter dir uses old name    | MED      | Phase 1-B| Rename to Atlas_Arbiter                                      |
| Workspace/SwissAgent dir uses old name | MED      | Phase 1-B| Rename to Atlas_SwissAgent                                   |
| Duplicate: AssetBrowserPanel           | LOW      | Phase 1-B| Consolidate into ContentBrowserPanel                         |
| Duplicate: InteractionDebugger         | LOW      | Phase 1-B| Consolidate into InteractionDebugPanel                       |
| CI/CD pipeline                         | MED      | Phase 2  | .github/ exists but no workflow files                        |
| Automated test harness                 | MED      | Phase 2  | Tests/ dir populated but not wired to CMake/CI               |
| Dedicated server build target          | MED      | Phase 4  | Networking exists, no server binary target yet               |
| Localization system                    | LOW      | Phase 5+ | No i18n layer                                                |
| Crash reporter                         | MED      | Phase 5  | crash_reporter.py exists in archive, not wired               |
| Mod support                            | LOW      | Phase 6+ | Planned post-launch                                          |
| Console platform layer                 | LOW      | Phase 6+ | PC-first, console deferred                                   
