# Game Identity

## Genre
3D FPS Survival Sandbox

## Core Layers
- **Voxel** = simulation truth (world state, destruction, construction)
- **Low-poly** = visual wrapper (rendering, aesthetics)
- **R.I.G.** = player progression platform (all capability flows through it)
- **Ships** = modular voxel-built systems (construction, combat, transport)
- **Editor** = runtime parity (Atlas Workspace mirrors game systems)

## Pillars
1. R.I.G.-centric progression — no system bypasses the backpack
2. Modular voxel construction — 32³ units, snap-grid assembly
3. Physical interaction only — no abstract UI without hardware
4. Editor/runtime parity — tooling validates what runtime enforces

## Non-Negotiables
- No system bypassing R.I.G.
- No UI without helmet module installed
- No advanced interaction without interface port
- No resource use without processing (centrifuge gate)
- Voxel layer is authoritative (Config: `voxelAuthoritative: true`)

## Code References
- `NF::RigState` — health, energy, oxygen, stamina
- `NF::EVAState` — suit integrity, oxygen supply, jetpack fuel
- `NF::SurvivalStatus` — radiation, temperature, vacuum, fire
- `NF::HUDState` — crosshair, notifications, display elements
- `NF::VoxelType` — Air, Stone, Dirt, Grass, Metal, Glass, Water, Ore_Iron, Ore_Gold, Ore_Crystal
- `NF::Chunk` — 16³ voxel array with mesh/collision dirty flags
