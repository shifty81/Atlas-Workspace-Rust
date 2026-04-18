# Atlas Engine — Overview

Atlas is a modular, data-driven game engine and simulation platform built in C++20.
All gameplay logic is authored as data and executable graphs — the engine itself
contains no hardcoded gameplay behavior.

## Key Capabilities

- **Standalone Editor** — Blender-style authoring environment with dockable panels
- **Runtime Client & Server** — Lean player runtime and headless authoritative server
- **Graph-based Gameplay VM** — Deterministic bytecode VM with hot-reloadable graphs
- **14+ Graph Systems** — Domain-specific graphs for world gen, AI, animation, audio, UI, and more
- **Procedural Generation** — Planet-scale terrain, galaxies, 2D tiles, characters, weapons
- **AI Systems** — Behavior graphs, NPC memory with decay, faction relationships, strategy decisions
- **Networking** — Client-server and P2P with lockstep/rollback foundations (partially implemented)
- **Project System** — Multi-project support with schema-validated `.atlas` project files
- **Hot-Reloadable Assets** — Zero-rebuild iteration via file timestamp polling

## Supported Game Types

Atlas is content-agnostic. All gameplay is implemented as data + graphs, not hardcoded logic. Supported game styles include:

- Space strategy (top-down / isometric overview)
- First-person interiors
- Planetary exploration
- Voxel worlds
- Top-down / adventure-style cameras
- Side-scrollers
- Large-scale simulations

## Core Philosophy

1. **Engine Is Not a Game** — The engine contains no gameplay logic
2. **Everything Is Data** — All behavior is authored as assets and executable graphs
3. **One Engine, Four Roles** — Editor, Runtime, Client, and Server share the same engine core
4. **Editor Is Primary** — The editor is not a separate tool; it is a first-class engine runtime
5. **Determinism First** — All simulation is deterministic for networking and replay
6. **Graphs Everywhere** — All domain logic (world gen, AI, animation, audio) is expressed as DAG-based graphs

## Current Status

The engine core, graph VM, world generation, AI systems, and gameplay systems are fully
implemented. The editor framework is functional with several panels, though some are stubs.
Networking has a working API but lockstep/rollback need completion. Production tooling
is in early stages.

See [09_DEVELOPMENT_ROADMAP.md](09_DEVELOPMENT_ROADMAP.md) for detailed status.
