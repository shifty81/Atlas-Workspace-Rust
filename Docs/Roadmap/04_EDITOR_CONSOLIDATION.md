> **HISTORICAL** — This document describes a completed foundation phase (Phases 0–71).
> The canonical source of truth for current and future phases is [00_MASTER_ROADMAP.md](00_MASTER_ROADMAP.md).

# Editor Consolidation

**Phase 3 complete. Phase 3b — Roster Expansion (active)**

> ⚠️ Phase 3 (S4–S189 story stubs) is archived. Phase 3b expands the primary tool roster from ~10 to 20 and formalizes the project adapter model.

## Purpose

Collapse editor sprawl into a focused set of primary tools. Implement real
NF::IHostedTool instances. Convert one-off editors into shared panels, services,
or project adapter modules. Separate workspace (OS layer) from editor tools.

## Status

### Workspace Architecture Separation
- [x] Create `Source/Workspace/` module (NF::Workspace) as the OS-like host layer
- [x] Move 36 workspace-core headers (shell, registries, services, AI host) to `NF/Workspace/`
- [x] Create forwarding headers in `NF/Editor/` for backward compatibility
- [x] Decouple WorkspaceShell from hardcoded tool includes (factory-based registration)
- [x] Create CoreToolRoster.h for primary tool registration

### One-Off Tool Removal
- [x] Archive 29 project-specific game editors to Legacy/ (AbilitySystem, Costume, Arcade, etc.)
- [x] Group 143 one-off editors into tool-specific sub-directories:
  - Scene/ (28 headers): terrain, foliage, nav mesh, water, weather, cameras, world tools
  - Asset/ (30 headers): textures, sprites, meshes, audio, particles, fonts, prefabs
  - Material/ (12 headers): shader graph, post-process, gradients, curves, lightmaps
  - Animation/ (13 headers): anim blueprints, timelines, cinematics, cloth/rope sim
  - Data/ (18 headers): data tables, localization, dialogue, input bindings, UI design
  - Logic/ (9 headers): logic graph, state graph, AI behavior, event bus
  - Build/ (7 headers): deployment, packaging, benchmarks, stress testing
  - AI/ (10 headers): pathfinding, perception, telemetry, diagnostics, profiling
  - Infra/ (11 headers): cloud storage, network topology, accessibility, display
  - ProjectSystems/ (5 headers): sandbox, playmode, replay, playtest

### Umbrella Include Reduction
- [x] Break Editor.h into focused module headers
- [x] Create EditorSharedPanels.h for shared panel includes
- [x] Create EditorToolRegistry.h for tool registration
- [x] Keep Editor.h minimal (core editor types only)
- [x] Update Editor.h to include CoreToolRoster.h

### Legacy Archival
- [x] Move S-story stub tests (test_s4–test_s189) to Tests/Editor/Legacy/
- [x] Move non-core V1 stub headers to Source/Editor/include/NF/Editor/Legacy/
- [x] Archive project-specific game editors to Source/Editor/include/NF/Editor/Legacy/
- [x] Remove archived tests from CMakeLists build

### Primary Tool Implementation (NF::IHostedTool)
Each tool below must be a full IHostedTool implementation, not a stub.
It must implement initialize/shutdown/activate/suspend/update, declare
its shared panels, and register with WorkspaceShell via ToolRegistry.

| Tool | ToolId | Status |
|------|--------|--------|
| SceneEditorTool | workspace.scene_editor | [x] Done |
| AssetEditorTool | workspace.asset_editor | [x] Done |
| MaterialEditorTool | workspace.material_editor | [x] Done |
| AnimationEditorTool | workspace.animation_editor | [x] Done |
| DataEditorTool | workspace.data_editor | [x] Done |
| VisualLogicEditorTool | workspace.visual_logic_editor | [x] Done |
| BuildTool | workspace.build_tool | [x] Done |
| AtlasAITool | workspace.atlasai | [x] Done |
| ProjectSystemsTool | workspace.project_systems | [x] Exists (adapter host) |
| ParticleEditorTool | workspace.particle_editor | [ ] Planned |
| AudioEditorTool | workspace.audio_editor | [ ] Planned |
| PhysicsEditorTool | workspace.physics_editor | [ ] Stub needed |
| TerrainEditorTool | workspace.terrain_editor | [ ] Planned (promote from Scene/) |
| CinematicEditorTool | workspace.cinematic_editor | [ ] Planned (promote from Animation/) |
| ProfilerTool | workspace.profiler | [ ] Stub needed |
| VersionControlTool | workspace.version_control | [ ] Planned (promote from service) |
| ScriptingConsoleTool | workspace.scripting_console | [ ] Planned (promote from ScriptingConsole.h) |
| SettingsTool | workspace.settings | [ ] Stub needed |
| UIEditorTool | workspace.ui_editor | [ ] Planned (rename from HUDEditor) |
| WorkspaceBrowserTool | workspace.browser | [ ] Stub needed |

### Shared Panel Extraction
Convert these from standalone headers to registered shared panels in PanelRegistry:
- [x] ContentBrowserPanel → `panel.content_browser` (ContentBrowserSharedPanel)
- [x] ComponentInspectorV1 → `panel.component_inspector` (ComponentInspectorSharedPanel)
- [x] DiagnosticPanelV1 → `panel.diagnostics` (DiagnosticsSharedPanel)
- [x] MemoryProfilerPanel → `panel.memory_profiler` (MemoryProfilerSharedPanel)
- [x] PipelineMonitorPanel → `panel.pipeline_monitor` (PipelineMonitorSharedPanel)
- [x] NotificationCenterEditor → `panel.notification_center` (NotificationCenterSharedPanel)

### Shared Service Extraction
- [x] FileIntakePipeline → moved to NF::Workspace
- [x] CodexSnippetMirror → moved to NF::Workspace
- [x] EditorEventBus → moved to NF::Workspace
- [x] ConsoleCommandBus → moved to NF::Workspace
- [x] GraphHostContract → remains in NF::Editor (depends on EditorPanel; shared service contract documented)

### NovaForge Adapter Panels (already done)
- [x] GameEconomyEditor → NovaForgeEconomyPanel
- [x] InventoryEditor → NovaForgeInventoryRulesPanel
- [x] ItemShopEditor → NovaForgeShopPanel
- [x] DailyQuestEditor → NovaForgeMissionRulesPanel
- [x] AchievementEditor → NovaForgeProgressionPanel
- [x] CharacterCreatorEditor → NovaForgeCharacterRulesPanel

## Architecture

```
Source/Workspace/     ← "OS" layer (NF::Workspace)
  include/NF/Workspace/
    Shell:      WorkspaceShell, ToolRegistry, PanelRegistry, layout, commands
    Services:   EventBus, CommandBus, Selection, Undo, FileIntake, Notifications
    AI:         AtlasAIPanelHost, AIPanelSession, AIActionSurface, CodexSnippetMirror

Source/Editor/        ← Tool implementations (NF::Editor)
  include/NF/Editor/
    Core:           EditorApp, EditorPanel, EditorTheme, EditorCamera, etc.
    CoreToolRoster: registerCoreTools() — wires primary tools into shell
    Scene/          scene editing sub-panels (terrain, foliage, cameras, etc.)
    Asset/          asset management sub-panels (texture, mesh, audio, etc.)
    Material/       material/shader sub-panels
    Animation/      animation sub-panels
    Data/           data table/config sub-panels
    Logic/          visual logic sub-panels
    Build/          build/deploy sub-panels
    AI/             AI diagnostics sub-panels
    Infra/          workspace infrastructure sub-panels
    ProjectSystems/ adapter-hosted sub-panels
    Legacy/         archived V1 stubs and project-specific editors
```

### Adapter Tool Model

Game-specific authoring tools belong in the **project's own Source/Editor/** directory, not in workspace core. They are loaded into the workspace shell at runtime through `ProjectSystemsTool` + `IGameProjectAdapter`.

```
NovaForge/Source/Editor/include/NovaForge/Editor/
  GameEconomyEditor.h
  InventoryEditor.h
  ItemShopEditor.h
  QuestEditor.h
  DailyQuestEditor.h
  ProgressionEditor.h
  CharacterCreatorEditor.h
  CostumeEditor.h
  BiomeEditor.h
  EcosystemEditor.h
  DungeonGenerator.h
  TrophyEditor.h
  VirtualCurrencyEditor.h
  SeasonPassEditor.h
  RewardSystemEditor.h
  AchievementEditor.h
```

**Rules:**
- Adapter tools do **not** count against the 20-tool roster cap
- `IGameProjectAdapter.h`, `IHostedTool.h`, and `ProjectSystemsTool.h` must always remain in workspace core
- Each project registers its adapter via `WorkspaceShell::setGameProjectAdapter()`
