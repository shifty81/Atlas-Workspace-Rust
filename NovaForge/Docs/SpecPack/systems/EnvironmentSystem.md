# Environment System Specification

## Overview
Environmental hazards affect the player through the R.I.G. survival systems.
All hazards are physical — they affect the world and the player's equipment.

## Hazard Types

| Hazard | Damage Type | Affected System | Mitigation |
|--------|-------------|----------------|------------|
| Vacuum | Oxygen drain | Life support | Sealed suit, O2 reserves |
| Radiation | Health DOT | Shield, armor | Radiation filter module |
| Extreme heat | Energy drain | Thermal regulation | EnvironFilter module |
| Extreme cold | Stamina drain | Thermal regulation | EnvironFilter module |
| Toxic atmosphere | Health DOT | Life support | EnvironFilter module |
| Storm/debris | Impact damage | Armor, shield | Shelter, shield module |

## Oxygen Simulation
- Base consumption: 1 unit/second
- EVA consumption: 2 units/second (higher exertion)
- Tank capacity: 300 units (5 min base)
- Refill: Life support module + atmosphere or O2 canister
- Depletion: Health damage at 2 HP/second when empty

## Existing Code
- `NF::SurvivalStatus` — radiation, temperature, inVacuum, onFire
- `NF::EVAState` — suit integrity, oxygen supply, jetpack fuel
- `eva_airlock_system.h` — checks suit oxygen before EVA
- `eva_airlock_exit_system.h` — validates suit oxygen, tether range
- Abyssal weather systems exist (abyssal_weather_system.h)

## Missing Systems
- Dedicated oxygen simulation (consumption/refill math)
- Storm generation and damage
- Temperature zones in world
- Atmospheric composition per area
- Environmental hazard visualization

## Schema Needs
- Hazard zone definitions per chunk/area
- Environmental stat modifiers
- Mitigation effectiveness values
