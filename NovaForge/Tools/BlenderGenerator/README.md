# BlenderGenerator — Blender Bridge for NovaForge

> **Canonical Name:** Atlas_BlenderGen (future migration to `Tools/Atlas_BlenderGen/`)
> **Pipeline Only:** All output goes through `.novaforge/pipeline/` — no direct file writes.

## Overview

BlenderGenerator is a **Blender add-on** (`novaforge_bridge.py`) that bridges Blender's
3D authoring capabilities with the NovaForge engine asset pipeline. It enables artists
to export meshes, rigs, and animations directly into the NovaForge content pipeline.

## Features

- One-click export from Blender to NovaForge content directory
- Mesh export with vertex colors and UV maps
- Skeleton/rig export compatible with NF::Animation
- Animation clip export with channel data
- Pipeline event generation for asset tracking

## File Structure

```
Tools/BlenderGenerator/
├── README.md                    # This file
└── novaforge_bridge.py          # Blender add-on (BG-1 scaffold)
```

## Roadmap

| Task | Description | Status |
|------|-------------|--------|
| BG-1 | Blender add-on scaffold (register/unregister, panel UI) | ✅ Done |
| BG-2 | Mesh export (vertices, normals, UVs, vertex colors → GLB) | ✅ Done |
| BG-3 | Skeleton export (bone hierarchy → skinned GLB) | ✅ Done |
| BG-4 | Animation export (NLA tracks → animation GLB) | ✅ Done |
| BG-5 | Pipeline integration (auto-emit ChangeEvent, BlenderBridge import) | ✅ Done |

## Pipeline Contract

- **Reads:** `Content/Incoming/` for import requests
- **Writes:** Exported assets to `Content/Models/`, `Content/Animations/`
- **Events:** `pipeline/changes/<timestamp>_blendergen_FileAdded.change.json`
- **Manifest:** Exported assets registered in `pipeline/manifest.json`

## Installation

1. Open Blender → Edit → Preferences → Add-ons → Install
2. Select `Tools/BlenderGenerator/novaforge_bridge.py`
3. Enable "NovaForge Bridge" in the add-ons list
4. The NovaForge panel appears in the 3D Viewport sidebar (N-panel)

## Usage

1. Model your asset in Blender
2. Open the NovaForge panel in the sidebar
3. Set the export path (defaults to `Content/Incoming/`)
4. Click "Export to NovaForge"
5. The asset appears in the editor's Content Browser after pipeline processing
