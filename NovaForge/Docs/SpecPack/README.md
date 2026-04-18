# NovaForge Spec Pack

Complete game design contracts, system schemas, C++ stubs, audit results,
and roadmap additions for the NovaForge project.

## Contents

### `/docs/core/` — Core Game Design
- **GameIdentity.md** — Genre, layers, pillars, non-negotiables
- **CoreLoop.md** — Early/mid/late game progression loops
- **SubsurfaceEscapeLoop.md** — Opening sequence state machine
- **InteractionModel.md** — Physical interaction tiers and hardware requirements

### `/docs/systems/` — System Specifications
- **VoxelSystem.md** — Voxel material properties and collapse rules
- **PowerSystem.md** — Power sources, consumers, priority load-shedding
- **ResourceSystem.md** — Mining yields and centrifuge conversion tables
- **CentrifugeSpec.md** — Processing station state machine and tiers
- **ModuleSystem.md** — Module installation contracts
- **ConstructionSystem.md** — Modular building specifications
- **EnvironmentSystem.md** — Environmental hazards and oxygen simulation

### `/docs/rig/` — R.I.G. System
- **RIG_System_Contract.md** — Tier progression, slot schema, power budgets
- **AttachmentGraph.md** — Module dependency tree
- **InterfacePort.md** — Link state machine for vehicle/terminal control
- **RIG_AI_Core.md** — Embedded AI feature flags and event system

### `/docs/gameplay/` — Gameplay Systems
- **BreachMinigame.md** — Hacking grid game specification
- **DroneSystem.md** — Drone types, commands, limits
- **SkillsResearch.md** — Skill categories and research gaps
- **VehiclesInterface.md** — Vehicle control via interface port
- **InjurySystem.md** — Persistent injury types and treatment

### `/docs/workspace/` — Atlas Workspace
- **AtlasWorkspaceShell.md** — Development environment architecture
- **AtlasUI_Framework.md** — Widget completeness and missing components
- **CodexSystem.md** — Knowledge base specification
- **AI_Broker.md** — AtlasAI integration capabilities
- **NotificationSystem.md** — Severity levels and workflow rules

### `/docs/roadmap/` — Roadmap
- **MasterRoadmap.md** — Contract-first implementation priority
- **Milestones.md** — 6 milestone delivery schedule
- **DependencyGraph.md** — System dependency chains

### `/docs/audit/` — Audit Results
- **RepoGapAnalysis.md** — What exists vs what's missing
- **SystemCoverageMatrix.md** — 50-system coverage table
- **MissingContracts.md** — 14 missing contracts by priority tier

## Schemas Added
| Schema | Path | Status |
|--------|------|--------|
| R.I.G. definition | `Schemas/rig.schema.json` | ✅ New |
| Voxel material | `Schemas/material.schema.json` | ✅ New |
| Interaction contract | `Schemas/interaction.schema.json` | ✅ New |
| Power system | `Schemas/power.schema.json` | ✅ New |
| Processing recipe | `Schemas/recipe.schema.json` | ✅ New |
| Centrifuge config | `Schemas/centrifuge.schema.json` | ✅ New |

## C++ Contract Stubs Added
All in `Source/Game/include/NF/Game/Game.h`:
- **SP1**: `VoxelMaterialDef`, `VoxelMaterialTable` — material properties lookup
- **SP2**: `CentrifugeState`, `CentrifugeJob`, `CentrifugeSystem` — resource processing
- **SP3**: `LinkState`, `InterfacePort` — physical interaction state machine
- **SP4**: `CollapseEvent`, `SandPhysicsSystem` — voxel collapse simulation
- **SP5**: `BreachState`, `BreachGrid`, `BreachMinigame` — hacking minigame
- **SP6**: `RigAIEvent`, `RigAIFeatures`, `RigAIAlert`, `RigAICore` — R.I.G. AI system

## Tests Added
24 new Catch2 tests in `Tests/Game/test_game.cpp`:
- SP1: Material properties, table defaults (2 tests)
- SP2: Centrifuge state names, job progress, tier/queue, lifecycle, power stall, capacity (6 tests)
- SP3: Link state names, state machine, failure/retry (3 tests)
- SP4: Collapse detection, simulate step, stone stability (3 tests)
- SP5: Breach state names, lifecycle, timeout, partial, bounds (5 tests)
- SP6: Event names, feature count, event routing, feature gating, enable/disable (5 tests)

**Total: 884 tests, 0 failures**
