# Atlas Engine — Replay & Formal Verification System

Atlas treats replay as a first-class system: every simulation state transition is
recorded, reproducible, and formally verifiable. A cryptographic hash ladder
guarantees cross-machine determinism, and TLA+ specifications ensure correctness
of core invariants. Together these systems close the loop between "it ran
correctly once" and "it is provably correct."

## Replay Model

Replay is a trace of three streams merged into a single deterministic timeline:

| Stream | Contents |
|--------|----------|
| UI Events | Mouse, keyboard, gamepad, voice — everything the input layer produces |
| ECS State | Full component snapshots at each tick boundary |
| Inputs | Network messages, RNG seeds, authority transitions |

### Binary Replay Format v1

Replay files use a deterministic binary serialization (little-endian, fixed-width
integers, no padding) so that identical simulations produce byte-identical files
regardless of compiler or platform. Every tick is recorded with a full state
snapshot and the input vector that produced it.

```
ReplayFile (v1)
 ├── Header        — magic, version, tick count, hash algorithm ID (currently fixed to BLAKE3; field exists for forward compatibility)
 ├── TickRecord[]   — one per simulation tick
 │    ├── tick_id        : u64
 │    ├── ecs_snapshot   : CompressedBytes
 │    ├── input_vector   : CompressedBytes
 │    └── hash           : [u8; 32]   (BLAKE3)
 └── Footer        — final hash, file checksum
```

## Hash Ladder

The hash ladder is a chained commitment scheme over the simulation timeline:

```
H[0] = Hash(InitialState)
H[n] = Hash(H[n-1] || ECS_State[n] || Inputs[n])
```

- **Algorithm:** BLAKE3 (256-bit)
- **Purpose:** cryptographic proof of determinism
- **Cross-machine comparison:** identical inputs must produce an identical hash
  sequence; any divergence is detected at the first differing tick
- The final hash in the ladder is written to the replay footer and serves as a
  fingerprint for the entire run

## Replay Timeline Panel

The Replay Timeline is a non-closable editor panel (see
[13_EDITOR_UI.md](13_EDITOR_UI.md)) that provides interactive control over
recorded sessions.

| Feature | Description |
|---------|-------------|
| Tick scrubbing | Drag through the timeline to jump to any tick |
| Side-by-side comparison | Load two replay files and view them in parallel |
| Branch from tick | Fork a new live session starting at any recorded tick |
| Input script injection | Insert synthetic input sequences at arbitrary points |
| Hash match indicators | Green/red markers showing per-tick hash agreement |
| Auto-diff on divergence | Automatically opens the state diff view when a hash mismatch is detected |

## Replay Controller Interface

`ReplayController` manages playback state and exposes a programmatic API for
editor panels and CI scripts.

```
ReplayController
 ├── Play()           — begin or resume forward playback
 ├── Pause()          — freeze at current tick
 ├── Step(n)          — advance exactly n ticks
 ├── Rewind(tick_id)  — jump backward to a specific tick
 ├── SetSpeed(factor) — 0.25× to 16× playback speed
 └── GetState()       — current tick, hash, playback status
```

- **Event bus hooks:** the controller emits `OnTickAdvanced`, `OnPaused`,
  `OnHashDivergence`, and `OnReplayEnd` events that other systems can subscribe
  to
- **Editor integration:** the Timeline Panel, Inspector, and Determinism Status
  panels all listen to these events to stay synchronized

## Proof System (TLA+)

Atlas maintains TLA+ specifications for critical subsystems. These specs are
machine-checked, not advisory documentation.

| Spec File | Domain | Key Invariants |
|-----------|--------|----------------|
| `ecs.tla` | ECS execution | System execution order is deterministic; systems are pure (no hidden state); tick output depends only on tick input |
| `replay.tla` | Hash ladder | Hash chain is append-only; replaying identical inputs produces identical hashes; no tick is skipped or duplicated |
| `layout.tla` | GUI layout solver | No sibling overlap; full parent coverage; min-size respected; dock mutations are commutative where expected |

### CI Integration

The TLC model checker runs automatically on every commit that modifies a `.tla`
file. A failing invariant blocks the merge.

## Proof Viewer Panel

The Proof Viewer is an optional editor panel that displays TLA+ specifications
and their verification results.

| Feature | Description |
|---------|-------------|
| Syntax highlighting | TLA+ keywords, operators, and identifiers are color-coded |
| Counterexample visualization | Failed invariants render the violating state trace as a navigable timeline |
| Proof failure → state diff | Clicking a counterexample state opens the ECS diff view at the corresponding tick |

## Replay → Proof Importer

The importer bridges the gap between recorded replays and formal verification.

1. **Trace conversion** — replay binary is parsed into a sequence of
   `(State, Input, Hash)` triples in the format expected by TLC
2. **Spec validation** — the converted trace is checked against `replay.tla` and
   `ecs.tla` invariants
3. **Hash equivalence proofs** — given two replay files from different machines,
   the importer verifies that their hash ladders are identical and reports the
   first divergent tick if they are not

## Cross-Engine Replay Equivalence

Atlas supports comparing its simulation output against a reference VM to catch
platform-specific non-determinism.

| Comparison | Method |
|------------|--------|
| Atlas ↔ reference VM | Run identical inputs through both engines; compare hash ladders tick-by-tick |
| Cross-platform validation | CI executes the same replay on Linux, Windows, and macOS; all three must produce identical final hashes |

## Determinism Contract

Determinism is enforced by a versioned contract that locks every element of the
simulation pipeline that affects replay equivalence.

### `determinism.json`

```json
{
  "version": 1,
  "replay_binary_format": "v1",
  "ecs_step_ordering": "topological_stable",
  "authority_resolution": "server_authoritative",
  "scheduler_semantics": "fixed_tick_64hz",
  "hash_ladder": {
    "algorithm": "BLAKE3",
    "chain_mode": "append_only"
  }
}
```

### `DETERMINISM_VERSION.md`

A human-readable companion document that describes the current determinism
contract in prose, including rationale for each locked parameter.

### CI Enforcement

- `determinism.json` must not change without an explicit version bump reviewed
  by at least two maintainers
- Any replay hash divergence between platforms fails the CI pipeline immediately

## Deterministic Allocator

The engine provides an arena allocator designed to eliminate
allocation-order-dependent behavior.

| Property | Guarantee |
|----------|-----------|
| Stable IDs | Every allocation returns a deterministic integer ID, not a pointer |
| No pointer-order dependence | Iteration order is defined by ID, never by memory address |
| Arena lifetime | Arenas are tick-scoped; all allocations within a tick share a single arena that is reset at tick end |
| Reproducibility | Given identical inputs, the allocator produces identical ID sequences across runs and platforms |

## Rendering Determinism (GPU-Level)

Replay fidelity extends to the rendering pipeline through a custom shader
intermediate representation.

### AtlasShaderIR

- **Purpose:** a portable, deterministic shader IR that abstracts over
  platform-specific GPU behavior
- **Compilation:** `AtlasShaderIR → SPIR-V` compiler emits validated SPIR-V
  modules
- **Validation:** the compiler rejects non-deterministic GPU operations
  (unordered atomics, undefined-order blending) at compile time
- **Font rasterization:** text rendering uses a deterministic rasterization
  policy — glyph bitmaps are pre-baked at fixed sizes and stored in a companion
  `.atlas_glyphs` archive alongside the replay file so that font engine
  differences do not cause visual divergence

## CI Integration

Replay and proof validation are embedded in the standard CI pipeline.

| Check | Trigger | Failure Behavior |
|-------|---------|------------------|
| Replay stability tests | Every commit | Hash divergence from baseline → block merge |
| UI replay → video export | Every commit | Exported videos are stored as CI artifacts for visual review |
| Cross-platform hash comparison | Nightly (Linux, Windows, macOS) | Any platform divergence → block merge and open issue |
| TLC proof validation | Any `.tla` file change | Invariant violation → block merge |
