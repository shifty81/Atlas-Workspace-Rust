# Voxel World Generation

## Status: Planned (Phase 2)

## Overview

NovaForge uses a chunk-based voxel world. World generation is split into:

1. **Heightmap Pass** — Perlin/Simplex noise layered heightmap
2. **Biome Pass** — Temperature + humidity classification
3. **Structure Pass** — POI, ruins, dungeons injected post-biome
4. **Decoration Pass** — Foliage, ore veins, surface detail

## Chunk Spec

- Chunk size: 32x32x32 voxels (configurable)
- World height: 256 voxels (8 chunks vertical)
- Coordinate system: Y-up

## References

- See `Docs/Architecture/VOXEL_RENDER_PIPELINE.md` for rendering side
- See `Source/World/` for implementation target
