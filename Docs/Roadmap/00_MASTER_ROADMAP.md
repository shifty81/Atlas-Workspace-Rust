# Master Roadmap

> **End Goal:** Atlas Workspace becomes a clean generic host that can load NovaForge
> from `.atlas`, present real editable documents and runtime-backed preview panels,
> run the shared NovaForge PCG pipeline in-editor, and support fast Play-In-Editor
> plus standalone game validation.
>
> Everything below exists to reach that goal. No open-ended brainstorm.

---

## Checkpoint — Foundation Work (Phases 0–71, Complete)

All infrastructure phases are done and checked. They delivered:

- Canon docs, naming scrub, repo hygiene (Phase 0)
- WorkspaceShell, ToolRegistry, PanelRegistry (Phase 1)
- UIBackend contracts, D3D11/DirectWrite stubs, GDI fallback (Phase 2)
- Editor consolidation, 8 hosted tools, shared panels (Phase 3)
- AtlasAI broker flow, Codex mirroring (Phase 4)
- ProjectRegistry, ProjectLoadContract, NovaForge adapter (Phase 5)
- Build presets, patch applier, release manifest (Phase 6)
- ViewportHostContract, TypographySystem (Phase 7)
- WorkspaceBootstrap, FrameController, ConsoleCommandBus, SelectionService (Phase 8)
- AssetCatalog, AssetTransformer, ContentRouter, AssetWatcher (Phase 9)
- WorkspaceProjectFile, ProjectSerializer, SettingsStore (Phase 10)
- CommandRegistry, CommandHistory, ActionMap (Phase 11)
- EventBus, NotificationBus (Phase 12)
- PreferenceRegistry, PreferenceController (Phase 13)
- PluginSystem (Phase 14)
- DiagnosticCollector, TelemetryCollector (Phase 15)
- ScriptEngine, AutomationTask (Phase 16)
- SearchEngine, SearchIndex (Phase 17)
- UndoStack, UndoManager (Phase 18)
- SessionManager (Phase 19)
- ClipboardManager (Phase 20)
- Viewport rendering contracts, viewport manager, end-to-end wiring (Phases 64–66)
- Asset browser, text input, tool render contract, NovaForge panel factories (Phases 67–70)
- EditorApp deprecation, AssetCatalogPopulator, settings/layout persistence, validator fixes (Phase 71)

**What this delivered:** Contracts, shells, registries, descriptors, and test infrastructure.

**What this did NOT deliver:** Real data-backed editor loops. Panels render chrome but
do not bind to editable game state. Viewport shows placeholder grids. NovaForge panels
have titles but no schemas. PCG is not connected. There is no Play-In-Editor mode.

That is what the phases below address.

---

## Phase A — Truth and Cleanup Lock

**Status: Complete**

**Goal:** Stop repo drift. Make the roadmap, status docs, and codebase honest about
what works vs. what is a stub. Remove forwarding shims and legacy cruft that creates
the illusion of completeness.

### Milestone A.1 — Repo Cleanup
- [x] Archive remaining forwarding shims between `Source/Editor` and `Source/Workspace`
- [x] Remove stale `EditorApp` references from non-test code
- [x] Rename `ArbiterReasoner.cpp` in pipeline to `AtlasAIReasoner.cpp`
- [x] Slim `main.cpp` — extract `LocalProjectAdapter` into dedicated unit
- [x] Remove overclaiming language from roadmap/status docs (presence ≠ completion)

### Milestone A.2 — Build and CI Hygiene
- [x] Verify all test suites build and pass on clean checkout (4125 tests, all green)
- [x] Ensure `validate_project.sh` stays at 79/79 (confirmed)
- [x] Add CI smoke build for `ATLAS_ENABLE_ONLINE_DEPS=ON` (`.github/workflows/ci.yml`)

### Milestone A.3 — Documentation Correction
- [x] Rewrite `Docs/Canon/00_PROJECT_STATUS.md` to reflect honest wiring state
- [x] Mark stale phase-specific roadmap docs (01–07) as historical
- [x] Update `Docs/Canon/09_DEFINITION_OF_DONE.md` to include "panel edits real data" criteria

**Success Criteria:** ✅
- Zero forwarding shims between Editor↔Workspace (37 archived to Deprecated/)
- All naming violations resolved (ArbiterReasoner → AtlasAIReasoner; SwissAgent absorbed into AtlasAI)
- Status docs accurately describe stub vs. wired state
- Build passes on clean checkout (4125 tests green)

---

## Phase B — Real Project Load

**Status: Complete**

**Goal:** When a user opens `NovaForge.atlas`, the workspace loads real project state—
not just adapter descriptors. Asset registries, gameplay data registries, and document
registries all populate from project files on disk.

### Milestone B.1 — .atlas Manifest Contract ✅
- [x] Extend `AtlasProjectFileLoader` to resolve all manifest fields into a typed `ProjectBootstrapResult`
- [x] Validate content roots, data roots, schema roots, and asset registry locations on load
- [x] Report missing/invalid paths as structured `ProjectLoadContract` errors

### Milestone B.2 — NovaForge Project Bootstrap ✅
- [x] Create `NovaForgeProjectBootstrap` service in `NovaForge/Source/EditorAdapter/`
  - Validates NovaForge project structure against `.atlas` manifest
  - Loads asset registry from `Content/` root into `AssetCatalog`
  - Loads gameplay data registries from `Data/` root
  - Populates `ProjectSystemsTool` with real data-backed panel descriptors
- [x] Wire bootstrap into `NovaForgeAdapter::initialize()` flow

### Milestone B.3 — NovaForge Document Registry ✅
- [x] Define `NovaForgeDocumentType` enum covering all authoring targets:
  - Item definition, structure archetype, biome definition, planet archetype
  - Faction definition, mission definition, progression rules
  - Character rules, economy rules, crafting definitions, PCG rulesets
- [x] Create `NovaForgeDocumentRegistry` — maps document type → schema + load/save paths
- [x] Create `NovaForgeDocument` — base class with dirty tracking, validate, save/apply/revert

### Milestone B.4 — Asset Registry Population ✅
- [x] Extend `AssetCatalogPopulator` to scan project content root recursively
- [x] Register discovered assets in `AssetCatalog` with full metadata (type, path, GUID)
- [x] Wire asset catalog into `ContentBrowserPanel` for live display
- [x] Wire asset selection into `InspectorPanel` for property display

**Success Criteria:** ✅
- Opening `NovaForge.atlas` populates 3+ registries with real data
- `ContentBrowserPanel` shows actual project assets
- `InspectorPanel` shows selected asset metadata
- Invalid project structure produces actionable error messages
- 40+ tests, all green (test_phase_b.cpp)

---

## Phase C — Panels Edit Real Data

**Status: Complete**

**Goal:** Gameplay and data panels stop being empty shells. Each panel binds to a
NovaForge document, loads real schema, and supports edit/save/revert.

### Milestone C.1 — Schema-Backed Panel Framework ✅
- [x] Create `IDocumentPanel` interface extending `IEditorPanel`:
  - `bindDocument(NovaForgeDocument*)`, `boundDocument()`, `hasDocument()`
  - `isDirty()`, `save()`, `revert()`, `validate()`
  - `onDocumentChanged()` callback, `dirtyTitle()`, `setOnDirtyCallback()`
- [x] `DocumentPanelBase` — common base: per-panel undo stack, dirty tracking,
      save/revert scaffolding, `loadFromDocument` / `applyToDocument` hooks
- [x] Per-panel `UndoStack` integrated via `pushPropertyEdit` helper

### Milestone C.2 — NovaForge Gameplay Panels (Real Content) ✅
- [x] `EconomyPanel` — currency definitions, pricing rules, inflation rate, undo/redo
- [x] `InventoryRulesPanel` — slot layout, storage rules, stacking config, undo/redo
- [x] `ShopPanel` — store listings, purchase conditions, global discount, undo/redo
- [x] `MissionRulesPanel` — objectives (Kill/Collect/Reach/…), chains, rewards, undo/redo
- [x] `ProgressionPanel` — XP curve (level thresholds), skill unlock tree, undo/redo
- [x] `CharacterRulesPanel` — class presets, stat caps, appearance config, undo/redo

### Milestone C.3 — Data Editor Tool (Real Content) ✅
- [x] `DocumentPropertyGrid` — schema-driven property grid (String/Float/Int/Bool/Enum)
- [x] `DocumentPropertyGridBuilder` — fluent builder for schema definition
- [x] Per-field validator callbacks, Enum option validation
- [x] `toFlatMap()` serialization, `resetToDefaults()`, dirty tracking per grid

### Milestone C.4 — Panel Save/Apply Pipeline ✅
- [x] `DocumentSavePipeline` — validates, writes JSON to project data directory
- [x] Per-panel undo/redo stack (cleared on successful save)
- [x] Dirty indicator via `dirtyTitle()` ("*" suffix when dirty)
- [x] Validation failure messaging via `NotificationCallback` sink
- [x] `revert()` — reloads from document and clears dirty state

**Success Criteria:** ✅
- All 6 NovaForge panels bind documents, load real data schemas, and support edit/save/revert
- Edits in any panel produce dirty state and can be saved/reverted
- Undo/redo works for all panel edits (per-panel UndoStack)
- Invalid data produces visible validation errors (ValidationFailed result + notifications)
- 58 new tests, 104 assertions, all green (test_phase_c.cpp)

---

## Phase D — Runtime-Backed Viewport

**Status: In Progress**

**Goal:** Viewport is no longer a placeholder. It renders real NovaForge preview scenes
using the same runtime code the game client uses.

### Milestone D.1 — NovaForge Preview Runtime Bridge ✅
- [x] Create `NovaForgePreviewRuntime` — lightweight runtime instance for editor preview
  - Instantiates NovaForge world/scene state
  - Provides `IViewportSceneProvider` implementation
  - Supports scene update tick without full game boot
- [x] Create `NovaForgePreviewWorld` — editable preview scene container
  - Entity instantiation and destruction
  - Transform manipulation
  - Material/mesh assignment

### Milestone D.2 — Scene Editor Viewport ✅
- [x] Wire `SceneEditorTool` to `NovaForgePreviewRuntime` as its scene provider (`attachSceneProvider()`)
- [x] Implement fly-camera controls (WASD + mouse look via `processCameraInput()`)
- [x] Implement gizmo state bound to selected entity (`gizmoState()` derives from world selection)
- [x] Selection → Inspector binding (`selectedEntityProperties()` returns flat property map)
- [x] Entity hierarchy → Outliner binding (`hierarchyOrder()` returns parent-before-child order)

### Milestone D.3 — Asset Preview Viewport ✅
- [x] Wire `AssetEditorTool` to a per-asset preview scene (`attachAssetPreviewProvider()`)
- [x] Preview shows selected asset as a `NovaForgeAssetPreview` scene provider
- [x] Editable transform, material, attachment metadata (setTransform/setMeshTag/setMaterialTag/setAttachmentTag)
- [x] Collider/socket/anchor editing for asset placement metadata
  - `ColliderDescriptor` (shape/extents/radius/isTrigger/tag), `setCollider*()` methods
  - `SocketDescriptor` — addSocket/removeSocket/setSocketTransform
  - `AnchorDescriptor` — addAnchor/removeAnchor/setAnchorTransform
- [x] PCG tag and placement metadata editing
  - `AssetPCGMetadata` (placementTag, generationTags, scale range, density, exclusionGroup)
  - `setPlacementTag`, `addGenerationTag`, `setPCGScaleRange`, `setPCGDensity`, etc.

### Milestone D.4 — Material Preview Viewport ✅
- [x] Create `NovaForgeMaterialPreview` — `IViewportSceneProvider` for `MaterialEditorTool`
  - `bindMaterial()` / `clearMaterial()` / `hasMaterial()`
  - Standard preview meshes: Sphere, Cube, Plane (`setPreviewMesh()`)
  - Shader tag (`setShaderTag()`)
  - Material parameters (`setParameter(name, value, type)`, `removeParameter()`, `resetParameterToDefault()`)
  - `apply()` / `revert()` baseline management
  - `properties()` — flat map for inspector display
- [x] Wire `MaterialEditorTool` to material preview via `attachMaterialPreviewProvider()`
  - `provideScene()` delegates to `NovaForgeMaterialPreview` when attached
  - Stub state (hasContent=false) when no provider or no material bound

### Milestone D.5 — D3D11 Backend Activation ✅
- [x] `createD3D11WithDirectWrite()` factory wired in `UIBackendSelector.h` — creates composited `D3D11Backend` + `DirectWriteTextBackend` pair
- [x] `D3D11WithDirectWritePair` struct — `isCreated()`, `geom` (IFrameBackend + IGeometryBackend + ITextureBackend), `text` (ITextRenderBackend)
- [x] `D3D11Backend` and `DirectWriteTextBackend` contract-level validation (stub builds, CI-safe)
- [x] All panels verified to compile and route through D3D11 path at the interface level
- [ ] Real D3D11 device init (awaits Windows CI with GPU — stub returns false until then)
- [ ] DirectWrite glyph rasterisation (awaits COM init on Windows target)

**Success Criteria (D.1–D.4):** ✅
- `NovaForgePreviewRuntime` implements `IViewportSceneProvider`; `provideScene()` reflects world state
- `SceneEditorTool.provideScene()` delegates to attached runtime when wired
- Fly-camera state updates on WASD/mouse input
- Gizmo state reflects selected entity position
- `selectedEntityProperties()` returns name/position/mesh/material for selected entity
- `hierarchyOrder()` returns parents before children
- `AssetEditorTool.provideScene()` delegates to `NovaForgeAssetPreview` when wired
- Asset preview collider/socket/anchor/PCG tag editing with apply()/revert() round-trips
- `MaterialEditorTool.provideScene()` delegates to `NovaForgeMaterialPreview` when wired
- Material parameter CRUD, preview mesh selection, shader tag — all with dirty/apply/revert
- 161 new tests (84 original + 79 D completion), all green

---

## Phase E — Shared PCG Preview Pipeline

**Status: In Progress**

**Goal:** Editor and game client use the same PCG core. Panels can edit PCG rules and
see regenerated results in the viewport immediately.

### Milestone E.1 — Shared NovaForge PCG Core ✅
- [x] Factor PCG generation code into a shared library usable by both client and editor
- [x] Define `PCGRuleSet` — typed rule container loaded from project data
  - addRule/setValue/removeRule/findRule/getValue/hasRule
  - resetToDefaults/resetRule, rulesInCategory, dirty tracking
- [x] Define `PCGGeneratorService` — stateless generator: rules + seed → placements
  - Deterministic: same inputs always produce same output
  - validate() warns on missing recommended rules
- [x] Define `PCGDeterministicSeedContext` — reproducible seed management
  - Universe seed + domain derivation (FNV-1a + xorshift mixing)
  - childContext, pinDomainSeed, registerDomain

### Milestone E.2 — Editor PCG Preview Service ✅
- [x] Create `PCGPreviewService` — editor-side wrapper around shared PCG core
  - Accepts rule document (`bindRuleSet()`) + seed (`setSeed()`)
  - Generates preview output (placements with assetTag/position/scale/yaw)
  - Caches results (`hasResult()`, `lastResult()`)
- [x] Wire PCG preview into viewport via `populatePreviewWorld(NovaForgePreviewWorld&)`
- [x] Support manual regeneration trigger (`forceRegenerate()`)
- [x] Support automatic regeneration on rule edit (`autoRegenerate` flag)

### Milestone E.3 — PCG Rule Editing ✅
- [x] `PCGPreviewService::setRuleValue()` — edits ruleset + triggers regen if auto=true
- [x] `PCGPreviewService::resetRules()` — reverts to defaults + triggers regen
- [x] Change callback (`setOnRegenerateCallback()`) — invoked after each successful regen
- [x] Domain override (`setDomainOverride()`) — controls which seed stream is used
- [x] `ProcGenRuleEditorPanel` — panel-level wiring: `bindDocument()`, `editRule()`, `resetRule()`, `resetAll()`, `save()`, `revert()`, `attachPreviewService()`
- [x] Save-back: `save()` commits dirty rule state as new snapshot; `revert()` restores snapshot

### Milestone E.4 — Asset PCG Metadata ✅
- [x] Asset editor (`NovaForgeAssetPreview`) exposes PCG placement tags and generation constraints
- [x] `PCGPreviewService::populatePreviewWorld()` uses asset/placement tags for entity mesh tags
- [x] Event-driven: `setPlacementTagAndNotify()` / `addGenerationTagAndNotify()` / `removeGenerationTagAndNotify()` / `setPCGScaleRangeAndNotify()` / `setPCGDensityAndNotify()` / `setPCGExclusionGroupAndNotify()` — each auto-triggers `PCGPreviewService::forceRegenerate()` when a service is attached
- [x] `attachPCGPreviewService()` / `detachPCGPreviewService()` — clean service lifecycle
- [x] `pcgRegenTriggerCount()` — tracks event-driven regeneration invocations

**Success Criteria (E.1–E.4):** ✅
- Same `PCGGeneratorService` usable in editor and game runtime (no engine dependency)
- Same inputs + same seed always produce identical `PCGGenerationResult`
- Rule edits via `PCGPreviewService` trigger live preview regeneration when autoRegenerate=true
- PCG output populates `NovaForgePreviewWorld` with positioned, tagged entities
- Deterministic seeds produce identical placement positions
- 82 new tests, 175 assertions, all green (test_phase_e.cpp)

---

## Phase F — Play-In-Editor (PIE)

**Status: In Progress**

**Goal:** Test gameplay from inside the workspace. Fast iteration loop for authoring.

### Milestone F.1 — Embedded PIE Runtime ✅
- [x] `PIEService` — manages embedded runtime instance lifecycle
  - `enter()` — snapshot editor world, start simulation; increments sessionId
  - `exit()` — destroy runtime, restore editor state; records session history
  - `pause()` / `resume()` / `step()` — runtime control with tick counting
  - `reset()` — restart from last snapshot, clears counters
- [x] `PIEState` enum (Stopped / Playing / Paused) with name helper
- [x] `PIEPerformanceCounters` — fps / entityCount / drawCallCount / memoryBytes / tickIndex / lastFrameMs
  - `tickFrame()` updates counters each playing frame; no-op while paused/stopped
- [x] `PIEDiagnosticEvent` — severity / message / source / tickIndex
- [x] `PIESessionRecord` — sessionId / durationMs / totalTicks / errorCount / events
- [x] Lifecycle callbacks: onEnter / onExit / onPause / onResume / onStep / onReset / onDiagnostic
- [ ] Runtime instance runs in viewport panel (same render surface) — Phase G
- [x] Editor panels switch to read-only during PIE — `PIEToolbarController::isReadOnly()` gate

### Milestone F.2 — PIE Input Mode ✅
- [x] `PIEInputRouter` — mode switch (Editor ↔ Game) with `routeToGame()` / `routeToEditor()`
- [x] `processKey()` / `processMouseButton()` / `processMouseMove()` dispatch to mode-correct sink
- [x] `isExitKey()` — detects escape (configurable `exitKeyCode`) in game mode
- [x] `modeSwitchCount` / `keyEventCount` / `mouseEventCount` / `mouseMoveCount`
- [x] `onModeChange` callback fires on each switch
- [x] PIE toolbar: Play/Pause/Stop/Step buttons — `PIEToolbarController` with enable states and callbacks

### Milestone F.3 — External Game Launch ✅
- [x] `PIEExternalLaunch` — manages external process lifecycle
- [x] `PIELaunchConfig` — executablePath / projectFilePath / args / buildConfiguration
- [x] `launch()` stub: Idle → Running (real fork/exec gated behind platform layer)
- [x] `terminate()` — Running → Exited
- [x] `simulateExit(code)` — Exited (0) or Crashed (nonzero)
- [x] `pushStdoutLine()` + `onStdoutLine` callback — routes console output to listeners
- [x] `onLaunched` / `onExited` callbacks
- [x] `launchCount` / `processId` / `lastExitCode` counters
- [ ] Real process launch (CreateProcess / fork+exec) — platform-specific activation
- [x] Console output piped to `ConsolePanel` — `PIEConsoleBridge` wires `PIEExternalLaunch` stdout → panel

### Milestone F.4 — PIE Diagnostics ✅
- [x] `pushDiagnostic()` records events in session diagnostics list (capped at 1024)
- [x] `errorCount()` counts Error + Critical severity events
- [x] `countBySeverity()` filters by severity
- [x] `onDiagnostic` callback invoked per event
- [x] `PIESessionRecord` captures all events at exit for session replay/debugging
- [ ] Performance counters UI panel — Phase G

**Success Criteria:**
- PIE enters and exits cleanly without corrupting editor state
- Gameplay is testable inside the viewport
- Input routing switches correctly between editor and game
- External game launch works as full validation path
- Runtime errors visible in editor during PIE

---

## Phase G — Full Tool Wiring

**Status: Complete**

**Goal:** All core hosted tools become functional for real development work.

### Milestone G.0 — Workspace Ownership Normalization ✅
- [x] `WorkspaceProjectState` — missing spine between WorkspaceShell and per-document authoring units
  - Active loaded project identity, adapter reference, load contract snapshot
  - Open document registry (OpenDocumentEntry): open, close, has, find, count
  - Active document context (setActiveDocument / activeDocumentId)
  - Aggregate dirty tracking (hasUnsavedChanges, dirtyDocumentCount)
  - Project-wide save/revert coordination (saveAll / revertAll)
  - Panel binding context (setActivePanelContext)
  - Change listener chain (addChangeListener / clearChangeListeners)
- [x] WorkspaceShell owns `WorkspaceProjectState`; loadProject/unloadProject notify it
- [x] WorkspaceShell exposes `projectState()` accessor
- [x] Ownership guardrail comments added to: WorkspaceShell.h, IGameProjectAdapter.h
- [x] ProjectSurfaceV1.h demoted to light browser surface (scope guardrail comment added)
- [x] Deprecated forwarding headers (WorkspaceShell, IGameProjectAdapter, SelectionService, UndoRedoSystem, EditorUndoSystem) hardened with `#pragma message` build-time warnings
- [x] 57 new tests in test_workspace_project_state.cpp, all green

### Milestone G.1 — Scene Editor
- [x] `SceneDocument.h` — world/level document model with entity hierarchy
- [x] Entity create/delete/duplicate (createEntity/destroyEntity/duplicateEntity)
- [x] Transform editing (setEntityTransform / entityTransform)
- [x] Component add/remove/edit (addComponent/removeComponent/setComponentProperty)
- [x] Scene save/load (save/load/serialize)

### Milestone G.2 — Asset Editor
- [x] `AssetDocument.h` — asset document model with metadata editing
- [x] LOD variant management (addLOD/removeLOD/findLOD/setLODScreenPercent)
- [x] Variant management (addVariant/removeVariant/findVariant)
- [x] Dependency table (addDependency/removeDependency)
- [x] Reimport settings (AssetImportSettings, setSourcePath)
- [x] Save/load/serialize contract
- [x] 3D preview with editable transform/material/attachment — `AssetPreviewState.h`
- [x] LOD/variant metadata editing — covered by `AssetDocument.h` (addLOD/removeVariant/etc.)
- [x] Import/reimport pipeline integration — `AssetReimportPipeline.h`
- [x] Asset dependency viewer — `AssetDependencyViewer.h`

### Milestone G.3 — Material Editor ✅
- [x] Material graph document model (`MaterialDocument.h`)
- [x] Node-based shader graph editing (addNode/removeNode, typed `MaterialNodeType`)
- [x] Parameter editing (setNodeParam/getNodeParam per node)
- [x] Pin management (addPin/removePin) and connection (connectPins/disconnectPin/isConnected)
- [x] Output node designation (outputNodeId)
- [x] Save/load/serialize contract

### Milestone G.4 — Animation Editor ✅
- [x] Animation clip document model (`AnimationDocument.h`)
- [x] Channel management (addChannel/removeChannel/findChannelByBone, `AnimChannelType`)
- [x] Keyframe management (addKeyframe/removeKeyframe, sorted by time, `AnimInterpolation`)
- [x] Clip metadata (durationMs, fps, isLooping)
- [x] Save/load/serialize contract

### Milestone G.5 — Visual Logic Editor ✅
- [x] Graph document model (`GraphDocument.h`)
- [x] Node management (addNode/removeNode/findNode, position, typeName, title)
- [x] Pin management (addPin/removePin, `GraphPinDir`, `GraphPinCategory`)
- [x] Connection management (connect/disconnect/isConnected)
- [x] Compile/validate with error reporting (`GraphCompileResult` — unconnected exec pin detection)
- [x] Node property bag (setNodeProperty/getNodeProperty)
- [x] Save/load/serialize contract

### Milestone G.6 — Build Tool ✅
- [x] Build task DAG (`BuildTaskGraph.h`) — addTask/depends, `BuildTaskState`
- [x] Topological order computation (dependency-first, Kahn's algorithm)
- [x] Task state transitions (setTaskState with durationMs)
- [x] Build log (pushOutputLine, `BuildLineType`, output callback)
- [x] Summary statistics (totalErrors/totalWarnings/totalDurationMs, `BuildGraphResult`)
- [x] resetAllToState — full build reset

### Milestone G.7 — AtlasAI Tool ✅
- [x] Context-aware request input (`AIRequestContext.h`) — activeDocumentId, activeObjectId, contextErrors
- [x] Conversation history (submitUserMessage/submitAssistantMessage, `AIMessageRole`)
- [x] Diff proposal display (`DiffProposal`, `DiffProposalLine`, `DiffLineAction`)
- [x] Apply/reject workflow (applyProposal/rejectProposal, `DiffProposalStatus`)
- [x] Codex snippet management (addCodexSnippet/removeCodexSnippet, `CodexSnippet`)

### Milestone G.8 — Data Editor ✅
- [x] Schema-driven data table (`DataTableDocument.h`) — addColumn/removeColumn, `DataColumnType`
- [x] Row management (addRow/removeRow/duplicateRow)
- [x] Cell access (setCell/getCell with column defaults)
- [x] Per-field validation (`DataCellValidationResult` — type, enum options, required)
- [x] CSV export (exportCsv) + importCsv stub
- [x] Save/load/serialize contract

**Success Criteria:** ✅
- All 8 core tools have document models covering their authoring surface
- Each document model supports create/edit/save/undo-ready dirty tracking
- SceneDocument: entity hierarchy, transforms, components, selection
- AssetDocument: LOD, variant, dependency, reimport metadata
- MaterialDocument: shader graph nodes, pins, connections, params
- AnimationDocument: channels, keyframes (time-sorted), clip metadata
- GraphDocument: nodes, pins, connections, compile with error reporting
- BuildTaskGraph: DAG, topological order, state, build log, summary stats
- AIRequestContext: context binding, messages, diff proposals, codex snippets
- DataTableDocument: columns, rows, cells, per-field validation, CSV export
- 121 new tests, 350 assertions, all green (test_phase_g.cpp)

---

## Phase H — UX Completion

**Status: Complete**

**Goal:** Workspace becomes stable and usable for daily development.

### Milestone H.1 — Preferences UI
- [x] Preferences window with category navigation
- [x] All 13+ registered preferences editable via UI
- [x] Project vs. user preference scoping visible
- [x] Import/export/reset settings

### Milestone H.2 — Keybind UI
- [x] Keybinding editor panel
- [x] Per-tool and global shortcut display
- [x] Rebind via capture (press desired key combo)
- [x] Conflict detection and resolution
- [x] Reset to defaults

### Milestone H.3 — Layout Persistence
- [x] Panel layout saves to disk on close
- [x] Layout restores on next open
- [x] Named layout presets (save/load/rename/delete)
- [x] Built-in layout presets (Default, Compact, Wide)

### Milestone H.4 — Command Palette
- [x] Ctrl+P opens searchable command list
- [x] Commands filtered by current context (tool, panel, scope)
- [x] Recent commands section
- [x] Fuzzy search matching

### Milestone H.5 — Notification Center
- [x] Notification panel with history
- [x] Toast popups for important events
- [x] Severity filtering (info/warning/error/critical)
- [x] Click-to-navigate for actionable notifications

### Milestone H.6 — Project Open Flow
- [x] Recent projects list on startup
- [x] File → Open Project flow with `.atlas` file picker
- [x] Project validation on open with error summary
- [x] New project wizard (create `.atlas` from template)

**Success Criteria:** ✅
- Preferences editable and persisted ✅
- Keybinds customizable and conflict-free ✅
- Panel layout survives restart ✅
- Command palette provides fast access to all commands ✅
- Notifications visible and actionable ✅
- Project open flow is clean and error-tolerant ✅

- 82 new tests, 238 assertions, all green (test_phase_h.cpp)

---

## Phase I — NovaForge End-to-End Project Load Integration

**Status: Complete**

**Goal:** Validate that the full project loading chain works with the real NovaForge
project structure: `.atlas` manifest parsing → `ProjectRegistry` → `NovaForgeAdapter`
initialization → bootstrap → panel registration. Close the gap between infrastructure
phases (A–H) and genuine project load confidence.

### Milestone I.1 — .atlas Manifest Validation
- [x] `AtlasProjectFileLoader.bootstrap()` validates the real `NovaForge.atlas`
- [x] Manifest fields (name, adapterId, capabilities, contentRoot) verified
- [x] No fatal validation entries produced

### Milestone I.2 — Project Directory Layout Validation
- [x] `NovaForgeProjectBootstrap` validates the real project root
- [x] `Content/`, `Data/`, `Config/`, `Schemas/` all confirmed present
- [x] `AssetCatalog` populated from `Content/` root

### Milestone I.3 — ProjectRegistry Load Flow
- [x] "novaforge" factory registered with project root captured in closure
- [x] `loadProject("novaforge")` returns `ProjectLoadState::Ready`
- [x] `loadProjectFromAtlasFile(atlasPath)` succeeds end-to-end
- [x] Contract carries content roots and custom commands

### Milestone I.4 — Adapter and Panel Verification
- [x] `NovaForgeAdapter.initialize()` succeeds with real project root
- [x] All 6 gameplay panel descriptors present (economy, inventory_rules, shop, mission_rules, progression, character_rules)
- [x] All panels hosted under `workspace.project_systems`
- [x] `createPanel()` factory instantiates each panel without errors
- [x] `shutdown()` clears all panels

### Milestone I.5 — ProjectOpenFlowController Real Validation
- [x] `validate()` upgraded: uses `AtlasProjectFileLoader.bootstrap()` when file is on disk
- [x] Non-existent paths: extension-only validation with warning (no regressions)
- [x] Real `NovaForge.atlas`: manifest content parsed, project name extracted from manifest
- [x] Malformed `.atlas` files produce validation errors
- [x] `confirmOpen()` after real validation succeeds and reaches `Open` state

**Success Criteria:** ✅
- Full load chain verified against the actual `NovaForge.atlas` and project structure
- 20 integration tests, all green (test_phase_i.cpp)

---

## Phase 72 — Viewport Entity Projection

**Status: Complete**

**Goal:** The software viewport renderer becomes scene-aware. Entities from
`NovaForgePreviewWorld` are projected into screen space and drawn as wireframe
boxes with cross markers, proving the full data pipeline end-to-end.

### Milestone 72.1 — ViewportEntityProxy carrier ✅
- [x] `ViewportEntityProxy` struct added to `IViewportSceneProvider.h`
  - World position (x/y/z), half-extents (halfW/H/D), selected flag, label ptr
- [x] `ViewportSceneState::entities` vector — carries one proxy per visible entity

### Milestone 72.2 — Scene provider populates proxies ✅
- [x] `NovaForgePreviewRuntime::provideScene()` iterates `m_world.entities()`
- [x] Populates position from `e.transform.position`, scale*0.5 as half-extents
- [x] Sets `proxy.selected` from `m_world.selectedEntityId()`
- [x] Skips invisible entities (`e.visible == false`)

### Milestone 72.3 — SoftwareViewportRenderer 3D→2D projection ✅
- [x] `buildCamera()` — derives right/up/forward from yaw/pitch, computes halfFovTan
- [x] `worldToView()` — transforms world point to camera space via dot products
- [x] `viewToScreen()` — perspective divide + NDC→pixel mapping
- [x] `projectPoint()` — public API, returns false for behind-camera points
- [x] `drawEntity()` — draws cross marker + projected bounding quad per entity
- [x] Selected entities use `selectColor`; others use `entityColor`
- [x] Behind-camera entities (vz ≤ 0) are culled

### Milestone 72.4 — Tests ✅
- [x] 22 new tests, 48 assertions, all green (test_phase72_viewport_entity_render.cpp)
  - Entity proxy population from `provideScene()`
  - Projection to correct screen positions (centre, left, right, up, down)
  - Behind-camera culling
  - Selected entity pixel colour
  - Hidden entity not rendered
  - End-to-end: world entity → `provideScene()` → `renderGrid()` → pixel

**Success Criteria:** ✅
- `provideScene()` populates `ViewportSceneState::entities` with correct positions/flags
- `projectPoint()` projects origin to screen centre for front-facing camera
- `renderGrid()` writes `entityColor`/`selectColor` pixels when entities are in scene
- Behind-camera entities do not produce entity pixels
- 22 new tests, all green

---

## Version Target

When Phases A–I are complete, the repository reaches **Atlas Workspace v1.0** — a
functional development workspace that loads NovaForge projects, presents real editable
content, renders runtime-backed viewport previews, runs shared PCG generation, and
supports Play-In-Editor for rapid iteration.

All further work (multiplayer preview, advanced VFX, cloud sync, marketplace) builds
on top of v1.0 and is not part of this roadmap.
