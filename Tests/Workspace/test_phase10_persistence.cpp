// Tests/Workspace/test_phase10_persistence.cpp
// Phase 10 — Project Persistence and Serialization
//
// Tests for:
//   1. WorkspaceProjectFile — version, sections, identity, serialize/parse round-trip
//   2. ProjectSerializer    — snapshot serialize/deserialize, round-trip
//   3. AssetCatalogSerializer — catalog serialize/deserialize, round-trip, metadata
//   4. SettingsStore        — layered get/set, typed accessors, precedence, observers, serialization
//   5. Integration          — full persistence cycle (serialize all → text → parse → restore)

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Workspace/WorkspaceProjectFile.h"
#include "NF/Workspace/ProjectSerializer.h"
#include "NF/Workspace/AssetCatalogSerializer.h"
#include "NF/Workspace/SettingsStore.h"
#include "NF/Workspace/AssetCatalog.h"

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — WorkspaceProjectFile
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ProjectFileVersion toString and parse round-trip", "[Phase10][ProjectFile]") {
    ProjectFileVersion v{2, 3, 7};
    std::string s = v.toString();
    CHECK(s == "2.3.7");

    ProjectFileVersion parsed;
    CHECK(ProjectFileVersion::parse(s, parsed));
    CHECK(parsed == v);
}

TEST_CASE("ProjectFileVersion parse rejects bad input", "[Phase10][ProjectFile]") {
    ProjectFileVersion v;
    CHECK_FALSE(ProjectFileVersion::parse("notaversion", v));
    CHECK_FALSE(ProjectFileVersion::parse("1.2",         v)); // missing patch
    CHECK_FALSE(ProjectFileVersion::parse("",            v));
}

TEST_CASE("ProjectFileVersion isCompatible", "[Phase10][ProjectFile]") {
    ProjectFileVersion file {1, 0, 0};
    ProjectFileVersion reader{1, 0, 0};
    CHECK(file.isCompatible(reader));

    reader.minor = 1; // reader is newer minor — ok
    CHECK(file.isCompatible(reader));

    ProjectFileVersion newerMajor{2, 0, 0};
    CHECK_FALSE(file.isCompatible(newerMajor));
}

TEST_CASE("ProjectFileVersion current() returns major 1", "[Phase10][ProjectFile]") {
    auto v = ProjectFileVersion::current();
    CHECK(v.major == 1);
}

TEST_CASE("ProjectFileSection set/get/has/remove", "[Phase10][ProjectFile]") {
    ProjectFileSection sec("TestSection");
    CHECK(sec.name() == "TestSection");
    CHECK(sec.empty());

    sec.set("key1", "value1");
    sec.set("key2", "value2");
    CHECK(sec.count() == 2);
    CHECK(sec.has("key1"));
    REQUIRE(sec.get("key1") != nullptr);
    CHECK(*sec.get("key1") == "value1");
    CHECK(sec.getOr("missing", "default") == "default");

    // Overwrite
    sec.set("key1", "newval");
    CHECK(*sec.get("key1") == "newval");

    CHECK(sec.remove("key2"));
    CHECK(sec.count() == 1);
    CHECK_FALSE(sec.remove("key2"));
}

TEST_CASE("ProjectFileSection clear empties entries", "[Phase10][ProjectFile]") {
    ProjectFileSection sec("S");
    sec.set("a", "1");
    sec.clear();
    CHECK(sec.empty());
}

TEST_CASE("WorkspaceProjectFile identity setters and getters", "[Phase10][ProjectFile]") {
    WorkspaceProjectFile f;
    f.setProjectId("proj-001");
    f.setProjectName("My Project");
    f.setContentRoot("/projects/my");
    f.setVersion({1, 2, 0});

    CHECK(f.projectId()   == "proj-001");
    CHECK(f.projectName() == "My Project");
    CHECK(f.contentRoot() == "/projects/my");
    CHECK(f.version().minor == 2);
}

TEST_CASE("WorkspaceProjectFile isValid checks", "[Phase10][ProjectFile]") {
    WorkspaceProjectFile f;
    CHECK_FALSE(f.isValid()); // no id or name

    f.setProjectId("id"); f.setProjectName("name");
    CHECK(f.isValid());
}

TEST_CASE("WorkspaceProjectFile section create and find", "[Phase10][ProjectFile]") {
    WorkspaceProjectFile f;
    auto& sec = f.section("Config");
    sec.set("theme", "dark");
    CHECK(f.sectionCount() == 1);
    CHECK(f.hasSection("Config"));

    const auto* found = f.findSection("Config");
    REQUIRE(found != nullptr);
    CHECK(*found->get("theme") == "dark");
}

TEST_CASE("WorkspaceProjectFile section re-access returns same section",
          "[Phase10][ProjectFile]") {
    WorkspaceProjectFile f;
    f.section("A").set("k", "v1");
    f.section("A").set("k2", "v2");
    const auto* sec = f.findSection("A");
    REQUIRE(sec != nullptr);
    CHECK(sec->count() == 2);
}

TEST_CASE("WorkspaceProjectFile removeSection works", "[Phase10][ProjectFile]") {
    WorkspaceProjectFile f;
    f.section("X").set("k", "v");
    CHECK(f.removeSection("X"));
    CHECK_FALSE(f.hasSection("X"));
    CHECK_FALSE(f.removeSection("X"));
}

TEST_CASE("WorkspaceProjectFile serialize produces magic header", "[Phase10][ProjectFile]") {
    WorkspaceProjectFile f;
    f.setProjectId("p"); f.setProjectName("N"); f.setContentRoot("/c");
    std::string text = f.serialize();
    CHECK(text.find("#atlasproject:") == 0);
    CHECK(text.find("project.id=p") != std::string::npos);
    CHECK(text.find("project.name=N") != std::string::npos);
}

TEST_CASE("WorkspaceProjectFile serialize/parse round-trip — identity",
          "[Phase10][ProjectFile]") {
    WorkspaceProjectFile src;
    src.setProjectId("proj-42");
    src.setProjectName("Atlas Demo");
    src.setContentRoot("/home/user/atlas");

    std::string text = src.serialize();

    WorkspaceProjectFile dst;
    CHECK(WorkspaceProjectFile::parse(text, dst));
    CHECK(dst.projectId()   == "proj-42");
    CHECK(dst.projectName() == "Atlas Demo");
    CHECK(dst.contentRoot() == "/home/user/atlas");
}

TEST_CASE("WorkspaceProjectFile serialize/parse round-trip — sections",
          "[Phase10][ProjectFile]") {
    WorkspaceProjectFile src;
    src.setProjectId("p"); src.setProjectName("N");
    src.section("Config").set("resolution", "1920x1080");
    src.section("Config").set("fullscreen", "false");

    WorkspaceProjectFile dst;
    CHECK(WorkspaceProjectFile::parse(src.serialize(), dst));
    const auto* sec = dst.findSection("Config");
    REQUIRE(sec != nullptr);
    CHECK(*sec->get("resolution") == "1920x1080");
    CHECK(*sec->get("fullscreen") == "false");
}

TEST_CASE("WorkspaceProjectFile parse rejects empty input", "[Phase10][ProjectFile]") {
    WorkspaceProjectFile f;
    CHECK_FALSE(WorkspaceProjectFile::parse("", f));
}

TEST_CASE("WorkspaceProjectFile parse rejects wrong magic", "[Phase10][ProjectFile]") {
    WorkspaceProjectFile f;
    CHECK_FALSE(WorkspaceProjectFile::parse("notaproject:1.0.0\n", f));
}

TEST_CASE("WorkspaceProjectFile parse version into version struct",
          "[Phase10][ProjectFile]") {
    WorkspaceProjectFile src;
    src.setProjectId("p"); src.setProjectName("N");
    src.setVersion({1, 2, 3});

    WorkspaceProjectFile dst;
    CHECK(WorkspaceProjectFile::parse(src.serialize(), dst));
    CHECK(dst.version().major == 1);
    CHECK(dst.version().minor == 2);
    CHECK(dst.version().patch == 3);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — ProjectSerializer
// ═════════════════════════════════════════════════════════════════

static WorkspaceShellSnapshot makeSnapshot() {
    WorkspaceShellSnapshot s;
    s.projectId       = "snap-001";
    s.projectName     = "Test Project";
    s.contentRoot     = "/test/content";
    s.activeToolId    = "scene.editor";
    s.registeredToolIds = {"scene.editor", "material.editor", "audio.editor"};
    s.visiblePanelIds   = {"panel.properties", "panel.assets"};
    return s;
}

TEST_CASE("WorkspaceShellSnapshot isValid", "[Phase10][ProjectSerializer]") {
    WorkspaceShellSnapshot s;
    CHECK_FALSE(s.isValid());
    s.projectId = "id"; s.projectName = "name";
    CHECK(s.isValid());
}

TEST_CASE("SerializeResult factory methods", "[Phase10][ProjectSerializer]") {
    auto ok   = SerializeResult::ok();
    auto fail = SerializeResult::fail("oops");
    CHECK(ok.succeeded);
    CHECK_FALSE(ok.failed());
    CHECK(fail.failed());
    CHECK(fail.errorMessage == "oops");
}

TEST_CASE("ProjectSerializer serialize to project file", "[Phase10][ProjectSerializer]") {
    auto snap = makeSnapshot();
    WorkspaceProjectFile file;
    auto res = ProjectSerializer::serialize(snap, file);
    CHECK(res.succeeded);
    CHECK(file.isValid());
    CHECK(file.projectId()   == "snap-001");
    CHECK(file.projectName() == "Test Project");
    CHECK(file.hasSection("Core"));
    CHECK(file.hasSection("Tools"));
    CHECK(file.hasSection("Panels"));
}

TEST_CASE("ProjectSerializer serialize stores active tool in Core section",
          "[Phase10][ProjectSerializer]") {
    auto snap = makeSnapshot();
    WorkspaceProjectFile file;
    ProjectSerializer::serialize(snap, file);
    const auto* core = file.findSection("Core");
    REQUIRE(core != nullptr);
    CHECK(*core->get("activeTool") == "scene.editor");
}

TEST_CASE("ProjectSerializer serialize stores tool count", "[Phase10][ProjectSerializer]") {
    auto snap = makeSnapshot();
    WorkspaceProjectFile file;
    ProjectSerializer::serialize(snap, file);
    const auto* tools = file.findSection("Tools");
    REQUIRE(tools != nullptr);
    CHECK(*tools->get("count") == "3");
    CHECK(*tools->get("tool.0") == "scene.editor");
    CHECK(*tools->get("tool.2") == "audio.editor");
}

TEST_CASE("ProjectSerializer serialize stores panel ids", "[Phase10][ProjectSerializer]") {
    auto snap = makeSnapshot();
    WorkspaceProjectFile file;
    ProjectSerializer::serialize(snap, file);
    const auto* panels = file.findSection("Panels");
    REQUIRE(panels != nullptr);
    CHECK(*panels->get("count") == "2");
    CHECK(*panels->get("panel.0") == "panel.properties");
}

TEST_CASE("ProjectSerializer rejects invalid snapshot", "[Phase10][ProjectSerializer]") {
    WorkspaceShellSnapshot bad;
    WorkspaceProjectFile file;
    auto res = ProjectSerializer::serialize(bad, file);
    CHECK(res.failed());
}

TEST_CASE("ProjectSerializer deserialize restores identity", "[Phase10][ProjectSerializer]") {
    auto snap = makeSnapshot();
    WorkspaceProjectFile file;
    ProjectSerializer::serialize(snap, file);

    WorkspaceShellSnapshot out;
    auto res = ProjectSerializer::deserialize(file, out);
    CHECK(res.succeeded);
    CHECK(out.projectId   == "snap-001");
    CHECK(out.projectName == "Test Project");
    CHECK(out.contentRoot == "/test/content");
}

TEST_CASE("ProjectSerializer deserialize restores tools and panels",
          "[Phase10][ProjectSerializer]") {
    auto snap = makeSnapshot();
    WorkspaceProjectFile file;
    ProjectSerializer::serialize(snap, file);

    WorkspaceShellSnapshot out;
    ProjectSerializer::deserialize(file, out);
    CHECK(out.registeredToolIds.size() == 3);
    CHECK(out.registeredToolIds[1]     == "material.editor");
    CHECK(out.visiblePanelIds.size()   == 2);
    CHECK(out.visiblePanelIds[1]       == "panel.assets");
    CHECK(out.activeToolId             == "scene.editor");
}

TEST_CASE("ProjectSerializer deserialize rejects invalid file",
          "[Phase10][ProjectSerializer]") {
    WorkspaceProjectFile bad;
    WorkspaceShellSnapshot out;
    auto res = ProjectSerializer::deserialize(bad, out);
    CHECK(res.failed());
}

TEST_CASE("ProjectSerializer round-trip preserves all fields",
          "[Phase10][ProjectSerializer]") {
    auto snap = makeSnapshot();
    WorkspaceShellSnapshot out;
    auto res = ProjectSerializer::roundTrip(snap, out);
    CHECK(res.succeeded);
    CHECK(out.projectId        == snap.projectId);
    CHECK(out.projectName      == snap.projectName);
    CHECK(out.activeToolId     == snap.activeToolId);
    CHECK(out.registeredToolIds == snap.registeredToolIds);
    CHECK(out.visiblePanelIds  == snap.visiblePanelIds);
}

TEST_CASE("ProjectSerializer round-trip empty tool list", "[Phase10][ProjectSerializer]") {
    WorkspaceShellSnapshot snap;
    snap.projectId = "p"; snap.projectName = "N";
    WorkspaceShellSnapshot out;
    auto res = ProjectSerializer::roundTrip(snap, out);
    CHECK(res.succeeded);
    CHECK(out.registeredToolIds.empty());
    CHECK(out.visiblePanelIds.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — AssetCatalogSerializer
// ═════════════════════════════════════════════════════════════════

static AssetDescriptor makeDesc(const std::string& cat, const std::string& src,
                                 AssetTypeTag tag,
                                 AssetImportState state = AssetImportState::Imported) {
    AssetDescriptor d;
    d.catalogPath  = cat;
    d.sourcePath   = src;
    d.displayName  = "Display_" + cat;
    d.typeTag      = tag;
    d.importState  = state;
    return d;
}

TEST_CASE("CatalogSerializeResult factory methods", "[Phase10][CatalogSerializer]") {
    auto ok   = CatalogSerializeResult::ok(5);
    auto fail = CatalogSerializeResult::fail("bad");
    CHECK(ok.succeeded);
    CHECK(ok.assetCount == 5);
    CHECK(fail.failed());
    CHECK(fail.errorMessage == "bad");
}

TEST_CASE("AssetCatalogSerializer serialize empty catalog", "[Phase10][CatalogSerializer]") {
    AssetCatalog cat;
    WorkspaceProjectFile file;
    file.setProjectId("p"); file.setProjectName("N");
    auto res = AssetCatalogSerializer::serialize(cat, file);
    CHECK(res.succeeded);
    CHECK(res.assetCount == 0);
    CHECK(file.hasSection("AssetCatalog"));
}

TEST_CASE("AssetCatalogSerializer serialize populates section keys",
          "[Phase10][CatalogSerializer]") {
    AssetCatalog cat;
    cat.add(makeDesc("T/rock.png", "/src/rock.png", AssetTypeTag::Texture));

    WorkspaceProjectFile file;
    file.setProjectId("p"); file.setProjectName("N");
    auto res = AssetCatalogSerializer::serialize(cat, file);
    CHECK(res.succeeded);
    CHECK(res.assetCount == 1);

    const auto* sec = file.findSection("AssetCatalog");
    REQUIRE(sec != nullptr);
    CHECK(sec->has("asset.0"));
    CHECK(*sec->get("written") == "1");
}

TEST_CASE("AssetCatalogSerializer deserialize restores descriptor",
          "[Phase10][CatalogSerializer]") {
    AssetCatalog src;
    src.add(makeDesc("T/rock.png", "/src/rock.png", AssetTypeTag::Texture));

    WorkspaceProjectFile file;
    file.setProjectId("p"); file.setProjectName("N");
    AssetCatalogSerializer::serialize(src, file);

    AssetCatalog dst;
    auto res = AssetCatalogSerializer::deserialize(file, dst);
    CHECK(res.succeeded);
    CHECK(res.assetCount == 1);
    CHECK(dst.count() == 1);

    const auto* d = dst.findByPath("T/rock.png");
    REQUIRE(d != nullptr);
    CHECK(d->sourcePath   == "/src/rock.png");
    CHECK(d->typeTag      == AssetTypeTag::Texture);
    CHECK(d->importState  == AssetImportState::Imported);
}

TEST_CASE("AssetCatalogSerializer deserialize fails without section",
          "[Phase10][CatalogSerializer]") {
    WorkspaceProjectFile file;
    file.setProjectId("p"); file.setProjectName("N");
    AssetCatalog cat;
    auto res = AssetCatalogSerializer::deserialize(file, cat);
    CHECK(res.failed());
}

TEST_CASE("AssetCatalogSerializer round-trip multiple assets",
          "[Phase10][CatalogSerializer]") {
    AssetCatalog src;
    src.add(makeDesc("T/rock.png",  "/src/rock.png",  AssetTypeTag::Texture));
    src.add(makeDesc("M/tree.fbx",  "/src/tree.fbx",  AssetTypeTag::Mesh));
    src.add(makeDesc("A/wind.wav",  "/src/wind.wav",  AssetTypeTag::Audio));

    AssetCatalog dst;
    auto res = AssetCatalogSerializer::roundTrip(src, dst);
    CHECK(res.succeeded);
    CHECK(dst.count() == 3);
    CHECK(dst.findByPath("T/rock.png") != nullptr);
    CHECK(dst.findByPath("M/tree.fbx") != nullptr);
    CHECK(dst.findByPath("A/wind.wav") != nullptr);
}

TEST_CASE("AssetCatalogSerializer round-trip preserves import state",
          "[Phase10][CatalogSerializer]") {
    AssetCatalog src;
    src.add(makeDesc("X/a.png", "/a.png", AssetTypeTag::Texture, AssetImportState::Dirty));
    src.add(makeDesc("X/b.png", "/b.png", AssetTypeTag::Texture, AssetImportState::Error));

    AssetCatalog dst;
    auto res = AssetCatalogSerializer::roundTrip(src, dst);
    CHECK(res.succeeded);
    const auto* a = dst.findByPath("X/a.png");
    const auto* b = dst.findByPath("X/b.png");
    REQUIRE(a); REQUIRE(b);
    CHECK(a->importState == AssetImportState::Dirty);
    CHECK(b->importState == AssetImportState::Error);
}

TEST_CASE("AssetCatalogSerializer round-trip preserves metadata",
          "[Phase10][CatalogSerializer]") {
    AssetCatalog src;
    auto d = makeDesc("T/rock.png", "/rock.png", AssetTypeTag::Texture);
    d.metadata.set("width", "2048");
    d.metadata.set("height", "1024");
    AssetId id = src.add(d);
    CHECK(id != INVALID_ASSET_ID);

    AssetCatalog dst;
    auto res = AssetCatalogSerializer::roundTrip(src, dst);
    CHECK(res.succeeded);

    const auto* rd = dst.findByPath("T/rock.png");
    REQUIRE(rd);
    CHECK(rd->metadata.has("width"));
    CHECK(*rd->metadata.get("width") == "2048");
    CHECK(*rd->metadata.get("height") == "1024");
}

TEST_CASE("AssetCatalogSerializer round-trip: path with pipe in display name",
          "[Phase10][CatalogSerializer]") {
    AssetCatalog src;
    auto d = makeDesc("T/a.png", "/a.png", AssetTypeTag::Texture);
    d.displayName = "Rock|Moss"; // contains pipe character
    src.add(d);

    AssetCatalog dst;
    auto res = AssetCatalogSerializer::roundTrip(src, dst);
    CHECK(res.succeeded);
    const auto* rd = dst.findByPath("T/a.png");
    REQUIRE(rd);
    CHECK(rd->displayName == "Rock|Moss");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — SettingsStore
// ═════════════════════════════════════════════════════════════════

TEST_CASE("settingsLayerName returns correct strings", "[Phase10][Settings]") {
    CHECK(std::string(settingsLayerName(SettingsLayer::Default)) == "Default");
    CHECK(std::string(settingsLayerName(SettingsLayer::Project)) == "Project");
    CHECK(std::string(settingsLayerName(SettingsLayer::User))    == "User");
}

TEST_CASE("SettingsStore basic set/get", "[Phase10][Settings]") {
    SettingsStore store;
    store.set("theme", "dark");
    CHECK(store.get("theme") == "dark");
    CHECK(store.has("theme"));
    CHECK(store.get("missing").empty());
}

TEST_CASE("SettingsStore getOr returns fallback on miss", "[Phase10][Settings]") {
    SettingsStore store;
    CHECK(store.getOr("missing", "default_val") == "default_val");
    store.set("k", "v");
    CHECK(store.getOr("k", "other") == "v");
}

TEST_CASE("SettingsStore typed accessors — bool", "[Phase10][Settings]") {
    SettingsStore store;
    store.setBool("fullscreen", true);
    CHECK(store.getBool("fullscreen") == true);
    store.setBool("vsync", false);
    CHECK(store.getBool("vsync") == false);
    CHECK(store.getBool("missing", true) == true);
}

TEST_CASE("SettingsStore typed accessors — int32", "[Phase10][Settings]") {
    SettingsStore store;
    store.setInt32("fps_limit", 120);
    CHECK(store.getInt32("fps_limit") == 120);
    store.setInt32("neg", -5);
    CHECK(store.getInt32("neg") == -5);
    CHECK(store.getInt32("missing", 42) == 42);
}

TEST_CASE("SettingsStore typed accessors — float", "[Phase10][Settings]") {
    SettingsStore store;
    store.setFloat("gamma", 2.2f);
    float v = store.getFloat("gamma");
    CHECK(v == Catch::Approx(2.2f).epsilon(0.001f));
    CHECK(store.getFloat("missing", 1.0f) == Catch::Approx(1.0f));
}

TEST_CASE("SettingsStore layer precedence: User > Project > Default",
          "[Phase10][Settings]") {
    SettingsStore store;
    store.set("key", "default_val", SettingsLayer::Default);
    store.set("key", "project_val", SettingsLayer::Project);
    store.set("key", "user_val",    SettingsLayer::User);
    CHECK(store.get("key") == "user_val");
}

TEST_CASE("SettingsStore layer precedence: Project > Default when no User override",
          "[Phase10][Settings]") {
    SettingsStore store;
    store.set("key", "default_val", SettingsLayer::Default);
    store.set("key", "project_val", SettingsLayer::Project);
    CHECK(store.get("key") == "project_val");
}

TEST_CASE("SettingsStore layer precedence: falls through to Default",
          "[Phase10][Settings]") {
    SettingsStore store;
    store.setDefault("key", "default_val");
    CHECK(store.get("key") == "default_val");
}

TEST_CASE("SettingsStore getFromLayer reads specific layer only",
          "[Phase10][Settings]") {
    SettingsStore store;
    store.set("k", "user_val",    SettingsLayer::User);
    store.set("k", "project_val", SettingsLayer::Project);
    CHECK(*store.getFromLayer("k", SettingsLayer::Project) == "project_val");
    CHECK(*store.getFromLayer("k", SettingsLayer::User)    == "user_val");
}

TEST_CASE("SettingsStore hasInLayer per-layer check", "[Phase10][Settings]") {
    SettingsStore store;
    store.set("k", "v", SettingsLayer::Project);
    CHECK(store.hasInLayer("k", SettingsLayer::Project));
    CHECK_FALSE(store.hasInLayer("k", SettingsLayer::User));
}

TEST_CASE("SettingsStore remove from layer", "[Phase10][Settings]") {
    SettingsStore store;
    store.set("k", "v");
    CHECK(store.remove("k", SettingsLayer::User));
    CHECK_FALSE(store.has("k"));
}

TEST_CASE("SettingsStore clearLayer empties one layer only", "[Phase10][Settings]") {
    SettingsStore store;
    store.set("k", "user_v",    SettingsLayer::User);
    store.set("k", "default_v", SettingsLayer::Default);
    store.clearLayer(SettingsLayer::User);
    CHECK(store.get("k") == "default_v"); // falls through to Default
    CHECK(store.countInLayer(SettingsLayer::User) == 0);
}

TEST_CASE("SettingsStore countInLayer and totalCount", "[Phase10][Settings]") {
    SettingsStore store;
    store.set("a", "1", SettingsLayer::User);
    store.set("b", "2", SettingsLayer::Project);
    store.set("c", "3", SettingsLayer::Default);
    CHECK(store.countInLayer(SettingsLayer::User)    == 1);
    CHECK(store.countInLayer(SettingsLayer::Project) == 1);
    CHECK(store.countInLayer(SettingsLayer::Default) == 1);
    CHECK(store.totalCount() == 3);
}

TEST_CASE("SettingsStore observer receives change notifications", "[Phase10][Settings]") {
    SettingsStore store;
    std::vector<std::string> log;
    store.addObserver([&](const std::string& key, const std::string& val, SettingsLayer layer) {
        log.push_back(key + "=" + val + "@" + settingsLayerName(layer));
    });

    store.set("theme", "dark");
    store.set("fps", "60", SettingsLayer::Project);
    CHECK(log.size() == 2);
    CHECK(log[0] == "theme=dark@User");
    CHECK(log[1] == "fps=60@Project");
}

TEST_CASE("SettingsStore clearObservers stops notifications", "[Phase10][Settings]") {
    SettingsStore store;
    int count = 0;
    store.addObserver([&](const std::string&, const std::string&, SettingsLayer) { ++count; });
    store.set("k", "v");
    store.clearObservers();
    store.set("k", "v2");
    CHECK(count == 1);
}

TEST_CASE("SettingsStore serializeLayer writes to project file section",
          "[Phase10][Settings]") {
    SettingsStore store;
    store.set("theme", "dark",  SettingsLayer::User);
    store.set("fps",   "120",   SettingsLayer::User);

    WorkspaceProjectFile file;
    file.setProjectId("p"); file.setProjectName("N");
    store.serializeLayer(SettingsLayer::User, file);

    const auto* sec = file.findSection("Settings.User");
    REQUIRE(sec != nullptr);
    CHECK(*sec->get("theme") == "dark");
    CHECK(*sec->get("fps")   == "120");
}

TEST_CASE("SettingsStore deserializeLayer restores from file", "[Phase10][Settings]") {
    WorkspaceProjectFile file;
    file.setProjectId("p"); file.setProjectName("N");
    file.section("Settings.User").set("theme", "light");
    file.section("Settings.User").set("fps", "60");

    SettingsStore store;
    CHECK(store.deserializeLayer(SettingsLayer::User, file));
    CHECK(store.get("theme") == "light");
    CHECK(store.getInt32("fps") == 60);
}

TEST_CASE("SettingsStore serialize/deserialize round-trip all layers",
          "[Phase10][Settings]") {
    SettingsStore src;
    src.set("k1", "user_v",    SettingsLayer::User);
    src.set("k2", "project_v", SettingsLayer::Project);
    src.set("k3", "default_v", SettingsLayer::Default);

    WorkspaceProjectFile file;
    file.setProjectId("p"); file.setProjectName("N");
    src.serializeAll(file);

    SettingsStore dst;
    dst.deserializeAll(file);
    CHECK(dst.getFromLayer("k1", SettingsLayer::User)    != nullptr);
    CHECK(*dst.getFromLayer("k1", SettingsLayer::User)    == "user_v");
    CHECK(*dst.getFromLayer("k2", SettingsLayer::Project) == "project_v");
    CHECK(*dst.getFromLayer("k3", SettingsLayer::Default) == "default_v");
}

TEST_CASE("SettingsStore deserializeLayer returns false for missing section",
          "[Phase10][Settings]") {
    WorkspaceProjectFile file;
    file.setProjectId("p"); file.setProjectName("N");
    SettingsStore store;
    CHECK_FALSE(store.deserializeLayer(SettingsLayer::User, file));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — Integration: full persistence cycle
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: full serialize cycle — snapshot + catalog + settings",
          "[Phase10][Integration]") {
    // ── Setup ────────────────────────────────────────────────────
    WorkspaceShellSnapshot snap;
    snap.projectId    = "full-001";
    snap.projectName  = "Integration Project";
    snap.contentRoot  = "/projects/int";
    snap.activeToolId = "scene.editor";
    snap.registeredToolIds = {"scene.editor", "audio.editor"};
    snap.visiblePanelIds   = {"panel.properties"};

    AssetCatalog srcCatalog;
    srcCatalog.add(makeDesc("T/sky.png", "/assets/sky.png", AssetTypeTag::Texture));
    srcCatalog.add(makeDesc("M/car.fbx", "/assets/car.fbx", AssetTypeTag::Mesh,
                             AssetImportState::Dirty));

    SettingsStore srcSettings;
    srcSettings.set("theme", "dark", SettingsLayer::User);
    srcSettings.setInt32("fps_limit", 120, SettingsLayer::Project);

    // ── Serialize all into one project file ───────────────────────
    WorkspaceProjectFile file;
    ProjectSerializer::serialize(snap, file);
    AssetCatalogSerializer::serialize(srcCatalog, file);
    srcSettings.serializeAll(file);

    // ── Write to text and parse back ──────────────────────────────
    std::string text = file.serialize();
    WorkspaceProjectFile restored;
    REQUIRE(WorkspaceProjectFile::parse(text, restored));

    // ── Restore snapshot ──────────────────────────────────────────
    WorkspaceShellSnapshot outSnap;
    auto sRes = ProjectSerializer::deserialize(restored, outSnap);
    REQUIRE(sRes.succeeded);
    CHECK(outSnap.projectName      == "Integration Project");
    CHECK(outSnap.activeToolId     == "scene.editor");
    CHECK(outSnap.registeredToolIds.size() == 2);

    // ── Restore catalog ───────────────────────────────────────────
    AssetCatalog outCatalog;
    auto cRes = AssetCatalogSerializer::deserialize(restored, outCatalog);
    REQUIRE(cRes.succeeded);
    CHECK(outCatalog.count() == 2);
    const auto* sky = outCatalog.findByPath("T/sky.png");
    REQUIRE(sky);
    CHECK(sky->typeTag     == AssetTypeTag::Texture);
    CHECK(sky->importState == AssetImportState::Imported);
    const auto* car = outCatalog.findByPath("M/car.fbx");
    REQUIRE(car);
    CHECK(car->importState == AssetImportState::Dirty);

    // ── Restore settings ──────────────────────────────────────────
    SettingsStore outSettings;
    outSettings.deserializeAll(restored);
    CHECK(outSettings.get("theme")         == "dark");
    CHECK(outSettings.getInt32("fps_limit") == 120);
}

TEST_CASE("Integration: project file version survives round-trip",
          "[Phase10][Integration]") {
    WorkspaceProjectFile src;
    src.setProjectId("v-test"); src.setProjectName("VersionTest");
    // Use the current schema version so isCompatible passes
    src.setVersion(ProjectFileVersion::current());

    WorkspaceProjectFile dst;
    REQUIRE(WorkspaceProjectFile::parse(src.serialize(), dst));
    CHECK(dst.version() == src.version());
    CHECK(dst.version().isCompatible(ProjectFileVersion::current()));
}

TEST_CASE("Integration: file with newer minor version is NOT compatible with older reader",
          "[Phase10][Integration]") {
    // A file written with minor=2 cannot be read by a reader at minor=0.
    // isCompatible() is called on the file version with the reader version as argument.
    ProjectFileVersion fileVer{1, 2, 5};
    ProjectFileVersion readerVer{1, 0, 0};
    CHECK_FALSE(fileVer.isCompatible(readerVer));
}

TEST_CASE("Integration: settings override precedence preserved through serialization",
          "[Phase10][Integration]") {
    SettingsStore src;
    src.set("key", "default_v", SettingsLayer::Default);
    src.set("key", "project_v", SettingsLayer::Project);
    // No User override — project should win after restore

    WorkspaceProjectFile file;
    file.setProjectId("p"); file.setProjectName("N");
    src.serializeAll(file);

    SettingsStore dst;
    dst.deserializeAll(file);
    CHECK(dst.get("key") == "project_v");
}
