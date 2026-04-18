# Atlas Engine/Client/Server Separation

Atlas enforces clean separation between engine core, client runtime, and server runtime. This document formalizes the architectural boundaries and dependency rules.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                   Atlas Build Targets                        │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌─────────────┐  ┌──────────────┐  ┌───────────────┐      │
│  │  AtlasClient│  │ AtlasRuntime │  │  AtlasServer  │      │
│  │   (Player)  │  │     (CLI)    │  │  (Headless)   │      │
│  └──────┬──────┘  └──────┬───────┘  └───────┬───────┘      │
│         │                │                   │               │
│         └────────────────┴───────────────────┘               │
│                          │                                   │
│                   ┌──────▼──────┐                            │
│                   │ AtlasEditor │                            │
│                   │  (Dev Tool) │                            │
│                   └──────┬──────┘                            │
│                          │                                   │
│         ┌────────────────┴────────────────┐                 │
│         │                                  │                 │
│  ┌──────▼──────┐                  ┌───────▼────────┐        │
│  │AtlasGameplay│                  │  AtlasEngine   │        │
│  │  (Gameplay) │                  │  (Core Engine) │        │
│  └─────────────┘                  └────────────────┘        │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

## Core Modules

### 1. AtlasEngine (Static Library)

**Purpose**: Core engine functionality shared by all executables  
**Location**: `engine/`  
**Dependencies**: None (self-contained)  
**CMake Target**: `AtlasEngine`

**Contains**:
- Core bootstrap and logging
- ECS framework
- Graph VM and compiler
- Asset system (registry, binary format, importers)
- Networking (NetContext, replication)
- Simulation (tick scheduler, replay, determinism)
- World generation (terrain, galaxies, streaming)
- AI systems (memory, behavior, strategy)
- Audio, physics, camera systems
- All graph types (world, animation, UI, etc.)
- Module loading system

**Contract**:
- ✅ Must be deterministic
- ✅ No hardcoded gameplay logic
- ✅ No OS-specific rendering
- ✅ Can be built headless

### 2. AtlasGameplay (Static Library)

**Purpose**: Shared gameplay systems used by both client and server  
**Location**: `modules/atlas_gameplay/`  
**Dependencies**: `AtlasEngine`  
**CMake Target**: `AtlasGameplay`

**Contains**:
- Faction system
- Combat mechanics
- Economy simulation
- Diplomacy systems
- Gameplay data structures

**Contract**:
- ✅ Must be deterministic
- ✅ Simulation-only logic
- ✅ No rendering or presentation code
- ✅ Shared by client and server identically

### 3. AtlasServer (Executable)

**Purpose**: Headless authoritative game server  
**Location**: `server/`  
**Dependencies**: `AtlasEngine`, `AtlasGameplay`  
**CMake Target**: `AtlasServer`

**Contains**:
- Server main loop
- Network authority logic
- No rendering or audio
- Headless simulation only

**Contract**:
- ✅ Authoritative simulation
- ✅ No graphics or audio dependencies
- ✅ Deterministic execution
- ❌ Never links rendering libraries
- ❌ Never uses OS display APIs

**Usage**:
```bash
./dist/AtlasServer
```

### 4. AtlasClient (Executable)

**Purpose**: Player runtime with rendering  
**Location**: `client/`  
**Dependencies**: `AtlasEngine`, `AtlasGameplay`  
**CMake Target**: `AtlasClient`

**Contains**:
- Client main loop
- Rendering integration
- Audio playback
- Input handling
- Client-side prediction

**Contract**:
- ✅ Presentation layer only
- ✅ Trusts server authority
- ✅ Can use GPU/audio APIs
- ❌ Never mutates authoritative state
- ❌ No server-only logic

**Usage**:
```bash
./dist/AtlasClient
```

### 5. AtlasRuntime (Executable)

**Purpose**: Standalone runtime with CLI project loading  
**Location**: `runtime/`  
**Dependencies**: `AtlasEngine`, `AtlasGameplay`  
**CMake Target**: `AtlasRuntime`

**Contains**:
- Runtime main loop
- Project loading from CLI
- Flexible execution modes

**Contract**:
- ✅ Can load any `.atlas` project file
- ✅ Supports both client and server modes
- ✅ Useful for automation and testing

**Usage**:
```bash
./dist/AtlasRuntime --project projects/atlas-sample/sample.atlas
```

### 6. AtlasEditor (Executable)

**Purpose**: Developer authoring tool  
**Location**: `editor/`  
**Dependencies**: `AtlasEngine`, `AtlasGameplay`  
**CMake Target**: `AtlasEditor`

**Contains**:
- Editor UI and panels
- Asset browser
- Graph editors
- Play-in-editor modes
- Developer tools

**Contract**:
- ✅ Superset of client functionality
- ✅ Additional authoring tools
- ✅ Editor is a first-class runtime
- ❌ Never in production builds

**Usage**:
```bash
./dist/AtlasEditor
```

## Dependency Rules

### Allowed Dependencies

```
AtlasClient  ───► AtlasGameplay ───► AtlasEngine
AtlasServer  ───┘
AtlasRuntime ───┘
AtlasEditor  ───┘
```

### Forbidden Dependencies

- ❌ AtlasEngine NEVER depends on gameplay modules
- ❌ AtlasEngine NEVER depends on client/server/editor
- ❌ AtlasServer NEVER depends on rendering libraries
- ❌ AtlasClient NEVER contains server-authority logic
- ❌ Game modules NEVER depend on each other directly

## Build Verification

The following must always pass:

```bash
# Build engine independently
cmake --build build --target AtlasEngine

# Build server without graphics
cmake --build build --target AtlasServer

# Verify no forbidden dependencies
# (Server should not link X11, OpenGL, Vulkan)
ldd dist/AtlasServer | grep -i vulkan  # Should be empty
ldd dist/AtlasServer | grep -i opengl  # Should be empty
```

## Game Module System

Game-specific code lives in separate modules:

```
projects/
├── eveoffline/
│   └── module/          # EveOfflineModule (IGameModule)
├── arena2d/
│   └── module/          # Arena2DModule (IGameModule)
└── atlas-sample/
    └── sample.atlas     # Minimal sample project
```

**Module Rules**:
- ✅ Modules extend engine via `IGameModule` interface
- ✅ Modules can depend on `AtlasEngine` and `AtlasGameplay`
- ✅ Modules are dynamically loadable
- ❌ Modules NEVER modify engine core
- ❌ Modules NEVER depend on other modules directly

## CMake Structure

```cmake
# Root CMakeLists.txt
add_subdirectory(engine)              # AtlasEngine (core)
add_subdirectory(modules/atlas_gameplay)  # AtlasGameplay (shared)
add_subdirectory(editor)              # AtlasEditor
add_subdirectory(runtime)             # AtlasRuntime
add_subdirectory(client)              # AtlasClient
add_subdirectory(server)              # AtlasServer

# Game modules
add_subdirectory(projects/eveoffline/module)  # EveOfflineModule
add_subdirectory(projects/arena2d/module)     # Arena2DModule
```

Each target explicitly declares dependencies:

```cmake
# server/CMakeLists.txt
add_executable(AtlasServer main.cpp)
target_link_libraries(AtlasServer AtlasEngine AtlasGameplay)
```

## Enforcement

Dependency boundaries are enforced via:

1. **CMake**: Explicit `target_link_libraries()` declarations
2. **Build System**: Separate build targets prevent accidental links
3. **Code Review**: PR reviews verify no cross-boundary violations
4. **Contract Scanner**: `tools/contract_scan.py` checks forbidden APIs
5. **CI**: Automated builds verify each target independently

## Migration Path

If game-specific code is found in `AtlasEngine`:

1. **Identify**: Code that is game-specific (not general-purpose)
2. **Extract**: Move to `AtlasGameplay` or game module
3. **Interface**: Define clean API boundary in `AtlasEngine`
4. **Verify**: Rebuild and test all targets
5. **Document**: Update this file with changes

## See Also

- [docs/01_ENGINE_ARCHITECTURE.md](01_ENGINE_ARCHITECTURE.md) - Detailed engine architecture
- [docs/10_REPO_SPLIT_PLAN.md](10_REPO_SPLIT_PLAN.md) - Module architecture details
- [docs/ATLAS_CORE_CONTRACT.md](ATLAS_CORE_CONTRACT.md) - Core engine invariants
- [docs/BUILDING.md](BUILDING.md) - Build instructions
- [CMakeLists.txt](../CMakeLists.txt) - Root build configuration
