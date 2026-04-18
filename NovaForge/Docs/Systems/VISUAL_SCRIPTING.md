# NovaForge — Visual Scripting (GraphVM)

## System Name
**GraphVM** — NovaForge's built-in visual scripting VM

## Purpose
- Node-based logic authoring for game behaviors
- In-editor visual scripting (no code required for designers)
- Compiled to bytecode for runtime execution

## Graph Types
| Type | Use |
|---|---|
| World | World-level events and state machines |
| Strategy | Fleet/faction AI behavior |
| Conversation | Dialogue trees |
| Behavior | NPC/entity behavior |
| Mission | Mission objective logic |
| Economy | Market/trading rules |

## Node Authoring Rules
- All nodes must have typed ports (input/output)
- No circular dependencies (topological sort enforced at compile)
- All graphs must be serializable to JSON (GraphSerializer)

## Compilation
- GraphCompiler → topological sort → register allocation → bytecode emission
- Bytecode runs on GraphVM (16 registers, 256 memory slots)
- Compile via editor command: `graph.compile` (F7)

## Promotion to Codex
When a graph is validated and tested:
1. Export to `Codex/Graphs/<GraphType>/<Name>.graph.json`
2. Tag with category and version
3. Atlas Workspace mirrors to shared Codex library

## Naming Standard
- Graph files: `<Category>_<Name>.graph.json` (e.g., `Behavior_PatrolLoop.graph.json`)
- Node classes: `NF::GraphNode` with typed name
- Port types: string-keyed, type-checked at compile

## Code Reference
- `NF::GraphVM` — bytecode execution engine
- `NF::Graph` / `NF::GraphNode` / `NF::GraphPort` / `NF::GraphLink` — model
- `NF::GraphCompiler` — topological sort + bytecode emitter
- `NF::GraphSerializer` — JSON round-trip
