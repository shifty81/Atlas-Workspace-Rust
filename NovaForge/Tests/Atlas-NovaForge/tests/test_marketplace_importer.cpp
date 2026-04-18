#include "../engine/assets/MarketplaceImporter.h"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

using namespace atlas::asset;

// Test suite for marketplace asset importing system
// Tests cover registry, importers, metadata handling, and options

void test_marketplace_registry() {
    MarketplaceImportRegistry registry;
    
    // Register importers
    registry.RegisterImporter(std::make_unique<ItchIOImporter>());
    registry.RegisterImporter(std::make_unique<UnrealMarketplaceImporter>());
    registry.RegisterImporter(std::make_unique<UnityAssetStoreImporter>());
    
    assert(registry.ImporterCount() == 3);
    
    // Check itch.io importer is available (no API key required)
    auto availableMarkets = registry.AvailableMarketplaces();
    assert(!availableMarkets.empty());
    
    // Find specific importers
    auto itchImporter = registry.FindImporter(MarketplaceType::ItchIO);
    assert(itchImporter != nullptr);
    assert(itchImporter->GetMarketplaceType() == MarketplaceType::ItchIO);
    assert(itchImporter->IsAvailable() == true);
    
    auto unrealImporter = registry.FindImporter(MarketplaceType::UnrealEngine);
    assert(unrealImporter != nullptr);
    assert(unrealImporter->GetMarketplaceType() == MarketplaceType::UnrealEngine);
    // Unreal requires API integration, so not available by default
    assert(unrealImporter->IsAvailable() == false);
    
    auto unityImporter = registry.FindImporter(MarketplaceType::UnityAssetStore);
    assert(unityImporter != nullptr);
    assert(unityImporter->GetMarketplaceType() == MarketplaceType::UnityAssetStore);
    // Unity requires API integration, so not available by default
    assert(unityImporter->IsAvailable() == false);
    
    std::cout << "[PASS] test_marketplace_registry" << std::endl;
}

void test_itch_io_importer() {
    ItchIOImporter importer;
    
    assert(importer.GetMarketplaceType() == MarketplaceType::ItchIO);
    assert(importer.IsAvailable() == true);
    
    // Create a test asset in cache
    std::string cacheDir = std::filesystem::temp_directory_path().string() + "/atlas_marketplace_cache";
    std::filesystem::create_directories(cacheDir);
    
    std::string testAssetPath = cacheDir + "/test_asset.png";
    std::ofstream testFile(testAssetPath);
    testFile << "fake png data";
    testFile.close();
    
    // Try to fetch the asset
    MarketplaceImportOptions options;
    options.downloadCache = cacheDir;
    
    auto fetchResult = importer.FetchAsset("test_asset.png", options);
    assert(fetchResult.success == true);
    assert(fetchResult.localPath == testAssetPath);
    assert(fetchResult.metadata.marketplace == MarketplaceType::ItchIO);
    assert(fetchResult.metadata.id == "test_asset.png");
    
    // Cleanup
    std::filesystem::remove(testAssetPath);
    
    std::cout << "[PASS] test_itch_io_importer" << std::endl;
}

void test_marketplace_metadata() {
    MarketplaceAssetMetadata metadata;
    metadata.id = "asset_12345";
    metadata.name = "Cool Game Asset";
    metadata.author = "AssetCreator";
    metadata.version = "1.0.0";
    metadata.description = "A great asset for games";
    metadata.tags = {"3D", "Characters", "Fantasy"};
    metadata.marketplace = MarketplaceType::UnityAssetStore;
    
    assert(metadata.id == "asset_12345");
    assert(metadata.name == "Cool Game Asset");
    assert(metadata.marketplace == MarketplaceType::UnityAssetStore);
    assert(metadata.tags.size() == 3);
    
    std::cout << "[PASS] test_marketplace_metadata" << std::endl;
}

void test_marketplace_import_options() {
    MarketplaceImportOptions options;
    options.validateAssets = true;
    options.preserveMetadata = true;
    options.apiKey = "test_api_key_12345";
    options.downloadCache = "/tmp/marketplace_cache";
    options.generateMipmaps = true;
    options.compressTextures = false;
    
    assert(options.validateAssets == true);
    assert(options.preserveMetadata == true);
    assert(options.apiKey == "test_api_key_12345");
    assert(options.downloadCache == "/tmp/marketplace_cache");
    
    std::cout << "[PASS] test_marketplace_import_options" << std::endl;
}

void test_unreal_marketplace_importer() {
    UnrealMarketplaceImporter importer;
    
    assert(importer.GetMarketplaceType() == MarketplaceType::UnrealEngine);
    // Should not be available without API integration
    assert(importer.IsAvailable() == false);
    
    std::cout << "[PASS] test_unreal_marketplace_importer" << std::endl;
}

void test_unity_assetstore_importer() {
    UnityAssetStoreImporter importer;
    
    assert(importer.GetMarketplaceType() == MarketplaceType::UnityAssetStore);
    // Should not be available without API integration
    assert(importer.IsAvailable() == false);
    
    std::cout << "[PASS] test_unity_assetstore_importer" << std::endl;
}

// Marketplace importer test functions
// These functions are called from tests/main.cpp
