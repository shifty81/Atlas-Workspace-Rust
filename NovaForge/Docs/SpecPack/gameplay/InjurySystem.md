# Injury System Specification

## Overview
Injuries represent persistent damage states that degrade player capabilities
until treated. They are distinct from HP damage.

## Injury Types

| Injury | Cause | Effect | Treatment |
|--------|-------|--------|-----------|
| Fracture | Fall damage, impact | -50% move speed | Medical module or medkit |
| Burn | Fire, explosion | -30% max health | Medical module or burn gel |
| Radiation sickness | Radiation exposure | -20% stamina regen | Radiation purge (medical) |
| Oxygen toxicity | O2 system malfunction | -40% vision clarity | Time + rest |
| Concussion | Head impact | -50% aim stability | Time + rest |
| Laceration | Sharp debris, combat | HP bleed (1/s) | Bandage or medical module |
| Hypothermia | Cold exposure | -30% move speed, -20% stamina | Heat source + time |
| Decompression | Vacuum exposure | -50% max health, -30% stamina | Medical module |

## Existing Code
- `NF::StatusEffectType×8`, `NF::AilmentStack`, `NF::StatusEffectSystem` — status effects (G18)
- These handle temporary effects but not persistent injury states

## Missing Contracts
- Injury persistence (survives save/load)
- Treatment requirements per injury
- Severity levels (minor/major/critical)
- Stacking rules (max injuries before incapacitation)
- Medical module capability requirements per injury type
