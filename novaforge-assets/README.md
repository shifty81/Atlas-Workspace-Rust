# NovaForge Assets

This directory contains game assets for the NovaForge project, sourced from the [Nova-Forge](https://github.com/shifty81/Nova-Forge) repository (a fork of [Veloren](https://veloren.net)).

## ⚠ Not committed to Git

The actual asset files are **not committed** to this repository. This directory is listed in `.gitignore` (binary content only — the subdirectory structure and this README are tracked).

Asset files are downloaded locally by running the fetch script:

```bash
bash Scripts/fetch_novaforge_assets.sh
```

## Directory structure

```
novaforge-assets/
  common/       # Shared game assets (items, entities, recipes, etc.)
  voxygen/      # Client-side assets (textures, shaders, UI, fonts, audio)
  world/        # Terrain data, map binaries, biome definitions
  server/       # Server-side config assets
```

## Environment variable

Set `NOVAFORGE_ASSETS_DIR` to override the default search path:

```bash
export NOVAFORGE_ASSETS_DIR=/path/to/novaforge-assets
cargo run --bin atlas-workspace
```

The `atlas-asset` crate reads this variable at startup. If not set, it looks for `novaforge-assets/` relative to the current working directory.

## Re-fetching assets

To re-download or update assets from Nova-Forge:

```bash
bash Scripts/fetch_novaforge_assets.sh
```

This script clones the Nova-Forge repository with Git LFS enabled, then extracts the `assets/` directory into this folder. It requires:
- Git (with Git LFS support)
- Internet access to `github.com`

## Licensing

All assets in this directory are sourced from Veloren (via Nova-Forge) and are covered by the **GNU General Public License v3.0**. See [`LICENSES/GPL-3.0`](../LICENSES/GPL-3.0) for the full license text.
