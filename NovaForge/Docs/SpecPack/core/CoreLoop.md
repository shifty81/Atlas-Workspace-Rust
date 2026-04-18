# Core Loop

## Early Game (Fully Defined)
1. Spawn in buried bunker (subsurface start)
2. Excavate sand/debris (voxel mining with basic tool)
3. Process raw materials via centrifuge (resource gate)
4. Upgrade R.I.G. modules (backpack-driven progression)
5. Repair freight elevator (infrastructure milestone)
6. Reach surface (world unlock)

## Mid Game (Partial — needs contracts)
- Build modular systems (32³ voxel units)
- Establish power networks (biomass → solar → hydrogen)
- Expand base/ship infrastructure
- Unlock interface port for vehicle/terminal control
- Drone assistance begins

## Late Game (Concept — needs balancing)
- Fleet management (>5 ships, AI-assisted)
- Neural link for fleet command interface
- Cross-system resource networks
- Station construction and defense

## Loop Dependencies
```
Mining → Centrifuge → Materials → R.I.G. Upgrade → New Capabilities
                                  ↓
                          Module Unlock → Slot Fill → System Access
                                                      ↓
                                              Power Grid → Active Systems
```

## Repo Status
- Early game loop: **Designed, partially implemented** (VoxelType, Chunk, RigState exist)
- Centrifuge: **NOT IMPLEMENTED** — blocks resource economy
- Power graph: **EXISTS** as ModulePowerGridSystem (CPU/PG tracking)
- Interface port: **NOT IMPLEMENTED** — blocks vehicle/terminal control
