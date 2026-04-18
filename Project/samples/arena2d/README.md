# Arena2D — Atlas Reference Project

Minimal 2D tile-based arena game demonstrating Atlas Engine capabilities.

## Purpose

This project serves as a **scalability proof** — it validates that Atlas can host
multiple games simultaneously without engine pollution.

## Project Structure

```
arena2d/
├── arena2d.atlas           # Project manifest
├── Plugin.toml             # Plugin descriptor
├── module/                 # Game module (IGameModule implementation)
│   ├── Arena2DModule.h     # Module header (components, factions, economy)
│   ├── Arena2DModule.cpp   # Module implementation
│   └── CMakeLists.txt      # Build configuration
├── worlds/                 # TileGraph files
│   └── arena.tilegraph     # Arena map generation
├── ai/                     # BehaviorGraph files
│   └── enemy_patrol.behaviorgraph
├── data/                   # Game content (JSON)
│   └── enemies.json
├── config/                 # Runtime configuration
│   └── runtime.json
├── assets/                 # Art assets (sprites, audio)
└── README.md
```

## Game Module

The `Arena2DModule` implements the engine's `IGameModule` interface:

- **3 factions**: Heroes, Monsters, Neutrals (with hostility/friendly relations)
- **2 economy resources**: Gold, XP
- **3 replication rules**: Position (every-tick), Health and SpriteInfo (on-change)
- **3 server rules**: enemySpawnRate, playerDamageMultiplier, xpMultiplier
- **Factory function**: `CreateArena2DModule()` for dynamic loading

## Key Features Demonstrated

- TileGraph-based procedural level generation
- BehaviorGraph-driven AI enemies
- IGameModule interface implementation
- Faction system with inter-faction relations
- Economy resource management
- Replication rule configuration
- Server rule tuning
- JSON-defined game content
- Atlas project conventions compliance

## Validation

This project can be loaded alongside EVE Offline or any other Atlas project.
Deleting this project directory should have zero impact on the engine or other projects.
