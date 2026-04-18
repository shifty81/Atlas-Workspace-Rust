# Atlas Engine — Template Repository System

Atlas is a forkable, game-specific operating system for game development.
The canonical workflow is fork → rename → restructure → ship. After
specialization, Atlas disappears as a name — the game owns the repo.
Forking Atlas does NOT weaken determinism guarantees.

## Overview

Atlas ships as a canonical template repo, not a framework dependency.
Games do not link against Atlas at runtime or pull it as a package — they
fork the entire repository, specialize it, and ship. The engine core
remains locked; everything else is game-owned.

## Template Structure

The template repository ships with the following layout:

```
atlas-template/
├── engine/          ← Atlas core (deterministic, invariant)
├── game/            ← Game-specific logic (dominant)
├── editor/          ← Self-hosted editor (configurable)
├── assets/          ← Deterministic asset graphs
├── specs/           ← TLA+/formal proofs
├── tools/           ← CI, lint, replay, AI
├── run/             ← One-click executables
├── atlas.build.json
└── README.md
```

After specialization a fork is restructured to reflect the game it serves:

```
my_game/
├── core/            ← Forked Atlas engine (locked)
├── gameplay/        ← ECS, graphs, AI, rules
├── content/         ← Assets, worlds, models
├── editor/          ← Game-branded editor
├── proofs/          ← Determinism + netcode
├── ci/
├── run/
└── GAME.md
```

## Atlas Init

`atlas init --game "GameName"` performs the following operations:

| Step | Description |
|------|-------------|
| Repo renaming | Top-level directory and project file updated |
| Namespace rewrite | `Atlas::` → `GameName::` across all source files |
| Binary renaming | Executable names reflect the game |
| Editor branding | Welcome screen, window titles, icons updated |
| CI pipeline rebinding | Workflow files retargeted to the new repo |
| Replay hash domain reset | New hash domain seed for the game's replay ladder |
| Core directory freezing | Engine directory marked read-only in CI |

## What Is Fixed vs What Is Game-Owned

### Immutable (Engine Contract)

Copied once, then locked. These components form the determinism contract
and must not be modified by game code:

- Deterministic scheduler
- Replay system (see [14_REPLAY_AND_PROOFS.md](14_REPLAY_AND_PROOFS.md))
- Allocator
- Graph VM (see [04_GRAPH_VM.md](04_GRAPH_VM.md))
- Formal spec interfaces
- CI validation rules

### Game-Owned (Fully Mutable)

Everything outside the engine contract is authored and owned by the game:

- ECS components
- Flow graphs (see [15_FLOW_GRAPH.md](15_FLOW_GRAPH.md))
- AI planners (see [08_AI_EDITOR_ASSIST.md](08_AI_EDITOR_ASSIST.md))
- Procedural modeling graphs (see [17_PROCEDURAL_MODELING.md](17_PROCEDURAL_MODELING.md))
- UI layout + DSL (see [18_GAME_GUI_AUTHORING.md](18_GAME_GUI_AUTHORING.md))
- Tutorials
- Assets
- Networking rules

## Template Contract (ATLAS_TEMPLATE_CONTRACT.md)

Every fork must preserve the following invariants regardless of how the
game restructures the repository:

1. **Deterministic execution semantics** — identical inputs produce
   identical outputs on all supported platforms
2. **Replay binary format compatibility** — replay files conform to the
   binary format defined in [14_REPLAY_AND_PROOFS.md](14_REPLAY_AND_PROOFS.md)
3. **Hash ladder construction rules** — chained BLAKE3 commitments follow
   the canonical `H[n] = Hash(H[n-1] || State[n] || Inputs[n])` scheme
4. **CI validation gates** — all merge requests pass determinism and
   replay verification before landing
5. **Formal spec interface compliance** — TLA+ interface contracts remain
   satisfiable after game-specific extensions

## Determinism Versioning Across Forks

Each fork tracks its `determinism.version` independently:

- Core determinism semantics are inherited from Atlas at fork time
- Game-specific determinism rules extend (never override) the core
- Version bumps are explicit and require migration
- A version change invalidates all prior replay files unless a migration
  path is provided

## Sharing Proof Techniques

Formal verification patterns are shared across forks without sharing game
code:

- A separate repository hosts TLA+ proof patterns
- Game forks import patterns as read-only references
- Common formal properties include ECS ordering, replay fidelity, and
  layout invariants
- Each game extends proofs for game-specific systems (AI, world gen,
  networking)

## AtlasAI in Template Context

After fork, the AI assistant (see [16_ATLAS_AI.md](16_ATLAS_AI.md))
becomes project-local:

| Behavior | Description |
|----------|-------------|
| Context scope | AI context is scoped to the game's project files |
| ECS awareness | Knows the game's components, graphs, and assets |
| Code generation | Generates code in the game's namespace |
| External naming | Never references "Atlas" in generated output |
| Guardrails | Permissions, diffs, and determinism lint still apply |

## Editor Customization

After fork, the editor (see [02_EDITOR_DESIGN.md](02_EDITOR_DESIGN.md))
becomes game-specific:

- Welcome screen describes the game, not Atlas
- Tutorials explain game mechanics rather than engine concepts
- Graph libraries reflect the game's DSL and domain nodes
- Unused engine features are hidden from the UI
- Tool panels are customized for the game's authoring workflow

## One-Sentence Definition

> Atlas is a deterministic game-design template repository that
> restructures itself entirely around a single project while preserving
> formal guarantees, replay fidelity, and toolchain correctness.
