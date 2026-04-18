# Panel and Service Matrix

## Shared Panels

These are reusable UI surfaces owned by workspace core, used across multiple tools.

| Panel | Used By | Residency | State |
|-------|---------|-----------|-------|
| Inspector | Scene, Asset, Material, Animation, Data | Workspace Core | Partial |
| Outliner/Hierarchy | Scene, Asset, Animation | Workspace Core | Partial |
| Content Browser | All tools | Workspace Core | Partial |
| Console/Log | All tools | Workspace Core | Done |
| Command Palette | All tools | Workspace Core | Done |
| Notifications | All tools | Workspace Core | Done |
| Properties/Details | Scene, Material, Asset, Data | Workspace Core | Partial |
| Viewport | Scene, Material, Particle, Animation | Workspace Core | Partial |
| Asset Preview | Asset, Material, Animation | Workspace Core | Planned |
| Settings | All tools | Workspace Core | Done |
| AtlasAI Chat | All tools | Workspace Core | Partial |
| Diagnostics | Build, AtlasAI, All tools | Workspace Core | Partial |
| Memory Profiler | Diagnostics | Workspace Core | Partial |
| Render Stats | Scene, Material | Workspace Core | Partial |
| Pipeline Monitor | Build, AtlasAI | Workspace Core | Partial |

## Shared Services

These are non-visual backend systems.

| Service | Used By | Residency | State |
|---------|---------|-----------|-------|
| Command Bus | All tools | Workspace Core | Done |
| Selection Model | All tools | Workspace Core | Done |
| Undo/Redo | All tools | Workspace Core | Done |
| Event Bus | All tools | Workspace Core | Partial |
| Asset Resolution | All tools | Workspace Core | Partial |
| Layout Save/Load | All tools | Workspace Core | Done |
| Validation Framework | Build, Data, Asset | Workspace Core | Planned |
| AtlasAI Broker Bridge | AtlasAI, Notifications | Workspace Core | Partial |
| File Intake | All tools | Workspace Core | Planned |
| Codex Mirror | AtlasAI, Build | Workspace Core | Planned |
| Graph Host Contract | Logic Editor, Shader Editor | Workspace Core | Planned |
| Hot Reload | All tools | Workspace Core | Planned |
| Version Control | All tools | Workspace Core | Planned |
| Scripting | Logic Editor, Console | Workspace Core | Planned |

## Project Adapter Panels (NovaForge)

These panels live in `NovaForge/Source/Editor/include/NovaForge/Editor/` and are loaded via IGameProjectAdapter through ProjectSystemsTool. They do **not** live in workspace core.

| Panel | Host Tool | State |
|-------|-----------|-------|
| Economy Panel | Project Systems | Planned |
| Inventory Rules Panel | Project Systems | Planned |
| Shop Panel | Project Systems | Planned |
| Mission Rules Panel | Project Systems | Planned |
| Progression Panel | Project Systems | Planned |
| Character Rules Panel | Project Systems | Planned |
| Quest Panel | Project Systems | Planned |
| Biome Panel | Project Systems | Planned |
| Ecosystem Panel | Project Systems | Planned |
| Trophy/Virtual Currency Panel | Project Systems | Planned |
| Dungeon/Room Generator | Project Systems | Planned |
| Season Pass Panel | Project Systems | Planned |
| Reward System Panel | Project Systems | Planned |
