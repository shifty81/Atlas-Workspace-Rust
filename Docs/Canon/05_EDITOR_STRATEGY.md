# Editor Strategy

## Goals

- Reduce editor count to a focused primary tool roster
- Maximize reuse through shared panels and services
- Enforce shared panel residency across tools
- Stop uncontrolled editor expansion

## Definitions

| Term | Meaning |
|------|---------|
| **Tool** | Top-level editor application within the workspace |
| **Panel** | Reusable UI component shared across tools |
| **Service** | Non-visual backend system shared across tools |
| **Plugin** | Project-specific extension loaded through adapter |
| **Archive** | Deferred or retired concept not in active build |

## Primary Tool Roster (up to 20 tools)

| # | Tool | Header | Purpose |
|---|------|--------|---------|
| 1 | Workspace Browser | `WorkspaceBrowserTool.h` | Project/workspace navigation |
| 2 | Scene Editor | `SceneEditorTool.h` | Scene/world editing, viewport, entity placement |
| 3 | Asset Editor | `AssetEditorTool.h` | Asset management, import, tagging, dependencies |
| 4 | Material Editor | `MaterialEditorTool.h` | Material/shader authoring, node graph |
| 5 | Visual Logic Editor | `VisualLogicEditorTool.h` | Blueprint/graph logic, state graphs |
| 6 | Animation Editor | `AnimationEditorTool.h` | Timeline, skeleton, blend trees, cinematics |
| 7 | UI Editor | `UIEditorTool.h` | HUD/UI layout, widget authoring |
| 8 | Data Editor | `DataEditorTool.h` | Data tables, config, dialogue, localization |
| 9 | Build Tool | `BuildTool.h` | Build pipeline, packaging, presets |
| 10 | AtlasAI Tool | `AtlasAITool.h` | AI broker, Codex, diagnostics, build-log routing |
| 11 | Project Systems Tool | `ProjectSystemsTool.h` | Adapter host — game-specific panels via IGameProjectAdapter |
| 12 | Particle / VFX Editor | `ParticleEditorTool.h` | Particle systems, VFX graphs, trails |
| 13 | Audio Editor | `AudioEditorTool.h` | Audio mixer, sound graph, SFX authoring |
| 14 | Physics Editor | `PhysicsEditorTool.h` | Colliders, rigid bodies, constraints, cloth/fluid sim |
| 15 | Terrain / World Builder | `TerrainEditorTool.h` | Terrain sculpting, foliage, road tools, world gen |
| 16 | Cinematic / Sequencer | `CinematicEditorTool.h` | Cutscenes, sequencer tracks, camera paths |
| 17 | Profiler / Diagnostics Tool | `ProfilerTool.h` | CPU/GPU profiler, memory, frame stats, debug draw |
| 18 | Version Control Tool | `VersionControlTool.h` | Source control surface, diff, history, changelists |
| 19 | Scripting Console | `ScriptingConsoleTool.h` | Live scripting surface, REPL, macro recording |
| 20 | Settings / Platform Tool | `SettingsTool.h` | Workspace settings, platform profiles, display, input |

## Project Adapter Tools

Game-specific authoring tools live in their own project source tree (e.g., `NovaForge/Source/Editor/`) and surface into the workspace shell through `ProjectSystemsTool` + `IGameProjectAdapter`. They do **not** count against the 20-tool limit.

Examples of project adapter tools (NovaForge):
- Economy Editor, Inventory Editor, Item Shop Editor
- Quest Editor, Daily Quest Editor, Progression Editor
- Character Creator Editor, Costume Editor
- Biome Editor, Ecosystem Editor, Dungeon Generator
- Trophy Editor, Virtual Currency Editor
- Season Pass Editor, Reward System Editor

**Core contract headers** that must always remain in workspace core:
- `IGameProjectAdapter.h` — adapter interface contract
- `IHostedTool.h` — hosted tool interface
- `ProjectSystemsTool.h` — adapter host tool (primary tool #11)

## Shared Panels

These are reusable, owned by workspace core, not duplicated per tool:

- Inspector / Properties
- Outliner / Hierarchy
- Content Browser
- Console / Log
- Command Palette / Search
- Notification Center
- Asset Preview
- Viewport Overlays

## Shared Services

- Command Bus
- Selection Model
- Undo/Redo
- Event Bus
- Asset Resolution
- Layout Save/Load
- Validation Framework
- AtlasAI Broker Bridge

## Rules

- No new one-off standalone editors without justification
- Primary tool roster cap is 20 workspace-native tools
- Panels are shared once, reused everywhere
- Services are centralized in NF::Workspace
- Project-specific authoring surfaces are hosted through adapters, not workspace core
- Game-specific authoring tools belong in the project's own Source/Editor/ and load via IGameProjectAdapter — they do not count against the 20
- Everything beyond the primary roster must be classified as panel, service, plugin, or archive
- One-off editors belong in tool sub-directories (Scene/, Asset/, etc.) not the root include dir
- WorkspaceShell is tool-agnostic — tools register via CoreToolRoster, not hardcoded in the shell
- New tool-specific sub-panels go in the parent tool's sub-directory (e.g., Scene/TerrainEditor.h)
