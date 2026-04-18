# NovaForge Architecture

## Purpose
NovaForge is the standalone game project hosted by [Atlas Suite](https://github.com/shifty81/AtlasToolingSuite).

## Non-Goals
This repository does not contain:
- Atlas Suite shell
- Atlas Suite workspace services
- generic editor host framework
- generic content intake ownership
- generic build orchestration ownership
- generic AI broker ownership
- generic repo migration tooling

## Repo Structure

```text
NovaForge/                    # C++ game source (CMake root)
  CMakeLists.txt
  App/                        # bootstrap and session management
  Client/                     # OpenGL rendering, networking, UI
  Server/                     # game server with ECS and gameplay
  Gameplay/                   # 20+ gameplay subsystems
  World/                      # world generation and environments
  UI/                         # runtime HUD and widgets
  Save/                       # save/load systems
  EditorTools/                # in-game editor panels
  Tools/                      # AI framework, Blender integration, PCG
  Data/                       # game definitions and configs
  Tests/                      # game-level C++ tests

Integrations/
  AtlasSuite/
    Runtime/                  # C# runtime bridge (Builder, Vehicles, Salvage, Constructs)
    Adapter/                  # NovaForge-specific Suite adapters (definition service, level, build command)
    Plugins/                  # project extension plugins (sample salvage plugin)

AtlasAI/
  ProjectAdapters/
    NovaForge/                # AI project adapter (bridge, ingestion, manifest)
```

## Atlas Suite Bridge
NovaForge connects to Atlas Suite via HTTP bridge (`http://127.0.0.1:8005`).
No shared assemblies — all communication is over the bridge protocol.
Atlas Suite must run cleanly without NovaForge present.

## Major Domains
- Core
- World
- Gameplay
- Rig
- Building
- Salvage
- Seasons
- Factions
- AI
- UI
- Client
- Server
- Validation
- Tests

## Current Design Locks
- Voxel layer is authoritative.
- Low-poly wrapper is deferred and later reflects voxel state.
- The player suit platform is the R.I.G.
- The R.I.G. starts as a minimal exo frame with life support.
- Helmet deployment and minimal HUD are part of early progression.
- Seasons are configurable client-side and server-side, with server authority as the normal mode.
