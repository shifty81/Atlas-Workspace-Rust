# Module Boundaries

## Source/Core

**Foundation layer.** No dependencies on other workspace modules.

Owns:
- Logging
- Memory utilities
- String utilities
- Math types
- Platform abstractions

## Source/Engine

**Atlas Engine.** Depends on Core only.

Owns:
- ECS
- Scene graph
- Asset system
- Behavior trees

## Source/UI

**AtlasUI framework.** Depends on Core.

Owns:
- Widget system
- Layout engine
- Theme/token system
- Backend abstraction (GDI, OpenGL, D3D11)
- Text rendering abstraction
- Draw list / render bridge

## Source/Editor

**Workspace editor host.** Depends on UI, Core, Engine.

Owns:
- Tool registry
- Panel framework
- Dock layout
- Editor-specific panels
- Primary tool implementations
- Project adapter contracts

## Source/Programs

**Executable entrypoints.** Depends on Editor, UI, Core.

Owns:
- AtlasWorkspace main.cpp
- Window creation
- Message pump
- Backend initialization

## Source/Pipeline

**Build orchestration and broker.** Depends on Core.

Owns:
- Workspace broker (session, indexing, analysis)
- Tool wiring
- Build pipeline

## Source/AI

**AtlasAI broker.** Depends on Core.

Owns:
- AI reasoning
- Context routing
- Model provider abstraction

## NovaForge/

**Hosted game project.** Depends on workspace through adapter.

Owns:
- Game logic
- World generation
- Gameplay systems
- Project-specific editor panels (through adapter)

## Rules

- No cross-layer leakage
- No game-specific logic in Core, UI, or Editor
- No workspace-core assumptions in NovaForge
- Projects interact through adapter contracts only

---

## Game Hosting in the Editor

`atlas-game` runs in two modes:

1. **Standalone** (`cargo run --bin atlas-game`) — boots `GameRunner`, loads `NovaForgeGameModule`, runs game loop
2. **Play-In-Editor (PIE)** — `atlas-editor` boots `PIEService` which instantiates `atlas-game` `GameRunner` inside the editor process, sharing `atlas-renderer` Vulkan surface

The boundary:

- `atlas-editor` owns: `WorkspaceShell`, `ToolRegistry`, `PanelRegistry`, `EditorViewport`, `PIEService`
- `atlas-game` owns: `GameRunner`, `GameModule`, all NovaForge game systems
- Shared: `atlas-renderer` (one `VkDevice`, two surfaces), `atlas-ecs` (separate `World` instances), `atlas-pcg` (shared `PcgManager`)
- `atlas-editor` **NEVER** imports game logic directly — all communication through `IGameProjectAdapter` trait
