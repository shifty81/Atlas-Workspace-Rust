# Master Roadmap — Contract-First Implementation

## Truth
Design: ~85% complete. Architecture: correct. Implementation readiness: ~40%.
**Blocked by missing enforceable system contracts.**

## Tier 1 — Immediate (Unlocks Everything)
These contracts block all downstream implementation.

| Contract | Status | Unlocks |
|----------|--------|---------|
| Centrifuge System | ❌ Missing | Resource economy, progression |
| Interface Port | ❌ Missing | Vehicle control, terminal access |
| Voxel Material Table | ❌ Missing | Mining yields, collapse rules |
| Sand/Collapse Physics | ❌ Missing | Subsurface escape loop |
| R.I.G. Slot Schema | ⚠️ Partial | Module installation validation |
| Power Graph Math | ⚠️ Partial (ModulePowerGridSystem exists) | Load shedding, fuel |

### Dependency Chain
```
Voxel Material → Mining Yields → Centrifuge Recipes → Processed Materials
                                                        ↓
                                                  Construction → Base Building
                                                        ↓
                                                  R.I.G. Upgrades → New Capabilities
                                                        ↓
                                                  Interface Port → Vehicle/Terminal Control
```

## Tier 2 — Core Gameplay
| System | Status | Depends On |
|--------|--------|-----------|
| Breach Minigame | Documented, not implemented | Interface Port |
| Injury System | Partially exists (StatusEffects) | R.I.G. medical module |
| Environment Sim | Partial (EVA/survival) | Voxel materials, power |
| HUD Contract | ✅ Exists (HUDState) | Helmet module rules |
| Scan & Mapping | ✅ Exists (scanner systems) | Scanner module |

## Tier 3 — Extended Systems
| System | Status | Depends On |
|--------|--------|-----------|
| Drone System | ✅ Implemented (3 subsystems) | Drone controller module |
| Vehicles | Partial (ships exist) | Interface Port |
| Fleet Layer | ✅ Implemented (AI captains) | Neural link module |
| Medical System | Concept | Injury system, R.I.G. Tier 3 |

## Workspace Milestones (Tooling-First Priority)
Per SpecRollup/08_ROADMAP_PRIORITY_RESET.md, tooling comes before game features.

| Phase | Goal | Status |
|-------|------|--------|
| WS-1 | AtlasUI foundation complete | ✅ Done (widgets, themes, commands) |
| WS-2 | PropertyGrid + Tree + Table | ❌ Missing |
| WS-3 | Layout persistence | ❌ Missing |
| WS-4 | Viewport host contract | ❌ Missing |
| WS-5 | Notification workflow | ⚠️ Partial |
| WS-6 | AtlasAI panel host | ❌ Missing |
| WS-7 | File intake pipeline | ❌ Missing |
| WS-8 | Settings panel | ❌ Missing |

## Implementation Order (Recommended)
1. **Voxel Material + Centrifuge** — unlocks resource economy
2. **Interface Port** — unlocks vehicle/terminal control
3. **Sand Physics** — unlocks subsurface escape loop
4. **PropertyGrid widget** — unlocks editor property editing
5. **Layout Persistence** — unlocks workspace usability
6. **Breach Minigame** — unlocks secured interactions
