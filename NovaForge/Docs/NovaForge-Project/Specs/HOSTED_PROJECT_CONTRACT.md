# Hosted Project Contract

NovaForge exists inside Atlas Suite as a hosted game project. Atlas Suite is the host environment. NovaForge is the hosted product.

## Atlas Suite owns
- shell and workspace environment
- project loading and registration
- generic editor hosts
- generic content intake framework
- generic build orchestration
- generic logging infrastructure
- generic AI/debug broker workflows
- generic release pipeline framework

## NovaForge owns
- game runtime (C++ CMake project under `NovaForge/`)
- project data and content definitions
- project-specific schemas and inspectors
- project-specific validation rules
- project build configuration
- client and server startup definitions
- project packaging metadata
- project tests
- project release metadata
- Atlas Suite integration layers (`Integrations/AtlasSuite/`)
- AtlasAI project adapter (`AtlasAI/ProjectAdapters/NovaForge/`)

## Bridge Protocol
- Atlas Suite reads `novaforge.project.json` at startup
- `AtlasAiBridgeService` sends tool calls to `http://127.0.0.1:8005`
- No shared assemblies — Suite never loads NovaForge DLLs
- NovaForge can be on any machine, any OS, and deployed independently

## Required Top-Level Layout
```text
NovaForge-Project/
  novaforge.project.json
  README.md
  NovaForge/                  # C++ game source (CMake)
  Integrations/AtlasSuite/    # runtime bridge, adapter, plugins
  AtlasAI/ProjectAdapters/    # AI project adapter
  Docs/
  Content/
  Config/
  Scripts/
  Tests/
  Packaging/
  Release/
```
