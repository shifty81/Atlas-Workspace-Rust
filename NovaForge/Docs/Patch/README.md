# Atlas Workspace / NovaForge Repo Patch — Missing Information Pack

This patch pack is for insertion into the repo as documentation-first coverage for project-chat decisions that are either missing, only partially represented, or only referenced from rollups.

## Purpose
- close spec drift between the current repo zip and the project chat
- provide concrete drop-in docs for missing gameplay/UI/workspace contracts
- give the repo a clean place to continue implementation without re-deriving decisions from chat history

## Recommended Insert Path
- `Docs/Patch/MissingInfo/` for markdown specs
- `Schemas/patch/` for JSON schema starters
- `Source/Patch/include/` for C++ header starters

## Highest Value Items In This Pack
- Launch-to-play flow
- Game HUD contract
- Game menu and screen flow
- Interaction screen family
- Equipment GUI stack
- Mech transformation extension of the R.I.G.
- Unified inventory and storage access topology
- Service and hangar screens
- External workspace integration notes for Google Drive and Steam Workshop direction
