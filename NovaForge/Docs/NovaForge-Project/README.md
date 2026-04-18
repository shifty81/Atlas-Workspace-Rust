# NovaForge

NovaForge is the standalone game project hosted by [Atlas Suite](https://github.com/shifty81/AtlasToolingSuite).

This repository contains the game project only. It does not own Atlas Suite shell behavior, generic tooling, generic editor hosts, repo migration engines, or workspace-level services.

## Atlas Suite Integration

NovaForge connects to Atlas Suite through:
1. **Project manifest** — `novaforge.project.json` declares paths, build targets, capabilities, and bridge config
2. **HTTP bridge** — Atlas Suite communicates with NovaForge via `http://127.0.0.1:8005`
3. **No shared assemblies** — all communication is over the bridge protocol

Atlas Suite must run cleanly without NovaForge present. NovaForge is a hosted project, not a Suite subsystem.

## Top-Level Layout
```text
NovaForge-Project/
  novaforge.project.json        # project manifest (v2)
  README.md
  ARCHITECTURE.md
  PROJECT_RULES.md
  BUILD_RULES.md
  ROADMAP.md
  CONTENT_RULES.md
  TASKS.md
  NovaForge/                    # C++ game source (CMake)
    CMakeLists.txt
    Client/
    Server/
    Gameplay/
    World/
    UI/
    Save/
    App/
    ...
  Integrations/
    AtlasSuite/
      Runtime/                  # C# runtime bridge services
      Adapter/                  # NovaForge-specific Suite adapters
      Plugins/                  # project extension plugins
  AtlasAI/
    ProjectAdapters/
      NovaForge/                # AI project adapter
  Docs/
    Specs/
    Migration/
    NovaForge/
    Runtime/
  Content/
  Config/
  Scripts/
  Tests/
  Intake/
  Packaging/
  Release/
```

## Building
```bash
cd NovaForge
cmake -B build -DCMAKE_BUILD_TYPE=Debug .
cmake --build build
```

## First Steps
1. Review the root rule files.
2. Review `Docs/Specs/`.
3. Review `ATLAS_SUITE_NOVAFORGE_ALIGNMENT_ROLLUP.md` in `Docs/` for full alignment directive.
4. Use `Scripts/bootstrap_repo.ps1` or `Scripts/bootstrap_repo.sh` locally.
5. Only import NovaForge-approved files through manifests and staged migration.
