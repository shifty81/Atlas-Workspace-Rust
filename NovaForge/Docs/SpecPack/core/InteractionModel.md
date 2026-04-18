# Interaction Model

## Core Rule
All player interaction is **physical**. There is no abstract UI without hardware.

## Interaction Tiers

| Tier | Name | Requirement | Examples |
|------|------|-------------|----------|
| 0 | **Passive** | None | Walking, looking, falling |
| 1 | **Physical** | Tool equipped | Mining, hitting, grabbing |
| 2 | **Linked** | Interface port installed | Vehicle control, terminal access |
| 3 | **Secured** | Port + breach success | Locked systems, enemy ships |

## Hardware Requirements
| Capability | Required Hardware |
|-----------|------------------|
| See HUD | Helmet module |
| Use tools | Tool mount slot |
| Control vehicles | Interface port (arm module) |
| Access terminals | Interface port |
| Override locks | Interface port + breach minigame |
| Fleet command | Neural link (Tier 3 R.I.G.) |
| Advanced scanning | Scanner module |
| Drone control | Drone controller module |

## Interaction Contract Schema
```json
{
  "interaction_type": "passive | physical | linked | secured",
  "requires_port": true/false,
  "requires_helmet": true/false,
  "requires_module": "module_id or null",
  "breach_difficulty": 0-10 (0 = no breach needed)
}
```

## Code References
- `NF::RigState` — tracks equipped modules
- `NF::EVAState` — suit and mobility state
- Interface port: **NOT YET IMPLEMENTED** — see InterfacePort spec
- Breach minigame: **DOCUMENTED** but no dedicated system class

## Schema File
→ `Schemas/interaction.schema.json`
