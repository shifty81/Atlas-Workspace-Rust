# Milestones — Contract Delivery Schedule

## Milestone 1: Resource Economy Foundation
**Deliverables:**
- [ ] `Schemas/material.schema.json` ✅ Created
- [ ] `Schemas/recipe.schema.json` ✅ Created
- [ ] `Schemas/centrifuge.schema.json` ✅ Created
- [ ] `VoxelMaterialTable` C++ struct (maps VoxelType → material properties)
- [ ] `CentrifugeSystem` C++ class (processing queue, state machine)
- [ ] Centrifuge recipe data files (`Data/Recipes/`)
- [ ] 10+ tests for centrifuge + material systems

**Unlocks:** All crafting, construction, R.I.G. upgrades

## Milestone 2: Physical Interaction Layer
**Deliverables:**
- [ ] `Schemas/interaction.schema.json` ✅ Created
- [ ] `InterfacePort` C++ class (link state machine)
- [ ] `BreachMinigame` C++ class (grid game system)
- [ ] Interaction validation against R.I.G. state
- [ ] 8+ tests for interface port + breach

**Unlocks:** Vehicle control, terminal access, hacking

## Milestone 3: Subsurface Simulation
**Deliverables:**
- [ ] `SandPhysicsSystem` C++ class (collapse simulation)
- [ ] `StructuralIntegrity` system (load-bearing calculation)
- [ ] Elevator state machine
- [ ] Sand collapse rules in material table
- [ ] 6+ tests for sand physics + structural integrity

**Unlocks:** Subsurface escape loop, mining depth

## Milestone 4: R.I.G. Contract Enforcement
**Deliverables:**
- [ ] `Schemas/rig.schema.json` ✅ Created
- [ ] R.I.G. slot validation logic
- [ ] Module dependency graph
- [ ] Tier upgrade cost table
- [ ] Power budget enforcement per tier
- [ ] 8+ tests for R.I.G. validation

**Unlocks:** Progression enforcement, editor validation

## Milestone 5: Workspace Completion
**Deliverables:**
- [ ] PropertyGrid widget
- [ ] TreeView widget
- [ ] TableView widget
- [ ] Layout persistence
- [ ] Viewport host contract
- [ ] Notification severity rules
- [ ] 12+ tests for new widgets

**Unlocks:** Usable editor for game development

## Milestone 6: AI + Environment
**Deliverables:**
- [ ] R.I.G. AI event bus
- [ ] Oxygen simulation
- [ ] Environmental hazard zones
- [ ] Injury persistence
- [ ] 10+ tests

**Unlocks:** Survival gameplay, AI assistance
