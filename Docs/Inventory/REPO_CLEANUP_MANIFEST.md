# Repo Cleanup Manifest

## Legend

| Action | Meaning |
|--------|---------|
| **edit** | Modify in place |
| **rename** | Move or rename path |
| **archive** | Move to Archive/ or Docs/Archive/ |
| **delete** | Remove from repo |
| **keep** | Keep as-is, verify only |
| **add** | Create new file |

---

## Pass 1: Repo Hygiene (Done)

| Action | Path | Status |
|--------|------|--------|
| add | .gitattributes | ✅ Done |
| edit | .gitignore | ✅ Done |
| edit | CMakeLists.txt (ATLAS_ENABLE_ONLINE_DEPS) | ✅ Done |
| edit | Tests/CMakeLists.txt (guard Catch2 fetch) | ✅ Done |

## Pass 2: Documentation Reset (Done)

| Action | Path | Status |
|--------|------|--------|
| edit | README.md | ✅ Done |
| add | Docs/Canon/* | ✅ Done |
| add | Docs/Roadmap/* | ✅ Done |
| add | Docs/Inventory/* | ✅ Done |

## Pass 3: Active-Path Naming (Done)

| Action | Path | Status |
|--------|------|--------|
| rename | Atlas/Workspace/Arbiter/ → Atlas/Workspace/AtlasAI/ | ✅ Done |
| edit | Source/Pipeline/src/WorkspaceBroker.cpp | ✅ Done |
| edit | Source/Pipeline/src/ArbiterReasoner.cpp | ✅ Done |
| edit | Source/Pipeline/src/ToolWiring.cpp | ✅ Done |
| edit | Source/Pipeline/include/NF/Pipeline/Pipeline.h | ✅ Done |
| edit | Source/Editor/include/NF/Editor/ToolEcosystem.h | ✅ Done |
| edit | Source/Programs/AtlasWorkspace/main.cpp | ✅ Done |
| edit | Tests/Pipeline/test_arbiter_reasoner.cpp | ✅ Done |
| edit | Tests/Pipeline/test_workspace_broker.cpp | ✅ Done |
| edit | Tests/Pipeline/test_tool_wiring.cpp | ✅ Done |
| edit | Tests/Pipeline/test_full_suite.cpp | ✅ Done |
| edit | Tests/Pipeline/test_pipeline.cpp | ✅ Done |
| edit | Tests/Editor/test_s8_editor.cpp | ✅ Done |

## Pass 4: Workspace Entrypoint (Done)

| Action | Path | Status |
|--------|------|--------|
| edit | Source/Programs/AtlasWorkspace/main.cpp | ✅ Done (GDI fallback annotation) |
| edit | Source/Programs/AtlasWorkspace/CMakeLists.txt | ✅ Done (no changes needed) |

## Pass 5: UI Backend (Done)

| Action | Path | Status |
|--------|------|--------|
| edit | Source/UI/include/NF/UI/GDIBackend.h (mark fallback) | ✅ Done |
| edit | Source/UI/include/NF/UI/OpenGLBackend.h (mark compat) | ✅ Done |
| add | Source/UI/include/NF/UI/UIBackendSelector.h | ✅ Done |
| add | Source/UI/include/NF/UI/D3D11Backend.h | ✅ Done |
| add | Source/UI/include/NF/UI/DirectWriteTextBackend.h | ✅ Done |

## Pass 6: Editor Consolidation (Done)

| Action | Path | Status |
|--------|------|--------|
| edit | Source/Editor/include/NF/Editor/Editor.h (reduce umbrella) | ✅ Done |
| edit | Source/Editor/src/Editor.cpp (composition root) | ✅ Done |
| add | Source/Editor/include/NF/Editor/EditorSharedPanels.h | ✅ Done |
| add | Source/Editor/include/NF/Editor/EditorToolRegistry.h | ✅ Done |

## Pass 7: Archive Legacy (Done)

| Action | Path | Status |
|--------|------|--------|
| archive | Tools/ArbiterAI/ → Archive/Legacy/ | ✅ Done |
| archive | Tools/SwissAgent/ → Archive/Legacy/ | ✅ Done |
| archive | AtlasAI/Atlas_Arbiter/ → Archive/Legacy/ | ✅ Done |
| archive | AtlasAI/Atlas_SwissAgent/ → Archive/Legacy/ | ✅ Done |
| archive | build_verify/ → Archive/Legacy/ | ✅ Done |

## Pass 8: NovaForge Adapter (Done)

| Action | Path | Status |
|--------|------|--------|
| add | Source/Editor/include/NF/Editor/IGameProjectAdapter.h | ✅ Done |
| add | Source/Editor/include/NF/Editor/ProjectSystemsTool.h | ✅ Done |
| add | NovaForge/Source/EditorAdapter/ (directory) | ✅ Done |

## Pass 9: Doc Cleanup (Done)

| Action | Path | Status |
|--------|------|--------|
| edit | AtlasAI/README.md | ✅ Done |
| edit | AtlasAI/ATLAS_AI_OVERVIEW.md | ✅ Done (historical, kept as-is) |
| edit | Atlas/Workspace/README.md | ✅ Done |

## Pass 10: Roster Expansion + Overspray Archive + Adapter Header Moves (Done)

| Action | Path | Status |
|--------|------|--------|
| edit | Docs/Canon/05_EDITOR_STRATEGY.md (20-tool roster + adapter model) | ✅ Done |
| edit | Docs/Inventory/EDITOR_TOOL_INVENTORY.md (20-tool target, 10 new tools, rename adapter section) | ✅ Done |
| edit | Docs/Roadmap/04_EDITOR_CONSOLIDATION.md (Phase 3b, 11 new tools in table, adapter model section) | ✅ Done |
| edit | Docs/Canon/00_PROJECT_STATUS.md (update editor surface line) | ✅ Done |
| edit | Docs/Inventory/PANEL_AND_SERVICE_MATRIX.md (adapter path note, 4 new adapter panels) | ✅ Done |
| add | Archive/Legacy/Editor_Overspray/README.md | ✅ Done |
| archive | 49 overspray headers → Archive/Legacy/Editor_Overspray/ | ✅ Done |
| add | NovaForge/Source/Editor/include/NovaForge/Editor/README.md | ✅ Done |
| move | 16 adapter headers → NovaForge/Source/Editor/include/NovaForge/Editor/ | ✅ Done |
