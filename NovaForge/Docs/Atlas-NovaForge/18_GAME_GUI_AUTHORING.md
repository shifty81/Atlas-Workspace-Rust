# Atlas Game GUI Authoring

Game GUI is not a separate system. It uses the same Atlas GUI runtime — the same
widgets, layout solver, event pipeline, and determinism guarantees that power the
editor. The only difference between editor UI and game UI is authority, permissions,
and mounting context. This document covers authoring game UI within that unified
framework. For the underlying widget system see [12_GUI_SYSTEM.md](12_GUI_SYSTEM.md);
for editor-specific panels see [13_EDITOR_UI.md](13_EDITOR_UI.md).

## Overview

- Game GUI is NOT a separate system — it uses the same Atlas GUI runtime
- Game UI is authored data: stored as assets, replayed, diffed, CI-tested
- Every game mechanic's UI is editable in the editor and testable in the client
- The editor is an authoring tool for ALL game UI: HUD, inventory, menus, dialogs, tooltips

## Core Principle: One GUI System

- Same widgets for editor and game
- Same layout solver
- Same event system
- Same determinism rules
- Same replay + CI support
- Difference is only: authority, permissions, and mounting context

## Game GUI Asset Format

```
GameGUIAsset
 ├── LayoutTree
 ├── WidgetDefinitions
 ├── Bindings
 │    ├── Inventory
 │    ├── Health
 │    ├── Abilities
 │    └── Quests
 └── InputMappings
```

- Assets are version-controlled, diff-friendly
- Schema-validated
- Hot-reloadable in editor

## Widget DSL for Game UI

- Declarative syntax for all game widgets
- Supports: panels, grids, lists, bars, tooltips, drag-drop
- Binds to ECS data and simulation state
- Compile-time validation of bindings

## Authoring Workflow (End-to-End)

1. Open relevant editor panel (e.g., Inventory Editor)
2. Modify widget layout
3. Diff generated automatically
4. Replay branch created
5. Hit "Test in Client" — same GUI runs live
6. Interaction recorded for replay
7. Bug found → replay saved
8. CI validates determinism

No rebuilds, no hacks, no mock UI.

## Editable Game Mechanics

Every game mechanic that has UI is fully authorable in the editor:

- Inventory system (grid widgets, slot management, drag-drop, tooltips)
- Health/status bars
- Ability/skill UI with cooldowns
- Quest tracker and objectives
- Dialog/conversation UI
- Map/minimap widgets
- Crafting interfaces
- Shop/economy UI
- Character stats and equipment
- Settings and keybind menus

## Play-In-Editor (PIE) Context

```cpp
enum class GUIContext {
    Editor,
    Game,
    Server
};
```

Same GUI tree, different visibility rules, permission caps, and input routing.
PIE is "just a flag" — not a separate system.

## Binding System

- Game UI binds to ECS components and simulation state
- Bindings are typed and schema-validated
- Data flows one-way: simulation → UI
- User input flows back via command system (never direct mutation)

## Headless Server GUI

- Server runs GUI tree with no renderer
- Commands still work
- State inspection via CLI: `atlas_server --gui-cmd inventory.dump player42`
- Same logic, zero duplication

## Permission Model

| Tier       | Can Edit Game UI | Can Edit Editor UI |
|------------|------------------|--------------------|
| ViewOnly   | ✖                | ✖                  |
| QA         | ✖                | ✖                  |
| Developer  | ✔                | ✔                  |
| CI         | ✖                | ✖                  |

## What This Prevents

- ❌ Separate "game UI system"
- ❌ Editor-only widgets
- ❌ Non-replayable UI bugs
- ❌ CI blind spots
- ❌ Hardcoded inventory screens
- ❌ DPI hacks
