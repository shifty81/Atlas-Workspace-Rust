# NovaForge — Architecture

## Module Dependency Graph

```
NF::Core ──────────────────────────────────────────────────────────────────┐
NF::Engine      → NF::Core                                                 │
NF::Renderer    → NF::Core, NF::Engine                                     │
NF::Physics     → NF::Core, NF::Engine                                     │
NF::Audio       → NF::Core                                                 │
NF::Animation   → NF::Core, NF::Engine                                     │
NF::Input       → NF::Core                                                 │
NF::Networking  → NF::Core, NF::Engine                                     │
NF::GraphVM     → NF::Core                                                 │
NF::AI          → NF::Core, NF::Engine, NF::GraphVM                        │
NF::World       → NF::Core, NF::Engine, NF::Game                           │
NF::Pipeline    → NF::Core                                                 │
NF::UI          → NF::Core, NF::Renderer                                   │
NF::Game        → NF::Core, NF::Engine, NF::UI, NF::Renderer               │
NF::Editor      → NF::Core, NF::Engine, NF::Renderer, NF::Game, NF::UI,   │
                   NF::GraphVM, NF::Pipeline                               │
                                                                           │
NovaForgeEditor → NF::Editor + all modules ────────────────────────────────┘
NovaForgeGame   → NF::Game + NF::Renderer + NF::UI + supporting modules
NovaForgeServer → NF::Game + NF::Networking + NF::AI + NF::World
```

## Source Layout

```
Source/
├── Core/          # Math, memory, logging, events, reflection, serialization
├── Engine/        # ECS, world/level, behavior trees, asset system
├── Renderer/      # RHI (OpenGL via WGL/GLAD), forward pipeline, mesh, materials
├── Physics/       # Rigid bodies, collision detection, character controller
├── Audio/         # Device, spatial audio, mixer, cues
├── Animation/     # Skeleton, blend tree, state machine, IK
├── Input/         # Keyboard, mouse, gamepad, action mappings
├── Networking/    # Sockets, replication, sessions, lockstep/rollback
├── GraphVM/       # Deterministic bytecode VM, compiler, serialization
├── AI/            # Behavior graphs, memory, NPC logic, faction relationships
├── World/         # World gen: cube-sphere, voxel, terrain, galaxy, streaming
├── Pipeline/      # S0 pipeline core: PipelineWatcher, Manifest, WatchLog
├── UI/            # Custom 2-D renderer (no ImGui) — quad batching + stb_easy_font
├── Game/          # Game layer — see domain layout below
├── Editor/        # Editor application, docking panels, viewport, toolbar
└── Programs/
    ├── NovaForgeEditor/   # Editor executable (development tool, does not ship)
    ├── NovaForgeGame/     # Standalone game client (shippable artifact)
    └── NovaForgeServer/   # Dedicated headless server
```

## Source/Game/ Domain Layout

> All game-layer code lives under `Source/Game/`. AtlasAI broker systems are
> **NOT** permitted here — they belong under `AtlasAI/` at the repo root.

| Domain | Key Types |
|--------|-----------|
| **Voxel** | VoxelType, Chunk, ChunkSerializer, VoxelEditCommand, VoxelPickService |
| **World** | WorldState, WorldSerializer, ChunkMesher, VoxelShader, ChunkRenderer |
| **Interaction** | ResourceType, ToolBelt, RigState, HUD, InteractionSystem, GameSession |
| **Movement** | FPSCamera, PlayerMovement, VoxelCollider, PlayerController |
| **Ships** | ShipClass, ShipModule, Ship, FlightController, CombatSystem |
| **Fleet** | Formation, CaptainPersonality, AICaptain, Fleet |
| **Economy** | ResourceMarket, Refinery, ManufacturingQueue, EconomySystem |
| **Exploration** | ProbeScanner, WormholeNetwork, AncientTechRegistry |
| **Interiors** | ShipRoom, ShipInterior, EVAState, SurvivalStatus |
| **Legend** | PlayerReputation, WorldBiasMap, NPCMemory, LegendStatus |
| **Quests** | MissionObjective, ActiveMission, MissionLog, QuestChain |
| **Dialogue** | DialogueCondition, DialogueNode, DialogueGraph, DialogueRunner |
| **Save** | SaveSlot, SaveData, GameSaveSerializer, SaveSystem |
| **Events** | WorldEventType, EventEffect, WorldEvent, WorldEventSystem |
| **TechTree** | TechCategory, TechNode, TechTree |
| **Progression** | PlayerLevel, SkillNode, SkillTree, ProgressionSystem |
| **Crafting** | CraftingRecipe, CraftingQueue, CraftingSystem |
| **Inventory** | ItemRarity, ItemSlot, Item, PlayerInventory, EquipmentLoadout |

## Layer Responsibilities

| Layer | Responsibility |
|-------|---------------|
| **Core** | Foundation: math, memory, logging, events, reflection, serialization |
| **Engine** | ECS framework, world/level management, behavior trees, asset system |
| **Renderer** | OpenGL RHI, forward rendering pipeline, mesh, materials, shaders |
| **Physics** | Rigid body simulation, collision detection, character controller |
| **Audio** | Audio device, spatial audio, mixer, sound cues |
| **Animation** | Skeleton, blend tree, state machine, IK |
| **Input** | Keyboard, mouse, gamepad, action mapping |
| **Networking** | Client-server, P2P, lockstep/rollback, replication, sessions |
| **GraphVM** | Deterministic bytecode VM for visual scripting (14 graph types) |
| **AI** | Behavior graphs, NPC memory, faction relationships, fleet AI |
| **World** | Procedural world gen, chunk streaming, terrain, galaxies |
| **Pipeline** | S0 pipeline core: PipelineWatcher, Manifest, WatchLog, ChangeEvent |
| **UI** | Custom 2D renderer, quad batching, text, panels (no ImGui) |
| **Game** | All game-layer domains: voxel, interaction, ships, economy, progression |
| **Editor** | Docking layout, 14+ panels, project path service, command registry, IDE |

## Executables

| Binary | Purpose |
|--------|---------|
| `NovaForgeEditor` | Full editor + runtime (superset) — development tool |
| `NovaForgeGame` | Standalone game client — shippable artifact |
| `NovaForgeServer` | Dedicated headless server for multiplayer |

## Design Locks

These decisions are **locked** and must not be revisited without a full phase reset:

1. **No ImGui** — UI is fully custom (OpenGL quad batching + stb_easy_font)
2. **Voxel-first** — All structure, mining, damage, and destruction are voxel operations
3. **Editor does not ship** — `NovaForgeGame` is the shippable artifact
4. **Determinism** — All simulation is bit-exact reproducible (networking + replay)
5. **AtlasAI boundary** — AI broker systems live under `AtlasAI/`, not `Source/Game/`
6. **NF:: namespace** — All engine/game code uses the `NF::` namespace
7. **CMake is truth** — No secondary build systems
