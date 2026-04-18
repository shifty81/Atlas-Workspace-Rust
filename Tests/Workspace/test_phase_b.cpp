// Phase B — Real Project Load
//
// Tests for:
//   B.1: ProjectBootstrapResult + AtlasProjectFileLoader::bootstrap()
//   B.2: NovaForgeProjectBootstrap service + NovaForgeAdapter wiring
//   B.3: NovaForgeDocumentType / NovaForgeDocument / NovaForgeDocumentRegistry
//   B.4: AssetCatalogPopulator::populateFromDirectory() + panel catalog wiring

#include <catch2/catch_test_macros.hpp>

#include "NF/Workspace/AtlasProjectFileLoader.h"
#include "NF/Workspace/AssetCatalog.h"
#include "NF/Workspace/AssetCatalogPopulator.h"
#include "NF/Workspace/ProjectLoadContract.h"
#include "NovaForge/EditorAdapter/NovaForgeProjectBootstrap.h"
#include "NovaForge/EditorAdapter/NovaForgeAdapter.h"
#include "NovaForge/EditorAdapter/NovaForgeDocument.h"
#include "NovaForge/EditorAdapter/NovaForgeDocumentRegistry.h"
#include "NF/Editor/ContentBrowserPanel.h"
#include "NF/Editor/InspectorPanel.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ── Helpers ────────────────────────────────────────────────────────────────

namespace {

// Write a minimal valid .atlas file to the given path.
void writeValidAtlas(const std::string& path,
                     const std::string& adapter    = "novaforge",
                     const std::string& contentRoot = "Content",
                     const std::string& assetsRoot  = "Assets") {
    std::ofstream f(path);
    f << "{\n"
      << "  \"schema\": \"atlas.project.v1\",\n"
      << "  \"name\": \"TestProject\",\n"
      << "  \"version\": \"1.0.0\",\n"
      << "  \"adapter\": \"" << adapter << "\",\n"
      << "  \"modules\": {\n"
      << "    \"content\": \"" << contentRoot << "\"\n"
      << "  },\n"
      << "  \"assets\": {\n"
      << "    \"root\": \"" << assetsRoot << "\"\n"
      << "  }\n"
      << "}\n";
}

// Create a unique temp directory under the project build dir for test isolation.
fs::path makeTempDir(const std::string& name) {
    fs::path base = fs::temp_directory_path() / ("test_phase_b_" + name);
    fs::create_directories(base);
    return base;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// B.1 — AtlasProjectFileLoader::bootstrap()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("B.1 bootstrap: valid atlas file produces success result", "[phase_b][b1]") {
    auto dir = makeTempDir("b1_valid");
    auto atlasPath = (dir / "test.atlas").string();
    writeValidAtlas(atlasPath);

    NF::AtlasProjectFileLoader loader;
    auto result = loader.bootstrap(atlasPath, false);

    REQUIRE(result.success);
    REQUIRE_FALSE(result.hasErrors());
    REQUIRE(result.manifest.name == "TestProject");
    REQUIRE(result.manifest.adapterId == "novaforge");
}

TEST_CASE("B.1 bootstrap: missing adapterId produces error entry", "[phase_b][b1]") {
    auto dir = makeTempDir("b1_no_adapter");
    auto atlasPath = (dir / "test.atlas").string();
    writeValidAtlas(atlasPath, "");  // empty adapter

    NF::AtlasProjectFileLoader loader;
    auto result = loader.bootstrap(atlasPath, false);

    REQUIRE_FALSE(result.success);
    REQUIRE(result.hasErrors());
    bool hasAdapterError = false;
    for (const auto& e : result.validationEntries) {
        if (e.code == "missing_adapter_id") hasAdapterError = true;
    }
    REQUIRE(hasAdapterError);
}

TEST_CASE("B.1 bootstrap: parse failure produces fatal entry", "[phase_b][b1]") {
    auto dir = makeTempDir("b1_bad_parse");
    auto atlasPath = (dir / "bad.atlas").string();
    // Non-existent file
    NF::AtlasProjectFileLoader loader;
    auto result = loader.bootstrap(atlasPath, false);

    REQUIRE_FALSE(result.success);
    REQUIRE(result.hasErrors());
    bool hasFatal = false;
    for (const auto& e : result.validationEntries) {
        if (e.severity == NF::ProjectValidationSeverity::Fatal) hasFatal = true;
    }
    REQUIRE(hasFatal);
}

TEST_CASE("B.1 bootstrap: checkPathsOnDisk=false skips filesystem checks", "[phase_b][b1]") {
    auto dir = makeTempDir("b1_no_disk_check");
    auto atlasPath = (dir / "test.atlas").string();
    // contentRoot and assetsRoot point to non-existent dirs
    writeValidAtlas(atlasPath, "novaforge", "/nonexistent/content", "/nonexistent/assets");

    NF::AtlasProjectFileLoader loader;
    auto result = loader.bootstrap(atlasPath, false);

    // Should succeed because checkPathsOnDisk=false
    REQUIRE(result.success);
    bool hasMissingRoot = false;
    for (const auto& e : result.validationEntries) {
        if (e.code == "missing_content_root" || e.code == "missing_assets_root")
            hasMissingRoot = true;
    }
    REQUIRE_FALSE(hasMissingRoot);
}

TEST_CASE("B.1 bootstrap: empty contentRoot produces warning entry", "[phase_b][b1]") {
    auto dir = makeTempDir("b1_empty_content");
    auto atlasPath = (dir / "test.atlas").string();
    writeValidAtlas(atlasPath, "novaforge", "", "Assets");

    NF::AtlasProjectFileLoader loader;
    auto result = loader.bootstrap(atlasPath, false);

    bool hasWarning = false;
    for (const auto& e : result.validationEntries) {
        if (e.severity == NF::ProjectValidationSeverity::Warning &&
            e.code == "empty_content_root")
            hasWarning = true;
    }
    REQUIRE(hasWarning);
}

TEST_CASE("B.1 bootstrap: success=false when error entries present", "[phase_b][b1]") {
    auto dir = makeTempDir("b1_success_false");
    auto atlasPath = (dir / "test.atlas").string();
    writeValidAtlas(atlasPath, "");  // no adapter = error

    NF::AtlasProjectFileLoader loader;
    auto result = loader.bootstrap(atlasPath, false);

    REQUIRE_FALSE(result.success);
}

TEST_CASE("B.1 bootstrap: checkPathsOnDisk=true reports missing roots", "[phase_b][b1]") {
    auto dir = makeTempDir("b1_disk_check");
    auto atlasPath = (dir / "test.atlas").string();
    writeValidAtlas(atlasPath, "novaforge", "/definitely/nonexistent/content", "/definitely/nonexistent/assets");

    NF::AtlasProjectFileLoader loader;
    auto result = loader.bootstrap(atlasPath, true);

    bool missingContent = false, missingAssets = false;
    for (const auto& e : result.validationEntries) {
        if (e.code == "missing_content_root") missingContent = true;
        if (e.code == "missing_assets_root")  missingAssets  = true;
    }
    REQUIRE(missingContent);
    REQUIRE(missingAssets);
    REQUIRE_FALSE(result.success);
}

// ─────────────────────────────────────────────────────────────────────────────
// B.2 — NovaForgeProjectBootstrap
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("B.2 bootstrap: non-existent project root produces error for Content/", "[phase_b][b2]") {
    NF::ProjectLoadContract contract;
    NovaForge::NovaForgeProjectBootstrap bs("/totally/nonexistent/project");
    bool ok = bs.run(contract);

    REQUIRE_FALSE(ok);
    bool hasContentError = false;
    for (const auto& e : contract.validationEntries) {
        if (e.code == "missing_content_dir") hasContentError = true;
    }
    REQUIRE(hasContentError);
}

TEST_CASE("B.2 bootstrap: valid project with Content/ and Data/ succeeds", "[phase_b][b2]") {
    auto root = makeTempDir("b2_valid_project");
    fs::create_directories(root / "Content");
    fs::create_directories(root / "Data");
    fs::create_directories(root / "Schema");

    NF::ProjectLoadContract contract;
    NovaForge::NovaForgeProjectBootstrap bs(root.string());
    bool ok = bs.run(contract);

    REQUIRE(ok);
    REQUIRE(contract.isValid());
}

TEST_CASE("B.2 bootstrap: missing Schemas/ produces warning entry", "[phase_b][b2]") {
    auto root = makeTempDir("b2_no_schema");
    fs::create_directories(root / "Content");
    fs::create_directories(root / "Data");
    // No Schemas/

    NF::ProjectLoadContract contract;
    NovaForge::NovaForgeProjectBootstrap bs(root.string());
    bs.run(contract);

    bool hasSchemaWarning = false;
    for (const auto& e : contract.validationEntries) {
        if (e.code == "missing_schemas_dir") hasSchemaWarning = true;
    }
    REQUIRE(hasSchemaWarning);
}

TEST_CASE("B.2 bootstrap: catalog populated from Content/ when provided", "[phase_b][b2]") {
    auto root = makeTempDir("b2_catalog_populate");
    fs::create_directories(root / "Content" / "Textures");
    fs::create_directories(root / "Data");

    // Write a fake texture file
    {
        std::ofstream f((root / "Content" / "Textures" / "rock.png").string());
        f << "fake png data";
    }

    NF::AssetCatalog catalog;
    NF::ProjectLoadContract contract;
    NovaForge::NovaForgeProjectBootstrap bs(root.string(), &catalog);
    bs.run(contract);

    REQUIRE(catalog.count() >= 1u);
    REQUIRE(bs.assetsPopulated() >= 1u);
}

TEST_CASE("B.2 bootstrap: adapter bootstrapContract() has entries after initialize", "[phase_b][b2]") {
    // Use a non-existent path — bootstrap runs but finds no dirs, adding warnings
    NovaForge::NovaForgeAdapter adapter("/nonexistent/novaforge_project");
    adapter.initialize();

    const auto& contract = adapter.bootstrapContract();
    // Should have validation entries (at minimum warnings for missing dirs)
    REQUIRE_FALSE(contract.validationEntries.empty());
}

TEST_CASE("B.2 bootstrap: adapter assetCatalog() is accessible", "[phase_b][b2]") {
    NovaForge::NovaForgeAdapter adapter("/nonexistent/project");
    adapter.initialize();
    // Just verify the accessor doesn't crash
    const NF::AssetCatalog& cat = adapter.assetCatalog();
    (void)cat;
    REQUIRE(true);
}

// ─────────────────────────────────────────────────────────────────────────────
// B.3 — NovaForgeDocument + NovaForgeDocumentRegistry
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("B.3 registry: registerBuiltins produces 22 registrations", "[phase_b][b3]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();
    REQUIRE(reg.count() == 22u);
}

TEST_CASE("B.3 registry: find() works for all 22 types", "[phase_b][b3]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();

    using T = NovaForge::NovaForgeDocumentType;
    const T types[] = {
        T::ItemDefinition, T::StructureArchetype, T::BiomeDefinition,
        T::PlanetArchetype, T::FactionDefinition, T::MissionDefinition,
        T::ProgressionRules, T::CharacterRules, T::EconomyRules,
        T::CraftingDefinition, T::PCGRuleset,
        // v2 types
        T::WorldDocument, T::LevelInstance, T::EntityTemplate,
        T::AssetDocument, T::MaterialDocument, T::AnimationDocument,
        T::EncounterTemplate, T::SpawnProfile, T::PCGContext,
        T::PCGPreset, T::VisualLogicGraph
    };
    for (auto t : types) {
        REQUIRE(reg.find(t) != nullptr);
    }
}

TEST_CASE("B.3 documentTypeName: returns correct strings", "[phase_b][b3]") {
    using T = NovaForge::NovaForgeDocumentType;
    REQUIRE(std::string(NovaForge::documentTypeName(T::ItemDefinition))     == "ItemDefinition");
    REQUIRE(std::string(NovaForge::documentTypeName(T::StructureArchetype)) == "StructureArchetype");
    REQUIRE(std::string(NovaForge::documentTypeName(T::BiomeDefinition))    == "BiomeDefinition");
    REQUIRE(std::string(NovaForge::documentTypeName(T::PlanetArchetype))    == "PlanetArchetype");
    REQUIRE(std::string(NovaForge::documentTypeName(T::FactionDefinition))  == "FactionDefinition");
    REQUIRE(std::string(NovaForge::documentTypeName(T::MissionDefinition))  == "MissionDefinition");
    REQUIRE(std::string(NovaForge::documentTypeName(T::ProgressionRules))   == "ProgressionRules");
    REQUIRE(std::string(NovaForge::documentTypeName(T::CharacterRules))     == "CharacterRules");
    REQUIRE(std::string(NovaForge::documentTypeName(T::EconomyRules))       == "EconomyRules");
    REQUIRE(std::string(NovaForge::documentTypeName(T::CraftingDefinition)) == "CraftingDefinition");
    REQUIRE(std::string(NovaForge::documentTypeName(T::PCGRuleset))         == "PCGRuleset");
    REQUIRE(std::string(NovaForge::documentTypeName(T::WorldDocument))     == "WorldDocument");
    REQUIRE(std::string(NovaForge::documentTypeName(T::LevelInstance))     == "LevelInstance");
    REQUIRE(std::string(NovaForge::documentTypeName(T::EntityTemplate))    == "EntityTemplate");
    REQUIRE(std::string(NovaForge::documentTypeName(T::AssetDocument))     == "AssetDocument");
    REQUIRE(std::string(NovaForge::documentTypeName(T::MaterialDocument))  == "MaterialDocument");
    REQUIRE(std::string(NovaForge::documentTypeName(T::AnimationDocument)) == "AnimationDocument");
    REQUIRE(std::string(NovaForge::documentTypeName(T::EncounterTemplate)) == "EncounterTemplate");
    REQUIRE(std::string(NovaForge::documentTypeName(T::SpawnProfile))      == "SpawnProfile");
    REQUIRE(std::string(NovaForge::documentTypeName(T::PCGContext))        == "PCGContext");
    REQUIRE(std::string(NovaForge::documentTypeName(T::PCGPreset))         == "PCGPreset");
    REQUIRE(std::string(NovaForge::documentTypeName(T::VisualLogicGraph))  == "VisualLogicGraph");
}

TEST_CASE("B.3 document: dirty tracking markDirty/clearDirty/isDirty", "[phase_b][b3]") {
    NovaForge::NovaForgeDocument doc(NovaForge::NovaForgeDocumentType::ItemDefinition, "/path/item.json");
    REQUIRE_FALSE(doc.isDirty());
    doc.markDirty();
    REQUIRE(doc.isDirty());
    doc.clearDirty();
    REQUIRE_FALSE(doc.isDirty());
}

TEST_CASE("B.3 document: save() with default implementation succeeds and clears dirty", "[phase_b][b3]") {
    NovaForge::NovaForgeDocument doc(NovaForge::NovaForgeDocumentType::EconomyRules, "/path/economy.json");
    doc.markDirty();
    REQUIRE(doc.isDirty());
    bool saved = doc.save();
    REQUIRE(saved);
    REQUIRE_FALSE(doc.isDirty());
}

TEST_CASE("B.3 document: revert() clears dirty flag", "[phase_b][b3]") {
    NovaForge::NovaForgeDocument doc(NovaForge::NovaForgeDocumentType::MissionDefinition, "/path/mission.json");
    doc.markDirty();
    bool reverted = doc.revert();
    REQUIRE(reverted);
    REQUIRE_FALSE(doc.isDirty());
}

TEST_CASE("B.3 document: type() and filePath() accessors work", "[phase_b][b3]") {
    NovaForge::NovaForgeDocument doc(NovaForge::NovaForgeDocumentType::BiomeDefinition, "/biomes/forest.biome.json");
    REQUIRE(doc.type() == NovaForge::NovaForgeDocumentType::BiomeDefinition);
    REQUIRE(doc.filePath() == "/biomes/forest.biome.json");
}

TEST_CASE("B.3 registry: createDocument produces valid document", "[phase_b][b3]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();

    auto doc = reg.createDocument(NovaForge::NovaForgeDocumentType::ItemDefinition, "/items/sword.item.json");
    REQUIRE(doc != nullptr);
    REQUIRE(doc->type() == NovaForge::NovaForgeDocumentType::ItemDefinition);
    REQUIRE(doc->filePath() == "/items/sword.item.json");
}

TEST_CASE("B.3 registry: all() returns all 22 registrations", "[phase_b][b3]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();
    auto all = reg.all();
    REQUIRE(all.size() == 22u);
}

// ─────────────────────────────────────────────────────────────────────────────
// B.4 — AssetCatalogPopulator::populateFromDirectory()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("B.4 populateFromDirectory: non-existent dir — filesScanned=0, no crash", "[phase_b][b4]") {
    NF::AssetCatalog catalog;
    NF::AssetCatalogPopulator populator;
    auto result = populator.populateFromDirectory(catalog, "/totally/nonexistent/dir");
    REQUIRE(result.filesScanned == 0u);
    REQUIRE(result.assetsAdded == 0u);
}

TEST_CASE("B.4 populateFromDirectory: real dir with files populates catalog", "[phase_b][b4]") {
    auto dir = makeTempDir("b4_populate");
    fs::create_directories(dir / "Textures");
    {
        std::ofstream f((dir / "Textures" / "rock.png").string());
        f << "fake";
    }
    {
        std::ofstream f((dir / "Textures" / "grass.png").string());
        f << "fake";
    }

    NF::AssetCatalog catalog;
    NF::AssetCatalogPopulator populator;
    auto result = populator.populateFromDirectory(catalog, dir.string());

    REQUIRE(result.filesScanned >= 2u);
    REQUIRE(result.assetsAdded >= 2u);
    REQUIRE(catalog.count() >= 2u);
}

TEST_CASE("B.4 populateFromDirectory: guid metadata is set on populated assets", "[phase_b][b4]") {
    auto dir = makeTempDir("b4_guid");
    {
        std::ofstream f((dir / "mesh.fbx").string());
        f << "fake";
    }

    NF::AssetCatalog catalog;
    NF::AssetCatalogPopulator populator;
    populator.populateFromDirectory(catalog, dir.string());

    REQUIRE(catalog.count() >= 1u);
    auto all = catalog.all();
    REQUIRE_FALSE(all.empty());
    const auto* guid = all[0]->metadata.get("guid");
    REQUIRE(guid != nullptr);
    REQUIRE_FALSE(guid->empty());
}

TEST_CASE("B.4 populateFromDirectory: rootDir metadata is set on populated assets", "[phase_b][b4]") {
    auto dir = makeTempDir("b4_rootdir");
    {
        std::ofstream f((dir / "audio.wav").string());
        f << "fake";
    }

    NF::AssetCatalog catalog;
    NF::AssetCatalogPopulator populator;
    auto rootStr = dir.string();
    populator.populateFromDirectory(catalog, rootStr);

    REQUIRE(catalog.count() >= 1u);
    auto all = catalog.all();
    REQUIRE_FALSE(all.empty());
    const auto* rootMeta = all[0]->metadata.get("rootDir");
    REQUIRE(rootMeta != nullptr);
    REQUIRE(*rootMeta == rootStr);
}

TEST_CASE("B.4 populateFromDirectory: duplicate files counted as duplicates", "[phase_b][b4]") {
    auto dir = makeTempDir("b4_duplicates");
    {
        std::ofstream f((dir / "shader.hlsl").string());
        f << "fake";
    }

    NF::AssetCatalog catalog;
    NF::AssetCatalogPopulator populator;
    // Populate once
    auto r1 = populator.populateFromDirectory(catalog, dir.string());
    REQUIRE(r1.assetsAdded == 1u);

    // Populate again — same file, should be a duplicate
    auto r2 = populator.populateFromDirectory(catalog, dir.string());
    REQUIRE(r2.duplicates == 1u);
    REQUIRE(r2.assetsAdded == 0u);
}

TEST_CASE("B.4 ContentBrowserPanel setAssetCatalog stores the pointer", "[phase_b][b4]") {
    NF::AssetCatalog catalog;
    NF::ContentBrowserPanel panel;
    REQUIRE(panel.assetCatalog() == nullptr);
    panel.setAssetCatalog(&catalog);
    REQUIRE(panel.assetCatalog() == &catalog);
}

TEST_CASE("B.4 InspectorPanel setSelectedAsset stores the descriptor pointer", "[phase_b][b4]") {
    NF::AssetDescriptor desc;
    desc.sourcePath  = "/path/to/asset.png";
    desc.catalogPath = "asset.png";
    desc.typeTag     = NF::AssetTypeTag::Texture;

    NF::InspectorPanel panel;
    REQUIRE(panel.selectedAsset() == nullptr);
    panel.setSelectedAsset(&desc);
    REQUIRE(panel.selectedAsset() == &desc);
    REQUIRE(panel.selectedAsset()->sourcePath == "/path/to/asset.png");
}

// ─────────────────────────────────────────────────────────────────────────────
// Storage Map v1 — New manifest format + document registry paths
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SM v1: v2 atlas format with projectId loads successfully", "[phase_b][storage_map]") {
    auto dir = makeTempDir("smv1_v2_format");
    auto atlasPath = (dir / "project.atlas").string();
    {
        std::ofstream f(atlasPath);
        f << "{\n"
          << "  \"projectId\": \"novaforge\",\n"
          << "  \"projectName\": \"NovaForge\",\n"
          << "  \"projectType\": \"NovaForge\",\n"
          << "  \"projectVersion\": 1,\n"
          << "  \"roots\": {\n"
          << "    \"content\": \"Content\",\n"
          << "    \"data\": \"Data\",\n"
          << "    \"config\": \"Config\",\n"
          << "    \"schemas\": \"Schemas\",\n"
          << "    \"generated\": \"Generated\",\n"
          << "    \"cache\": \"Cache\"\n"
          << "  },\n"
          << "  \"registries\": {\n"
          << "    \"asset\": \"Data/Registries/AssetRegistry.json\",\n"
          << "    \"document\": \"Data/Registries/DocumentRegistry.json\",\n"
          << "    \"schema\": \"Data/Registries/SchemaRegistry.json\",\n"
          << "    \"pcg\": \"Data/Registries/PCGRegistry.json\",\n"
          << "    \"build\": \"Data/Registries/BuildTargetRegistry.json\",\n"
          << "    \"launch\": \"Data/Registries/LaunchTargetRegistry.json\"\n"
          << "  },\n"
          << "  \"startup\": {\n"
          << "    \"layout\": \"novaforge_default\",\n"
          << "    \"tool\": \"scene\",\n"
          << "    \"world\": \"Data/Worlds/DefaultWorld.json\"\n"
          << "  },\n"
          << "  \"config\": {\n"
          << "    \"projectSettings\": \"Config/WorkspaceProjectSettings.json\",\n"
          << "    \"pieSettings\": \"Config/PIESettings.json\",\n"
          << "    \"pcgPreviewDefaults\": \"Config/PCGPreviewDefaults.json\"\n"
          << "  }\n"
          << "}\n";
    }
    NF::AtlasProjectFileLoader loader;
    bool ok = loader.loadFromFile(atlasPath);
    REQUIRE(ok);
    const auto& m = loader.manifest();
    REQUIRE(m.projectId == "novaforge");
    REQUIRE(m.name == "NovaForge");
    REQUIRE(m.adapterId == "novaforge");
    REQUIRE(m.projectVersion == 1);
    REQUIRE(m.dataRoot == "Data");
    REQUIRE(m.configRoot == "Config");
    REQUIRE(m.schemasRoot == "Schemas");
    REQUIRE(m.generatedRoot == "Generated");
    REQUIRE(m.cacheRoot == "Cache");
    REQUIRE(m.registryAsset == "Data/Registries/AssetRegistry.json");
    REQUIRE(m.registryDocument == "Data/Registries/DocumentRegistry.json");
    REQUIRE(m.registrySchema == "Data/Registries/SchemaRegistry.json");
    REQUIRE(m.registryPCG == "Data/Registries/PCGRegistry.json");
    REQUIRE(m.registryBuild == "Data/Registries/BuildTargetRegistry.json");
    REQUIRE(m.registryLaunch == "Data/Registries/LaunchTargetRegistry.json");
    REQUIRE(m.startupLayout == "novaforge_default");
    REQUIRE(m.startupTool == "scene");
    REQUIRE(m.startupWorld == "Data/Worlds/DefaultWorld.json");
    REQUIRE(m.configProjectSettings == "Config/WorkspaceProjectSettings.json");
    REQUIRE(m.configPIESettings == "Config/PIESettings.json");
    REQUIRE(m.configPCGPreviewDefaults == "Config/PCGPreviewDefaults.json");
}

TEST_CASE("SM v1: v2 atlas format bootstrap() succeeds with no errors", "[phase_b][storage_map]") {
    auto dir = makeTempDir("smv1_bootstrap");
    auto atlasPath = (dir / "project.atlas").string();
    {
        std::ofstream f(atlasPath);
        f << "{\n"
          << "  \"projectId\": \"testproject\",\n"
          << "  \"projectName\": \"Test Project\",\n"
          << "  \"projectType\": \"TestAdapter\",\n"
          << "  \"projectVersion\": 1,\n"
          << "  \"roots\": { \"content\": \"Content\", \"data\": \"Data\" }\n"
          << "}\n";
    }
    NF::AtlasProjectFileLoader loader;
    auto result = loader.bootstrap(atlasPath, false);
    REQUIRE(result.success);
    REQUIRE_FALSE(result.hasErrors());
    REQUIRE(result.manifest.projectId == "testproject");
    REQUIRE(result.manifest.name == "Test Project");
    REQUIRE(result.manifest.adapterId == "testadapter"); // projectType lowercased
}

TEST_CASE("SM v1: document registry Rules/ path for CharacterRules", "[phase_b][storage_map]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();
    const auto* r = reg.find(NovaForge::NovaForgeDocumentType::CharacterRules);
    REQUIRE(r != nullptr);
    REQUIRE(r->dataSubdir == "Rules");
}

TEST_CASE("SM v1: document registry Rules/ path for EconomyRules", "[phase_b][storage_map]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();
    const auto* r = reg.find(NovaForge::NovaForgeDocumentType::EconomyRules);
    REQUIRE(r != nullptr);
    REQUIRE(r->dataSubdir == "Rules");
}

TEST_CASE("SM v1: document registry Rules/ path for ProgressionRules", "[phase_b][storage_map]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();
    const auto* r = reg.find(NovaForge::NovaForgeDocumentType::ProgressionRules);
    REQUIRE(r != nullptr);
    REQUIRE(r->dataSubdir == "Rules");
}

TEST_CASE("SM v1: document registry PCG/RuleSets path for PCGRuleset", "[phase_b][storage_map]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();
    const auto* r = reg.find(NovaForge::NovaForgeDocumentType::PCGRuleset);
    REQUIRE(r != nullptr);
    REQUIRE(r->dataSubdir == "PCG/RuleSets");
}

TEST_CASE("SM v1: document registry Worlds/ path for WorldDocument", "[phase_b][storage_map]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();
    const auto* r = reg.find(NovaForge::NovaForgeDocumentType::WorldDocument);
    REQUIRE(r != nullptr);
    REQUIRE(r->dataSubdir == "Worlds");
}

TEST_CASE("SM v1: document registry PCG/Contexts path for PCGContext", "[phase_b][storage_map]") {
    NovaForge::NovaForgeDocumentRegistry reg;
    reg.registerBuiltins();
    const auto* r = reg.find(NovaForge::NovaForgeDocumentType::PCGContext);
    REQUIRE(r != nullptr);
    REQUIRE(r->dataSubdir == "PCG/Contexts");
}

TEST_CASE("SM v1: bootstrap validates Config/ directory", "[phase_b][storage_map]") {
    auto root = makeTempDir("smv1_config_check");
    fs::create_directories(root / "Content");
    fs::create_directories(root / "Data");
    // No Config/ — should produce warning

    NF::ProjectLoadContract contract;
    NovaForge::NovaForgeProjectBootstrap bs(root.string());
    bs.run(contract);

    bool hasConfigWarning = false;
    for (const auto& e : contract.validationEntries) {
        if (e.code == "missing_config_dir") hasConfigWarning = true;
    }
    REQUIRE(hasConfigWarning);
}

TEST_CASE("SM v1: bootstrap validates Schemas/ directory (plural)", "[phase_b][storage_map]") {
    auto root = makeTempDir("smv1_schemas_check");
    fs::create_directories(root / "Content");
    fs::create_directories(root / "Data");
    fs::create_directories(root / "Config");
    // No Schemas/ — should produce warning

    NF::ProjectLoadContract contract;
    NovaForge::NovaForgeProjectBootstrap bs(root.string());
    bs.run(contract);

    bool hasSchemasWarning = false;
    for (const auto& e : contract.validationEntries) {
        if (e.code == "missing_schemas_dir") hasSchemasWarning = true;
    }
    REQUIRE(hasSchemasWarning);
}

TEST_CASE("SM v1: bootstrap records Generated/ info entry when present", "[phase_b][storage_map]") {
    auto root = makeTempDir("smv1_generated_check");
    fs::create_directories(root / "Content");
    fs::create_directories(root / "Data");
    fs::create_directories(root / "Generated");

    NF::ProjectLoadContract contract;
    NovaForge::NovaForgeProjectBootstrap bs(root.string());
    bs.run(contract);

    bool hasGeneratedInfo = false;
    for (const auto& e : contract.validationEntries) {
        if (e.code == "generated_dir_found") hasGeneratedInfo = true;
    }
    REQUIRE(hasGeneratedInfo);
}

TEST_CASE("SM v1: full project dir layout passes bootstrap", "[phase_b][storage_map]") {
    auto root = makeTempDir("smv1_full_layout");
    fs::create_directories(root / "Content" / "Meshes");
    fs::create_directories(root / "Data" / "Worlds");
    fs::create_directories(root / "Data" / "Rules");
    fs::create_directories(root / "Data" / "PCG" / "RuleSets");
    fs::create_directories(root / "Data" / "Registries");
    fs::create_directories(root / "Config");
    fs::create_directories(root / "Schemas");
    fs::create_directories(root / "Generated");
    fs::create_directories(root / "Cache");

    NF::ProjectLoadContract contract;
    NovaForge::NovaForgeProjectBootstrap bs(root.string());
    bool ok = bs.run(contract);

    REQUIRE(ok);
    REQUIRE(contract.isValid());
    // Should have no errors or fatals
    REQUIRE(contract.countBySeverity(NF::ProjectValidationSeverity::Error) == 0u);
    REQUIRE(contract.countBySeverity(NF::ProjectValidationSeverity::Fatal) == 0u);
}
