# Construction System Specification

## Overview
Construction uses processed materials to build modular structures (32³ voxel units).
Each module snaps to a grid and connects to the power/resource network.

## Construction Flow
```
Select Blueprint → Place Ghost → Supply Materials → Build Timer → Complete
```

## Blueprint Requirements
Each constructable structure requires:
- Material list (processed materials only)
- Power connection point
- Structural attachment point
- Minimum R.I.G. tier

## Modular Grid
- Unit size: 32×32×32 voxels
- Snap alignment: 1-voxel grid
- Rotation: 90° increments only
- Max structure size: engine-limited (chunk loading)

## Construction Types

| Type | Size | Materials | Power | Notes |
|------|------|-----------|-------|-------|
| Wall panel | 1 unit | metal_ingot ×2 | 0 | Passive structural |
| Floor panel | 1 unit | metal_ingot ×2 | 0 | Passive structural |
| Door | 1 unit | metal_ingot ×3, crystal_lens ×1 | 1 kW | Requires interface port |
| Window | 1 unit | glass_sheet ×4 | 0 | Passive, transparent |
| Solar panel | 1 unit | glass_sheet ×2, gold_ingot ×1 | -15 kW | Generator |
| Battery rack | 1 unit | metal_ingot ×4, iron_ingot ×2 | -10 kW (stored) | Storage |
| Centrifuge | 2 units | metal_ingot ×6, iron_ingot ×4 | 3-8 kW | Processing |

## Missing Contracts
- Full blueprint schema
- Construction timer calculations
- Structural load validation
- Power routing through connected modules
- Deconstruction/salvage rules

## Existing Code
- `NF::CraftingSystem` — could be extended for construction
- Grid construction references in exploration_components.h
- ModuleType::PowerNode for grid connectivity
