# Atlas Project Guidelines

This document defines the mandatory rules for all projects built on Atlas Engine.
Violations will result in project load failure.

Atlas is an engine-first platform. Projects are data- and module-driven.
No project is allowed to modify or bypass engine systems.

---

## 1. Project Structure (Mandatory)

Every Atlas project MUST follow this structure:

```
project_root/
├── project.atlas          # Project manifest (exactly one)
├── worlds/                # WorldGraph files only
├── strategy/              # StrategyGraph files only (optional)
├── ai/                    # AI rules and knowledge graphs (optional)
├── assets/                # Models, textures, audio
├── data/                  # Game content (JSON)
└── config/                # Configuration files (optional)
```

Unknown top-level folders are ignored but discouraged.

---

## 2. project.atlas Rules

- Exactly one `.atlas` file per project
- Must be located at project root
- Must declare:
  - `schema` (must be `atlas.project.v1`)
  - `name` (non-empty)
  - `version` (semver format)
- Optional: `description`, `modules`, `runtime`, `assets`
- The `.atlas` file is the single source of truth

See `schemas/atlas.project.v1.json` for the full specification.

---

## 3. WorldGraphs

- Stored ONLY in `worlds/`
- File extension: `.worldgraph`
- Must be deterministic
- Must conform to `schemas/atlas.worldgraph.v1.json`
- Must not:
  - Spawn entities
  - Reference ECS
  - Access rendering APIs
  - Read/write files

**WorldGraphs produce data, not behavior.**

---

## 4. StrategyGraphs

- Stored ONLY in `strategy/`
- File extension: `.strategygraph`
- Influence decision-making ONLY
- Must conform to `schemas/atlas.strategygraph.v1.json`
- Must not:
  - Affect physics
  - Modify world geometry
  - Create entities

**StrategyGraphs are advisory systems.**

---

## 5. AI Systems

Projects may define:
- AI rules (JSON)
- AI knowledge graphs
- AI graph diffs (for editor-assisted authoring)

Projects may NOT:
- Execute engine code from AI output
- Modify graphs without validation
- Bypass editor confirmation

**AI is an author, not an authority.**

---

## 6. Determinism Requirements

All project logic MUST be:
- Deterministic
- Seedable
- Replay-safe

Any non-deterministic system must be explicitly isolated and must not
affect the simulation state.

**Same project + same seed + same inputs = same outputs (always).**

---

## 7. Validation & CI

Projects MUST pass:
- `AtlasRuntime --project <file>.atlas --validate-only`
- Graph compilation (all WorldGraphs and StrategyGraphs)
- Determinism checks (if multiplayer enabled)

Failing validation blocks execution.

---

## 8. Plugin Rules

Plugins extend engine capability, not project content.
Projects may use plugins but must not:
- Include plugin source code
- Depend on non-deterministic plugins
- Bypass the plugin validation system

See the engine plugin documentation for plugin development rules.

---

## 9. Modding Rules

Mods are data overlays applied on top of projects.
Mods may:
- Change graph parameters
- Replace assets
- Add new content (worlds, factions, etc.)

Mods may NOT:
- Include DLLs or executables
- Modify engine behavior
- Introduce non-deterministic systems

---

## 10. Load Order

```
Atlas Engine
  ↓
Plugins
  ↓
Project (.atlas manifest)
  ↓
Mods (optional data overlays)
```

Mods can override project data.
Plugins cannot override engine behavior.

---

## 11. Breaking These Rules

If you think you need to break a rule:
- You probably need a new engine feature
- Open an engine issue instead

These rules exist to protect long-term scalability.

---

## Related Documents

- [Atlas Engine Repository](https://github.com/shifty81/Atlas) — Engine architecture and documentation
- [Atlas Naming Conventions](https://github.com/shifty81/Atlas/blob/main/docs/ATLAS_NAMING_CONVENTIONS.md) — Code style rules
- [Atlas Simulation Philosophy](https://github.com/shifty81/Atlas/blob/main/docs/ATLAS_SIMULATION_PHILOSOPHY.md) — Simulation design
