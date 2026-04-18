# Atlas Engine — Editor Design

The Editor is a Blender-style, dockable authoring environment. It is a standalone
application — not a mode of the client — built as a superset of the runtime.

## Editor Principles

- Dockable panels (recursive dock nodes with horizontal/vertical splits)
- Context-sensitive tools
- Data-driven editors
- Non-modal workflows
- Everything live-editable
- Undo/redo is core, not optional (CommandHistory implemented)

## Implementation Status

### Implemented ✅

- **UI Framework** — EditorPanel base class, DockNode, EditorLayout manager
- **Console Panel** — Command parsing (`spawn_entity`, `ecs.dump`, `set tickrate`, `net.mode`, `help`)
- **World Graph Panel** — Node palette, canvas, inspector, and profiler sections
- **Project Picker Panel** — Project selection with recent projects list
- **Voice Command Panel** — Voice command processing and intent dispatch
- **Interaction Debugger Panel** — Logs interactions (speaker, text, intent, confidence)
- **Net Inspector Panel** — Network state debugging (mode, peers, RTT, bandwidth)
- **Game Packager Panel** — Build target/mode UI (scaffolded, no build logic yet)
- **Editor Assistant** — Routes intents for explanations (interface only)

- **ECS Inspector Panel** — Entity list with component types and summary view

### Not Yet Implemented ⬜

- **Graph Editor Panel** — Stub exists (no visual graph editing logic)
- **Asset Browser** — Not implemented
- **AI Asset Generation** — AIAggregator interface exists, no LLM integration

## Editor Capabilities (Current)

- Panel docking and layout management
- Console command execution
- World graph visualization
- Project loading and switching
- Voice command testing
- Interaction debugging
- Network state monitoring

## Editor Capabilities (Planned)

- Scene editing
- Live simulation preview
- Graph-based mechanics authoring
- Network testing (P2P, dedicated server loopback)
- AI-assisted content generation
- Hot reload visualization

## Default Layout

```
┌────────────────────────────────────┐
│ Top Bar (Mode / Play / Net)        │
├──────────────┬─────────────────────┤
│              │                     │
│  Tool Shelf  │   3D / Scene View   │
│              │                     │
├──────────────┼─────────────────────┤
│ Properties   │ Outliner / ECS Tree │
├──────────────┴─────────────────────┤
│ Console / Timeline / Node Graph    │
└────────────────────────────────────┘
```

Every panel is an Atlas panel.
Everything can be rearranged and saved.

## Core Panels

| Panel | Status | Description |
|-------|--------|-------------|
| Console | ✅ Implemented | Command-line automation with entity spawning |
| World Graph | ✅ Implemented | Node palette, canvas, inspector |
| Project Picker | ✅ Implemented | Project selection and recent list |
| Voice Commands | ✅ Implemented | Voice testing and intent dispatch |
| Interaction Debugger | ✅ Implemented | Interaction logging and analysis |
| Net Inspector | ✅ Implemented | Network topology and stats |
| Game Packager | 🔧 Scaffolded | Build UI exists, no functional logic |
| ECS Inspector | ✅ Implemented | Entity list, component types, summary |
| Graph Editor | ⬜ Stub | Not yet implemented |
| Asset Browser | ⬜ Missing | Not yet created |
| Profiler | ⬜ Planned | Not yet created |

## Play-In-Editor Modes (Planned)

- Simulate (no player)
- Possess Entity
- Client-Server Loopback
- P2P Test Session

## Extended Design

For expanded editor UI design including Unreal-grade aesthetics, editor shell architecture,
editor attach protocol, permission tiers, headless server GUI, and self-hosting, see
[13_EDITOR_UI.md](13_EDITOR_UI.md).

For the underlying custom GUI system (DSL, layout solver, widget system, replay), see
[12_GUI_SYSTEM.md](12_GUI_SYSTEM.md).

For game UI authoring within the editor (inventory, HUD, menus), see
[18_GAME_GUI_AUTHORING.md](18_GAME_GUI_AUTHORING.md).
