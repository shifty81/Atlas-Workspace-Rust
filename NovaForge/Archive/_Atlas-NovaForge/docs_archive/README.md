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
║      ███╗   ██╗ ██████╗ ██╗   ██╗ █████╗                         ║
║      ████╗  ██║██╔═══██╗██║   ██║██╔══██╗                        ║
║      ██╔██╗ ██║██║   ██║██║   ██║███████║                        ║
║      ██║╚██╗██║██║   ██║╚██╗ ██╔╝██╔══██║                        ║
║      ██║ ╚████║╚██████╔╝ ╚████╔╝ ██║  ██║                        ║
║      ╚═╝  ╚═══╝ ╚═════╝   ╚═══╝  ╚═╝  ╚═╝                        ║
║                                                                  ║
║      ███████╗ ██████╗ ██████╗  ██████╗ ███████╗                  ║
║      ██╔════╝██╔═══██╗██╔══██╗██╔════╝ ██╔════╝                  ║
║      █████╗  ██║   ██║██████╔╝██║  ███╗█████╗                    ║
║      ██╔══╝  ██║   ██║██╔══██╗██║   ██║██╔══╝                    ║
║      ██║     ╚██████╔╝██║  ██║╚██████╔╝███████╗                  ║
║      ╚═╝      ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚══════╝                  ║
║                                                                  ║
║          ⚙️  Modular · Data-Driven · Deterministic  ⚙️           ║
║                   G A M E   E N G I N E                          ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝
```

# Atlas NovaForge

Atlas NovaForge is a modular, data-driven game engine and PvE space simulator built in C++20. The **Atlas Engine** provides deterministic simulation, 14+ graph systems, networking, and a custom GUI framework. **Nova Forge** is the game built on top — a tri-modal (FPS + Flight + Fleet Command) space sim with procedural worlds, legend-driven AI, and an in-game editor.

> **Project Status: Active Development — Refactoring & Integration Phase**

---

### Key Features

- **Deterministic Simulation** — Bit-exact reproducible ticks with hash-ladder verification
- **Editor → In-Game Tooling** — Converting standalone editor to toggleable in-game ToolingLayer (see [Editor Conversion Plan](docs/EDITOR_CONVERSION_PLAN.md))
- **Tri-Modal Gameplay** — FPS boarding, ship flight, and RTS fleet command with hard mode boundaries
- **Graph VM** — Deterministic bytecode virtual machine with compile/execute, hot-reload, and serialization
- **14 Graph Systems** — World gen, animation, AI, audio, UI, story, and more
- **Procedural Generation** — Planets, galaxies, ship hulls, FPS interiors, low-poly style enforcement
- **AI Systems** — Behavior graphs, memory with decay, faction relationships, strategy decisions
- **Networking** — Client-server and P2P with lockstep/rollback, replication rules, and replay
- **Legend System** — Player behavior shapes future worlds through reputation and world bias (planned)
- **Project & Plugin System** — Schema-validated `.atlas` files, mod loader, plugin registry

---

## 🗺️ Project Status

> See [docs/09_DEVELOPMENT_ROADMAP.md](docs/09_DEVELOPMENT_ROADMAP.md) for engine roadmap.
> See [docs/ROADMAP.md](docs/ROADMAP.md) for game roadmap.
> See [docs/GAME_DESIGN_VISION.md](docs/GAME_DESIGN_VISION.md) for the full game design vision.

### Engine Systems (Atlas Engine)

```
  Phase 1       Phase 2       Phase 3       Phase 4       Phase 5      Phase 5b
 Core Engine     Editor      Networking    World Gen     Gameplay     Graph Sys
──────────────────────────────────────────────────────────────────────────────────
 ██████████    ██████████    ██████████    ██████████    ██████████    ██████████
   100%          100%          100%          100%          100%          100%
  ✅ Done       ✅ Done       ✅ Done       ✅ Done       ✅ Done       ✅ Done
```

### Game Systems (Nova Forge)

```
  Server         Client       Vertical      Editor       FPS/Flight    Legend
  Systems       Integration    Slice       Conversion     /Fleet       System
──────────────────────────────────────────────────────────────────────────────────
 ██████████    ████░░░░░░    ██░░░░░░░░    ░░░░░░░░░░    ░░░░░░░░░░    ░░░░░░░░░░
   100%          40%           20%            0%            0%            0%
  ✅ Done     🔧 Active     🔧 Active     📋 Planned    📋 Planned    📋 Planned
```

### Active Work

```
  Spaghetti     Code          Build
  Cleanup      Refactor      Integration
──────────────────────────────────────────
 ████░░░░░░    ██░░░░░░░░    ██░░░░░░░░
    40%           20%           20%
 🔧 Active    🔧 Active     🔧 Active
```

| Status | Meaning |
|--------|---------|
| ✅ Done | Fully implemented and tested |
| 🔧 Active | Core logic works, actively being developed |
| 📋 Planned | Designed, not yet implemented |

---

### What Works Today

These systems are implemented, tested, and functional:

**Atlas Engine (418 C++ source files)**
- ✅ Deterministic simulation engine with hash-ladder verification
- ✅ 14 graph system types with compile, execute, and serialization paths
- ✅ ECS framework with serialization and rollback support
- ✅ Binary asset format with hash integrity checking
- ✅ Replay recording with divergence detection
- ✅ Networking with lockstep/rollback, client-server and P2P
- ✅ Custom GUI DSL and layout solver
- ✅ Editor with 14+ dockable panels and AI assistant framework
- ✅ Game packager pipeline (Validate → Cook → Bundle → Emit)
- ✅ Project system with schema-validated `.atlas` files
- ✅ World generation (cube-sphere, voxel, terrain, galaxy, noise, streaming)
- ✅ TLA+ formal specifications and CI determinism gate

**Nova Forge Server (537 C++ files, 164 game systems)**
- ✅ 27 baseline server systems with 5,263+ test assertions passing
- ✅ 102 ships, 159+ modules, 137 skills — all data-driven
- ✅ Combat, fleet, economy, mining, missions, exploration systems
- ✅ Procedural ship generation with faction shape language
- ✅ AI NPCs (miners, haulers, pirates, traders, security)

**Nova Forge Client (156 C++ files)**
- ✅ OpenGL deferred rendering pipeline
- ✅ Atlas UI widget system with EVE-style dark theme
- ✅ Ship HUD (shield/armor/hull arcs, capacitor, module rack)
- 🔧 Client-server integration (in progress)

### What's In Progress

- **Spaghetti Code Cleanup** — Splitting monolithic test file (31K lines), reducing system boilerplate, decomposing GameSession god object (see [docs/SPAGHETTI_CODE_AUDIT.md](docs/SPAGHETTI_CODE_AUDIT.md))
- **Code Refactoring** — JSON helper consolidation, file splits, naming consistency (see [docs/REFACTORING_PLAN.md](docs/REFACTORING_PLAN.md))
- **Vertical Slice** — One full star system: fly, dock, mine, trade, fight, warp — end-to-end playable
- **Client Integration** — Wiring all server systems to C++ client rendering and UI

### What's Planned

- **Editor Conversion** — Convert standalone AtlasEditor to in-game ToolingLayer with EditorCommandBus (see [docs/EDITOR_CONVERSION_PLAN.md](docs/EDITOR_CONVERSION_PLAN.md))
- **FPS + Flight + Fleet Command** — Tri-modal gameplay with PlayerModeController and hard mode boundaries (see [docs/GAME_DESIGN_VISION.md](docs/GAME_DESIGN_VISION.md))
- **Legend System** — Player reputation driving world bias across runs
- **Procedural FPS Interiors** — Low-poly style with grammar-based room generation

---

## Architecture

The project has two major layers:

### Atlas Engine (Core)

Shared engine library powering all executables:

| Binary          | Purpose                                       |
|-----------------|-----------------------------------------------|
| `AtlasEditor`   | Full editor + runtime (superset)              |
| `AtlasRuntime`  | Standalone runtime with CLI project loading   |
| `AtlasClient`   | Player runtime                                |
| `AtlasServer`   | Headless authoritative server                 |

### Nova Forge (Game)

PvE space simulator built on Atlas Engine:

| Binary              | Purpose                                    |
|---------------------|--------------------------------------------|
| `nova_forge_server` | Authoritative game server (tick-based sim) |
| `nova_forge_client` | Player client (rendering, UI, audio)       |

### Layer Responsibilities

| Layer | Responsibility |
|-------|---------------|
| **Engine Core** | ECS, Graph VM, determinism, assets, networking foundations |
| **Server (Authoritative)** | Tick-based simulation, AI, economy, combat, persistence |
| **Client (Display)** | Rendering, interpolation, UI, input, audio — no game logic authority |
| **ToolingLayer (Planned)** | In-game editor overlay, command bus, live editing |

---

## Repository Structure

```
Atlas-NovaForge/
├── engine/              # Atlas Engine — core static library (418 files)
│   ├── core/            # Engine bootstrap, logging, config, determinism contract
│   ├── ecs/             # Entity-Component-System framework
│   ├── graphvm/         # Deterministic Graph VM + compiler + serialization
│   ├── assets/          # Asset registry, binary format, hot reload
│   ├── net/             # Networking (client-server + P2P)
│   ├── sim/             # Tick scheduler, simulation, replay recorder
│   ├── world/           # World generation (cube-sphere, voxel, terrain, galaxy)
│   ├── ai/              # AI signals, memory, relationships, behavior graphs
│   ├── ui/              # UI composition graph
│   └── ...              # 30+ subsystem directories
│
├── editor/              # Standalone editor application (121 files)
│   ├── ui/              # Docking, layout, panel framework
│   ├── panels/          # 14+ panels (Console, ECS Inspector, Graph Editor, etc.)
│   ├── tools/           # Game packager panel, visual diff tools
│   └── assistant/       # Editor AI assistant
│
├── cpp_server/          # Nova Forge dedicated server (537 files)
│   ├── src/systems/     # 164 game systems
│   ├── src/data/        # Game data loading (ships, modules, skills)
│   └── include/         # Server headers
│
├── cpp_client/          # Nova Forge game client (156 files)
│   ├── src/             # Rendering, UI, networking, audio
│   └── include/         # Client headers
│
├── modules/             # Game module libraries
│   └── atlas_gameplay/  # Factions, combat, economy
│
├── tests/               # Engine unit tests (203 test files)
├── atlas_tests/         # Additional test suite (51 test files)
│
├── data/                # Game data (JSON ships, modules, skills, missions)
├── schemas/             # Versioned JSON schemas
├── projects/            # Sample game projects
├── docs/                # Documentation (90+ files)
├── tools/               # Development tools and scripts
├── archive/             # Historical planning notes and reference material
└── CMakeLists.txt       # Root build configuration
```

---

## Building

### Prerequisites

- CMake 3.22+
- C++20 compatible compiler (GCC 13+, Clang 15+, MSVC 2022+)

### Build

```bash
# Build all targets in Release mode
./build.sh

# Build specific targets
./build.sh server client          # Server and client only
./build.sh editor                 # Developer client (editor) only
./build.sh engine                 # Engine and gameplay libraries only

# Build options
./build.sh -b Debug all           # Debug build
./build.sh --clean --test all     # Clean rebuild with tests
./build.sh --install              # Install SDK to dist/sdk/

# See all options
./build.sh --help
```

Executables are placed in `dist/` by default.

### Run

```bash
./dist/AtlasEditor                                              # Editor
./dist/AtlasRuntime --project projects/atlas-sample/sample.atlas # Runtime
./dist/AtlasClient                                              # Client
./dist/AtlasServer                                              # Server
```

### Tests

```bash
./build.sh --test        # Via build script
cd build && ctest        # Or manually
```

---

## Core Design Principles

1. **Engine Is Not a Game** — The engine contains no gameplay logic; all behavior is data
2. **Everything Is Data** — All behavior is authored as assets and executable graphs
3. **One Engine, Four Roles** — Editor, Runtime, Client, and Server share the same core
4. **Determinism First** — All simulation is deterministic for networking and replay
5. **Editor Becomes Tooling** — Editor converts to in-game ToolingLayer, not a separate application
6. **Graphs Everywhere** — 14+ domain-specific graph types power world gen, AI, animation, and more

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

---

## Documentation

### Key Documents

| Document | Description |
|----------|-------------|
| [Game Design Vision](docs/GAME_DESIGN_VISION.md) | **NEW** — FPS/Flight/Fleet Command design, legend system, visual style |
| [Editor Conversion Plan](docs/EDITOR_CONVERSION_PLAN.md) | **NEW** — Converting standalone editor to in-game ToolingLayer |
| [Spaghetti Code Audit](docs/SPAGHETTI_CODE_AUDIT.md) | Server code cleanup plan (5 critical issues) |
| [Refactoring Plan](docs/REFACTORING_PLAN.md) | Codebase-wide refactoring (8 phases) |
| [Nova Forge Roadmap](docs/ROADMAP.md) | Game-specific development phases |

### Engine Documentation

| Document | Description |
|----------|-------------|
| [Overview](docs/00_OVERVIEW.md) | High-level engine capabilities and philosophy |
| [Engine Architecture](docs/01_ENGINE_ARCHITECTURE.md) | Module structure, boot flow, runtime modes |
| [Editor Design](docs/02_EDITOR_DESIGN.md) | Editor panels, docking, play-in-editor modes |
| [Asset System](docs/03_ASSET_SYSTEM.md) | Asset types, binary format, hot reload |
| [Graph VM](docs/04_GRAPH_VM.md) | Bytecode VM, instruction set, compilation |
| [Networking](docs/05_NETWORKING.md) | NetContext, replication, lockstep/rollback |
| [World Generation](docs/06_WORLD_GENERATION.md) | Terrain, planets, galaxies, streaming |
| [Development Roadmap](docs/09_DEVELOPMENT_ROADMAP.md) | Engine phase-by-phase status |
| [Core Contract](docs/ATLAS_CORE_CONTRACT.md) | Non-negotiable engine invariants |
| [Determinism Enforcement](docs/ATLAS_DETERMINISM_ENFORCEMENT.md) | Compile-time, runtime, CI enforcement |
| [Building](docs/BUILDING.md) | Build prerequisites, script usage, troubleshooting |

### Reference

| Document | Description |
|----------|-------------|
| [GUI System](docs/12_GUI_SYSTEM.md) | Custom GUI architecture, DSL, layout solver |
| [Editor UI](docs/13_EDITOR_UI.md) | Editor aesthetics, panels, self-hosting |
| [Replay & Proofs](docs/14_REPLAY_AND_PROOFS.md) | Replay system, hash ladder, TLA+ verification |
| [Flow Graph](docs/15_FLOW_GRAPH.md) | Blueprint-like visual scripting, IR, debugger |
| [AtlasAI](docs/16_ATLAS_AI.md) | AI assistant, web aggregation, context-aware prompts |
| [Architecture Reference](docs/ARCHITECTURE.md) | Detailed module-by-module reference |
| [Naming Conventions](docs/ATLAS_NAMING_CONVENTIONS.md) | Code style and naming rules |

## Development Tools

| Tool | Purpose |
|------|---------|
| **Replay Minimizer** (`tools/replay_minimizer.py`) | Reduces failing replays to minimal reproduction cases |
| **Contract Scanner** (`tools/contract_scan.py`) | Scans for forbidden APIs that violate the Core Contract |
| **CMake Contract Enforcement** (`engine/cmake/AtlasContractEnforcement.cmake`) | Enforces layer dependencies at build time |

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.
