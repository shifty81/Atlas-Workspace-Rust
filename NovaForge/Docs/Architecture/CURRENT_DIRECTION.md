# NovaForge — Current Direction

**Last updated: 2026-04**

## What This Repo Is
NovaForge is a voxel-first, FPS-primary, modular game engine and editor monorepo.
It is built to host a single shipping game title: **NovaForge** (working title).

## What This Repo Is Not
- It is not a general-purpose engine SDK
- It is not a multi-game platform
- It is not an AtlasSuite migration target (that era is closed)
- It does not and will never use ImGui

## Core Architecture Rules (Hard Locks)
1. **Voxel-authoritative**: Voxel layer owns damage, mining, structure, mass
2. **Low-poly overlay**: Low-poly meshes are visual shells over voxel truth
3. **FPS first**: Moment-to-moment gameplay is first-person
4. **Fleet secondary**: Fleet command unlocks at >5 fleet members
5. **Custom UI only**: UIRenderer + GDI/GPU backend, no ImGui
6. **R.I.G. spine**: Player progression is gated through R.I.G. upgrades
7. **Single AI broker**: AtlasAI is the only external AI integration point
8. **Hosted project**: NovaForge runs standalone OR inside Atlas Workspace

## Current Phase Status
- Phases 0–9 (Engine): COMPLETE
- Game Phases G1–G17: COMPLETE
- G18–G20: In progress
- Atlas Workspace integration: Contracted (see HOSTED_PROJECT_CONTRACT.md)

## What Atlas Workspace Provides
- Shell / launcher
- AtlasAI broker
- Codex / snippet storage
- Build log routing
- Asset intake
- Account linking (GitHub, Google)

## What NovaForge Provides to Workspace
- Game executables (Editor, Game, Server)
- Content/Data/Schema roots
- Build targets
- Pipeline events
- Project manifest (project.atlas.json)
