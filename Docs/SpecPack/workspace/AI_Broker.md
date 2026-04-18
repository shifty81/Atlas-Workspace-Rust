# AtlasAI Broker

## Overview
The AtlasAI Broker is the intelligence layer of the workspace. It provides
debugging assistance, code suggestions, and automated tool orchestration.

## Existing Implementation
- `AtlasAI/` — canonical AI location at repo root
- `AtlasAI/Atlas_Arbiter/` — rule-based reasoning (historical; archived)
- `AtlasAI/Atlas_SwissAgent/` — multi-tool agent (historical; archived, absorbed into AtlasAI)
- `NF::WorkspaceBroker` — AI session management (Pipeline module)
- `NF::AtlasAIReasoner` — declarative rule engine (Pipeline module)
- Pipeline tool adapters (BlenderGen, ContractScanner, ReplayMinimizer, AtlasAI)

## Broker Capabilities
| Feature | Status | Notes |
|---------|--------|-------|
| Session management | ✅ Done | WorkspaceBroker in Pipeline |
| Context indexing | ✅ Done | WorkspaceBroker session contexts |
| Rule evaluation | ✅ Done | ArbiterReasoner rules/violations |
| CI gate | ✅ Done | ArbiterReasoner CI summary |
| Tool dispatch | ✅ Done | ToolRegistry event routing |
| Panel host | ❌ Missing | AtlasAI panel in editor |
| Proactive suggestions | ❌ Missing | AI-initiated recommendations |
| Error escalation | ❌ Missing | Auto-route errors to AI analysis |
| Fix application | ❌ Missing | AI applies suggested fixes |

## Missing Integration
- AtlasAI panel host in editor (visible AI interface)
- Error notification → AI analysis pipeline
- Suggested fix → one-click apply workflow
- Codex integration (store/retrieve AI knowledge)
- Project-aware context (understand game systems)
