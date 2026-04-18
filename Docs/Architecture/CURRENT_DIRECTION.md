# Current Direction

> See also: `Docs/Canon/01_LOCKED_DIRECTION.md`

## Active Development Path

Atlas Workspace is a **generic development host** for editor tools, build systems,
and project orchestration. It is not a game.

### Runtime Stack

```
AtlasWorkspace.exe
  └─ WorkspaceShell (composition root)
       ├─ ToolRegistry (8 primary tools)
       ├─ PanelRegistry (14 shared panels)
       ├─ ConsoleCommandBus (command dispatch)
       ├─ InputRouter (per-frame input ownership)
       ├─ SelectionService
       ├─ EditorEventBus
       ├─ WorkspaceViewportManager
       ├─ LayoutManagerV1
       ├─ WorkspaceShellContract
       └─ ProjectSystemsTool (adapter-hosted panels)
```

### Rendering Path

```
WorkspaceRenderer → UIRenderer → GDIBackend → Win32
```

GDI is the active fallback. D3D11+DirectWrite is the target primary backend.

### Project Hosting

Projects load through `IGameProjectAdapter`. NovaForge is the primary hosted
project, providing 6 gameplay panels through `NovaForgeAdapter`.

### What Is NOT Active

- WPF shell (archived)
- ImGui integration (not canonical)
- Tile Authoring Tool (deferred)
- Full D3D11 rendering (stubs exist, not executing)
