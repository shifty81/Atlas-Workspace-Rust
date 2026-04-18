# Atlas Projects

This directory contains Atlas Engine game projects. Each project is a self-contained
directory with a `.atlas` manifest file that defines its structure.

## Available Projects

| Project | Description |
|---------|-------------|
| `eveoffline/` | PVE space simulator — the reference implementation |
| `atlas-sample/` | Minimal sample project for learning |

## Creating a New Project

1. Create a new directory under `projects/`
2. Add a `.atlas` manifest file (see `schemas/atlas.project.v1.json`)
3. Create required subdirectories: `worlds/`, `assets/`, `data/`
4. Add WorldGraph files to `worlds/`
5. Open the `.atlas` file in Atlas Editor

## Project Structure (Required)

```
my-project/
├── project.atlas          # Project manifest (required)
├── worlds/                # WorldGraph files
├── strategy/              # StrategyGraph files (optional)
├── data/                  # Game content
├── assets/                # Models, textures, audio
└── ai/                    # AI rules and knowledge graphs (optional)
```

See [PROJECT_GUIDELINES.md](../docs/PROJECT_GUIDELINES.md) for complete rules.
