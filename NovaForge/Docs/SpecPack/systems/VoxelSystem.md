# Voxel System Specification

## Overview
Voxels are the simulation truth layer. All world state, destruction, construction,
and resource extraction operates on the voxel grid.

## Existing Implementation
```cpp
enum class VoxelType : uint8_t {
    Air = 0, Stone, Dirt, Grass, Metal, Glass, Water,
    Ore_Iron, Ore_Gold, Ore_Crystal, Count
};

struct Chunk {
    static constexpr int CHUNK_SIZE = 16;
    VoxelType voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    // mesh/collision dirty flags, get/set, isFullyAir, solidCount
};
```

## Missing: Material Properties Table
Each VoxelType needs a material definition:

| VoxelType | Density | Hardness | IsLoose | Collapse | Yield |
|-----------|---------|----------|---------|----------|-------|
| Air | 0 | 0 | false | false | - |
| Stone | 2.5 | 8 | false | false | gravel |
| Dirt | 1.5 | 2 | true | true | soil |
| Grass | 1.3 | 2 | true | true | soil |
| Metal | 7.8 | 9 | false | false | scrap_metal |
| Glass | 2.2 | 3 | false | false | silica |
| Water | 1.0 | 0 | true | false | - |
| Ore_Iron | 5.0 | 7 | false | false | raw_iron |
| Ore_Gold | 8.0 | 6 | false | false | raw_gold |
| Ore_Crystal | 3.0 | 5 | false | false | raw_crystal |

## Missing: Collapse Rules
- Loose materials (dirt, grass) collapse when unsupported
- Support = solid voxel directly below OR within support radius
- Collapse cascade: each fallen voxel re-evaluates neighbors
- Sand physics: loose material flows to fill gaps

## Missing: Structural Integrity
- Load-bearing calculation for constructed structures
- Weight propagation through connected voxels
- Failure threshold per material type
- Damage propagation from impact/explosion

## Schema File
→ `Schemas/material.schema.json`

## Code Location
- `Source/Game/include/NF/Game/Game.h` (VoxelType, Chunk)
- `Source/World/` (WorldGenerator, world management)
