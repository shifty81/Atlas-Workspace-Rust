# Master Roadmap — Rust-First (Phase 0–6)

> **Primary Direction**: Rust + Vulkan. All new systems are written in Rust.
> C++ in `Source/` and `NovaForge/` is the specification and blueprint for the Rust port.
>
> Reset Date: 2026-04-18

---

## COMPLETED WORK

What exists in Rust today:

- **Cargo workspace**: 22 crates scaffolded, all compile cleanly
- **atlas-core / atlas-math / atlas-ecs / atlas-pcg / atlas-world / atlas-workspace**: fully implemented, 439 tests
- **atlas-renderer**: Vulkan pipeline implemented (context, swapchain, shader IR, spatial hash, PBR material, render config); awaiting live surface
- **atlas-editor**: app shell, 5 panels (Outliner, Properties, Viewport, AssetBrowser, Console), CommandStack, SelectionState, entity commands, GameBuildSystem, GameProjectAdapter trait — in progress
- **atlas-game**: GameRunner, NullGameModule, GameModule trait — in progress
- **C++ Blueprint preserved**: Phases A–I in C++ (`Source/`, `NovaForge/`) are the specification for the Rust port

---

## Phase 0 — Rust Foundation Completion (Current, In Progress)

**Goal**: All 22 crates have real implementations. Build and test clean.

### Milestone 0.1 — Renderer Activation

- [ ] Wire Vulkan surface via winit + ash-window
- [ ] SPIR-V compilation pipeline (glslc or naga)
- [ ] GPU terrain mesh upload from atlas-pcg heightmap
- [ ] Headless render loop integration test

### Milestone 0.2 — atlas-input implementation

- [ ] winit event to InputState mapping
- [ ] Key/button/axis abstraction
- [ ] Action binding system

### Milestone 0.3 — atlas-physics implementation

- [ ] AABB collision detection
- [ ] RigidBody + atlas-ecs integration
- [ ] Ray-cast query API

### Milestone 0.4 — atlas-sim implementation

- [ ] Fixed-timestep game loop
- [ ] System scheduler (atlas-ecs SystemRegistry)
- [ ] Deterministic frame counter

### Milestone 0.5 — atlas-asset implementation

- [ ] Asset handle + UUID registry
- [ ] Load-from-disk pipeline (JSON + binary)
- [ ] Hot-reload file watcher

### Milestone 0.6 — atlas-schema implementation

- [ ] JSON schema definition types
- [ ] Schema-driven property grid (for atlas-editor Inspector)
- [ ] Validation result types

### Milestone 0.7 — atlas-abi implementation

- [ ] Stable extern C ABI for plugin loading
- [ ] Plugin descriptor (name, version, capabilities)

### Milestone 0.8 — Remaining stub crates

- [ ] atlas-animation: clip, channel, keyframe, interpolation
- [ ] atlas-sound: audio source, mixer, SFX/music track
- [ ] atlas-script: Lua or Rhai integration, AutomationTask
- [ ] atlas-graphvm: node graph VM, pin types, execution engine
- [ ] atlas-net: client/server, message framing
- [ ] atlas-ai: AtlasAI broker, request context, conversation, diff proposals, Codex
- [ ] atlas-ui: egui widget extensions, panel framework

**Success Criteria**: `cargo build --workspace` zero errors, `cargo test --workspace` 200+ tests, all 22 crates non-empty, Vulkan headless loop running

---

## Phase 1 — Editor Core (atlas-editor Completion) — Planned

**Goal**: `atlas-editor` is a functional egui workspace shell mirroring the C++ WorkspaceShell.

### Milestone 1.1 — Workspace Shell

- [ ] `WorkspaceShell` struct owning `ToolRegistry`, `PanelRegistry`, `EventBus`
- [ ] `IEditorTool` trait (render, update, title, id)
- [ ] `IEditorPanel` trait (reusable panel interface)
- [ ] `EditorApp` (top-level egui App)
- [ ] DockSpace layout manager
- [ ] Panel layout persistence (JSON)

### Milestone 1.2 — Shared Panels

- [ ] Inspector/Properties panel (schema-driven)
- [ ] Outliner/Hierarchy panel (atlas-ecs SceneGraph)
- [ ] Content Browser panel (atlas-asset catalog)
- [ ] Console/Log panel (atlas-core log routing)
- [ ] Notification Center (severity-filtered)

### Milestone 1.3 — Command System

- [ ] `CommandRegistry`, `CommandHistory`, `ActionMap`
- [ ] Command palette (Ctrl+P, fuzzy search)
- [ ] Keyboard shortcut binding

### Milestone 1.4 — Project Open Flow

- [ ] `.atlas` manifest parser (JSON)
- [ ] `ProjectRegistry`, `ProjectLoadContract`
- [ ] Recent projects + file picker
- [ ] New project wizard

**Success Criteria**: `atlas-editor` launches egui shell, 5 panels functional, command palette works, `.atlas` file openable

---

## Phase 2 — Game Project Adapter & NovaForge Rust Port Part 1 — Planned

**Goal**: NovaForge game logic begins Rust port. `game_project_adapter.rs` connects to real NovaForge Rust systems.

### Milestone 2.1 — IGameProjectAdapter (Rust)

- [ ] `IGameProjectAdapter` trait (initialize, shutdown, tool_descriptors, create_panel)
- [ ] `ProjectSystemsTool` (adapter host)
- [ ] `NovaForgeAdapter` implementation

### Milestone 2.2 — NovaForge Project Bootstrap (Rust)

- [ ] `NovaForgeProjectBootstrap` (validates `.atlas`, loads content roots)
- [ ] `AssetCatalog` (scans `Content/`, registers assets with UUID)
- [ ] `DataRegistry` (loads JSON from `Data/`)
- [ ] `DocumentRegistry`

### Milestone 2.3 — NovaForge Document Types (Rust)

- [ ] `SceneDocument` (entity hierarchy, transforms, components)
- [ ] `AssetDocument` (LOD variants, dependencies, reimport settings)
- [ ] `MaterialDocument` (shader graph nodes, pins, connections, params)
- [ ] `AnimationDocument` (channels, keyframes, clip metadata)
- [ ] `GraphDocument` (visual logic, compile + validate)
- [ ] `DataTableDocument` (columns, rows, cells, CSV export)
- [ ] `BuildTaskGraph` (DAG, topological order, build log)

### Milestone 2.4 — NovaForge Gameplay Panels (Rust)

- [ ] EconomyPanel (currency definitions, pricing rules)
- [ ] InventoryRulesPanel (slot layout, storage rules)
- [ ] ShopPanel (store listings, purchase conditions)
- [ ] MissionRulesPanel (objectives, chains, rewards)
- [ ] ProgressionPanel (XP curve, skill unlock tree)
- [ ] CharacterRulesPanel (class presets, stat caps)

**Success Criteria**: `NovaForge.atlas` opens in `atlas-editor`, 6 gameplay panels show schema-driven data, 80+ tests

---

## Phase 3 — Renderer Integration & Viewport — Planned

**Goal**: `atlas-renderer` delivers a real Vulkan viewport inside `atlas-editor`.

### Milestone 3.1 — Vulkan Editor Viewport

- [ ] `EditorViewport` (egui + Vulkan surface via egui-winit + ash-window)
- [ ] Fly-camera controls (WASD + mouse look via atlas-input)
- [ ] Entity gizmos (translate/rotate/scale)
- [ ] Grid and axis overlay

### Milestone 3.2 — Scene Viewport

- [ ] `SceneEditorTool` renders NovaForge preview world
- [ ] Entity selection to Inspector binding
- [ ] Transform manipulation via gizmos

### Milestone 3.3 — PCG Preview in Viewport

- [ ] `PCGPreviewService` (Rust) wrapping atlas-pcg `PcgManager`
- [ ] Rule editing → live regeneration → viewport update
- [ ] `ProcGenRuleEditorPanel`

**Success Criteria**: Vulkan viewport renders in editor, scene entities visible, PCG world appears, camera controls work

---

## Phase 4 — Game Runtime & Play-In-Editor — Planned

**Goal**: `atlas-game` runs standalone AND as embedded PIE inside `atlas-editor`.

### Milestone 4.1 — atlas-game Runtime Completion

- [ ] `GameRunner` (fixed-timestep loop via atlas-sim)
- [ ] `GameModule` trait (plugin-style game module registration)
- [ ] `NovaForgeGameModule` (NovaForge-specific systems)
- [ ] Save/load via `DeltaEditStore` + JSON

### Milestone 4.2 — NovaForge Rust Game Systems

- [ ] `EconomySystem` (currency transactions via atlas-ecs)
- [ ] `InventorySystem` (item management, slot rules)
- [ ] `MissionSystem` (objective tracking, chain progression)
- [ ] `ProgressionSystem` (XP, level threshold, skill unlock)
- [ ] `CharacterSystem` (class stats, appearance config)
- [ ] `ShopSystem` (store listings, purchase flow)
- [ ] `PCGWorldSystem` (integrates atlas-pcg into game loop)

### Milestone 4.3 — Play-In-Editor (PIE)

- [ ] `PIEService` (enter/exit/pause/resume/step/reset)
- [ ] `PIEState` enum (Stopped/Playing/Paused)
- [ ] `PIEPerformanceCounters` (fps, entity count, draw calls, memory)
- [ ] Editor panels read-only during PIE
- [ ] Input routing: Editor mode vs Game mode (atlas-input)
- [ ] PIE toolbar in atlas-editor (Play/Pause/Stop/Step)

### Milestone 4.4 — External Game Launch

- [ ] `PIEExternalLaunch` (spawn atlas-game as child process)
- [ ] stdout routing to Console panel

**Success Criteria**: `cargo run --bin atlas-game` boots NovaForge, PIE enters/exits cleanly, 80+ tests

---

## Phase 5 — AtlasAI & Advanced Editor Tools — Planned

**Goal**: atlas-ai broker wired into atlas-editor. Full 20-tool roster in Rust.

### Milestone 5.1 — AtlasAI (Rust)

- [ ] `AtlasAIBroker` (single AI integration point)
- [ ] `AIRequestContext`, conversation history, `DiffProposal`, `CodexSnippet`
- [ ] Build log → AI analysis routing
- [ ] AI panel in atlas-editor

### Milestone 5.2 — Remaining Tool Roster

- [ ] `ParticleEditorTool`, `AudioEditorTool`, `PhysicsEditorTool`
- [ ] `TerrainEditorTool`, `CinematicEditorTool`, `ProfilerTool`
- [ ] `VersionControlTool`, `ScriptingConsoleTool`, `SettingsTool`

### Milestone 5.3 — Visual Logic Editor (atlas-graphvm)

- [ ] `GraphDocument` backed by atlas-graphvm execution engine
- [ ] Node types: Event, Action, Condition, Variable
- [ ] Runtime execution in PIE

---

## Phase 6 — Polish, CI, Packaging & v1.0 — Planned

### Milestone 6.1 — CI/CD

- [ ] GitHub Actions: build + test + clippy + fmt (Linux + Windows + macOS)
- [ ] Shader compilation in CI
- [ ] Release packaging

### Milestone 6.2 — Documentation

- [ ] All crate-level rustdoc complete
- [ ] Architecture diagrams updated for Rust
- [ ] Contributor guide

### Milestone 6.3 — Performance

- [ ] Rayon profiling pass on atlas-pcg
- [ ] atlas-renderer render graph optimization
- [ ] atlas-ecs component storage SoA optimization

---

## VERSION TARGET

**Atlas Workspace v1.0** = Phase 6 complete: 22 fully implemented Rust crates, NovaForge ported to Rust, Vulkan renderer live, PIE working, AtlasAI integrated, 20-tool editor roster, CI + packaging.
