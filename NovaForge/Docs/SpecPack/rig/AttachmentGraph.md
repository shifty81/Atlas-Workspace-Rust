# Attachment Graph

## Overview
R.I.G. modules follow a dependency tree — attachments unlock more attachments.
No module can be installed without its prerequisites being met.

## Dependency Rules
1. Backpack platform is always present (root node)
2. Each slot has a type that restricts what modules can be installed
3. Some modules require other modules to be installed first
4. Tier gates prevent installing modules above the current R.I.G. tier

## Attachment Tree
```
Backpack (Root — always present)
├── Life Support (built-in, Tier 0)
│   ├── O2 Recycler (Tier 1)
│   ├── Radiation Filter (Tier 2)
│   └── Multi-Env Filter (Tier 3)
├── Helmet (deployable, Tier 0)
│   ├── Basic HUD (Tier 0)
│   ├── Camera Overlay (Tier 1)
│   ├── Full HUD (Tier 2)
│   └── Neural Link Display (Tier 3)
├── Power Core (built-in, Tier 0)
│   ├── Battery Pack (Tier 1)
│   ├── Solar Panel (Tier 1)
│   ├── Fuel Cell (Tier 2)
│   └── Fusion Core (Tier 3)
├── Utility Slots (Tier 1+)
│   ├── Scanner (Tier 1)
│   ├── Tool Mount (Tier 1)
│   ├── Weapon Mount (Tier 2)
│   └── Drone Controller (Tier 2)
├── Arm Modules (Tier 2+)
│   ├── Wrist Terminal (Tier 2)
│   └── Interface Port (Tier 3) ← CRITICAL for vehicle/terminal control
└── Propulsion (Tier 2+)
    ├── Basic Jetpack (Tier 2)
    ├── Thruster Pack (Tier 2)
    ├── Grav Boots (Tier 2)
    └── EVA Suite (Tier 3)
```

## Validation Contract
When attempting to install module M:
1. `M.required_tier <= rig.current_tier`
2. `M.slot_type` matches available slot
3. `M.power_draw <= rig.power_remaining`
4. All `M.prerequisites` are installed
5. Slot is not occupied

## Missing
- Prerequisite definitions per module
- Upgrade path costs (materials + time)
- Visual attachment point coordinates
