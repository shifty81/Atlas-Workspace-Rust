# Skills & Research Specification

## Overview
Skills improve player effectiveness over time. Research unlocks new blueprints
and capabilities. Both are gated by R.I.G. tier and resource investment.

## Existing Implementation
- `NF::PlayerLevel` — cap 50 (G15)
- `NF::SkillNode`, `NF::SkillTree` — skill progression (G15)
- `NF::ProgressionSystem` — XP and level management (G15)
- `NF::TechCategory×7`, `NF::TechNode`, `NF::TechTree` — tech prereqs (G14)
- Schema: `Schemas/skill.schema.json`
- Data: `Data/Skills/` — 7 skill category files

## Skill Categories (from existing schema)
Combat, Engineering, Science, Leadership, Piloting, Mining, Trading, Stealth

## Missing Contracts
- Skill effect values (what does Mining 5 actually do?)
- Research cost tables (materials + time)
- Skill/tech interaction with R.I.G. tier
- Passive vs active skill effects
- Skill point allocation rules
