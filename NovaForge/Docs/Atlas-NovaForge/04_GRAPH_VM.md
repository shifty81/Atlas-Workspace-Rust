# Atlas Engine — Graph VM

## Purpose

The Graph VM executes gameplay, simulation, and generation logic.

It is:
- **Deterministic** — Same inputs always produce same outputs
- **Serializable** — Bytecode can be saved and loaded
- **Network-replicable** — Safe for lockstep and rollback
- **Hot-reloadable** — Editor changes take effect immediately

## Execution Model

```
Graph (Editor) → Compiler → Bytecode → Graph VM → ECS Writes
```

## Bytecode Instruction Set

```cpp
enum class OpCode : uint8_t {
    NOP = 0,
    LOAD_CONST,    // Push constant from pool
    LOAD_VAR,      // Load local variable
    STORE_VAR,     // Store to local variable
    ADD,           // Pop two, push sum
    SUB,           // Pop two, push difference
    MUL,           // Pop two, push product
    DIV,           // Pop two, push quotient
    CMP_EQ,        // Pop two, push equality result
    CMP_LT,        // Pop two, push less-than result
    CMP_GT,        // Pop two, push greater-than result
    JUMP,          // Unconditional jump
    JUMP_IF_FALSE, // Conditional jump
    EMIT_EVENT,    // Emit event to ECS/EventBus
    END            // Halt execution
};
```

## Instruction Format

```cpp
struct Instruction {
    OpCode opcode;
    uint32_t a;
    uint32_t b;
    uint32_t c;
};
```

## Graph IR (Editor-Side)

```cpp
enum class NodeType {
    Constant,
    Add, Sub, Mul, Div,
    CompareLT, Branch, Event,
};

struct Node {
    NodeID id;
    NodeType type;
    int64_t constantValue = 0;
};

struct Edge {
    NodeID from;
    NodeID to;
};

struct Graph {
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    NodeID entry;
};
```

## Network Safety

Forbidden in Graph VM:
- File IO
- Threads
- Heap allocation during execution
- System time
- Random without seed

## Example: Damage Calculation

```
LOAD target_hp
LOAD weapon_damage
SUB
STORE target_hp
EMIT_EVENT DamageApplied
```
