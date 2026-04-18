# Vehicles & Interface Contract

## Overview
Vehicles are controllable entities accessed through the interface port.
All vehicle interaction follows the linked interaction model.

## Vehicle Types

| Type | Interface Level | Crew | Function |
|------|----------------|------|----------|
| Freight elevator | Basic | 0 | Vertical transport |
| Mining rover | Basic | 1 | Surface resource extraction |
| Transport shuttle | Standard | 2 | Personnel/cargo transport |
| Fighter ship | Standard | 1 | Combat |
| Capital ship | Advanced | 10+ | Fleet operations |

## Control Flow
```
Approach Vehicle → Interface Port Contact → Link → Enter Control → Drive/Fly
```

## Requirements
- Interface port installed on R.I.G.
- Port tier >= vehicle interface level
- Vehicle powered and functional
- No other player currently controlling

## Missing Systems
- Vehicle interface contract (standardized control API)
- Vehicle damage model
- Vehicle construction (voxel-built ships)
- Vehicle docking system
- Passenger mechanics

## Existing Code
- `NF::ShipClass`, `NF::Ship`, `NF::FlightController` — ship systems (G4)
- `NF::ShipRoom`, `NF::ShipInterior` — interior systems (G8)
- `NF::Formation`, `NF::AICaptain`, `NF::Fleet` — fleet AI (G5)
- Ship database: `Data/Ships/` — 15+ ship class files
