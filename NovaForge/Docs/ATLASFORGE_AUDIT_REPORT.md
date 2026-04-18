# AtlasForge Audit Report

> **Source:** `shifty81/AtlasForge` (branch: main, SHA: `2cafe95`)
> **Target:** `tempnovaforge` consolidation repository
> **Date:** Auto-generated during Phase 5 audit

---

## Repository Overview

**Atlas Game Engine** — A modular, data-driven game engine and simulation platform built in C++20. All gameplay is authored as data and executable graphs; the engine contains no hardcoded gameplay logic.

### Key Capabilities
- **Deterministic Simulation** — bit-exact reproducible ticks with hash-ladder verification
- **Standalone Editor** — Blender-style authoring with 14+ dockable panels and AI assistant
- **Runtime Client & Server** — lean player runtime + headless authoritative server sharing one engine core
- **Graph VM** — deterministic bytecode VM with compile/execute, hot-reload, serialization
- **14 Graph Types** — world gen, animation, AI, audio, UI, character, weapon, tile, story, strategy, flow, interaction, sound, procedural material
- **Procedural Generation** — planet-scale terrain, galaxies, tiles, weapons, characters, narrative
- **AI Systems** — behavior graphs, memory with decay, faction relationships, LLM backend, strategy decisions
- **Networking** — client-server + P2P, lockstep/rollback, replication, QoS, packet loss sim
- **Project & Plugin System** — multi-project `.atlas` files, mod loader, plugin registry
- **Replay & Verification** — full replay, divergence detection, TLA+ formal specs
- **Scripting VM** — sandboxed script execution with ABI capsules
- **Production Pipeline** — asset cooker, game packager, certified builds, build audit logs

### Build System
- CMake 3.22+, C++20, top-level `CMakeLists.txt` orchestrates engine/editor/server/client/runtime/tests
- Makefile convenience wrapper with build/test/run/install/SDK targets
- `build.sh` / `run.sh` / `setup.sh` shell scripts
- Dockerfile present

### Development Status
All 12 phases marked 100% complete (Core → Editor → Networking → WorldGen → Gameplay → Graph Systems → AI → Interaction → Project/Plugin → Production → Polish → GUI Hardening → Replay/Verify → Flow Graph → AtlasAI/GUI → CI & Build).

---

## Directory Structure

### Top-Level Layout (14 directories + 10 files)

```
AtlasForge/
├── engine/           # Core engine (34 subdirectories, ~290 files)
├── editor/           # Editor application (4 subdirs, ~82 files)
├── server/           # Headless server entry point (2 files)
├── client/           # Player client entry point (2 files)
├── runtime/          # Lean runtime entry point (2 files)
├── modules/          # Gameplay module library (1 subdir, ~9 files)
├── projects/         # Sample game projects (3 projects)
├── tests/            # Test suite (~195 test files + 1 subdir)
├── docs/             # Documentation (~55 markdown files)
├── tools/            # Python/shell dev tools + Blender addon (~13 files)
├── schemas/          # JSON schemas (5 files)
├── specs/            # TLA+ formal specifications (3 files)
├── assets/           # Font assets (1 subdir, 2 files)
├── tileeditor/       # Standalone tile editor app (2 files)
├── CMakeLists.txt    # Root build file
├── CMakePresets.json  # Build presets
├── Makefile          # Convenience targets
├── Dockerfile        # Container build
├── build.sh          # Build script
├── run.sh            # Run script
├── setup.sh          # Setup script
├── README.md         # 24KB comprehensive README
├── CONTRIBUTING.md   # Contribution guide
└── LICENSE           # License file
```

### engine/ — Core Engine (34 subdirectories, ~290 source files)

| Subdirectory       | Files | Contents |
|-------------------|-------|---------|
| `abi/`            | 4     | ABICapsule (.h/.cpp), ABIRegistry (.h/.cpp) |
| `ai/`             | 26    | AIAssetDecisionFramework, AIMemory, AISignalRegistry, AtlasAICore, BehaviorGraph, BehaviorNodes, DiplomacyIntent, FactionRouter, LLMBackend, Personality, ProceduralGenerator, ProjectContext, RelationshipModel, WebAggregationKB |
| `animation/`      | 6     | AnimationGraph, AnimationNodes, DeterministicAnimationGraph |
| `asset_graph/`    | 4     | AssetGraph, AssetGraphExecutor, DamageState |
| `assets/`         | 22    | AssetBinary, AssetCategoryRegistry, AssetFormat, AssetImporter, AssetRegistry, AssetSerializer, AssetValidator, DependencyGraph, HttpClient, MarketplaceImporter, ServerAssetValidator, SocketHttpClient |
| `audio/`          | 2     | AudioEngine (.h/.cpp) |
| `camera/`         | 3     | Camera (.h/.cpp), CameraProjectionPolicy |
| `character/`      | 4     | CharacterGraph, CharacterNodes |
| `cmake/`          | 2     | AtlasContractEnforcement.cmake, check_include_firewall.cmake |
| `command/`        | 2     | CommandHistory (.h/.cpp) |
| `conversation/`   | 3     | ConversationGraph (.h/.cpp), ConversationNodes |
| `core/`           | 11+5  | Engine (.h/.cpp), Logger, CrashHandler, DeterministicAllocator, PermissionManager, EnginePhase; `contract/` subdir: AtlasContract, DeterministicRNG, IncludeFirewall, SimulationGuard, determinism.json |
| `ecs/`            | 4     | ECS (.h/.cpp), BuiltinComponents, ComponentCategory |
| `flow/`           | 11    | FlowGraphCodegen, FlowGraphDebugger, FlowGraphIR, FlowGraphRefactorer, GameFlowGraph, GameFlowNodes |
| `gameplay/`       | 4     | MechanicAsset, SkillTree |
| `graphvm/`        | 11    | CollaborativeEditor, GraphCache, GraphCompiler, GraphIR, GraphSerializer, GraphVM |
| `input/`          | 2     | InputManager (.h/.cpp) |
| `interaction/`    | 11    | Intent, IntentResolver, Interaction, InteractionRouter, InteractionSystem, RuleIntentResolver, Utterance, VoiceAdapter |
| `mod/`            | 4     | ModAssetRegistry, ModLoader |
| `module/`         | 3     | IGameModule, ModuleLoader |
| `net/`            | 8     | NetContext, NetHardening, QoSScheduler, Replication |
| `physics/`        | 2     | PhysicsWorld (.h/.cpp) |
| `platform/`       | 5     | PlatformWindow, Win32Window, X11Window |
| `plugin/`         | 2     | PluginSystem (.h/.cpp) |
| `procedural/`     | 31    | BuildQueue, ConstraintSolver, DeterministicRNG, HullMeshGenerator, InteriorNode, LODBakingGraph/Nodes, ModuleScaling, ModuleTier, PCGDomain, PCGManager, PCGVerify, PlanetaryBase, ProceduralMaterialGraph/Nodes, ProceduralMeshGraph/Nodes |
| `production/`     | 14    | AssetCooker, BuildAuditLog, BuildManifest, BuildProfile, CertifiedBuild, GamePackager, PlatformTarget |
| `project/`        | 2     | ProjectManager (.h/.cpp) |
| `render/`         | 28    | AtlasShaderIR, EditorViewportFramebuffer, GBuffer, GLRenderer, GLViewportFramebuffer, InstancedRenderer, NullRendererBackend, PBRMaterial, PostProcess, RenderAPI, RendererBackend, RendererCapabilities, RendererFactory, ShadowMap, SpatialHash, VulkanRenderer |
| `rules/`          | 2     | ServerRules (.h/.cpp) |
| `schema/`         | 2     | SchemaValidator (.h/.cpp) |
| `script/`         | 6     | ScriptSandbox, ScriptSystem, ScriptVM |
| `sim/`            | 40    | ComponentMigration, DesyncReproducer, DeterminismVersioning, FPDriftDetector, HotReloadConfig, JobTracer, ReplayDivergenceInspector, ReplayProofExporter, ReplayRecorder, ReplayVersioning, SaveSystem, SimMirror, SimulationStateAuditor, StateHasher, TLCModelChecker, TickScheduler, TickStepDebugger, TimeModel, WorldState, WorldStateSerializer |
| `sound/`          | 4     | SoundGraph, SoundNodes |
| `story/`          | 2     | StoryGraph (.h/.cpp) |
| `strategygraph/`  | 3     | StrategyGraph, StrategyNodes |
| `tile/`           | 8     | TileChunkBuilder, TileGraph, TileNodes, TileRenderer |
| `tools/`          | 34    | AnimationPreviewTool, BatchTransformTool, EditorCommandBus, EditorEventBus, EditorToolRegistry, EntityInspectorTool, EnvironmentControlTool, ITool, LayerTagSystem, LiveEditTool, MaterialOverrideTool, NPCSpawnerTool, PCGSnapshotTool, PrefabPlacementTool, ProjectNamespaceRewriter, SceneBookmarkManager, SimulationStepTool, UndoableCommandBus |
| `ui/`             | 75+   | CheckboxManager, ColorPickerManager, ComboBoxManager, DiagnosticsOverlay, DockManager, FocusManager, FontBootstrap, GUIDSLParser, GUIInputRecorder, GameGUIAsset, GameGUIBinding, HUDOverlay, HeadlessGUI, InputFieldManager, MenuManager, ScrollManager, SliderManager, SplitterManager, TabManager, TextRenderer, ToolbarManager, TooltipManager, TreeNodeManager, UIBackend, UICommandBus, UIConstants, UIDrawList, UIEventRouter, UIGraph, UILayoutSolver, UILogicGraph, UILogicNodes, UIManager, UINodes, UIRenderer, UISceneGraph, UIScreenGraph, UIScrollState, UIStyle, WidgetDSL, `atlas/` subdir |
| `voice/`          | 2     | VoiceCommand (.h/.cpp) |
| `weapon/`         | 4     | WeaponGraph, WeaponNodes |
| `world/`          | 20    | CubeSphereLayout, ExplainableNode, GalaxyGenerator, HeightfieldMesher, NoiseGenerator, TerrainMeshGenerator, VoxelGridLayout, WorldGraph, WorldLayout, WorldNodes, WorldStreamer |

### editor/ — Editor Application (~82 files across 4 subdirs)

| Subdirectory       | Files | Contents |
|-------------------|-------|---------|
| `panels/`         | 56    | AIDebuggerPanel, AIDiffViewerPanel, AssetBrowserPanel, AtlasAssistantPanel, CIDashboardPanel, ConsolePanel, DesyncVisualizerPanel, ECSInspectorPanel, GameMechanicsUIPanel, InteractionDebugPanel/Debugger, InventoryEditorPanel, JobTracePanel, MaterialEditorPanel, MeshViewerPanel, NetInspectorPanel, PrefabEditorPanel, ProfilerPanel, ProjectPickerPanel, ProofViewerPanel, QuestEditorPanel, ReplayTimelinePanel, RuleGraphEditorPanel, SaveFileBrowserPanel, StateHashDiffPanel, TilePalettePanel, TruthUIPanel, VoiceCommandPanel, WorldGraphPanel |
| `tools/`          | 10    | AssetDiffCommitFlow, GamePackagerPanel, IEditorToolModule, PlayInEditor, TileEditorModule, VisualDiff |
| `ai/`             | 4     | AIAggregator, TemplateAIBackend |
| `assistant/`      | 4     | AssetGraphAssistant, EditorAssistant |
| `ui/`             | 12    | DefaultEditorLayout, DockNode, EditorAttachProtocol, EditorLayout, EditorPanel, EditorTheme, LauncherScreen, LayoutPersistence |

### Other Directories

| Directory          | Files | Contents |
|-------------------|-------|---------|
| `server/`         | 2     | CMakeLists.txt, main.cpp — headless server entry |
| `client/`         | 2     | CMakeLists.txt, main.cpp — player client entry |
| `runtime/`        | 2     | CMakeLists.txt, main.cpp — lean runtime entry |
| `modules/atlas_gameplay/` | 9 | CMakeLists.txt, cmake config, CombatFramework, EconomySystem, FactionSystem |
| `projects/eveoffline/` | 2 files + 8 dirs | Plugin.toml, README, .atlas file + ai/, assets/, config/, conversations/, data/, module/, strategy/, worlds/ |
| `projects/arena2d/` | 3 files + 6 dirs | Plugin.toml, README, .atlas file + ai/, assets/, config/, data/, module/, worlds/ |
| `projects/atlas-sample/` | 3 files + 4 dirs | Plugin.toml, README, .atlas file + assets/, config/, data/, worlds/ |
| `tests/`          | ~197  | CMakeLists.txt, main.cpp, `replays/` subdir, ~195 test_*.cpp files (+ 2 .py test files) |
| `docs/`           | ~55   | 22 numbered docs (00-21), ~33 reference docs (ARCHITECTURE, BUILDING, conventions, contracts, etc.) |
| `schemas/`        | 5     | atlas.build.v1.json, atlas.conversation.v1.json, atlas.project.v1.json, atlas.strategygraph.v1.json, atlas.worldgraph.v1.json |
| `specs/`          | 3     | ecs.tla, layout.tla, replay.tla — TLA+ formal specifications |
| `assets/fonts/`   | 2     | Inter-Regular.ttf, builtin_fallback.json |
| `tileeditor/`     | 2     | CMakeLists.txt, main.cpp — standalone tile editor |
| `tools/`          | ~13   | atlas_init.py, contract_scan.py, crash_reporter.py, create_atlas_project.py, determinism_rules.yaml, replay_inspector.py, replay_minimizer.py, state_diff_viewer.py, verify_dependencies.sh, blender-addon/ (3 files + presets/) |

---

## Content Categories

### 1. Engine Core (`engine/`) — ~290 files
- **What:** Complete C++20 game engine with 34 module subdirectories covering core, ECS, rendering, networking, physics, audio, AI, procedural generation, scripting, simulation, UI, and more.
- **Overlap:** HIGH — Phase 4 (Atlas-NovaForge) already extracted ~40 engine module directories to `Source/Engine/`. The engine subdirectory names and file contents are substantially the same origin codebase.
- **Unique content:** `engine/script/` (ScriptVM/ScriptSandbox — 6 files), `engine/abi/` (4 files), `engine/core/contract/` (5 files with determinism.json), potential implementation differences in heavily evolved modules.

### 2. Editor (`editor/`) — ~82 files
- **What:** Standalone editor with 28+ dockable panels, AI assistant framework, play-in-editor, tile editor module, visual diff, layout persistence.
- **Overlap:** HIGH — Phase 4 extracted editor panels and tools to `Source/`. Panel names match closely.
- **Unique content:** `editor/ai/TemplateAIBackend` (LLM-based AI backend for editor), `editor/assistant/AssetGraphAssistant`, `editor/ui/EditorAttachProtocol` may have evolved implementations.

### 3. Server/Client/Runtime Entry Points — 6 files
- **What:** Thin `main.cpp` entry points for server, client, and runtime executables.
- **Overlap:** MODERATE — Phase 3/4 extracted server headers and client code but these are standalone mains.
- **Unique content:** Clean separation pattern useful as reference architecture.

### 4. Modules (`modules/atlas_gameplay/`) — 9 files
- **What:** Gameplay module library with CombatFramework, EconomySystem, FactionSystem + CMake config.
- **Overlap:** LOW — Phase 4 may have similar concepts but this is a distinct installable CMake module.
- **Unique content:** CMake-installable gameplay module pattern is novel.

### 5. Sample Projects (`projects/`) — 3 projects, ~30+ files
- **What:** Three sample game projects (eveoffline, arena2d, atlas-sample) with `.atlas` project files, Plugin.toml configs, game data, AI configs, world definitions.
- **Overlap:** MODERATE — Phase 4 extracted sample project structures.
- **Unique content:** `eveoffline` project has conversations/, strategy/ dirs; project structure demonstrates full plugin/mod pattern.

### 6. Tests (`tests/`) — ~197 files
- **What:** Comprehensive C++ test suite covering all engine systems, plus 2 Python test files and a replays/ directory.
- **Overlap:** HIGH — Phase 3/4 extracted tests to `Tests/` (871 existing test files).
- **Unique content:** Some test files cover newer systems (e.g., `test_flow_codegen.cpp`, `test_script_and_abi.cpp`, `test_tlc_model_checker.cpp`).

### 7. Documentation (`docs/`) — ~55 files
- **What:** Comprehensive markdown documentation covering engine architecture, editor design, asset system, graph VM, networking, world gen, gameplay, AI, replay, flow graphs, scripting, CI/build, binary compatibility, formal specifications, and contributor rules.
- **Overlap:** HIGH — Phase 4 extracted 20+ docs to `Docs/Atlas-NovaForge/`.
- **Unique content:** Additional docs: `ATLAS_SDK.md`, `BLENDER_ADDON_DESIGN.md`, `CI_FAILURE_PLAYBOOK.md`, `CORE_SYSTEMS_VERIFICATION.md`, `GUI_ISSUES_ANALYSIS.md`, `MARKETPLACE_IMPORTING.md`, `MENU_SYSTEM_IMPLEMENTATION.md`, `MENU_TESTING_GUIDE.md`, `NOVAFORGE_ENGINE_FEATURE_PLAN.md`, `SAVE_SYSTEM.md`, `STATE_MODEL.md`, `TIME_MODEL.md`, and several implementation guides.

### 8. Tools (`tools/`) — ~13 files + blender-addon/
- **What:** Python scripts (atlas_init, contract_scan, crash_reporter, create_atlas_project, replay_inspector, replay_minimizer, state_diff_viewer), shell scripts (verify_dependencies), YAML rules, and a Blender spaceship generator addon.
- **Overlap:** MODERATE — Phase 4 extracted some tools. `Tools/` has ReplayMinimizer, ContractScanner, BlenderGenerator, etc.
- **Unique content:** `crash_reporter.py`, `verify_dependencies.sh`, `determinism_rules.yaml`, blender addon presets.

### 9. Schemas (`schemas/`) — 5 files
- **What:** JSON schema definitions for atlas projects, builds, conversations, strategy graphs, world graphs.
- **Overlap:** EXACT — `Schemas/` in tempnovaforge has same 5 files plus 4 game-data schemas from Phase 3.

### 10. Specs (`specs/`) — 3 TLA+ files
- **What:** Formal TLA+ specifications for ECS, layout, and replay systems.
- **Overlap:** HIGH — Phase 4 may have extracted these.

### 11. Assets (`assets/fonts/`) — 2 files
- **What:** Inter-Regular.ttf font + builtin_fallback.json bitmap font data.
- **Overlap:** LOW — Need to check if `Content/Assets/` has fonts.

### 12. Build Infrastructure — 10 files
- **What:** CMakeLists.txt, CMakePresets.json, Makefile, Dockerfile, build.sh, run.sh, setup.sh, .gitignore, .dockerignore, CONTRIBUTING.md, LICENSE.
- **Overlap:** MODERATE — tempnovaforge has its own CMakeLists.txt, Makefile, Dockerfile.

---

## Overlap Analysis

### What tempnovaforge Already Has (Phases 0–4)

| Category | tempnovaforge Count | AtlasForge Count | Overlap |
|----------|-------------------|-----------------|---------|
| Engine source files | ~1,820 | ~290 | AtlasForge is the *origin*; tempnovaforge has more (expanded from multiple repos) |
| Test files | ~871 | ~197 | tempnovaforge has more tests already |
| Documentation | ~182 | ~55 | tempnovaforge accumulated docs from multiple phases |
| Schemas | 9 | 5 | tempnovaforge superset (5 identical + 4 game-data) |
| Tools | Various | ~13 | Partial overlap |

### Phase-by-Phase Comparison

**Phase 3 (Nova-Forge-Expeditions):** Extracted game systems, server headers, tests, PCG, renderer, editor tools → These are a subset of AtlasForge's `engine/` and `editor/` directories.

**Phase 4 (Atlas-NovaForge):** Extracted 40 engine module directories, editor panels, client code, game data, schemas, specs, tools, sample projects, docs, tests → This was explicitly an extraction *from* the AtlasForge repository. Nearly all AtlasForge content was extracted in Phase 4.

### Conclusion
**AtlasForge is the primary upstream source** that Phases 3 and 4 extracted from. The content in AtlasForge should be nearly fully represented in tempnovaforge already. However, AtlasForge may have continued to evolve after extraction, so any differences represent delta content worth capturing.

---

## Extraction Recommendation

### EXTRACT (8 files → canonical paths)

| Source | Target | Files | Rationale |
|--------|--------|-------|-----------|
| `engine/script/ScriptSandbox.{h,cpp}` | `Source/Engine/include/atlas/script/` + `src/script/` | 2 | Scripting sandbox if not already present |
| `engine/script/ScriptSystem.{h,cpp}` | `Source/Engine/include/atlas/script/` + `src/script/` | 2 | Script system integration |
| `engine/script/ScriptVM.{h,cpp}` | `Source/Engine/include/atlas/script/` + `src/script/` | 2 | Script virtual machine |
| `engine/abi/ABICapsule.{h,cpp}` | `Source/Engine/include/atlas/abi/` + `src/abi/` | 2 | ABI capsule system for plugin boundary |
| `engine/abi/ABIRegistry.{h,cpp}` | `Source/Engine/include/atlas/abi/` + `src/abi/` | 2 | ABI registry for versioned interfaces |
| `engine/core/contract/determinism.json` | `Source/Engine/include/atlas/core/contract/` | 1 | Determinism contract specification |
| `docs/SAVE_SYSTEM.md` | `Docs/Atlas-NovaForge/` | 1 | Save system documentation if missing |
| `docs/STATE_MODEL.md` | `Docs/Atlas-NovaForge/` | 1 | State model documentation if missing |
| `docs/TIME_MODEL.md` | `Docs/Atlas-NovaForge/` | 1 | Time model documentation if missing |
| `docs/CI_FAILURE_PLAYBOOK.md` | `Docs/Atlas-NovaForge/` | 1 | CI failure playbook if missing |
| `docs/CORE_SYSTEMS_VERIFICATION.md` | `Docs/Atlas-NovaForge/` | 1 | Verification docs if missing |
| `docs/ATLAS_SDK.md` | `Docs/Atlas-NovaForge/` | 1 | SDK documentation if missing |
| `tools/crash_reporter.py` | `Tools/AtlasForge/` | 1 | Crash reporter utility |
| `tools/verify_dependencies.sh` | `Tools/AtlasForge/` | 1 | Dependency verification script |
| `tools/determinism_rules.yaml` | `Tools/AtlasForge/` | 1 | Determinism scanning rules |

**EXTRACT total: ~18 files** (new or potentially missing content)

### ARCHIVE (Full repository → `Archive/_AtlasForge/`)

| Source | Target | Files | Rationale |
|--------|--------|-------|-----------|
| `engine/` (all) | `Archive/_AtlasForge/usable_snippets/engine/` | ~290 | Canonical reference for the original engine organization; superseded by expanded Source/Engine/ |
| `editor/` (all) | `Archive/_AtlasForge/usable_snippets/editor/` | ~82 | Editor panel/tool implementations as reference |
| `server/main.cpp` | `Archive/_AtlasForge/usable_snippets/server/` | 1 | Server entry point reference |
| `client/main.cpp` | `Archive/_AtlasForge/usable_snippets/client/` | 1 | Client entry point reference |
| `runtime/main.cpp` | `Archive/_AtlasForge/usable_snippets/runtime/` | 1 | Runtime entry point reference |
| `modules/atlas_gameplay/` | `Archive/_AtlasForge/usable_snippets/modules/` | 9 | Gameplay module pattern reference |
| `projects/` (all 3) | `Archive/_AtlasForge/usable_snippets/projects/` | ~30 | Sample project structure reference |
| `docs/` (all) | `Archive/_AtlasForge/usable_snippets/docs/` | ~55 | Full doc set as reference |
| `tests/` (all) | `Archive/_AtlasForge/usable_snippets/tests/` | ~197 | Test suite as reference |
| `tools/` (remaining) | `Archive/_AtlasForge/usable_snippets/tools/` | ~10 | Tool scripts as reference |
| `specs/` | `Archive/_AtlasForge/usable_snippets/specs/` | 3 | TLA+ specs reference |
| `tileeditor/` | `Archive/_AtlasForge/usable_snippets/tileeditor/` | 2 | Tile editor entry point |
| `README.md` | `Archive/_AtlasForge/README.md` | 1 | Original README preservation |
| `CONTRIBUTING.md` | `Archive/_AtlasForge/CONTRIBUTING.md` | 1 | Contribution guide |
| `Makefile` | `Archive/_AtlasForge/Makefile` | 1 | Build convenience targets reference |

**ARCHIVE total: ~685 files** (useful reference, superseded by tempnovaforge canonical content)

### SKIP (Build artifacts, duplicates — no extraction value)

| Source | Files | Rationale |
|--------|-------|-----------|
| `CMakeLists.txt` (root) | 1 | Build config — tempnovaforge has its own |
| `CMakePresets.json` | 1 | Build presets — superseded |
| `Dockerfile` | 1 | Container build — tempnovaforge has its own |
| `build.sh`, `run.sh`, `setup.sh` | 3 | Shell scripts — tempnovaforge has build.cmd |
| `.gitignore`, `.dockerignore` | 2 | Config files — not transferable |
| `LICENSE` | 1 | Same license already present |
| `schemas/` (5 files) | 5 | Already identical in `Schemas/` |
| `assets/fonts/Inter-Regular.ttf` | 1 | Binary font file — check if needed |

**SKIP total: ~15 files** (build artifacts, exact duplicates, or non-transferable config)

---

## Summary

| Category | File Count | Recommendation |
|----------|-----------|----------------|
| **EXTRACT** (new content) | ~18 | Copy to canonical tempnovaforge paths |
| **ARCHIVE** (reference) | ~685 | Archive as usable snippets |
| **SKIP** (no value) | ~15 | Do not extract |
| **Total AtlasForge** | **~718** | |

### Key Finding
AtlasForge is the **primary origin repository** from which Phases 3 and 4 already extracted the vast majority of content. The repository is essentially **fully consumed** by prior extraction phases. The remaining value is:

1. **~18 potentially new files** (script VM, ABI system, specific docs/tools) that may not have been extracted
2. **~685 reference files** useful as the authoritative "original form" archive for cross-referencing during future refactoring
3. The **Makefile build pattern** and **project structure conventions** serve as architectural documentation

### Recommended Next Steps
1. Verify which of the 18 EXTRACT candidates are truly missing from tempnovaforge
2. Create `Archive/_AtlasForge/usable_snippets/` with the full engine/editor/test snapshot
3. Diff key files (Engine.cpp, GraphVM.cpp, etc.) between AtlasForge and tempnovaforge to catch any post-extraction evolution
