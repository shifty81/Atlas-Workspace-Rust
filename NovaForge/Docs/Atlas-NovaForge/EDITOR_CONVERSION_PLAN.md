# Atlas NovaForge — Editor System Conversion Plan

> **Status**: Phase 4 Complete — All core editor tools implemented  
> **Date**: March 2026  
> **Scope**: Convert standalone AtlasEditor into an in-game ToolingLayer accessible from within the client runtime

---

## Executive Summary

The current Atlas Editor (`AtlasEditor` executable) is a standalone authoring environment with 14+ dockable panels, a custom GUI DSL, and Play-In-Editor support. This plan describes the conversion to an **in-game ToolingLayer** — a toggleable editor overlay that runs inside the game client, enabling live editing, asset generation, and world manipulation without leaving the game.

This conversion aligns with the project's direction toward a unified FPS + Flight + Fleet Command game where developers and modders need to iterate on content in-context.

---

## Current Editor Architecture

| Component | Status | Description |
|-----------|--------|-------------|
| **Panel Framework** | ✅ Complete | 14+ panels (Console, ECS Inspector, Graph Editor, etc.) |
| **GUI DSL** | ✅ Complete | Declarative layout language with docking/splits/tabs |
| **Layout Persistence** | ✅ Complete | Save/restore panel arrangements to JSON |
| **Play-In-Editor** | ✅ Complete | Simulate, pause, step, possess entity, loopback |
| **Rendering Layer** | ✅ Complete | UIDrawList + bitmap font, GL viewport framebuffer |
| **AI Assistant** | ✅ Complete | Explain, suggest, generate with HttpLLMBackend |
| **Undo/Redo** | ✅ Complete | Command-pattern history across editor operations |

---

## Target Architecture

### Core Concept

```
CoreRuntime
 ├── SimulationLayer       (game logic, ECS, AI, physics)
 ├── RenderingLayer        (OpenGL, deferred pipeline, UI)
 ├── InputLayer            (keyboard, mouse, gamepad)
 ├── NetworkLayer          (client-server, replication)
 └── ToolingLayer          (editor lives here — toggleable)
```

The editor becomes a **ToolingLayer** — a module that can be activated/deactivated at runtime without restarting the application. The game client and editor share the same runtime bootstrap.

### Unified RuntimeBootstrap

```cpp
class RuntimeBootstrap {
public:
    void Initialize(RuntimeMode mode);  // Editor, Game, Server
private:
    void LoadCoreSystems();             // ECS, EventBus, ConfigManager
    void LoadPlatformSystems(RuntimeMode mode);  // Runtime/Game, Editor, Server
    void LoadGameModules();             // Ships, PCG, AI, Story
};
```

All modes (Game, Editor, Server) share `LoadCoreSystems()`. Mode-specific logic is isolated in `LoadPlatformSystems()`. No duplicated boot logic.

---

## Key Components

### 1. ToolingLayer

Manages activation/deactivation of editor tools within the game client.

```cpp
class ITool {
public:
    virtual ~ITool() = default;
    virtual void Activate() = 0;
    virtual void Deactivate() = 0;
    virtual void Update(float deltaTime) = 0;
};

class ToolingLayer {
private:
    std::vector<std::unique_ptr<ITool>> activeTools;
public:
    void RegisterTool(std::unique_ptr<ITool> tool);
    void UnregisterTool(ITool* tool);
    void UpdateTools(float deltaTime);
    void OpenEditorUI();  // toggles in-game overlay
};
```

### 2. EditorCommandBus

Decouples tool actions from game state changes. Supports undo/redo.

```cpp
struct ICommand {
    virtual ~ICommand() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
};

class EditorCommandBus {
private:
    std::queue<std::unique_ptr<ICommand>> commandQueue;
    std::stack<std::unique_ptr<ICommand>> undoStack;
    std::stack<std::unique_ptr<ICommand>> redoStack;
public:
    void PostCommand(std::unique_ptr<ICommand> cmd);
    void ProcessCommands();
    void UndoLast();
    void RedoLast();
};
```

**Critical rule**: The editor must **never** directly mutate simulation data. All modifications go through the EditorCommandBus as replayable commands.

### 3. Editor Tools (Implemented)

All planned editor tools have been implemented and ported from the NovaForge repository.

| Tool | Purpose | Status |
|------|---------|--------|
| **MapEditorTool** | Place/modify star systems and islands | ✅ Complete |
| **ShipModuleEditorTool** | Assemble, tweak, and test ships live | ✅ Complete |
| **NPCSpawnerTool** | Spawn NPCs, fleets, or missions | ✅ Complete |
| **ResourceBalancerTool** | Tweak economy and resource distribution | ✅ Complete |
| **AssetPalettePanel** | Categorised library of reusable asset prefabs | ✅ Complete |
| **PhysicsTunerPanel** | Real-time physics parameter adjustment | ✅ Complete |
| **EnvironmentControlTool** | Global environment parameters | ✅ Complete |
| **EventTimelineTool** | Visualise and edit event timelines | ✅ Complete |
| **BatchOperationsTool** | Batch transforms on multiple objects | ✅ Complete |
| **SceneBookmarkManager** | Save/restore camera positions and scene states | ✅ Complete |
| **LayerTagSystem** | Organise entities by layer/tag | ✅ Complete |
| **SnapAlignTool** | Snap and align objects on a grid | ✅ Complete |
| **AssetStatsPanel** | Asset usage statistics | ✅ Complete |
| **AnimationEditorTool** | Edit animation clips and blending | ✅ Complete |
| **LightingControlTool** | Real-time lighting adjustment | ✅ Complete |
| **ScriptConsole** | In-game scripting console | ✅ Complete |
| **CameraViewTool** | Camera manipulation and bookmarks | ✅ Complete |
| **MaterialShaderTool** | Material and shader editing | ✅ Complete |
| **HotkeyActionManager** | Customisable hotkey bindings | ✅ Complete |
| **FunctionAssignmentTool** | Assign functions to entities | ✅ Complete |
| **EditPropagationTool** | Propagate edits to related objects | ✅ Complete |
| **VisualDiffTool** | Visualise differences between states | ✅ Complete |
| **DeltaEditStore** | Record and replay structured world edits | ✅ Complete |
| **SimulationStepController** | Pause, step, resume simulation | ✅ Complete |
| **MultiSelectionManager** | Multi-object selection support | ✅ Complete |
| **PrefabLibrary** | Prefab management and instantiation | ✅ Complete |

### 4. Supporting Infrastructure (Implemented)

| Component | Status | Description |
|-----------|--------|-------------|
| **ISystem / SystemRegistry** | ✅ Complete | Base interface for ECS systems with priority-ordered ticking |
| **EntityCommands** | ✅ Complete | Undoable create/destroy/set/remove component commands |
| **UndoableCommandBus** | ✅ Complete | Command bus with full undo/redo history |
| **EditorEventBus** | ✅ Complete | Event system for editor tool communication |
| **EditorToolLayer** | ✅ Complete | Runtime tool layer managing all editor tools |
| **DeltaEditStore** | ✅ Complete | Records structured edits on top of PCG seeds with JSON serialization |

---

## Conversion Phases

### Phase 1 — RuntimeBootstrap Unification (1–2 weeks) ✅ COMPLETE

| Step | Action | Effort | Status |
|------|--------|--------|--------|
| 1.1 | Define `RuntimeMode` enum (Game, Editor, Server) | 1 day | ✅ Done |
| 1.2 | Consolidate shared boot logic into `RuntimeBootstrap::LoadCoreSystems()` | 3 days | ✅ Done |
| 1.3 | Isolate mode-specific initialization into `LoadPlatformSystems()` | 2 days | ✅ Done |
| 1.4 | Verify all executables boot through `RuntimeBootstrap` | 2 days | ✅ Done |
| 1.5 | Remove duplicate initialization code across client/editor/server | 2 days | ✅ Done |

**Outcome**: Single boot path; Editor, Game, Server share the same core systems.

### Phase 2 — ToolingLayer Framework (2–3 weeks) ✅ COMPLETE

| Step | Action | Effort | Status |
|------|--------|--------|--------|
| 2.1 | Implement `ITool` interface and `ToolingLayer` manager | 2 days | ✅ Done (IToolModule, ToolModuleRegistry, ToolingLayer) |
| 2.2 | Implement `EditorCommandBus` with execute/undo/redo | 3 days | ✅ Done (EditorCommandBus.h/.cpp, 9 tests passing) |
| 2.3 | Add hotkey toggle (e.g., F1) for ToolingLayer overlay | 1 day | ✅ Done (SetToggleKey) |
| 2.4 | Gate editor access by build flag / dev authentication | 1 day | ✅ Done (ATLAS_INCLUDE_TOOLS) |
| 2.5 | Migrate Console panel as first tool (proof of concept) | 3 days | ✅ Done |
| 2.6 | Wire tool updates into game loop | 1 day | ✅ Done |

**Outcome**: Working in-game editor overlay with console.

### Phase 3 — Panel Migration (3–4 weeks) ✅ COMPLETE

All existing editor panels have been migrated to the ToolingLayer:

| Step | Action | Effort | Status |
|------|--------|--------|--------|
| 3.1 | Migrate ECS Inspector | 2 days | ✅ Done |
| 3.2 | Migrate Graph Editor | 3 days | ✅ Done |
| 3.3 | Migrate World Graph panel | 2 days | ✅ Done |
| 3.4 | Migrate Net Inspector | 2 days | ✅ Done |
| 3.5 | Migrate Profiler panel | 1 day | ✅ Done |
| 3.6 | Migrate Asset Browser | 2 days | ✅ Done |
| 3.7 | Migrate remaining panels | 1 week | ✅ Done |

**Outcome**: All existing editor functionality available as in-game tools.

### Phase 4 — New Game-Specific Tools (2–3 weeks) ✅ COMPLETE

All game-specific tools have been implemented and ported from NovaForge:

| Step | Action | Effort | Status |
|------|--------|--------|--------|
| 4.1 | MapEditorTool (star system placement) | 3 days | ✅ Done |
| 4.2 | ShipModuleEditorTool (live ship assembly) | 3 days | ✅ Done |
| 4.3 | NPCSpawnerTool (entity injection) | 2 days | ✅ Done |
| 4.4 | AssetGeneratorTool (procedural content) | 3 days | ✅ Done |
| 4.5 | Additional tools (26 total) | — | ✅ Done |

**Outcome**: Full creative toolset accessible from within the game.

### Phase 5 — Multiplayer & Authority (1–2 weeks)

| Step | Action | Effort |
|------|--------|--------|
| 5.1 | Editor commands routed through host authority | 2 days |
| 5.2 | Permission tiers (ViewOnly, QA, Developer, Admin) | 2 days |
| 5.3 | Co-op editor session support | 3 days |

**Outcome**: Multiplayer-safe editor with authority model.

---

## In-Game Workflow

```
[Game Update Loop]
  ├─ Process Player Input
  ├─ Update GameState (ships, AI, PCG)
  ├─ Update ToolingLayer (if editor open)
  │    └─ Update Active Tools
  │         └─ Post Commands to EditorCommandBus
  ├─ Process EditorCommandBus
  ├─ Render World
  └─ Render Tooling UI (overlay)
```

- The game is fully playable even if the editor is closed.
- Editor tools are optional and modular — can be expanded without touching runtime systems.
- Commands are queued and processed safely, maintaining simulation consistency.

---

## Architecture Rules

1. **ToolingLayer never mutates simulation directly** — all changes go through EditorCommandBus
2. **Editor actions are replayable** — commands are deterministic
3. **Host authority in multiplayer** — only host/admin can modify macro state
4. **Build-flag gated** — editor tools can be stripped from release builds
5. **Same boot path** — Editor and Game share RuntimeBootstrap, no duplicate initialization

---

## Migration from Standalone Editor

The existing `AtlasEditor` executable will be maintained during the transition period. Once all panels are migrated to the ToolingLayer, the standalone editor becomes an optional development convenience that simply boots `RuntimeBootstrap` in Editor mode with the ToolingLayer pre-activated.

Long-term, both the game client and the editor share the same binary, distinguished only by the `RuntimeMode` flag.
