# Atlas Engine — Asset System

## Asset Rules

1. One format (`.atlas` / `.atlasb`)
2. No editor-only data
3. Deterministic loading
4. Graph-backed execution
5. Hot-reloadable
6. Same format for editor, client, and server
7. Hash-addressed — assets are immutable, instances are mutable

## Asset Types

```cpp
enum class AssetType : uint16_t {
    Graph,
    World,
    Planet,
    Galaxy,
    Mesh,
    Material,
    Mechanic,
    VoxelSchema,
    GameType,
    EditorProfile,
    Texture,
    Audio,
    Font,
    SpriteSheet,
};
```

## Binary Layout (`.atlasb`)

```
[AssetHeader]        — 22 bytes (magic + version + type + size + hash)
[SerializedData]     — Variable-length payload
```

### Asset Header

```cpp
constexpr uint32_t ASSET_MAGIC = 0x41544C53; // "ATLS"

struct AssetHeader {
    uint32_t magic;        // ASSET_MAGIC
    uint16_t version;
    AssetType type;
    uint32_t size;         // Payload size in bytes
    uint64_t hash;         // FNV-1a hash of payload
};
```

### Graph Binary Format

For graph-type assets, `AssetBinary::WriteGraph()` serializes bytecode:

```
[AssetHeader]
[constantCount : 4 bytes]
[constants     : constantCount × sizeof(float)]
[codeCount     : 4 bytes]
[instructions  : codeCount × sizeof(uint32_t)]
```

`AssetBinary::ReadGraph()` validates the magic number and reads the
constants and instructions back into a `Bytecode` structure.

## Asset Import Pipeline

External assets are converted to Atlas's `.atlasb` format through the
`AssetImportRegistry` (`engine/assets/AssetImporter.h`).

### Supported Import Formats

| Category | Formats | Importer Class |
|----------|---------|----------------|
| Mesh | `.fbx`, `.obj`, `.gltf`, `.glb` | `MeshImporter` |
| Texture | `.png`, `.dds`, `.tga` | `TextureImporter` |
| Audio | `.wav`, `.ogg`, `.flac` | `AudioImporter` |
| Font | `.ttf`, `.otf` | `FontImporter` |
| Sprite | `.sprite`, `.spritesheet` | `SpriteSheetImporter` |

### Import Options

```cpp
struct ImportOptions {
    bool generateMipmaps    = false;
    bool compressTextures   = false;
    bool calculateNormals   = false;
    bool flipUVs            = false;
    uint32_t maxTextureSize = 4096;
    std::string outputDirectory;
};
```

### Import Flow

```
Source File → AssetImportRegistry::Import()
  → FindImporter(path)
  → Importer::Import(path, options)
  → Write AssetHeader + payload → .atlasb
  → Return ImportResult { success, outputPath, outputType, outputHash }
```

## Asset Validation

The `AssetValidator` (`engine/assets/AssetValidator.h`) provides integrity
verification, schema migration, and dependency tracking.

### Validation

- `Validate(path)` — Full file validation (header magic + hash integrity)
- `ValidateHeader(path)` — Header-only check (magic number)
- `ComputeFileHash(path)` — FNV-1a hash of the data payload

### Schema Migration

Assets support forward migration between schema versions:

```cpp
validator.RegisterMigration(1, 2, "Add texture compression field");
validator.RegisterMigration(2, 3, "Add LOD support");

auto path = validator.MigrationPath(1, 3);
// Returns [{1→2}, {2→3}]
```

### Dependency Graph

The validator tracks inter-asset dependencies and detects cycles:

```cpp
validator.AddDependency("material_stone", "texture_stone_diffuse");
validator.AddDependency("prefab_wall", "material_stone");

auto deps = validator.GetDependencies("prefab_wall");
// ["material_stone", "mesh_wall"]

bool cycle = validator.HasCircularDependency("prefab_wall");
// false (no cycle)
```

## Asset Category Registry

The `AssetCategoryRegistry` classifies assets by content and system usage:

| Content Category | Examples |
|-----------------|----------|
| Visual | Mesh, Material, Texture |
| Audio | Audio, SpriteSheet |
| Data | Graph, Mechanic, GameType |
| Font | Font |
| Composite | World, Planet, Galaxy |

| System Usage | Description |
|-------------|-------------|
| Gameplay | Used by game logic |
| UI | User interface assets |
| World | World generation |
| Rendering | Visual rendering |
| AI | Artificial intelligence |
| Animation | Animation system |
| Physics | Physics simulation |
| General | Cross-cutting |

## Save / Load Path

### Editor Save
```
Editor → AssetCompiler → Binary Asset → AssetRegistry → Live Reload
```

### Runtime Load
```
Engine → AssetRegistry → Deserialize → Graph VM Compile → Runtime Instance
```

## Hot Reload

The engine includes a file watcher that monitors the assets directory:

```
File Change → AssetWatcher → AssetRegistry::Reload(id) → GraphVM::Rebind() → Runtime continues
```

### Safe Reload Rules
- Graphs replace atomically
- Old instances finish tick
- New instances use new data
- State migrated if compatible

## Multiplayer Constraint
- Server reloads first
- Clients receive updated asset
- Checksum mismatch → client resync
