# NovaForge — Modding Guide

> This guide explains how to create and load mods for Nova Forge. All game data is stored as JSON and can be freely modified or extended.

## Data Directory Structure

```
Data/
├── Ships/           # Ship definitions (hull, slots, stats)
├── Modules/         # Module definitions (weapons, shields, engines, etc.)
├── Skills/          # Skill tree definitions (ranks, prerequisites)
├── Missions/        # Mission scripts and parameters
└── Universe/        # Sector and region definitions
```

## JSON Schemas

All data files are validated against JSON schemas in `Schemas/`:

| Schema | Validates |
|--------|-----------|
| `ship.schema.json` | Ship definitions |
| `module.schema.json` | Module definitions |
| `skill.schema.json` | Skill definitions |
| `mission.schema.json` | Mission definitions |

## Creating a Ship Mod

### 1. Define the ship

Create a new JSON file in `Data/Ships/`:

```json
{
  "id": "mod_custom_frigate",
  "name": "Custom Frigate",
  "class": "Frigate",
  "techLevel": 1,
  "hull": {
    "hp": 1200,
    "armor": 80,
    "shield": 400,
    "mass": 12000.0
  },
  "slots": {
    "highSlots": 3,
    "midSlots": 3,
    "lowSlots": 2,
    "rigSlots": 2
  },
  "capacitor": {
    "capacity": 300.0,
    "rechargeRate": 4.5
  },
  "propulsion": {
    "maxSpeed": 350.0,
    "warpSpeed": 3.0,
    "agility": 3.2
  },
  "description": "A custom frigate with balanced stats."
}
```

### 2. Validate against schema

Use any JSON Schema validator to check your file against `Schemas/ship.schema.json`.

### 3. Load in-game

Place the file in `Data/Ships/` and it will be discovered automatically by the content browser and game data loader.

## Creating a Module Mod

Create a file in `Data/Modules/`:

```json
{
  "id": "mod_laser_turret",
  "name": "Custom Laser Turret",
  "type": "Weapon",
  "slotType": "High",
  "stats": {
    "damage": 45.0,
    "rateOfFire": 2.5,
    "range": 8000.0,
    "tracking": 0.6,
    "capacitorUsage": 12.0
  },
  "description": "A modded laser turret with improved tracking."
}
```

## Creating a Skill Mod

Create a file in `Data/Skills/`:

```json
{
  "id": "mod_advanced_tactics",
  "name": "Advanced Fleet Tactics",
  "category": "Leadership",
  "maxRank": 5,
  "trainingMultiplier": 4,
  "prerequisites": ["fleet_command_3"],
  "bonusPerRank": {
    "fleetWarpSpeed": 0.05,
    "fleetAgility": 0.03
  },
  "description": "Improves fleet coordination and agility."
}
```

## Creating a Mission Mod

Create a file in `Data/Missions/`:

```json
{
  "id": "mod_patrol_mission",
  "name": "Patrol the Frontier",
  "type": "Combat",
  "level": 3,
  "objectives": [
    {
      "type": "destroy",
      "target": "pirate_frigate",
      "count": 5
    },
    {
      "type": "visit",
      "location": "frontier_beacon"
    }
  ],
  "rewards": {
    "credits": 50000,
    "reputation": 0.1,
    "items": ["mod_laser_turret"]
  },
  "description": "Clear pirate threats from the frontier zone."
}
```

## Voxel Editing

Voxel worlds can be saved and loaded as JSON. The `WorldSerializer` handles multi-chunk serialization with full round-trip support.

**Voxel types available:**
- Air, Stone, Dirt, Grass, Metal, Glass, Water, Ore_Iron, Ore_Gold, Ore_Crystal

## Graph Visual Scripting

The GraphVM supports 14 graph types for modding game logic:
- World, Strategy, Conversation, Behavior, Animation, Character
- Weapon, Tile, Sound, UI, UIScreen, GameFlow, Story, Asset

Graphs are saved as JSON and can be compiled to bytecode for deterministic execution.

## Best Practices

1. **Use unique IDs** — Prefix mod IDs with your mod name (e.g., `mymod_ship_name`)
2. **Validate schemas** — Always validate against the JSON schemas before testing
3. **Test locally** — Use the NovaForge Editor to browse and validate your data files
4. **Keep backups** — The editor supports undo/redo for voxel edits

## File Format Reference

All data files use JSON format with the following conventions:
- UTF-8 encoding
- 2-space indentation
- Snake_case for IDs
- CamelCase for display names
- Numeric values use appropriate types (int for counts, float for rates)
