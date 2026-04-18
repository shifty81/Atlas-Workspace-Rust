# NovaForge — Build Modes

## Mode 1: Standalone
Default mode. Builds all targets.
```
cmake --preset debug -DNF_BUILD_TESTS=ON
cmake --build --preset debug
```

## Mode 2: Workspace Hosted
Used when Atlas Workspace is the host. Workspace controls which targets are built.
```
cmake --preset debug -DNF_HOSTED=ON
```
In hosted mode:
- NF_BUILD_EDITOR, NF_BUILD_GAME, NF_BUILD_SERVER default to OFF
- Workspace triggers specific target builds via CMake --target flags
- Tests can still be enabled with -DNF_BUILD_TESTS=ON

## Mode 3: CI
Used in GitHub Actions.
```
cmake --preset ci-debug -DNF_BUILD_TESTS=ON
cmake --build --preset ci-debug
ctest --preset ci-debug
```

## CMake Options Summary
| Option | Default (Standalone) | Default (Hosted) | Description |
|---|---|---|---|
| NF_STANDALONE | ON | OFF | Standalone build flag |
| NF_HOSTED | OFF | ON | Workspace-hosted flag |
| NF_BUILD_EDITOR | ON | OFF | Build editor executable |
| NF_BUILD_GAME | ON | OFF | Build game executable |
| NF_BUILD_SERVER | ON | OFF | Build server executable |
| NF_BUILD_TESTS | OFF | OFF | Build test suite |

## VS2022 Preset
```
cmake --preset vs2022
```
Generates a Visual Studio 2022 solution. Works in both modes.
For tests: add `-DNF_BUILD_TESTS=ON`.
