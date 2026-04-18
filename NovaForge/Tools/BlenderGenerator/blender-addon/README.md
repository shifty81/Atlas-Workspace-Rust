# Atlas Blender Spaceship Generator

A Blender addon for procedural spaceship generation, designed to work with
the Atlas Engine asset pipeline.

## Features

- Procedural hull generation from parametric profiles
- Module slot placement (weapons, engines, bays, sensors)
- Tier-based visual detail (5 tiers: basic → elite)
- One-click export to Atlas Engine format (.obj + .atlas_meta)
- Deterministic generation from seed values

## Installation

1. Open Blender → Edit → Preferences → Add-ons → Install
2. Select `atlas_spaceship_generator.py`
3. Enable "Atlas: Spaceship Generator" in the add-ons list

## Usage

1. Open the sidebar (N key) → Atlas tab
2. Set a seed value (or leave random)
3. Choose hull class: Fighter, Frigate, Cruiser, Battleship, Capital
4. Adjust parameters (length, beam, module count)
5. Click "Generate Ship"
6. Click "Export for Atlas" to produce engine-ready assets

## Atlas Engine Integration

The generator produces:
- `.obj` mesh file (hull + module slots as empties)
- `.atlas_meta` JSON sidecar with material hints, slot metadata, and seed

The Atlas Engine's `AssetImporter` can load both files:

```cpp
auto ship = assetImporter.LoadMesh("ships/cruiser_01.obj");
auto meta = assetImporter.LoadMeta("ships/cruiser_01.atlas_meta");
```

See `ENGINE_INTEGRATION.md` for the full pipeline walkthrough.

## Requirements

- Blender 3.6+ (uses geometry nodes API)
- Python 3.10+ (bundled with Blender)

## File Structure

```
tools/blender-addon/
├── README.md                       # This file
├── ENGINE_INTEGRATION.md           # Engine asset pipeline docs
├── atlas_spaceship_generator.py    # Main Blender addon
└── presets/                        # Hull class parameter presets
    ├── fighter.json
    ├── frigate.json
    ├── cruiser.json
    ├── battleship.json
    └── capital.json
```
