// Tests/Workspace/test_phase54_breadcrumb.cpp
// Phase 54 — WorkspaceBreadcrumb: BreadcrumbItemKind, BreadcrumbItem,
//             BreadcrumbTrail, BreadcrumbHistory, BreadcrumbManager
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceBreadcrumb.h"

// ═════════════════════════════════════════════════════════════════
// BreadcrumbItemKind
// ═════════════════════════════════════════════════════════════════

TEST_CASE("BreadcrumbItemKind: name helpers", "[breadcrumb][kind]") {
    REQUIRE(std::string(NF::breadcrumbItemKindName(NF::BreadcrumbItemKind::Root))     == "Root");
    REQUIRE(std::string(NF::breadcrumbItemKindName(NF::BreadcrumbItemKind::Category)) == "Category");
    REQUIRE(std::string(NF::breadcrumbItemKindName(NF::BreadcrumbItemKind::Item))     == "Item");
    REQUIRE(std::string(NF::breadcrumbItemKindName(NF::BreadcrumbItemKind::Leaf))     == "Leaf");
}

// ═════════════════════════════════════════════════════════════════
// BreadcrumbItem
// ═════════════════════════════════════════════════════════════════

static NF::BreadcrumbItem makeItem(const std::string& id,
                                   const std::string& label,
                                   NF::BreadcrumbItemKind kind = NF::BreadcrumbItemKind::Item) {
    NF::BreadcrumbItem item;
    item.id    = id;
    item.label = label;
    item.kind  = kind;
    return item;
}

TEST_CASE("BreadcrumbItem: default invalid", "[breadcrumb][item]") {
    NF::BreadcrumbItem item;
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("BreadcrumbItem: valid with id and label", "[breadcrumb][item]") {
    auto item = makeItem("scene.main", "Main Scene");
    REQUIRE(item.isValid());
}

TEST_CASE("BreadcrumbItem: invalid without label", "[breadcrumb][item]") {
    NF::BreadcrumbItem item;
    item.id = "x";
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("BreadcrumbItem: equality by id", "[breadcrumb][item]") {
    auto a = makeItem("a", "A");
    auto b = makeItem("a", "B"); // same id, different label
    auto c = makeItem("c", "C");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ═════════════════════════════════════════════════════════════════
// BreadcrumbTrail
// ═════════════════════════════════════════════════════════════════

TEST_CASE("BreadcrumbTrail: default empty", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    REQUIRE(trail.empty());
    REQUIRE(trail.depth() == 0);
    REQUIRE(trail.current() == nullptr);
    REQUIRE(trail.root()    == nullptr);
}

TEST_CASE("BreadcrumbTrail: push valid item", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    REQUIRE(trail.push(makeItem("proj", "Project", NF::BreadcrumbItemKind::Root)));
    REQUIRE(trail.depth() == 1);
    REQUIRE(trail.current() != nullptr);
    REQUIRE(trail.current()->id == "proj");
    REQUIRE(trail.root()->id    == "proj");
}

TEST_CASE("BreadcrumbTrail: push invalid rejected", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    REQUIRE_FALSE(trail.push(NF::BreadcrumbItem{}));
    REQUIRE(trail.empty());
}

TEST_CASE("BreadcrumbTrail: push duplicate id rejected", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    trail.push(makeItem("a", "A"));
    REQUIRE_FALSE(trail.push(makeItem("a", "A2")));
    REQUIRE(trail.depth() == 1);
}

TEST_CASE("BreadcrumbTrail: pop", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    trail.push(makeItem("a", "A"));
    trail.push(makeItem("b", "B"));
    REQUIRE(trail.pop());
    REQUIRE(trail.depth() == 1);
    REQUIRE(trail.current()->id == "a");
}

TEST_CASE("BreadcrumbTrail: pop empty returns false", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    REQUIRE_FALSE(trail.pop());
}

TEST_CASE("BreadcrumbTrail: contains and findById", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    trail.push(makeItem("proj", "Project"));
    trail.push(makeItem("scene", "Scene"));
    REQUIRE(trail.contains("proj"));
    REQUIRE(trail.contains("scene"));
    REQUIRE_FALSE(trail.contains("missing"));
    REQUIRE(trail.findById("proj")  != nullptr);
    REQUIRE(trail.findById("nope")  == nullptr);
}

TEST_CASE("BreadcrumbTrail: truncateTo", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    trail.push(makeItem("proj",   "Project"));
    trail.push(makeItem("scene",  "Scene"));
    trail.push(makeItem("entity", "Entity"));
    trail.push(makeItem("comp",   "Component"));

    REQUIRE(trail.truncateTo("scene"));
    REQUIRE(trail.depth() == 2);
    REQUIRE(trail.current()->id == "scene");
}

TEST_CASE("BreadcrumbTrail: truncateTo unknown returns false", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    trail.push(makeItem("a", "A"));
    REQUIRE_FALSE(trail.truncateTo("missing"));
    REQUIRE(trail.depth() == 1);
}

TEST_CASE("BreadcrumbTrail: root stays at index 0", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    trail.push(makeItem("r", "Root", NF::BreadcrumbItemKind::Root));
    trail.push(makeItem("a", "A"));
    trail.push(makeItem("b", "B"));
    REQUIRE(trail.root()->id    == "r");
    REQUIRE(trail.current()->id == "b");
}

TEST_CASE("BreadcrumbTrail: clear empties trail", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail trail;
    trail.push(makeItem("a", "A"));
    trail.clear();
    REQUIRE(trail.empty());
}

TEST_CASE("BreadcrumbTrail: equality", "[breadcrumb][trail]") {
    NF::BreadcrumbTrail a, b;
    a.push(makeItem("x", "X"));
    b.push(makeItem("x", "X"));
    REQUIRE(a == b);
    b.push(makeItem("y", "Y"));
    REQUIRE(a != b);
}

// ═════════════════════════════════════════════════════════════════
// BreadcrumbHistory
// ═════════════════════════════════════════════════════════════════

TEST_CASE("BreadcrumbHistory: default empty", "[breadcrumb][history]") {
    NF::BreadcrumbHistory hist;
    REQUIRE(hist.empty());
    REQUIRE_FALSE(hist.canBack());
    REQUIRE_FALSE(hist.canForward());
    REQUIRE(hist.current() == nullptr);
}

TEST_CASE("BreadcrumbHistory: push records trail", "[breadcrumb][history]") {
    NF::BreadcrumbHistory hist;
    NF::BreadcrumbTrail trail;
    trail.push(makeItem("a", "A"));
    hist.push(trail);
    REQUIRE(hist.size() == 1);
    REQUIRE(hist.current() != nullptr);
    REQUIRE(hist.current()->current()->id == "a");
}

TEST_CASE("BreadcrumbHistory: back and forward", "[breadcrumb][history]") {
    NF::BreadcrumbHistory hist;
    NF::BreadcrumbTrail t1, t2, t3;
    t1.push(makeItem("a", "A"));
    t2.push(makeItem("a", "A")); t2.push(makeItem("b", "B"));
    t3.push(makeItem("a", "A")); t3.push(makeItem("b", "B")); t3.push(makeItem("c", "C"));

    hist.push(t1);
    hist.push(t2);
    hist.push(t3);

    REQUIRE(hist.canBack());
    REQUIRE_FALSE(hist.canForward());

    const auto* prev = hist.back();
    REQUIRE(prev != nullptr);
    REQUIRE(prev->depth() == 2); // t2

    REQUIRE(hist.canForward());
    const auto* next = hist.forward();
    REQUIRE(next != nullptr);
    REQUIRE(next->depth() == 3); // t3
}

TEST_CASE("BreadcrumbHistory: back when at start returns nullptr", "[breadcrumb][history]") {
    NF::BreadcrumbHistory hist;
    NF::BreadcrumbTrail t; t.push(makeItem("a", "A"));
    hist.push(t);
    REQUIRE(hist.back() == nullptr);
}

TEST_CASE("BreadcrumbHistory: push clears forward history", "[breadcrumb][history]") {
    NF::BreadcrumbHistory hist;
    NF::BreadcrumbTrail t1, t2, t3;
    t1.push(makeItem("a", "A"));
    t2.push(makeItem("a", "A")); t2.push(makeItem("b", "B"));
    t3.push(makeItem("a", "A")); t3.push(makeItem("b", "B")); t3.push(makeItem("c", "C"));

    hist.push(t1);
    hist.push(t2);
    (void)hist.back(); // go back to t1
    hist.push(t3); // branch from t1 — clears t2 forward entry
    REQUIRE_FALSE(hist.canForward());
}

TEST_CASE("BreadcrumbHistory: clear resets state", "[breadcrumb][history]") {
    NF::BreadcrumbHistory hist;
    NF::BreadcrumbTrail t; t.push(makeItem("a", "A"));
    hist.push(t);
    hist.clear();
    REQUIRE(hist.empty());
    REQUIRE_FALSE(hist.canBack());
}

// ═════════════════════════════════════════════════════════════════
// BreadcrumbManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("BreadcrumbManager: navigate appends to trail", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    REQUIRE(mgr.navigate(makeItem("proj", "Project", NF::BreadcrumbItemKind::Root)));
    REQUIRE(mgr.trail().depth() == 1);
    REQUIRE(mgr.navigate(makeItem("scene", "Scene")));
    REQUIRE(mgr.trail().depth() == 2);
    REQUIRE(mgr.trail().current()->id == "scene");
}

TEST_CASE("BreadcrumbManager: navigate invalid item returns false", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    REQUIRE_FALSE(mgr.navigate(NF::BreadcrumbItem{}));
    REQUIRE(mgr.trail().empty());
}

TEST_CASE("BreadcrumbManager: popTo truncates trail", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    mgr.navigate(makeItem("proj",   "Project"));
    mgr.navigate(makeItem("scene",  "Scene"));
    mgr.navigate(makeItem("entity", "Entity"));

    REQUIRE(mgr.popTo("scene"));
    REQUIRE(mgr.trail().depth() == 2);
    REQUIRE(mgr.trail().current()->id == "scene");
}

TEST_CASE("BreadcrumbManager: popTo unknown returns false", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    mgr.navigate(makeItem("a", "A"));
    REQUIRE_FALSE(mgr.popTo("missing"));
    REQUIRE(mgr.trail().depth() == 1);
}

TEST_CASE("BreadcrumbManager: pop removes top", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    mgr.navigate(makeItem("a", "A"));
    mgr.navigate(makeItem("b", "B"));
    REQUIRE(mgr.pop());
    REQUIRE(mgr.trail().depth() == 1);
    REQUIRE(mgr.trail().current()->id == "a");
}

TEST_CASE("BreadcrumbManager: pop on empty trail returns false", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    REQUIRE_FALSE(mgr.pop());
}

TEST_CASE("BreadcrumbManager: back navigates history", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    mgr.navigate(makeItem("a", "A"));
    mgr.navigate(makeItem("b", "B"));
    REQUIRE(mgr.canBack());
    REQUIRE(mgr.back());
    REQUIRE(mgr.trail().depth() == 1);
    REQUIRE(mgr.trail().current()->id == "a");
}

TEST_CASE("BreadcrumbManager: forward re-applies history", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    mgr.navigate(makeItem("a", "A"));
    mgr.navigate(makeItem("b", "B"));
    mgr.back();
    REQUIRE(mgr.canForward());
    REQUIRE(mgr.forward());
    REQUIRE(mgr.trail().depth() == 2);
    REQUIRE(mgr.trail().current()->id == "b");
}

TEST_CASE("BreadcrumbManager: canBack/canForward", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    REQUIRE_FALSE(mgr.canBack());
    mgr.navigate(makeItem("a", "A"));
    mgr.navigate(makeItem("b", "B"));
    REQUIRE(mgr.canBack());
    REQUIRE_FALSE(mgr.canForward());
    mgr.back();
    REQUIRE(mgr.canForward());
}

TEST_CASE("BreadcrumbManager: reset clears trail and history", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    mgr.navigate(makeItem("a", "A"));
    mgr.navigate(makeItem("b", "B"));
    mgr.reset();
    REQUIRE(mgr.trail().empty());
    REQUIRE_FALSE(mgr.canBack());
    REQUIRE_FALSE(mgr.canForward());
}

TEST_CASE("BreadcrumbManager: observer fires on navigate", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.navigate(makeItem("a", "A"));
    REQUIRE(count == 1);
}

TEST_CASE("BreadcrumbManager: observer fires on popTo", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    mgr.navigate(makeItem("a", "A"));
    mgr.navigate(makeItem("b", "B"));
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.popTo("a");
    REQUIRE(count == 1);
}

TEST_CASE("BreadcrumbManager: observer fires on back/forward", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    mgr.navigate(makeItem("a", "A"));
    mgr.navigate(makeItem("b", "B"));
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.back();
    REQUIRE(count == 1);
    mgr.forward();
    REQUIRE(count == 2);
}

TEST_CASE("BreadcrumbManager: observer fires on reset", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.reset();
    REQUIRE(count == 1);
}

TEST_CASE("BreadcrumbManager: clearObservers stops notifications", "[breadcrumb][manager]") {
    NF::BreadcrumbManager mgr;
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.navigate(makeItem("a", "A"));
    REQUIRE(count == 1);
    mgr.clearObservers();
    mgr.navigate(makeItem("b", "B"));
    REQUIRE(count == 1);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Breadcrumb integration: full drill-down and back navigation", "[breadcrumb][integration]") {
    NF::BreadcrumbManager mgr;

    // Simulate Project → Scene → Entity → Component
    mgr.navigate(makeItem("proj",  "MyProject", NF::BreadcrumbItemKind::Root));
    mgr.navigate(makeItem("scene", "MainScene", NF::BreadcrumbItemKind::Category));
    mgr.navigate(makeItem("hero",  "Hero",      NF::BreadcrumbItemKind::Item));
    mgr.navigate(makeItem("health","Health",    NF::BreadcrumbItemKind::Leaf));

    REQUIRE(mgr.trail().depth()       == 4);
    REQUIRE(mgr.trail().current()->id == "health");

    // Navigate back twice
    mgr.back();
    REQUIRE(mgr.trail().current()->id == "hero");
    mgr.back();
    REQUIRE(mgr.trail().current()->id == "scene");

    // Navigate to a different leaf from scene
    mgr.navigate(makeItem("enemies", "Enemies", NF::BreadcrumbItemKind::Item));
    REQUIRE(mgr.trail().depth() == 3);
    REQUIRE_FALSE(mgr.canForward()); // forward history cleared by new navigate
}

TEST_CASE("Breadcrumb integration: popTo mid-trail then continue", "[breadcrumb][integration]") {
    NF::BreadcrumbManager mgr;
    mgr.navigate(makeItem("root",  "Root"));
    mgr.navigate(makeItem("dir",   "Dir"));
    mgr.navigate(makeItem("subdir","SubDir"));
    mgr.navigate(makeItem("file",  "File"));

    mgr.popTo("dir");
    REQUIRE(mgr.trail().depth() == 2);
    REQUIRE(mgr.trail().current()->id == "dir");

    mgr.navigate(makeItem("other", "Other"));
    REQUIRE(mgr.trail().depth() == 3);
    REQUIRE(mgr.trail().current()->id == "other");
}
