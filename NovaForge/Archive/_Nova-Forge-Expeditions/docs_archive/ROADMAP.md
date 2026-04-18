# Nova-Forge: Expeditions — Development Roadmap
### Engine · Tooling · GUI · PCG · AI · Cinematic · Experimental
> **Scope**: This roadmap covers Dev Infrastructure, GUI Systems, Procedural Content Generation (PCG), AtlasForgeAI integration, Advanced Cinematic & Visualization systems, and Experimental/Futuristic features. No game-specific mechanics, combat, economy, or gameplay features are included until all pillars are solid and refined.

---

## Current State Snapshot

| Pillar | Status | Notes |
|--------|--------|-------|
| Dev Infrastructure | 🔧 In Progress | Build system solid; spaghetti cleanup ~40% done |
| GUI System | ✅ Foundation complete | Widget system, DSL, layout solver, ToolingLayer done |
| PCG | ✅ Foundation complete | WorldGraph, noise, terrain, galaxy done; expansion needed |
| AI / AtlasForgeAI | 🔧 Partial | HttpLLMBackend, editor assistant done; offline loop not built |
| Cinematic & Visualization | 📋 Planned | Cinematic preview, heatmaps, collab editing not started |
| Experimental | 📋 Planned | Multi-agent network, VR/AR, predictive simulation not started |

---

## Roadmap Overview

```
Phase A  →  Phase B  →  Phase C  →  Phase D  →  Phase E  →  Phase F  →  Phase G
  Dev          GUI         PCG       AtlasForgeAI  Integration  Cinematic  Experimental
 Health      Hardening   Expansion   Agent Loop    & Polish    & Viz Sys    & Futuristic
```

All phases can have sub-tasks running in parallel. Phase A must be substantially complete before Phase D. Phases B and C are independent and can advance in parallel. Phases F and G build on all prior phases.

---

## Phase A — Dev Infrastructure & Code Health

> **Goal**: Clean, fast, maintainable codebase that is easy to build, test, and extend. Every later phase depends on this foundation.

### A1 — Spaghetti Code Cleanup (🔧 Active)

Based on `docs/SPAGHETTI_CODE_AUDIT.md`.

- [x] Extract `novaforge_core` static library from CMakeLists.txt (deduplication)
- [x] Split monolithic `test_systems.cpp` (35K lines → 23 per-domain test files)
- [x] Create `test_framework.h` with shared `assertTrue`, `approxEqual`, `addComp`
- [x] Create `json_helpers.h` for shared JSON parsing across data layer
- [x] Begin `SingleComponentSystem<C>` template migration (CapacitorSystem, ShieldRechargeSystem)
- [ ] Complete `SingleComponentSystem<C>` migration across all applicable systems (~60 remaining)
- [ ] Create `StateMachineSystem<C, PhaseEnum>` template for phase-driven systems
- [ ] Create `RechargeSystem<C>` template for recharge-pattern systems
- [ ] Migrate all 164 server systems to appropriate template bases
- [ ] Create `TestFixture` base class to eliminate remaining test boilerplate
- [ ] Decompose `GameSession` god object into `CombatHandler`, `StationHandler`, `MovementHandler`, `MissionHandler`, `ScannerHandler`
- [ ] CMakeLists.txt: reduce from 668 to < 250 lines; per-subdirectory CMakeLists files

**Target metrics:**
- Largest single source file: < 500 lines
- Average system boilerplate: ~20 lines (down from ~80)
- `GameSession` forward declarations: 0
- CMakeLists.txt lines: < 250

---

### A2 — Build System Consolidation

- [ ] Unify `build.sh` across Atlas-NovaForge and Nova-Forge-Expeditions into a single parameterized script
- [ ] Add `./build.sh ai` target — builds AtlasForgeAI Python stubs alongside C++ targets
- [ ] Add `./build.sh tools` target — builds font atlas generator, contract scanner, replay minimizer standalone
- [ ] Add incremental build support: skip unchanged modules via CMake dependency tracking
- [ ] Add build timing breakdown (per-module compile time report at end of build)
- [ ] Docker-based reproducible build environment (`Dockerfile.dev`) for Linux/Windows cross-build
- [ ] One-command dev environment bootstrap: `./setup.sh --full` installs all dependencies, builds all targets, runs all tests
- [ ] Build artifact signing for certified builds (hash + timestamp manifest)

---

### A3 — Test Infrastructure

- [ ] Per-domain CTest suites: `ctest -R gui`, `ctest -R pcg`, `ctest -R ai` run only relevant tests
- [ ] Add fuzz test harness for GUI DSL parser (libFuzzer or AFL integration)
- [ ] Property-based test harness for PCG node evaluation (random seeds, invariant checks)
- [ ] Add headless UI replay test runner to CI (run all `.atlasreplay` files, assert frame hashes)
- [ ] Add mutation testing target (verify test suite catches real bugs by mutating source)
- [ ] Automated test coverage reporting (gcov/llvm-cov → HTML report in `dist/coverage/`)
- [ ] Contract scanner in CI: fail PR if any layer-boundary violation detected
- [ ] Golden replay CI gate: determinism check on every merge to main

---

### A4 — Developer Tooling

- [ ] `atlas doctor` CLI — diagnose build environment, missing deps, config issues
- [ ] `atlas new-system <name>` CLI — scaffold a new ECS system from template with header, source, test file
- [ ] `atlas new-tool <name>` CLI — scaffold a new editor ToolingLayer tool
- [ ] `atlas new-node <type> <name>` CLI — scaffold a new PCG/UI/Flow graph node
- [ ] Live diagnostics server: expose engine metrics over HTTP (tick rate, memory, system timings) for browser-based monitoring
- [ ] Dev console autocomplete: all registered system names, component types, and PCG node types available via tab completion
- [ ] `atlas diff <replay_a> <replay_b>` CLI — compare two replay files and output divergence report
- [ ] `atlas audit` CLI — run contract scan + determinism check + schema validation in one command

---

### A5 — Documentation & Conventions

- [ ] Auto-generated API docs from source (Doxygen or custom extractor → `docs/api/`)
- [ ] Architecture decision record (ADR) template and process (`docs/adr/` directory)
- [ ] Naming conventions lint rule in CI (enforce `ATLAS_NAMING_CONVENTIONS.md` rules automatically)
- [ ] Living changelog: auto-generated from commit messages, filtered by system tag
- [ ] Contribution guide updated: new contributor bootstrap, first system tutorial, coding contract

---

## Phase B — GUI System Hardening & Expansion

> **Goal**: A production-grade, extensible GUI system that covers all editor and game UI needs with live preview, full theming, animation, and accessibility support. The current foundation is solid; this phase refines and extends it significantly.

### B1 — Widget System Expansion

Current widgets: Panel, Button, Text, Image, List, SlotGrid, InputField.

- [ ] `Checkbox` widget — binary toggle with label, deterministic state
- [ ] `RadioGroup` widget — mutually exclusive option set
- [ ] `Dropdown` / `ComboBox` widget — scrollable option list with search filter
- [ ] `Slider` widget — float/int range input with snap-to-grid support
- [ ] `ProgressBar` widget — determinate and indeterminate modes
- [ ] `TreeView` widget — collapsible hierarchical list (used by ECS inspector, asset browser)
- [ ] `Tooltip` widget — deferred hover text with configurable delay and placement
- [ ] `ContextMenu` widget — right-click popup with nested submenu support
- [ ] `ColorPicker` widget — HSV wheel + RGB hex input (for material editor, theme editor)
- [ ] `NumberField` widget — typed numeric input with increment/decrement arrows and unit labels
- [ ] `SplitView` widget — user-resizable two-pane view (replaces current hardcoded split nodes)
- [ ] `ScrollArea` widget — virtualized scrolling for large lists (1000+ items at no fps cost)
- [ ] `Notification` widget — toast messages with timeout and action buttons
- [ ] `Modal` / `Dialog` widget — blocking overlay with focus trap and keyboard dismiss
- [ ] `TabBar` widget — refactor existing tab groups into a first-class widget
- [ ] `Canvas2D` widget — arbitrary 2D draw surface for graph editors and curve editors

---

### B2 — Layout Solver Hardening

- [ ] Anchored layouts: pin edges to parent (top/bottom/left/right/center) in addition to ratio splits
- [ ] Min/max constraints per axis independently (width range separate from height range)
- [ ] Flex-style wrapping: horizontal/vertical lists that wrap onto new lines when space runs out
- [ ] Overflow handling: scroll, clip, or expand parent — configurable per widget
- [ ] Responsive breakpoints: define layout variants at viewport width thresholds (useful for wide-monitor vs narrow panel layouts)
- [ ] Multi-monitor awareness: correct DPI scaling per monitor, no blurry text on mixed-DPI setups
- [ ] Layout solver formal invariant tests: automated proofs that No-Overlap and Full-Coverage hold after every operation
- [ ] Layout performance budget: O(n) solver pass even for deeply nested trees; profile regression test

---

### B3 — Theme Engine

- [ ] Theme definition format: `theme.atlas.json` with color palette, spacing scale, border radii, font stack
- [ ] Multiple built-in themes: `atlas-dark` (current), `atlas-light`, `atlas-high-contrast`
- [ ] Live theme hot-reload: change `theme.atlas.json` → UI updates without restart
- [ ] Per-panel theme overrides: individual tools can override specific theme tokens
- [ ] Theme editor panel (in ToolingLayer): visual editor for all theme tokens with live preview
- [ ] Token-based system: all widget styles reference semantic tokens (`color.accent`, `spacing.md`) not literal values
- [ ] CSS-like specificity: widget type < class < instance override chain
- [ ] Icon system: SVG icon set compiled to a deterministic sprite atlas; no hardcoded pixel positions
- [ ] DPI-aware icons: 1x / 2x / 3x variants, selected automatically by display DPI

---

### B4 — Animation & Transitions

- [ ] Widget transition system: fade-in/out, slide, scale — expressed as deterministic keyframe data, not float lerps
- [ ] Panel open/close animations: ToolingLayer tools animate in/out smoothly
- [ ] Scroll physics: configurable inertia and easing for scroll areas
- [ ] Loading spinners and skeleton loaders: for async operations (asset loading, AI generation)
- [ ] Cursor-responsive interactions: hover highlight, press scale, release bounce — all data-driven
- [ ] Replay compatibility: all animation state is recorded and replayable (no hidden float accumulator)

---

### B5 — GUI DSL Enhancements

- [ ] `@include` directive: split large layout files into composable fragments
- [ ] `@theme` directive: declare which theme a layout uses
- [ ] Conditional blocks: `@if BUILD_FLAG { ... }` for platform-specific UI variants
- [ ] Data binding syntax: `text = "{{entity.name}}"` — bind widget properties to ECS data
- [ ] Event handler syntax: `on_click = "cmd:OpenInventory"` — link widget events to command bus
- [ ] DSL formatter / pretty-printer: auto-format layout files on save
- [ ] DSL language server: autocomplete and error checking in VS Code via LSP adapter
- [ ] Schema migration: `migrate_dsl.py` tool to upgrade layout files between DSL versions
- [ ] DSL round-trip fidelity test: parse → serialize → compare, must be identical

---

### B6 — Editor UI Panels Refinement

> Refine existing panels; no new game-specific panels.

- [ ] **ECS Inspector**: component tree with inline editing, entity search, pin-entity feature
- [ ] **Graph Editor**: minimap, multi-node select + drag, node group/comment boxes, port type colors
- [ ] **Asset Browser**: thumbnail previews, tag-based filtering, drag-to-scene placement
- [ ] **Console**: REPL with history, syntax highlighting, multi-line input, output filtering by severity
- [ ] **Profiler Panel**: flame graph view, per-system allocation tracking, GPU timing integration
- [ ] **Atlas Assistant Panel**: streaming response display, prompt library, conversation history, diff preview inline
- [ ] **Replay Timeline**: waveform-style input visualization, multi-track comparison, divergence markers
- [ ] **Proof Viewer**: TLA+ syntax highlighting with error underlines, invariant status badges
- [ ] **CI Dashboard**: per-workflow run status, test result breakdown, coverage delta
- [ ] **Theme Editor**: live token editor with immediate preview (new panel)
- [ ] **DSL Editor**: syntax-highlighted `.atlas.layout` file editor with parse errors inline (new panel)

---

### B7 — Headless & CI GUI Mode

- [ ] Headless GUI tick: run full widget tree evaluation per tick with no renderer (already exists for server mode) — expand test coverage
- [ ] Snapshot testing: capture widget tree state at tick N as JSON; compare against golden snapshot
- [ ] Visual regression testing: render GUI to offscreen framebuffer in CI; pixel-compare against golden images
- [ ] GUI coverage report: which widgets/panels were exercised in each test run
- [ ] Replay video export: CI pipeline exports `.mp4` from headless replay + GUI replay combined

---

### B8 — Voice-Driven Overlay Commands

- [ ] Extend existing `VoiceCommandRegistry` into the ToolingLayer overlay: speak commands to trigger AI actions, spawn assets, or run PCG
- [ ] "Show heatmap" / "open profiler" / "generate terrain" — voice triggers mapped to overlay tool commands
- [ ] Push-to-talk toggle (`Alt+V`) to avoid accidental activation
- [ ] Voice command visualizer: show recognized phrase and matched intent in overlay before executing
- [ ] Context-aware grammar: available voice commands adapt to which tool panel is active
- [ ] Voice command confidence threshold: low-confidence interpretations shown for confirmation before apply
- [ ] Voice command log panel: scrollable history of recognized commands and their results

---

### B9 — Adaptive Contextual Hotkeys

- [ ] AI observes which operations a developer performs most in each panel — surfaces top-5 as suggested hotkeys
- [ ] Hotkey conflict resolver: detect collisions across all active tools and propose alternatives
- [ ] Per-context hotkey profiles: different bindings activate when in WorldGraph editor, Script Console, or Cinematic Editor
- [ ] Hotkey macro recording: record a sequence of actions and bind it to a single key
- [ ] `HotkeyActionManager` expansion: all new tools auto-register their actions with the manager on load
- [ ] Visual hotkey overlay: press `Ctrl+?` to see a context-sensitive cheatsheet of all active bindings

---

### B10 — Procedural Widget & Gizmo Designer

- [ ] **Gizmo authoring DSL**: describe custom 3D manipulator shapes in text format — compiled to GPU draw calls
- [ ] **Widget composer**: mix-and-match existing widget primitives into new composite widgets via a visual node editor
- [ ] **Context-sensitive gizmos**: gizmo shape, color, and interaction handles change based on selected entity type (ship hull vs. terrain patch vs. PCG node)
- [ ] **Interactive handles**: custom drag handles on gizmos feed directly into PCG parameters (drag a noise amplitude handle to tune terrain)
- [ ] Gizmo LOD: simplify manipulator geometry as camera moves away, ensuring always-visible but never distracting
- [ ] Gizmo replay: all gizmo interactions recorded as semantic events and replayable deterministically
- [ ] Gizmo library panel: browseable palette of built-in and project-specific gizmos

---

### B11 — Procedural HUD & Game UI Generation

- [ ] **AI HUD generator**: describe the HUD in plain English → AtlasForgeAI produces a complete Widget DSL layout
- [ ] **Role-based HUD variants**: generate HUD layouts per player role (pilot HUD vs. FPS HUD vs. fleet commander view) from a single description
- [ ] **Live HUD prototyping**: WYSIWYG HUD editor with drag-and-drop widget placement, bound to live ECS data
- [ ] **HUD animation authoring**: define transition animations for HUD elements (power-up flash, damage vignette) as data curves
- [ ] **HUD test harness**: simulate ECS state changes (shield hit, capacitor drain) and watch HUD respond in the editor without running the game
- [ ] **HUD export**: serialize authored HUD to Widget DSL file; auto-validated against widget schema before save

---

### B12 — AI-Assisted UI Skinning

- [ ] **Theme generation from description**: "dark industrial sci-fi with blue accents" → AtlasForgeAI generates a complete `theme.atlas.json`
- [ ] **Color harmony engine**: AI enforces WCAG contrast ratios and color harmony rules on all generated palettes
- [ ] **Style transfer**: upload a reference image (screenshot, concept art) → AI extracts color palette and applies it as a theme
- [ ] **Accessibility audit**: AI scans current theme for contrast failures, small touch targets, and missing focus indicators
- [ ] **Per-faction UI skins**: generate distinct UI skin variants per faction tag (Terran, Pirate, Corporate) from a base theme
- [ ] **Skin preview grid**: show all registered theme variants side-by-side in Theme Editor for quick comparison

---

## Phase C — PCG System Expansion & Refinement

> **Goal**: A comprehensive, data-driven, AI-augmentable PCG pipeline covering worlds, interiors, props, materials, sounds, and placement. All generators are graph-based, seed-reproducible, and hot-reloadable.

### C1 — WorldGraph Node Library Expansion

Current nodes: Perlin, FBM, HeightField, DensityField, Mask, cube-sphere, voxel, terrain mesher.

- [ ] **Voronoi** node — Worley noise with configurable distance metric (Euclidean, Manhattan, Chebyshev)
- [ ] **Domain Warp** node — warp input coordinates before evaluation (organic terrain shapes)
- [ ] **Erosion** node — hydraulic and thermal erosion simulation on heightfields
- [ ] **Ridged FBM** node — sharp mountain ridges
- [ ] **Curl Noise** node — divergence-free noise for fluid-like flow patterns
- [ ] **Blend** node — weighted mix of two value channels with mask
- [ ] **Remap** node — curve-based remap with editable spline (replaces hardcoded linear remaps)
- [ ] **Threshold** / **Classify** node — segment a field into discrete biome bands
- [ ] **Scatter** node — Poisson-disc or stratified sample placement on a surface
- [ ] **Slope** node — derive slope angle from a heightfield gradient
- [ ] **Curvature** node — local concavity/convexity of a surface (useful for erosion, snow, moss placement)
- [ ] **Distance Field** node — signed distance from a shape primitive or mesh
- [ ] **Mask From Biome** node — combine multiple threshold/classify outputs into a biome mask
- [ ] **Combine** node — boolean union/intersection/subtraction of density fields
- [ ] **Tile** node — repeat a sub-graph result across a grid with optional variation
- [ ] Node palette categorization in WorldGraph panel: Noise / Shape / Filter / Combine / Output

---

### C2 — Material & Shader PCG

- [ ] **MaterialGraph** node library: albedo gen, normal gen, roughness gen, AO bake, emissive mask
- [ ] Procedural material blending based on biome mask (rock/sand blend at threshold)
- [ ] Triplanar projection node: eliminate UV stretching on steep terrain
- [ ] Detail material layer: close-up surface variation composited on base material
- [ ] Shader graph hot-reload: change `.atlasshader` → GPU recompile in < 1 second
- [ ] Shader graph debugger: visualize intermediate channel values per-pixel in editor
- [ ] `AtlasShaderIR` determinism test: verify shader output is bit-identical across platforms
- [ ] Material LOD graph: automatically simplify material as camera distance increases
- [ ] Decal system graph: layered world-space decals (damage, weathering, markings)

---

### C3 — Interior & Structural PCG

> For modular architectural space generation (ships, stations, bunkers). Not FPS gameplay — this is the generator system itself.

- [ ] **Grammar-based room graph**: L-system / shape grammar rules that produce room connectivity graphs
- [ ] **Room solver**: pack room graph into 3D space respecting adjacency and size constraints
- [ ] **Corridor connector**: A*-based corridor routing between room entrances
- [ ] **Module palette system**: define a set of modular mesh pieces (wall, corner, door, window) per style tag
- [ ] **Style tag propagation**: assign style (industrial, sci-fi, derelict) at region level; all modules within inherit
- [ ] **Interior detail scatter**: place props, decals, and lights within rooms via scatter + constraint nodes
- [ ] **Structural integrity simulation**: detect floating or unsupported rooms; auto-reinforce
- [ ] **Interior→WorldGraph bridge node**: interior PCG result feeds back into world streaming as a chunk type

---

### C4 — Placement & Scatter Learning

- [ ] **PCG Placement Learner**: observe manual placements in MapEditorTool → extract placement rules as WorldGraph parameters
- [ ] **Constraint-based scatter**: define keep-out zones, required clearances, and surface type requirements per object class
- [ ] **Density map**: author per-area object density with a painted mask in the editor
- [ ] **Hierarchical scatter**: coarse scatter of large objects → fine scatter of small objects that avoid large object footprints
- [ ] **Variation seeding**: each placed instance receives a unique sub-seed for local variation (rotation, scale, material variation)
- [ ] **Placement history log**: every PCG-placed object records its seed, rules applied, and position — enables exact reproduction and delta editing
- [ ] **DeltaEditStore integration**: manual overrides layer on top of PCG placements; PCG re-runs preserve overrides

---

### C5 — PCG Performance & Streaming

- [ ] Async PCG evaluation: WorldGraph nodes execute on worker threads; results stream into the chunk cache
- [ ] GPU-accelerated nodes: noise, erosion, and heightfield nodes optionally run as compute shaders
- [ ] Node result caching: content-addressed cache keyed by (node hash + input hash); skip re-evaluation when inputs unchanged
- [ ] Progressive LOD generation: low-resolution result available immediately; high-resolution refines in background
- [ ] Memory budget tracking: each PCG node declares output memory bounds; total per-chunk budget enforced
- [ ] PCG profiler panel: per-node execution time, cache hit rate, memory usage — integrated in Profiler Panel

---

### C6 — PCG Quality & Debugging

- [ ] **WorldGraph debugger**: breakpoints on nodes, single-step execution, per-node output visualized in viewport
- [ ] **Seed explorer**: UI to sweep a parameter over a range and thumbnail the results
- [ ] **Biome preview panel**: show biome mask, height distribution, moisture, and slope as 2D color maps
- [ ] **Determinism stress test**: run the same WorldGraph with the same seed 1000 times on multiple threads; assert bit-identical output
- [ ] **PCG validation node**: declare output invariants (height in [0,1], density non-negative) — fail fast on rule violation
- [ ] **Regression suite**: golden heightfield snapshots per test scene; CI fails if PCG output drifts

---

### C7 — Audio PCG

- [ ] **SoundGraph expansion**: new nodes for granular synthesis, spectral morphing, and convolution reverb
- [ ] **Environment audio system**: ambient soundscape generated from biome mask (wind strength from height + slope, water from moisture)
- [ ] **Procedural music graph**: melody/harmony/rhythm composed from game state parameters
- [ ] **Adaptive music system**: music graph responds to engine state variables (tension level, location type, time of day) — no game logic, pure engine signal
- [ ] SoundGraph debugger: oscilloscope and frequency spectrum visualization per node
- [ ] Offline audio baking: pre-render common SoundGraph states to `.wav` cache for runtime performance

---

### C8 — Procedural Ecosystem & Weather Systems

> These are PCG *generator frameworks* — not gameplay systems. They produce data consumed by the renderer and world simulation.

- [ ] **Weather graph**: DAG-based weather pattern generator with nodes for pressure, humidity, temperature, wind — all seeded and deterministic
- [ ] **Precipitation node**: derive rain, snow, or fog from weather graph outputs; output density field consumed by renderer
- [ ] **Day/night cycle graph**: parameterized sky color, sun angle, and ambient light curves; authored as spline nodes
- [ ] **Seasonal variation**: blend weather and biome parameters across a configurable seasonal cycle
- [ ] **Flora placement graph**: scatter trees, grass, and undergrowth using biome mask + slope + moisture constraints
- [ ] **Fauna rig placement**: place animated rig anchor points (roost, burrow, patrol path) via scatter graph; no AI behavior — data only
- [ ] **Environmental LOD**: flora and weather details streamed at distance using the existing WorldStreamer infrastructure
- [ ] **Ecosystem balance validator**: check that flora density, water sources, and elevation form a plausible ecosystem; surface warnings in PCG debugger

---

### C9 — Drag-and-Drop Procedural Placement UI

- [ ] **AI-generated object placement in viewport**: drag a WorldGraph-generated prop (console, container, antenna) from Asset Palette into the 3D viewport
- [ ] **Smart snap**: AI calculates valid placement surface (floor, wall, ceiling) and snaps automatically; no manual axis alignment
- [ ] **Scale inference**: AI infers appropriate scale from surrounding context (a console next to a door vs. a cargo bay)
- [ ] **Placement rule learning**: after several manual placements, AI extracts a placement rule and offers to add it to the WorldGraph scatter node
- [ ] **Override stack**: AI-placed objects are delta overrides on top of PCG seeds — PCG regeneration preserves manual placements
- [ ] **Multi-object placement**: box-select a region, choose a prop category, AI scatters them respecting clearance rules
- [ ] **Undo/redo**: all drag-and-drop placements go through `EditorCommandBus`; fully undoable

---

### C10 — Cross-Project PCG Knowledge Transfer

- [ ] **PCG style library**: save a WorldGraph configuration (biome rules, scatter params, noise stack) as a named style asset
- [ ] **Cross-project import**: load a style asset from a different Atlas project into the current WorldGraph palette
- [ ] **Style diff viewer**: compare two PCG style presets side-by-side to understand differences
- [ ] **Improvement propagation**: if a style is refined in one project, a merge-request style notification appears in other projects that use it
- [ ] **AI style tagging**: AI automatically tags saved styles (e.g., "volcanic", "arctic", "urban ruins") for searchability
- [ ] **Community-compatible export**: styles export as standalone `.atlas.pcgstyle` files — shareable without the full project

> **Goal**: Build the full AtlasForgeAI agent loop — an offline-first, multi-language, media-capable AI development assistant that integrates with the engine's ToolingLayer, PCG systems, and build pipeline. This is the "Swiss-army knife" dev system described in the Chat.md planning sessions.

### D1 — Offline LLM Integration

- [ ] **Ollama adapter**: `OllamaLLMBackend` implementing `IAssistantBackend` — connects to local Ollama server on `localhost:11434`
- [ ] **llama.cpp adapter**: `LlamaCppLLMBackend` — direct in-process llama.cpp embedding (no server required)
- [ ] **Model manager panel** (ToolingLayer tool): list available local models, pull new models, set active model
- [ ] **Backend fallback chain**: `[LocalLLM → HttpLLM → OfflineStub]` — auto-downgrade gracefully
- [ ] **Context window budgeting**: estimate token count before sending; truncate context intelligently
- [ ] **Model capability registry**: per-model caps (context length, code quality tier, speed) — influence prompt strategy
- [ ] **Offline knowledge base**: local vector index of engine docs, schema files, and source headers — queried before LLM
- [ ] Documentation indexer: `atlas index-docs` CLI command re-indexes `docs/` and engine headers into local knowledge base
- [ ] **First-run model download**: `./setup.sh --ai` pulls a default coding model (DeepSeek Coder or Code Llama) via Ollama

---

### D2 — AtlasForgeAI Agent Core

The agent loop: read prompt → decide action → call tools → parse response → apply → loop.

- [ ] `AtlasForgeAI/core/agent_loop.py` — main agent loop with tool dispatch
- [ ] `AtlasForgeAI/core/context_manager.py` — build structured context snapshots for LLM prompts
- [ ] `AtlasForgeAI/core/response_parser.py` — parse structured LLM output (JSON tool calls, DSL, AST diffs)
- [ ] `AtlasForgeAI/core/approval_gate.py` — every agent action requires explicit user approval before apply
- [ ] `AtlasForgeAI/core/action_history.py` — undo stack for applied AI actions
- [ ] Agent loop integrated into ToolingLayer: `Alt+F12` toggles AtlasForgeAI overlay panel
- [ ] Streaming response display: tokens appear in real-time as the model generates
- [ ] Interrupt support: user can stop a generation mid-stream
- [ ] Session persistence: agent context saved across editor restarts

---

### D3 — Tool System

All tools are sandboxed; the agent may only call tools that have been explicitly registered and granted permission.

**File Tools**
- [ ] `tool_read_file(path)` — read file content into agent context
- [ ] `tool_write_file(path, content)` — write file with diff preview and user approval
- [ ] `tool_patch_file(path, diff)` — apply a unified diff to a file
- [ ] `tool_list_dir(path)` — list directory contents
- [ ] `tool_search_code(query, path_glob)` — grep-style search across project files

**Build Tools**
- [ ] `tool_build(target, config)` — trigger `./build.sh <target>` and capture output
- [ ] `tool_run_tests(filter)` — run `ctest -R <filter>` and return results
- [ ] `tool_compile_check(file)` — syntax-check a single file without full build

**Editor Tools**
- [ ] `tool_spawn_entity(component_json)` — create an entity via EditorCommandBus
- [ ] `tool_add_component(entity_id, component_json)` — add component via command bus
- [ ] `tool_set_property(entity_id, component, field, value)` — set component property
- [ ] `tool_get_ecs_snapshot()` — return current ECS state as JSON context

**PCG Tools**
- [ ] `tool_run_worldgraph(graph_path, seed)` — evaluate a WorldGraph and return output summary
- [ ] `tool_set_node_param(graph, node, param, value)` — tweak a WorldGraph node parameter
- [ ] `tool_preview_pcg(graph_path, seed)` — generate a 2D thumbnail of PCG output

**Asset Tools**
- [ ] `tool_list_assets(type_filter)` — list registered assets by type
- [ ] `tool_generate_asset(schema, params)` — create a new asset file from a schema template
- [ ] `tool_validate_asset(path)` — run schema validation on an asset file

---

### D4 — Code Generation Pipeline

- [ ] **System scaffolding**: given a description, generate header + source + test for a new ECS system
- [ ] **Component scaffolding**: generate a component struct with serialization and schema
- [ ] **WorldGraph node scaffolding**: generate a new PCG node with typed ports and evaluation stub
- [ ] **GUI layout generation**: generate `.atlas.layout` DSL from natural language description
- [ ] **FlowGraph generation**: generate a Flow Graph JSON from description of desired behavior
- [ ] **Schema generation**: generate `.atlas.json` schema from a description of data
- [ ] **Diff-first workflow**: all generated code shown as unified diff before any file is written
- [ ] **Iterative refinement**: "make the shield regeneration faster" → agent diffs the specific lines only
- [ ] Multi-language stubs:
  - [ ] C++ system/component/node stubs
  - [ ] Python tool/pipeline stubs
  - [ ] Lua script stubs
  - [ ] GLSL/AtlasShader stubs

---

### D5 — Dev Mode Overlay

The in-engine, always-accessible AI development interface.

- [ ] `Alt+F12` opens/closes the full AtlasForgeAI Dev Mode overlay
- [ ] Overlay layout (split into four zones):
  - **Top**: prompt input bar with context selector (World / Assets / Code / ECS)
  - **Left**: agent conversation history with diff previews inline
  - **Center**: active game viewport (unobscured — overlay is transparent/framed)
  - **Right**: tool call log + action history + undo button
- [ ] Context-aware prompt suggestions: overlay shows relevant prompts based on current editor panel
- [ ] "Ask about this" right-click: right-click any asset, node, entity, or panel → "Ask AtlasForgeAI" pre-fills context
- [ ] Live code preview: generated code shown in a split editor pane before apply
- [ ] Hot-reload integration: after file write, trigger hot-reload automatically (with user consent)
- [ ] Dev Mode persistence: window layout, conversation, and pending diffs survive engine restarts

---

### D6 — Multi-Language Build Runners

- [ ] `AtlasForgeAI/runners/cpp_runner.py` — invoke cmake/make/msbuild, parse errors, return structured result
- [ ] `AtlasForgeAI/runners/python_runner.py` — run Python scripts, capture stdout/stderr, timeout
- [ ] `AtlasForgeAI/runners/lua_runner.py` — run Lua scripts via luajit or lua5.4
- [ ] `AtlasForgeAI/runners/glsl_runner.py` — compile GLSL via glslangValidator, parse errors
- [ ] Runner error parser: extract file/line/message from compiler output for each language
- [ ] Error-fix loop: on build failure, agent automatically reads the error, diffs a fix, and prompts for approval
- [ ] Compiler feedback integration: build errors surfaced inline in AI conversation as clickable links

---

### D7 — Media Generation Pipelines

> Offline, open-source only. No cloud dependencies.

**Texture Generation**
- [ ] `AtlasForgeAI/media/texture_pipeline.py` — Stable Diffusion interface via local API (ComfyUI or AUTOMATIC1111)
- [ ] `AtlasForgeAI/media/texture_upscaler.py` — ESRGAN-based upscaling to power-of-two resolution
- [ ] `AtlasForgeAI/media/normal_map_gen.py` — generate normal map from albedo (NormalMap-Online algorithm)
- [ ] `AtlasForgeAI/media/texture_validator.py` — check format, size, power-of-two, mip count
- [ ] Texture output → asset registry: generated textures auto-registered as `.atlasb` assets

**3D Mesh Generation**
- [ ] `AtlasForgeAI/media/blender_bridge.py` — headless Blender integration via `blender --python`
- [ ] `AtlasForgeAI/media/mesh_generator.py` — generate simple mesh variants (procedural ship hulls, props) via Blender Python API
- [ ] Mesh output → asset pipeline: generated `.glb`/`.obj` cooked into `.atlasb` binary format

**Audio Generation**
- [ ] `AtlasForgeAI/media/tts_pipeline.py` — text-to-speech via Coqui TTS (offline, no cloud)
- [ ] `AtlasForgeAI/media/sfx_generator.py` — SFX generation via AudioCraft/MusicGen (offline)
- [ ] `AtlasForgeAI/media/audio_validator.py` — check sample rate, bit depth, channel count
- [ ] Audio output → SoundGraph integration: generated clips available as SoundGraph source nodes

---

### D8 — PCG Learning & AI-Driven PCG

- [ ] **Style profiler**: analyze manually authored WorldGraph outputs and extract parameter statistics
- [ ] **Similarity search**: given a reference scene, find WorldGraph configurations that produce similar output
- [ ] **AI node suggestion**: in WorldGraph panel, AtlasForgeAI suggests next nodes based on current graph topology
- [ ] **Parameter tuning**: "make terrain more mountainous" → agent identifies and tweaks relevant noise parameters
- [ ] **Biome description to graph**: "arctic plateau with volcanic ridges" → AtlasForgeAI generates a WorldGraph DSL
- [ ] **Placement style learning**: observe manual prop placements → infer scatter density, clearance rules, surface affinity
- [ ] **Feedback loop**: player-area traversal data (from replay) informs density adjustments in next PCG pass

---

### D9 — AtlasForgeAI Safety & Quality

- [ ] **Approval gate hard enforcement**: no file is ever modified without explicit user `[Apply]` click
- [ ] **Sandboxed execution**: build runners and Python tools run in subprocess with timeout and resource limits
- [ ] **Allowlist-based filesystem access**: agent may only access paths under project root and workspace
- [ ] **No secrets in context**: API keys, passwords, and env vars are scrubbed from context before sending to LLM
- [ ] **Output validation**: generated assets are schema-validated before apply; invalid output rejected
- [ ] **Determinism contract**: AI-generated assets are hashed and logged; replays never depend on AI presence
- [ ] **Audit log**: every tool call, approval, and applied action written to `workspace/ai_audit.log`
- [ ] **Rollback**: `atlas ai undo` reverts the last N applied AI actions using git-style patch reversal

---

### D10 — Multi-Agent Collaboration System

> Multiple AtlasForgeAI agent instances working in parallel on different concerns — code, assets, PCG, QA — with a shared coordination layer.

- [ ] **Agent registry**: named agents with designated domains (`code-agent`, `pcg-agent`, `asset-agent`, `qa-agent`)
- [ ] **Task dispatcher**: break a developer prompt into sub-tasks and route each to the appropriate agent
- [ ] **Inter-agent message bus**: agents communicate structured results (e.g., pcg-agent completes biome → asset-agent picks up to generate matching textures)
- [ ] **Conflict negotiation**: if two agents propose changes to the same file, the orchestrator presents a merge UI for developer resolution
- [ ] **Multi-Agent Collaboration Panel** (ToolingLayer tool): visualize all active agents, their current tasks, progress bars, and pending diffs
- [ ] **Parallel approval queue**: developer sees all pending agent diffs in one queue; approve, reject, or defer individually
- [ ] **Agent memory sharing**: completed tasks are stored in a shared session memory; new agents start informed
- [ ] **Agent priority tiers**: critical path agents (code, build) get higher token budgets and faster model tiers
- [ ] **Agent simulation mode**: run agents against a snapshot of the project (not live files) for dry-run planning

---

### D11 — Interactive AI Debugger

- [ ] **Step-through execution**: AI can walk through FlowGraph or WorldGraph execution tick-by-tick, explaining each node's decision
- [ ] **Natural language trace**: at each step, AI narrates "Node X fired because Y exceeded threshold Z" — in plain English
- [ ] **Breakpoint injection**: developer sets a breakpoint on a node or ECS system; AI pauses and explains the state at that point
- [ ] **Anomaly detection**: AI flags unusual patterns in replay data (unexpected state transitions, outlier tick durations) and explains them
- [ ] **"Why did this happen?" prompt**: select any entity or event in replay → AI explains the causal chain in plain English
- [ ] **Fix suggestion mode**: after explaining a bug, AI proposes a diff to fix it — same approval-gate workflow as all other AI actions
- [ ] **Debugger panel integration**: Interactive AI Debugger lives as a ToolingLayer panel alongside the existing Graph Editor and Replay Timeline

---

### D12 — Voice Command Macros & Learning

- [ ] **Macro recording**: say "start recording macro", perform actions, say "end macro" — AI captures the sequence as a named command
- [ ] **Macro playback**: say the macro name → AI replays the entire action sequence
- [ ] **Macro editor**: review, rename, delete, and reorder recorded macros in a ToolingLayer panel
- [ ] **AI macro suggestion**: after 3+ repetitions of the same action sequence, AI offers to create a macro automatically
- [ ] **Voice shorthand learning**: user can say "that thing I always do after building" → AI searches macro history for best match
- [ ] **Cross-session macro persistence**: macros saved to `workspace/macros.atlas.json`; loaded on editor start
- [ ] **Macro version control**: macros tracked in git alongside project files; diffs show what changed in a macro

---

### D13 — Dynamic Tutorial Recording & Playback

- [ ] **Tutorial recorder**: developer presses "Record Tutorial", performs a workflow → system captures every UI interaction and AI prompt as an annotated replay
- [ ] **Narration layer**: developer can add spoken or text annotations at any point in the recording
- [ ] **AI tutorial generation**: given a feature name ("WorldGraph biome authoring"), AI generates a step-by-step interactive tutorial using engine docs and recorded examples
- [ ] **Adaptive skill level**: tutorial system detects developer familiarity (beginner / intermediate / advanced) from prior interaction patterns and adjusts depth
- [ ] **Tutorial player panel**: plays back recorded tutorials inside the editor with overlay highlights and tooltips
- [ ] **Task automation**: if a tutorial step is purely mechanical (e.g., "click the Compile button"), AI offers to execute it on behalf of the developer
- [ ] **Tutorial library**: browseable catalogue of all recorded and AI-generated tutorials, searchable by topic and difficulty

---

### D14 — AI Personality Profiles

- [ ] **Personality system**: configurable AI voice, tone, and interaction style — stored in `workspace/ai_personality.atlas.json`
- [ ] **Built-in profiles**: `Mentor` (patient, explanatory), `Partner` (collaborative, terse), `Technical` (precise, code-first), `Creative` (generative, suggestive)
- [ ] **Custom profile authoring**: developer defines a custom personality with tone adjectives, verbosity level, humor allowance, and preferred explanation depth
- [ ] **Context-adaptive tone**: AI shifts from creative to technical automatically when switching from PCG authoring to debugging a compiler error
- [ ] **Per-project personality**: each `.atlas` project file can specify a default personality for team consistency
- [ ] **Personality preview**: test a profile by sending a sample prompt and seeing the response style before committing

---

### D15 — Cross-Project Knowledge Base

- [ ] **Project knowledge index**: each project auto-indexes its own docs, schemas, graph nodes, and source headers into a local vector store
- [ ] **Cross-project import**: explicitly allow the AI to query knowledge from a second project's index (e.g., "use the PCG patterns from AtlasForge")
- [ ] **Knowledge delta tracking**: when a source project updates, the importing project gets a notification to re-sync relevant knowledge
- [ ] **Privacy boundary**: cross-project knowledge transfer requires explicit per-project opt-in; no passive data leakage
- [ ] **Portable knowledge packages**: export a project's AI knowledge base as a `.atlas.knowledgepack` file — distributable alongside code
- [ ] **Knowledge provenance**: every AI suggestion shows which knowledge source it was drawn from (inline citation)

> **Goal**: All four pillars (Dev, GUI, PCG, AI) working together as a seamless whole. The developer experience is fast, predictable, and delightful.

### E1 — AI + GUI Authoring

- [ ] AtlasForgeAI can generate complete layout DSL files from natural language description
- [ ] AI diff preview uses the DSL Editor panel for side-by-side comparison
- [ ] "Explain this panel layout" — AI describes the current DSL structure in plain English
- [ ] "Add a search bar to the Asset Browser" — AI proposes DSL patch, user applies
- [ ] Widget autocomplete powered by local knowledge base (knows all registered widget types and their properties)
- [ ] Theme token suggestions: AI recommends appropriate semantic tokens for new widgets

---

### E2 — AI + PCG Authoring

- [ ] WorldGraph panel: AI context menu on any node — "Explain", "Suggest alternatives", "Tune parameters"
- [ ] "Generate a desert canyon biome" → AtlasForgeAI produces a complete WorldGraph JSON
- [ ] Seed explorer powered by AI: "find 5 seeds that produce interesting mountain formations" → agent runs seed sweeps
- [ ] PCG regression guard: if a WorldGraph edit changes output beyond a threshold, AI flags it as a regression candidate
- [ ] Biome description panel: plain-English biome descriptions auto-linked to WorldGraph parameter sets

---

### E3 — Full Offline Dev Workflow

- [ ] **Zero-cloud development**: entire build, test, AI, and asset generation pipeline runs with no internet
- [ ] **Model package**: `./setup.sh --ai` bundles all required models into `workspace/models/` (DeepSeek Coder + Stable Diffusion + Coqui TTS)
- [ ] **Offline knowledge base auto-build**: `./setup.sh` indexes all docs and headers; no external search needed
- [ ] **Air-gapped CI**: determinism gate, contract scan, and golden replay tests all run without network access
- [ ] `atlas offline-check` — verify all dev dependencies are available offline, report any missing items

---

### E4 — Live Collaboration Hooks

- [ ] ToolingLayer authority model: in multiplayer dev session, only the host can apply PCG or AI changes
- [ ] Live cursor sharing in Graph Editor: see where remote collaborators are editing
- [ ] Conflict detection: if two devs edit the same WorldGraph node concurrently, merge conflict panel activates
- [ ] AI session sharing: dev A prompts AI; dev B sees the proposed diff and can approve or reject
- [ ] EditorEventBus broadcast: all ToolingLayer tool actions broadcast to connected peers in dev session

---

### E5 — Performance & Scalability

- [ ] GUI render budget: frame budget enforced per panel (< 0.5ms per panel at 60fps target)
- [ ] PCG async pipeline: all WorldGraph evaluation off main thread; no frame stalls during streaming
- [ ] AI context streaming: LLM responses piped to UI incrementally; no blocking waits
- [ ] Build parallelism audit: ensure all CMake targets build in parallel on N cores; no unnecessary serialization
- [ ] Memory budget tracking: each subsystem declares and reports allocation; CI fails if budget exceeded

---

### E6 — Final Hardening

- [ ] Full end-to-end test: open editor → author WorldGraph → run AI suggestion → apply PCG → hot-reload → replay — all automated
- [ ] Certified build: hash ladder verification of engine + PCG + AI module versions bundled into release manifest
- [ ] ABI lock: GUI widget enum, event structs, and command bus signatures frozen and documented for plugin compatibility
- [ ] Plugin SDK export: `find_package(AtlasEngine)` provides GUI, PCG, and AI headers for third-party tool development
- [ ] Developer onboarding benchmark: new developer from zero to running editor with AI assistant in < 15 minutes

---

### E7 — Real-Time Performance & Optimization AI

- [ ] **Frame budget monitor**: AI continuously watches frame times per system; alerts when a system exceeds its budget
- [ ] **Automated profiling suggestions**: "Your TerrainMeshGenerator is taking 8ms — here are three optimizations to try" with diff proposals
- [ ] **Shader complexity analysis**: AI reads shader source and estimates instruction count, texture fetches, and branching; flags expensive patterns
- [ ] **Asset over-budget detection**: AI scans scene and flags assets exceeding polycount, texture memory, or draw-call budgets
- [ ] **Memory fragmentation detector**: AI monitors allocator state over time and suggests defragmentation or pool resizing
- [ ] **CI performance gate**: automated performance regression test — fail build if any tracked metric degrades beyond threshold
- [ ] **Optimization history log**: all AI-suggested optimizations and their measured impact stored in `workspace/perf_history.atlas.json`

---

### E8 — Integrated Asset Library

- [ ] **Local asset library panel**: searchable, tagged catalogue of all assets across all registered projects
- [ ] **Category browser**: browse by type (Mesh / Texture / Sound / Graph / Schema / Layout), tag, and project source
- [ ] **Thumbnail preview**: async-generated thumbnails for all mesh and texture assets; cached to disk
- [ ] **In-library testing**: preview a WorldGraph node library or SoundGraph snippet directly from the library panel before placing
- [ ] **AI-driven tagging**: AI auto-generates searchable tags for every asset based on its schema and content
- [ ] **Usage tracking**: each asset reports where it is referenced in the current project
- [ ] **Dependency graph viewer**: show the full dependency chain of any asset (what does this asset depend on? what depends on it?)
- [ ] **Version snapshots**: each asset stores a hash-tagged version history; roll back to any prior state
- [ ] **Cross-project asset sharing**: explicitly flag assets as shareable; other projects can reference them by URI

---

### E9 — Offline Simulation Lab

- [ ] **Headless world sim**: run a WorldGraph + scatter + weather + ecosystem simulation for N ticks without the renderer or game logic
- [ ] **Fleet behavior test harness**: spin up N AI-driven entities with scripted goals; observe emergent movement and resource contention — all offline
- [ ] **PCG stress tester**: generate 1000 unique seeds, render thumbnails, flag seeds with artifacts or invariant violations
- [ ] **Performance sim**: run the engine at 10× speed with all systems active and log per-system timing distribution
- [ ] **Comparison runs**: run the same simulation with two different parameter sets; diff the outputs automatically
- [ ] **Sim lab panel** (ToolingLayer): configure sim runs, track active simulations, browse results, and link back to the causal PCG graphs
- [ ] **Export sim results**: simulation outputs (heatmaps, entity traces, metric CSVs) exported to `workspace/sim_results/`

---

### E10 — Predictive Behavior Modeling (Dev Tool)

- [ ] **Interaction predictor**: given a WorldGraph configuration, AI predicts where generated areas will see the most traversal (based on prior replay data)
- [ ] **Hotspot map**: generate a predicted-interest heat map for a world before any player has touched it
- [ ] **Parameter sensitivity analysis**: "which WorldGraph parameter has the most impact on traversal patterns?" — AI sweeps and reports
- [ ] **Bottleneck predictor**: given a corridor layout from Interior PCG, AI identifies likely flow bottlenecks based on geometry
- [ ] **Developer use modeling**: AI tracks which editor panels and tools each developer uses most → personalizes suggested workflows
- [ ] **All results are dev-tooling only**: no predicted data modifies runtime behavior; purely an authoring aid

---

### E11 — Real-Time Visual Debugging & Heatmaps

- [ ] **System activity heatmap overlay**: in viewport, colorize world regions by the density of ECS system ticks (hot = many systems active)
- [ ] **PCG event frequency map**: visualize how often each WorldGraph node was evaluated per region over the last N frames
- [ ] **Broken asset highlighter**: AI scans the scene each tick and outlines entities with missing components, null references, or schema violations
- [ ] **Misaligned component detector**: flag entities where transform hierarchy is inconsistent with physics body
- [ ] **Memory usage overlay**: viewport tint showing memory cost per streaming chunk (red = near budget, green = safe)
- [ ] **AI agent activity overlay**: visualize all active AtlasForgeAI tool calls as floating indicators above their affected entities
- [ ] **Heatmap recording**: record heatmap state over time as a replay layer; scrub through the Replay Timeline to see heatmaps evolve
- [ ] **Custom overlay API**: `DiagnosticsOverlay::RegisterHeatmapLayer(name, samplerFn)` — any subsystem can contribute a custom heatmap layer

---

## Phase F — Advanced Cinematic & Visualization Systems

> **Goal**: Give developers and artists a cinematic preview and real-time editing experience directly inside the engine — no external software needed. Procedural cinematics, live camera tools, and multiplayer-safe collaborative editing.

### F1 — Integrated Cinematic Preview

- [ ] **Cinematic Editor panel** (ToolingLayer tool): a timeline-based camera sequence editor embedded in the engine
- [ ] **Camera track authoring**: add keyframes for position, rotation, FOV, and focus distance on a scrub timeline
- [ ] **Live preview mode**: press Play in the Cinematic Editor to fly the authored camera path through the current world scene
- [ ] **Multiple camera modes**: static cut, dolly, orbit, crane, handheld shake — all authored as curve data
- [ ] **Lighting preview**: toggle between day/night or weather states in the Cinematic Editor without affecting game state
- [ ] **Depth-of-field authoring**: set focus distance and bokeh radius per keyframe; preview in real-time
- [ ] **Cinematic export**: render the authored camera sequence to an image sequence or `.mp4` (headless, no game running)
- [ ] **AI framing suggestions**: AtlasForgeAI analyzes the scene and suggests camera positions that frame the most interesting elements
- [ ] **AI dynamic story beats**: given a sequence of world events (from replay data), AI proposes a matching cinematic cut sequence

---

### F2 — Autonomous Procedural Cinematics

> The engine can self-direct cinematic sequences from world events — no hand-authored keyframes required.

- [ ] **Event-to-camera mapper**: define rules like "explosion event → cut to wide shot of source position + 2s hold + dolly in"
- [ ] **Cinematic grammar system**: library of shot types (establishing, reaction, cutaway, reveal) expressed as parameterizable templates
- [ ] **AI director mode**: AI selects shot type, camera angle, and duration based on current world state and prior shots (avoids repetition)
- [ ] **Narrative pacing engine**: AI adjusts cut frequency based on action intensity — calmer exploration = long, slow shots; intense sequences = rapid cuts
- [ ] **Cinematics replay layer**: all auto-directed cinematics recorded as a replay layer; scrub and review in Replay Timeline
- [ ] **Procedural subtitle/caption system**: AI generates contextual caption overlays for automated sequences (location names, event descriptions)

---

### F3 — Real-Time Collaborative Editing

- [ ] **Shared editor session**: two or more developers open the same project; changes propagate in real-time over the existing NetContext
- [ ] **Presence indicators**: each connected developer has a colored cursor/outline visible to peers in the viewport and graph editors
- [ ] **Role locks**: developer A edits WorldGraph; developer B edits an asset — AI detects and warns about cross-boundary dependencies
- [ ] **Live diff feed**: a feed panel shows all in-flight changes from peers as they type, before they are committed
- [ ] **Co-author commit attribution**: all applied changes record which developer authored them in the edit history
- [ ] **Session replay**: the full collaborative editing session can be replayed to reconstruct who changed what and when
- [ ] **AI mediator**: when two developers edit conflicting parts of the same graph, AI proposes a merge and explains the conflict in plain English
- [ ] **Observer mode**: QA or team lead joins as read-only observer; sees all developer cursors and live diffs without write access

---

### F4 — Interactive AI Debugger (Full Integration)

- [ ] **Step-through any graph**: developer selects a WorldGraph, FlowGraph, or BehaviorGraph node and steps through execution with AI narration
- [ ] **Live value inspection**: hover over any port in any graph type and see the current computed value as a tooltip
- [ ] **Causal chain explorer**: click any entity property in ECS Inspector → AI traces the full chain of systems and events that produced the current value
- [ ] **Anomaly alerts**: AI runs a background monitor and surfaces warnings like "this entity's capacitor is recharging at 0 when it should be positive"
- [ ] **Fix-and-verify loop**: AI proposes a diff for each flagged anomaly; after apply, immediately re-runs the affected graph and reports if the anomaly cleared
- [ ] **Regression watch**: mark a value or condition as a "watch point"; AI alerts if it changes unexpectedly across builds

---

## Phase G — Experimental & Futuristic Systems

> **Goal**: Push the boundaries of what a game engine dev environment can do. These are high-value, long-horizon features that become achievable once Phases A–F are complete. They are planned now to ensure the architecture can support them.

### G1 — Multi-Agent AI Development Network

- [ ] **Agent swarm coordinator**: orchestrate 5–20 parallel AI agents across code writing, asset generation, PCG tuning, QA testing, and documentation
- [ ] **Negotiation protocol**: agents bid on tasks based on their specialization and current load; coordinator assigns optimally
- [ ] **Conflict graph**: real-time visualization of which agents are working on dependent files; highlight risk of merge conflicts before they happen
- [ ] **Agent progress dashboard**: see every agent's current task, estimated completion, token budget used, and pending approvals in one panel
- [ ] **Human-in-the-loop throttle**: developer can set a "max approvals per minute" to prevent getting overwhelmed by simultaneous agent proposals
- [ ] **Agent specialization training**: each agent accumulates a session history of approved changes; its future suggestions become more relevant over time
- [ ] **Parallel build pipeline**: code-agent writes a system; build-agent compiles immediately; qa-agent writes the test — all three work in sequence without developer intervention between them

---

### G2 — Predictive Procedural Universe Simulation

- [ ] **Offline universe sim**: run an entire galaxy (star systems, weather, flora, faction placement data) through a simulated time span (days, seasons, years) without a renderer
- [ ] **Emergent world evolution**: PCG parameters evolve over simulated time (biomes shift, regions get more or less resource-rich) based on simple rules
- [ ] **Snapshot browser**: navigate to any point in the simulated time range and inspect the world state; load any snapshot as the starting seed for a playable session
- [ ] **Divergence explorer**: run the same universe from the same seed with two different PCG rule sets; visualize where they diverge over time
- [ ] **Influence propagation model**: simulate how a single changed PCG parameter (e.g., rainfall increases) ripples through flora density, NPC anchor placement density, and ore vein distribution over time
- [ ] **Export to playable**: at any simulated time point, export the world state as a full set of `.atlasb` chunk files ready to load in the engine

---

### G3 — VR/AR Procedural Editing Mode

- [ ] **VR editing framework**: engine can output a stereoscopic render to a VR headset (OpenXR integration)
- [ ] **6DOF controller gizmos**: grab, scale, and rotate PCG scatter objects with physical hand controllers; full undo/redo support
- [ ] **Immersive WorldGraph editor**: float WorldGraph nodes in 3D space; connect ports with virtual cables
- [ ] **AR overlay mode**: connect to a running build on a separate screen; AR overlay shows live ECS entity bounds and component values floating above the rendered world
- [ ] **AI assistant in VR**: AtlasForgeAI overlay visible as a floating panel in VR space; speak prompts via voice, see responses as floating text
- [ ] **Collaborative VR session**: multiple developers in the same VR editing space; see each other's avatars and edits in real time
- [ ] **Fallback graceful**: VR/AR is a fully optional rendering path; if no headset is present, all editing tools remain available in standard flat mode

---

### G4 — Autonomous Content Publishing & Demo Generation

- [ ] **Demo builder**: AI assembles a playable demo build by selecting representative world seeds, configuring a starting scenario, and packaging a self-contained executable
- [ ] **Trailer generator**: AI scripts a cinematic sequence (using Phase F tools) covering key world visuals, runs it through the cinematic exporter, and packages the video
- [ ] **Automated release notes**: AI reads recent commit history and approved AI action logs; generates a human-readable changelog
- [ ] **Screenshot bot**: headless mode renders a set of curated camera positions across the world and selects the visually richest frames
- [ ] **Showcase packaging**: AI bundles demo executable + trailer + screenshots + changelog into a release folder structure ready for distribution
- [ ] **CI integration**: `atlas publish --demo` triggers the full pipeline — build, world gen, cinematic render, packaging — in a single CI job

---

### G5 — Adaptive AI Personality & Developer Modeling

- [ ] **Developer behavior model**: AI builds a per-developer usage profile (preferred tools, common workflows, frequent errors, response-to-suggestion rate)
- [ ] **Proactive suggestions**: based on the developer model, AI surfaces relevant suggestions before being asked ("you usually run the shader debugger after material changes — want me to open it?")
- [ ] **Adaptive verbosity**: AI shortens explanations for operations the developer has performed many times; expands explanations for new patterns
- [ ] **Error anticipation**: AI detects when a developer is about to make a change that historically causes build failures and warns proactively
- [ ] **Mood-responsive tone**: AI detects repeated rejections of suggestions and shifts to a shorter, less prescriptive style; detects exploratory sessions and becomes more generative
- [ ] **Team profile aggregation**: developer models from all team members inform shared style defaults (e.g., if the whole team favors a particular naming pattern, AI adopts it for codegen)
- [ ] **Privacy controls**: developer can view, edit, and delete their behavior model at any time; model never leaves local storage

---

## Phase Sequencing Summary

```
NOW
│
├── A1  Spaghetti cleanup (ongoing)
├── A2  Build consolidation
├── A3  Test infrastructure
├── A4  Developer tooling
├── A5  Documentation & conventions
│
├── B1–B7   Widget set + layout + themes + DSL + panels + CI GUI    ← parallel with C
├── B8–B12  Voice overlay, hotkeys, gizmos, HUD gen, AI skinning     ← parallel with C
│
├── C1–C7   PCG nodes + materials + interiors + placement + perf + audio  ← parallel with B
├── C8–C10  Ecosystem/weather PCG, drag-drop placement, cross-project PCG
│
├── D1   Offline LLM integration
├── D2   Agent core loop
├── D3   Tool system
├── D4   Code generation pipeline
├── D5   Dev Mode overlay
├── D6   Multi-language runners
├── D7   Media generation pipelines          ← can overlap D4–D5
├── D8   PCG + AI coupling
├── D9   Safety & quality gates
├── D10  Multi-agent system                   ← can overlap D7–D9
├── D11  Interactive AI debugger
├── D12  Voice macros & learning
├── D13  Tutorial recording & playback
├── D14  AI personality profiles
├── D15  Cross-project knowledge base
│
├── E1–E6   AI+GUI, AI+PCG, offline workflow, collab, perf, hardening
├── E7–E11  Performance AI, asset library, sim lab, predictive modeling, heatmaps
│
├── F1  Integrated cinematic preview
├── F2  Autonomous procedural cinematics
├── F3  Real-time collaborative editing
├── F4  Interactive AI debugger (full integration)
│
├── G1  Multi-agent AI development network     ← advanced, long-horizon
├── G2  Predictive procedural universe simulation
├── G3  VR/AR procedural editing mode
├── G4  Autonomous content publishing & demo generation
└── G5  Adaptive AI personality & developer modeling
│
GAME FEATURES BEGIN HERE
```

---

## Milestones

| Milestone | Criteria | Target |
|-----------|----------|--------|
| **M1: Clean Foundation** | Phase A complete. All 164 systems use template bases. `GameSession` decomposed. Build < 3 min. | After Phase A |
| **M2: Full GUI Toolkit** | Phase B complete. All widgets exist. Theme editor live. Voice overlay active. Gizmo designer working. | After Phase B |
| **M3: PCG Studio** | Phase C complete. WorldGraph debugger live. Interior + ecosystem PCG working. Drag-drop placement active. Cross-project styles importable. | After Phase C |
| **M4: AtlasForgeAI Online** | Phase D complete. Offline LLM running. Multi-agent system active. Voice macros + tutorial recorder live. Dev Mode overlay visible. | After Phase D |
| **M5: Integrated Dev Platform** | Phase E complete. Full offline workflow. AI + GUI + PCG interconnected. Heatmaps live. Sim lab operational. Certified build passing. | After Phase E |
| **M6: Cinematic & Collab** | Phase F complete. Cinematic editor live. Procedural cinematics working. Real-time collaborative editing stable. | After Phase F |
| **M7: Next-Level Platform** | Phase G substantially complete. Multi-agent swarm operational. Universe sim browseable. VR editing mode functional. Demo builder automated. | After Phase G |
| **Game Features** | All milestones met. No infra debt. Begin adding vertical gameplay. | After M7 |

---

## Feature Summary

| Phase | Area | Key Features |
|-------|------|-------------|
| A | Dev Infrastructure | Spaghetti cleanup, build consolidation, test infra, dev CLI tooling, docs |
| B | GUI | 13 new widgets, layout solver hardening, theme engine, animations, DSL enhancements, panel refinements, voice overlay, adaptive hotkeys, gizmo designer, HUD gen, AI skinning |
| C | PCG | 15 new WorldGraph nodes, material PCG, interior PCG, ecosystem/weather, drag-drop placement, placement learning, PCG perf + debugging, audio PCG, cross-project styles |
| D | AtlasForgeAI | Offline LLM (Ollama/llama.cpp), agent core loop, tool system, code gen, Dev Mode overlay, multi-language runners, media pipelines, PCG-AI coupling, multi-agent, AI debugger, voice macros, tutorial recorder, AI personality, cross-project KB |
| E | Integration | AI+GUI authoring, AI+PCG authoring, full offline workflow, live collab, perf AI, asset library, sim lab, predictive modeling, heatmaps |
| F | Cinematic & Viz | Cinematic preview editor, procedural cinematics, real-time collab editing, integrated AI debugger |
| G | Experimental | Multi-agent network, universe simulation, VR/AR editing, autonomous demo gen, adaptive AI personality |

---

## What This Roadmap Intentionally Excludes

The following will be addressed **after** all milestones are met:
- Ship combat mechanics
- Economy and trading systems
- FPS boarding gameplay
- Fleet command orders
- NPC AI behavior (faction, patrol, aggression)
- Mission generation and quest systems
- Player progression and legend system
- Multiplayer matchmaking
- Any other gameplay feature from `GAME_DESIGN_VISION.md`

> The engine, tools, AI systems, and cinematic pipeline must be production-solid before game features are built on top of them. Building game systems on shaky infrastructure creates double the work.

---

*Last updated: March 2026*
*Maintained by: Nova-Forge: Expeditions core team*
