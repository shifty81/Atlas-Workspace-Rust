# Subsurface Escape Loop

## Overview
The player begins buried in a subsurface bunker. The entire early game is a
structured escape sequence that teaches core mechanics through necessity.

## States
```
START_BUNKER → EXCAVATION → PROCESSING → REPAIR → ASCENT → SURFACE_UNLOCK
```

| State | Trigger | Systems Used |
|-------|---------|-------------|
| START_BUNKER | Game start | R.I.G. init, basic HUD |
| EXCAVATION | First tool use | Voxel mining, sand physics |
| PROCESSING | Centrifuge built | Resource conversion |
| REPAIR | Materials acquired | Construction, power |
| ASCENT | Elevator functional | Infrastructure, power grid |
| SURFACE_UNLOCK | Surface reached | World expansion, mid-game |

## Requirements
- Centrifuge must be built before meaningful progression
- Power system must be active before elevator repair
- Shaft must be structurally sound (collapse prevention)
- Elevator mechanism must be fully repaired

## Fail Conditions
- **Shaft collapse** — unsupported sand falls, blocks progress
- **Power failure** — systems offline, must repair or find fuel
- **Oxygen depletion** — R.I.G. life support drained
- **Structural failure** — poorly-built supports break under load

## Missing Systems (Blockers)
- `CentrifugeSystem` — resource conversion (NOT IMPLEMENTED)
- `SandPhysicsSystem` — collapse simulation (NOT IMPLEMENTED)
- `ElevatorStateMachine` — repair/operation states (NOT IMPLEMENTED)
- `StructuralIntegrity` — load-bearing calculation (NOT IMPLEMENTED)

## Existing Systems (Usable)
- `NF::VoxelType` — material types for mining
- `NF::Chunk` — voxel world structure
- `NF::RigState` — player stats tracking
- `NF::SurvivalStatus` — environmental hazards
- `ModulePowerGridSystem` — power budget management
