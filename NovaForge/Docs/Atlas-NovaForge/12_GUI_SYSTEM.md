# Atlas Engine — GUI System Architecture

Atlas uses a single, unified GUI system for the editor, game UI, and headless server.
There is no ImGui dependency — every widget, layout solver, and event path is custom-built
and designed for deterministic replay from day one.

## Design Philosophy

- **State-first, not render-first.** The GUI tree is a data structure that exists
  independently of any renderer. Headless mode runs the full UI tree with no
  draw calls; the renderer is an optional consumer of that tree.
- **One system, three contexts.** Editor panels, in-game HUD, and headless server
  UI share the same widget set, layout solver, and event bus. The difference
  between them is authority, permissions, and mounting — not technology.
- **Deterministic and replayable.** Every GUI interaction produces a semantic event
  that can be recorded, replayed, and diffed. CI pipelines can run UI replays
  and export video without human interaction.
- **Declarative and version-controlled.** Layouts are described in a text-based DSL
  that is diff-friendly and supports schema migration between versions.

## Widget System

Widgets live in `engine/ui/` and are organized around two complementary
abstractions: a flat widget registry (`UIScreenGraph`) and a node-graph
evaluation model (`UIGraph`).

### Widget Types

| Type | Category | Description |
|------|----------|-------------|
| Panel | Container | Rectangular region; root of a widget subtree |
| Button | Interactive | Click target with pressed/released semantics |
| Text | Display | Static or data-bound text label |
| Image | Display | Texture-backed rectangular region |
| List | Container | Ordered vertical collection of children |
| SlotGrid | Container | Grid of discrete item slots (inventories, toolbars) |
| InputField | Interactive | Editable text with cursor and selection |

These types are declared in the `UIWidgetType` enum (`UIScreenGraph.h`).

### Rendering Primitives

All widgets decompose into a small set of render primitives: **rect**, **text**,
**icon**, **border**, and **image**. The renderer is only responsible for
drawing these primitives — it never interprets widget semantics.

### Node-Graph Evaluation

`UIGraph` (`UIGraph.h`) exposes a DAG of `UINode` objects connected by typed
ports (`Float`, `Bool`, `String`, `Layout`, `Action`). The graph is compiled
into a topological execution order and evaluated once per frame via
`UIGraph::Execute`. Cycle detection and edge-type validation run at compile
time.

```
UIGraph::Compile()          — topological sort, cycle + type check
UIGraph::Execute(UIContext) — evaluate nodes in sorted order
```

Built-in node types (`UINodes.h`):

| Node | Category | Purpose |
|------|----------|---------|
| PanelNode | Container | Sized rectangular region |
| ButtonNode | Interactive | Click detection, action output |
| TextNode | Display | Text content with data binding |
| ListNode | Container | Dynamic child collection |

## Screen Graph

`UIScreen` (`UIScreenGraph.h`) is a flat, ID-keyed widget registry that tracks
parent–child relationships, visibility, and spatial layout. It is the runtime
counterpart of the node graph — once a `UIGraph` is compiled and evaluated, the
results are materialized into a `UIScreen` for hit-testing and rendering.

```cpp
uint32_t id = screen.AddWidget(UIWidgetType::Button, "fire", x, y, w, h);
screen.SetParent(id, toolbarId);
screen.SetVisible(id, true);
```

## Layout Solver

### Integer-Only Math

All layout arithmetic uses integer coordinates and ratio-based splits
expressed as rational numbers. Floating-point is used only at the final
rasterization step. This guarantees bit-identical layout across platforms and
replay sessions.

### Formal Invariants

The layout solver enforces these invariants at every frame:

1. **No overlap** — sibling widgets never share pixels.
2. **Full coverage** — the union of siblings exactly fills the parent.
3. **Min-size respect** — no widget is smaller than its declared minimum.
4. **Deterministic docking** — the same sequence of dock mutations always
   produces the same layout.

### Constraint System

Panel sizing is expressed as constraints (minimum, preferred, maximum) with
ratio-based weights for flexible space distribution. The solver resolves
constraints in a single pass over the dock tree.

### Dock Mutation Operations

| Operation | Effect |
|-----------|--------|
| Split | Divide a node into two children (horizontal or vertical) |
| Merge | Collapse two siblings back into their parent |
| Resize | Adjust the split ratio between siblings |
| Move | Detach a panel and re-dock it at a new location |

## Editor Framework

Editor-specific layout lives in `editor/ui/` and builds on top of the engine
widget system.

### EditorPanel

`EditorPanel` (`EditorPanel.h`) is the base class for every editor panel. It
exposes a name, a visibility flag, and a `Draw()` entry point.

```cpp
class EditorPanel {
public:
    virtual const char* Name() const = 0;
    virtual void Draw() = 0;
    virtual bool IsVisible() const;
    virtual void SetVisible(bool visible);
};
```

### DockNode

`DockNode` (`DockNode.h`) is a binary tree node that describes a recursive
split layout. Each node is either a leaf holding an `EditorPanel*` or an
internal node with a split direction and ratio.

```cpp
struct DockNode {
    DockSplit split;      // None | Horizontal | Vertical
    float splitRatio;     // 0.0–1.0
    std::unique_ptr<DockNode> a;
    std::unique_ptr<DockNode> b;
    EditorPanel* panel;   // non-null only for leaves
};
```

### EditorLayout

`EditorLayout` (`EditorLayout.h`) owns the root `DockNode` and a flat list of
registered panels. It drives the recursive `DrawNode` traversal each frame.

## GUI DSL

Layouts are described in a declarative, text-based DSL that is designed for
version control and code review.

### Syntax Overview

```
layout "DefaultEditor" {
    split horizontal 0.25 {
        panel "ToolShelf"
        split vertical 0.70 {
            panel "SceneView"
            panel "Properties"
        }
    }
    dock bottom 0.30 {
        tabs {
            panel "Console"
            panel "Timeline"
            panel "NodeGraph"
        }
    }
}
```

### Capabilities

- **Docking targets** — panels declare where they can be docked (top, bottom,
  left, right, center, float).
- **Tab groups** — multiple panels share a single dock slot with tab switching.
- **Splits** — horizontal and vertical splits with explicit ratios.
- **Migration** — a version header enables automatic migration when the DSL
  schema changes between engine releases.

## Event System

### Semantic Input Recording

The event system records **semantic** UI events — "button `inventory_open`
pressed" — not raw pixel coordinates. Events reference widgets by stable UI ID,
so replays remain valid even if the layout changes.

### UICommandBus

All UI events flow through a thread-safe, lock-free `UICommandBus`. Producers
(input thread, network, replay) enqueue commands; the UI thread drains and
dispatches them once per frame. This guarantees ordering and avoids
data races without heavy synchronization.

### Event Flow

```
Input Source → UICommandBus → UI Thread Drain → Widget Dispatch → State Update
```

## GUI Replay

### Recording

The GUI input recorder captures every semantic event with its tick number and
target widget ID. The recording is a flat, append-only log that can be stored
alongside simulation replays.

### Playback

Playback feeds the recorded events back through the `UICommandBus` at the
correct tick. Because the layout solver and widget evaluation are deterministic,
playback reproduces the exact UI state frame-by-frame.

### CI Integration

- Automated tests can run UI replays in headless mode.
- A post-replay pass can export per-frame screenshots or video for visual
  regression testing.
- Deterministic font rasterization is enforced (fixed glyph atlas, integer
  metrics) so pixel-level comparison is stable across machines.

## ABI Compatibility

### GUI ABI Lock

The GUI exposes a locked ABI surface so that pre-compiled panels and plugins
can be loaded without recompilation when the engine is updated. The ABI covers:

- Widget type enum values
- Event struct layouts
- UICommandBus enqueue/dequeue signatures

### Versioning

- GUI asset schemas carry a version number and are validated on load.
- Breaking changes require a version bump and a migration path in the DSL
  loader.

## File Locations

| Path | Contents |
|------|----------|
| `engine/ui/UIGraph.h/.cpp` | Node-graph evaluation model (DAG of UINodes) |
| `engine/ui/UINodes.h/.cpp` | Built-in widget nodes (Panel, Button, Text, List) |
| `engine/ui/UIScreenGraph.h/.cpp` | Flat widget registry with parent–child tracking |
| `engine/ui/UIRenderer.h/.cpp` | Abstract rendering interface (DrawRect, DrawText, etc.) |
| `engine/ui/FontBootstrap.h/.cpp` | Font system initialisation after renderer init |
| `engine/ui/DiagnosticsOverlay.h/.cpp` | Toggleable FPS / viewport / DPI / mouse overlay |
| `engine/ui/TextRenderer.h/.cpp` | Backend-agnostic text rendering interface |
| `engine/ui/UIEventRouter.h/.cpp` | Input event routing with z-order dispatch and focus |
| `engine/ui/UIDrawList.h/.cpp` | Deferred draw command buffer for deterministic rendering |
| `engine/ui/UILayoutSolver.h/.cpp` | Constraint-based layout solver |
| `engine/ui/UICommandBus.h/.cpp` | Thread-safe semantic event bus |
| `engine/ui/HUDOverlay.h/.cpp` | In-game HUD with warnings and time controls |
| `engine/core/EnginePhase.h` | Engine lifecycle state machine (Boot → Launcher → Editor) |
| `editor/ui/EditorPanel.h` | Base class for editor panels |
| `editor/ui/EditorLayout.h/.cpp` | Dock-tree layout manager |
| `editor/ui/DockNode.h` | Binary split tree node for docking |
| `editor/ui/LauncherScreen.h/.cpp` | Unreal-style project browser / launcher |
| `editor/tools/IEditorToolModule.h` | Abstract interface for editor tool modules |
| `editor/tools/TileEditorModule.h/.cpp` | Gold-standard tile editor tool module |

## Font System

The engine uses a custom font pipeline — **no ImGui dependency**.

### Font Bootstrap

`FontBootstrap` (`engine/ui/FontBootstrap.h`) initialises the default UI font
**after** the renderer is ready. Call order is critical:

```
Renderer::Init()
UIContext::Init()
FontBootstrap::Init(assetRoot, dpiScale)
FontBootstrap::RebuildFontAtlas()   // on renderer reset
```

Without correct font initialisation, text renders as squares (missing glyph
rectangles). The bootstrap searches for `<assetRoot>/fonts/Inter-Regular.ttf`
and falls back gracefully if the file is absent.

### Text Rendering Interface

`TextRenderer` (`engine/ui/TextRenderer.h`) is a backend-agnostic abstract
interface. Concrete implementations for DX11, Vulkan, and OpenGL will derive
from it. A `NullTextRenderer` is provided for headless / server mode.

Key operations:

| Method | Purpose |
|--------|---------|
| `LoadFontAtlas(jsonPath)` | Load an offline-generated font atlas |
| `DrawText(font, text, x, y, color)` | Render text at a position |
| `MeasureText(font, text)` | Measure pixel width without drawing |
| `RebuildFontTexture(handle)` | Rebuild after renderer reset |

### Font Atlas Pipeline (Offline)

Font atlases are generated offline to avoid runtime TTF parsing:

1. `tools/font_atlas_gen` bakes a TTF into `Inter-Regular.png` + `Inter-Regular.json`.
2. Atlas is RGBA8, one texture per font size, deterministic glyph layout.
3. Runtime loads the JSON for metrics and the PNG as a GPU texture.

## Diagnostics Overlay

A toggleable overlay (`engine/ui/DiagnosticsOverlay.h`) renders live statistics
in both the editor and client. It is enabled by default in the editor and can
be toggled at runtime.

### Displayed Metrics

- FPS / frame time
- Viewport resolution
- DPI scale
- Mouse position
- Current tick

### Integration

```cpp
// In main loop — after all other UI, before EndFrame()
if (Input::IsKeyDown(Key::LeftControl) && Input::WasKeyPressed(Key::Grave))
    DiagnosticsOverlay::Toggle();

DiagnosticsOverlay::Render(renderer, uiCtx, dpiScale, mouseX, mouseY);
```

## UI Event Routing

`UIEventRouter` (`engine/ui/UIEventRouter.h`) routes platform input events to
the correct UI target. It replaces the previous direct dispatch that skipped
the UI layer entirely.

### Dispatch Rules

1. Targets are sorted by **z-order** (descending) before dispatch.
2. The **first target that consumes** the event wins.
3. `MouseDown` sets the **active capture** target; subsequent events go to it
   until `MouseUp`.
4. `MouseMove` updates **hover tracking**.
5. Editor UI consumes before game UI — preventing accidental game input.

### Focus Model

```cpp
struct UIFocusState {
    UIEventTarget* hovered;   // under cursor
    UIEventTarget* active;    // mouse capture
    UIEventTarget* keyboard;  // keyboard focus
};
```

## UI Draw Command Buffer

`UIDrawList` (`engine/ui/UIDrawList.h`) buffers draw commands during the UI
update pass and replays them through the renderer in one batch. This:

- Decouples UI logic from the graphics API
- Enables deterministic rendering (same commands → same output)
- Makes UI frames inspectable and replayable
- Supports debug UI visualization

## Engine Lifecycle

`EnginePhase` (`engine/core/EnginePhase.h`) defines the engine lifecycle as an
explicit state machine:

```
Boot → Launcher → ProjectLoad → Editor / Client / Server → Shutdown
```

Systems can query the current phase to assert correct initialisation order and
adapt behaviour (e.g. the UI switches between launcher and editor layouts).

## Launcher Screen

`LauncherScreen` (`editor/ui/LauncherScreen.h`) implements an Unreal-style
project browser displayed before the editor opens:

- Scans a projects directory for `project.atlas` files
- Displays project name, path, engine version, last-opened date
- Supports Open, Create New, Browse, and Quit actions
- Launches the editor with the selected project path

### Launcher UI Tree

```
Root
 ├── Header  (logo, engine version)
 ├── ProjectList
 │    ├── ProjectCard …
 └── Footer  (Open / New / Browse / Quit)
```

## Editor Tool Module System

`IEditorToolModule` (`editor/tools/IEditorToolModule.h`) is the abstract
interface every editor tool must implement. This is the most important editor
architecture interface — if a tool cannot be expressed through it, the
interface must be extended, not bypassed.

### Interface

```cpp
class IEditorToolModule {
    virtual std::string Name() const = 0;
    virtual void OnRegister() = 0;
    virtual void OnUnregister() = 0;
    virtual void RegisterPanels() = 0;
    virtual void RegisterMenus() = 0;
    virtual void RegisterModes() = 0;
    virtual bool HandleInput(uint32_t keyCode, bool pressed) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
};
```

### Tile Editor (Gold Standard)

`TileEditorModule` (`editor/tools/TileEditorModule.h`) is the reference
implementation that touches all tool concerns: UI, data, undo, serialisation,
and standalone builds.

Features: paint, erase, multi-layer, grid coordinates, per-instance overrides.

Editor modes: Paint, Erase, Select, LayerEdit, RuleEdit.

### Standalone Tool Builds

Any tool module can compile as a standalone executable using the same code:

```cpp
int main() {
    EngineCore::Init();
    EditorContext ctx;
    ctx.LoadModule("TileEditor");
    UI::Run();
}
```

Build profiles control which modules are included:

```json
{
  "Editor": ["AllTools"],
  "TileEditor": ["Core", "TileEdit", "UI"],
  "Animator": ["Core", "Animation", "UI"]
}
```

## Project Capability System

Projects declare enabled capabilities in `project.atlas`:

```json
{
  "capabilities": [
    "Rendering2D", "Rendering3D", "Physics3D",
    "UI", "Networking", "AI"
  ]
}
```

This determines:
- Which systems are compiled and loaded
- Which editor tools are available
- Which blueprint nodes are exposed
- Build profile contents
