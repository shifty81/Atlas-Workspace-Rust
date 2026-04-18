# NovaForge — Voxel + Low-Poly Render Pipeline

## Core Rule
**Voxels are authoritative. Low-poly is visual.**

## Data Ownership
| Layer | Owns |
|---|---|
| Voxel | Mass, mining damage, structural integrity, collision, material type |
| Low-Poly | Visual identity, readability, surface detail, animation |

## Pipeline Flow
```
VoxelWorld (authoritative)
    │
    ├─► ChunkMesher → VoxelMesh (blocky, debug/mining layer)
    │
    └─► LowPolyOverlay → LowPolyMesh (smooth visual shell)
                              │
                              └─► DamageExposure → reveals voxel damage state
```

## Damage Rule
When a voxel is damaged:
1. Voxel health decreases (authoritative)
2. Low-poly mesh receives damage overlay (cracks, deformation)
3. If voxel is destroyed, low-poly section is removed or replaced with debris

## Asset Layering
- Low-poly assets live in `Content/Meshes/LowPoly/`
- Voxel type definitions live in `Content/Definitions/`
- Each low-poly module covers one functional interactable (door, panel, engine nacelle)

## Mining / Repair
- Mining targets the voxel layer only
- Low-poly follows voxel state, never leads it
- Repair restores voxel health; low-poly regenerates accordingly

## Performance
- Voxel mesh: generated per chunk, rebuilt on dirty
- Low-poly mesh: loaded as static asset, skinned to voxel damage state
- Frustum culling applied at chunk level (Frustum::testAABB)
