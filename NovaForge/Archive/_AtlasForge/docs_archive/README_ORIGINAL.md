```
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║       █████╗ ████████╗██╗      █████╗ ███████╗                   ║
║      ██╔══██╗╚══██╔══╝██║     ██╔══██╗██╔════╝                   ║
║      ███████║   ██║   ██║     ███████║███████╗                   ║
║      ██╔══██║   ██║   ██║     ██╔══██║╚════██║                   ║
║      ██║  ██║   ██║   ███████╗██║  ██║███████║                   ║
║      ╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚══════╝                   ║
║                                                                  ║
║          ⚙️  Modular · Data-Driven · Deterministic  ⚙️           ║
║                   G A M E   E N G I N E                          ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝
```

# Atlas Game Engine

Atlas is a modular, data-driven game engine and simulation platform built in C++20. All gameplay is authored as data and executable graphs — the engine contains no hardcoded gameplay logic.

### Key Features

- **Deterministic Simulation** — Bit-exact reproducible ticks with hash-ladder verification and CI determinism gate
- **Standalone Editor** — Blender-style authoring environment with 14+ dockable panels and AI assistant framework
- **Runtime Client & Server** — Lean player runtime and headless authoritative server sharing one engine core
- **Graph VM** — Deterministic bytecode virtual machine with compile/execute, hot-reload, and serialization
- **Graph-Based Systems** — 14 domain-specific graph types for world gen, animation, AI, audio, UI, and more
- **Procedural Generation** — Planet-scale terrain, galaxies, tiles, weapons, characters, and narrative
- **AI Systems** — Behavior graphs, memory with decay, faction relationships, strategy decisions
- **Networking** — Client-server and P2P with lockstep/rollback, replication rules, and replay
- **Project & Plugin System** — Multi-project support with schema-validated `.atlas` files, mod loader, and plugin registry
- **Replay & Verification** — Full replay recording, divergence detection, and TLA+ formal specifications

---

## 🗺️ Development Roadmap

> See [docs/09_DEVELOPMENT_ROADMAP.md](docs/09_DEVELOPMENT_ROADMAP.md) for full details.

```
  Phase 1       Phase 2       Phase 3       Phase 4       Phase 5      Phase 5b      Phase 5c
 Core Engine     Editor      Networking    World Gen     Gameplay     Graph Sys     AI Systems
──────────────────────────────────────────────────────────────────────────────────────────────────
 ██████████    ██████████    ██████████    ██████████    ██████████    ██████████    ██████████
   100%          100%          100%          100%          100%          100%          100%
  ✅ Done       ✅ Done       ✅ Done       ✅ Done       ✅ Done       ✅ Done       ✅ Done

  Phase 5d      Phase 5e      Phase 6       Phase 7       Phase 8       Phase 9       Phase 10
 Interaction   Proj/Plugin   Production     Polish       GUI Harden   Replay/Verify  Flow Graph
──────────────────────────────────────────────────────────────────────────────────────────────────
 ██████████    ██████████    ██████████    ██████████    ██████████    ██████████    ██████████
   100%          100%          100%          100%          100%          100%          100%
  ✅ Done       ✅ Done       ✅ Done       ✅ Done       ✅ Done       ✅ Done       ✅ Done

  Phase 11      Phase 12
 AtlasAI/GUI   CI & Build
──────────────────────────
 ██████████    ██████████
    100%          100%
  ✅ Done       ✅ Done
```

<table>
<tr>
<td width="25%">

**Phase 1 — Core Engine** ✅ 100%

- ✅ Bootstrap & config
- ✅ ECS framework
- ✅ Graph VM & compiler
- ✅ Asset registry
- ✅ Binary asset format
- ✅ Hot reload

</td>
<td width="25%">

**Phase 2 — Editor** ✅ 100%

- ✅ UI framework
- ✅ Panel docking system
- ✅ 14+ panels with logic
- ✅ Console, inspector, graph editor
- ✅ Layout persistence (save/restore)
- ✅ Play-In-Editor (simulate, possess, loopback)
- ✅ Rendering layer (UIDrawList + bitmap font)
- ✅ UI backend integration (GL + viewport FB)

</td>
<td width="25%">

**Phase 3 — Networking** ✅ 100%

- ✅ NetContext API
- ✅ Dedicated server loop
- ✅ P2P support
- ✅ Lockstep sync
- ✅ Rollback/replay
- ✅ Production hardening (timeouts, reconnect, bandwidth, heartbeat)
- ✅ Packet loss simulation & QoS scheduler
- ✅ CRC32 checksums, reliable/unreliable delta

</td>
<td width="25%">

**Phase 4 — World Gen** ✅ 100%

- ✅ WorldLayout interface
- ✅ Cube-sphere math
- ✅ Voxel grid layout
- ✅ Terrain mesh gen
- ✅ Noise nodes
- ✅ World streaming
- ✅ Galaxy generation

</td>
</tr>
<tr>
<td width="25%">

**Phase 5 — Gameplay** ✅ 100%

- ✅ Mechanic assets
- ✅ Camera system
- ✅ Input mapping
- ✅ Physics integration
- ✅ Audio system

</td>
<td width="25%">

**Phase 5b — Graph Systems** ✅ 100%

- ✅ All 14 graph types
- ✅ Compile & execute paths
- ✅ Serialization
- ✅ Graph VM integration

</td>
<td width="25%">

**Phase 5c — AI Systems** ✅ 100%

- ✅ Behavior graphs
- ✅ Memory with decay
- ✅ Faction relationships
- ✅ Strategy decisions
- ✅ Conversation graphs

</td>
<td width="25%">

**Phase 5d — Interaction** ✅ 100%

- ✅ Intent/utterance system
- ✅ Voice command registry
- ✅ Console command parsing
- ✅ Interaction debugger

</td>
</tr>
<tr>
<td width="25%">

**Phase 5e — Project/Plugin** ✅ 100%

- ✅ Project loading & validation
- ✅ Schema validation system
- ✅ Plugin registry
- ✅ Mod asset loader

</td>
<td width="25%">

**Phase 6 — Production** ✅ 100%

- ✅ Game packager (full pipeline: Validate → Cook → Bundle → Emit)
- ✅ Asset cooker (source → binary cooking)
- ✅ Build profiles
- ✅ Mod loader
- ✅ Platform targeting
- ✅ Certified build pipeline
- ✅ Build audit log & manifest

</td>
<td width="25%">

**Phase 7 — Polish** ✅ 100%

- ✅ Undo/redo system
- ✅ Visual diff tools
- ✅ Profiler panel
- ✅ Replay recorder
- ✅ Crash analysis

</td>
<td width="25%">

**Phase 8 — GUI & Editor Hardening** ✅ 100%

- ✅ GUI DSL & layout solver
- ✅ Panel framework
- ✅ Layout persistence
- ✅ Play-In-Editor
- ✅ Editor self-hosting (DSL-defined layout)
- ✅ Unreal-grade dark theme & typography

</td>
</tr>
<tr>
<td width="25%">

**Phase 9 — Replay & Verification** ✅ 100%

- ✅ Hash-ladder replay
- ✅ Divergence detection
- ✅ TLA+ formal specs
- ✅ Replay recorder (save-points, v3 format)

</td>
<td width="25%">

**Phase 10 — Flow Graph & Procedural** ✅ 100%

- ✅ Blueprint-like flow graph
- ✅ IR & debugger
- ✅ Procedural modeling
- ✅ Mesh/material graphs

</td>
<td width="25%">

**Phase 11 — AtlasAI & Game GUI** ✅ 100%

- ✅ AI assistant framework
- ✅ Web aggregation design
- ✅ Template AI backend (offline)
- ✅ LLM backend integration (HttpLLMBackend)
- ✅ Game GUI widget DSL & bindings

</td>
<td width="25%">

**Phase 12 — CI & Build** ✅ 100%

- ✅ CI determinism gate
- ✅ Build system & scripts
- ✅ Certified build pipeline
- ✅ First-run experience (`atlas_init.py`, `run.sh`)

</td>
</tr>
</table>

| Status | Meaning |
|--------|---------|
| ✅ Complete | Fully implemented and tested |
| 🔧 Functional | Core logic works, some features need polish |
| 📋 Scaffolded | Headers/interfaces exist, implementation in progress |

---

## 🖥️ Editor Status

| Area | Status |
|------|--------|
| **Panel count** | 14+ panels with functional logic (Console, ECS Inspector, Graph Editor, World Graph, Net Inspector, Profiler, etc.) |
| **Rendering layer** | ✅ UIDrawList rendering with bitmap font; `Draw()` implemented for all panels |
| **Docking infrastructure** | ✅ Layout serialization, split/tab docking framework exists |
| **AI assistant** | ✅ Framework present (explain, suggest, generate) with HttpLLMBackend wired |
| **GUI DSL** | ✅ Custom DSL and layout solver implemented; editor is self-hosted via DSL |

## ✅ What Works Today

These systems are implemented, tested, and functional:

- **Deterministic simulation engine** — bit-exact tick execution with hash-ladder verification
- **14 graph system types** — all have compile, execute, and serialization paths
- **Full save/load** — binary asset format with hash integrity checking
- **Replay recording** — record/playback with divergence detection
- **ECS framework** — entity management with serialization and rollback support
- **Networking** — lockstep and rollback foundations with client-server and P2P
- **1971 unit tests pass** — comprehensive coverage across all engine systems
- **CI determinism gate** — automated verification that simulation is bit-exact
- **Play-In-Editor** — simulate, pause, step, possess entity, loopback, state restore
- **Editor layout persistence** — save/restore panel arrangements to/from JSON
- **Network hardening** — connection timeouts, reconnection, bandwidth throttling, heartbeat monitoring
- **Game packager pipeline** — full Validate → Cook → Bundle → Emit workflow
- **Undo/redo** — command-pattern history across editor operations
- **Project system** — schema-validated `.atlas` project files with multi-project support
- **World generation** — cube-sphere, voxel, terrain, galaxy, noise, and streaming
- **AI systems** — behavior graphs, faction memory, strategy decisions, conversation graphs

---

## Architecture

Atlas ships as four executables, all linked against the same engine core:

| Binary          | Purpose                                       |
|-----------------|-----------------------------------------------|
| `AtlasEditor`   | Full editor + runtime (superset)              |
| `AtlasRuntime`  | Standalone runtime with CLI project loading   |
| `AtlasClient`   | Player runtime                                |
| `AtlasServer`   | Headless authoritative server                 |

## Game Modules

EveOffline ships permanently inside this repository as a first-party game example. Both the engine and EveOffline are actively developed together, and EveOffline will serve as the shipped reference game once the engine is complete. For the module architecture and interface details, see [docs/10_REPO_SPLIT_PLAN.md](docs/10_REPO_SPLIT_PLAN.md).

## Repository Structure

```
Atlas/
├── engine/              # Core engine static library
│   ├── core/            # Engine bootstrap, logging, config
│   │   └── contract/    # Determinism contract enforcement headers
│   ├── ecs/             # Entity-Component-System framework
│   ├── graphvm/         # Deterministic Graph VM + compiler + serialization + cache
│   ├── assets/          # Asset registry, binary format, hot reload
│   ├── net/             # Networking (client-server + P2P)
│   ├── sim/             # Tick scheduler, deterministic simulation, replay recorder
│   ├── world/           # World generation (cube-sphere, voxel, terrain, galaxy, streaming)
│   ├── input/           # Input mapping system
│   ├── camera/          # Camera system with world mode policies
│   ├── physics/         # Physics simulation (rigid bodies, AABB collision)
│   ├── audio/           # Audio engine
│   ├── gameplay/        # Mechanic assets & skill trees
│   ├── ai/              # AI signals, memory, relationships, behavior graphs
│   ├── interaction/     # Intent/utterance system (voice, AI, console)
│   ├── voice/           # Voice command registry and matching
│   ├── conversation/    # Dialogue and memory graphs
│   ├── strategygraph/   # Strategy decision graphs (influence, threat, scoring)
│   ├── animation/       # Animation graph + modifier system
│   ├── character/       # Character generation graph
│   ├── weapon/          # Weapon construction graph
│   ├── tile/            # 2D tileset generation graph
│   ├── sound/           # Procedural audio/synth graph
│   ├── story/           # Story/narrative graph
│   ├── flow/            # Game flow graph (boot → credits)
│   ├── ui/              # UI composition graph
│   ├── asset_graph/     # Asset graph executor
│   ├── command/         # Undo/redo command history
│   ├── project/         # Project loading and validation
│   ├── schema/          # Schema validation system
│   ├── plugin/          # Plugin system (validation, registry)
│   ├── mod/             # Mod asset registry and mod loader
│   ├── module/          # Game module interface and dynamic loader
│   ├── production/      # Asset cooker, game packager, build profiles, platform targeting
│   └── rules/           # Server rules (live parameter tuning)
│
├── modules/
│   └── atlas_gameplay/  # AtlasGameplay static lib (factions, combat, economy)
│
├── editor/              # Standalone editor application
│   ├── ui/              # Docking, layout, panel framework
│   ├── panels/          # Console, ECS Inspector, Net Inspector, World Graph,
│   │                    # Project Picker, Voice Commands, Interaction Debugger, Profiler
│   ├── tools/           # Game packager panel, visual diff tools
│   ├── ai/              # AI aggregator for asset generation
│   └── assistant/       # Editor assistant (explain, suggest)
│
├── runtime/             # Standalone runtime (CLI project loading)
├── client/              # Player runtime client
├── server/              # Headless dedicated server
│
├── tests/               # Unit tests (90 test files)
│
├── tools/               # Development tools and scripts
│   └── contract_scan.py # Determinism contract violation scanner
│
├── schemas/             # Versioned JSON schemas
│   ├── atlas.project.v1.json
│   ├── atlas.worldgraph.v1.json
│   ├── atlas.strategygraph.v1.json
│   └── atlas.conversation.v1.json
│
├── projects/            # Sample game projects
│   ├── eveoffline/      # Space strategy reference project
│   │   └── module/      # EveOfflineModule (IGameModule implementation)
│   ├── arena2d/         # 2D arena reference project
│   └── atlas-sample/    # Minimal sample project
│
├── docs/                # Documentation
├── logs/                # Build and runtime logs (gitignored contents)
└── CMakeLists.txt       # Root build configuration
```

## Building

### Prerequisites

- CMake 3.22+
- C++20 compatible compiler (GCC 13+, Clang 15+, MSVC 2022+)

### Build

The easiest way to build is with the included build script:

```bash
# Build all targets (server, client, editor, runtime) in Release mode
./build.sh

# Build specific targets
./build.sh server client          # Server and client only
./build.sh editor                 # Developer client (editor) only
./build.sh engine                 # Engine and gameplay libraries only

# Build options
./build.sh -b Debug all           # Debug build
./build.sh -b Development editor  # Development build (optimized + debug symbols)
./build.sh --clean --test all     # Clean rebuild with tests
./build.sh -o ./my-output server  # Custom output directory
./build.sh --install              # Install SDK (headers + libs + cmake configs) to dist/sdk/

# See all options
./build.sh --help
```

Executables are placed in `dist/` by default. SDK artifacts (for external game modules) are installed to `dist/sdk/` with the `--install` flag.

You can also build manually with CMake:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Run

```bash
# Editor / developer client (authoring + testing)
./dist/AtlasEditor

# Runtime (standalone with project loading)
./dist/AtlasRuntime --project projects/atlas-sample/sample.atlas

# Client (player runtime)
./dist/AtlasClient

# Server (headless)
./dist/AtlasServer
```

### Tests

```bash
# Via build script
./build.sh --test

# Or manually
cd build
ctest
# or
./tests/AtlasTests
```

### AI Assistant Configuration (Optional)

Atlas includes an AI-powered editor assistant that can help with content generation. By default, it runs in offline mode with no network access. To enable the HTTP LLM backend:

```bash
# Configure environment variables
export ATLAS_LLM_ENDPOINT="https://api.openai.com/v1/chat/completions"
export ATLAS_LLM_MODEL="gpt-4"
export ATLAS_LLM_API_KEY="your-api-key-here"

# Then run the editor
./dist/AtlasEditor
```

Supported backends:
- OpenAI (GPT-3.5, GPT-4)
- Azure OpenAI
- Local LLMs (llama.cpp, Ollama)
- Any OpenAI-compatible API

For detailed configuration options, see [docs/16_ATLAS_AI.md](docs/16_ATLAS_AI.md).

---

## Core Design Principles

1. **Engine Is Not a Game** — The engine contains no gameplay logic; all behavior is data
2. **Everything Is Data** — All behavior is authored as assets and executable graphs
3. **One Engine, Four Roles** — Editor, Runtime, Client, and Server share the same core
4. **Editor Is Primary** — The editor is a first-class engine runtime, not a separate tool
5. **Determinism First** — All simulation is deterministic for networking and replay
6. **Graphs Everywhere** — 14+ domain-specific graph types power world gen, AI, animation, audio, and more

## Graph Systems

Atlas uses a DAG-based graph execution model across many domains:

| Graph Type | Purpose |
|------------|---------|
| WorldGraph | Procedural terrain and world generation |
| StrategyGraph | AI strategy decisions (influence, threat) |
| ConversationGraph | Dialogue and NPC memory |
| BehaviorGraph | Authorable AI behavior trees |
| AnimationGraph | Animation state machines with modifiers |
| CharacterGraph | Modular character generation |
| WeaponGraph | Weapon construction and wear |
| TileGraph | 2D procedural tile maps |
| SoundGraph | Procedural audio synthesis |
| UIGraph | UI composition (panels, buttons, layouts) |
| UIScreenGraph | Screen-level UI management |
| GameFlowGraph | Game state flow (boot → credits) |
| StoryGraph | Narrative generation and branching |
| AssetGraph | Asset pipeline execution |

## Documentation

See the [docs/](docs/) directory for detailed documentation:

| Document | Description |
|----------|-------------|
| [Overview](docs/00_OVERVIEW.md) | High-level engine capabilities and philosophy |
| [Engine Architecture](docs/01_ENGINE_ARCHITECTURE.md) | Module structure, boot flow, runtime modes |
| [Editor Design](docs/02_EDITOR_DESIGN.md) | Editor panels, docking, play-in-editor modes |
| [Asset System](docs/03_ASSET_SYSTEM.md) | Asset types, binary format, hot reload |
| [Graph VM](docs/04_GRAPH_VM.md) | Bytecode VM, instruction set, compilation |
| [Networking](docs/05_NETWORKING.md) | NetContext, replication, lockstep/rollback |
| [World Generation](docs/06_WORLD_GENERATION.md) | Terrain, planets, galaxies, streaming |
| [Gameplay Mechanics](docs/07_GAMEPLAY_MECHANICS.md) | Mechanic assets, skill trees, cameras |
| [AI Editor Assist](docs/08_AI_EDITOR_ASSIST.md) | AI-assisted content generation |
| [Development Roadmap](docs/09_DEVELOPMENT_ROADMAP.md) | Phase-by-phase development status |
| [GUI System](docs/12_GUI_SYSTEM.md) | Custom GUI architecture, DSL, layout solver |
| [Editor UI](docs/13_EDITOR_UI.md) | Unreal-grade editor aesthetics, panels, self-hosting |
| [Replay & Proofs](docs/14_REPLAY_AND_PROOFS.md) | Replay system, hash ladder, TLA+ verification |
| [Flow Graph](docs/15_FLOW_GRAPH.md) | Blueprint-like visual scripting, IR, debugger |
| [AtlasAI](docs/16_ATLAS_AI.md) | AI assistant, web aggregation, context-aware prompts |
| [Procedural Modeling](docs/17_PROCEDURAL_MODELING.md) | Blender-like modeling, mesh/material graphs |
| [Game GUI Authoring](docs/18_GAME_GUI_AUTHORING.md) | Game UI as authored data, widget DSL |
| [Template Repository](docs/19_TEMPLATE_REPO.md) | Forkable template system, atlas init |
| [CI & Build](docs/20_CI_AND_BUILD.md) | CI policies, build system, first-run experience |
| [Formal Specifications](docs/21_FORMAL_SPECIFICATIONS.md) | TLA+ specs for ECS, replay, layout |
| [Core Contract](docs/ATLAS_CORE_CONTRACT.md) | Non-negotiable engine invariants |
| [Determinism Enforcement](docs/ATLAS_DETERMINISM_ENFORCEMENT.md) | Compile-time, runtime, CI enforcement |
| [Lockdown Checklist](docs/ATLAS_LOCKDOWN_CHECKLIST.md) | Engine feature-freeze gate |
| [Editor Status](docs/EDITOR_STATUS.md) | Editor completion status and gap analysis |
| [Building](docs/BUILDING.md) | Build prerequisites, script usage, logs, troubleshooting |
| [Architecture Reference](docs/ARCHITECTURE.md) | Detailed module-by-module reference |
| [Naming Conventions](docs/ATLAS_NAMING_CONVENTIONS.md) | Code style and naming rules |
| [Simulation Philosophy](docs/ATLAS_SIMULATION_PHILOSOPHY.md) | Determinism and simulation design |
| [Next Implementation Tasks](docs/NEXT_IMPLEMENTATION_TASKS.md) | Remaining tasks and priority order |

## Development Tools

Atlas includes several powerful development and debugging tools:

| Tool | Purpose |
|------|---------|
| **Replay Minimizer** (`tools/replay_minimizer.py`) | Automatically reduces failing replays to minimal reproduction cases using binary search. Essential for debugging determinism issues. |
| **Contract Scanner** (`tools/contract_scan.py`) | Scans simulation code for forbidden APIs that violate the Atlas Core Contract (wall-clock time, non-deterministic RNG, etc.). |
| **CMake Contract Enforcement** (`cmake/AtlasContractEnforcement.cmake`) | Enforces architectural layer dependencies at build time. Prevents core/ from depending on higher layers, ensures simulation isolation. |

Example usage:
```bash
# Minimize a failing replay
python tools/replay_minimizer.py \
    --replay tests/replays/failing.atlasreplay \
    --test-command "./build/tests/AtlasTests --replay {replay}" \
    --output tests/replays/minimal.atlasreplay

# Scan for contract violations
python tools/contract_scan.py --path engine
```

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.
