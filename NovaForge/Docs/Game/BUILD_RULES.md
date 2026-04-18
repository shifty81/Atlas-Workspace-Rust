# NovaForge — Build Rules

## Canonical Build Commands

```bash
# Configure (debug + tests)
cmake --preset debug -DNF_BUILD_TESTS=ON

# Build
cmake --build --preset debug

# Run tests
ctest --preset debug

# Configure (Windows, VS 2022, debug + tests)
cmake --preset windows-x64-debug-tests

# Configure (CI)
cmake --preset ci-debug
cmake --build --preset ci-debug
ctest --preset ci-debug
```

## Presets

| Preset | Generator | Purpose |
|--------|-----------|---------|
| `debug` | Ninja/default | Local dev, tests ON |
| `release` | Ninja/default | Local release, tests OFF |
| `windows-x64` | VS 2022 x64 | Windows release |
| `windows-x64-debug` | VS 2022 x64 | Windows debug, tests OFF |
| `windows-x64-debug-tests` | VS 2022 x64 | Windows debug, tests ON |
| `ci-debug` | Ninja | GitHub Actions debug |
| `ci-release` | Ninja | GitHub Actions release |

## MSVC Runtime Library Rule

**Always enforce a consistent MSVC runtime library to avoid LNK4098.**

The root `CMakeLists.txt` sets this before `project()`:

```cmake
cmake_policy(SET CMP0091 NEW)
# ...inside project block:
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
```

This selects:
- **Debug** → `/MDd` (MultiThreadedDebugDLL)
- **Release** → `/MD` (MultiThreadedDLL)

**Do not** mix `/MT` and `/MD` targets. Every library added via FetchContent or
vcpkg must use the same runtime. If a third-party library hard-codes a different
runtime, either replace it or add a custom CMake wrapper to override it.

## Module CMakeLists.txt Pattern

Every `Source/<Module>/CMakeLists.txt` must follow this pattern:

```cmake
add_library(NF_<Module> STATIC src/<Module>.cpp)
add_library(NF::<Module> ALIAS NF_<Module>)

target_include_directories(NF_<Module>
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(NF_<Module>
    PUBLIC NF::Core
    # ... other deps
)

set_target_properties(NF_<Module> PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
```

The `NF::<Module>` alias is required so that `target_link_libraries` can use
the namespaced form throughout the project.

## Test Executables

Tests live in `Tests/<Module>/test_<module>.cpp` and are built as separate
executables per module via `Tests/CMakeLists.txt`. Each uses Catch2 via
FetchContent (`v3.5.2`). Tests are only built when `NF_BUILD_TESTS=ON`.

## Output Directories

All output lands under `${CMAKE_BINARY_DIR}/`:
- Executables → `bin/`
- Static libraries → `lib/`
- Archives → `lib/`

## vcpkg Integration

Dependencies are managed via `vcpkg.json`. The baseline pin is:
`7516a02de04e8f8ff4e4beb8f8eb00552b67a5f2`

To bootstrap vcpkg in CI:
```bash
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.sh  # or .bat on Windows
cmake --preset ci-debug -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
```

## GitHub Actions

CI runs on push to `main` and on all pull requests. The workflow:
1. Checks out repo
2. Bootstraps vcpkg
3. Configures with `ci-debug` preset
4. Builds
5. Runs `ctest --preset ci-debug`

Failures in any step block merge.
