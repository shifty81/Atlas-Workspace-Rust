# Atlas Engine — Core Implementation Reference

> Canonical reference for Atlas engine internals. This document covers the
> interaction pipeline, asset graph, bytecode VM, content graphs, skill systems,
> AI/diplomacy, editor infrastructure, serialization formats, and networking.

---

## 1. Interaction System

### Pipeline

```
Utterance → IntentResolver → Intent → InteractionRouter → Response
```

The interaction system is **rule-based and deterministic**, making it safe for
multiplayer replication. Every player-facing interaction follows the same
pipeline regardless of input modality.

### Design Rules

| Concern | Rule |
|---|---|
| Conversation logic | Deterministic rule evaluation; no stochastic output |
| LLM sandboxing | AI may *suggest* intents but **never** executes them directly |
| Voice adapter | All output is text-only; voice synthesis is a presentation layer concern |
| NPC knowledge | Each NPC maintains a local knowledge graph scoped to what it has observed |

### Intent Resolution

The `IntentResolver` maps raw utterances to typed `Intent` objects. Resolution
order:

1. Exact keyword match
2. Pattern / template match
3. Contextual disambiguation (conversation state)
4. Fallback (unknown intent)

---

## 2. Asset Graph System

Assets are authored, composed, and baked through a node-based `AssetGraph` that
mirrors the `WorldGraph` evaluation pattern.

### Core Concepts

| Concept | Description |
|---|---|
| **Archetype** | Base template defining default properties for an asset class |
| **Mutation** | Delta applied to an archetype for procedural variation |
| **Semantic Tags** | Categorization labels (e.g. `organic`, `metallic`, `worn`) used for filtering and search |
| **Asset Recipe** | Composition rule that combines multiple archetypes into a final asset |

### Pipelines

- **Headless Bake** — CLI tool that compiles asset graphs offline without a display context.
- **Hot-Reload** — Runtime file watcher triggers rebake and seamless swap of in-memory assets.
- **LOD Pipeline** — Automatic level-of-detail mesh generation from a high-poly source.
- **Damage Mesh Pipeline** — Pre-authored or procedural damage states baked alongside the base mesh.

### Visual Consistency

- **Faction Identity** — Palette, shape language, and material rules bound to a faction archetype.
- **Economy-Driven Evolution** — Asset appearance shifts based on simulation-derived wealth/scarcity signals.
- **Wear System** — Visual wear accumulates proportionally to simulation time.

### Extensibility

- **AI-Assisted Authoring** — Generative suggestions constrained by archetype + tag rules.
- **Provenance & Debugging** — Every baked asset records its full derivation chain.
- **Mod-Safe Extension** — Mods add new archetypes/mutations without overwriting engine assets.

---

## 3. Graph IR & Compiler

### IRNode

```cpp
struct IRNode {
    uint32_t id;
    NodeType type;     // Param, Constant, Binary, Output
    OpCode   opcode;   // ADD, SUB, MUL, CLAMP, ...
    uint32_t inputs[2];
    float    constant;
    uint32_t param_index;
};
```

### GraphIR Container

```cpp
class GraphIR {
public:
    uint32_t AddParam(const char* name, float default_val);
    uint32_t AddConstant(float value);
    uint32_t AddBinary(OpCode op, uint32_t lhs, uint32_t rhs);
    uint32_t AddOutput(const char* name, uint32_t source);
};
```

### Binary Format

| Offset | Size | Field |
|--------|------|-------|
| 0 | 4 | Magic `"GIR0"` |
| 4 | 4 | Node count (little-endian u32) |
| 8 | 4 | Param count |
| 12 | 4 | Output count |
| 16 | … | Node array |

All multi-byte values are **little-endian**.

### Compiler Pipeline

```
Parse → Validate → Normalize → Toposort → Lower → Emit → Hash
```

**`GraphCompiler.h`** exposes:

```cpp
struct CompileError {
    uint32_t    node_id;
    const char* message;
};

struct CompileOptions {
    bool fold_constants  = true;
    bool eliminate_dead   = true;
    bool sort_lexicographic = true;
};

CompileResult Compile(const GraphIR& ir, const CompileOptions& opts);
```

### Normalization Rules

1. Parameters without explicit defaults are assigned `0.0f`.
2. Commutative operands are sorted lexicographically by node id.
3. Duplicate constants are merged.

### BytecodeWriter

- Performs linear-scan register allocation over the toposorted node list.
- Emits fixed-width instructions (see §4).
- Appends a param table and output map after the instruction stream.

---

## 4. Bytecode VM

### Opcodes

| Opcode | Encoding | Operands | Description |
|--------|----------|----------|-------------|
| `LOAD_PARAM` | `0x01` | dst, param_idx | Load parameter into register |
| `ADD` | `0x10` | dst, srcA, srcB | `dst = srcA + srcB` |
| `SUB` | `0x11` | dst, srcA, srcB | `dst = srcA - srcB` |
| `MUL` | `0x12` | dst, srcA, srcB | `dst = srcA * srcB` |
| `MULC` | `0x13` | dst, src, imm32 | `dst = src * immediate` |
| `CLAMP` | `0x20` | dst, src, lo, hi | `dst = clamp(src, lo, hi)` |
| `OUTPUT` | `0xF0` | output_idx, src | Write register to output slot |

All instructions are **11 bytes, fixed-width** for cache-friendly dispatch.

### Determinism Contract

| Guarantee | Mechanism |
|---|---|
| IEEE 754 compliance | No `-ffast-math`, no compiler relaxation flags |
| No denormals | Flush-to-zero enabled at VM entry |
| No heap allocation | All registers are stack-local |
| No exceptions | Error states are encoded as NaN sentinel values |
| No float branching | Control flow is data-independent |

### Optimizer Passes

1. **Constant Folding** — Evaluate pure-constant sub-expressions at compile time.
2. **Dead Node Elimination** — Remove nodes whose outputs are never consumed.

### Bytecode Hot-Patching

Live-patching rules:

- **Allowed**: overwrite immediate values, update param defaults.
- **Forbidden**: change instruction count, reorder instructions, alter register assignments.

This ensures a running VM can accept tuning changes without recompilation.

---

## 5. Content Graph Systems

Every content graph follows the same lifecycle:

```
Define Nodes → Compile (§3) → VM Execution (§4) → Output
```

### CharacterGraph

Modular layers composited in order:

1. **Body** — Base silhouette and proportions
2. **Equipment** — Slotted gear overlays
3. **Damage** — Procedural or baked damage decals

Output is a sprite sheet baked per unique combination.

### AnimationGraph

- **State Machine** — Named states with transition conditions.
- **Modifier Stack** — Additive layers applied in priority order:
  damage → emotion → skill modifiers.

### WeaponGraph

Modular construction from five slot types:

| Slot | Role |
|------|------|
| Receiver | Core frame and fire mode |
| Barrel | Range, accuracy, visual length |
| Magazine | Capacity, reload speed |
| Optics | Zoom, overlay HUD |
| Stabilizer | Recoil reduction, weight |

Each slot supports **wear** (visual degradation) and **mutation** (procedural
variation within constraints).

### TileGraph

2D autotiling system:

- Adjacency bitmask per tile (4-bit or 8-bit neighborhood).
- Tile selection by bitmask lookup table.
- Edge-blending rules for cross-biome transitions.

### SoundGraph

- **Procedural Synthesis** — Oscillators, filters, and envelopes defined as graph nodes.
- **Action Binding** — Game actions map to sound-graph trigger inputs.
- Output is a PCM buffer or a real-time audio stream.

### UIScreenGraph

State-driven UI composition:

- Screens are graph roots; widgets are child nodes.
- Visibility and layout driven by bound game-state values.
- Supports menus, HUD overlays, and inventory panels.

### StoryGraph

Narrative branching structure:

```
StoryState → Character → Conflict → Choice → Outcome
```

Each `Choice` node fans out to one or more `Outcome` nodes, which in turn
update `StoryState`. Cycles are allowed (recurring conflicts).

### GameFlowGraph

Top-level application state machine:

```
Boot → Splash → Menu → Gameplay → Credits
                 ↑         |
                 └─ Pause ←─┘
```

Transitions are triggered by engine events or player input.

---

## 6. Skill Systems

All five paradigms emit a unified effect type:

```cpp
struct SkillEffect {
    SystemID system;   // Target subsystem (combat, crafting, diplomacy, …)
    EffectType type;   // Buff, unlock, modifier, …
    float value;       // Magnitude
};
```

### Paradigms

| # | Paradigm | Progression Model |
|---|----------|-------------------|
| 1 | **Classic DAG Tree** | Prerequisite-based unlock; nodes form a directed acyclic graph |
| 2 | **Usage-Driven** | Skills improve through repeated use; XP accrues per action |
| 3 | **Modular Loadouts** | Discrete skill sets equipped/unequipped like gear |
| 4 | **Procedural Discovery** | Skills found through player experimentation and combination |
| 5 | **Faction/Civilization** | Macro-scale cultural progression tied to faction simulation |

Games may combine paradigms; the engine treats each as an independent provider
of `SkillEffect` values that merge additively.

---

## 7. AI & Diplomacy

### FactionRouter

Central dispatcher for faction-level decisions. Receives world-state snapshots
and emits faction actions (trade offers, declarations, migrations).

### DiplomacyState

```cpp
struct DiplomacyState {
    float trust;      // [-1, 1]  cumulative relationship quality
    float hostility;  // [ 0, 1]  current aggression level
};
```

### Personality Traits

| Trait | Range | Influence |
|-------|-------|-----------|
| Aggression | 0–1 | Threshold for hostile action |
| Honor | 0–1 | Weight of treaty/promise adherence |
| Paranoia | 0–1 | Sensitivity to perceived threats |

### Cultural Diffusion

Faction traits spread between neighbors via diffusion equations:

```
trait_next = trait_curr + Sum (neighbor_trait - trait_curr) * diffusion_rate * dt
```

### Advanced Mechanics

- **Player-Taught Skills** — Players can train NPC factions, transferring skill graph nodes.
- **Forbidden / Black-Market Skills** — Skills flagged as illegal within a faction's policy; usable but with diplomatic penalties.

---

## 8. Editor Infrastructure

### Retained-Mode UI

```cpp
struct UINode {
    uint32_t     id;
    LayoutMode   layout;   // Vertical, Horizontal, Dock, Absolute
    Rect         rect;
    UINode*      children;
    uint32_t     child_count;
};

struct UITree {
    UINode* root;
    void    Layout();
    void    Render();
};
```

### Docking Layout Pass Order

```
Left → Right → Top → Bottom → Center
```

Each pass consumes available space from the remaining rect, with Center
receiving whatever remains.

### Input Routing

Three-state machine:

```
Hovered ──(mouse down)──→ Focused ──(drag start)──→ Captured
    ↑                         |                          |
    └───────(release)─────────┴──────(release)───────────┘
```

- **Hovered** — Visual feedback only.
- **Focused** — Receives keyboard input.
- **Captured** — Exclusive pointer ownership (drag, resize).

### Graph Inspector

- Per-node value display synchronized to the running VM.
- **Debug Recorder** — Captures per-tick snapshots for offline replay.
- **Breakpoints** — Halt VM execution when a node output crosses a threshold.

### Editor State Machine

```
Boot → ProjectSelect → Loading → Idle → Editing → Playing → Paused
                                    ↓                           |
                                  Baking → Building             |
                                    ↓                           |
                                  Error ←───────────────────────┘
```

---

## 9. Serialization & File Formats

### Format Summary

| Extension | Purpose | Encoding |
|-----------|---------|----------|
| `.graph.yaml` | Authoring source | YAML text (diffable) |
| `.graphbc` | Compiled bytecode | Binary (see below) |
| `.atlasmesh` | Mesh cache | Binary (see below) |
| `.save` | Game save | Chunk-based binary |
| `.agraphreplay` | Debug replay | Tick-indexed binary |

### `.graphbc` Layout

| Offset | Size | Field |
|--------|------|-------|
| 0 | 4 | Magic `"GBC0"` |
| 4 | 4 | Version |
| 8 | 4 | Instruction count |
| 12 | 4 | Param count |
| 16 | 4 | Output count |
| 20 | 12 | Reserved / padding |
| 32 | 11 × N | Instruction stream |
| … | … | Param table |

Total header: **32 bytes**.

### `.atlasmesh` Layout

Header followed by interleaved buffers:

1. Vertex positions
2. Index buffer
3. Attribute buffer (normals, UVs)
4. Damage state buffer
5. Wear buffer

### `.save` Format

| Offset | Size | Field |
|--------|------|-------|
| 0 | 4 | Magic `"ATLS"` |
| 4 | 4 | Format version |
| 8 | 8 | Timestamp (Unix µs) |
| 16 | 4 | Chunk count |
| 20 | 28 | Reserved |
| 48 | … | Chunk data |

Total header: **48 bytes**. Chunks are versioned independently for
forward-readable saves.

### Delta Compression

Save diffs use a three-stage pipeline:

```
Structural Diff → Numeric Delta → RLE
```

---

## 10. Networking

### Replication Model

The **server evaluates all bytecode graphs**; clients receive outputs only.
Graph structure and parameters never leave the server.

### GraphOutputPacket

```cpp
struct GraphOutputPacket {
    EntityID   entity;
    GraphID    graph;
    uint32_t   tick;
    float      outputs[];   // variable-length
};
```

### Rollback

```cpp
struct RollbackFrame {
    uint32_t tick;
    uint64_t input_hash;
    float    graph_inputs[];   // snapshot of inputs at this tick
};
```

On misprediction the server re-evaluates from the stored frame and sends
corrected outputs.

### Security Boundary

- Text and voice data **never cross the wire** — all interaction resolution
  happens server-side and only the resulting intent/response IDs are replicated.

---

## 11. Engine Philosophy

> *"Game = Configuration"*

Games built on Atlas are **plugins**, not engine forks. The engine provides
systems; games provide data, schemas, and configuration.

### Plugin Contract

A valid game plugin must:

1. Declare its schema (node types, param ranges, output names).
2. Register content graphs with the engine's graph registry.
3. Never call internal engine APIs — only the public plugin interface.

### Schema Authoring Rules

- Every parameter must have a default value.
- Output names must be unique within a graph.
- Node types are namespaced to avoid mod collisions.

### Completeness Criterion

> *"If a new game requires engine code changes, the engine is incomplete."*

Any feature request that cannot be satisfied by configuration, schema
extension, or graph composition indicates a gap in the engine that must be
addressed at the engine level — not worked around in game code.
