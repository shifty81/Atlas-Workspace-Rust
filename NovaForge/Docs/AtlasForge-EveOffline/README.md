<p align="center">
  <img src="eveoffline.PNG" alt="EVEOFFLINE — PVE Space Simulator" width="900"/>
</p>

<h3 align="center">EVEOFFLINE — A PVE-focused space simulator for solo play and small groups (2–20 players)</h3>
<p align="center">
  Built with <b>C++20 / OpenGL</b> · Powered by the <a href="https://github.com/shifty81/Atlas"><b>Atlas Engine</b></a>
</p>

<p align="center">
  <code>Status: Active R&D</code> · <code>Platforms: Linux · macOS · Windows</code> · <code>License: TBD</code>
</p>

---

## What Is This?

**EVEOFFLINE** is a PVE space simulation inspired by EVE Online — rebuilt from scratch
with a custom engine. AI drives the universe: economy, pirates, factions, and fleet behavior
all run whether or not the player is watching.

This repository contains the **game project** — client, server, game data, and project
configuration. The [Atlas Engine](https://github.com/shifty81/Atlas) is developed
separately as a standalone, game-agnostic engine.

---

## 🏗️ Repository Structure

```
EVEOFFLINE/
├── cpp_client/             ← Game client (OpenGL 3D renderer)
├── cpp_server/             ← Dedicated server (up to ~75 players)
├── data/                   ← Moddable game content (JSON)
│   ├── ships/                 102+ ship definitions
│   ├── modules/               159+ module definitions
│   ├── skills/                137 skill definitions
│   ├── universe/              Solar systems, stargates, wormholes
│   ├── missions/              Mission templates (5 levels × 7 types)
│   ├── market/                Economy and pricing
│   ├── npcs/                  NPC pilots, factions, corporations
│   ├── industry/              Mining, manufacturing, PI
│   └── ...
├── projects/
│   └── eveoffline/         ← Atlas Engine project files
│       ├── eveoffline.atlas   Project manifest
│       ├── worlds/            WorldGraph files
│       ├── strategy/          StrategyGraph files
│       ├── conversations/     ConversationGraph files
│       ├── ai/                AI configuration
│       ├── config/            Runtime configuration
│       ├── data/              Project data manifest
│       ├── assets/            Models, textures, audio
│       ├── Code/              Game-specific code
│       ├── Schemas/           Data validation schemas
│       └── Tests/             Project-specific tests
├── tools/                  ← Modding utilities
├── docs/                   ← Documentation
├── scripts/                ← Build scripts
├── .github/workflows/      ← CI/CD pipelines
├── PROJECT_CONTEXT.md      ← Game vision and design pillars
├── CMakeLists.txt          ← Build configuration
├── Dockerfile              ← Server container build
└── Makefile                ← Development shortcuts
```

> **[Project Context →](PROJECT_CONTEXT.md)** · **[Project Guidelines →](docs/PROJECT_GUIDELINES.md)**

---

## 🚀 Quick Start

```bash
# Linux/macOS — install deps and build
sudo apt-get install build-essential cmake libgl1-mesa-dev libglew-dev \
  libglfw3-dev libglm-dev nlohmann-json3-dev libopenal-dev libfreetype-dev

# Build specific targets
./scripts/build_project.sh Release client     # Game client
./scripts/build_project.sh Release server     # Dedicated server
./scripts/build_project.sh Release test       # Build and run all tests
./scripts/build_project.sh Release validate   # Validate project structure

# Or use Make shortcuts
make build-client   # Build game client
make build-server   # Build dedicated server
make test           # Run all tests
make validate       # Validate project structure

# Docker — run dedicated server
docker build -t eveoffline-server .
docker run -p 8765:8765 eveoffline-server
```

---

## 🎮 Game Features

<table>
<tr><td width="50%" valign="top">

**Combat & Movement**
- Module activation, target locking, damage types, EW
- Approach, orbit, keep-at-range, warp, align
- Fleet system with AI or human wingmates

**Ships & Factions**
- 102 ships across frigates → titans
- Tech I + Tech II specializations
- 4 factions: Solari · Veyren · Aurelian · Keldari

</td><td width="50%" valign="top">

**Economy & Industry**
- Mining, manufacturing, market, contracts
- AI miners, haulers, pirates drive the economy
- Resources extracted → moved → produced → destroyed

**Exploration & Missions**
- Probe scanning, deadspace, wormholes
- 5 levels × 7 mission types
- 137 skills across 20 categories

</td></tr>
</table>

---

## 🔧 Modding

All game content is JSON in `data/` — fully moddable:

```
data/ships/     102+ ship definitions       data/universe/   Solar systems, stargates
data/modules/   159+ module definitions      data/missions/   Mission templates
data/skills/    137 skill definitions        data/market/     Economy and pricing
```

**Tools:** `validate_json.py` · `create_ship.py` · `BlenderSpaceshipGenerator/`
→ **[Modding Guide](docs/MODDING_GUIDE.md)**

---

## 🔗 Atlas Engine

This game is built on the [Atlas Engine](https://github.com/shifty81/Atlas) — a modular,
deterministic C++20 game engine. The engine is developed in its own repository and provides:

- **ECS Framework** — Entity/Component/System with type-safe components
- **Graph VM** — Deterministic bytecode VM for visual scripting
- **WorldGraph** — DAG-based procedural world generation
- **Networking** — Client-Server + P2P with lockstep/rollback
- **Asset System** — Binary format, registry, hot reload

The `projects/eveoffline/` directory contains Atlas project files (`.atlas` manifest,
world graphs, strategy graphs, etc.) that are designed to be portable — they can be
copied into the Atlas repo's `projects/` directory for development with the full
Atlas Editor.

---

## 📚 Documentation

| Topic | Links |
|-------|-------|
| **Getting Started** | [Tutorial](docs/TUTORIAL.md) · [Build Guides](docs/guides/) · [C++ Client Quickstart](docs/development/CPP_CLIENT_QUICKSTART.md) |
| **Design** | [Project Context](PROJECT_CONTEXT.md) · [Roadmap](docs/ROADMAP.md) |
| **Development** | [Contributing](docs/CONTRIBUTING.md) · [CI/CD](docs/development/CI_CD.md) |
| **UI & Client** | [Atlas UI](docs/atlas-ui/) · [C++ Client](docs/cpp_client/) · [Game Mechanics](docs/game_mechanics/) |
| **Atlas Engine** | [Atlas Repo](https://github.com/shifty81/Atlas) · [Repo Split Plan](https://github.com/shifty81/Atlas/blob/main/docs/10_REPO_SPLIT_PLAN.md) |

---

## 🤝 Contributing

Contributions are welcome! See [CONTRIBUTING.md](docs/CONTRIBUTING.md).

## 📝 License

[To be determined]

---

<sub>EVEOFFLINE is an indie PVE space simulator. All in-game content uses original naming conventions. Not affiliated with CCP Games.</sub>
