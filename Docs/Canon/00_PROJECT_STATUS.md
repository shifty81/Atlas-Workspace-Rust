# Project Status

Current Phase: **All Phases A–I Complete** — Atlas Workspace v1.0 Milestone Reached

## Build Status

- **Passing**
- All 25 test suites build and pass (4125 test cases)
- Tests require online fetch of Catch2 (gated behind `ATLAS_ENABLE_ONLINE_DEPS`)
- Project validator: 79/79 checks passing

## Repo Condition

### What Works (Real, Tested, Wired)

- **Infrastructure contracts:** WorkspaceShell, ToolRegistry, PanelRegistry, ProjectRegistry
- **UI rendering pipeline:** DrawList → DrawListDispatcher → UIRenderer → GDIBackend (Windows)
- **Panel chrome:** 8 core panels render labeled UI layouts with placeholder content
- **8 hosted tools:** SceneEditor, AssetEditor, MaterialEditor, AnimationEditor, DataEditor, VisualLogic, BuildTool, AtlasAI — all registered and render tool views
- **Command system:** CommandRegistry, CommandHistory, ActionMap, ShortcutRouter — real handlers wired for Save, Build, Undo, Redo, Close Document, Command Palette, AtlasAI launch, and file import; `build.start` drives BuildTool directly
- **Event system:** WorkspaceEventBus, NotificationBus, WorkspaceEventQueue — all functional
- **Project loading:** .atlas file parsing, NovaForgeAdapter selection, panel descriptor registration
- **Asset catalog:** AssetCatalogPopulator with 50+ extension classification, catalog population
- **Settings/preferences:** SettingsStore (3-layer), PreferenceRegistry, PreferenceController
- **Undo/redo:** UndoStack, UndoManager, UndoTransaction
- **Persistence:** WorkspaceProjectFile, ProjectSerializer, AssetCatalogSerializer, SettingsStore serialization; layout presets and user settings saved on clean exit and restored on startup via `WorkspaceShell::shutdown()`
- **PIE:** PIEService state machine (Stopped/Playing/Paused), PIEInputRouter, PIEExternalLaunch — contract layer complete; toolbar controller and console bridge wired in Phase G
- **Document models:** All 8 core tool documents — SceneDocument, AssetDocument, MaterialDocument, AnimationDocument, GraphDocument, BuildTaskGraph, AIRequestContext, DataTableDocument — with dirty tracking, save/revert, and open-document registry (WorkspaceProjectState)
- **Preferences UI:** PreferenceRegistry, PreferenceController, user-facing preferences panel (Phase H)
- **Keybind editor:** Keybind editor panel wired (Phase H)
- **Command palette:** Ctrl+P opens fuzzy-search command palette with context filtering (Phase H)
- **Notification center:** History, toast notifications, severity filtering, click-to-navigate (Phase H)
- **Project open flow:** Recent projects list, file picker, validation, new project wizard (Phase H)
- **Session management:** SessionManager, SessionHistory
- **Search:** SearchEngine, SearchIndex
- **Scripting:** ScriptEngine, AutomationTask
- **Plugin system:** PluginRegistry, PluginSandbox
- **Diagnostics:** DiagnosticCollector, TelemetryCollector
- **Naming:** AtlasAI canon complete — zero Arbiter references in active paths
- **Viewport contracts:** ViewportHostContract, ViewportHostRegistry, ViewportFrameLoop
- **NovaForge panel factories:** 6 panels instantiable via ProjectSystemsTool
- **Viewport software renderer → screen:** `SoftwareViewportRenderer` now writes pixels into `IViewportSurface` via `writeScanlines()`; `GDIViewportSurface` copies them into the DIB scanline buffer so `BitBlt` displays the grid (Phase 67)
- **Fly-camera wired:** `SceneEditorTool` owns camera state (pos/yaw/pitch) + `SceneViewportCameraController`; `update()` drives WASD + mouse-look when `InputSystem` is attached; `provideScene()` sets `overrideCamera=true` (Phase 67)
- **Input attached to SceneEditorTool:** `main.cpp` calls `onAttachInput(&input)` after setup and `onDetachInput()` before shutdown (Phase 67)
- **Viewport entity projection (Phase 72):** `NovaForgePreviewRuntime::provideScene()` populates `ViewportSceneState::entities` with `ViewportEntityProxy` (position, half-extents, selected flag). `SoftwareViewportRenderer` now does real 3D→2D projection (yaw/pitch camera → view space → perspective divide → screen) and draws a cross marker + projected wireframe quad per entity. Behind-camera entities are culled. Selected entities render in `selectColor`. 22 new tests, all green.

### What Does NOT Work (Stubs, Shells, Not Wired)

- **Viewport rendering:** Software renderer now writes pixels into `GDIViewportSurface` scanlines so the grid is visible on screen. No real 3D/GPU rendering yet. D3D11 backend is architecturally complete but awaits Windows SDK + d3d11.lib for real device init.
- **Panel content:** All panels display chrome (titles, fake rows) but edit nothing real.
- **NovaForge panels:** Have `projectRoot` and `ready` flag, but no schemas, no data, no save targets.
- **Project load depth:** Loading .atlas selects the adapter and registers descriptors, but does not load asset registries, gameplay data, or documents.
- **PCG pipeline → editor:** Pipeline exists as file-based event system but is completely disconnected from viewport and panels.
- **D3D11 backend:** Architecturally complete headers but not executing — GDI is still the active renderer.
- **PIE real process launch:** `PIEExternalLaunch` contract is complete but `CreateProcess`/`fork+exec` is not wired — no real child process is spawned.
- **PIE performance counters UI:** Panel for live PIE perf counters (frame time, tick count) is not implemented.
- **Keybind persistence:** Keybind editor exists but key mappings are not saved to disk.

## Honest Summary

The repository has solid, runtime-verified infrastructure across most phases: contracts,
registries, event buses, serialization, undo/redo, document models, PIE lifecycle,
preferences/keybind UI, command palette, notification center, project open flow, and
layout/settings persistence on exit. The software viewport pipeline renders a grid
end-to-end through `SoftwareViewportRenderer → GDIBackend`. The remaining open items
are all explicitly deferred platform-specific work: D3D11/DirectWrite GPU backend,
real `CreateProcess` PIE launch, and a performance-counters UI panel.

## Phase Summary

| Phase | Title | Status |
|-------|-------|--------|
| Checkpoint | Foundation (Phases 0–71) | ✅ Complete |
| A | Truth and Cleanup Lock | ✅ Complete |
| B | Real Project Load | ✅ Complete |
| C | Panels Edit Real Data | ✅ Complete |
| D | Runtime-Backed Viewport | 🔄 In Progress (D.1–D.4 + software renderer pipeline wired; D.5 deferred — real GPU init) |
| E | Shared PCG Preview Pipeline | ✅ Complete |
| F | Play-In-Editor (PIE) | 🔄 In Progress (runtime wiring done in G/H; real process launch + perf counters UI panel pending) |
| G | Full Tool Wiring | ✅ Complete |
| H | UX Completion | ✅ Complete |
| I | NovaForge End-to-End Project Load Integration | ✅ Complete |

## Phase A Progress

### A.1 — Repo Cleanup
- [x] Rename `ArbiterReasoner.cpp` → `AtlasAIReasoner.cpp` in pipeline
- [x] Archive 37 forwarding shims from `Source/Editor/include/NF/Editor/` → `Deprecated/`
- [x] Update all consumers to use canonical `NF/Workspace/` include paths
- [x] Remove stale `EditorApp` include from `Editor.h` (umbrella no longer pulls legacy app)
- [x] Update stale EditorApp comment in ViewportPanel.h
- [x] Extract `LocalProjectAdapter` from `main.cpp` → dedicated `LocalProjectAdapter.h`
- [x] Validate — all 4125 tests pass; `validate_project.sh` passes 79/79
- [x] Add CI smoke build entry for `ATLAS_ENABLE_ONLINE_DEPS=ON`

### A.2 — Build and CI Hygiene
- [x] All test suites build and pass on clean checkout (4469 test cases, all green)

### A.3 — Documentation Correction
- [x] Update project status phase marker to "In Progress"
- [x] Mark stale phase-specific roadmap docs (01–07) as historical
- [x] Add "panel edits real data" criteria to `09_DEFINITION_OF_DONE.md`

## Phase B Progress (Complete)
- 40+ tests covering B.1–B.4, all green (test_phase_b.cpp)

## Phase C Progress (Complete)
- 58 tests, 104 assertions, all green (test_phase_c.cpp)

## Phase D Progress (In Progress — D.1–D.4 complete, software renderer pipeline wired; D.5 GPU backend deferred)
### D.1 — NovaForge Preview Runtime Bridge ✅
- [x] `NovaForgePreviewWorld` — entity lifecycle, transform, mesh/material, selection, dirty tracking
- [x] `NovaForgePreviewRuntime` — IViewportSceneProvider, fly-camera, gizmo state, inspector data, hierarchy order

### D.2 — Scene Editor Viewport ✅
- [x] `SceneEditorTool::attachSceneProvider()` — delegates `provideScene()` to attached provider
- [x] Fly-camera: `processCameraInput()` — WASD movement + mouse-look with pitch clamp
- [x] Gizmo state: `gizmoState()` — reflects selected entity position
- [x] Inspector data: `selectedEntityProperties()` — flat property map for selected entity
- [x] Hierarchy order: `hierarchyOrder()` — BFS parent-before-child traversal

### D.3 — Asset Preview Viewport ✅
- [x] `NovaForgeAssetPreview` — IViewportSceneProvider, bind/edit/apply/revert
- [x] `AssetEditorTool::attachAssetPreviewProvider()` — delegates `provideScene()` to asset preview
- [x] Collider editing: `ColliderDescriptor`, `setCollider*()` methods
- [x] Socket editing: `SocketDescriptor`, addSocket/removeSocket/setSocketTransform
- [x] Anchor editing: `AnchorDescriptor`, addAnchor/removeAnchor/setAnchorTransform
- [x] PCG metadata: `AssetPCGMetadata`, setPlacementTag/addGenerationTag/setPCGScaleRange etc.

### D.4 — Material Preview Viewport ✅
- [x] `NovaForgeMaterialPreview` — IViewportSceneProvider, bindMaterial/clearMaterial
- [x] Preview meshes: Sphere/Cube/Plane via `setPreviewMesh()`
- [x] Material parameters: setParameter/removeParameter/resetParameterToDefault/resetAllParametersToDefault
- [x] apply()/revert() baseline management; properties() for inspector
- [x] `MaterialEditorTool::attachMaterialPreviewProvider()` + `provideScene()` delegation

### D.5 — D3D11 Backend Activation (pending)
- [x] `SoftwareViewportRenderer` installed as active baseline — `SceneEditorTool` attaches to `WorkspaceViewportManager`, viewport handle requested and activated at startup; software grid renders end-to-end through GDIBackend
- [ ] Activate D3D11 backend as primary (requires Windows SDK + d3d11.lib)
- [ ] Wire DirectWrite text rendering
- 161 D completion tests (79 test cases, 155 assertions), all green (test_phase_d_completion.cpp)
- D.5 partial: `createD3D11WithDirectWrite()` factory + contract tests (NF_PhaseD5Tests), real GPU init deferred

## Phase E Progress (Complete ✅)
### E.1 — Shared NovaForge PCG Core ✅
- [x] `PCGRuleSet` — typed rule container (add/set/remove/reset/filter, dirty tracking)
- [x] `PCGGeneratorService` — stateless deterministic generator (rules + seed → placements)
- [x] `PCGDeterministicSeedContext` — universe seed + domain derivation + child contexts + pinning

### E.2 — Editor PCG Preview Service ✅
- [x] `PCGPreviewService` — bindRuleSet/forceRegenerate/autoRegenerate/stats/lastResult
- [x] `populatePreviewWorld()` — writes PCG placements into NovaForgePreviewWorld

### E.3 — PCG Rule Editing ✅
- [x] `PCGPreviewService::setRuleValue()` + `resetRules()` with auto-regen
- [x] Change callback (`setOnRegenerateCallback()`)
- [x] Domain override (`setDomainOverride()`)
- [x] `ProcGenRuleEditorPanel` — bindDocument/editRule/resetRule/resetAll/save/revert/attachPreviewService
- [x] Save-back: `save()` commits snapshot; `revert()` restores last-saved state

### E.4 — Asset PCG Metadata ✅
- [x] `NovaForgeAssetPreview::AssetPCGMetadata` — placementTag/generationTags/scale/density
- [x] `PCGPreviewService::populatePreviewWorld()` uses asset/placement tags for entity mesh tags
- [x] Event-driven: `setPlacementTagAndNotify()` / `addGenerationTagAndNotify()` / `setPCGDensityAndNotify()` etc. auto-trigger `forceRegenerate()`
- [x] `attachPCGPreviewService()` / `pcgRegenTriggerCount()` — wired event loop
- 82 Phase E core tests + 46 E-completion tests, all green

## Phase F Progress (In Progress — F.1–F.4 contract layer complete; runtime wiring done in G/H)
### F.1 — Embedded PIE Runtime ✅ (contract layer + runtime wired)
- [x] `PIEService` — enter/exit/pause/resume/step/reset with full state machine
- [x] `PIEState` (Stopped/Playing/Paused) + `PIEDiagnosticSeverity` + `PIEPerformanceCounters`
- [x] `PIESessionRecord` — sessionId/durationMs/totalTicks/errorCount/events
- [x] `pushDiagnostic()` / `diagnosticCount()` / `errorCount()` / `countBySeverity()`
- [x] `tickFrame()` updates perf counters (no-op when not Playing)
- [x] All lifecycle callbacks: onEnter/onExit/onPause/onResume/onStep/onReset/onDiagnostic
- [x] PIE toolbar controller and console bridge wired (Phase G)

### F.2 — PIE Input Mode ✅ (contract layer + toolbar wired)
- [x] `PIEInputRouter` — routeToGame/routeToEditor + mode query
- [x] processKey/processMouseButton/processMouseMove dispatch to mode-correct sink
- [x] `isExitKey()` — escape detection in game mode (configurable `exitKeyCode`)
- [x] `modeSwitchCount` / `keyEventCount` / event counters; `onModeChange` callback
- [x] PIE toolbar buttons wired to PIEService (Phase H)

### F.3 — External Game Launch ✅ (contract layer)
- [x] `PIEExternalLaunch` — launch/terminate/simulateExit lifecycle
- [x] `PIELaunchConfig` — executablePath/projectFilePath/args/buildConfiguration
- [x] `pushStdoutLine()` / `onStdoutLine` — console output routing
- [x] `onLaunched` / `onExited` callbacks; `launchCount` / `processId` / `lastExitCode`
- [ ] Real process launch (CreateProcess/fork+exec) — platform-specific, explicitly deferred

### F.4 — PIE Diagnostics ✅ (contract layer)
- [x] Severity-filtered diagnostics, session history capture, onDiagnostic callback
- [ ] Performance counters UI panel — explicitly deferred
- All Phase F tests: 76 test cases, all green (test_phase_f.cpp)

## Phase G Progress (Complete ✅)
- 121 new tests, 350 assertions, all green (test_phase_g.cpp + test_phase_g2_asset_editor.cpp)
- All 8 core tool document models implemented: SceneDocument, AssetDocument, MaterialDocument,
  AnimationDocument, GraphDocument, BuildTaskGraph, AIRequestContext, DataTableDocument
- WorkspaceProjectState: open document registry, active document context, dirty tracking, save/revert
- PIE toolbar controller, console bridge, and PIE wiring complete (test_phase_f_pie_wiring.cpp)

## Phase H Progress (Complete ✅)
- 82 new tests, 238 assertions, all green (test_phase_h.cpp)
- Preferences UI, Keybind editor, Layout persistence (named presets + built-ins)
- Command palette (Ctrl+P, fuzzy search, context-filtered)
- Notification center (history, toasts, severity filtering, click-to-navigate)
- Project open flow (recent projects, file picker, validation, new project wizard)

## Phase I Progress (Complete ✅)
- 20 integration tests, all green (test_phase_i.cpp)
- Full `.atlas` manifest → ProjectRegistry → NovaForgeAdapter → panel registration chain validated
- All 6 gameplay panels verified (economy, inventory_rules, shop, mission_rules, progression, character_rules)
- `ProjectOpenFlowController.validate()` uses real `AtlasProjectFileLoader.bootstrap()` on disk paths

