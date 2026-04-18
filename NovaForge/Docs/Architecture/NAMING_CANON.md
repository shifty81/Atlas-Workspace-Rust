# NovaForge — Naming Canon

## Rule: This document is the single source of truth for naming.

## Canonical Names
| Old / Deprecated | Canonical | Notes |
|---|---|---|
| ArbiterAI | AtlasAI | AI broker system |
| Arbiter | AtlasAI | AI broker system |
| SwissAgent | AtlasAI | AI broker system |
| AtlasSuite | Atlas Workspace | Hosting shell |
| AtlasToolingSuite | Atlas Workspace | Hosting shell |
| NovaForge Editor (ImGui) | NovaForge Editor (Custom UI) | No ImGui ever |

## Enforcement
- No code or doc may use deprecated names
- All tool READMEs must reflect canonical names
- Config keys must use canonical names
- Test file headers must use canonical names

## Module Names (unchanged)
- NF::Core, NF::Engine, NF::Game, NF::Editor, NF::GraphVM
- NF::AI, NF::Networking, NF::Pipeline, NF::World
- NF::Renderer, NF::UI, NF::Physics, NF::Audio, NF::Animation, NF::Input

## Program Names (unchanged)
- NovaForgeEditor
- NovaForgeGame  
- NovaForgeServer
