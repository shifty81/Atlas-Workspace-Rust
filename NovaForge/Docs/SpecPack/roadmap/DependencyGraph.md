# Dependency Graph

## Critical Path
```
                    ┌─────────────────┐
                    │ Voxel Material   │
                    │ Table            │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
     ┌────────────┐  ┌──────────┐  ┌──────────────┐
     │ Sand       │  │ Mining   │  │ Structural   │
     │ Physics    │  │ Yields   │  │ Integrity    │
     └─────┬──────┘  └────┬─────┘  └──────┬───────┘
           │               │               │
           ▼               ▼               │
     ┌───────────┐  ┌──────────────┐       │
     │ Subsurface│  │ Centrifuge   │       │
     │ Escape    │  │ System       │       │
     │ Loop      │  └──────┬───────┘       │
     └───────────┘         │               │
                           ▼               ▼
                    ┌──────────────┐ ┌──────────────┐
                    │ Processed    │ │ Construction │
                    │ Materials    │ │ System       │
                    └──────┬───────┘ └──────┬───────┘
                           │               │
              ┌────────────┼───────────────┘
              ▼            ▼
     ┌────────────┐  ┌──────────┐
     │ R.I.G.     │  │ Base     │
     │ Upgrades   │  │ Building │
     └─────┬──────┘  └──────────┘
           │
     ┌─────┼──────────────┐
     ▼     ▼              ▼
┌────────┐ ┌──────────┐ ┌──────────┐
│ Inter- │ │ Advanced │ │ Drone    │
│ face   │ │ Modules  │ │ Control  │
│ Port   │ │          │ │          │
└────┬───┘ └──────────┘ └──────────┘
     │
     ▼
┌────────────┐
│ Vehicle    │
│ Control    │
│ + Breach   │
└────────────┘
```

## System ↔ Schema Dependencies

| System | Required Schema | Status |
|--------|----------------|--------|
| Voxel Mining | material.schema.json | ✅ Created |
| Centrifuge | recipe.schema.json, centrifuge.schema.json | ✅ Created |
| R.I.G. Upgrade | rig.schema.json | ✅ Created |
| Module Install | module.schema.json | ✅ Existed |
| Interaction | interaction.schema.json | ✅ Created |
| Power Grid | power.schema.json | ✅ Created |
| Ship Build | ship.schema.json | ✅ Existed |
| Mission | mission.schema.json | ✅ Existed |
| Skills | skill.schema.json | ✅ Existed |

## Blocking Chains
1. **No material table** → no mining yields → no raw materials → no centrifuge input → no processed materials → no construction → no upgrades
2. **No interface port** → no vehicle control → no terminal access → no mid-game progression
3. **No sand physics** → no collapse → no subsurface tension → no early game danger
4. **No power math** → no load shedding → no survival pressure → no resource management
