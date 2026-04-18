# Atlas Engine — Flow Graph System

The Flow Graph is a Blueprint-like visual scripting system for authoring gameplay
logic, UI behavior, tutorials, modeling pipelines, and AI behavior trees. Every
graph is deterministic by construction: nodes are either pure or explicitly
side-effecting, and the compiler enforces this classification at build time.
Flow graphs compile to Graph VM bytecode (see [04_GRAPH_VM.md](04_GRAPH_VM.md)).

## Overview

- **Visual authoring** — node-and-wire editor for logic, no text code required
- **Deterministic by construction** — all nodes are pure or explicitly side-effecting
- **Schema-validated** — every port, connection, and graph structure is checked against a schema
- **Hot-reloadable** — graph changes take effect immediately in the editor
- **Compiles to bytecode** — final output is Graph VM bytecode, identical to hand-written graphs

## Flow Graph IR (Intermediate Representation)

Flow graphs are serialized to a version-controlled intermediate representation
before compilation. The IR is the single source of truth for a graph's structure
and is designed for diffability and long-term stability.

| Property | Guarantee |
|----------|-----------|
| Format | JSON-based, one node per line where practical |
| Diff-friendly | Stable key ordering, no cosmetic churn on re-save |
| Schema-validated | Every IR file is validated against `schemas/flow_graph.schema.json` before compilation |
| Versioned | `format_version` field enables automatic migration between versions |
| Deterministic compilation | Identical IR always produces identical bytecode |

### IR Structure

```
FlowGraphIR
 ├── header        — format version, graph name, graph type (gameplay / ui / ai)
 ├── nodes[]       — ordered list of node definitions
 │    ├── id            : u64
 │    ├── type          : NodeType
 │    ├── ports_in[]    : TypedPort
 │    ├── ports_out[]   : TypedPort
 │    └── metadata      : position, comment, color
 ├── edges[]       — connections between ports
 │    ├── from_node     : u64
 │    ├── from_port     : u16
 │    ├── to_node       : u64
 │    └── to_port       : u16
 └── config        — graph-level settings (determinism mode, tick budget)
```

## Node Types

| Category | Nodes | Description |
|----------|-------|-------------|
| Logic | Branch, Loop, Compare, Select | Control flow decisions and iteration |
| Math | Add, Sub, Mul, Div, Clamp, Lerp | Arithmetic and interpolation |
| ECS | Query, Mutate, Spawn, Destroy | ECS entity and component operations |
| UI | ShowPanel, SetText, BindData | Widget manipulation (UI graphs only) |
| Event | OnTick, OnInput, OnStateChange | Entry points triggered by engine events |
| Control Flow | Sequence, Parallel, Delay | Execution ordering and tick-based delays |
| Custom | Project-defined | Game projects register custom node types |

## Node Operation Schema

Each node declares typed input and output ports. The compiler validates every
connection against these declarations.

```cpp
struct PortDecl {
    PortID      id;
    std::string name;
    TypeID      type;       // schema-validated type
    PortDir     direction;  // In or Out
};

enum class DeterminismCategory : uint8_t {
    Pure,              // No side effects, same inputs → same outputs
    Effect,            // Side-effecting but deterministic (e.g., ECS write)
    Nondeterministic,  // Requires explicit opt-in (e.g., system time)
};

struct NodeDecl {
    NodeType             type;
    std::vector<PortDecl> ports;
    DeterminismCategory  determinism;
};
```

- **Type compatibility** — connections are rejected at edit time if port types
  do not match or cannot be implicitly converted
- **Determinism propagation** — a graph containing any `Nondeterministic` node
  is itself classified as nondeterministic

## Graph Restrictions

Graphs are classified by type, and each type enforces a restriction set that the
compiler checks before emitting bytecode.

| Graph Type | Allowed Node Categories | Forbidden Operations |
|------------|------------------------|----------------------|
| Gameplay | All | None (full access) |
| UI | Logic, Math, UI, Event, Control Flow | ECS mutations (Spawn, Destroy, Mutate) |
| AI | Logic, Math, ECS (read-only), Event (OnTick, OnStateChange only), Control Flow | ECS mutations, UI operations |

- **Nondeterministic nodes** are flagged at compile time with a warning; they
  must be explicitly allowed per graph via the `config.allow_nondeterministic`
  flag or the build fails
- **UI graphs** enforce a safe subset: no ECS writes, no spawning, no direct
  world mutation — they may only read ECS data through bound properties

## Graph Debugger

The debugger attaches to a running Graph VM instance and provides interactive
inspection of graph execution.

| Feature | Description |
|---------|-------------|
| Node breakpoints | Pause execution when a specific node is reached |
| Edge breakpoints | Pause when a value is transmitted along a specific edge |
| Conditional pauses | Break when a port value matches a user-defined condition |
| Timeline stepping | Step forward and backward through the graph execution timeline |
| Authority heatmap | Color overlay showing execution frequency per node over a time window |
| Live value inspection | Hover any port to see its current value during paused execution |

- **Editor integration:** the debugger state is displayed in the Graph Editor
  Panel and synchronized with the Replay Timeline (see
  [14_REPLAY_AND_PROOFS.md](14_REPLAY_AND_PROOFS.md))
- **Breakpoint persistence:** breakpoints are saved per-graph in the editor
  project file and survive editor restarts

## Graph → C++ Codegen

For release builds, flow graphs can be ahead-of-time compiled to C++ source
files that are then compiled with the engine. This eliminates the Graph VM
interpreter overhead while preserving identical behavior.

### Codegen Pipeline

```
FlowGraphIR → Codegen Compiler → Generated .cpp/.h → Engine Build
```

| Property | Guarantee |
|----------|-----------|
| Behavioral equivalence | Generated C++ produces identical outputs to the interpreted graph for all inputs |
| Determinism preserved | The same determinism classification applies; nondeterministic nodes produce compile-time warnings in codegen and are gated behind `config.allow_nondeterministic` |
| Debug symbols | Generated code includes source-map comments linking back to node IDs in the IR |
| Incremental | Only changed graphs are re-generated; unchanged graphs reuse cached output |

### Generated Code Shape

```cpp
// Auto-generated from FlowGraphIR — do not edit
// Graph: DamageCalculation (gameplay)
// Node count: 5

void DamageCalculation_Execute(GraphContext& ctx) {
    // [Node 1] LOAD target_hp
    auto target_hp = ctx.LoadVar(0);
    // [Node 2] LOAD weapon_damage
    auto weapon_damage = ctx.LoadVar(1);
    // [Node 3] SUB
    auto result = target_hp - weapon_damage;
    // [Node 4] STORE target_hp
    ctx.StoreVar(0, result);
    // [Node 5] EMIT_EVENT DamageApplied
    ctx.EmitEvent(EventID::DamageApplied);
}
```

## UI Logic Graphs

UI logic graphs are a restricted subset of flow graphs designed for safe,
side-effect-free UI behavior. They bind to widget events exposed by the GUI
system (see [12_GUI_SYSTEM.md](12_GUI_SYSTEM.md)).

| Capability | Example |
|------------|---------|
| Panel visibility | Show/hide panels based on game state |
| Data display | Bind ECS component values to text labels |
| Input handling | Route button clicks to UI-scoped actions |
| Animation triggers | Start/stop widget animations on state change |
| State machines | Manage multi-step UI flows (menus, wizards, tutorials) |

- UI graphs **cannot** mutate ECS state — they read through bound properties
  and emit UI-scoped events that the gameplay layer consumes
- This separation ensures that UI logic never causes desynchronization in
  networked simulations

## Graph Editor Panel

The Graph Editor is a dockable panel in the Atlas editor (see
[13_EDITOR_UI.md](13_EDITOR_UI.md)) that provides the primary authoring surface
for flow graphs.

| Feature | Description |
|---------|-------------|
| Node palette | Searchable list of available node types, filtered by graph type |
| Execution order overlay | Numbered badges showing the compiler-determined execution sequence |
| Tick cost display | Per-node cost estimate (in microseconds) displayed as a badge |
| Nondeterminism highlighting | Nodes classified as `Nondeterministic` are outlined in red |
| Source navigation | Click a node to jump to its declaration in source code or schema config |
| Minimap | Zoomed-out overview of the full graph for navigation |
| Undo/redo | Full undo history for all graph editing operations |

## Versioning & Migration

The flow graph IR format is versioned to support long-term evolution without
breaking existing graphs.

| Rule | Detail |
|------|--------|
| Version field | Every IR file contains a `format_version` integer in its header |
| Automatic migration | On load, graphs at older versions are migrated forward through a chain of version-specific transforms |
| Backward compatibility | The engine always supports loading graphs from at least the previous two major versions |
| Migration testing | Each migration transform has a corresponding test that round-trips a known graph through the upgrade path |
| No silent data loss | Migrations that cannot preserve a node emit a compile-time warning listing affected nodes |

### Migration Pipeline

```
Load IR (v_old) → Migration v_old→v_old+1 → ... → Migration v_cur-1→v_cur → Compile
```
