# Missing Contracts

## Assessment Date: 2026-04-05

## What Is a Contract?
A contract is an enforceable definition: JSON schema + C++ type + validation logic + tests.
Without contracts, systems describe behavior but cannot enforce it.

---

## Tier 1 — Implementation Blockers

### 1. Voxel Material Table
- **Schema:** `Schemas/material.schema.json` ✅ Created
- **C++ Type Needed:** `VoxelMaterialTable` mapping VoxelType → density, hardness, isLoose, yield
- **Validation:** Editor must verify material properties before world generation
- **Impact:** Blocks mining yields, collapse rules, construction materials

### 2. Centrifuge Processing System
- **Schema:** `Schemas/recipe.schema.json` ✅, `Schemas/centrifuge.schema.json` ✅ Created
- **C++ Type Needed:** `CentrifugeSystem` with job queue, state machine, power consumption
- **Validation:** Recipe inputs must match inventory, power must be available
- **Impact:** Blocks entire resource economy

### 3. Interface Port
- **Schema:** `Schemas/interaction.schema.json` ✅ Created
- **C++ Type Needed:** `InterfacePort` with link state machine
- **Validation:** Port tier must match target requirements
- **Impact:** Blocks vehicle control, terminal access, breach mechanic

### 4. Sand/Collapse Physics
- **Schema:** Material `can_collapse` + `is_loose` flags ✅ In material schema
- **C++ Type Needed:** `SandPhysicsSystem` with neighbor evaluation, cascade
- **Validation:** Structural integrity check after each voxel removal
- **Impact:** Blocks subsurface escape loop, mining danger

### 5. R.I.G. Slot Validation
- **Schema:** `Schemas/rig.schema.json` ✅ Created
- **C++ Type Needed:** Slot validation in `rig_system` (accepts, tier, power)
- **Validation:** Module install must check slot type, tier, power budget
- **Impact:** Blocks progression enforcement

---

## Tier 2 — Gameplay Depth

### 6. Power Consumption Math
- **Schema:** `Schemas/power.schema.json` ✅ Created
- **Missing:** Fuel consumption rates, load shedding algorithm values
- **Impact:** No survival pressure from power management

### 7. R.I.G. Upgrade Costs
- **Missing:** Material costs per tier upgrade, time requirements
- **Impact:** No concrete progression pacing

### 8. Breach Minigame Rules
- **Missing:** Grid generation algorithm, ICE placement, difficulty curve
- **Impact:** No hacking mechanic

### 9. Injury Persistence
- **Missing:** Injury severity levels, treatment requirements, persistence across save/load
- **Impact:** StatusEffects are temporary only — no lasting consequences

### 10. Environmental Hazard Values
- **Missing:** Oxygen consumption rates, temperature damage, radiation DOT values
- **Impact:** Survival is conceptual, not balanced

---

## Tier 3 — Workspace Contracts

### 11. Viewport Host Contract
- **Missing:** 3D rendering surface interface, camera control API
- **Impact:** No standardized viewport for editor

### 12. PropertyGrid Contract
- **Missing:** Property editing widget for entity/component inspection
- **Impact:** Editor cannot edit game objects

### 13. Layout Persistence Contract
- **Missing:** Panel arrangement save/load, workspace state
- **Impact:** Editor layout resets each session

### 14. File Intake Pipeline
- **Missing:** Drag-and-drop import workflow, format detection
- **Impact:** No asset import from external tools

---

## Contract Implementation Template
For each missing contract, implementation requires:
1. JSON schema in `Schemas/` (validation definition)
2. C++ types in `Source/Game/include/NF/Game/Game.h` (runtime struct/class)
3. Tests in `Tests/Game/test_game.cpp` (behavior verification)
4. Data files in `Data/` (concrete values)
5. Editor validation (schema check in tooling)

## Priority Order
```
1. Centrifuge → unlocks economy
2. Material Table → unlocks mining/collapse
3. Interface Port → unlocks interaction
4. R.I.G. Slots → unlocks progression
5. Sand Physics → unlocks subsurface
6. Power Math → unlocks survival
```
