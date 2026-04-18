# Procedural Content Generation (PCG) — Overview

## Scope

PCG covers all procedurally generated content in NovaForge including:
- World generation (voxel terrain, biomes)
- Dungeon/structure generation
- Loot table generation
- NPC spawn and placement
- Spaceship generation (via Atlas_BlenderGenerator)

## Systems

| System                  | Location                        | Status    |
|-------------------------|---------------------------------|-----------|
| Voxel World Gen         | Source/World/PCG/               | Planned   |
| Structure Generator     | Source/World/Structures/        | Planned   |
| Loot Tables             | Source/Game/Loot/               | Planned   |
| NPC Spawner             | Source/Game/NPC/                | Planned   |
| Spaceship Generator     | Tools/AtlasAI/BlenderGenerator/ | Migrating |

## Architecture

PCG systems are seeded, deterministic, and replay-safe. All generators accept a `uint64_t seed` and produce identical output given the same seed.

## Integration

PCG hooks into the World subsystem via `IWorldGenerator` interface (planned).
