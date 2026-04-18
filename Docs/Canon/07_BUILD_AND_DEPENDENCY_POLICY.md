# Build and Dependency Policy

## CMake Options

| Option | Default | Purpose |
|--------|---------|---------|
| `ATLAS_BUILD_TESTS` | `OFF` | Build workspace test suite |
| `ATLAS_BUILD_WORKSPACE` | `ON` | Build AtlasWorkspace executable |
| `ATLAS_ENABLE_ONLINE_DEPS` | `OFF` | Allow online dependency fetches |

## Dependency Rules

- No hard network dependency for configure
- Tests require Catch2; fetch only when `ATLAS_ENABLE_ONLINE_DEPS=ON`
- If local/vendored Catch2 is available, use it first
- If tests are enabled without deps available, fail with clear message
- vcpkg dependencies tracked in `vcpkg.json`

## Build Presets

| Preset | Generator | Config | Tests |
|--------|-----------|--------|-------|
| `debug` | Ninja | Debug | ON |
| `release` | Ninja | Release | OFF |
| `ci-debug` | Ninja | Debug | ON |
| `ci-release` | Ninja | Release | OFF |
| `vs2022` | VS 2022 | Debug | ON |

## Audit Snapshot Rules

Never include in audit zips:

- `.git/`
- `build/`, `out/`, `Builds/`
- `.vs/`, `.idea/`
- `Binaries/`, `Intermediate/`
- `build_verify/`
- Generated logs (unless specifically auditing them)
- Cache folders

## NovaForge Build Gating

- NovaForge is not built by default from workspace root
- Use `NF_STANDALONE=ON` for independent NovaForge builds
- Use `NF_HOSTED=ON` when building through workspace
