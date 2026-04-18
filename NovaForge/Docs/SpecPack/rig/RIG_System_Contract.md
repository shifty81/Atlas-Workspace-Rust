# R.I.G. System Contract

## Overview
The R.I.G. (Reactive Interface Gear) is the player's core progression spine.
Every capability unlock is gated through R.I.G. module upgrades.
The backpack is the root authority — all other systems depend on it.

## Existing Implementation
- `NF::RigState` — health, energy, oxygen, stamina, tick, takeDamage, heal
- `rig_system.h/cpp` — R.I.G. core system
- `rig_link_system.h/cpp` — ship port linking with stat bonuses
- `rig_locker_system.h` — R.I.G. storage management
- `visual_rig_system.h/cpp` — R.I.G. visual representation
- `Docs/Gameplay/RIG_SYSTEM.md` — tier progression spec

## Tier Progression (from existing spec)

| Tier | Unlocks | Power Budget |
|------|---------|-------------|
| 0 | Life support, basic armor, minimal HUD | 5 kW |
| 1 | Utility slot, basic shield, camera overlay | 15 kW |
| 2 | Wrist terminal, crafting station, jetpack, full HUD | 30 kW |
| 3 | Advanced shields, multi-env life support, EVA suite, neural link | 60 kW |
| 4+ | Experimental systems, fleet command | 100+ kW |

## Slot Schema (MISSING — needed)
```json
{
  "tier": 2,
  "slots": [
    { "id": "backpack_core", "type": "core", "locked": true },
    { "id": "helmet", "type": "interface", "accepts": ["hud", "camera", "scanner"] },
    { "id": "chest_left", "type": "armor", "accepts": ["armor", "shield"] },
    { "id": "chest_right", "type": "utility", "accepts": ["tool_mount", "weapon_mount"] },
    { "id": "arm_left", "type": "interface", "accepts": ["wrist_terminal", "interface_port"] },
    { "id": "arm_right", "type": "utility", "accepts": ["tool_mount", "interface_port"] },
    { "id": "back_upper", "type": "propulsion", "accepts": ["jetpack", "thruster_pack"] },
    { "id": "back_lower", "type": "power", "accepts": ["battery", "solar_panel", "reactor"] }
  ]
}
```

## Missing Contracts
- Slot validation rules (which modules fit which slots)
- Power budget per tier (hard cap enforcement)
- Module dependency graph
- Upgrade cost table
- Tier unlock conditions

## Schema File
→ `Schemas/rig.schema.json`
