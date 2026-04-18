#include <catch2/catch_test_macros.hpp>

// ── Phase 67 — WorkspaceAssetBrowser tests ───────────────────────────────────
// Tests the query-driven asset browser model:
//   AssetBrowserSortMode enum + name helpers
//   AssetBrowserFilter   — type/state masks, namePattern, pathPrefix, maxResults
//   AssetBrowserEntry    — validity
//   AssetBrowserState    — selection helpers
//   AssetBrowser         — catalog binding, filter, refresh, select, observers, reset

#include "NF/Workspace/WorkspaceAssetBrowser.h"
#include "NF/Workspace/AssetCatalog.h"

using namespace NF;

// ── Helpers ───────────────────────────────────────────────────────────────────

static AssetId addAsset(AssetCatalog& cat,
                        const std::string& catalogPath,
                        AssetTypeTag tag,
                        const std::string& displayName = "",
                        AssetImportState state = AssetImportState::Imported) {
    AssetDescriptor d;
    d.sourcePath  = "/src/" + catalogPath;
    d.catalogPath = catalogPath;
    d.displayName = displayName.empty() ? catalogPath : displayName;
    d.typeTag     = tag;
    d.importState = state;
    AssetId id = cat.add(std::move(d));
    return id;
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowserSortMode
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowserSortMode names are correct", "[Phase67][SortMode]") {
    CHECK(std::string(assetBrowserSortModeName(AssetBrowserSortMode::ByName))       == "ByName");
    CHECK(std::string(assetBrowserSortModeName(AssetBrowserSortMode::ByType))       == "ByType");
    CHECK(std::string(assetBrowserSortModeName(AssetBrowserSortMode::ByPath))       == "ByPath");
    CHECK(std::string(assetBrowserSortModeName(AssetBrowserSortMode::ByImportTime)) == "ByImportTime");
    CHECK(std::string(assetBrowserSortModeName(AssetBrowserSortMode::BySize))       == "BySize");
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowserFilter
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowserFilter default accepts all types", "[Phase67][Filter]") {
    AssetBrowserFilter f;
    CHECK(f.acceptsType(AssetTypeTag::Texture));
    CHECK(f.acceptsType(AssetTypeTag::Mesh));
    CHECK(f.acceptsType(AssetTypeTag::Unknown));
}

TEST_CASE("AssetBrowserFilter typeMask filters by type", "[Phase67][Filter]") {
    AssetBrowserFilter f;
    f.typeMask = 1u << static_cast<uint8_t>(AssetTypeTag::Texture);
    CHECK( f.acceptsType(AssetTypeTag::Texture));
    CHECK(!f.acceptsType(AssetTypeTag::Mesh));
    CHECK(!f.acceptsType(AssetTypeTag::Audio));
}

TEST_CASE("AssetBrowserFilter importStateMask filters by state", "[Phase67][Filter]") {
    AssetBrowserFilter f;
    f.importStateMask = 1u << static_cast<uint8_t>(AssetImportState::Imported);
    CHECK( f.acceptsImportState(AssetImportState::Imported));
    CHECK(!f.acceptsImportState(AssetImportState::Dirty));
    CHECK(!f.acceptsImportState(AssetImportState::Error));
}

TEST_CASE("AssetBrowserFilter default import state mask accepts all", "[Phase67][Filter]") {
    AssetBrowserFilter f;
    CHECK(f.acceptsImportState(AssetImportState::Imported));
    CHECK(f.acceptsImportState(AssetImportState::Dirty));
    CHECK(f.acceptsImportState(AssetImportState::Error));
}

TEST_CASE("AssetBrowserFilter isEmpty detects empty filter", "[Phase67][Filter]") {
    AssetBrowserFilter f;
    CHECK(f.isEmpty());
    f.namePattern = "rock";
    CHECK_FALSE(f.isEmpty());
    f.clear();
    CHECK(f.isEmpty());
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowserEntry
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowserEntry default is invalid", "[Phase67][Entry]") {
    AssetBrowserEntry e;
    CHECK_FALSE(e.isValid());
}

TEST_CASE("AssetBrowserEntry with id and catalogPath is valid", "[Phase67][Entry]") {
    AssetBrowserEntry e;
    e.id          = 1u;
    e.catalogPath = "Textures/rock.png";
    e.typeTag     = AssetTypeTag::Texture;
    CHECK(e.isValid());
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowserState
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowserState default has no selection", "[Phase67][State]") {
    AssetBrowserState s;
    CHECK_FALSE(s.hasSelection());
    CHECK(s.selectedEntry() == nullptr);
}

TEST_CASE("AssetBrowserState selectedEntry returns nullptr when id not in results", "[Phase67][State]") {
    AssetBrowserState s;
    s.selectedId = 42u;
    CHECK(s.selectedEntry() == nullptr);
}

TEST_CASE("AssetBrowserState clearSelection resets selectedId", "[Phase67][State]") {
    AssetBrowserState s;
    s.selectedId = 5u;
    s.clearSelection();
    CHECK_FALSE(s.hasSelection());
}

TEST_CASE("AssetBrowserState clear resets everything", "[Phase67][State]") {
    AssetBrowserState s;
    s.selectedId = 3u;
    s.dirty      = true;
    s.clear();
    CHECK_FALSE(s.hasSelection());
    CHECK_FALSE(s.dirty);
    CHECK(s.results.empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — construction / binding
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser default state: no catalog, no results", "[Phase67][Browser]") {
    AssetBrowser b;
    CHECK(b.catalog() == nullptr);
    CHECK(b.resultCount() == 0);
    CHECK_FALSE(b.hasResults());
    CHECK_FALSE(b.isDirty());
}

TEST_CASE("AssetBrowser setCatalog stores catalog pointer", "[Phase67][Browser]") {
    AssetCatalog cat;
    AssetBrowser b;
    b.setCatalog(&cat);
    CHECK(b.catalog() == &cat);
    CHECK(b.isDirty()); // marked dirty after setCatalog
}

TEST_CASE("AssetBrowser setCatalog null clears pointer", "[Phase67][Browser]") {
    AssetCatalog cat;
    AssetBrowser b;
    b.setCatalog(&cat);
    b.setCatalog(nullptr);
    CHECK(b.catalog() == nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — refresh
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser refresh with no catalog returns 0", "[Phase67][Browser]") {
    AssetBrowser b;
    CHECK(b.refresh() == 0u);
    CHECK(b.resultCount() == 0);
}

TEST_CASE("AssetBrowser refresh with empty catalog returns 0", "[Phase67][Browser]") {
    AssetCatalog cat;
    AssetBrowser b;
    b.setCatalog(&cat);
    CHECK(b.refresh() == 0u);
}

TEST_CASE("AssetBrowser refresh returns all assets with empty filter", "[Phase67][Browser]") {
    AssetCatalog cat;
    addAsset(cat, "Textures/rock.png",  AssetTypeTag::Texture);
    addAsset(cat, "Meshes/cube.obj",    AssetTypeTag::Mesh);
    addAsset(cat, "Audio/click.wav",    AssetTypeTag::Audio);

    AssetBrowser b;
    b.setCatalog(&cat);
    uint32_t count = b.refresh();
    CHECK(count == 3u);
    CHECK(b.resultCount() == 3u);
    CHECK(b.hasResults());
}

TEST_CASE("AssetBrowser refresh clears dirty flag", "[Phase67][Browser]") {
    AssetCatalog cat;
    AssetBrowser b;
    b.setCatalog(&cat);
    CHECK(b.isDirty());
    b.refresh();
    CHECK_FALSE(b.isDirty());
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — filter: type mask
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser typeMask filter returns only matching types", "[Phase67][Browser][Filter]") {
    AssetCatalog cat;
    addAsset(cat, "Textures/rock.png", AssetTypeTag::Texture);
    addAsset(cat, "Meshes/cube.obj",   AssetTypeTag::Mesh);
    addAsset(cat, "Textures/dirt.png", AssetTypeTag::Texture);

    AssetBrowserFilter f;
    f.typeMask = 1u << static_cast<uint8_t>(AssetTypeTag::Texture);

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    CHECK(b.refresh() == 2u);
    for (const auto& e : b.state().results)
        CHECK(e.typeTag == AssetTypeTag::Texture);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — filter: import state mask
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser importStateMask filter includes only matching states", "[Phase67][Browser][Filter]") {
    AssetCatalog cat;
    addAsset(cat, "a.png", AssetTypeTag::Texture, "", AssetImportState::Imported);
    addAsset(cat, "b.png", AssetTypeTag::Texture, "", AssetImportState::Dirty);
    addAsset(cat, "c.png", AssetTypeTag::Texture, "", AssetImportState::Error);

    AssetBrowserFilter f;
    f.importStateMask = 1u << static_cast<uint8_t>(AssetImportState::Dirty)
                      | 1u << static_cast<uint8_t>(AssetImportState::Error);

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    CHECK(b.refresh() == 2u);
    for (const auto& e : b.state().results)
        CHECK((e.importState == AssetImportState::Dirty || e.importState == AssetImportState::Error));
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — filter: name pattern
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser namePattern filter is case-insensitive on displayName", "[Phase67][Browser][Filter]") {
    AssetCatalog cat;
    addAsset(cat, "Textures/rock_diffuse.png", AssetTypeTag::Texture, "Rock Diffuse");
    addAsset(cat, "Textures/grass.png",        AssetTypeTag::Texture, "Grass");
    addAsset(cat, "Textures/ROCK_normal.png",  AssetTypeTag::Texture, "Rock Normal");

    AssetBrowserFilter f;
    f.namePattern = "ROCK";

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    CHECK(b.refresh() == 2u);
}

TEST_CASE("AssetBrowser namePattern matches catalogPath too", "[Phase67][Browser][Filter]") {
    AssetCatalog cat;
    addAsset(cat, "Audio/footstep_gravel.wav", AssetTypeTag::Audio, "Footstep Gravel");
    addAsset(cat, "Audio/ambient.wav",         AssetTypeTag::Audio, "Ambient");

    AssetBrowserFilter f;
    f.namePattern = "gravel";

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    CHECK(b.refresh() == 1u);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — filter: path prefix
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser pathPrefix filters to subtree", "[Phase67][Browser][Filter]") {
    AssetCatalog cat;
    addAsset(cat, "Textures/rock.png",   AssetTypeTag::Texture);
    addAsset(cat, "Meshes/cube.obj",     AssetTypeTag::Mesh);
    addAsset(cat, "Textures/grass.png",  AssetTypeTag::Texture);

    AssetBrowserFilter f;
    f.pathPrefix = "Textures/";

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    CHECK(b.refresh() == 2u);
    for (const auto& e : b.state().results)
        CHECK(e.catalogPath.compare(0, 9, "Textures/") == 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — filter: maxResults
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser maxResults caps the result count", "[Phase67][Browser][Filter]") {
    AssetCatalog cat;
    for (int i = 0; i < 10; ++i) {
        addAsset(cat, "Textures/t" + std::to_string(i) + ".png", AssetTypeTag::Texture);
    }

    AssetBrowserFilter f;
    f.maxResults = 3u;

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    CHECK(b.refresh() == 3u);
    CHECK(b.resultCount() == 3u);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — clearFilter
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser clearFilter marks dirty and resets filter", "[Phase67][Browser]") {
    AssetCatalog cat;
    addAsset(cat, "Textures/rock.png", AssetTypeTag::Texture);
    addAsset(cat, "Meshes/cube.obj",   AssetTypeTag::Mesh);

    AssetBrowserFilter f;
    f.typeMask = 1u << static_cast<uint8_t>(AssetTypeTag::Texture);

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    b.refresh();
    CHECK(b.resultCount() == 1u);

    b.clearFilter();
    CHECK(b.isDirty());
    b.refresh();
    CHECK(b.resultCount() == 2u);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — sort modes
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser ByName sort returns ascending display names", "[Phase67][Browser][Sort]") {
    AssetCatalog cat;
    addAsset(cat, "b.png", AssetTypeTag::Texture, "Zebra");
    addAsset(cat, "a.png", AssetTypeTag::Texture, "Apple");
    addAsset(cat, "c.png", AssetTypeTag::Texture, "Mango");

    AssetBrowserFilter f;
    f.sortMode = AssetBrowserSortMode::ByName;

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    b.refresh();

    REQUIRE(b.resultCount() == 3u);
    CHECK(b.state().results[0].displayName == "Apple");
    CHECK(b.state().results[1].displayName == "Mango");
    CHECK(b.state().results[2].displayName == "Zebra");
}

TEST_CASE("AssetBrowser ByType sort groups by type tag value", "[Phase67][Browser][Sort]") {
    AssetCatalog cat;
    addAsset(cat, "sound.wav",   AssetTypeTag::Audio);   // type 4
    addAsset(cat, "rock.png",    AssetTypeTag::Texture);  // type 2
    addAsset(cat, "cube.obj",    AssetTypeTag::Mesh);     // type 3

    AssetBrowserFilter f;
    f.sortMode = AssetBrowserSortMode::ByType;

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    b.refresh();

    REQUIRE(b.resultCount() == 3u);
    // Texture(2) < Mesh(3) < Audio(4)
    CHECK(b.state().results[0].typeTag == AssetTypeTag::Texture);
    CHECK(b.state().results[1].typeTag == AssetTypeTag::Mesh);
    CHECK(b.state().results[2].typeTag == AssetTypeTag::Audio);
}

TEST_CASE("AssetBrowser ByPath sort returns ascending catalog paths", "[Phase67][Browser][Sort]") {
    AssetCatalog cat;
    addAsset(cat, "Z/z.png", AssetTypeTag::Texture);
    addAsset(cat, "A/a.png", AssetTypeTag::Texture);
    addAsset(cat, "M/m.png", AssetTypeTag::Texture);

    AssetBrowserFilter f;
    f.sortMode = AssetBrowserSortMode::ByPath;

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    b.refresh();

    REQUIRE(b.resultCount() == 3u);
    CHECK(b.state().results[0].catalogPath == "A/a.png");
    CHECK(b.state().results[1].catalogPath == "M/m.png");
    CHECK(b.state().results[2].catalogPath == "Z/z.png");
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — selection
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser select returns true for valid result id", "[Phase67][Browser][Selection]") {
    AssetCatalog cat;
    AssetId id = addAsset(cat, "Textures/rock.png", AssetTypeTag::Texture);

    AssetBrowser b;
    b.setCatalog(&cat);
    b.refresh();

    CHECK(b.select(id));
    CHECK(b.selectedId() == id);
}

TEST_CASE("AssetBrowser select returns false for id not in results", "[Phase67][Browser][Selection]") {
    AssetBrowser b;
    CHECK_FALSE(b.select(999u));
    CHECK(b.selectedId() == INVALID_ASSET_ID);
}

TEST_CASE("AssetBrowser clearSelection resets to no selection", "[Phase67][Browser][Selection]") {
    AssetCatalog cat;
    AssetId id = addAsset(cat, "a.png", AssetTypeTag::Texture);

    AssetBrowser b;
    b.setCatalog(&cat);
    b.refresh();
    b.select(id);
    REQUIRE(b.selectedId() == id);

    b.clearSelection();
    CHECK(b.selectedId() == INVALID_ASSET_ID);
    CHECK(b.selectedEntry() == nullptr);
}

TEST_CASE("AssetBrowser selectedEntry returns correct entry", "[Phase67][Browser][Selection]") {
    AssetCatalog cat;
    AssetId id = addAsset(cat, "rock.png", AssetTypeTag::Texture, "Rock");

    AssetBrowser b;
    b.setCatalog(&cat);
    b.refresh();
    b.select(id);

    const AssetBrowserEntry* e = b.selectedEntry();
    REQUIRE(e != nullptr);
    CHECK(e->id == id);
    CHECK(e->displayName == "Rock");
}

TEST_CASE("AssetBrowser refresh deselects entry no longer in results", "[Phase67][Browser][Selection]") {
    AssetCatalog cat;
    AssetId id1 = addAsset(cat, "rock.png", AssetTypeTag::Texture);
    addAsset(cat, "cube.obj", AssetTypeTag::Mesh);

    AssetBrowser b;
    b.setCatalog(&cat);
    b.refresh();
    b.select(id1);
    REQUIRE(b.selectedId() == id1);

    // Apply filter that excludes id1
    AssetBrowserFilter f;
    f.typeMask = 1u << static_cast<uint8_t>(AssetTypeTag::Mesh);
    b.setFilter(f);
    b.refresh();

    CHECK(b.selectedId() == INVALID_ASSET_ID);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — observers
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser refresh observer fires with result count", "[Phase67][Browser][Observer]") {
    AssetCatalog cat;
    addAsset(cat, "a.png", AssetTypeTag::Texture);
    addAsset(cat, "b.png", AssetTypeTag::Texture);

    AssetBrowser b;
    b.setCatalog(&cat);

    uint32_t fired = 0;
    uint32_t lastCount = 0;
    b.addRefreshObserver([&](uint32_t count) {
        ++fired;
        lastCount = count;
    });

    b.refresh();
    CHECK(fired == 1u);
    CHECK(lastCount == 2u);
}

TEST_CASE("AssetBrowser selection observer fires on select", "[Phase67][Browser][Observer]") {
    AssetCatalog cat;
    AssetId id = addAsset(cat, "a.png", AssetTypeTag::Texture);

    AssetBrowser b;
    b.setCatalog(&cat);
    b.refresh();

    AssetId notified = INVALID_ASSET_ID;
    b.addSelectionObserver([&](AssetId sid) { notified = sid; });

    b.select(id);
    CHECK(notified == id);
}

TEST_CASE("AssetBrowser clearObservers removes all observers", "[Phase67][Browser][Observer]") {
    AssetCatalog cat;
    addAsset(cat, "a.png", AssetTypeTag::Texture);

    AssetBrowser b;
    b.setCatalog(&cat);

    uint32_t fired = 0;
    b.addRefreshObserver([&](uint32_t) { ++fired; });
    b.clearObservers();
    b.refresh();
    CHECK(fired == 0u);
}

TEST_CASE("AssetBrowser addRefreshObserver rejects over MAX_OBSERVERS", "[Phase67][Browser][Observer]") {
    AssetBrowser b;
    uint32_t added = 0;
    for (uint32_t i = 0; i < 10; ++i) {
        if (b.addRefreshObserver([](uint32_t) {})) ++added;
    }
    CHECK(added == AssetBrowser::MAX_OBSERVERS);
}

// ─────────────────────────────────────────────────────────────────────────────
// AssetBrowser — reset
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser reset clears catalog, state, and observers", "[Phase67][Browser]") {
    AssetCatalog cat;
    addAsset(cat, "a.png", AssetTypeTag::Texture);

    AssetBrowser b;
    b.setCatalog(&cat);
    b.refresh();
    b.addRefreshObserver([](uint32_t) {});
    REQUIRE(b.resultCount() == 1u);

    b.reset();
    CHECK(b.catalog() == nullptr);
    CHECK(b.resultCount() == 0u);
    // Observer was cleared: refresh after reset should not crash
    b.refresh();
}

// ─────────────────────────────────────────────────────────────────────────────
// Integration
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AssetBrowser integration: multi-filter pipeline", "[Phase67][Integration]") {
    AssetCatalog cat;
    addAsset(cat, "Characters/hero_diffuse.png", AssetTypeTag::Texture, "Hero Diffuse", AssetImportState::Imported);
    addAsset(cat, "Characters/hero_normal.png",  AssetTypeTag::Texture, "Hero Normal",  AssetImportState::Dirty);
    addAsset(cat, "Characters/hero.obj",         AssetTypeTag::Mesh,    "Hero Mesh",    AssetImportState::Imported);
    addAsset(cat, "UI/button.png",               AssetTypeTag::Texture, "Button",       AssetImportState::Imported);
    addAsset(cat, "Audio/bgm.wav",               AssetTypeTag::Audio,   "BGM",          AssetImportState::Imported);

    AssetBrowserFilter f;
    // Only Textures
    f.typeMask        = 1u << static_cast<uint8_t>(AssetTypeTag::Texture);
    // Only Imported
    f.importStateMask = 1u << static_cast<uint8_t>(AssetImportState::Imported);
    // Only in Characters/ subtree
    f.pathPrefix      = "Characters/";
    f.sortMode        = AssetBrowserSortMode::ByName;

    AssetBrowser b;
    b.setCatalog(&cat);
    b.setFilter(f);
    uint32_t count = b.refresh();

    // Should match: hero_diffuse.png (Texture + Imported + Characters/)
    // hero_normal.png is Dirty → excluded
    // hero.obj is Mesh → excluded
    CHECK(count == 1u);
    REQUIRE(b.hasResults());
    CHECK(b.state().results[0].displayName == "Hero Diffuse");
}

TEST_CASE("AssetBrowser integration: full selection round-trip", "[Phase67][Integration]") {
    AssetCatalog cat;
    AssetId id1 = addAsset(cat, "a.png", AssetTypeTag::Texture, "Alpha");
    AssetId id2 = addAsset(cat, "b.png", AssetTypeTag::Texture, "Beta");

    AssetBrowser b;
    b.setCatalog(&cat);

    // Track both observers
    uint32_t refreshFired = 0;
    AssetId  lastSelected = INVALID_ASSET_ID;
    b.addRefreshObserver([&](uint32_t) { ++refreshFired; });
    b.addSelectionObserver([&](AssetId id) { lastSelected = id; });

    b.refresh();
    CHECK(refreshFired == 1u);

    b.select(id1);
    CHECK(lastSelected == id1);

    b.select(id2);
    CHECK(lastSelected == id2);
    CHECK(b.selectedId() == id2);

    const AssetBrowserEntry* e = b.selectedEntry();
    REQUIRE(e != nullptr);
    CHECK(e->displayName == "Beta");

    b.clearSelection();
    CHECK(b.selectedId() == INVALID_ASSET_ID);
}
