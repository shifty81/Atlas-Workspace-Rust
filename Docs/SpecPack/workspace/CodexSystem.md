# Codex System

## Overview
The Codex is the workspace knowledge base. It stores logs, assets, solutions,
and AI-generated insights for reuse across development sessions.

## Data Types
| Type | Source | Use |
|------|--------|-----|
| Build logs | CI/local builds | Error diagnosis, pattern detection |
| Asset metadata | Content pipeline | Asset dependency tracking |
| Fix records | AtlasAI suggestions | Solution reuse, regression prevention |
| Design docs | Manual entry | System reference, contract validation |
| Session notes | Developer input | Context preservation |

## Existing Infrastructure
- AtlasAI broker system (workspace-level AI)
- Pipeline manifest (asset tracking)
- Console/log panel (runtime logging)
- Content browser (asset browsing)

## Missing
- Codex storage schema (structured knowledge DB)
- Retrieval/search system
- AI-powered auto-indexing
- Cross-session persistence
- GitHub/Google integration hooks

## Schema Needs
```json
{
  "type": "object",
  "properties": {
    "id": { "type": "string" },
    "type": { "enum": ["log", "asset", "fix", "doc", "note"] },
    "content": { "type": "string" },
    "tags": { "type": "array", "items": { "type": "string" } },
    "source": { "type": "string" },
    "timestamp": { "type": "string", "format": "date-time" },
    "related_files": { "type": "array", "items": { "type": "string" } }
  }
}
```
