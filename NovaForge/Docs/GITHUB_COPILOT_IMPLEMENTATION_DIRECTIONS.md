# GitHub Copilot — Implementation Directions

> This document is the baseline reference for all Copilot-assisted implementation
> work in the NovaForge editor. Read this before writing any editor code.

---

## Editor Architecture Overview

The NovaForge editor is a **Win32/OpenGL desktop application** with no ImGui or
third-party UI libraries. Every pixel is owned by the custom UI renderer.

### Entry Point

`Source/Programs/NovaForgeEditor/main.cpp` creates a Win32 window, initializes
the OpenGL context via WGL, and drives the `NF::EditorApp` update loop.

### EditorApp

`NF::EditorApp` is the root owner of all editor state. It holds:

| Member | Type | Purpose |
|--------|------|---------|
| `m_dockLayout` | `DockLayout` | Slot-based panel layout manager |
| `m_toolbar` | `EditorToolbar` | Top toolbar with tool items |
| `m_commandStack` | `CommandStack` | Undo/redo stack |
| `m_selection` | `SelectionService` | Multi-select tracking |
| `m_contentBrowser` | `ContentBrowser` | Directory scanner |
| `m_recentFiles` | `RecentFilesList` | MRU list |
| `m_launchService` | `LaunchService` | Game exe launch |
| `m_projectPaths` | `ProjectPathService` | Project root discovery |
| `m_ideService` | `IDEService` | IDE: indexer, navigator, breadcrumb |
| `m_graphEditorPanel` | `GraphEditorPanel*` | Visual scripting panel |

---

## Panel Layout — 7-Panel Docking

The editor has **7 docked panels** arranged around a central viewport:

```
┌─────────────────────────────────────────────────────────┐
│                      Toolbar                            │
├──────────┬──────────────────────────────┬───────────────┤
│          │                              │               │
│ Hierarchy│         Viewport             │  Inspector    │
│          │                              │               │
├──────────┼──────────────────────────────┤               │
│          │                              │               │
│ Content  │         Console              │               │
│ Browser  │                              │               │
├──────────┴──────────────────────────────┴───────────────┤
│                   IDE / Graph Editor                     │
└─────────────────────────────────────────────────────────┘
```

| Panel | Class | Role |
|-------|-------|------|
| Viewport | `ViewportPanel` | 3D scene view, fly-cam (WASD + right-click), gizmos |
| Inspector | `InspectorPanel` | Property editor for selected entity/asset |
| Hierarchy | `HierarchyPanel` | Scene entity tree |
| Content Browser | `ContentBrowserPanel` | Asset directory, drag-drop |
| Console | `ConsolePanel` | Log output, command input |
| Graph Editor | `GraphEditorPanel` | Visual script graph editing |
| IDE | `IDEPanel` | Code navigator, symbol search, breadcrumb |

---

## Viewport Camera Controller

`ViewportCameraController` is owned by `ViewportPanel`:

- **Activate**: hold right mouse button (Mouse2)
- **Move**: WASD (forward/back/left/right)
- **Vertical**: Q (down) / E (up)
- **Sprint**: hold Shift (3× speed)
- `EditorApp::update(float dt, InputSystem&)` routes to `viewportPanel()->updateCamera(dt, input)`

---

## Command Registry

All editor actions are registered as named commands in `EditorCommandRegistry`:

```
file.new          file.open         file.save         file.save_as
edit.undo         edit.redo         edit.select_all   edit.deselect
view.reset_layout
graph.new_graph   graph.open_graph  graph.add_node    graph.remove_node  graph.compile
ide.go_to_definition  ide.find_references  ide.go_back  ide.index_project
```

Commands carry: name, display label, hotkey string, enabled flag, and a callable.

---

## Property Editor

`PropertyEditor` uses reflection offsets to read/write entity properties:

| PropertyType | Widget | Undo helper |
|---|---|---|
| `Bool` | Checkbox | `makeBoolChange` |
| `Int32` | Integer spinner | `makeIntChange` |
| `Float` | Float spinner | `makeFloatChange` |
| `String` | Text input | `makeStringChange` |
| `Vec3` | XYZ float inputs | `makeVec3Change` |
| `Color` | RGBA color picker | `makeColorChange` |

All property changes go through the `CommandStack` for undo/redo.

---

## IDE Service

`IDEService` owns:

| Subsystem | Purpose |
|-----------|---------|
| `ProjectIndexer` | Scans `Source/`, `Content/`, `Config/`; classifies files by extension; groups by module; indexes symbols |
| `CodeNavigator` | go-to-definition, find-references, search-symbols, filter-by-kind |
| `BreadcrumbTrail` | Navigation history, capped at 50 entries |

`EditorApp::init(w, h, executablePath)` calls `ide.index_project` automatically.

---

## Graph Editor

`GraphEditorPanel` owns a `Graph*` (`m_currentGraph`). Operations:

| Command | Action |
|---------|--------|
| `graph.new_graph` | Creates a new empty graph |
| `graph.add_node` | Adds a node to the current graph |
| `graph.remove_node` | Removes selected node |
| `graph.add_link` | Connects two ports |
| `graph.compile` | Compiles graph to bytecode via `GraphCompiler`, loads into `GraphVM` |

The panel's `render()` draws nodes as colored rectangles with port labels.

---

## UI Renderer

The custom UI renderer (`NF::UIRenderer`) provides:

- `drawRect(x, y, w, h, color)` — filled quad
- `drawRectOutline(x, y, w, h, color)` — outlined rect
- `drawText(x, y, text, color)` — character-based text via stb_easy_font

OpenGL state: 2D orthographic projection, depth test OFF, blend ON (alpha).
All draw calls are batched into a single vertex buffer per frame.

---

## Project Path Service

`ProjectPathService` discovers the project root relative to the executable path:

- `contentPath()` → `<root>/Content`
- `dataPath()` → `<root>/Data`
- `configPath()` → `<root>/Config`
- `savePath()` → `<root>/Saved`

`ContentBrowserPanel` auto-roots to `projectPaths().contentPath()` on init.

---

## Copilot Conventions

When generating editor code for this project:

1. **No ImGui** — never suggest `ImGui::*` calls. Use the custom `UIRenderer`.
2. **NF:: namespace** — all new types in the `NF::` namespace.
3. **Header in `include/NF/Editor/Editor.h`** — all editor types declared there.
4. **Source in `src/Editor.cpp`** — all implementations there.
5. **Catch2 tests in `Tests/Editor/test_editor.cpp`** — use `TEST_CASE` / `SECTION`.
6. **Commands via registry** — new actions registered in `EditorCommandRegistry`, not called directly.
7. **Undo via CommandStack** — any state mutation that the user can undo goes through `CommandStack::push()`.
8. **Determinism** — no `rand()`, no floating-point non-determinism in simulation paths.
9. **Phase discipline** — only implement what the current phase requires. Do not scope-creep.
