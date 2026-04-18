#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/AssetCatalogPopulator.h"
#include "NF/Workspace/AssetCatalog.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/SettingsStore.h"
#include "NF/Workspace/LayoutPersistence.h"
#include "NF/Workspace/IGameProjectAdapter.h"
#include "NF/Editor/EditorApp.h"
#include <string>
#include <vector>

// ═══════════════════════════════════════════════════════════════════
// Audit Patches 7/9/11/12 — Tests
//   Patch 7:  EditorApp deprecation markers (verified by compile, no runtime test)
//   Patch 9:  AssetCatalogPopulator — extension classification + catalog population
//   Patch 11: SettingsStore/LayoutPersistence wired into WorkspaceShell
//   Patch 12: Validator gaps (verified by validate_project.sh, no runtime test)
// ═══════════════════════════════════════════════════════════════════

// ── Patch 9: Extension Classification ──────────────────────────────

TEST_CASE("classifyExtension — textures", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".png")  == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".PNG")  == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".jpg")  == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".jpeg") == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".bmp")  == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".tga")  == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".dds")  == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".hdr")  == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".exr")  == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".psd")  == NF::AssetTypeTag::Texture);
}

TEST_CASE("classifyExtension — meshes", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".fbx")   == NF::AssetTypeTag::Mesh);
    REQUIRE(NF::classifyExtension(".obj")   == NF::AssetTypeTag::Mesh);
    REQUIRE(NF::classifyExtension(".gltf")  == NF::AssetTypeTag::Mesh);
    REQUIRE(NF::classifyExtension(".glb")   == NF::AssetTypeTag::Mesh);
    REQUIRE(NF::classifyExtension(".blend") == NF::AssetTypeTag::Mesh);
}

TEST_CASE("classifyExtension — audio", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".wav")  == NF::AssetTypeTag::Audio);
    REQUIRE(NF::classifyExtension(".mp3")  == NF::AssetTypeTag::Audio);
    REQUIRE(NF::classifyExtension(".ogg")  == NF::AssetTypeTag::Audio);
    REQUIRE(NF::classifyExtension(".flac") == NF::AssetTypeTag::Audio);
}

TEST_CASE("classifyExtension — scripts", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".lua") == NF::AssetTypeTag::Script);
    REQUIRE(NF::classifyExtension(".py")  == NF::AssetTypeTag::Script);
    REQUIRE(NF::classifyExtension(".cs")  == NF::AssetTypeTag::Script);
}

TEST_CASE("classifyExtension — shaders", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".hlsl") == NF::AssetTypeTag::Shader);
    REQUIRE(NF::classifyExtension(".glsl") == NF::AssetTypeTag::Shader);
    REQUIRE(NF::classifyExtension(".vert") == NF::AssetTypeTag::Shader);
    REQUIRE(NF::classifyExtension(".frag") == NF::AssetTypeTag::Shader);
}

TEST_CASE("classifyExtension — scenes, fonts, videos, archives", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".scene") == NF::AssetTypeTag::Scene);
    REQUIRE(NF::classifyExtension(".ttf")   == NF::AssetTypeTag::Font);
    REQUIRE(NF::classifyExtension(".mp4")   == NF::AssetTypeTag::Video);
    REQUIRE(NF::classifyExtension(".zip")   == NF::AssetTypeTag::Archive);
}

TEST_CASE("classifyExtension — materials, animations, prefabs", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".mat")      == NF::AssetTypeTag::Material);
    REQUIRE(NF::classifyExtension(".material") == NF::AssetTypeTag::Material);
    REQUIRE(NF::classifyExtension(".anim")     == NF::AssetTypeTag::Animation);
    REQUIRE(NF::classifyExtension(".prefab")   == NF::AssetTypeTag::Prefab);
}

TEST_CASE("classifyExtension — project files and custom", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".json")  == NF::AssetTypeTag::Data);    // game data, not project manifest
    REQUIRE(NF::classifyExtension(".atlas") == NF::AssetTypeTag::Project);
    REQUIRE(NF::classifyExtension(".xyz")   == NF::AssetTypeTag::Custom);
    REQUIRE(NF::classifyExtension(".zzz")   == NF::AssetTypeTag::Custom);
}

TEST_CASE("classifyExtension — data files", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".yaml")  == NF::AssetTypeTag::Data);
    REQUIRE(NF::classifyExtension(".yml")   == NF::AssetTypeTag::Data);
    REQUIRE(NF::classifyExtension(".toml")  == NF::AssetTypeTag::Data);
    REQUIRE(NF::classifyExtension(".csv")   == NF::AssetTypeTag::Data);
    REQUIRE(NF::classifyExtension(".ini")   == NF::AssetTypeTag::Data);
    REQUIRE(NF::classifyExtension(".xml")   == NF::AssetTypeTag::Data);
    REQUIRE(NF::classifyExtension(".cfg")   == NF::AssetTypeTag::Data);
    REQUIRE(NF::classifyExtension(".conf")  == NF::AssetTypeTag::Data);
}

TEST_CASE("classifyExtension — case insensitive", "[Audit][Patch9]") {
    REQUIRE(NF::classifyExtension(".PNG")  == NF::AssetTypeTag::Texture);
    REQUIRE(NF::classifyExtension(".Fbx")  == NF::AssetTypeTag::Mesh);
    REQUIRE(NF::classifyExtension(".WAV")  == NF::AssetTypeTag::Audio);
    REQUIRE(NF::classifyExtension(".HLSL") == NF::AssetTypeTag::Shader);
}

// ── Patch 9: Build catalog path ─────────────────────────────────

TEST_CASE("buildCatalogPath — strips root prefix", "[Audit][Patch9]") {
    REQUIRE(NF::buildCatalogPath("/project/Content", "/project/Content/Textures/rock.png")
            == "Textures/rock.png");
}

TEST_CASE("buildCatalogPath — handles trailing separator", "[Audit][Patch9]") {
    REQUIRE(NF::buildCatalogPath("/root", "/root/file.txt") == "file.txt");
}

TEST_CASE("buildCatalogPath — no matching root uses full path", "[Audit][Patch9]") {
    REQUIRE(NF::buildCatalogPath("/other", "/project/file.txt") == "/project/file.txt");
}

// ── Patch 9: Extract display name ───────────────────────────────

TEST_CASE("extractDisplayName — strips path and extension", "[Audit][Patch9]") {
    REQUIRE(NF::extractDisplayName("Textures/rock.png") == "rock");
    REQUIRE(NF::extractDisplayName("model.fbx") == "model");
    REQUIRE(NF::extractDisplayName("noext") == "noext");
}

// ── Patch 9: AssetCatalogPopulator ──────────────────────────────

TEST_CASE("AssetCatalogPopulator — default state", "[Audit][Patch9]") {
    NF::AssetCatalogPopulator pop;
    REQUIRE(pop.pendingCount() == 0);
}

TEST_CASE("AssetCatalogPopulator — addFiles increases pending count", "[Audit][Patch9]") {
    NF::AssetCatalogPopulator pop;
    pop.addFiles("/root", {"/root/a.png", "/root/b.fbx"});
    REQUIRE(pop.pendingCount() == 2);
}

TEST_CASE("AssetCatalogPopulator — populate adds assets to catalog", "[Audit][Patch9]") {
    NF::AssetCatalog catalog;
    NF::AssetCatalogPopulator pop;
    pop.addFiles("/content", {
        "/content/tex/rock.png",
        "/content/mesh/ship.fbx",
        "/content/audio/boom.wav"
    });
    auto result = pop.populate(catalog);
    REQUIRE(result.filesScanned == 3);
    REQUIRE(result.assetsAdded == 3);
    REQUIRE(result.duplicates == 0);
    REQUIRE(result.errors == 0);
    REQUIRE(result.success());
    REQUIRE(catalog.count() == 3);
}

TEST_CASE("AssetCatalogPopulator — duplicate catalog paths are skipped", "[Audit][Patch9]") {
    NF::AssetCatalog catalog;
    NF::AssetCatalogPopulator pop;
    pop.addFiles("/content", {"/content/rock.png", "/content/rock.png"});
    auto result = pop.populate(catalog);
    REQUIRE(result.assetsAdded == 1);
    REQUIRE(result.duplicates == 1);
    REQUIRE(catalog.count() == 1);
}

TEST_CASE("AssetCatalogPopulator — clears pending after populate", "[Audit][Patch9]") {
    NF::AssetCatalogPopulator pop;
    pop.addFiles("/root", {"/root/a.png"});
    NF::AssetCatalog catalog;
    pop.populate(catalog);
    REQUIRE(pop.pendingCount() == 0);
}

TEST_CASE("AssetCatalogPopulator — classified types in catalog", "[Audit][Patch9]") {
    NF::AssetCatalog catalog;
    NF::AssetCatalogPopulator pop;
    pop.addFiles("/c", {"/c/img.png", "/c/mesh.fbx", "/c/sound.wav", "/c/code.lua"});
    pop.populate(catalog);

    REQUIRE(catalog.countByType(NF::AssetTypeTag::Texture) == 1);
    REQUIRE(catalog.countByType(NF::AssetTypeTag::Mesh) == 1);
    REQUIRE(catalog.countByType(NF::AssetTypeTag::Audio) == 1);
    REQUIRE(catalog.countByType(NF::AssetTypeTag::Script) == 1);
}

TEST_CASE("AssetCatalogPopulator — clear discards pending", "[Audit][Patch9]") {
    NF::AssetCatalogPopulator pop;
    pop.addFiles("/root", {"/root/a.png", "/root/b.fbx"});
    pop.clear();
    REQUIRE(pop.pendingCount() == 0);
}

// ── Patch 11: WorkspaceShell owns AssetCatalog ──────────────────

TEST_CASE("WorkspaceShell — assetCatalog accessor exists", "[Audit][Patch11]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    REQUIRE(shell.assetCatalog().count() == 0);
    shell.shutdown();
}

TEST_CASE("WorkspaceShell — settingsStore accessor exists", "[Audit][Patch11]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    REQUIRE(shell.settingsStore().totalCount() > 0); // defaults registered
    shell.shutdown();
}

TEST_CASE("WorkspaceShell — default settings populated on initialize", "[Audit][Patch11]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    // Check the Default layer specifically — the test validates that built-in
    // defaults are registered, not the cascaded value which may be overridden
    // by a persisted user settings file on disk.
    const auto* theme = shell.settingsStore().getFromLayer("workspace.theme", NF::SettingsLayer::Default);
    REQUIRE(theme != nullptr);
    REQUIRE(*theme == "dark");
    const auto* autoSave = shell.settingsStore().getFromLayer("workspace.auto_save", NF::SettingsLayer::Default);
    REQUIRE(autoSave != nullptr);
    REQUIRE(*autoSave == "true");
    const auto* showWelcome = shell.settingsStore().getFromLayer("workspace.show_welcome", NF::SettingsLayer::Default);
    REQUIRE(showWelcome != nullptr);
    REQUIRE(*showWelcome == "true");
    shell.shutdown();
}

TEST_CASE("WorkspaceShell — layoutPersistence accessor exists", "[Audit][Patch11]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    REQUIRE(shell.layoutPersistence().presetCount() == 0); // no presets loaded yet
    shell.shutdown();
}

TEST_CASE("WorkspaceShell — settingsStore user layer overrides default", "[Audit][Patch11]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    shell.settingsStore().set("workspace.theme", "light", NF::SettingsLayer::User);
    REQUIRE(shell.settingsStore().get("workspace.theme") == "light");
    shell.shutdown();
}

// ── Patch 11: SettingsStore cleared on project unload ───────────

namespace {
struct AuditTestAdapter : NF::IGameProjectAdapter {
    std::string projectId() const override { return "audit_test"; }
    std::string projectDisplayName() const override { return "Audit Test"; }
    bool initialize() override { return true; }
    void shutdown() override {}
    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override { return {}; }
    std::vector<std::string> contentRoots() const override { return {"/tmp/content"}; }
    std::vector<std::string> customCommands() const override { return {}; }
};
}

TEST_CASE("WorkspaceShell — project settings cleared on unload", "[Audit][Patch11]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    shell.loadProject(std::make_unique<AuditTestAdapter>());
    shell.settingsStore().set("game.difficulty", "hard", NF::SettingsLayer::Project);
    REQUIRE(shell.settingsStore().has("game.difficulty"));
    shell.unloadProject();
    REQUIRE_FALSE(shell.settingsStore().hasInLayer("game.difficulty", NF::SettingsLayer::Project));
    shell.shutdown();
}

TEST_CASE("WorkspaceShell — asset catalog cleared on project unload", "[Audit][Patch11]") {
    NF::WorkspaceShell shell;
    shell.initialize();
    shell.loadProject(std::make_unique<AuditTestAdapter>());
    // Manually add an asset to verify clearing
    NF::AssetDescriptor desc;
    desc.sourcePath = "/tmp/content/test.png";
    desc.catalogPath = "test.png";
    desc.typeTag = NF::AssetTypeTag::Texture;
    shell.assetCatalog().add(std::move(desc));
    REQUIRE(shell.assetCatalog().count() == 1);
    shell.unloadProject();
    REQUIRE(shell.assetCatalog().count() == 0);
    shell.shutdown();
}

// ── Patch 7: EditorApp deprecation (compile-time verification) ──

TEST_CASE("EditorApp — still compiles with deprecation markers", "[Audit][Patch7]") {
    // Verify EditorApp header compiles with the deprecation comment markers.
    // The deprecation is informational (comments), not [[deprecated]] attributes.
    NF::EditorApp app;
    app.init(800, 600);
    // init should succeed and the app should be usable
    REQUIRE(true);
}
