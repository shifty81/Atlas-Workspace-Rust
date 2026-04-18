# Atlas Editor — Status & Gap Analysis

> This document provides an honest assessment of the Atlas Editor's completion
> status, what works today, and what remains to be done before the editor is
> fully functional.

---

## Overview

The Atlas Editor is one of four executables built from the engine core. It is
designed to be a Blender-style authoring environment with dockable panels,
graph editing, and AI-assisted workflows.

**Current state:** The editor's **logic layer is solid** — 14+ panels have
real, functional implementations. The **rendering layer is deferred** — all
`Draw()` methods are no-ops awaiting full integration with the Atlas custom
UI rendering backend.

---

## What Works Today ✅

### Panels with Functional Logic

| Panel | Key Features | Status |
|-------|-------------|--------|
| **Console** | Command parsing, entity spawning, ECS dump, net queries | ✅ Full |
| **World Graph Editor** | Node palette, canvas, inspector, compile/preview | ✅ Full |
| **ECS Inspector** | Entity snapshots, component listing, state blocks | ✅ Full |
| **Asset Browser** | Registry scanning, filtering, sorting, selection | ✅ Full |
| **Profiler** | Frame timing history, system metrics, average/peak | ✅ Full |
| **Network Inspector** | Mode, peer count, RTT, connection snapshots | ✅ Full |
| **Project Picker** | Recent projects list, project loading | ✅ Full |
| **Voice Commands** | Listen state, phrase processing, intent routing | ✅ Full |
| **Replay Timeline** | Markers, divergence detection, frame branching | ✅ Full |
| **CI Dashboard** | Pipeline runs, check results, pass/fail tracking | ✅ Full |
| **Job Trace** | Execution order, determinism checking | ✅ Full |
| **State Hash Diff** | Hash comparison, component breakdown, divergence detail | ✅ Full |
| **Save File Browser** | Directory scanning, file metadata, selection | ✅ Full |
| **Proof Viewer** | TLA+ syntax highlighting, tokenizer | ✅ Full |
| **Mesh Viewer** | Mesh loading, view modes, vertex selection, bounds | ✅ Full |
| **Material Editor** | Material parameters, preview mode, dirty tracking | ✅ Full |
| **Prefab Editor** | Entity hierarchy, components, drag-and-drop composition | ✅ Full |
| **AI Debugger** | AI diagnostics, severity filtering, fix application, LLM integration | ✅ Full |

### Infrastructure

| Component | Status |
|-----------|--------|
| `EditorPanel` base class | ✅ Visibility, closable flags, name |
| `EditorLayout` | ✅ Panel registry, tree traversal rendering |
| `DockNode` tree | ✅ Recursive splits, split ratio, panel assignment |
| `EditorAttachProtocol` | ✅ Standalone, client-attach, server-attach modes |
| `CommandHistory` | ✅ Undo/redo with execute/undo stacks |

---

## Rendering Layer Status ✅

All editor panels now produce deferred draw commands via `UIDrawList`.
Panels use the established pattern of clearing the draw list, emitting
draw commands (background, title, data), and exposing a `GetDrawList()`
accessor for the rendering backend to consume.

```cpp
void SomePanel::Draw() {
    m_drawList.Clear();
    m_drawList.DrawRect({0, 0, w, h}, bgColor);
    m_drawList.DrawText({4, 4, w, 20}, "Title", textColor);
    // ... data-driven draw commands ...
}
```

All panels emit draw commands via `UIDrawList` which can be flushed to
any concrete `UIRenderer` implementation (OpenGL, Vulkan, or headless).

### Remaining Rendering Work

1. **Full Vulkan draw pipeline** — Vulkan renderer initializes but has
   minimal draw support. OpenGL renderer is functional.
2. **Font atlas** — Fallback placeholder glyphs work; real
   Inter-Regular.ttf not yet shipped.

### Editor Main Loop

The `editor/main.cpp` entry point:
- ✅ Initializes engine correctly (Core, Render, UI, ECS, Networking, Editor)
- ✅ Uses `EditorAttachProtocol` for mode selection
- ✅ Input events routed from platform window to UIEventRouter
- ✅ Diagnostics overlay toggled via Ctrl+` (backtick) key
- ✅ Mouse position tracked for overlay and UI hit-testing
- ✅ `BuildEditorUI()` parses DefaultEditorDSL and builds widget tree from parsed nodes
- 🔧 Falls back to hard-coded layout only when DSL parsing fails

### AI Assistant

| Component | Status | Gap |
|-----------|--------|-----|
| `AtlasAICore` | ✅ Intent registry, permissions, request routing | — |
| `AtlasAssistantPanel` | ✅ Prompt/suggestion UI framework | Suggestions are placeholder |
| `AIDiffViewerPanel` | ✅ Hunk accept/reject workflow | No generated diffs |
| `EditorAssistant` | ✅ Router with LLM backend integration | Falls back to hardcoded when no LLM |
| `AssetGraphAssistant` | ✅ LLM-powered suggestions, explanations, mutations | Falls back to templates when no LLM |
| `AIBackend` | ✅ OfflineLLMBackend + HttpLLMBackend + LLMBackendFactory | Production deployment via env vars |

### Game Packager

- ✅ UI scaffolding (target selection, build mode, settings)
- ✅ Build wired to `GamePackager::Package()` with `AssetCooker` + `BuildProfile` integration

---

## What's Fully Missing ❌

| Feature | Description | Blocking? |
|---------|-------------|-----------|
| **Play-In-Editor** | ✅ Implemented — Simulate, Pause, Step, Possess Entity, Client-Server Loopback, State Restore | Done |
| **Mesh Viewer** | ✅ Implemented — 3D preview with wireframe toggle, normals, vertex selection | Done |
| **Material Editor** | ✅ Implemented — Shader parameter editing, preview mode, dirty tracking | Done |
| **Prefab Editor** | ✅ Implemented — Entity hierarchy, component composition, parent-child relationships | Done |
| **Quest/Story Editor** | ✅ Implemented — Node graph, connections, preview mode, export/import | Done |
| **Inventory Editor** | ✅ Implemented — Items, filtering, sorting, export/import | Done |
| **Permission Enforcement** | ✅ Implemented — RequestOperation validates tier + mode restrictions | Done |
| **Layout Persistence** | ✅ Implemented — Save/restore panel arrangement to/from JSON | Done |
| **Live Edit Rules** | ✅ Implemented — HotReloadConfig with tick-gated changes, locking, versioning | Done |

---

## Path to a Fully Functional Editor

### Phase A — Minimum Viable Editor ✅

1. ✅ Wire the Atlas custom UI rendering backend (`UISceneGraph`, `UILayoutSolver`, `WidgetDSL`)
2. ✅ Wire `Draw()` methods to render their maintained state via UIDrawList
3. ✅ Replace hard-coded layout with DSL-driven or saved layout
4. ✅ Verify all 20+ panels render correctly

### Phase B — Core Workflows ✅

1. ✅ Implement Play-In-Editor (simulate mode)
2. ✅ Wire Game Packager to AssetCooker + BuildProfile
3. Connect AI assistant to a local LLM or API endpoint
4. ✅ Add layout save/restore

### Phase C — Polish (Months)

1. Implement remaining editor panels (material, quest, inventory)
2. Permission tier enforcement
3. Unreal-grade aesthetics (dark theme, icons, typography)
4. Editor self-hosting via Atlas GUI DSL

---

## Test Coverage

All editor panel logic is covered by the existing test suite:

- `test_next_tasks.cpp` — StateHashDiffPanel, divergence, component breakdown
- `test_next_tasks_phase2.cpp` — SaveFileBrowserPanel, hash ladder, replay
- `test_replay_timeline_panel.cpp` — Timeline, markers, divergence callbacks
- `test_profiler_panel.cpp` — Frame timing, system metrics
- `test_atlas_assistant_panel.cpp` — Prompt, suggestions, conversation
- `test_ai_diff_viewer.cpp` — Hunk accept/reject, apply
- `test_ci_dashboard.cpp` — Pipeline runs, checks, history
- `test_game_mechanics_ui.cpp` — Elements, preview, export/import
- `test_engine_input_routing.cpp` — UIEventRouter wiring, mouse tracking, overlay toggle
- `test_next_tasks_phase3.cpp` — Launcher descriptor parsing, CertifiedBuild steps, DSL layout verification
- `test_mesh_viewer.cpp` — Mesh loading, view modes, vertex selection, bounds
- `test_material_editor.cpp` — Material parameters, preview mode, dirty tracking
- `test_prefab_editor.cpp` — Entity hierarchy, components, parent-child relationships
- `test_panel_draw_impl.cpp` — AssetBrowser, NetInspector, JobTrace, StateHashDiff Draw() rendering
