// Tests/Workspace/test_phase29_annotation.cpp
// Phase 29 — Workspace Annotation System
//
// Tests for:
//   1. AnnotationKind   — enum name helpers
//   2. AnnotationAnchor — anchor location; isValid
//   3. Annotation       — annotation descriptor; isValid, equality, timestamp
//   4. AnnotationManager— add/remove/update/resolve/reopen/filter/observer
//   5. Integration      — full annotation pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceAnnotation.h"
#include <string>
#include <vector>

using namespace NF;

static Annotation makeAnnotation(const std::string& id, AnnotationKind kind = AnnotationKind::Note,
    const std::string& body = "body text", const std::string& target = "panel_a",
    const std::string& author = "user1") {
    return {id, kind, author, body, {target, "", 0.0f, 0.0f}, false, 0};
}

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — AnnotationKind enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("annotationKindName returns correct strings", "[Phase29][AnnotationKind]") {
    CHECK(std::string(annotationKindName(AnnotationKind::Note))     == "Note");
    CHECK(std::string(annotationKindName(AnnotationKind::Warning))  == "Warning");
    CHECK(std::string(annotationKindName(AnnotationKind::Todo))     == "Todo");
    CHECK(std::string(annotationKindName(AnnotationKind::Bookmark)) == "Bookmark");
    CHECK(std::string(annotationKindName(AnnotationKind::Review))   == "Review");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — AnnotationAnchor
// ═════════════════════════════════════════════════════════════════

TEST_CASE("AnnotationAnchor default is invalid", "[Phase29][AnnotationAnchor]") {
    AnnotationAnchor a;
    CHECK_FALSE(a.isValid());
    CHECK(a.targetId.empty());
}

TEST_CASE("AnnotationAnchor valid with targetId", "[Phase29][AnnotationAnchor]") {
    AnnotationAnchor a{"panel_scene", "node_42", 1.5f, 2.0f};
    CHECK(a.isValid());
    CHECK(a.targetId    == "panel_scene");
    CHECK(a.contextKey  == "node_42");
    CHECK(a.x           == 1.5f);
    CHECK(a.y           == 2.0f);
}

TEST_CASE("AnnotationAnchor invalid without targetId", "[Phase29][AnnotationAnchor]") {
    AnnotationAnchor a{"", "ctx", 0.0f, 0.0f};
    CHECK_FALSE(a.isValid());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — Annotation
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Annotation default is invalid", "[Phase29][Annotation]") {
    Annotation a;
    CHECK_FALSE(a.isValid());
    CHECK(a.id.empty());
    CHECK_FALSE(a.resolved);
    CHECK(a.kind == AnnotationKind::Note);
}

TEST_CASE("Annotation valid construction", "[Phase29][Annotation]") {
    Annotation a = makeAnnotation("ann_1", AnnotationKind::Todo, "Fix this", "panel_a", "alice");
    CHECK(a.isValid());
    CHECK(a.id     == "ann_1");
    CHECK(a.kind   == AnnotationKind::Todo);
    CHECK(a.body   == "Fix this");
    CHECK(a.author == "alice");
    CHECK_FALSE(a.resolved);
}

TEST_CASE("Annotation invalid without id", "[Phase29][Annotation]") {
    auto a = makeAnnotation("");
    CHECK_FALSE(a.isValid());
}

TEST_CASE("Annotation invalid without body", "[Phase29][Annotation]") {
    Annotation a{"ann_1", AnnotationKind::Note, "user", "", {"target", "", 0, 0}, false, 0};
    CHECK_FALSE(a.isValid());
}

TEST_CASE("Annotation invalid without anchor target", "[Phase29][Annotation]") {
    Annotation a{"ann_1", AnnotationKind::Note, "user", "body", {"", "", 0, 0}, false, 0};
    CHECK_FALSE(a.isValid());
}

TEST_CASE("Annotation equality by id", "[Phase29][Annotation]") {
    auto a = makeAnnotation("id_1");
    auto b = makeAnnotation("id_1", AnnotationKind::Warning);
    auto c = makeAnnotation("id_2");
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — AnnotationManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("AnnotationManager empty state", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    CHECK(m.empty());
    CHECK(m.count() == 0);
}

TEST_CASE("AnnotationManager add", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    CHECK(m.add(makeAnnotation("a1")));
    CHECK(m.isRegistered("a1"));
    CHECK(m.count() == 1);
}

TEST_CASE("AnnotationManager duplicate add fails", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    CHECK_FALSE(m.add(makeAnnotation("a1")));
}

TEST_CASE("AnnotationManager invalid annotation rejected", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    CHECK_FALSE(m.add(Annotation{}));
}

TEST_CASE("AnnotationManager add assigns timestamp", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    m.add(makeAnnotation("a2"));
    CHECK(m.findById("a1")->timestamp < m.findById("a2")->timestamp);
}

TEST_CASE("AnnotationManager remove", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    CHECK(m.remove("a1"));
    CHECK_FALSE(m.isRegistered("a1"));
}

TEST_CASE("AnnotationManager remove unknown fails", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    CHECK_FALSE(m.remove("nope"));
}

TEST_CASE("AnnotationManager update", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1", AnnotationKind::Note, "original"));
    auto updated = makeAnnotation("a1", AnnotationKind::Todo, "updated body");
    CHECK(m.update(updated));
    CHECK(m.findById("a1")->body == "updated body");
    CHECK(m.findById("a1")->kind == AnnotationKind::Todo);
}

TEST_CASE("AnnotationManager update unknown fails", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    CHECK_FALSE(m.update(makeAnnotation("nope")));
}

TEST_CASE("AnnotationManager resolve", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    CHECK(m.resolve("a1"));
    CHECK(m.findById("a1")->resolved);
}

TEST_CASE("AnnotationManager resolve already resolved fails", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    m.resolve("a1");
    CHECK_FALSE(m.resolve("a1"));
}

TEST_CASE("AnnotationManager reopen", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    m.resolve("a1");
    CHECK(m.reopen("a1"));
    CHECK_FALSE(m.findById("a1")->resolved);
}

TEST_CASE("AnnotationManager reopen already open fails", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    CHECK_FALSE(m.reopen("a1"));
}

TEST_CASE("AnnotationManager findByTarget", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1", AnnotationKind::Note, "body", "panel_a"));
    m.add(makeAnnotation("a2", AnnotationKind::Note, "body", "panel_a"));
    m.add(makeAnnotation("a3", AnnotationKind::Note, "body", "panel_b"));
    CHECK(m.findByTarget("panel_a").size() == 2);
    CHECK(m.findByTarget("panel_b").size() == 1);
}

TEST_CASE("AnnotationManager findByAuthor", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1", AnnotationKind::Note, "b", "t", "alice"));
    m.add(makeAnnotation("a2", AnnotationKind::Note, "b", "t", "alice"));
    m.add(makeAnnotation("a3", AnnotationKind::Note, "b", "t", "bob"));
    CHECK(m.findByAuthor("alice").size() == 2);
    CHECK(m.findByAuthor("bob").size()   == 1);
}

TEST_CASE("AnnotationManager findByKind", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1", AnnotationKind::Todo));
    m.add(makeAnnotation("a2", AnnotationKind::Todo));
    m.add(makeAnnotation("a3", AnnotationKind::Note));
    CHECK(m.findByKind(AnnotationKind::Todo).size() == 2);
    CHECK(m.findByKind(AnnotationKind::Note).size() == 1);
}

TEST_CASE("AnnotationManager unresolved and resolved filters", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    m.add(makeAnnotation("a2"));
    m.add(makeAnnotation("a3"));
    m.resolve("a2");
    CHECK(m.unresolved().size() == 2);
    CHECK(m.resolved().size()   == 1);
}

TEST_CASE("AnnotationManager allIds", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    m.add(makeAnnotation("a2"));
    m.add(makeAnnotation("a3"));
    CHECK(m.allIds().size() == 3);
}

TEST_CASE("AnnotationManager observer fires on add", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    std::string lastId; bool lastAdded = false;
    m.addObserver([&](const Annotation& a, bool added) { lastId = a.id; lastAdded = added; });
    m.add(makeAnnotation("a1"));
    CHECK(lastId == "a1");
    CHECK(lastAdded);
}

TEST_CASE("AnnotationManager observer fires on remove", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    bool lastAdded = true;
    m.addObserver([&](const Annotation&, bool added) { lastAdded = added; });
    m.add(makeAnnotation("a1"));
    m.remove("a1");
    CHECK_FALSE(lastAdded);
}

TEST_CASE("AnnotationManager observer fires on resolve", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    bool lastResolved = false;
    m.addObserver([&](const Annotation& a, bool) { lastResolved = a.resolved; });
    m.add(makeAnnotation("a1"));
    m.resolve("a1");
    CHECK(lastResolved);
}

TEST_CASE("AnnotationManager removeObserver stops notifications", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    int calls = 0;
    uint32_t id = m.addObserver([&](const Annotation&, bool) { ++calls; });
    m.removeObserver(id);
    m.add(makeAnnotation("a1"));
    CHECK(calls == 0);
}

TEST_CASE("AnnotationManager clear empties all", "[Phase29][AnnotationManager]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    m.add(makeAnnotation("a2"));
    m.clear();
    CHECK(m.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full annotation pipeline: add → resolve → reopen → remove", "[Phase29][Integration]") {
    AnnotationManager m;

    std::vector<std::pair<std::string,bool>> events;
    m.addObserver([&](const Annotation& a, bool added) { events.push_back({a.id, added}); });

    m.add(makeAnnotation("todo_1", AnnotationKind::Todo, "Fix null check", "asset_manager"));
    m.add(makeAnnotation("note_1", AnnotationKind::Note, "Great work here", "asset_manager"));
    m.resolve("todo_1");
    m.reopen("todo_1");
    m.remove("note_1");

    // add(1) + add(2) + resolve(3) + reopen(4) + remove(5)
    CHECK(events.size() == 5);
    CHECK(events[4].second == false); // remove
    CHECK(m.count() == 1);
    CHECK(m.unresolved().size() == 1);
}

TEST_CASE("Filter by target and kind returns correct subsets", "[Phase29][Integration]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1", AnnotationKind::Todo,     "Fix", "scene_panel", "alice"));
    m.add(makeAnnotation("a2", AnnotationKind::Review,   "Rev", "scene_panel", "bob"));
    m.add(makeAnnotation("a3", AnnotationKind::Todo,     "Fix", "asset_panel", "alice"));
    m.add(makeAnnotation("a4", AnnotationKind::Bookmark, "Bkm", "asset_panel", "alice"));

    CHECK(m.findByTarget("scene_panel").size() == 2);
    CHECK(m.findByTarget("asset_panel").size() == 2);
    CHECK(m.findByKind(AnnotationKind::Todo).size()     == 2);
    CHECK(m.findByKind(AnnotationKind::Review).size()   == 1);
    CHECK(m.findByKind(AnnotationKind::Bookmark).size() == 1);
    CHECK(m.findByAuthor("alice").size() == 3);
    CHECK(m.findByAuthor("bob").size()   == 1);
}

TEST_CASE("Timestamps increment monotonically", "[Phase29][Integration]") {
    AnnotationManager m;
    m.add(makeAnnotation("a1"));
    m.add(makeAnnotation("a2"));
    m.add(makeAnnotation("a3"));
    CHECK(m.findById("a1")->timestamp < m.findById("a2")->timestamp);
    CHECK(m.findById("a2")->timestamp < m.findById("a3")->timestamp);
}

TEST_CASE("clearObservers stops all notifications", "[Phase29][Integration]") {
    AnnotationManager m;
    int calls = 0;
    m.addObserver([&](const Annotation&, bool) { ++calls; });
    m.clearObservers();
    m.add(makeAnnotation("a1"));
    CHECK(calls == 0);
}
