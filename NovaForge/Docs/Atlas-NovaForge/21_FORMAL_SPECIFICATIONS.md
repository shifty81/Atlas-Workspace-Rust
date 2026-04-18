# Atlas Formal Specifications

Atlas uses TLA+ and related formal methods to verify critical engine invariants.
Specifications cover ECS execution ordering, replay hash integrity, GUI layout
correctness, and deterministic allocation. The TLC model checker runs in CI on
every spec change, and a proof viewer integrated into the editor lets developers
browse specifications and counterexamples inline.

## TLA+ Specifications

### `ecs.tla` — ECS Execution Model

Models deterministic system execution order within the ECS. The specification
captures entity creation and destruction, component access patterns, and the
fixed ordering guarantee that underpins replay determinism.

| Invariant | Description |
|-----------|-------------|
| Fixed ordering | Systems execute in their registered order every tick |
| No hidden state | All state is visible through the component store |
| Pure function semantics | System functions are pure: same inputs produce same outputs |
| No cross-system mutation | A system may not write to components owned by another system during the same step |

The core specification is approximately 10–15 lines of TLA+ that model a
sequence of system steps over a set of entities and components, with refinement
predicates linking the abstract model to the real ECS implementation.

### `replay.tla` — Replay Hash Ladder

Models the chained hash commitment scheme used by the replay system:

```
H[n] = Hash(H[n-1] || ECS_State[n] || Inputs[n])
```

| Invariant | Description |
|-----------|-------------|
| Chain integrity | Every hash is derived from exactly the previous hash, the current state, and the current inputs |
| No gaps | The hash sequence is contiguous — no tick may be skipped |
| Deterministic construction | Identical input sequences produce identical hash sequences |

The specification also includes an authority model (server-authoritative
transitions) and rollback semantics (state restoration that preserves the hash
ladder).

### `layout.tla` — GUI Layout Solver

Models integer-only layout computation for the dockable panel system. The
specification is approximately 10–15 lines of TLA+ covering four key invariants:

| ID | Invariant | Description |
|----|-----------|-------------|
| I1 | No overlap | No two panels may occupy the same pixel region |
| I2 | Full coverage | The union of all panels covers the entire available viewport |
| I3 | Minimum size | Every panel respects its declared minimum width and height |
| I4 | Deterministic docking | Panel arrangement is determined solely by docking order, not by insertion timing |

## Deterministic Allocator Specification

The arena allocator has a formal model that guarantees allocation-order
independence. This is the foundation for cross-platform determinism in the ECS
and replay systems.

| Property | Guarantee |
|----------|-----------|
| Stable IDs | Every allocation returns a deterministic integer ID, not a pointer |
| No pointer-order dependence | Iteration order is defined by ID, never by memory address |
| ID-based scheme | Allocation identity is an integer token, decoupled from physical address |
| Platform independence | Given identical inputs, the allocator produces identical ID sequences on every platform |

## Authority & Rollback Specification

The networking layer uses a server-authoritative model with client-side
prediction. The formal specification covers:

| Property | Description |
|----------|-------------|
| Server authority | The server is the single source of truth for all simulation state |
| Client prediction | Clients may run ahead speculatively; predictions are discarded on mismatch |
| Rollback safety | State restoration preserves the hash ladder — rolling back and replaying must produce the same hashes |
| Merge semantics | Concurrent authority changes are resolved with a deterministic merge function |
| Partition handling | Network partitions are detected and resolved without diverging the hash ladder |

## Proof Viewer Panel

The editor includes a dedicated proof viewer panel for working with TLA+
specifications:

- **Syntax-highlighted TLA+ display** — specifications render with full syntax
  highlighting inside the editor
- **Inline browsing** — navigate specs without leaving the editor workspace
- **Counterexample visualization** — when TLC finds a counterexample, the viewer
  displays the violating state sequence as a navigable timeline
- **Linked navigation** — proof failures link directly to the ECS state diff and
  the corresponding position on the replay timeline

## Replay → Proof Importer

The proof importer bridges runtime execution and formal verification:

| Step | Description |
|------|-------------|
| Trace export | Runtime replay traces are serialized into a TLA+ checkable format |
| Spec validation | Exported traces are checked against the formal specification to confirm the real execution satisfies all invariants |
| Hash equivalence | The importer generates hash equivalence proofs that compare execution across machines |
| Counterexample output | When a trace violates a specification, the importer produces a counterexample that can be loaded in the proof viewer |

## Cross-Engine Replay Equivalence

Atlas provides a formal framework for comparing its execution against a
reference VM. This is used for cross-platform determinism verification.

| Comparison | Method |
|------------|--------|
| Atlas ↔ reference VM | Run identical inputs through both engines; compare hash ladders tick-by-tick |
| Cross-platform validation | CI executes the same replay on multiple platforms; all must produce identical final hashes |
| Equivalence proof | The hash ladder comparison serves as the equivalence proof between implementations |

## CI Integration

TLA+ verification is embedded in the standard CI pipeline.

| Check | Trigger | Failure Behavior |
|-------|---------|------------------|
| TLC model checking | Any `.tla` file change | Invariant violation → block merge |
| Counterexample archival | TLC failure | Counterexamples stored as CI artifacts |
| Proof viewer loading | CI counterexample produced | Proof viewer can load counterexamples directly from CI artifacts |
| Spec validation gate | Every PR modifying specs | Spec validation must pass before merge is permitted |

## Render & Audio Determinism

Headless and stub modes maintain formal guarantees without requiring real
hardware output.

| Stub | Behavior |
|------|----------|
| Render stubs | Produce verifiable output without a GPU; output is clearly synthetic, never pretending to be real rendering |
| Audio stubs | Produce deterministic placeholder output; waveform data is reproducible across runs |
| Hash integrity | Both stub types participate in the hash ladder — stubbed output hashes identically across platforms |
| "Stubs without lying" | Stub output is explicitly marked as synthetic so it cannot be confused with real render or audio output |

## File Locations

| Path | Contents |
|------|----------|
| `specs/` | TLA+ specification files (`ecs.tla`, `replay.tla`, `layout.tla`) |
| `engine/core/contract/` | Compile-time enforcement headers that link formal invariants to C++ static assertions |
| `tools/` | Proof tooling, CI integration scripts, and the replay → proof importer |
