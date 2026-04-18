# NovaForge — Build Output Directory Structure

## Overview

NovaForge uses CMake with presets to manage builds across platforms. All build
artifacts land under `Builds/<preset>/` with a consistent layout.

## Canonical Directory Layout

```
Builds/<preset>/
├── bin/
│   ├── <Config>/                    # Debug, Release, RelWithDebInfo
│   │   ├── NovaForgeEditor(.exe)    # Development editor
│   │   ├── NovaForgeGame(.exe)      # Standalone game client
│   │   ├── NovaForgeServer(.exe)    # Dedicated headless server
│   │   └── Tests/                   # All test executables
│   │       ├── NF_CoreTests(.exe)
│   │       ├── NF_EngineTests(.exe)
│   │       ├── NF_PhysicsTests(.exe)
│   │       ├── NF_GameTests(.exe)
│   │       ├── NF_GraphVMTests(.exe)
│   │       ├── NF_InputTests(.exe)
│   │       ├── NF_RendererTests(.exe)
│   │       ├── NF_AudioTests(.exe)
│   │       ├── NF_AnimationTests(.exe)
│   │       ├── NF_NetworkingTests(.exe)
│   │       ├── NF_AITests(.exe)
│   │       ├── NF_EditorTests(.exe)
│   │       ├── NF_PipelineTests(.exe)
│   │       └── NF_UITests(.exe)
│   └── (single-config generators put executables here directly)
├── lib/
│   └── <Config>/                    # Static libraries (.a / .lib)
└── compile_commands.json            # For IDE integration
```

## Build Presets

| Preset | Generator | Platform | Build Dir |
|--------|-----------|----------|-----------|
| `debug` | Ninja | Linux/macOS | `Builds/debug` |
| `release` | Ninja | Linux/macOS | `Builds/release` |
| `vs2022` | VS 2022 | Windows | `Builds/vs2022` |
| `windows-x64-debug` | VS 2022 | Windows | `Builds/windows-x64-debug` |
| `windows-x64-debug-tests` | VS 2022 | Windows | `Builds/windows-x64-debug-tests` |
| `ci-debug` | Ninja | CI (Linux) | `Builds/ci-debug` |
| `ci-release` | Ninja | CI (Linux) | `Builds/ci-release` |

## Build Scripts

### Linux / macOS
```bash
# Full build with progress bars and log mirroring
./Scripts/build.sh Debug --test

# Build specific target
./Scripts/build.sh Debug --editor
./Scripts/build.sh Debug --game
./Scripts/build.sh Debug --server

# Quick build via Make
make build           # Debug, all targets
make editor          # Debug, editor only
make test            # Debug, build + run tests
```

### Windows
```batch
:: Full build with progress and logging
Scripts\build_win.cmd Debug --test

:: Build specific target
Scripts\build_win.cmd Debug --editor

:: Legacy MSBuild wrapper
build.cmd Debug
```

## Log Mirroring

All build output is simultaneously:
1. Displayed on screen with progress bars and colors
2. Written to `Logs/build.log` (plain text, ANSI stripped)
3. Test results written to `Logs/test.log`

The editor also writes session logs to `Logs/editor.log`.

## Atlas Workspace — AI Tool Packages

Atlas AI tools live in a separate directory structure from C++ build output.
Python/Docker tools don't have Debug/Release configs, so they stay out of `bin/`.

```
Atlas/
└── Workspace/
    ├── Arbiter/              ← Atlas_Arbiter deployable package
    │   ├── arbiter_cli.py
    │   ├── rules/            ← .arbiter.json rule files
    │   ├── Dockerfile
    │   └── config.toml
    ├── SwissAgent/           ← Atlas_SwissAgent deployable package
    │   ├── cli.py
    │   ├── core/
    │   ├── llm/
    │   ├── tools/
    │   ├── Dockerfile
    │   └── config.toml
    └── Sessions/             ← Runtime session data (gitignored)
```

| Directory | Purpose | Source Code |
|-----------|---------|-------------|
| `AtlasAI/` | AI tool source code and reference snippets | Committed |
| `Atlas/Workspace/` | Deployable tool packages (staged for execution) | Committed (except Sessions/) |
| `.novaforge/pipeline/` | Runtime IPC — ChangeEvent JSON between tools | Gitignored |

## Pipeline IPC Directory

All tools (C++ engine, Atlas AI, Blender bridge) communicate through the pipeline:

```
.novaforge/
├── pipeline/
│   ├── changes/       ← .change.json events dropped here by any tool
│   ├── assets/        ← imported/generated assets
│   ├── worlds/        ← live world-state snapshots
│   ├── scripts/       ← GraphVM bytecode / logic graph JSON
│   ├── animations/    ← exported rigs, clips, IK configs
│   └── sessions/      ← AtlasAI session logs
├── manifest.json      ← GUID → asset path registry
└── watch.log          ← append-only event log
```

## Runtime Directories

| Directory | Purpose | Gitignored |
|-----------|---------|------------|
| `Saved/` | Editor session state (layout, settings) | Yes |
| `Logs/` | Build and runtime log files | Yes (*.log) |
| `Content/` | Engine-level content (fonts, definitions) | No |
| `Data/` | Game configuration data (ships, modules, etc.) | No |
| `Config/` | Project configuration | No |
| `Atlas/Workspace/Sessions/` | AI tool session data | Yes |
| `.novaforge/pipeline/` | Pipeline IPC (ChangeEvent files) | Yes |
