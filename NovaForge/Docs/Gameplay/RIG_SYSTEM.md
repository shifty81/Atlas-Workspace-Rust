# NovaForge — R.I.G. System Specification

## Overview
The R.I.G. (Reactive Interface Gear) is the player's core progression spine.
Every major capability unlock is gated through R.I.G. module upgrades.

## Starter State
- Backpack-first initialization
- Life support is built into the platform (not separate)
- Helmet deploys from the back assembly
- Minimal HUD at initial state (health, oxygen, energy bars only)
- No wrist terminal at start
- No backpack crafting at start

## Module Categories
| Category | Description |
|---|---|
| Utility | Tool attachment, scanner, probe launcher |
| Armor | Physical damage reduction, impact resistance |
| Shield | Energy barrier, recharge rate |
| Life Support | O2 recycling, radiation filter, thermal regulation |
| Propulsion | Jetpack, thruster pack, grav boots |
| Interface | Wrist terminal, HUD expansion, neural link |

## Progression Path
```
Tier 0 (Start)
  ├── Backpack platform (life support integrated)
  ├── Basic armor plating
  └── Minimal HUD (health/O2/energy)

Tier 1 (Early unlock)
  ├── Utility slot (scanner / tool mount)
  ├── Shield module (basic)
  └── Helmet camera overlay

Tier 2 (Mid unlock)
  ├── Wrist terminal (mission log, map, crafting shortcuts)
  ├── Backpack crafting station
  ├── Propulsion (basic jetpack)
  └── Full HUD (objectives, contacts, notifications)

Tier 3 (Late unlock)
  ├── Advanced shields (reactive/adaptive)
  ├── Advanced life support (multi-environment)
  ├── Full propulsion (EVA thruster suite)
  └── Neural link (fleet command interface)
```

## HUD Unlock Rule
- HUD elements are HIDDEN until the corresponding R.I.G. module is installed
- Fleet panel only appears when Neural Link is installed AND fleet > 5 members
- Notification center appears at Tier 1

## Helmet Deploy Behavior
- Starts retracted into back assembly
- Deploys automatically when entering vacuum or hostile atmosphere
- Can be manually deployed via R.I.G. interface
- EVA state ties directly to helmet status

## Code Reference
- `NF::RigState` — health, energy, oxygen, stamina, tick, takeDamage, heal
- `NF::EVAState` — suit integrity, oxygen supply, jetpack fuel, velocity
- `NF::SurvivalStatus` — radiation, temperature, inVacuum, onFire
- `NF::HUDState` — crosshair, notifications, addNotification, tick
