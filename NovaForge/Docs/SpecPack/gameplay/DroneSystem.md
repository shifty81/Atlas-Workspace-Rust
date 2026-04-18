# Drone System Specification

## Overview
Drones are autonomous or semi-autonomous units deployed from the R.I.G.
drone controller module. They extend the player's reach and capabilities.

## Existing Implementation ✅
- `drone_system.h/cpp` — main drone system
- `drone_logistics_system.h/cpp` — logistics drones
- `salvage_drone_system.h/cpp` — salvage drones
- `Data/Modules/drones.json` — drone definitions
- Tests: `test_drone_system.cpp`, `test_drone_logistics_system.cpp`, `test_salvage_drone_system.cpp`

## Drone Types

| Type | Function | Range | Power Draw |
|------|----------|-------|-----------|
| Scout | Area scanning, threat detection | 500m | 2 kW |
| Mining | Autonomous resource extraction | 100m | 4 kW |
| Logistics | Item transport between locations | 200m | 3 kW |
| Salvage | Wreck component recovery | 150m | 3 kW |
| Combat | Defensive fire support | 300m | 6 kW |
| Repair | Hull/module repair assistance | 50m | 5 kW |

## Command System
- Direct command: player selects drone, assigns target
- Area command: player designates zone, drones auto-task
- AI-assisted: R.I.G. AI assigns tasks based on context
- Recall: all drones return to player

## Limits
- Max active drones: R.I.G. tier dependent (Tier 2: 2, Tier 3: 4, Tier 4: 8)
- Drone controller module required
- Each drone consumes power from R.I.G. budget

## Missing Contracts
- Drone behavior state machine
- Command priority system
- Drone-to-drone coordination
- Drone damage/repair model
- Visual representation specs
