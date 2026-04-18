# Resource System Specification

## Overview
Raw materials are unusable. All resources must pass through processing
(centrifuge) before they can be used for construction or upgrades.

## Resource Pipeline
```
Mining (VoxelType) → Raw Material → Centrifuge → Processed Material → Crafting/Construction
```

## Raw Materials (from mining)

| VoxelType Mined | Raw Material | Stack Size |
|-----------------|-------------|------------|
| Stone | gravel | 64 |
| Dirt | soil | 64 |
| Metal | scrap_metal | 32 |
| Glass | silica | 32 |
| Ore_Iron | raw_iron | 16 |
| Ore_Gold | raw_gold | 8 |
| Ore_Crystal | raw_crystal | 8 |

## Centrifuge Conversion Table

| Input | Output | Ratio | Time (s) | Power (kW) |
|-------|--------|-------|----------|------------|
| gravel ×4 |ite_powder ×1 | 4:1 | 5 | 3 |
| soil ×4 | nutrient_paste ×1 | 4:1 | 3 | 2 |
| scrap_metal ×2 | metal_ingot ×1 | 2:1 | 8 | 5 |
| silica ×2 | glass_sheet ×1 | 2:1 | 6 | 4 |
| raw_iron ×1 | iron_ingot ×1 | 1:1 | 10 | 6 |
| raw_gold ×1 | gold_ingot ×1 | 1:1 | 15 | 8 |
| raw_crystal ×1 | crystal_lens ×1 | 1:1 | 20 | 10 |

## Processed Materials (for crafting)

| Material | Used For |
|----------|----------|
| metal_ingot | Structural components, tools, modules |
| iron_ingot | Armor, heavy construction |
| gold_ingot | Electronics, advanced modules |
| crystal_lens | Scanners, shields, optics |
| glass_sheet | Windows, displays, solar panels |
| ite_powder | Concrete, filler material |
| nutrient_paste | Life support fuel, biogenerator fuel |

## Existing Code
- `NF::CraftingRecipe` — recipe system exists (G16)
- `NF::CraftingQueue` — FIFO processing queue exists
- `NF::CraftingSystem` — level-gated crafting exists
- `NF::PlayerInventory` — stacking inventory exists (G17)

## Missing Code
- `CentrifugeSystem` — conversion processing (NOT IMPLEMENTED)
- Raw material yield from voxel mining link
- Processing time/power cost enforcement
- Centrifuge UI panel

## Schema File
→ `Schemas/recipe.schema.json`
