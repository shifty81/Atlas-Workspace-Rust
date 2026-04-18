# NovaForge — Hosted Project Contract

This document defines the interface between NovaForge and Atlas Workspace.

## Project Manifest
Location: `Project/project.atlas.json`
This file is the authoritative contract for hosted loading.

## Entrypoints
| Role | Target |
|---|---|
| Editor | NovaForgeEditor |
| Game | NovaForgeGame |
| Server | NovaForgeServer |

## Content Roots
| Role | Path |
|---|---|
| Content | Content/ |
| Data | Data/ |
| Schemas | Schemas/ |
| Config | Config/ |
| Logs | Logs/ |
| Codex | Codex/ |
| Asset Intake | Content/Incoming/ |

## Build Contract
- Build system: CMake
- Standalone mode: `cmake --preset debug` (builds all)
- Hosted mode: `cmake --preset debug -DNF_HOSTED=ON` (Workspace controls targets)
- Test preset: `ctest --preset debug`

## Pipeline Events
NovaForge emits pipeline events to `.novaforge/pipeline/` (NF::Pipeline module).
Atlas Workspace watches this directory for change events.

## Logger Contract
Build and runtime errors are written to `Logs/build.logger`.
Format: `[LEVEL] [MODULE] [MESSAGE]`
Atlas Workspace routes `.logger` files to AtlasAI for diagnostics.

## Validation
Run `Scripts/validate_project.sh` (or `Scripts/validate_project.bat` on Windows)
to verify this contract is satisfied before any hosted deployment.
