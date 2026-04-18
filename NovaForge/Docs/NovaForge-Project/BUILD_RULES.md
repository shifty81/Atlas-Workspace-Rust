# Build Rules

## C++ Game Build (CMake)

The game source lives under `NovaForge/` and builds with CMake:

```bash
cd NovaForge
cmake -B build -DCMAKE_BUILD_TYPE=Debug .
cmake --build build
```

CMake targets defined in `novaforge.project.json`:
- `NovaForgeClient` — game client
- `NovaForgeServer` — dedicated server
- `ctest` — project tests

Optional: `-DNOVAFORGE_ENABLE_ATLASAI_INTEGRATION=ON` to build with AtlasAI bridge support.

## Module Dependency Graph (NovaForge/CMakeLists.txt)
- Gameplay: no internal deps
- World: no internal deps
- Save: depends on Gameplay, World
- UI: depends on Gameplay
- App: depends on Gameplay, World, Save, UI
- Client: depends on App
- Server: depends on App

## Integration Layers

Integration code is **not** built by the game CMake tree — it lives in separate directories:

- `Integrations/AtlasSuite/Runtime/` — C# runtime bridge services (Builder, Vehicles, Salvage, Constructs)
- `Integrations/AtlasSuite/Adapter/` — NovaForge-specific Suite adapters (definition service, level, build command)
- `Integrations/AtlasSuite/Plugins/` — project extension plugins
- `AtlasAI/ProjectAdapters/NovaForge/` — AI adapter (bridge, ingestion, manifest)

## Rules
- no circular references
- no tooling modules in game source
- no Atlas Suite shell dependencies in game code
- no editor-only dependencies inside runtime modules
- integration code stays in `Integrations/` and `AtlasAI/`, not in `NovaForge/`
- validation runs before packaging
- packaging fails if validation fails
- prefer compile-safe incremental implementation
