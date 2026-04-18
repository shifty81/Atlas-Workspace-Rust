// Tests/Workspace/test_phase30_filter_index.cpp
// Phase 30 — Workspace Filter and Search Index
//
// Tests for:
//   1. IndexedItemKind    — enum name helpers
//   2. IndexedItem        — searchable item descriptor; isValid, hasTag, hasField
//   3. FilterQuery        — text / kind / tag / field filter evaluation
//   4. WorkspaceFilterIndex — item registry with query execution and observers
//   5. Integration        — full filter index pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceFilterIndex.h"
#include <string>
#include <vector>

using namespace NF;

static IndexedItem makeItem(const std::string& id, IndexedItemKind kind,
    const std::string& label,
    std::vector<std::string> tags = {},
    std::unordered_map<std::string,std::string> fields = {}) {
    IndexedItem item;
    item.id     = id;
    item.kind   = kind;
    item.label  = label;
    item.tags   = std::move(tags);
    item.fields = std::move(fields);
    return item;
}

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — IndexedItemKind enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("indexedItemKindName returns correct strings", "[Phase30][IndexedItemKind]") {
    CHECK(std::string(indexedItemKindName(IndexedItemKind::Asset))   == "Asset");
    CHECK(std::string(indexedItemKindName(IndexedItemKind::Panel))   == "Panel");
    CHECK(std::string(indexedItemKindName(IndexedItemKind::Tool))    == "Tool");
    CHECK(std::string(indexedItemKindName(IndexedItemKind::Node))    == "Node");
    CHECK(std::string(indexedItemKindName(IndexedItemKind::Command)) == "Command");
    CHECK(std::string(indexedItemKindName(IndexedItemKind::Custom))  == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — IndexedItem
// ═════════════════════════════════════════════════════════════════

TEST_CASE("IndexedItem default is invalid", "[Phase30][IndexedItem]") {
    IndexedItem item;
    CHECK_FALSE(item.isValid());
    CHECK(item.id.empty());
    CHECK(item.label.empty());
    CHECK(item.tags.empty());
    CHECK(item.fields.empty());
}

TEST_CASE("IndexedItem valid construction", "[Phase30][IndexedItem]") {
    auto item = makeItem("asset_1", IndexedItemKind::Asset, "Player Mesh",
        {"character", "mesh"}, {{"type", "fbx"}, {"size", "large"}});
    CHECK(item.isValid());
    CHECK(item.id    == "asset_1");
    CHECK(item.kind  == IndexedItemKind::Asset);
    CHECK(item.label == "Player Mesh");
    CHECK(item.hasTag("character"));
    CHECK(item.hasTag("mesh"));
    CHECK_FALSE(item.hasTag("audio"));
    CHECK(item.hasField("type"));
    CHECK(item.fieldValue("type") == "fbx");
    CHECK(item.fieldValue("missing") == "");
}

TEST_CASE("IndexedItem invalid without id", "[Phase30][IndexedItem]") {
    auto item = makeItem("", IndexedItemKind::Asset, "label");
    CHECK_FALSE(item.isValid());
}

TEST_CASE("IndexedItem invalid without label", "[Phase30][IndexedItem]") {
    auto item = makeItem("id_1", IndexedItemKind::Asset, "");
    CHECK_FALSE(item.isValid());
}

TEST_CASE("IndexedItem equality by id", "[Phase30][IndexedItem]") {
    auto a = makeItem("id_1", IndexedItemKind::Asset,   "A");
    auto b = makeItem("id_1", IndexedItemKind::Command, "B");
    auto c = makeItem("id_2", IndexedItemKind::Asset,   "A");
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — FilterQuery
// ═════════════════════════════════════════════════════════════════

TEST_CASE("FilterQuery empty query matches everything", "[Phase30][FilterQuery]") {
    FilterQuery q;
    auto item = makeItem("id_1", IndexedItemKind::Asset, "Player Mesh");
    CHECK(q.matchesItem(item));
}

TEST_CASE("FilterQuery text match case-insensitive", "[Phase30][FilterQuery]") {
    auto item = makeItem("id_1", IndexedItemKind::Asset, "Player Mesh");
    FilterQuery q;
    q.text = "player";
    CHECK(q.matchesItem(item));
    q.text = "PLAYER";
    CHECK(q.matchesItem(item));
    q.text = "mesh";
    CHECK(q.matchesItem(item));
    q.text = "audio";
    CHECK_FALSE(q.matchesItem(item));
}

TEST_CASE("FilterQuery kind filter", "[Phase30][FilterQuery]") {
    auto item = makeItem("id_1", IndexedItemKind::Asset, "label");
    FilterQuery q;
    q.filterKind = true;
    q.kind       = IndexedItemKind::Asset;
    CHECK(q.matchesItem(item));
    q.kind = IndexedItemKind::Panel;
    CHECK_FALSE(q.matchesItem(item));
}

TEST_CASE("FilterQuery required tags all must match", "[Phase30][FilterQuery]") {
    auto item = makeItem("id_1", IndexedItemKind::Asset, "label", {"a", "b", "c"});
    FilterQuery q;
    q.requiredTags = {"a", "b"};
    CHECK(q.matchesItem(item));
    q.requiredTags = {"a", "d"};
    CHECK_FALSE(q.matchesItem(item));
}

TEST_CASE("FilterQuery required fields all must exist", "[Phase30][FilterQuery]") {
    auto item = makeItem("id_1", IndexedItemKind::Asset, "label", {}, {{"type","fbx"},{"lod","0"}});
    FilterQuery q;
    q.requiredFields = {"type", "lod"};
    CHECK(q.matchesItem(item));
    q.requiredFields = {"type", "missing"};
    CHECK_FALSE(q.matchesItem(item));
}

TEST_CASE("FilterQuery combined text + kind + tags + fields", "[Phase30][FilterQuery]") {
    auto item = makeItem("id_1", IndexedItemKind::Asset, "Player Mesh",
        {"character"}, {{"type", "fbx"}});
    FilterQuery q;
    q.text         = "player";
    q.filterKind   = true;
    q.kind         = IndexedItemKind::Asset;
    q.requiredTags = {"character"};
    q.requiredFields = {"type"};
    CHECK(q.matchesItem(item));

    q.text = "audio"; // text mismatch
    CHECK_FALSE(q.matchesItem(item));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — WorkspaceFilterIndex
// ═════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceFilterIndex empty state", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    CHECK(idx.empty());
    CHECK(idx.count() == 0);
}

TEST_CASE("WorkspaceFilterIndex addItem", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    CHECK(idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh")));
    CHECK(idx.isIndexed("i1"));
    CHECK(idx.count() == 1);
}

TEST_CASE("WorkspaceFilterIndex duplicate addItem fails", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh"));
    CHECK_FALSE(idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh")));
}

TEST_CASE("WorkspaceFilterIndex invalid item rejected", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    CHECK_FALSE(idx.addItem(IndexedItem{}));
}

TEST_CASE("WorkspaceFilterIndex removeItem", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh"));
    CHECK(idx.removeItem("i1"));
    CHECK_FALSE(idx.isIndexed("i1"));
}

TEST_CASE("WorkspaceFilterIndex removeItem unknown fails", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    CHECK_FALSE(idx.removeItem("nope"));
}

TEST_CASE("WorkspaceFilterIndex updateItem", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    idx.addItem(makeItem("i1", IndexedItemKind::Asset, "old label"));
    auto updated = makeItem("i1", IndexedItemKind::Panel, "new label");
    CHECK(idx.updateItem(updated));
    CHECK(idx.findById("i1")->label == "new label");
    CHECK(idx.findById("i1")->kind  == IndexedItemKind::Panel);
}

TEST_CASE("WorkspaceFilterIndex updateItem unknown fails", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    CHECK_FALSE(idx.updateItem(makeItem("nope", IndexedItemKind::Asset, "label")));
}

TEST_CASE("WorkspaceFilterIndex query by text", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    idx.addItem(makeItem("a1", IndexedItemKind::Asset,   "Player Mesh"));
    idx.addItem(makeItem("a2", IndexedItemKind::Asset,   "Player Animation"));
    idx.addItem(makeItem("a3", IndexedItemKind::Command, "Delete"));
    FilterQuery q;
    q.text = "player";
    CHECK(idx.query(q).size() == 2);
}

TEST_CASE("WorkspaceFilterIndex findByKind", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    idx.addItem(makeItem("a1", IndexedItemKind::Asset,   "mesh"));
    idx.addItem(makeItem("a2", IndexedItemKind::Asset,   "texture"));
    idx.addItem(makeItem("p1", IndexedItemKind::Panel,   "outliner"));
    CHECK(idx.findByKind(IndexedItemKind::Asset).size() == 2);
    CHECK(idx.findByKind(IndexedItemKind::Panel).size() == 1);
    CHECK(idx.findByKind(IndexedItemKind::Tool).empty());
}

TEST_CASE("WorkspaceFilterIndex findByTag", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    idx.addItem(makeItem("a1", IndexedItemKind::Asset, "Mesh", {"character","hires"}));
    idx.addItem(makeItem("a2", IndexedItemKind::Asset, "Tex",  {"ui","hires"}));
    idx.addItem(makeItem("a3", IndexedItemKind::Asset, "Anim", {"character"}));
    CHECK(idx.findByTag("character").size() == 2);
    CHECK(idx.findByTag("hires").size()     == 2);
    CHECK(idx.findByTag("ui").size()        == 1);
    CHECK(idx.findByTag("audio").empty());
}

TEST_CASE("WorkspaceFilterIndex allIds", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    idx.addItem(makeItem("i1", IndexedItemKind::Asset,   "A"));
    idx.addItem(makeItem("i2", IndexedItemKind::Panel,   "B"));
    idx.addItem(makeItem("i3", IndexedItemKind::Command, "C"));
    CHECK(idx.allIds().size() == 3);
}

TEST_CASE("WorkspaceFilterIndex observer fires on addItem", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    std::string lastId; bool lastAdded = false;
    idx.addObserver([&](const IndexedItem& i, bool added) { lastId = i.id; lastAdded = added; });
    idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh"));
    CHECK(lastId == "i1");
    CHECK(lastAdded);
}

TEST_CASE("WorkspaceFilterIndex observer fires on removeItem", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    bool lastAdded = true;
    idx.addObserver([&](const IndexedItem&, bool added) { lastAdded = added; });
    idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh"));
    idx.removeItem("i1");
    CHECK_FALSE(lastAdded);
}

TEST_CASE("WorkspaceFilterIndex removeObserver stops notifications", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    int calls = 0;
    uint32_t id = idx.addObserver([&](const IndexedItem&, bool) { ++calls; });
    idx.removeObserver(id);
    idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh"));
    CHECK(calls == 0);
}

TEST_CASE("WorkspaceFilterIndex clear empties all items", "[Phase30][WorkspaceFilterIndex]") {
    WorkspaceFilterIndex idx;
    idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh"));
    idx.addItem(makeItem("i2", IndexedItemKind::Panel, "panel"));
    idx.clear();
    CHECK(idx.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full filter index pipeline: add → query → update → remove", "[Phase30][Integration]") {
    WorkspaceFilterIndex idx;

    std::vector<std::pair<std::string,bool>> events;
    idx.addObserver([&](const IndexedItem& i, bool added) { events.push_back({i.id, added}); });

    idx.addItem(makeItem("asset_mesh",  IndexedItemKind::Asset, "Player Mesh",  {"character","lod0"}, {{"type","fbx"}}));
    idx.addItem(makeItem("asset_tex",   IndexedItemKind::Asset, "Player Tex",   {"character","ui"},   {{"type","png"}}));
    idx.addItem(makeItem("cmd_delete",  IndexedItemKind::Command, "Delete All", {}, {}));
    idx.addItem(makeItem("panel_scene", IndexedItemKind::Panel,   "Scene Panel",{}, {}));

    CHECK(events.size() == 4);

    // Query all assets
    FilterQuery q1;
    q1.filterKind = true;
    q1.kind = IndexedItemKind::Asset;
    CHECK(idx.query(q1).size() == 2);

    // Query by text "player"
    FilterQuery q2;
    q2.text = "player";
    CHECK(idx.query(q2).size() == 2);

    // Query by tag "character"
    FilterQuery q3;
    q3.requiredTags = {"character"};
    CHECK(idx.query(q3).size() == 2);

    // Update mesh → change kind and label
    auto updated = makeItem("asset_mesh", IndexedItemKind::Panel, "Updated Mesh", {"character"}, {});
    idx.updateItem(updated);

    // Now only one Asset left
    CHECK(idx.query(q1).size() == 1);

    idx.removeItem("cmd_delete");
    CHECK(idx.count() == 3);
    CHECK(events.back().second == false);
}

TEST_CASE("Combined filter: text + kind + tags + fields", "[Phase30][Integration]") {
    WorkspaceFilterIndex idx;
    idx.addItem(makeItem("a1", IndexedItemKind::Asset, "Sword Mesh",   {"weapon","lod0"}, {{"type","fbx"}}));
    idx.addItem(makeItem("a2", IndexedItemKind::Asset, "Shield Mesh",  {"armor","lod0"},  {{"type","fbx"}}));
    idx.addItem(makeItem("a3", IndexedItemKind::Asset, "Sword Texture",{"weapon"},        {{"type","png"}}));

    FilterQuery q;
    q.text           = "sword";
    q.filterKind     = true;
    q.kind           = IndexedItemKind::Asset;
    q.requiredTags   = {"weapon"};
    q.requiredFields = {"type"};

    // Matches a1 (sword mesh, weapon, fbx) and a3 (sword texture, weapon, png)
    CHECK(idx.query(q).size() == 2);

    q.text = "mesh";
    CHECK(idx.query(q).size() == 1); // only a1
}

TEST_CASE("clearObservers stops all notifications", "[Phase30][Integration]") {
    WorkspaceFilterIndex idx;
    int calls = 0;
    idx.addObserver([&](const IndexedItem&, bool) { ++calls; });
    idx.clearObservers();
    idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh"));
    CHECK(calls == 0);
}

TEST_CASE("Multiple observers all fire", "[Phase30][Integration]") {
    WorkspaceFilterIndex idx;
    int c1 = 0, c2 = 0;
    idx.addObserver([&](const IndexedItem&, bool) { ++c1; });
    idx.addObserver([&](const IndexedItem&, bool) { ++c2; });
    idx.addItem(makeItem("i1", IndexedItemKind::Asset, "mesh"));
    CHECK(c1 == 1);
    CHECK(c2 == 1);
}
