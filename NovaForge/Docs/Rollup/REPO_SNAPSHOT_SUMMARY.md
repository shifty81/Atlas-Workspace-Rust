# Repo Snapshot Summary

## Top-level structure
- .git
- Additions
- Archive
- AtlasAI
- Codex
- Config
- Content
- Data
- Docs
- Logs
- Project
- Schemas
- Scripts
- Source
- Tests
- ThirdParty
- Tools
- cmake

## Quick read
The repo already has broad folder coverage for engine, editor, data, docs, tools, and tests. That suggests the main problem is not lack of surface area. It is drift, placeholder coverage, and lack of hard contracts between systems.

## Indicators from file layout
- `Source/Editor` is broad and panel-heavy.
- `Docs` and `Additions` already contain multiple spec and starter-pack style drops.
- `Data`, `Schemas`, and `Content` imply an intent to become data-driven.
- `Archive` is large, which supports the need for clearer active-vs-archived boundary rules.

## Immediate repo governance needs
- canonical ownership map for workspace vs hosted project
- schema-first validation
- artifact intake rules
- contract scanner / CI enforcement
- migration and deprecation plan for old UI paths
