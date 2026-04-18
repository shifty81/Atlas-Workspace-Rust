# Archive: shifty81/AtlasForge-EveOffline

**Archived:** 2025-07-17
**Source:** https://github.com/shifty81/AtlasForge-EveOffline
**Source SHA:** `56dd9f44e58b2a67bce28adb70a607ef037d4d0d`
**Merge Phase:** Phase 10
**Status:** ✅ Done

## Description

EVEOFFLINE — a PVE-focused space simulator (2–20 players) built with C++20/OpenGL
on the Atlas Engine. Full game project with client, dedicated server, ECS systems,
JSON-driven game data (102+ ships, 159+ modules, 137 skills), AI economy, and
modding support. Networking uses client-server + P2P with lockstep/rollback.

## Source Repository Structure

```
AtlasForge-EveOffline/
├── README.md                  ← Game project overview
├── PROJECT_CONTEXT.md         ← Vision, pillars, design rules
├── CMakeLists.txt             ← Root build (C++20)
├── Dockerfile                 ← Server container build
├── Makefile                   ← Dev shortcuts
├── .dockerignore
├── cpp_client/                ← OpenGL game client (~30K lines CMakeLists)
│   └── CMakeLists.txt
├── cpp_server/                ← Dedicated server (up to ~75 players)
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── src/
│   │   ├── main.cpp, server.cpp, game_session.cpp
│   │   ├── auth/, config/, data/, ecs/
│   │   ├── network/, systems/, ui/, utils/
│   │   └── test_systems.cpp (~554K)
│   ├── include/
│   │   ├── server.h, game_session.h
│   │   ├── auth/, components/, config/, data/
│   │   ├── ecs/, network/, systems/, ui/, utils/
│   └── config/, docs/, build.sh, build.bat, run_tests.sh
├── data/                      ← JSON game content (moddable)
│   ├── ships/ (102+), modules/ (159+), skills/ (137)
│   ├── universe/, missions/, market/, npcs/
│   ├── industry/, fleet/, exploration/, wormholes/
│   ├── asteroid_fields/, character_creation/, contracts/
│   ├── corporations/, planetary_interaction/, security/, ui/
│   └── gas_types.json, ice_types.json
├── docs/                      ← 55+ documentation files
│   ├── (21 root-level .md files)
│   ├── atlas-ui/ (2 files)
│   ├── cpp_client/ (24 files)
│   ├── development/ (6 files)
│   ├── design/, features/, game_mechanics/
│   ├── getting-started/, guides/, testing/
│   └── images/
├── projects/eveoffline/       ← Atlas Engine project files
├── scripts/                   ← Build scripts (7 files)
├── tools/                     ← Modding tools
│   ├── BlenderSpaceshipGenerator/
│   ├── create_ship.py, validate_json.py
│   └── README.md
└── .github/                   ← CI/CD workflows
```

## What Was Extracted

### Docs → `Docs/AtlasForge-EveOffline/` (55 files)

All documentation migrated to the canonical Docs location:
- `README.md` — Game project overview
- `PROJECT_CONTEXT.md` — Vision and design pillars
- `docs/` — Full docs tree (root, atlas-ui, cpp_client, development)
  - Includes roadmap, tutorials, architecture, modding guide
  - Client docs: rendering, networking phases, UI, audio, model loading
  - Development docs: build system, CI/CD, quickstart guides

### Code Snapshots → `Archive/_AtlasForge-EveOffline/code_snapshots/`

Representative code files archived for reference:
- `cpp_server/main.cpp` — Server entry point
- `cpp_server/server.cpp` — Server implementation
- `cpp_server/server.h` — Server header
- `cpp_server/game_session.h` — Game session management header
- `cpp_server/CMakeLists.txt` — Server build configuration
- `cpp_server/README.md` — Server README
- `cpp_client/CMakeLists.txt` — Client build configuration (30K)

### Build Config → `Archive/_AtlasForge-EveOffline/build_config/`

- `CMakeLists.txt` — Root CMake (C++20, client + server targets)
- `Dockerfile` — Multi-stage server container build
- `Makefile` — Development shortcuts (build, test, validate)
- `.dockerignore`

## What Was NOT Extracted (and why)

| Content | Reason |
|---------|--------|
| `data/` (JSON game data, 20+ dirs) | Voluminous game content; lives in source repo only |
| `cpp_client/` source files | Large codebase; only CMakeLists archived as reference |
| `cpp_server/src/` (full tree) | Key files archived; full tree stays in source repo |
| `cpp_server/test_systems.cpp` (554K) | Too large for archive snapshot |
| `projects/eveoffline/` | Atlas Engine project files; source repo specific |
| `scripts/` (7 build scripts) | Build-system specific; source repo only |
| `tools/` (ship creation, validation) | Modding tools; source repo specific |
| `docs/design/`, `docs/features/`, etc. | Sub-directories not yet individually audited |
| `eveoffline.PNG` (264K) | Binary screenshot |
| `.github/` workflows | CI/CD config; source repo specific |

## Overlap Analysis

| Area | Overlap | Resolution |
|------|---------|------------|
| `Source/` networking code | No existing networking code in monorepo Source/ | Docs archived; source stays in original repo |
| `Docs/AtlasForge-EveOffline/` | Directory did not exist previously | Created new; all docs placed here |
| `tools/BlenderSpaceshipGenerator/` | Exists in source repo; Phase 11 covers this addon separately | Not extracted from this repo |

## Migration Checklist

- [x] Audit source repo contents
- [x] Identify usable code, docs, and assets
- [x] Extract code snapshots to `Archive/_AtlasForge-EveOffline/code_snapshots/`
- [x] Extract build config to `Archive/_AtlasForge-EveOffline/build_config/`
- [x] Merge docs into `Docs/AtlasForge-EveOffline/`
- [x] Update ARCHIVE_SUMMARY.md with full audit
- [x] Mark phase as ✅ Done
