# Atlas Workspace — Deployable AI Tool Packages

> **Source code** lives in `AtlasAI/` (repo root).
> **Deployable packages** live here in `Atlas/Workspace/`.
> **Legacy subsystems** archived to `Archive/Legacy/`.

## Directory Layout

```
Atlas/
└── Workspace/
    ├── AtlasAI/              ← AtlasAI deployable package
    ├── BlenderBridge/        ← Blender integration
    ├── Sessions/             ← Runtime session data (gitignored)
    └── SwissAgent/           ← SwissAgent deployable package (legacy)
```

## Relationship to Other Directories

| Directory | Purpose | Committed |
|-----------|---------|-----------|
| `AtlasAI/` | AI tool **source code** and reference docs | Yes |
| `Atlas/Workspace/` | **Deployable** tool packages | Yes (except Sessions/) |
| `Archive/Legacy/` | **Archived** legacy tools and subsystems | Yes |

## Pipeline Contract

All tools communicate exclusively through `.novaforge/pipeline/`:

- **Input:** Reads `pipeline/changes/*.change.json` events
- **Output:** Writes `pipeline/changes/<timestamp>_atlasai_<type>.change.json`
- **No direct file writes** to source — everything goes through pipeline events

## Usage

```bash
# Run Arbiter from workspace
cd Atlas/Workspace/Arbiter
python arbiter_cli.py --help

# Run SwissAgent from workspace
cd Atlas/Workspace/SwissAgent
python cli.py --help

# Or via Docker
docker build -t atlas-arbiter Atlas/Workspace/Arbiter/
docker build -t atlas-swissagent Atlas/Workspace/SwissAgent/
```

## Build Script Integration

The build scripts (`Scripts/build.sh`, `Scripts/build_win.cmd`) scaffold this
directory as a post-build step, ensuring the workspace is always ready after
a successful C++ build.
