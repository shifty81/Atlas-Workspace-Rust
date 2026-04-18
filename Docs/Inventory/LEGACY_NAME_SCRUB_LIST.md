# Legacy Name Scrub List

## Active Renames

| Old Name | New Name | Scope | Status |
|----------|----------|-------|--------|
| Arbiter | AtlasAI | All active code/docs | ✅ Done |
| ArbiterAI | AtlasAI | Tools, modules | ✅ Done |
| AtlasArbiter | AtlasAI | Modules, docs | ✅ Done |
| ARBITER | ATLAS_AI | Config keys, macros | ✅ Done |
| arbiter | atlas_ai | File names, symbols | ✅ Done |
| arbiter_reasoner | atlas_ai_reasoner | Pipeline symbols | ✅ Done |
| Workspace/Arbiter | Workspace/AtlasAI | Directory paths | ✅ Done |
| SwissAgent | AtlasAI | Pipeline adapter, broker, tool runner | ✅ Done |
| Atlas Suite | Atlas Workspace | Product naming | ✅ Done |
| AtlasToolingSuite | Atlas Workspace | Product naming | ✅ Done |
| MasterRepo.exe | AtlasWorkspace.exe | Executable naming | ✅ Done |

## Files Updated

All active-path Arbiter and SwissAgent references have been resolved:

- `Source/Pipeline/src/AtlasAIReasoner.cpp` — renamed from ArbiterReasoner.cpp
- `Source/Pipeline/src/WorkspaceBroker.cpp` — SwissAgent → AtlasAI
- `Source/Pipeline/src/ToolWiring.cpp` — SwissAgentAdapter removed, absorbed into AtlasAIAdapter
- `Source/Pipeline/include/NF/Pipeline/Pipeline.h` — SwissAgentAdapter class removed
- `Source/Editor/include/NF/Editor/ToolEcosystem.h` — m_swissAgent + m_arbiter → m_atlasAI
- `Source/Workspace/include/NF/Workspace/BrokerFlowController.h` — dual-tool skip → single AtlasAI check
- All Pipeline test files updated to use AtlasAIAdapter

## Rules

- Active docs: correct to canon
- Historical docs: prefix with "Historical:" or move to Docs/Archive
- Archive content: leave as-is (do not rename inside archive)

