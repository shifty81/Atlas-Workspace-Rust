# Atlas Engine — Architecture

This document defines the **baseline architecture** for the Atlas Engine, establishing the
structural contracts and module boundaries that all development must follow.

## Overview

Atlas is a **deterministic, tool-first game engine** extracted from the EVEOFFLINE game project.
It separates reusable engine systems from game-specific content, enabling multiple projects to
share the same core technology.

```
Atlas/
├── engine/              # Atlas Engine (game-agnostic static library)
│   ├── core/            # Engine lifecycle, logging
│   ├── ecs/             # Entity-Component-System framework
│   ├── graphvm/         # Deterministic bytecode graph VM, serialization, caching
│   ├── assets/          # Asset registry, binary format, hot reload
│   ├── net/             # Networking (client-server, P2P)
│   ├── sim/             # Tick scheduler, deterministic simulation
│   ├── world/           # Procedural world generation, WorldGraph, heightfield meshing
│   ├── tile/            # TileGraph — 2D tile-based procedural generation
│   ├── strategygraph/   # Strategy decision graphs (influence, threat, scoring)
│   ├── conversation/    # Dialogue + memory graphs (ConversationGraph)
│   ├── ai/              # AI signals, memory, relationships, BehaviorGraph
│   ├── character/       # CharacterGraph — modular character generation
│   ├── animation/       # AnimationGraph — animation state machines + modifiers
│   ├── weapon/          # WeaponGraph — weapon construction + wear
│   ├── sound/           # SoundGraph — procedural audio generation
│   ├── story/           # StoryGraph — narrative generation
│   ├── ui/              # UIGraph — UI composition system
│   ├── flow/            # GameFlowGraph — game flow state machine
│   ├── schema/          # Schema validation system
│   ├── camera/          # World modes and camera projection policies
│   ├── input/           # Input mapping system
│   ├── physics/         # Physics simulation (rigid bodies, AABB collision)
│   ├── audio/           # Audio engine
│   ├── gameplay/        # Mechanic assets & skill trees
│   ├── project/         # Project loading and validation (.atlas files)
│   ├── command/         # Undo/redo command system
│   ├── interaction/     # Unified intent/utterance system (voice, AI, console)
│   ├── voice/           # Voice command registry and matching
│   ├── plugin/          # Plugin validation, registry
│   ├── mod/             # Mod asset registry
│   ├── module/          # Game module interface and dynamic loader
│   ├── asset_graph/     # Asset graph executor
│   └── rules/           # Server rules system (live parameter tuning)
│
├── modules/
│   └── atlas_gameplay/  # AtlasGameplay static lib (factions, combat, economy)
│
├── projects/
│   ├── eveoffline/      # EveOffline project data and game module
│   │   └── module/      # EveOfflineModule (IGameModule implementation)
│   ├── atlas-sample/    # Generic sample project
│   └── arena2d/         # Arena 2D sample project
│
├── editor/              # Atlas Editor (authoring tool)
│   ├── panels/          # Console, ECS Inspector, Net Inspector, World Graph,
│   │                    # Project Picker, Voice Commands, Interaction Debugger
│   ├── ui/              # Dock layout, panel framework
│   ├── tools/           # Game packager panel
│   ├── ai/              # AI aggregator
│   └── assistant/       # Editor assistant (explain, suggest)
│
├── runtime/             # Atlas Runtime (standalone CLI, module loading)
├── client/              # Player runtime client
├── server/              # Headless dedicated server
│
├── tests/               # Engine unit tests (90 test files)
│
├── schemas/             # Versioned JSON schemas
│   ├── atlas.project.v1.json
│   ├── atlas.worldgraph.v1.json
│   ├── atlas.strategygraph.v1.json
│   └── atlas.conversation.v1.json
│
├── projects/            # Sample game projects
│   ├── eveoffline/      # Space strategy reference project
│   ├── arena2d/         # 2D arena reference project
│   └── atlas-sample/    # Minimal sample project
│
└── docs/                # Design documents and guides
```

## Engine Modules

### Core (`engine/core/`)
- **Engine**: Lifecycle management, mode selection (Editor/Client/Server)
- **Logger**: Structured logging
- **Capabilities**: Role-based feature gating (AssetWrite, Rendering, NetAuthority, etc.)

### ECS (`engine/ecs/`)
- Type-safe entity creation and destruction
- Component storage via `std::any` (type-indexed)
- Per-tick update callbacks

### Graph VM (`engine/graphvm/`)
- Deterministic bytecode virtual machine
- Graph IR (nodes + edges) compiled to linear opcodes
- Arithmetic, comparison, branching, variables, events

### Assets (`engine/assets/`)
- Binary asset format (`.atlasb`) with header, type, version
- Registry with filesystem scanning
- Hot-reload via file timestamp polling

### Networking (`engine/net/`)
- Modes: Standalone, Client, Server, P2P_Host, P2P_Peer
- Packet-based send/receive with tick stamping
- Peer management (add, remove, RTT tracking)
- Loopback mode for local testing
- Snapshot save/rollback (using ECS binary serialization)
- Replication rules (rule-based dirty tracking, component-level delta collection/application)
- Input frame recording and replay with configurable input apply callback

### Simulation (`engine/sim/`)
- Fixed-rate tick scheduler
- Deterministic time stepping
- Time model (SimulationTime / WorldTime / PresentationTime layers)
- Formal world state model (state categorization, snapshotting, rollback)
- Save/load system (versioned binary format with hash integrity verification)
- Replay recorder and hash ladder verification
- Replay divergence inspector

### World Generation (`engine/world/`)
- Cube-sphere planetary projection with LOD
- Voxel grid chunked layout
- Perlin/FBM noise generation (seeded, deterministic)
- Terrain mesh generation with normals
- World streaming with disk cache
- Galaxy generation (star systems, regions, star classes)
- **WorldGraph**: DAG-based world generation graph with compile/execute pipeline
  - Typed value system (Float, HeightField, Seed, Mask, etc.)
  - Topological sort compilation with cycle detection
  - Deterministic per-chunk execution
  - Concrete nodes: Seed, Noise (FBM), Blend, Clamp, Constant
- **HeightfieldMesher**: Heightfield → mesh pipeline with LOD support

### Project System (`engine/project/`)
- Load and validate `.atlas` project files
- Schema-versioned project format (`atlas.project.v1`)
- Module references (world graphs, strategy graphs, AI)
- Runtime configuration (entry world, tick rate)

### Command System (`engine/command/`)
- Abstract `Command` base with Execute/Undo
- `CommandHistory` for undo/redo stacks
- Foundation for editor mutations and multiplayer sync

### Interaction System (`engine/interaction/`)
- **Utterance**: Raw text with speaker/listener context
- **Intent**: Resolved action with parameters and confidence score
- **IntentResolver**: Abstract resolver interface (rule-based, ML, hybrid)
- **PatternIntentResolver**: Case-insensitive phrase matching
- **IntentRegistry**: Central dispatch for intent handlers
- Unified pipeline: Voice → Text → AI → Intent → Command → System

### Voice Commands (`engine/voice/`)
- **VoiceCommandRegistry**: Register and match voice phrases to intents
- Context-aware filtering (game, editor, or both)
- Longest-match phrase resolution
- Bridge between voice input and the intent system

### Plugin System (`engine/plugin/`)
- **PluginDescriptor**: Name, version, compatibility, determinism flag
- **PluginValidator**: Validates descriptor, version compatibility, determinism
- **PluginRegistry**: Register, find, filter plugins by type
- Enforcement: plugins must be deterministic, version-compatible
- Plugin types: graph-extension, editor-panel, asset-importer

### Strategy Graphs (`engine/strategygraph/`)
- DAG-based strategy decision graph with compile/execute pipeline
- **StrategyPinType**: ScalarField, VectorField, Mask, Influence, Heatmap
- **StrategyNode**: Abstract node base with typed ports and evaluation
- **StrategyGraph**: DAG container with topological sort, cycle detection, type validation
- Concrete nodes: InfluenceRing, ThreatField, ObjectiveScore, Constant, ReadAISignal, EmitAction
- Deterministic execution for multiplayer-safe AI decisions
- StrategyGraphs are advisory — they never affect physics or spawn entities

### Server Rules (`engine/rules/`)
- **RuleDescriptor**: Name, base value, multiplier, bounds, hot-reload flag, replay impact
- **ServerRules**: Singleton registry for live parameter tuning
- Multiplier-based: `final_value = base_value * multiplier`
- Clamped to declared min/max bounds
- Hot-reloadable rules apply at tick boundaries
- Replay-aware: rules affecting replay are explicitly marked

### Game Module System (`engine/module/`)
- **IGameModule**: Abstract interface for game-specific logic modules
- **GameModuleDesc**: Module name and version descriptor
- **GameModuleContext**: Engine subsystem references (World, NetContext, ReplicationManager, ServerRules, AssetRegistry, ProjectDescriptor)
- **ModuleLoader**: Dynamic loading via `dlopen`/`LoadLibrary` with static fallback for tests
- Lifecycle: RegisterTypes → ConfigureReplication → ConfigureServerRules → OnStart → OnTick → OnShutdown
- Factory symbol: `extern "C" CreateGameModule()` exported by shared library modules

### EveOffline Module (`projects/eveoffline/module/`)
- Shipped game example (permanently in-repo) implementing `IGameModule` for the EveOffline space simulator
- Registers 5 factions (Amarr, Caldari, Gallente, Minmatar, Pirates) with alliance/hostility relations
- Registers 5 economy resources (ISK, Tritanium, Pyerite, Mexallon, Isogen)
- Configures 3 replication rules (ShipPosition, ShipHealth, Inventory)
- Configures 5 server rules (mining yield, NPC spawn rate, market updates, pirate aggression, warp speed)
- Exports `CreateGameModule()` factory for dynamic loading

### Arena2D Module (`projects/arena2d/module/`)
- Minimal 2D arena reference implementation of `IGameModule`
- Registers 3 factions (Heroes, Monsters, Neutrals) with hostility/friendly relations
- Registers 2 economy resources (Gold, XP)
- Configures 3 replication rules (Position, Health, SpriteInfo)
- Configures 3 server rules (enemy spawn rate, player damage multiplier, XP multiplier)
- Exports `CreateArena2DModule()` factory for dynamic loading

### SDK / CMake Export
- `AtlasEngine` and `AtlasGameplay` targets export CMake config files via `install(EXPORT ...)`
- External game modules can use `find_package(AtlasEngine)` and `find_package(AtlasGameplay)` to link against the SDK
- `build.sh --install` publishes headers, static libraries, and CMake configs to `dist/sdk/`

## AtlasGameplay Library (`modules/atlas_gameplay/`)

Reusable gameplay frameworks, game-agnostic (no project-specific names):

- **FactionSystem**: Faction registration, symmetric relation tracking (Neutral/Friendly/Hostile/Allied)
- **CombatFramework**: Unit registration, damage/armor resolution, health tracking
- **EconomySystem**: Named resource management with capacity limits, spend/add transactions

### Conversation Graphs (`engine/conversation/`)
- DAG-based dialogue and memory graph with compile/execute pipeline
- **ConversationPinType**: Dialogue, Choice, Memory, Relationship, Signal
- **ConversationNode**: Abstract node base with typed ports and evaluation
- Concrete nodes: Dialogue, PlayerChoice, MemoryWrite, RelationshipDelta
- ConversationGraphs emit state — they never execute actions or access ECS
- Authority model: ConversationGraph → emits state → AISignals → StrategyGraph → decides

### AI Signals (`engine/ai/`)
- **AISignalRegistry**: Singleton registry for namespaced numeric AI inputs
- Signals are registered with namespace + name (e.g. `faction.morale`)
- Read/write with tick tracking for temporal awareness
- Used as inputs to StrategyGraph decision nodes
- Mods may register namespaced signals but may not override core signals

### AI Memory (`engine/ai/`)
- **AIMemory**: Key-value memory storage with tick-based decay
- Memories have strength values that decay each tick
- Permanent memories (decay rate = 0) persist indefinitely
- Expired memories (strength ≤ 0) are automatically removed on tick
- Used by NPC AI for persistent state across encounters

### Relationship Model (`engine/ai/`)
- **RelationshipModel**: Symmetric relationship tracking between entity pairs
- Entity pair → float value (symmetric: A→B == B→A)
- Modify relationships with delta adjustments
- Foundation for NPC faction standing and social dynamics

### TileGraph (`engine/tile/`)
- DAG-based 2D tile-map generation graph with compile/execute pipeline
- **TilePinType**: TileID, TileMap, Float, Mask, Seed, Metadata
- Concrete nodes: BaseTile, NoiseField, TileSelect, BiomeGate
- Deterministic procedural 2D worlds (roguelikes, tactics, side-scrollers)

### SoundGraph (`engine/sound/`)
- DAG-based procedural audio generation graph
- **SoundPinType**: AudioBuffer, Float, Seed, Trigger, Envelope
- Concrete nodes: Oscillator, Gain, Mix, Envelope
- Deterministic sound synthesis without external audio libraries

### CharacterGraph (`engine/character/`)
- DAG-based modular character generation graph
- **CharacterPinType**: Float, Mesh, Material, Skeleton, Seed, Equipment
- Concrete nodes: BaseBody, Skeleton, Material, Equipment
- Characters generated from seed + faction + parameters

### AnimationGraph (`engine/animation/`)
- DAG-based animation state machine with modifier system
- **AnimPinType**: Float, Pose, Modifier, Trigger, Mask
- Concrete nodes: Clip, Blend, Modifier (damage/skill/emotion), StateMachine
- Modifiers are additive layers (limp, recoil, tremor) that warp poses

### WeaponGraph (`engine/weapon/`)
- DAG-based weapon construction graph
- **WeaponPinType**: Float, Stats, Seed, Component, Profile
- Concrete nodes: Receiver, Barrel, RecoilProfile, WeaponStats
- Weapon wear degrades performance; faction identity drives variation

### BehaviorGraph (`engine/ai/`)
- DAG-based authorable AI behavior graph
- **BehaviorPinType**: Float, Bool, Action, Perception, EmotionState
- Concrete nodes: ThreatAssessment, UtilityScore, ActionSelector, EmotionUpdate
- AI behavior becomes content, not code — same graph works for FPS, RTS, NPCs

### UIGraph (`engine/ui/`)
- DAG-based UI composition graph system
- **UIPinType**: Float, Bool, String, Layout, Action
- Concrete nodes: Panel, Button, Text, List
- Title screens, inventories, HUDs authored as graphs

### GameFlowGraph (`engine/flow/`)
- DAG-based game flow state machine
- **FlowPinType**: Trigger, State, Bool, Float, String
- Concrete nodes: State, Transition, Timer, Condition
- Full game flow (Boot → Menu → Gameplay → Credits) as a graph

### Schema Validator (`engine/schema/`)
- **SchemaDefinition**: ID, version, fields, node definitions
- **SchemaValidator**: Validates schemas (unique IDs, valid versions, field integrity)
- Engine-level enforcement — schemas validated before load

### Graph Cache (`engine/graphvm/`)
- **GraphCache**: Execution result caching with tick-based invalidation
- Cache key = hash(graphID, seed, lod) for deterministic lookup
- EvictBefore(tick) removes stale entries
- Prevents redundant graph re-evaluation

### Graph Serialization (`engine/graphvm/`)
- **JsonBuilder**: Minimal JSON writer for graph persistence
- **JsonReader**: Minimal JSON parser for graph loading
- No external JSON library dependency — pure C++ standard library

### Camera / World Modes (`engine/camera/`)
- **WorldMode**: SideScroller2D, TopDown2D, TopDownOrbit2_5D, Isometric2_5D
- **CameraProjectionPolicy**: Mode-specific projection parameters
- Gameplay logic must never depend on projection
- Camera defines perception, not simulation

## Runtime Modes

| Mode   | Rendering | Networking | Asset Write | Graph Edit |
|--------|-----------|------------|-------------|------------|
| Editor | ✅         | Optional   | ✅           | ✅          |
| Client | ✅         | ✅          | ❌           | ❌          |
| Server | ❌         | ✅          | ❌           | ❌          |

## Determinism Contract

All simulation systems must satisfy:

> **Same project + same seed + same inputs = same outputs (always)**

Rules:
- No system clock access in simulation
- No global RNG — use injected `DeterministicRNG` only
- No threading outside the tick scheduler
- Fixed-point or deterministic floating-point only

## Project Schema

Projects are defined by a single `project.atlas` JSON file conforming to
`schemas/atlas.project.v1.json`. See the schema file for the full specification.

### Graph Schemas

- **WorldGraph** files (`.worldgraph`) conform to `schemas/atlas.worldgraph.v1.json`
- **StrategyGraph** files (`.strategygraph`) conform to `schemas/atlas.strategygraph.v1.json`
- **ConversationGraph** files (`.conversation`) conform to `schemas/atlas.conversation.v1.json`
- **TileGraph** files (`.tilegraph`) for 2D procedural generation
- **BehaviorGraph** files (`.behaviorgraph`) for AI behavior authoring

### Project Directory

Sample game projects live under `projects/` and are loaded via `.atlas` manifest files:

```
projects/
├── eveoffline/           # Space strategy reference project
│   └── eveoffline.atlas  # Project manifest with world graphs, strategy, AI config
├── arena2d/              # 2D arena reference project
│   └── arena2d.atlas     # Project manifest with AI, asset configuration
└── atlas-sample/         # Minimal sample project
    └── sample.atlas      # Minimal project manifest
```

## Editor Flow

1. Atlas Editor launches
2. **Project Picker** modal appears — user selects a `.atlas` file
3. Project is mounted and validated
4. Declared modules are initialized (WorldGraphs, StrategyGraphs, AI)
5. Editor enters Project Context — tools unlock
6. User can press ▶ Play to launch runtime for live testing

### Editor Panels

| Panel | Status | Purpose |
|-------|--------|---------|
| Console | ✅ | Command execution and logging |
| World Graph Editor | ✅ | Visual WorldGraph authoring and preview |
| Project Picker | ✅ | Project selection and recent projects |
| Voice Commands | ✅ | Voice command testing and monitoring |
| Interaction Debugger | ✅ | AI interaction logging and analysis |
| Network Inspector | ✅ | Network state debugging |
| Game Packager | ✅ | Build configuration UI |
| ECS Inspector | ✅ | Entity and component exploration |
| Asset Browser | ✅ | Asset browsing, filtering, and sorting |

## Build System

The root `CMakeLists.txt` controls all build targets. **All targets are ON by default**
so a single build produces every executable, making everything debuggable at the same time.

| Option              | Default | Description                     |
|---------------------|---------|---------------------------------|
| BUILD_ATLAS_ENGINE  | ON      | Atlas Engine static library     |
| BUILD_ATLAS_GAMEPLAY| ON      | AtlasGameplay static library    |
| BUILD_ATLAS_TESTS   | ON      | Engine unit tests               |
| BUILD_ATLAS_EDITOR  | ON      | Atlas Editor application        |
| BUILD_ATLAS_RUNTIME | ON      | Atlas Runtime application       |
| BUILD_CLIENT        | ON      | Player runtime client           |
| BUILD_SERVER        | ON      | Headless dedicated server       |

Individual targets can be disabled when only a subset is needed (e.g. CI engine-only builds).

## Testing

All engine modules have corresponding test files in `tests/`.
Tests use `assert()` with `[PASS]` output and are registered in `main.cpp`.
Run with: `cd build && ctest` or `./tests/AtlasTests`

## Module Dependency Diagram

The following diagram shows the dependency relationships between major
Atlas modules. Arrows point from dependent to dependency.

```
┌─────────────────────────────────────────────────────────┐
│                     Game Projects                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │  EveOffline   │  │   Arena2D    │  │   (Custom)   │  │
│  │  Module       │  │   Module     │  │   Modules    │  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │
└─────────┼──────────────────┼──────────────────┼─────────┘
          │                  │                  │
          ▼                  ▼                  ▼
    ┌─────────────────────────────────────────────────┐
    │              AtlasGameplay Library              │
    │  (FactionSystem, CombatFramework, EconomySystem)│
    └──────────────────────┬──────────────────────────┘
                           │
                           ▼
    ┌─────────────────────────────────────────────────┐
    │                  AtlasEngine                    │
    │                                                 │
    │  ┌──────┐  ┌──────┐  ┌──────┐  ┌───────────┐  │
    │  │ Core │  │ ECS  │  │ Net  │  │ GraphVM   │  │
    │  └──┬───┘  └──┬───┘  └──┬───┘  └─────┬─────┘  │
    │     │         │         │             │         │
    │  ┌──▼─────────▼─────────▼─────────────▼──────┐  │
    │  │           Simulation Layer                │  │
    │  │  (TickScheduler, TimeModel, WorldState,   │  │
    │  │   StateHasher, SaveSystem, ReplayRecorder)│  │
    │  └─────────────────┬─────────────────────────┘  │
    │                    │                             │
    │  ┌─────────────────▼─────────────────────────┐  │
    │  │           Content Systems                 │  │
    │  │  World · AI · Assets · Audio · Physics    │  │
    │  │  Animation · Character · Weapon · Story   │  │
    │  │  Conversation · Flow · UI · Camera        │  │
    │  └───────────────────────────────────────────┘  │
    └─────────────────────────────────────────────────┘
                           │
          ┌────────────────┼────────────────┐
          ▼                ▼                ▼
    ┌──────────┐    ┌──────────┐    ┌──────────┐
    │  Editor  │    │  Client  │    │  Server  │
    │ (panels, │    │ (player  │    │(headless │
    │  tools,  │    │  runtime)│    │ runtime) │
    │  assist) │    └──────────┘    └──────────┘
    └──────────┘
```

### Key Dependency Rules

- **Engine** depends on nothing external; C++ standard library only
- **AtlasGameplay** depends on **AtlasEngine**
- **Game modules** depend on **AtlasGameplay** and **AtlasEngine**
- **Editor, Client, Server** each link against **AtlasEngine**
- **Simulation code** cannot include rendering headers (enforced by include firewall)
- **Tests** link against **AtlasEngine**, **AtlasGameplay**, and all game modules

### Layer Enforcement

Atlas enforces strict layer boundaries at compile time via `IncludeFirewall.h`:

| Layer | May Include | Forbidden |
|-------|-------------|-----------|
| `core/` | C++ stdlib only | Simulation, rendering, editor, platform |
| `sim/`, `ecs/`, `physics/`, `ai/` | `core/` only | Rendering, editor, platform windows |
| `render/` | `core/`, platform APIs | Simulation internals |
| `editor/` | `core/`, `render/`, `assets/` | Simulation internals (uses commands/events) |

Enforcement mechanisms:
- **Compile-time**: `IncludeFirewall.h` triggers `#error` on forbidden header detection
- **CI**: `contract_scan.py` scans simulation code for banned APIs
- **Runtime**: `test_include_firewall.cpp` verifies no cross-layer includes exist
- **CMake**: Layer-specific `target_link_libraries` prevent linking violations
