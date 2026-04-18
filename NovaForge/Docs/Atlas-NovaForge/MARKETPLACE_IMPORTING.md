# Marketplace Asset Importing

Atlas provides an extensible system for importing assets from external marketplaces directly into the Atlas asset format. This allows game developers to leverage existing asset libraries from popular platforms while maintaining Atlas's deterministic asset guarantees.

## Supported Marketplaces

### itch.io
- **Status**: Basic support available
- **API Requirements**: None (public assets)
- **Supported Formats**: PNG, OBJ, FBX, WAV, OGG, TTF
- **Availability**: Always available

### Unreal Engine Marketplace
- **Status**: Stub implementation (requires Epic Games Store API integration)
- **API Requirements**: Epic Games Store API credentials
- **Supported Formats**: .uasset (converted to intermediate formats)
- **Availability**: Requires API key configuration

### Unity Asset Store
- **Status**: Stub implementation (requires Unity API integration)
- **API Requirements**: Unity Asset Store API credentials
- **Supported Formats**: .prefab, .asset (converted to intermediate formats)
- **Availability**: Requires API key configuration

## Architecture

### Core Components

1. **IMarketplaceImporter**: Abstract base class for marketplace-specific importers
2. **MarketplaceImportRegistry**: Central registry for managing importers
3. **MarketplaceAssetMetadata**: Structured metadata for marketplace assets
4. **MarketplaceFetchResult**: Result of fetching an asset from a marketplace
5. **MarketplaceImportOptions**: Configuration for import operations

### Import Pipeline

```
Marketplace → Fetch → Download → Convert → Import → Atlas Binary Format
```

1. **Fetch**: Download asset from marketplace using marketplace-specific API
2. **Download**: Cache asset locally in temporary directory
3. **Convert**: Convert marketplace-specific formats to intermediate formats (e.g., .uasset → .fbx)
4. **Import**: Use standard Atlas importers to convert to .atlasb binary format
5. **Validate**: Hash verification and integrity checks

## Usage

### Basic Import Example

```cpp
#include "engine/assets/MarketplaceImporter.h"

using namespace atlas::asset;

// Create registry and register importers
MarketplaceImportRegistry registry;
registry.RegisterImporter(std::make_unique<ItchIOImporter>());
registry.RegisterImporter(std::make_unique<UnrealMarketplaceImporter>());
registry.RegisterImporter(std::make_unique<UnityAssetStoreImporter>());

// Configure import options
MarketplaceImportOptions options;
options.outputDirectory = "assets/imported/";
options.downloadCache = "/tmp/marketplace_cache";
options.validateAssets = true;
options.preserveMetadata = true;

// Import asset from itch.io
ImportResult result = registry.FetchAndImport(
    MarketplaceType::ItchIO,
    "my-asset-id",
    options
);

if (result.success) {
    std::cout << "Asset imported to: " << result.outputPath << std::endl;
    std::cout << "Asset hash: " << result.outputHash << std::endl;
} else {
    std::cerr << "Import failed: " << result.errorMessage << std::endl;
}
```

### Manual Fetch and Import

```cpp
// Get specific importer
IMarketplaceImporter* importer = registry.FindImporter(MarketplaceType::ItchIO);

// Fetch asset from marketplace
MarketplaceFetchResult fetchResult = importer->FetchAsset("asset-id", options);

if (fetchResult.success) {
    // Inspect metadata
    std::cout << "Asset: " << fetchResult.metadata.name << std::endl;
    std::cout << "Author: " << fetchResult.metadata.author << std::endl;
    std::cout << "Version: " << fetchResult.metadata.version << std::endl;
    
    // Import to Atlas format
    ImportResult importResult = importer->ImportAsset(fetchResult, options);
}
```

## Implementation Details

### Format Conversion

Assets from external marketplaces often use proprietary formats that need conversion:

- **Unreal (.uasset)**: Binary asset format → Intermediate format (FBX/OBJ) → Atlas
- **Unity (.prefab)**: YAML-based prefab format → Intermediate format (OBJ) → Atlas
- **itch.io**: Standard formats (already supported) → Direct import

### Hash Verification

All imported assets are hash-verified to ensure:
- **Immutability**: Assets cannot be modified after import
- **Determinism**: Same asset always produces same hash
- **Security**: Tampered assets are detected and rejected

### Metadata Preservation

Marketplace metadata is preserved during import:
```cpp
struct MarketplaceAssetMetadata {
    std::string id;              // Marketplace-specific ID
    std::string name;            // Asset name
    std::string author;          // Creator/publisher
    std::string version;         // Version string
    std::string description;     // Asset description
    std::vector<std::string> tags; // Categories/tags
    MarketplaceType marketplace; // Source marketplace
};
```

## Asset Validation

The import system performs multiple validation checks:

1. **File existence**: Verify downloaded files exist
2. **Format validation**: Check file extensions match expected formats
3. **Hash verification**: Compute and verify asset hashes
4. **Structural validation**: Parse headers and verify structure
5. **Size limits**: Enforce maximum file sizes

## Caching Strategy

Downloaded assets are cached to avoid redundant downloads:

- **Cache Location**: Configurable via `MarketplaceImportOptions::downloadCache`
- **Cache Key**: Asset ID + marketplace type
- **Cache Invalidation**: Manual or based on version changes
- **Default Location**: System temp directory + `/atlas_marketplace_cache`

## Future Enhancements

### Phase 1 (Current)
- ✅ Basic framework and abstractions
- ✅ itch.io integration (stub)
- ✅ Unreal/Unity stubs for future implementation

### Phase 2 (Complete)
- ✅ Real API integration for Unreal Engine Marketplace
- ✅ Real API integration for Unity Asset Store
- ✅ .uasset binary header parser and converter
- ✅ .prefab YAML parser and converter
- ✅ API credential management
- ✅ Hot-reload monitoring for marketplace assets
- ✅ Asset validation dashboard
- ✅ Mod asset sandboxing with budget enforcement

### Phase 3 (Future)
- [ ] Steam Workshop integration
- [ ] Sketchfab integration
- [ ] Direct HTTP/HTTPS asset downloads
- [ ] Batch import operations
- [ ] Asset update notifications
- [ ] Automatic dependency resolution

## API Setup

### itch.io
itch.io public assets require no API key. For private/paid assets:

1. Generate an API key at https://itch.io/user/settings/api-keys
2. Pass the key via `MarketplaceImportOptions::apiKey`:

```cpp
MarketplaceImportOptions options;
options.apiKey = "YOUR_ITCHIO_API_KEY";
```

### Unreal Engine Marketplace
Requires Epic Games Store API credentials:

1. Obtain an API credential through the Epic Games Developer Portal.
2. Set the credential on the importer:

```cpp
auto unreal = std::make_unique<UnrealMarketplaceImporter>();
unreal->SetApiCredential("YOUR_EPIC_CREDENTIAL");

// Or set via the options:
MarketplaceImportOptions options;
options.apiKey = "YOUR_EPIC_CREDENTIAL";
```

3. The importer validates `.uasset` binary headers (magic number
   `0xC1832A9E`) and extracts asset version and class name.

### Unity Asset Store
Requires Unity Asset Store API credentials:

1. Obtain an API credential from Unity Dashboard → Settings → API keys.
2. Set the credential on the importer:

```cpp
auto unity = std::make_unique<UnityAssetStoreImporter>();
unity->SetApiCredential("YOUR_UNITY_CREDENTIAL");
```

3. The importer parses `.prefab` YAML files and extracts `m_Name`,
   `m_MeshData`, and `m_Materials` keys.

### HTTP Client Configuration

All marketplace importers accept a pluggable HTTP client. Atlas ships
two implementations:

- **NullHttpClient** — always returns errors (for offline/test use).
- **SocketHttpClient** — real POSIX-socket HTTP client with URL
  parsing, timeouts, and connection reuse.

```cpp
SocketHttpClient httpClient;
registry.SetHttpClient(&httpClient);
```

## Hot-Reload Monitoring

The `MarketplaceHotReloader` watches imported assets for file changes
and triggers re-import when modifications are detected:

```cpp
MarketplaceHotReloader reloader;
reloader.Watch("asset123", MarketplaceType::ItchIO,
               "/cache/asset123.png", initialHash);

// In your update loop:
size_t changed = reloader.CheckForUpdates();
for (const auto& dirty : reloader.DirtyAssets()) {
    // Re-import the asset
    registry.FetchAndImport(dirty.marketplace, dirty.assetId, options);
    reloader.ClearDirty(dirty.assetId);
}
```

## Asset Validation Dashboard

The `AssetValidationDashboard` runs a suite of checks on imported
assets (file existence, size, extension recognition, readability):

```cpp
AssetValidationDashboard dash;
auto report = dash.Validate("my_asset", "/path/to/asset.png");

if (report.AllPassed()) {
    std::cout << "All checks passed" << std::endl;
} else {
    std::cout << report.FailCount() << " failures, "
              << report.WarnCount() << " warnings" << std::endl;
}

// Validate all assets in a directory:
auto reports = dash.ValidateDirectory("/cache/marketplace/");
```

## Mod Asset Sandboxing

The `ModAssetSandbox` enforces per-mod resource budgets and
deterministic hash verification:

```cpp
ModAssetSandbox sandbox;
ModSandboxBudget budget;
budget.maxAssetCount = 100;
budget.maxTotalBytes = 64 * 1024 * 1024; // 64 MB

sandbox.RegisterMod("my_mod", budget);

// Assets are hash-verified on add
bool ok = sandbox.AddAsset("my_mod", "/path/to/asset.png", expectedHash);
```

## Security Considerations

All marketplace imports enforce Atlas's deterministic guarantees:

- **No runtime network access**: Assets must be pre-downloaded
- **Hash-based verification**: Assets are immutable
- **Sandbox validation**: Imported assets cannot access filesystem or network
- **No arbitrary code execution**: Assets are data-only

See also:
- [docs/03_ASSET_SYSTEM.md](03_ASSET_SYSTEM.md) - Core asset system
- [docs/ATLAS_CORE_CONTRACT.md](ATLAS_CORE_CONTRACT.md) - Determinism guarantees
- [engine/assets/AssetImporter.h](../engine/assets/AssetImporter.h) - Base importer system
