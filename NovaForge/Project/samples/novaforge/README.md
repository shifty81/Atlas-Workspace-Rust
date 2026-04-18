# NovaForge

**NovaForge** is a PVE space simulator built on the [Atlas Engine](../../README.md). It is inspired by EVE Online and focuses on solo and cooperative PVE gameplay — exploration, mining, missions, and faction warfare — all running on the Atlas deterministic server-authoritative runtime.

---

## Overview

| Property        | Value                              |
|-----------------|------------------------------------|
| Project ID      | `novaforge`                        |
| Version         | `0.1.0`                            |
| Entry World     | `worlds/galaxy.worldgraph`         |
| Tick Rate       | 30 Hz                              |
| Max Players     | 20                                 |
| Engine          | Atlas Engine `>=0.1.0`             |

---

## Project Structure

```
novaforge/
├── novaforge.atlas          # Project manifest
├── Plugin.toml              # Engine plugin descriptor
├── README.md
│
├── worlds/
│   ├── galaxy.worldgraph         # Top-level galaxy world graph
│   └── station_interior.tilegraph
│
├── strategy/
│   └── faction_ai.strategygraph  # Faction-level AI strategy
│
├── conversations/
│   └── captain_greeting.conversation
│
├── ai/
│   ├── combat.behaviorgraph
│   ├── trade.behaviorgraph
│   ├── patrol.behaviorgraph
│   └── npc_config.json
│
├── data/                    # All game content data (JSON)
│   ├── manifest.json
│   ├── ships/
│   ├── modules/
│   ├── npcs/
│   ├── missions/
│   ├── universe/
│   └── ...
│
├── config/
│   └── runtime.json         # Server runtime configuration
│
├── assets/                  # Art, audio, and visual assets (placeholder)
│
├── Code/
│   ├── GameInit.h / .cpp    # Engine entry point
│   └── Adapters/
│       └── ECSAdapter.h / .cpp
│
├── Schemas/
│   ├── novaforge.ship.v1.json
│   ├── novaforge.module.v1.json
│   └── novaforge.npc.v1.json
│
├── module/
│   ├── NovaForgeModule.h
│   ├── NovaForgeModule.cpp
│   └── CMakeLists.txt
│
└── Tests/
    └── test_project_structure.cpp
```

---

## Getting Started

### Open in Atlas Forge (Editor)

1. Launch **Atlas Forge**.
2. Choose **Open Project** and browse to `projects/novaforge/novaforge.atlas`.
3. The galaxy world graph opens as the default view.

### Run Validation Tests

```bash
atlas_runtime --project projects/novaforge/novaforge.atlas --validate-only
```

Or compile and run the C++ structure tests directly:

```bash
g++ -std=c++17 Tests/test_project_structure.cpp -o test_novaforge
./test_novaforge /path/to/projects/novaforge
```

### Build the Game Module

```bash
cmake -B build -S .
cmake --build build --target NovaForgeModule
```

---

## Gameplay Systems

| System              | Description                                              |
|---------------------|----------------------------------------------------------|
| **Factions**        | Amarr, Caldari, Gallente, Minmatar, Pirates              |
| **Ships**           | Frigates → Capitals, Tech I & Tech II variants           |
| **Modules**         | Weapons, defense, propulsion, mining, drones, utility    |
| **Missions**        | Level 1–5 PVE missions, epic arcs, incursions            |
| **Mining**          | Asteroid belts, ice fields, gas clouds                   |
| **Exploration**     | Deadspace complexes, anomalies, wormholes                |
| **Economy**         | Market orders, blueprints, manufacturing, contracts      |
| **AI**              | Patrol, combat, trade behavior graphs                    |

---

## Configuration

Runtime settings are in `config/runtime.json`. Key server rules:

| Rule                    | Default | Range     |
|-------------------------|---------|-----------|
| `miningYieldMultiplier` | 1.0     | 0.1 – 5.0 |
| `npcSpawnRate`          | 1.0     | 0.0 – 3.0 |
| `marketUpdateInterval`  | 300 s   | 0.5 – 10× |
| `pirateAggressionLevel` | 1.0     | 0.0 – 5.0 |
| `warpSpeedMultiplier`   | 1.0     | 0.5 – 3.0 |

---

## License

See the root [LICENSE](../../LICENSE) file.
