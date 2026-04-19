# Master Roadmap — Rust-First (Phase 0–7)

> **Primary Direction**: Rust + Vulkan. All new systems are written in Rust.
> C++ in `Source/` and `NovaForge/` is the specification and blueprint for the Rust port.
>
> Reset Date: 2026-04-18 | Last Audit: 2026-04-18

---

## COMPLETED WORK

What exists in Rust today (M18 — 2026-04-19):

- **Cargo workspace**: 23 crates (22 atlas-* + novaforge-game), all compile cleanly
- **atlas-core / atlas-math / atlas-ecs / atlas-pcg / atlas-world / atlas-workspace**: fully implemented
- **atlas-renderer**: Vulkan surface via `ash-window` wired, full acquire→present pipeline, GBuffer, PBR material, shadow maps, post-process, instanced renderer, spatial hash, `TerrainMesh::from_heightmap()`
- **atlas-editor**: app shell, 5 panels, CommandStack, SelectionState, entity commands, GameBuildSystem, GameProjectAdapter, ViewportHost, ViewportRegistry, NotificationCenter, LayoutPersistence, PropertyGrid, **AtlasManifest, ProjectRegistry, IEditorTool/IEditorPanel/ToolRegistry** (M16)
- **atlas-game**: GameRunner, NullGameModule, GameModule trait
- **atlas-asset**: AssetRegistry, AssetMeta, AssetGraph + `NOVAFORGE_ASSETS_DIR` load path
- **atlas-ui**: ScrollList (virtual scroll), TreeView, UiLogCapture
- **novaforge-game** (GPL v3.0): NovaForgeGameModule + NovaForgeAdapter (GameProjectAdapter) + NovaForgeProjectBootstrap + AssetCatalog + DataRegistry + DocumentRegistry + 6 gameplay panels + NovaForgeDocument (22-type enum) + DocumentSavePipeline + PanelUndoStack + NovaForgePreviewWorld (M17) + PCG Core (PcgRuleSet + PcgDeterministicSeedContext + PcgGeneratorService + PcgPreviewService + ProcGenRuleEditorPanel) (M17) + **NovaForgePreviewRuntime (fly-camera, gizmo, hierarchy)** + **DocumentPropertyGrid + DocumentPropertyGridBuilder** + **NovaForgeAssetPreview (colliders, sockets, anchors, PCG metadata)** + **NovaForgeMaterialPreview (parameters, preview mesh)** + **ScenePreviewBinder + AssetPreviewBinder + MaterialPreviewBinder** (M18)
- **832 passing unit tests** across the workspace
- **C++ Blueprint preserved**: `Source/`, `NovaForge/` are the specification for the Rust port

---

## Phase 0 — Rust Foundation Completion (Current, In Progress)

**Goal**: All 23 crates have real implementations. Build and test clean.

### Milestone 0.1 — Renderer Activation ✅ COMPLETE

- [x] Wire Vulkan surface via winit + ash-window (`VulkanContext::new_with_window`)
- [x] SPIR-V compilation pipeline (glslc / naga) — `Scripts/build_shaders.sh`
- [x] GPU terrain mesh upload from atlas-pcg heightmap (`TerrainMesh::from_heightmap`)
- [x] Headless render loop integration test (NullRendererBackend)

### Milestone 0.1b — Renderer Polish (In Progress)

- [ ] Wire `atlas-workspace/main.rs` standalone Vulkan boot path (`--vulkan` flag)
- [ ] PCGPreviewService: atlas-pcg TerrainGenerator → TerrainMesh → RenderLoop
- [ ] SPIR-V embedded blobs compiled at build time via `build.rs`

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

### Milestone 0.5 — atlas-asset completion

- [x] Asset handle + UUID registry (`AssetRegistry`, `AssetMeta`)
- [x] Load path via `NOVAFORGE_ASSETS_DIR` environment variable
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

**Success Criteria**: `cargo build --workspace` zero errors, `cargo test --workspace` 600+ tests, all 23 crates non-empty, Vulkan headless loop running

---

## Phase 1 — Editor Core (atlas-editor Completion) — In Progress

**Goal**: `atlas-editor` is a functional egui workspace shell mirroring the C++ WorkspaceShell.

### Milestone 1.1 — Workspace Shell ✅ COMPLETE

- [x] `WorkspaceShell` struct owning `ToolRegistry`, `PanelRegistry`, `EventBus`
- [x] `EditorApp` (top-level egui App)
- [x] Panel layout persistence (JSON) — `LayoutPersistence`, `PanelLayout`, `DockSide`
- [x] `IEditorTool` trait (update, id, title) — M16
- [x] `IEditorPanel` trait (panel_id, panel_title) — M16
- [x] `ToolRegistry` (register, activate, tick_active) — M16
- [ ] DockSpace layout manager (egui docking)

### Milestone 1.2 — Shared Panels ✅ LARGELY COMPLETE

- [x] Inspector/Properties panel — `PropertyGrid`, `PropertySection`, `PropertyEntry`, `PropertyValue`
- [x] Outliner/Hierarchy panel — `OutlinerPanel` (atlas-ecs SceneGraph)
- [x] Content Browser panel — `AssetBrowserPanel` (atlas-asset catalog)
- [x] Console/Log panel — `ConsolePanel` (atlas-core log routing, `UiLogCapture`)
- [x] Notification Center — `NotificationCenter`, `NotificationSeverity`
- [x] Viewport panel — `ViewportPanel`, `ViewportHost`, `ViewportRegistry`

### Milestone 1.3 — Command System ✅ PARTIALLY COMPLETE

- [x] `CommandStack` (undo/redo), `CommandHistory`, `ActionMap`
- [x] `SpawnEntityCommand`, `DeleteEntityCommand`, `RenameEntityCommand`
- [ ] Command palette (Ctrl+P, fuzzy search)
- [ ] Keyboard shortcut binding

### Milestone 1.4 — Project Open Flow ✅ PARTIALLY COMPLETE (M16)

- [x] `.atlas` manifest parser (JSON) — `AtlasManifest`, `AtlasManifestRoots`, `AtlasManifestRuntime` (M16)
- [x] `ProjectRegistry`, `LoadedProject` (M16)
- [ ] Recent projects + file picker
- [ ] New project wizard

**Success Criteria**: `atlas-editor` launches egui shell, 5 panels functional, command palette works, `.atlas` file openable

---

## Phase 2 — Game Project Adapter & NovaForge Rust Port Part 1 — In Progress

**Goal**: NovaForge game logic begins Rust port. `game_project_adapter.rs` connects to real NovaForge Rust systems.

> **License note**: All code in `crates/novaforge-game/` is GPL v3.0 (inherited from Veloren via Nova-Forge).
> Atlas Workspace core crates (`atlas-*`) must never depend on `novaforge-game`.
> Communication flows through the `IGameProjectAdapter` trait boundary.

### Milestone 2.0 — NovaForge Assets Infrastructure ✅ COMPLETE

- [x] `novaforge-assets/` directory structure (common/, voxygen/, world/, server/)
- [x] `Scripts/fetch_novaforge_assets.sh` — downloads LFS assets from Nova-Forge repo
- [x] `.gitignore` rules: binary assets excluded, directory skeleton tracked
- [x] `novaforge-assets/README.md` — explains local asset store + re-fetch instructions
- [x] `NOVAFORGE_ASSETS_DIR` environment variable support in `atlas-asset`

### Milestone 2.1 — IGameProjectAdapter (Rust) ✅ COMPLETE (M16)

- [x] `GameProjectAdapter` trait (`game_project_adapter.rs`) with `initialize_project` + `tool_descriptors`
- [x] `EditorSession`, `PieState` — Play-In-Editor session management
- [x] `NovaForgeAdapter` implementing `GameProjectAdapter` fully (M16)
- [x] `ToolDescriptor` returned by adapter for tool registration (M16)
- [ ] `ProjectSystemsTool` (adapter host panel in editor UI)

### Milestone 2.2 — NovaForge Project Bootstrap (Rust) ✅ COMPLETE (M16)

- [x] `NovaForgeProjectBootstrap` (validates `.atlas`, resolves content roots) (M16)
- [x] `AssetCatalog` (scans novaforge-assets/, registers assets) (M16)
- [x] `DataRegistry` (loads JSON files from `Data/`) (M16)
- [x] `DocumentRegistry` (type registry + open document tracking) (M16)

### Milestone 2.3 — NovaForge Document Types (Rust) ✅ COMPLETE (M17)

- [x] `NovaForgeDocumentType` enum (22 variants — all C++ types ported) (M17)
- [x] `NovaForgeDocument` base (dirty tracking, validate/save/revert lifecycle) (M17)
- [x] `DocumentSavePipeline` (validate → write → clear dirty, notification callback) (M17)
- [x] `DocumentPanelValidationSeverity`, `DocumentPanelValidationMessage` (M17)
- [x] `PanelUndoEntry`, `PanelUndoStack` (per-panel undo/redo) (M17)
- [x] `NovaForgePreviewWorld` (512 entity cap, transforms, mesh/material tags, selection, dirty tracking) (M17)
- [x] Document type name lookup (`document_type_name()`) (M17)
- [x] `DocumentPropertyGrid` + `DocumentPropertyGridBuilder` (schema-driven property grid) (M18)

### Milestone 2.4 — NovaForge Gameplay Panels (Rust) ✅ DATA MODEL COMPLETE (M16)

- [x] EconomyPanel — currency definitions, pricing rules, validation (M16)
- [x] InventoryRulesPanel — slot layout, storage rules, validation (M16)
- [x] ShopPanel — store listings, purchase conditions, validation (M16)
- [x] MissionRulesPanel — objectives, chains, rewards, validation (M16)
- [x] ProgressionPanel — XP curve, skill unlock tree, validation (M16)
- [x] CharacterRulesPanel — class presets, stat caps, validation (M16)
- [ ] egui rendering for all 6 panels (M19)

### Milestone 2.5 — NovaForge PCG Core (Rust) ✅ COMPLETE (M17)

- [x] `PcgRuleValueType` enum (Float/Int/Bool/String/Vec2/Vec3/Range/Tag) (M17)
- [x] `PcgRule` + `PcgRuleSet` (512 rule cap, dirty tracking, add/set/remove/reset) (M17)
- [x] `PcgDeterministicSeedContext` (FNV-1a domain seed derivation, child contexts, pinned seeds) (M17)
- [x] `PcgGeneratorService` (stateless xorshift PRNG, density×count placements, validation) (M17)
- [x] `PcgPreviewService` (result caching, auto-regen, seed management) (M17)
- [x] `ProcGenRuleEditorPanel` (bind/edit/save/revert/preview wiring) (M17)

### Milestone 2.5b — NovaForge Preview Viewport Layer (Rust) ✅ COMPLETE (M18)

- [x] `NovaForgePreviewRuntime` (fly-camera WASD + mouse look, gizmo state, inspector data, hierarchy BFS) (M18)
- [x] `NovaForgeAssetPreview` (colliders, sockets, anchors, PCG metadata, apply/revert, E.4 notify) (M18)
- [x] `NovaForgeMaterialPreview` (material parameters, preview mesh switching, apply/revert) (M18)
- [x] `NovaForgeScenePreviewBinder` + `NovaForgeAssetPreviewBinder` + `NovaForgeMaterialPreviewBinder` (M18)

### Milestone 2.6 — NovaForge ECS Integration (Rust)

- [ ] Port Veloren/NF component types → atlas-ecs `ComponentStore`
- [ ] Port Veloren/NF system traits → atlas-ecs `SystemRegistry`
- [ ] Physics bridging → atlas-physics
- [ ] World generation mapping: NF `world/` heightmap → atlas-pcg `TerrainGenerator`

**Success Criteria**: `NovaForge.atlas` opens in `atlas-editor`, 6 gameplay panels show schema-driven data, NovaForge assets load from `novaforge-assets/`, 850+ tests

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

## Phase 7 — Release Build Pipeline — Planned

**Goal**: Full release build pipeline producing distributable packages for Linux, Windows, and macOS.

### Milestone 7.1 — Clean Release Build

- [ ] `cargo build --release --workspace` zero warnings
- [ ] All `#[allow(dead_code)]` / `#[allow(unused)]` annotations removed or justified
- [ ] Release profile tuning in `Cargo.toml` (LTO, opt-level, strip)

### Milestone 7.2 — Asset Packaging

- [ ] `Scripts/package_release.sh` — collect binaries + novaforge-assets/ into dist/
- [ ] Linux: `atlas-workspace`, `atlas-game`, `novaforge-assets/` → `.tar.gz`
- [ ] Windows: same → `.zip`
- [ ] macOS: same → `.dmg` (via `cargo-bundle` or hdiutil)
- [ ] Asset manifest generation (checksums for integrity verification)

### Milestone 7.3 — Makefile Release Targets

- [ ] `make release` — `cargo build --release --workspace`
- [ ] `make package` — create platform package in `dist/`
- [ ] `make dist` — build + package + checksum

### Milestone 7.4 — GitHub Actions Release CI

- [ ] Matrix: `ubuntu-latest`, `windows-latest`, `macos-latest`
- [ ] Triggered on `v*` tags
- [ ] Uploads release artifacts to GitHub Releases
- [ ] Shader compilation step in CI

### Milestone 7.5 — NovaForge Standalone Distribution

- [ ] `cargo run --bin atlas-game` boots NovaForge standalone (no editor)
- [ ] Assets auto-located from `NOVAFORGE_ASSETS_DIR` or embedded path
- [ ] Release binary includes asset fetch instructions in README
- [ ] Option to bundle a minimal asset subset for CI smoke testing

**Success Criteria**: `make dist` produces runnable packages on all three platforms, GitHub release pipeline runs on tag push, NovaForge launches from release binary

---

## VERSION TARGET

**Atlas Workspace v1.0** = Phase 7 complete: 23 fully implemented Rust crates, NovaForge ported to Rust, Vulkan renderer live, PIE working, AtlasAI integrated, 20-tool editor roster, CI + packaging for all platforms.
