# Voxel Render Pipeline

> **Status: Conceptual / Deferred**

The voxel render pipeline is a planned rendering subsystem for voxel-based
content within the Atlas Workspace viewport system.

## Architecture (Planned)

```
VoxelVolume → ChunkManager → MeshGenerator → RenderBatch → Viewport
```

### Components

| Component | Description | Status |
|-----------|-------------|--------|
| VoxelVolume | 3D grid of voxel data | Conceptual |
| ChunkManager | Spatial partitioning of voxel data into renderable chunks | Conceptual |
| MeshGenerator | Greedy meshing / marching cubes for chunk surface extraction | Conceptual |
| RenderBatch | Instanced draw calls for visible chunks | Conceptual |
| LOD System | Distance-based chunk detail reduction | Conceptual |

### Integration Points

- Viewport: Renders through `IViewportSceneProvider` interface
- Asset Catalog: Voxel data stored as project assets
- Editor Tool: `SceneEditorTool` or dedicated VoxelEditorTool

### Dependencies

Requires D3D11 backend execution (currently stubs only) for performant rendering.
GDI fallback is insufficient for real-time voxel rendering.

See `Docs/Canon/04_UI_BACKEND_STRATEGY.md` for backend roadmap.
