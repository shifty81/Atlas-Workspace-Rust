# Atlas Editor UI — Extended Design

This document extends [02_EDITOR_DESIGN.md](02_EDITOR_DESIGN.md) with additional
UI architecture decisions covering the editor shell, attach protocol, live editing
rules, and runtime HUD. It does not duplicate panel listings or implementation
status already tracked in that document. For the underlying widget system, layout
solver, and GUI DSL, see [12_GUI_SYSTEM.md](12_GUI_SYSTEM.md).

## Overview

- Unreal Engine–inspired professional aesthetics and interaction feel
- The editor is the primary product — not an afterthought bolted onto the runtime
- The editor is the permanent outer shell of the runtime, not a mode that can be
  toggled off
- Even during gameplay the editor shell remains present and accessible

## Editor Shell (Always Present)

The editor shell is a persistent frame around the world viewport. It is never
destroyed or hidden — only its interior content changes between authoring and
play states.

```
AtlasRuntime
 ├── EditorShell (always present)
 │    ├── MainMenu (always)
 │    ├── Toolbar (always)
 │    ├── DockRoot (always)
 │    │    ├── MainPanels (always)
 │    │    └── OptionalPanels (dynamic)
 │    └── StatusBar (always)
 └── WorldViewport (authoritative render target)
```

### Non-Closable Main Panels

These panels always exist in the layout tree. They may be collapsed or tabbed
behind other panels, but they cannot be removed from the dock hierarchy.

| Panel | Purpose |
|-------|---------|
| World Outliner | Entity hierarchy and selection |
| Asset Browser | Project asset navigation and preview |
| Inspector | Component and property editing |
| Replay Timeline | Tick-level scrubbing and playback controls |
| Console / Log | Command entry and diagnostic output |
| Permissions / Authority | Authority state and permission tier display |
| Determinism Status | Hash validation and desync warnings |

### Optional / Context Panels

These panels are created on demand and may be closed freely.

| Panel | Purpose |
|-------|---------|
| Asset Diff Viewer | Side-by-side comparison of asset versions |
| Material Graph Editor | Node-based material authoring |
| AI Behavior Editor | Behavior tree and utility graph editing |
| Quest Editor | Quest flow and condition authoring |
| GUI Layout Editor | Visual DSL layout editing |
| Inventory Editor | Item slot and container configuration |
| Profiler | Frame timing, ECS query cost, memory |
| Net Inspector | Network topology and packet debugging |
| Voice Commands | Voice input testing and intent dispatch |

## Style Guide

The editor targets Unreal-grade visual quality in its chrome and controls.

- **Theme** — dark base with accent colors for selection, warnings, and errors
- **Typography** — a single professional sans-serif family; consistent sizes for
  headers, labels, and body text
- **Spacing** — uniform margins and padding derived from a base unit; no ad-hoc
  pixel values
- **Icons** — every panel header, toolbar button, and context menu entry uses a
  vector icon from a shared atlas
- **State indicators** — panel headers display connection state (connected,
  active, warning) via color-coded badges
- **Docking animations** — panel drag-and-dock transitions are smooth and
  deterministic so they replay identically

## Editor Attach Protocol

The editor is not limited to running as a monolithic application. It supports
multiple attach modes that allow it to observe or mutate a running session.

### Attach Modes

| Mode | Description |
|------|-------------|
| Standalone | Editor launches its own runtime instance |
| Live Client | Editor attaches to a running client process |
| Headless Server | Editor attaches to a dedicated server over the network |
| Replay | Editor opens a replay file for offline inspection |

### Attach Flow

1. Target process starts and advertises "Editor available" on a local socket or
   network port.
2. Editor discovers the target and initiates an attach handshake.
3. On successful attach the editor gains **read-only** access to the world state.
4. The operator may explicitly enter **Mutate World** mode, which is gated by
   their permission tier.

### Permission Tiers

| Tier | Capabilities |
|------|-------------|
| ViewOnly | Inspect state, browse assets, read logs |
| QA | ViewOnly + place markers, capture snapshots |
| Developer | QA + spawn entities, modify components, hot-reload assets |
| CI | Developer + headless batch commands, automated test runs |
| Admin | Full authority including schema and permission changes |

### Mutation Rules

- Every mutation is tick-stamped with the simulation tick at which it is applied.
- All mutations are logged to the command history and are undoable.
- Mutations are replayable — the replay file records editor commands alongside
  simulation events.

## Live Edit Rules

Not all changes can be applied at runtime. The following table defines the
expected behavior for each category of change.

| Change Category | Runtime Behavior |
|-----------------|-----------------|
| Config values | Live update — applied on the next tick |
| Asset data | Hot reload — asset is reloaded and references are patched |
| System logic | Requires rebuild — editor prompts for recompilation |
| Nondeterministic ops | Blocked — the editor rejects operations that would break determinism |

## Play-In-Editor

Play-In-Editor does not replace the editor shell. Instead, the GUI tree gains a
context tag that controls visibility, permission caps, and input routing.

```cpp
enum class GUIContext {
    Editor,
    Game,
    Server
};
```

- **Editor** — full panel visibility, all permissions active, input routed to
  editor widgets first.
- **Game** — game HUD visible, editor panels collapsed or overlaid, input routed
  to the game viewport by default.
- **Server** — no renderer; panels exist as queryable state only.

The same GUI tree is used in all three contexts. Switching contexts changes
visibility masks and input priority — it does not rebuild the tree.

## Editor Self-Hosting

The editor UI is itself defined using the Atlas GUI DSL
(see [12_GUI_SYSTEM.md](12_GUI_SYSTEM.md)). This means:

- Editor panels are described in the same declarative format as game UI.
- The GUI Layout Editor panel can open and modify the editor's own layout files.
- A bootstrap sequence loads a minimal hard-coded layout, then replaces it with
  the DSL-defined layout once the DSL loader is available.

## Client HUD (Minimal Runtime Shell)

When the editor is not attached, the client displays a minimal overlay HUD.

```
┌─────────────────────────────────────────────────┐
│ ProjectName  Tick:4821  Δ12.3ms  Mode:Client    │  ← top-left
│                          EditorStatus  Branch    │  ← top-right
│                                                  │
│                   Viewport                       │
│                                                  │
│ Console (read-only)  DeterminismWarnings         │  ← bottom-left
│                          ⏸ ⏪ ⏩ Step            │  ← bottom-right
└─────────────────────────────────────────────────┘
```

| Region | Contents |
|--------|----------|
| Top-left | Project name, tick count, delta time, mode, authority |
| Top-right | Editor connection status, replay state, branch name |
| Bottom-left | Read-only console output, determinism warnings |
| Bottom-right | Time controls (pause, rewind, step forward) |

The console becomes writable when an editor with sufficient permissions attaches.

## Headless Server GUI

The headless server runs the full GUI tree with no renderer attached.

- Panels exist as state objects and can be queried or mutated through the
  `UICommandBus`.
- CLI interaction exposes panel commands directly:

```
atlas_server --gui-cmd inventory.dump player42
atlas_server --gui-cmd console.exec "spawn_entity Asteroid 100 200 300"
```

- Automated tests and CI pipelines interact with the GUI tree through the same
  command interface, ensuring that headless and rendered paths exercise identical
  code.

## Welcome Screen (First Launch)

On first launch the editor presents a welcome panel with the following actions:

| Action | Shortcut | Description |
|--------|----------|-------------|
| Open Project | — | Browse for an existing `.atlas` project file |
| Create New Project | — | Scaffold a new project from a template |
| Load Example Project | — | Open one of the bundled example projects |
| Documentation | F1 | Open the Atlas documentation |
| Build Project | Ctrl+B | Build the currently loaded project |
| Run Project | F5 | Launch Play-In-Editor |
