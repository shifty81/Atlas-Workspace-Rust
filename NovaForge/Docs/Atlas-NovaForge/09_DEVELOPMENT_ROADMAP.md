# Atlas Engine — Development Roadmap

> This roadmap reflects the **actual implementation status** of each feature.
> Items marked as stubs have scaffolding code but lack functional logic.

---

## Phase 1 — Core Engine ✅

All core engine systems are implemented and tested.

- [x] Engine bootstrap (config, logging, lifecycle management)
- [x] ECS framework (entity creation, typed components, tick callbacks)
- [x] Graph VM interpreter (bytecode execution with full opcode dispatch)
- [x] Graph IR and compiler (topological sort, cycle detection, compilation)
- [x] Graph serialization (JSON builder/reader, no external dependencies)
- [x] Graph cache (tick-based invalidation, hash-keyed lookup)
- [x] Asset registry (filesystem scanning, type indexing)
- [x] Binary asset format (`.atlasb` with header, type, version)
- [x] Hot reload (file timestamp polling)
- [x] Schema validation system (versioned schemas, field integrity checks)
- [x] Tick scheduler (fixed-rate deterministic time stepping)
- [x] Time model (SimulationTime / WorldTime / PresentationTime layers)
- [x] Formal world state model (state categorization, snapshotting, rollback)
- [x] Save/load system (versioned binary format with hash integrity verification)

## Phase 2 — Editor ✅

The editor framework is complete. All panels have functional Draw() implementations using UIDrawList. The GL rendering backend provides bitmap font text rendering and a scene viewport framebuffer. The editor UI is self-hosted via the Atlas GUI DSL. Layout resize propagation is wired end-to-end.

- [x] UI framework (EditorPanel base class, visibility control)
- [x] Panel docking system (recursive dock nodes, horizontal/vertical splits)
- [x] Editor layout manager (tree traversal rendering)
- [x] Console panel (command parsing, entity spawning, ECS dump, net queries)
- [x] World graph panel (node palette, canvas, inspector, profiler sections)
- [x] Project picker panel (project selection, recent projects list)
- [x] Voice command panel (voice processing and intent dispatch)
- [x] Interaction debugger panel (interaction logging and analysis)
- [x] Net inspector panel (network state, peers, RTT, bandwidth)
- [x] ECS inspector panel (entity list, component types, summary view)
- [x] Graph editor panel (node enumeration, compile/preview, summary)
- [x] Asset browser (registry scanning, filtering, sorting, selection)
- [x] Editor assistant (asset graph suggestions, explanations, mutations)
- [x] Editor layout persistence (save/restore panel arrangements to JSON)
- [x] Play-In-Editor (simulate, pause, step, possess entity, loopback, state restore)

## Phase 3 — Networking ✅

Core networking API is implemented. Lockstep and rollback methods exist
and are functional. Replication rules are implemented.

- [x] NetContext API (mode selection, peer management, packet send/receive)
- [x] Dedicated server loop (headless mode, tick processing)
- [x] P2P host/peer support (peer add/remove, RTT tracking)
- [x] Loopback mode for testing
- [x] Lockstep synchronization (ECS state serialization into snapshots)
- [x] Rollback/replay (ECS state restore from snapshot, input frame replay)
- [x] Replication rules (rule-based dirty tracking, delta collection/application)
- [x] Production hardening (connection timeouts, reconnection, bandwidth throttling, heartbeat monitoring)

## Phase 4 — World Generation ✅

The most complete subsystem. Full procedural generation pipeline from
noise to terrain to galaxies, with streaming and disk caching.

- [x] WorldLayout interface (abstract coordinate mapping)
- [x] Cube-sphere planetary projection with LOD
- [x] Voxel grid chunked layout
- [x] Terrain mesh generation with normals
- [x] Graph noise nodes (Perlin, FBM — seeded, deterministic)
- [x] World streaming with disk cache
- [x] Galaxy generation (star systems, regions, spiral arms, star classes)
- [x] WorldGraph (DAG-based generation with typed values, topological compilation)
- [x] Heightfield mesher (heightfield → mesh pipeline with LOD)

## Phase 5 — Gameplay ✅

Core gameplay systems are implemented.

- [x] Mechanic asset format (MechanicAsset with parameter maps)
- [x] Skill tree system (nodes, connections, unlocking)
- [x] Camera system (world modes, projection policies)
- [x] Input mapping system (bindings, state tracking, callbacks)
- [x] Physics integration (rigid bodies, AABB collision detection, forces)
- [x] Audio system (sound loading, playback state management)

## Phase 5b — Graph Systems ✅

Atlas uses a DAG-based graph execution model across 14+ domains.
All graph types are implemented with compile/execute pipelines.

- [x] AnimationGraph (blend, pose, trigger, mask, modifier system)
- [x] CharacterGraph (modular character generation from seed)
- [x] WeaponGraph (weapon construction, wear degradation)
- [x] TileGraph (2D procedural tile maps)
- [x] SoundGraph (procedural audio synthesis)
- [x] StoryGraph (narrative generation and branching)
- [x] GameFlowGraph (boot → menu → gameplay → credits)
- [x] UIGraph + UIScreenGraph (UI composition)
- [x] StrategyGraph (influence, threat, scoring decisions)
- [x] ConversationGraph (dialogue flow, memory, relationships)
- [x] BehaviorGraph (AI behavior authoring)
- [x] AssetGraph (asset pipeline execution)

## Phase 5c — AI Systems ✅

AI subsystems with memory, signals, relationships, and behavior authoring.

- [x] AISignalRegistry (namespaced numeric inputs with tick tracking)
- [x] AIMemory (key-value storage with tick-based decay)
- [x] RelationshipModel (symmetric entity pair tracking)
- [x] FactionRouter (faction-based routing)
- [x] BehaviorGraph (authorable AI behavior as content)

## Phase 5d — Interaction & Voice ✅

Unified intent pipeline for console, voice, and AI inputs.

- [x] Utterance/Intent system (raw text → resolved action)
- [x] PatternIntentResolver (case-insensitive phrase matching)
- [x] IntentRegistry (central dispatch for handlers)
- [x] VoiceCommandRegistry (context-aware phrase → intent matching)
- [x] InteractionSystem (unified pipeline: Voice → Text → Intent → Command)

## Phase 5e — Project & Plugin System ✅

Multi-project architecture with schema validation and plugin support.

- [x] Project loading and validation (`.atlas` project files)
- [x] Schema-versioned project format
- [x] Plugin system (descriptor, validator, registry)
- [x] Mod asset registry (basic registration)
- [x] Server rules system (live parameter tuning with bounds)
- [x] Game module interface (IGameModule, GameModuleContext, dynamic/static loading)
- [x] AtlasGameplay library (FactionSystem, CombatFramework, EconomySystem)
- [x] EveOffline game module (shipped game example, permanently in-repo; IGameModule implementation with factions, economy, replication, server rules)
- [x] Arena2D game module (minimal 2D arena IGameModule implementation with factions, economy, replication, server rules)
- [x] CMake SDK export (install/export rules for find_package(AtlasEngine) and find_package(AtlasGameplay))

## Phase 6 — Production ✅

Production tooling is functional with build profiles, mod loading, and platform targeting. The packager is wired to the asset cooker with a full Validate → Cook → Bundle → Emit pipeline.

- [x] Game packager panel (UI scaffolding for build targets and modes)
- [x] Game packager pipeline (Validate → Cook → Bundle → Emit with progress callbacks)
- [x] Asset cooker (source → binary cooking with header, batch processing, progress callbacks)
- [x] Build profiles (Debug/Development/Release with optimization, symbols, assertions, compiler/linker flags, overrides)
- [x] Mod loader (mod descriptors, registration, enable/disable, dependency validation, directory scanning)
- [x] Platform targeting (Windows/Linux/macOS/Web with toolchains, defines, module exclusion, host detection)

## Phase 7 — Polish ✅

All polish systems are implemented.

- [x] Undo/redo system (CommandHistory with execute/undo/redo stacks)
- [x] Visual diff tools (key-value and graph snapshot comparison, diff summarization)
- [x] Profiler panel (frame timing history, system metrics, average/peak tracking, pause/resume)
- [x] Replay recorder (input frame recording, binary save/load, tick-based lookup)
- [x] Crash analysis tools (breadcrumb trail, crash report generation, file output, callbacks)

## Phase 8 — GUI System & Editor Hardening ✅

Advanced GUI architecture, editor refinements, and determinism enforcement are complete. The editor self-hosts its own layout via the Atlas GUI DSL. The dark professional theme (UIStyle) provides Unreal-grade aesthetics with dark backgrounds, accent colours, and professional typography.

- [x] Custom GUI widget system (layout solver, DSL compiler, deterministic rendering)
- [x] GUI DSL (declarative layout language with docking, splits, tabs)
- [x] GUI input recorder and replay (semantic events, CI video export)
- [x] Unreal-grade editor aesthetics (dark theme, professional typography, icons)
- [x] Editor self-hosting (editor UI defined in Atlas GUI DSL)
- [x] Non-closable main panels (World Outliner, Asset Browser, Inspector, Replay Timeline, Console, Permissions)
- [x] Editor attach protocol (attach to live client, headless server, or replay)
- [x] Permission tiers (ViewOnly, QA, Developer, CI, Admin)
- [x] Client HUD overlay (tick count, determinism warnings, time controls)
- [x] Headless server GUI (state-only panel tree, CLI commands)

## Phase 9 — Replay & Formal Verification ✅

Replay system closure, hash ladder verification, and TLA+ formal proofs.

- [x] Hash ladder verification (H[n] = Hash(H[n-1] || State[n] || Inputs[n]))
- [x] Replay Timeline panel (scrub, compare, branch, inject inputs)
- [x] Replay divergence inspector (auto-open diff view on hash mismatch)
- [x] TLA+ specifications (ecs.tla, replay.tla, layout.tla)
- [x] TLC model checker CI integration (spec registration, invariant extraction, CI exit codes)
- [x] Proof viewer panel (syntax-highlighted TLA+ in editor with tokenizer)
- [x] Replay → proof importer (trace → TLA+/JSON/CSV checkable format)
- [x] Cross-platform replay equivalence verification
- [x] Deterministic allocator (arena, stable IDs, no pointer-order dependence)
- [x] AtlasShaderIR (deterministic shader compilation)

## Phase 10 — Flow Graph & Procedural Content ✅

Blueprint-like visual scripting and Blender-like procedural modeling.

- [x] Flow Graph IR (file format, schema validation, version migration)
- [x] Flow Graph VM runtime (deterministic node execution)
- [x] Flow Graph debugger (breakpoints, timeline stepping, value inspection)
- [x] Flow Graph → C++ codegen (release build optimization)
- [x] UI logic graphs (safe subset for UI behavior)
- [x] Procedural mesh graph library (primitives, operations, deformers)
- [x] Procedural material & shader graphs
- [x] LOD & mesh baking graphs
- [x] Deterministic animation graphs (bone manipulation, IK/FK, blend trees)
- [x] Collaborative graph editing (live cursors, conflict resolution)

## Phase 11 — AtlasAI & Game GUI Authoring ✅

AI-assisted authoring and unified game UI system. The HttpLLMBackend provides OpenAI-compatible LLM integration with env-based configuration (ATLAS_LLM_ENDPOINT, ATLAS_LLM_MODEL, ATLAS_LLM_API_KEY). The EditorAssistant and AssetGraphAssistant are both wired to the LLMBackendRegistry.

- [x] AtlasAI core (structured I/O, permission-gated intents)
- [x] Atlas Assistant panel (context-aware prompts, diff preview)
- [x] AI diff viewer (preview generated changes before applying)
- [x] AI-assisted flow graph refactoring
- [x] Web aggregation knowledge base
- [x] Game GUI asset format (layout tree, widget definitions, bindings)
- [x] Widget DSL for game UI (inventory, HUD, menus, dialogs)
- [x] Game GUI binding system (ECS → UI data flow)
- [x] Full editor authoring for all game mechanics UI

## Phase 12 — CI, Build & Template System ✅

Production-grade CI, build experience, and template repository support.

- [x] Determinism CI gate (golden replay comparison, hash validation)
- [x] Contract bot (automated PR violation scanning)
- [x] atlas.build.json schema (artifact definitions, validation requirements)
- [x] First-run experience (one-command build, /run shortcuts)
- [x] Build manifest (executable hashes, layout hashes, DSL version)
- [x] atlas init tool (namespace rewrite, core freeze, editor branding)
- [x] Template contract (ATLAS_TEMPLATE_CONTRACT.md for forks)
- [x] Determinism versioning strategy across forks
- [x] CI dashboard panel (in-editor CI results visualization)
- [x] Certified build mode (audit-grade verification artifacts)

## Phase 18 — Determinism Debug Tooling ✅

Tick-step debugging, simulation state auditing, and cross-platform FP drift detection are fully tested.

- [x] TickStepDebugger (step forward/backward, tick breakpoints, hash-mismatch breakpoints, callbacks)
- [x] SimulationStateAuditor (system registration, state block declaration, hidden-state audit)
- [x] FPDriftDetector (cross-platform hash comparison, per-system drift analysis, report formatting/export)

## Phase 20 — Desync Reproduction & Network QoS ✅

One-command desync reproduction, priority-based packet scheduling, and server-side asset validation are fully tested.

- [x] DesyncReproducer (capture desync state, generate save/replay/report artifacts, crash report bundles)
- [x] QoSScheduler (priority-based packet ordering, bandwidth budgets, congestion detection, queue overflow)
- [x] ServerAssetValidator (manifest-based hash validation, server-side asset gating)

---

---

## Next Phases (New Direction)

> These phases reflect the project's pivot toward tri-modal gameplay and in-game editor tooling.
> See [GAME_DESIGN_VISION.md](GAME_DESIGN_VISION.md) and [EDITOR_CONVERSION_PLAN.md](EDITOR_CONVERSION_PLAN.md) for full details.

## Phase 21 — Spaghetti Code Cleanup 🔧

Active refactoring of server codebase to reduce technical debt. See [SPAGHETTI_CODE_AUDIT.md](SPAGHETTI_CODE_AUDIT.md).

- [x] Split monolithic test file (`test_systems.cpp` — 35K lines → 23 per-domain test files in `cpp_server/tests/`)
- [x] Create `test_framework.h` with shared `assertTrue`, `approxEqual`, `addComp` utilities
- [ ] Create `TestFixture` base class to eliminate remaining boilerplate
- [ ] Create system template bases (`SingleComponentSystem<C>`, `StateMachineSystem<C>`, `RechargeSystem<C>`)
- [ ] Decompose GameSession god object into domain-specific handlers (Combat, Station, Movement, Mission, Scanner)
- [ ] Extract shared sources into `novaforge_core` static library (CMake deduplication)
- [ ] Consolidate JSON parsing duplication across data layer

## Phase 22 — Editor Conversion ✅ COMPLETE

Standalone AtlasEditor converted to in-game ToolingLayer. All tools ported from NovaForge. See [EDITOR_CONVERSION_PLAN.md](EDITOR_CONVERSION_PLAN.md).

- [x] Unify RuntimeBootstrap across Game/Editor/Server modes (`engine/bootstrap/RuntimeBootstrap.h`)
- [x] Implement ToolingLayer framework (ITool interface, tool manager, hotkey toggle)
- [x] Implement EditorCommandBus (command queue, execute, undo/redo)
- [x] Migrate existing editor panels to ToolingLayer tools
- [x] Add new game-specific tools (26 total: MapEditor, ShipModuleEditor, NPCSpawner, AssetPalette, PhysicsTuner, etc.)
- [x] UndoableCommandBus, EditorEventBus, EntityCommands, DeltaEditStore
- [x] SceneBookmarkManager, LayerTagSystem, HotkeyActionManager
- [ ] Multiplayer authority model for editor access

## Phase 23 — Tri-Modal Gameplay ✅ COMPLETE

FPS + Flight + Fleet Command with hard mode boundaries. See [GAME_DESIGN_VISION.md](GAME_DESIGN_VISION.md).

- [x] PlayerModeController (FPS/Flight/Fleet Command state machine)
- [x] IPlayerMode interface (OnEnter/OnExit/Tick, input/camera/UI ownership)
- [x] Mode transition system (fade, authority transfer, camera swap, input remap)
- [x] Fleet Command intent system (6 orders: Engage, Defend, Escort, Retreat, Hold, Regroup)
- [x] FPS combat state machine (Exploring → Alert → Engaged → Wounded → Breached → Downed)
- [x] FPS enemy archetypes (Grunt, Breacher, Technician) and squad roles
- [x] Procedural FPS interior generation with grammar-based room flow
- [x] Low-poly visual style enforcement (flat shading, poly budgets, palette system)

## Phase 24 — Legend & World Simulation ✅ COMPLETE

Player reputation and dynamic faction ecosystem. See [GAME_DESIGN_VISION.md](GAME_DESIGN_VISION.md).

- [x] Player reputation model (aggression, stealth, precision, authority, chaos profiles)
- [x] Legend taxonomy (persistent legends, campaign memory, run memory)
- [x] Boss mutation system (procedural bosses that adapt to legend pressure)
- [x] Campaign chapter structure (Incursion → Escalation → Fracture → Confrontation → Aftermath)
- [x] Sector simulation (stability, hostility, resource wealth per sector)
- [x] Faction doctrine system (aggression, expansion, lawfulness per faction)
- [x] Heat propagation (player actions create pressure that diffuses through sectors)
- [x] Drift stabilizers (faction fatigue, economic elasticity, era resets)

## Phase 25 — Vertical Slice: "Derelict Intercept" 📋

One complete mission proving all three modes work end-to-end.

- [ ] Accept contract (station UI)
- [ ] Flight travel to asteroid field
- [ ] Fleet contact → enter Fleet Command → issue ENGAGE order
- [ ] Flight combat → disable target ship
- [ ] Dock / EVA → FPS boarding
- [ ] FPS objective (secure core module)
- [ ] Extract → debrief → rewards

---

## Implementation Summary

| Area | Status | Notes |
|------|--------|-------|
| Core Engine | ✅ Complete | ECS, Graph VM, assets, simulation |
| Graph Systems (14 types) | ✅ Complete | All graph types implemented with DAG execution |
| World Generation | ✅ Complete | Full pipeline: noise → terrain → galaxy → streaming |
| AI Systems | ✅ Complete | Memory, signals, relationships, behavior graphs |
| Gameplay | ✅ Complete | Camera, input, physics, audio, mechanics |
| Interaction/Voice | ✅ Complete | Unified intent pipeline |
| Project/Plugin | ✅ Complete | Multi-project, schema validation, plugins, game modules |
| Editor Framework | ✅ Complete | All panels with UIDrawList; GL bitmap font rendering; self-hosted DSL layout |
| Networking | ✅ Complete | API, lockstep/rollback, replication, production hardening |
| Production Tools | ✅ Complete | Game packager pipeline, asset cooker, build profiles, platform targeting |
| Polish | ✅ Complete | Undo/redo, visual diff, profiler, replay recorder, crash analysis |
| Game Module System | ✅ Complete | IGameModule, AtlasGameplay, EveOffline, SDK export |
| GUI & Editor Hardening | ✅ Complete | DSL, layout solver, dark theme, self-hosting |
| Replay & Verification | ✅ Complete | TLA+ specs, proof viewer, replay→proof export, ShaderIR |
| Flow Graph & Procedural | ✅ Complete | Flow Graph IR/VM/debugger, procedural mesh/material/LOD |
| AtlasAI & Game GUI | ✅ Complete | HttpLLMBackend, Game GUI DSL and bindings |
| CI & Build | 🔧 Functional | CI gates and build scripts work |
| Determinism Debug Tooling | ✅ Complete | TickStepDebugger, SimulationStateAuditor, FPDriftDetector |
| Desync & Network QoS | ✅ Complete | DesyncReproducer, QoSScheduler, ServerAssetValidator |
| Scripting VM | ✅ Complete | Stack-based bytecode VM, sandboxed builtins |
| Binary Compatibility | ✅ Complete | ABIVersion, ABICapsule, ABIRegistry |
| **Spaghetti Code Cleanup** | **🔧 Active** | **Monolith splits, boilerplate reduction, GameSession decomposition** |
| **Editor Conversion** | **✅ Complete** | **26 editor tools, ToolingLayer, EditorCommandBus, undo/redo** |
| **Tri-Modal Gameplay** | **✅ Complete** | **FPS + Flight + Fleet Command, PlayerModeController, FleetCommandSystem** |
| **Legend System** | **✅ Complete** | **Reputation model, memory taxonomy, boss mutations, sector simulation** |
| **Vertical Slice** | **🔧 Active** | **"Derelict Intercept" proving all modes end-to-end** |
