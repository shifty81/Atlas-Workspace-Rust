# Nova Forge — Project Roadmap

> **Consolidated roadmap** — For deep design detail, see the [Master Design Bible](design/MASTER_DESIGN_BIBLE.md).
> Archived planning docs are in [`docs/archive/`](archive/).

---

## Project Vision

Nova Forge is a **PvE-focused space simulator** for solo play or small groups (2–20 players), built on the custom **Atlas Engine** (C++/OpenGL). The game delivers EVE-like depth — ships, skills, fitting, combat, missions, exploration — powered by an AI-driven economy where every NPC is a real economic actor. The simulation is server-authoritative, tick-based (10–20 Hz), and deterministic for networking and replays. All UI is rendered through the custom Atlas UI retained-mode system with an EVE-style sci-fi dark theme. Content is fully moddable via JSON.

---

## Architecture

| Layer | Responsibility |
|-------|---------------|
| **Server (Authoritative)** | Tick-based simulation, ECS, AI, economy, combat, persistence |
| **Client (Display)** | Rendering, interpolation, UI, input, audio — no game logic authority |
| **Networking** | Server snapshots → Client cache → Interpolation → Render |

---

## Design Enforcement Rules (Non-Negotiable)

Every new system or feature **must** pass all three checks:

1. **Atlas-fit** — Does it belong naturally inside Atlas Engine?
2. **Simulation-first** — Does it work without UI, audio, or player input?
3. **Lore-consistent** — Would captains in-universe believe this exists?

If it fails any one → it doesn't ship.

---

## Systems Overview

| System | Key Subsystems | Status |
|--------|---------------|--------|
| **Atlas Engine Core** | ECS, Deterministic Scheduler, EventBus, Plugin System | ✅ Complete |
| **Networking** | TCP Client/Server, Interpolation, Delta Compression, Snapshot Replication | ✅ Complete |
| **Rendering** | OpenGL, Deferred Pipeline, Shadow Mapping, Post-Processing | ✅ Complete |
| **Atlas UI** | Widgets, Panels, Docking (DockNode tree), Photon Dark Theme | ✅ Complete |
| **Atlas Editor** | 16+ Panels, PCG Tools, Dock Layout, Debug Console | ✅ Complete |
| **Ship HUD** | Control Ring (shield/armor/hull arcs), Capacitor Bar, Module Rack, Target Brackets | ✅ Complete |
| **PCG Ship Generation** | Spine-based Hulls, Turrets, Faction Shape Language, Procedural Materials | ✅ Complete |
| **PCG Content** | Stations, Interiors, Characters, Galaxy Generation | ✅ Complete |
| **Combat** | Lock/Fire/Damage Layers, Wrecks, Salvage, Security Response | ✅ Complete |
| **Fleet System** | Wings (5×5), Doctrine, Morale, Chatter, Fracture Mechanics | ✅ Complete |
| **AI NPCs** | Miners, Haulers, Pirates, Traders, Security, Intent-driven Behavior Trees | ✅ Complete |
| **Economy** | Markets, Supply/Demand, Trade Routes, Dynamic Tax, Broker Fees | ✅ Complete |
| **Mining & Industry** | Ore, Ice, Refining, Manufacturing, Planetary Industry | ✅ Complete |
| **Missions** | Procedural Templates, Objectives, Rewards, Branching Chains, Consequences | ✅ Complete |
| **Exploration** | Anomalies, Wormholes, Scan Probes, Jump Gates | ✅ Complete |
| **Audio** | OpenAL, Positional 3D, Fleet Audio, Warp Adaptive Audio | ✅ Complete |
| **Physics** | Rigid Body Dynamics, AABB Collision | ✅ Complete |
| **Animation** | Graph Pipeline, Clip/Blend/Modifier/StateMachine Nodes | ✅ Complete |
| **Warp System** | Tunnel Shader Stack, Adaptive Audio, Mass-based Intensity, Anomalies | ✅ Complete |
| **Titan & Meta-Threat** | Assembly Pressure, Logistics Distortion, Rumor Propagation, Galactic Response | ✅ Complete |
| **FPS & Interiors** | EVA Airlock, Salvage Paths, Lavatory, Rig Locker, Clone Bay | ✅ Complete |
| **Planetary** | Space→Planet Transition, Rover Bay, Grid Construction, Hangars | ✅ Complete |
| **Community & Modding** | Content Repository, Validation, Mission Editor, Mod Manager, Doc Generator | ✅ Complete |
| **Vertical Slice Integration** | Full star system: fly, fight, mine, trade, dock — end-to-end | 🔧 In Progress |
| **Client Polish & Optimization** | Profiling, Spatial Partitioning, 500+ Entity Performance | 🔧 In Progress |
| **Steam / Distribution** | Packaging, Installer, Steam Integration | 📋 Planned |

---

## Development Phases

| Phase | Name | Version | Status |
|-------|------|---------|--------|
| **0** | Foundation — Engine, rendering, ECS, basic flight, HUD, audio | v0.1 | ✅ Complete |
| **1** | Core Fleet — Player + 4 captains, fitting, mining, warp, stations | v0.1–v0.2 | ✅ Complete |
| **2** | Living Universe — Background sim, NPC life, economy engine, combat aftermath | v0.2 | ✅ Complete |
| **3** | Wing System & Midgame — 3×5 wings, doctrine, tactical overlay, imperfect info | v0.3 | ✅ Complete |
| **4** | Full Fleet Doctrine & Endgame — 5×5 fleet, ideology, fracture, persistence | v0.4+ | ✅ Complete |
| **5** | Titan Systems & Meta-Threat — Galactic emergent threats, warp anomalies | v0.5+ | ✅ Complete |
| — | Vertical Slice — One full star system, end-to-end playable | — | 🔧 In Progress |
| — | Client Integration & Polish — Smooth networking, full HUD, performance | — | 🔧 In Progress |

---

## Current Priorities

1. **Spaghetti Code Cleanup** — Split monolithic test file, reduce system boilerplate, decompose GameSession god object (see [SPAGHETTI_CODE_AUDIT.md](SPAGHETTI_CODE_AUDIT.md))
2. **Code Refactoring** — JSON helper consolidation, file splits, naming consistency (see [REFACTORING_PLAN.md](REFACTORING_PLAN.md))
3. **Vertical Slice** — One full star system: fly, dock, mine, trade, fight, warp — end-to-end playable
4. **Client–Server Integration** — Wire up all server systems to C++ client rendering and UI
5. ~~**Editor Conversion**~~ — ✅ Complete — All 26 editor tools ported from NovaForge and wired into ToolingLayer (see [EDITOR_CONVERSION_PLAN.md](EDITOR_CONVERSION_PLAN.md))
6. **FPS/Flight/Fleet Command** — Implement tri-modal gameplay with PlayerModeController (see [GAME_DESIGN_VISION.md](GAME_DESIGN_VISION.md))
7. **Performance Profiling** — Maintain 20 Hz server tick with 500+ entities; 60 FPS client with 200 visible ships

---

## New Direction: Tri-Modal Gameplay

Nova Forge is pivoting from a pure space sim to a **tri-modal FPS + Flight + Fleet Command** experience:

- **FPS Mode** — Boots on deck, EVA, boarding, procedural low-poly interiors
- **Flight Mode** — Direct ship piloting with class-based flight identity
- **Fleet Command Mode** — RTS-style intent-based fleet orders via tactical terminal

See [GAME_DESIGN_VISION.md](GAME_DESIGN_VISION.md) for full design details including:
- PlayerModeController architecture
- Fleet command vocabulary (6 orders)
- FPS combat state machine and enemy archetypes
- Low-poly visual style enforcement
- Legend/reputation system
- Campaign structure
- Procedural solar system density generation

---

## Explicitly Out of Scope

- ❌ PvP combat (PvE focus)
- ❌ Player empires / sovereignty
- ❌ Full MMO economy simulation
- ❌ Scripted storylines (emergent only)
- ❌ Fake distances or rubber-band difficulty
- ❌ Client-authoritative logic
- ❌ Theme-park content

---

## Development Milestones

| Milestone | Target | Criteria |
|-----------|--------|----------|
| **Engine Baseline** | ✅ Done | ECS, rendering, audio, physics, networking, UI framework |
| **All Server Systems** | ✅ Done | 5,263+ test assertions passing across 164 ECS systems |
| **Atlas Engine Modules** | ✅ Done | 14 graph types, determinism, replay, AI, world gen |
| **Code Cleanup** | 🔧 Active | Split monoliths, reduce boilerplate, decompose god objects |
| **Vertical Slice** | 🔧 Active | One star system fully playable end-to-end |
| **Editor Conversion** | ✅ Complete | In-game ToolingLayer with 26 editor tools, EditorCommandBus, undo/redo |
| **Tri-Modal Gameplay** | ✅ Complete | FPS + Flight + Fleet Command with mode controller |
| **Legend System** | ✅ Complete | Reputation-driven world bias across runs |
| **Alpha** | TBD | 2–4 players, smooth networking, core gameplay loop |
| **Beta** | TBD | Full UI, content variety, performance targets met |
| **Release** | TBD | Packaged builds, mod support, documentation |

---

## Naming Conventions

| Scope | Prefix | Example |
|-------|--------|---------|
| Engine-wide | `Atlas` | `AtlasDeterministicScheduler` |
| Simulation | `Sim` | `SimFleetMoraleComponent` |
| Rendering | `AtlasRender` | `AtlasRenderTacticalOverlay` |
| UI | `UI_` | `UI_TacticalOverlayRing` |
| Game-specific | `EVO_` | `EVO_PirateCoalitionDoctrine` |

**Forbidden:** Generic names (`Manager`, `Controller`, `Handler`), engine-agnostic names (`InventorySystem`).
Components = pure data, no logic, serializable. Systems = stateless logic, deterministic, scheduled by Atlas ECS.

---

*For detailed system designs, see the [Master Design Bible](design/MASTER_DESIGN_BIBLE.md).*
*Archived planning documents: [`docs/archive/`](archive/)*
