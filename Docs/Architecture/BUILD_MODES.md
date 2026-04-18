# Build Modes

## CMake Presets

| Preset | Config | Tests | Use Case |
|--------|--------|-------|----------|
| `default` | Debug | OFF | Development builds |
| `ci-release-tests` | Release | ON | CI smoke testing |

## Build Flags

| Flag | Default | Description |
|------|---------|-------------|
| `ATLAS_BUILD_TESTS` | OFF | Enable test target compilation |
| `ATLAS_ENABLE_ONLINE_DEPS` | OFF | Allow online fetch of test deps (Catch2) |
| `ATLAS_BUILD_WORKSPACE` | ON | Build the Workspace module |
| `ATLAS_BUILD_PROGRAMS` | ON | Build executable programs |

## Build Commands

```bash
# Debug with tests
cmake -S . -B build -DATLAS_BUILD_TESTS=ON -DATLAS_ENABLE_ONLINE_DEPS=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure

# Release (CI)
cmake --preset ci-release-tests
cmake --build --preset ci-release-tests
ctest --preset ci-release-tests

# Docker
docker build -t atlas-workspace .
```

## Module Dependencies

```
NF::Core           (no deps)
NF::Input           → Core
NF::Physics         → Core
NF::Audio           → Core
NF::Animation       → Core
NF::Networking      → Core
NF::Engine          → Core, Physics, Audio, Animation, Networking
NF::Renderer        → Core, Engine
NF::UI              → Core, Input
NF::AI              → Core
NF::Pipeline        → Core
NF::GraphVM         → Core
NF::Workspace       → Core, Input, UI, AI, Pipeline, Engine, Renderer
NF::Editor          → Workspace, Core, Input, UI, AI, Pipeline, Engine, Renderer
NF::NovaForgeAdapter → Workspace (INTERFACE)
```

See `Docs/Canon/07_BUILD_AND_DEPENDENCY_POLICY.md` for dependency tier rules.
