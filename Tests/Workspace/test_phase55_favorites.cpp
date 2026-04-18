// Tests/Workspace/test_phase55_favorites.cpp
// Phase 55 — WorkspaceFavorites: FavoriteItemKind, FavoriteItem,
//             FavoriteFolder, FavoritesManager
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceFavorites.h"

// ═════════════════════════════════════════════════════════════════
// FavoriteItemKind
// ═════════════════════════════════════════════════════════════════

TEST_CASE("FavoriteItemKind: name helpers", "[favorites][kind]") {
    REQUIRE(std::string(NF::favoriteItemKindName(NF::FavoriteItemKind::Asset))  == "Asset");
    REQUIRE(std::string(NF::favoriteItemKindName(NF::FavoriteItemKind::Tool))   == "Tool");
    REQUIRE(std::string(NF::favoriteItemKindName(NF::FavoriteItemKind::Scene))  == "Scene");
    REQUIRE(std::string(NF::favoriteItemKindName(NF::FavoriteItemKind::File))   == "File");
    REQUIRE(std::string(NF::favoriteItemKindName(NF::FavoriteItemKind::Panel))  == "Panel");
    REQUIRE(std::string(NF::favoriteItemKindName(NF::FavoriteItemKind::Custom)) == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// FavoriteItem
// ═════════════════════════════════════════════════════════════════

static NF::FavoriteItem makeFavItem(const std::string& id,
                                    const std::string& label,
                                    NF::FavoriteItemKind kind = NF::FavoriteItemKind::Asset,
                                    uint64_t addedMs = 100) {
    NF::FavoriteItem item;
    item.id      = id;
    item.label   = label;
    item.kind    = kind;
    item.addedMs = addedMs;
    return item;
}

TEST_CASE("FavoriteItem: default invalid", "[favorites][item]") {
    NF::FavoriteItem item;
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("FavoriteItem: valid with id and label", "[favorites][item]") {
    auto item = makeFavItem("tex.01", "Cobblestone");
    REQUIRE(item.isValid());
}

TEST_CASE("FavoriteItem: invalid without label", "[favorites][item]") {
    NF::FavoriteItem item;
    item.id = "x";
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("FavoriteItem: equality by id", "[favorites][item]") {
    auto a = makeFavItem("a", "A");
    auto b = makeFavItem("a", "B");
    auto c = makeFavItem("c", "C");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ═════════════════════════════════════════════════════════════════
// FavoriteFolder
// ═════════════════════════════════════════════════════════════════

static NF::FavoriteFolder makeFolder(const std::string& id, const std::string& name) {
    NF::FavoriteFolder f;
    f.id   = id;
    f.name = name;
    return f;
}

TEST_CASE("FavoriteFolder: default empty", "[favorites][folder]") {
    NF::FavoriteFolder f;
    REQUIRE(f.count() == 0);
    REQUIRE(f.empty());
}

TEST_CASE("FavoriteFolder: valid with id and name", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    REQUIRE(f.isValid());
}

TEST_CASE("FavoriteFolder: invalid without id", "[favorites][folder]") {
    NF::FavoriteFolder f;
    f.name = "Test";
    REQUIRE_FALSE(f.isValid());
}

TEST_CASE("FavoriteFolder: addItem", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    REQUIRE(f.addItem(makeFavItem("a", "Alpha")));
    REQUIRE(f.count() == 1);
    REQUIRE(f.containsItem("a"));
}

TEST_CASE("FavoriteFolder: addItem rejects invalid", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    NF::FavoriteItem bad;
    REQUIRE_FALSE(f.addItem(bad));
}

TEST_CASE("FavoriteFolder: addItem rejects duplicate", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    REQUIRE(f.addItem(makeFavItem("a", "Alpha")));
    REQUIRE_FALSE(f.addItem(makeFavItem("a", "Alpha2")));
}

TEST_CASE("FavoriteFolder: removeItem", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    f.addItem(makeFavItem("a", "Alpha"));
    REQUIRE(f.removeItem("a"));
    REQUIRE(f.count() == 0);
}

TEST_CASE("FavoriteFolder: removeItem unknown", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    REQUIRE_FALSE(f.removeItem("nope"));
}

TEST_CASE("FavoriteFolder: findItem", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    f.addItem(makeFavItem("a", "Alpha"));
    REQUIRE(f.findItem("a") != nullptr);
    REQUIRE(f.findItem("a")->label == "Alpha");
    REQUIRE(f.findItem("nope") == nullptr);
}

TEST_CASE("FavoriteFolder: moveItem", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    f.addItem(makeFavItem("a", "Alpha"));
    f.addItem(makeFavItem("b", "Beta"));
    f.addItem(makeFavItem("c", "Charlie"));
    REQUIRE(f.moveItem("c", 0));
    REQUIRE(f.items()[0].id == "c");
    REQUIRE(f.items()[1].id == "a");
    REQUIRE(f.items()[2].id == "b");
}

TEST_CASE("FavoriteFolder: moveItem unknown returns false", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    f.addItem(makeFavItem("a", "Alpha"));
    REQUIRE_FALSE(f.moveItem("nope", 0));
}

TEST_CASE("FavoriteFolder: moveItem invalid index", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    f.addItem(makeFavItem("a", "Alpha"));
    REQUIRE_FALSE(f.moveItem("a", 5));
}

TEST_CASE("FavoriteFolder: clear", "[favorites][folder]") {
    auto f = makeFolder("gen", "General");
    f.addItem(makeFavItem("a", "Alpha"));
    f.addItem(makeFavItem("b", "Beta"));
    f.clear();
    REQUIRE(f.empty());
}

// ═════════════════════════════════════════════════════════════════
// FavoritesManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("FavoritesManager: default empty", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    REQUIRE(mgr.folderCount() == 0);
}

TEST_CASE("FavoritesManager: addFolder", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    REQUIRE(mgr.addFolder(makeFolder("gen", "General")));
    REQUIRE(mgr.folderCount() == 1);
    REQUIRE(mgr.hasFolder("gen"));
}

TEST_CASE("FavoritesManager: addFolder rejects invalid", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    NF::FavoriteFolder bad;
    REQUIRE_FALSE(mgr.addFolder(bad));
}

TEST_CASE("FavoritesManager: addFolder rejects duplicate", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    REQUIRE_FALSE(mgr.addFolder(makeFolder("gen", "General2")));
}

TEST_CASE("FavoritesManager: removeFolder", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    REQUIRE(mgr.removeFolder("gen"));
    REQUIRE(mgr.folderCount() == 0);
}

TEST_CASE("FavoritesManager: removeFolder unknown", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    REQUIRE_FALSE(mgr.removeFolder("nope"));
}

TEST_CASE("FavoritesManager: addItem to folder", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    REQUIRE(mgr.addItem("gen", makeFavItem("a", "Alpha")));
    REQUIRE(mgr.findFolder("gen")->containsItem("a"));
}

TEST_CASE("FavoritesManager: addItem to unknown folder", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    REQUIRE_FALSE(mgr.addItem("nope", makeFavItem("a", "Alpha")));
}

TEST_CASE("FavoritesManager: removeItem from folder", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    mgr.addItem("gen", makeFavItem("a", "Alpha"));
    REQUIRE(mgr.removeItem("gen", "a"));
    REQUIRE_FALSE(mgr.findFolder("gen")->containsItem("a"));
}

TEST_CASE("FavoritesManager: removeItem unknown item", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    REQUIRE_FALSE(mgr.removeItem("gen", "nope"));
}

TEST_CASE("FavoritesManager: globalFavorites merges and sorts by addedMs", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("f1", "Folder1"));
    mgr.addFolder(makeFolder("f2", "Folder2"));
    mgr.addItem("f1", makeFavItem("a", "Alpha", NF::FavoriteItemKind::Asset, 100));
    mgr.addItem("f1", makeFavItem("b", "Beta",  NF::FavoriteItemKind::Asset, 300));
    mgr.addItem("f2", makeFavItem("c", "Charlie", NF::FavoriteItemKind::Tool, 200));

    auto global = mgr.globalFavorites();
    REQUIRE(global.size() == 3);
    REQUIRE(global[0].id == "b");  // 300 - newest
    REQUIRE(global[1].id == "c");  // 200
    REQUIRE(global[2].id == "a");  // 100 - oldest
}

TEST_CASE("FavoritesManager: globalFavorites deduplicates by id", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("f1", "Folder1"));
    mgr.addFolder(makeFolder("f2", "Folder2"));
    mgr.addItem("f1", makeFavItem("a", "Alpha", NF::FavoriteItemKind::Asset, 100));
    mgr.addItem("f2", makeFavItem("a", "AlphaDup", NF::FavoriteItemKind::Asset, 200));

    auto global = mgr.globalFavorites();
    REQUIRE(global.size() == 1);
    REQUIRE(global[0].id == "a");
}

TEST_CASE("FavoritesManager: observer on addItem", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    int addCount = 0;
    mgr.addObserver([&](const NF::FavoriteItem&, bool added) {
        if (added) ++addCount;
    });
    mgr.addItem("gen", makeFavItem("a", "Alpha"));
    REQUIRE(addCount == 1);
}

TEST_CASE("FavoritesManager: observer on removeItem", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    mgr.addItem("gen", makeFavItem("a", "Alpha"));
    int removeCount = 0;
    mgr.addObserver([&](const NF::FavoriteItem&, bool added) {
        if (!added) ++removeCount;
    });
    mgr.removeItem("gen", "a");
    REQUIRE(removeCount == 1);
}

TEST_CASE("FavoritesManager: clearObservers", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    int count = 0;
    mgr.addObserver([&](const NF::FavoriteItem&, bool) { ++count; });
    mgr.clearObservers();
    mgr.addItem("gen", makeFavItem("a", "Alpha"));
    REQUIRE(count == 0);
}

TEST_CASE("FavoritesManager: serialize empty", "[favorites][serialization]") {
    NF::FavoritesManager mgr;
    REQUIRE(mgr.serialize().empty());
}

TEST_CASE("FavoritesManager: serialize round-trip", "[favorites][serialization]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    mgr.addItem("gen", makeFavItem("a", "Alpha", NF::FavoriteItemKind::Asset, 100));
    mgr.addItem("gen", makeFavItem("b", "Beta", NF::FavoriteItemKind::Tool, 200));

    std::string text = mgr.serialize();
    NF::FavoritesManager mgr2;
    REQUIRE(mgr2.deserialize(text));
    REQUIRE(mgr2.folderCount() == 1);
    REQUIRE(mgr2.findFolder("gen")->count() == 2);
    REQUIRE(mgr2.findFolder("gen")->findItem("a")->label == "Alpha");
    REQUIRE(mgr2.findFolder("gen")->findItem("b")->addedMs == 200);
}

TEST_CASE("FavoritesManager: serialize escapes pipe in label", "[favorites][serialization]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    auto item = makeFavItem("x", "Foo|Bar", NF::FavoriteItemKind::File, 50);
    item.path = "path|with|pipes";
    mgr.addItem("gen", item);

    std::string text = mgr.serialize();
    NF::FavoritesManager mgr2;
    REQUIRE(mgr2.deserialize(text));
    auto* restored = mgr2.findFolder("gen")->findItem("x");
    REQUIRE(restored != nullptr);
    REQUIRE(restored->label == "Foo|Bar");
    REQUIRE(restored->path == "path|with|pipes");
}

TEST_CASE("FavoritesManager: deserialize empty", "[favorites][serialization]") {
    NF::FavoritesManager mgr;
    REQUIRE(mgr.deserialize(""));
    REQUIRE(mgr.folderCount() == 0);
}

TEST_CASE("FavoritesManager: clear", "[favorites][manager]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("gen", "General"));
    mgr.addItem("gen", makeFavItem("a", "Alpha"));
    mgr.clear();
    REQUIRE(mgr.folderCount() == 0);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-folder workflow with global view", "[favorites][integration]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("assets", "Assets"));
    mgr.addFolder(makeFolder("tools", "Tools"));

    mgr.addItem("assets", makeFavItem("tex.01", "Cobblestone", NF::FavoriteItemKind::Asset, 300));
    mgr.addItem("assets", makeFavItem("tex.02", "Brick",       NF::FavoriteItemKind::Asset, 100));
    mgr.addItem("tools",  makeFavItem("tool.scene", "Scene Ed", NF::FavoriteItemKind::Tool, 200));

    auto global = mgr.globalFavorites();
    REQUIRE(global.size() == 3);
    REQUIRE(global[0].id == "tex.01");
    REQUIRE(global[2].id == "tex.02");

    // Remove one, verify global updates
    mgr.removeItem("assets", "tex.01");
    global = mgr.globalFavorites();
    REQUIRE(global.size() == 2);
}

TEST_CASE("Integration: serialize/deserialize multi-folder preserves structure", "[favorites][integration]") {
    NF::FavoritesManager mgr;
    mgr.addFolder(makeFolder("f1", "Folder One"));
    mgr.addFolder(makeFolder("f2", "Folder Two"));
    mgr.addItem("f1", makeFavItem("a", "A", NF::FavoriteItemKind::Scene, 10));
    mgr.addItem("f2", makeFavItem("b", "B", NF::FavoriteItemKind::Panel, 20));

    std::string text = mgr.serialize();
    NF::FavoritesManager mgr2;
    mgr2.deserialize(text);

    REQUIRE(mgr2.folderCount() == 2);
    REQUIRE(mgr2.findFolder("f1")->count() == 1);
    REQUIRE(mgr2.findFolder("f2")->count() == 1);
    REQUIRE(mgr2.findFolder("f1")->findItem("a")->kind == NF::FavoriteItemKind::Scene);
    REQUIRE(mgr2.findFolder("f2")->findItem("b")->kind == NF::FavoriteItemKind::Panel);
}
