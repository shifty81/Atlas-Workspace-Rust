# Atlas Engine — World Generation

## Core Principle

You do not edit the generated world.
You edit the **generator graph + parameters**.
The world is a cache, not the source of truth.

## World Layout Interface

The engine does not know if a world is 2D, 3D, voxel, planet, or graph-based.
Everything goes through the `WorldLayout` interface.

### Layout Types

| Layout             | Description       |
|--------------------|-------------------|
| `linear_1d`        | Side scroller     |
| `grid_2d`          | Top-down / tile-based |
| `voxel_3d`         | Voxel-based 3D     |
| `cube_sphere`      | Planet surfaces    |
| `orbital_graph`    | Space systems      |
| `hierarchical_cells` | Strategy maps   |

## Planet Generation

- Cube-sphere mapping (6 faces, quadtree per face)
- LOD rings based on camera distance
- Generated via Graph VM
- Editable chunks with delta overlays
- Runtime deformation support

### Cube-Sphere Coordinate

```cpp
enum CubeFace { POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z };

struct CubeSphereCoord {
    CubeFace face;
    int x, y;
    int lod;
};
```

## Galaxy Generation

- Spiral galaxy templates
- Up to 100,000+ star systems
- Systems generated on demand (seed + rules)
- Only seed, rules, and player changes are persisted

## Chunk Streaming

Worlds are streamed in chunks. You never load the whole planet.
The `WorldStreamer` class manages chunk lifecycle:

### Chunk States

| State | Description |
|-------|-------------|
| `Unloaded` | No data in memory or on disk |
| `Loading` | Load request queued, awaiting data |
| `Loaded` | Chunk data resident in memory |
| `Cached` | Data saved to disk, freed from memory |

### Streaming Protocol

```cpp
WorldStreamer streamer(layout, "cache/world");
streamer.Update(viewerPos, lod, loadRadius, unloadRadius);
```

The `Update()` method continuously manages chunks based on viewer position:

1. Chunks within `loadRadius` are requested via `RequestLoad()`
2. `SetChunkData()` marks a chunk as Loaded once generation completes
3. Chunks beyond `unloadRadius` are unloaded via `UnloadChunk()`
4. Unloaded chunks are optionally cached to disk for fast reload

### Disk Cache

The streamer supports transparent disk caching:
- `SaveChunkToCache(chunkID)` — writes chunk data to the cache directory
- `LoadChunkFromCache(chunkID)` — restores chunk from disk instead of regenerating
- Cache directory is configurable per-streamer instance

### Query API

- `GetChunkState(chunkID)` — returns current chunk state
- `GetLoadedChunks()` — returns all in-memory chunks
- `LoadedCount()` / `CachedCount()` — telemetry counters

### Generation Flow

```
Chunk Request → WorldStreamer → NoiseGenerator → TerrainMeshGenerator → ChunkData
```

## Terrain Mesh Generation

The `TerrainMeshGenerator` produces vertex/index data for terrain chunks:

```cpp
TerrainMesh mesh = TerrainMeshGenerator::Generate(chunk, resolution, chunkSize, heightFn);
```

- Generates a grid of `resolution × resolution` quads
- Supports pluggable height functions
- Computes per-vertex normals from face normals

## Noise Generation

The `NoiseGenerator` provides deterministic procedural noise:

- **Perlin2D**: Classic 2D Perlin noise with configurable seed
- **FBM2D**: Fractal Brownian Motion layering multiple Perlin octaves

```cpp
float height = NoiseGenerator::FBM2D(x, z, octaves, lacunarity, persistence, seed);
```

All noise functions are deterministic given the same seed, supporting the engine's
determinism-first design principle.

## Galaxy Generation

The `GalaxyGenerator` produces spiral galaxy star systems:

- Configurable arm count, spread, rotation
- Core density bias for central bulge
- Deterministic from seed
- Region-filtered queries for streaming

```cpp
GalaxyParams params;
params.armCount = 4;
params.systemCount = 100000;
auto systems = GalaxyGenerator::Generate(params);
```

## GameType Templates

Game types define how worlds are generated, structured, streamed, and simulated:

```json
{
  "id": "planet_demo",
  "type": "GameType",
  "data": {
    "world_layout": "cube_sphere",
    "chunk_schema": "planet_height_chunks",
    "generation_graph": "planet_height_v1",
    "streaming_policy": "planet_streaming",
    "camera_profile": "orbital_follow"
  }
}
```

## WorldGraph — DAG-Based Generation

The `WorldGraph` (`engine/world/WorldGraph.h`) provides a DAG-based
generation pipeline where each node transforms typed data.

### Value Types

| Type | Description |
|------|-------------|
| `Float` | Scalar value |
| `Int` | Integer value |
| `Vec2` | 2D vector |
| `Vec3` | 3D vector |
| `Bool` | Boolean flag |
| `Seed` | RNG seed |
| `HeightField` | 2D height data grid |
| `DensityField` | 3D density volume |
| `Mask` | Binary region mask |

### Node Model

Each node is a subclass of `WorldNode` with typed input/output ports:

```cpp
class WorldNode {
public:
    virtual const char* GetName() const = 0;
    virtual const char* GetCategory() const = 0;
    virtual std::vector<WorldPort> Inputs() const = 0;
    virtual std::vector<WorldPort> Outputs() const = 0;
    virtual void Evaluate(const WorldContext& ctx,
                          const std::vector<WorldValue>& inputs,
                          std::vector<WorldValue>& outputs) const = 0;
};
```

### Execution Context

Each evaluation receives a `WorldContext` with:
- World seed
- Current LOD level
- Chunk coordinates (X, Y, Z)

### Compilation & Execution

1. `Compile()` — topological sort with cycle detection
2. `Execute(ctx)` — runs nodes in sorted order, passing outputs
   to downstream inputs via `(nodeID << 32 | port)` keys
