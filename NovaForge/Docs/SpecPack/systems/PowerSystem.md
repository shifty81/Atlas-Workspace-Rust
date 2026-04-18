# Power System Specification

## Overview
All active systems consume power. Power budget is managed through a graph model
where generators feed consumers through a priority system.

## Existing Implementation
The `ModulePowerGridSystem` exists in the server systems layer:
- Location: `Source/Game/include/NF/Game/Server/systems/module_power_grid_system.h`
- Tracks CPU and Power Grid (PG) budgets
- Online/offline module state management
- Forces excess modules offline when capacity drops

## Power Sources (Progression)

| Source | Tier | Output (kW) | Notes |
|--------|------|-------------|-------|
| Biomass Generator | 0 | 5 | Starter, requires organic fuel |
| Battery Pack | 1 | 10 (stored) | Rechargeable, limited capacity |
| Solar Panel | 1 | 15 | Day-cycle dependent |
| Fuel Cell | 2 | 30 | Requires hydrogen fuel |
| Fusion Reactor | 3 | 100 | Late-game, high tier |

## Power Consumers

| System | Draw (kW) | Priority | Notes |
|--------|-----------|----------|-------|
| Life Support | 2 | 1 (highest) | Always on if power available |
| HUD | 1 | 2 | Requires helmet |
| Scanner | 3 | 3 | Active when scanning |
| Shield | 8 | 4 | Active when enabled |
| Interface Port | 2 | 5 | Active when linked |
| Drone Controller | 5 | 6 | Active when drones deployed |
| Jetpack | 10 | 7 | Active during flight |
| Weapons | varies | 8 | Active during combat |

## Priority Rules
1. When power drops below total demand, lowest priority systems go offline first
2. Life support NEVER goes offline (emergency reserve)
3. Systems shed load in reverse priority order
4. User can manually override priorities

## Missing Contracts
- Power generation math (fuel consumption rates)
- Load balancing algorithm
- Priority override persistence
- Power routing through physical connections (wire/conduit)
- Cascading failure propagation

## Schema File
→ `Schemas/power.schema.json`
