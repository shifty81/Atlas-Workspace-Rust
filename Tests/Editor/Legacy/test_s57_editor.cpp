#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include <filesystem>
#include <fstream>

using namespace NF;

// ── ContentEntryType ─────────────────────────────────────────────

TEST_CASE("ContentEntryType enum values exist", "[Editor][S57]") {
    REQUIRE(ContentEntryType::Directory != ContentEntryType::Scene);
    REQUIRE(ContentEntryType::Mesh      != ContentEntryType::Material);
    REQUIRE(ContentEntryType::Texture   != ContentEntryType::Audio);
    REQUIRE(ContentEntryType::Script    != ContentEntryType::Data);
    REQUIRE(ContentEntryType::Unknown   != ContentEntryType::Directory);
}

// ── ContentBrowser::classifyExtension ────────────────────────────

TEST_CASE("ContentBrowser classifies scene extensions", "[Editor][S57]") {
    REQUIRE(ContentBrowser::classifyExtension(".scene") == ContentEntryType::Scene);
    REQUIRE(ContentBrowser::classifyExtension(".world") == ContentEntryType::Scene);
}

TEST_CASE("ContentBrowser classifies mesh extensions", "[Editor][S57]") {
    REQUIRE(ContentBrowser::classifyExtension(".obj")  == ContentEntryType::Mesh);
    REQUIRE(ContentBrowser::classifyExtension(".fbx")  == ContentEntryType::Mesh);
    REQUIRE(ContentBrowser::classifyExtension(".mesh") == ContentEntryType::Mesh);
}

TEST_CASE("ContentBrowser classifies texture extensions", "[Editor][S57]") {
    REQUIRE(ContentBrowser::classifyExtension(".png") == ContentEntryType::Texture);
    REQUIRE(ContentBrowser::classifyExtension(".jpg") == ContentEntryType::Texture);
    REQUIRE(ContentBrowser::classifyExtension(".tga") == ContentEntryType::Texture);
}

TEST_CASE("ContentBrowser classifies audio extensions", "[Editor][S57]") {
    REQUIRE(ContentBrowser::classifyExtension(".wav") == ContentEntryType::Audio);
    REQUIRE(ContentBrowser::classifyExtension(".ogg") == ContentEntryType::Audio);
}

TEST_CASE("ContentBrowser classifies script extensions", "[Editor][S57]") {
    REQUIRE(ContentBrowser::classifyExtension(".lua")   == ContentEntryType::Script);
    REQUIRE(ContentBrowser::classifyExtension(".graph") == ContentEntryType::Script);
}

TEST_CASE("ContentBrowser classifies data extensions", "[Editor][S57]") {
    REQUIRE(ContentBrowser::classifyExtension(".json") == ContentEntryType::Data);
}

TEST_CASE("ContentBrowser unknown extension returns Unknown", "[Editor][S57]") {
    REQUIRE(ContentBrowser::classifyExtension(".xyz") == ContentEntryType::Unknown);
    REQUIRE(ContentBrowser::classifyExtension("")     == ContentEntryType::Unknown);
}

// ── ContentBrowser ───────────────────────────────────────────────

TEST_CASE("ContentBrowser starts empty with no root", "[Editor][S57]") {
    ContentBrowser cb;
    REQUIRE(cb.rootPath().empty());
    REQUIRE(cb.currentPath().empty());
    REQUIRE(cb.entryCount() == 0);
}

TEST_CASE("ContentBrowser setRootPath sets root and current", "[Editor][S57]") {
    ContentBrowser cb;
    cb.setRootPath("/tmp");
    REQUIRE(cb.rootPath() == "/tmp");
    REQUIRE(cb.currentPath() == "/tmp");
    REQUIRE(cb.isAtRoot());
}

TEST_CASE("ContentBrowser navigateTo invalid path returns false", "[Editor][S57]") {
    ContentBrowser cb;
    cb.setRootPath("/tmp");
    REQUIRE_FALSE(cb.navigateTo("/nonexistent_path_12345_xyz"));
    REQUIRE(cb.currentPath() == "/tmp");
}

TEST_CASE("ContentBrowser navigateTo valid directory succeeds", "[Editor][S57]") {
    std::string dir = "/tmp/nf_s57_cb_test";
    std::filesystem::create_directories(dir);

    ContentBrowser cb;
    cb.setRootPath("/tmp");
    bool ok = cb.navigateTo(dir);
    REQUIRE(ok);
    REQUIRE(cb.currentPath() == dir);
    REQUIRE_FALSE(cb.isAtRoot());

    std::filesystem::remove_all(dir);
}

TEST_CASE("ContentBrowser navigateUp returns to parent", "[Editor][S57]") {
    std::string dir = "/tmp/nf_s57_cb_nav";
    std::filesystem::create_directories(dir);

    ContentBrowser cb;
    cb.setRootPath("/tmp");
    cb.navigateTo(dir);
    REQUIRE_FALSE(cb.isAtRoot());

    bool up = cb.navigateUp();
    REQUIRE(up);
    REQUIRE(cb.currentPath() == "/tmp");
    REQUIRE(cb.isAtRoot());

    std::filesystem::remove_all(dir);
}

TEST_CASE("ContentBrowser navigateUp at root returns false", "[Editor][S57]") {
    ContentBrowser cb;
    cb.setRootPath("/tmp");
    REQUIRE_FALSE(cb.navigateUp());
}

TEST_CASE("ContentBrowser refresh lists files in directory", "[Editor][S57]") {
    std::string dir = "/tmp/nf_s57_refresh";
    std::filesystem::create_directories(dir);
    { std::ofstream f(dir + "/model.obj");  f << "v 0 0 0"; }
    { std::ofstream f(dir + "/scene.scene"); f << "{}"; }

    ContentBrowser cb;
    cb.setRootPath(dir);
    cb.refresh();
    REQUIRE(cb.entryCount() == 2);

    std::filesystem::remove_all(dir);
}

// ── ProjectPathService ───────────────────────────────────────────

TEST_CASE("ProjectPathService defaults are empty before init", "[Editor][S57]") {
    ProjectPathService pps;
    REQUIRE(pps.projectRoot().empty());
    REQUIRE(pps.contentPath().empty());
    REQUIRE(pps.dataPath().empty());
    REQUIRE(pps.configPath().empty());
}

TEST_CASE("ProjectPathService init sets executable path", "[Editor][S57]") {
    ProjectPathService pps;
    pps.init("/usr/bin/myeditor");
    REQUIRE(pps.executablePath() == "/usr/bin/myeditor");
}

TEST_CASE("ProjectPathService resolvePath returns non-empty string", "[Editor][S57]") {
    ProjectPathService pps;
    pps.init("/tmp/nf_s57_fake_exe");
    std::string resolved = pps.resolvePath("Content/Textures");
    REQUIRE_FALSE(resolved.empty());
}

// ── LaunchService ────────────────────────────────────────────────

TEST_CASE("LaunchService setGameExecutableName and setBuildDirectory", "[Editor][S57]") {
    LaunchService ls;
    ls.setGameExecutableName("AtlasGame");
    ls.setBuildDirectory("/tmp/build");
    std::string path = ls.resolveGamePath();
    // Path is either empty (exe not found) or contains the exe name
    bool valid = path.empty() || (path.find("AtlasGame") != std::string::npos);
    REQUIRE(valid);
}

// ── RecentFilesList ──────────────────────────────────────────────

TEST_CASE("RecentFilesList starts empty", "[Editor][S57]") {
    RecentFilesList rfl;
    REQUIRE(rfl.empty());
    REQUIRE(rfl.count() == 0);
}

TEST_CASE("RecentFilesList addFile inserts at front", "[Editor][S57]") {
    RecentFilesList rfl;
    rfl.addFile("/project/a.scene");
    rfl.addFile("/project/b.scene");
    REQUIRE(rfl.count() == 2);
    REQUIRE(rfl.files()[0] == "/project/b.scene");
    REQUIRE(rfl.files()[1] == "/project/a.scene");
}

TEST_CASE("RecentFilesList addFile deduplicates", "[Editor][S57]") {
    RecentFilesList rfl;
    rfl.addFile("/project/a.scene");
    rfl.addFile("/project/b.scene");
    rfl.addFile("/project/a.scene");
    REQUIRE(rfl.count() == 2);
    REQUIRE(rfl.files()[0] == "/project/a.scene");
}

TEST_CASE("RecentFilesList respects max entries", "[Editor][S57]") {
    RecentFilesList rfl;
    rfl.setMaxEntries(3);
    for (int i = 0; i < 5; ++i) {
        rfl.addFile("/project/" + std::to_string(i) + ".scene");
    }
    REQUIRE(rfl.count() == 3);
}

TEST_CASE("RecentFilesList clear empties the list", "[Editor][S57]") {
    RecentFilesList rfl;
    rfl.addFile("/project/a.scene");
    rfl.clear();
    REQUIRE(rfl.empty());
}
