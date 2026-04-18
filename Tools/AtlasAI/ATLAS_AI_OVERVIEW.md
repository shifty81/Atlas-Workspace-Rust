# AtlasAI — Atlas Intelligence Subsystem

**Formerly:** ArbiterAI / Arbiter / SwissAgent (all consolidated here)

## What is AtlasAI?

AtlasAI is the unified AI layer for the NovaForge + Atlas engine suite. It replaces and consolidates:

| Old Name     | Old Repo                  | New Name / Location         |
|--------------|---------------------------|-----------------------------|
| ArbiterAI    | shifty81/ArbiterAI        | Tools/AtlasAI               |
| Arbiter      | shifty81/Arbiter          | Tools/AtlasAI               |
| SwissAgent   | shifty81/SwissAgent       | Tools/AtlasAI/Atlas_SwissAgent |

## Subsystem Modules

- **Atlas_Arbiter** — Core AI decision engine (C# + Python backend)
- **Atlas_SwissAgent** — Multi-tool agent runner (Python + JS frontend)
- **Atlas_NPC** — In-game NPC behaviour (C++ engine integration, planned)
- **Atlas_Pathfinding** — Navigation mesh + flow field AI (C++, planned)
- **Atlas_Dialogue** — Dialogue tree + LLM-assisted NPC speech (planned)

## Naming Canon

All AI features, classes, and modules follow the prefix `Atlas_` or namespace `AtlasAI::`.

Examples:
- `AtlasAI::Arbiter`
- `AtlasAI::SwissAgent`
- `Atlas_NPCController`
- `Atlas_PathfindingSystem`

## Integration Points

- Engine: `Source/AI/` (C++ runtime AI)
- Tools: `Tools/AtlasAI/` (Python/JS/C# tooling)
- Docs: `Docs/AI/`

## Status

| Module              | Status        |
|---------------------|---------------|
| Atlas_Arbiter       | Migrating     |
| Atlas_SwissAgent    | Migrating     |
| Atlas_NPC           | Planned       |
| Atlas_Pathfinding   | Planned       |
| Atlas_Dialogue      | Planned       |
