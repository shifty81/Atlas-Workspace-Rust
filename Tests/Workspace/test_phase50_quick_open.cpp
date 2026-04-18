// Tests/Workspace/test_phase50_quick_open.cpp
// Phase 50 — WorkspaceQuickOpen: QuickOpenItemKind, QuickOpenItem,
//             QuickOpenQuery, QuickOpenProvider, QuickOpenSession, QuickOpenManager
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceQuickOpen.h"

// ─── Helper: build a simple provider ────────────────────────────────────

static NF::QuickOpenProvider makeProvider(const std::string& id,
                                          std::vector<NF::QuickOpenItem> items) {
    NF::QuickOpenProvider p;
    p.id      = id;
    p.name    = id;
    p.populate = [items](const NF::QuickOpenQuery&) { return items; };
    return p;
}

// ═════════════════════════════════════════════════════════════════
// QuickOpenItemKind
// ═════════════════════════════════════════════════════════════════

TEST_CASE("QuickOpenItemKind: name helpers", "[quickopen][kind]") {
    REQUIRE(std::string(NF::quickOpenItemKindName(NF::QuickOpenItemKind::File))    == "File");
    REQUIRE(std::string(NF::quickOpenItemKindName(NF::QuickOpenItemKind::Tool))    == "Tool");
    REQUIRE(std::string(NF::quickOpenItemKindName(NF::QuickOpenItemKind::Command)) == "Command");
    REQUIRE(std::string(NF::quickOpenItemKindName(NF::QuickOpenItemKind::Symbol))  == "Symbol");
    REQUIRE(std::string(NF::quickOpenItemKindName(NF::QuickOpenItemKind::Custom))  == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// QuickOpenItem
// ═════════════════════════════════════════════════════════════════

TEST_CASE("QuickOpenItem: default invalid", "[quickopen][item]") {
    NF::QuickOpenItem item;
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("QuickOpenItem: valid with id and label", "[quickopen][item]") {
    NF::QuickOpenItem item;
    item.id    = "tool.scene_editor";
    item.label = "Scene Editor";
    item.kind  = NF::QuickOpenItemKind::Tool;
    REQUIRE(item.isValid());
}

TEST_CASE("QuickOpenItem: invalid without label", "[quickopen][item]") {
    NF::QuickOpenItem item;
    item.id = "x";
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("QuickOpenItem: invalid without id", "[quickopen][item]") {
    NF::QuickOpenItem item;
    item.label = "X";
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("QuickOpenItem: equality by id", "[quickopen][item]") {
    NF::QuickOpenItem a, b, c;
    a.id = "x"; a.label = "X";
    b.id = "x"; b.label = "Y";
    c.id = "z"; c.label = "Z";
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ═════════════════════════════════════════════════════════════════
// QuickOpenQuery
// ═════════════════════════════════════════════════════════════════

TEST_CASE("QuickOpenQuery: empty text matches all items", "[quickopen][query]") {
    NF::QuickOpenItem item;
    item.id = "t"; item.label = "anything";
    item.kind = NF::QuickOpenItemKind::File;
    NF::QuickOpenQuery q;
    REQUIRE(q.matches(item));
    REQUIRE(q.score(item) == NF::QuickOpenQuery::kScoreContains);
}

TEST_CASE("QuickOpenQuery: case-insensitive match", "[quickopen][query]") {
    NF::QuickOpenItem item;
    item.id = "t"; item.label = "SceneEditor";
    NF::QuickOpenQuery q;
    q.text = "sceneeditor";
    REQUIRE(q.matches(item));
    q.text = "SCENEEDITOR";
    REQUIRE(q.matches(item));
    q.text = "SceneEditor";
    REQUIRE(q.matches(item));
}

TEST_CASE("QuickOpenQuery: non-matching text returns false", "[quickopen][query]") {
    NF::QuickOpenItem item;
    item.id = "t"; item.label = "SceneEditor";
    NF::QuickOpenQuery q;
    q.text = "physics";
    REQUIRE_FALSE(q.matches(item));
    REQUIRE(q.score(item) == NF::QuickOpenQuery::kScoreNone);
}

TEST_CASE("QuickOpenQuery: score Exact > Prefix > Contains", "[quickopen][query]") {
    auto makeItem = [](const std::string& label) {
        NF::QuickOpenItem i;
        i.id = label; i.label = label;
        return i;
    };

    NF::QuickOpenQuery q;
    q.text = "scene";

    auto exact    = makeItem("scene");
    auto prefix   = makeItem("sceneeditor");
    auto contains = makeItem("my scene file");

    REQUIRE(q.score(exact)    == NF::QuickOpenQuery::kScoreExact);
    REQUIRE(q.score(prefix)   == NF::QuickOpenQuery::kScorePrefix);
    REQUIRE(q.score(contains) == NF::QuickOpenQuery::kScoreContains);

    REQUIRE(q.score(exact)    > q.score(prefix));
    REQUIRE(q.score(prefix)   > q.score(contains));
    REQUIRE(q.score(contains) > NF::QuickOpenQuery::kScoreNone);
}

TEST_CASE("QuickOpenQuery: kind filter", "[quickopen][query]") {
    NF::QuickOpenItem fileItem, toolItem;
    fileItem.id = "f"; fileItem.label = "scene.scene";
    fileItem.kind = NF::QuickOpenItemKind::File;
    toolItem.id = "t"; toolItem.label = "scene editor";
    toolItem.kind = NF::QuickOpenItemKind::Tool;

    NF::QuickOpenQuery q;
    q.text         = "scene";
    q.filterByKind = true;
    q.filterKind   = NF::QuickOpenItemKind::Tool;

    REQUIRE_FALSE(q.matches(fileItem));
    REQUIRE(q.matches(toolItem));
}

// ═════════════════════════════════════════════════════════════════
// QuickOpenProvider
// ═════════════════════════════════════════════════════════════════

TEST_CASE("QuickOpenProvider: invalid without id", "[quickopen][provider]") {
    NF::QuickOpenProvider p;
    p.name     = "N";
    p.populate = [](const NF::QuickOpenQuery&) { return std::vector<NF::QuickOpenItem>{}; };
    REQUIRE_FALSE(p.isValid());
}

TEST_CASE("QuickOpenProvider: invalid without name", "[quickopen][provider]") {
    NF::QuickOpenProvider p;
    p.id       = "x";
    p.populate = [](const NF::QuickOpenQuery&) { return std::vector<NF::QuickOpenItem>{}; };
    REQUIRE_FALSE(p.isValid());
}

TEST_CASE("QuickOpenProvider: invalid without populate", "[quickopen][provider]") {
    NF::QuickOpenProvider p;
    p.id   = "x";
    p.name = "X";
    REQUIRE_FALSE(p.isValid());
}

TEST_CASE("QuickOpenProvider: valid with all fields", "[quickopen][provider]") {
    auto p = makeProvider("files", {});
    REQUIRE(p.isValid());
}

// ═════════════════════════════════════════════════════════════════
// QuickOpenSession
// ═════════════════════════════════════════════════════════════════

TEST_CASE("QuickOpenSession: default closed state", "[quickopen][session]") {
    NF::QuickOpenSession sess("palette.main");
    REQUIRE(sess.id() == "palette.main");
    REQUIRE_FALSE(sess.isOpen());
    REQUIRE(sess.results().empty());
    REQUIRE_FALSE(sess.hasSubmit());
}

TEST_CASE("QuickOpenSession: addProvider valid", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    REQUIRE(sess.addProvider(makeProvider("files", {})));
    REQUIRE(sess.providerCount() == 1);
    REQUIRE(sess.hasProvider("files"));
}

TEST_CASE("QuickOpenSession: addProvider invalid rejected", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    NF::QuickOpenProvider bad;
    REQUIRE_FALSE(sess.addProvider(bad));
    REQUIRE(sess.providerCount() == 0);
}

TEST_CASE("QuickOpenSession: addProvider duplicate rejected", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    REQUIRE(sess.addProvider(makeProvider("files", {})));
    REQUIRE_FALSE(sess.addProvider(makeProvider("files", {})));
    REQUIRE(sess.providerCount() == 1);
}

TEST_CASE("QuickOpenSession: removeProvider", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    sess.addProvider(makeProvider("files", {}));
    REQUIRE(sess.removeProvider("files"));
    REQUIRE(sess.providerCount() == 0);
    REQUIRE_FALSE(sess.hasProvider("files"));
}

TEST_CASE("QuickOpenSession: removeProvider unknown returns false", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    REQUIRE_FALSE(sess.removeProvider("no.such"));
}

TEST_CASE("QuickOpenSession: open clears state and marks open", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    sess.open();
    REQUIRE(sess.isOpen());
    REQUIRE(sess.results().empty());
    REQUIRE_FALSE(sess.hasSubmit());
}

TEST_CASE("QuickOpenSession: close marks closed", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    sess.open();
    sess.close();
    REQUIRE_FALSE(sess.isOpen());
}

TEST_CASE("QuickOpenSession: query when not open returns 0", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    NF::QuickOpenItem item;
    item.id = "x"; item.label = "X";
    sess.addProvider(makeProvider("p", {item}));
    NF::QuickOpenQuery q;
    REQUIRE(sess.query(q) == 0);
    REQUIRE(sess.results().empty());
}

TEST_CASE("QuickOpenSession: query collects from all providers", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    NF::QuickOpenItem a, b;
    a.id = "a"; a.label = "Alpha";
    b.id = "b"; b.label = "Beta";
    sess.addProvider(makeProvider("p1", {a}));
    sess.addProvider(makeProvider("p2", {b}));
    sess.open();
    NF::QuickOpenQuery q;
    REQUIRE(sess.query(q) == 2);
    REQUIRE(sess.results().size() == 2);
}

TEST_CASE("QuickOpenSession: query filters by text", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    NF::QuickOpenItem a, b, c;
    a.id = "a"; a.label = "Scene Editor";
    b.id = "b"; b.label = "Asset Browser";
    c.id = "c"; c.label = "scene helper";
    sess.addProvider(makeProvider("p", {a, b, c}));
    sess.open();
    NF::QuickOpenQuery q;
    q.text = "scene";
    REQUIRE(sess.query(q) == 2);
}

TEST_CASE("QuickOpenSession: query sorts by score descending", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    NF::QuickOpenItem exact, prefix, contains;
    exact.id    = "e"; exact.label    = "scene";
    prefix.id   = "p"; prefix.label   = "sceneeditor";
    contains.id = "c"; contains.label = "my scene file";
    sess.addProvider(makeProvider("p", {contains, prefix, exact}));
    sess.open();
    NF::QuickOpenQuery q;
    q.text = "scene";
    sess.query(q);
    REQUIRE(sess.results()[0].id == "e"); // exact first
    REQUIRE(sess.results()[1].id == "p"); // prefix second
    REQUIRE(sess.results()[2].id == "c"); // contains third
}

TEST_CASE("QuickOpenSession: query caps at maxResults", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    std::vector<NF::QuickOpenItem> items;
    for (int i = 0; i < 10; ++i) {
        NF::QuickOpenItem it;
        it.id    = std::to_string(i);
        it.label = "item " + std::to_string(i);
        items.push_back(it);
    }
    sess.addProvider(makeProvider("p", items));
    sess.open();
    NF::QuickOpenQuery q;
    q.maxResults = 3;
    REQUIRE(sess.query(q) == 3);
    REQUIRE(sess.results().size() == 3);
}

TEST_CASE("QuickOpenSession: submit valid item", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    NF::QuickOpenItem item;
    item.id = "t"; item.label = "Tool";
    sess.addProvider(makeProvider("p", {item}));
    sess.open();
    NF::QuickOpenQuery q;
    sess.query(q);
    REQUIRE(sess.submit("t"));
    REQUIRE_FALSE(sess.isOpen());
    REQUIRE(sess.hasSubmit());
    REQUIRE(sess.submitted() != nullptr);
    REQUIRE(sess.submitted()->id == "t");
}

TEST_CASE("QuickOpenSession: submit unknown id returns false", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    NF::QuickOpenItem item;
    item.id = "t"; item.label = "Tool";
    sess.addProvider(makeProvider("p", {item}));
    sess.open();
    NF::QuickOpenQuery q;
    sess.query(q);
    REQUIRE_FALSE(sess.submit("no.such.id"));
    REQUIRE(sess.isOpen()); // still open
}

TEST_CASE("QuickOpenSession: submit when not open returns false", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    REQUIRE_FALSE(sess.submit("anything"));
}

TEST_CASE("QuickOpenSession: open clears previous submission", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    NF::QuickOpenItem item;
    item.id = "t"; item.label = "T";
    sess.addProvider(makeProvider("p", {item}));
    sess.open();
    NF::QuickOpenQuery q;
    sess.query(q);
    sess.submit("t");
    REQUIRE(sess.hasSubmit());
    // Re-open
    sess.open();
    REQUIRE_FALSE(sess.hasSubmit());
}

TEST_CASE("QuickOpenSession: MAX_PROVIDERS capacity enforced", "[quickopen][session]") {
    NF::QuickOpenSession sess("s");
    for (size_t i = 0; i < NF::QuickOpenSession::MAX_PROVIDERS; ++i) {
        std::string pid = "p" + std::to_string(i);
        REQUIRE(sess.addProvider(makeProvider(pid, {})));
    }
    REQUIRE_FALSE(sess.addProvider(makeProvider("overflow", {})));
    REQUIRE(sess.providerCount() == NF::QuickOpenSession::MAX_PROVIDERS);
}

// ═════════════════════════════════════════════════════════════════
// QuickOpenManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("QuickOpenManager: default empty state", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    REQUIRE(mgr.empty());
    REQUIRE(mgr.sessionCount() == 0);
}

TEST_CASE("QuickOpenManager: createSession", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    auto* sess = mgr.createSession("palette.main");
    REQUIRE(sess != nullptr);
    REQUIRE(sess->id() == "palette.main");
    REQUIRE(mgr.sessionCount() == 1);
    REQUIRE_FALSE(mgr.empty());
}

TEST_CASE("QuickOpenManager: createSession duplicate rejected", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    REQUIRE(mgr.createSession("s") != nullptr);
    REQUIRE(mgr.createSession("s") == nullptr);
    REQUIRE(mgr.sessionCount() == 1);
}

TEST_CASE("QuickOpenManager: createSession empty id rejected", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    REQUIRE(mgr.createSession("") == nullptr);
}

TEST_CASE("QuickOpenManager: removeSession", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    mgr.createSession("s");
    REQUIRE(mgr.removeSession("s"));
    REQUIRE(mgr.empty());
}

TEST_CASE("QuickOpenManager: removeSession unknown returns false", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    REQUIRE_FALSE(mgr.removeSession("no.such"));
}

TEST_CASE("QuickOpenManager: findSession", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    mgr.createSession("palette");
    REQUIRE(mgr.findSession("palette") != nullptr);
    REQUIRE(mgr.findSession("missing") == nullptr);
    REQUIRE(mgr.hasSession("palette"));
    REQUIRE_FALSE(mgr.hasSession("missing"));
}

TEST_CASE("QuickOpenManager: observer fires on notifySubmit", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    mgr.createSession("s");

    std::vector<std::pair<std::string, std::string>> log;
    mgr.addObserver([&](const std::string& sid, const NF::QuickOpenItem& item) {
        log.push_back({sid, item.id});
    });

    NF::QuickOpenItem item;
    item.id = "tool.scene"; item.label = "Scene Editor";
    mgr.notifySubmit("s", item);

    REQUIRE(log.size() == 1);
    REQUIRE(log[0].first  == "s");
    REQUIRE(log[0].second == "tool.scene");
}

TEST_CASE("QuickOpenManager: clearObservers stops notifications", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    int count = 0;
    mgr.addObserver([&](const std::string&, const NF::QuickOpenItem&) { ++count; });
    NF::QuickOpenItem item;
    item.id = "x"; item.label = "X";
    mgr.notifySubmit("s", item);
    REQUIRE(count == 1);
    mgr.clearObservers();
    mgr.notifySubmit("s", item);
    REQUIRE(count == 1);
}

TEST_CASE("QuickOpenManager: MAX_SESSIONS capacity enforced", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    for (size_t i = 0; i < NF::QuickOpenManager::MAX_SESSIONS; ++i) {
        REQUIRE(mgr.createSession("s" + std::to_string(i)) != nullptr);
    }
    REQUIRE(mgr.createSession("overflow") == nullptr);
}

TEST_CASE("QuickOpenManager: clear removes all", "[quickopen][manager]") {
    NF::QuickOpenManager mgr;
    mgr.createSession("s");
    int count = 0;
    mgr.addObserver([&](const std::string&, const NF::QuickOpenItem&) { ++count; });
    mgr.clear();
    REQUIRE(mgr.empty());
    NF::QuickOpenItem item; item.id = "x"; item.label = "X";
    mgr.notifySubmit("s", item);
    REQUIRE(count == 0); // observers cleared
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("QuickOpen integration: full open-query-submit flow", "[quickopen][integration]") {
    NF::QuickOpenManager mgr;
    auto* sess = mgr.createSession("palette.main");
    REQUIRE(sess != nullptr);

    // Build providers
    NF::QuickOpenItem tool, file1, file2;
    tool.id = "tool.scene_editor"; tool.label = "Scene Editor";
    tool.kind = NF::QuickOpenItemKind::Tool;
    file1.id = "file.level1"; file1.label = "level1.scene";
    file1.kind = NF::QuickOpenItemKind::File;
    file2.id = "file.level2"; file2.label = "level2.scene";
    file2.kind = NF::QuickOpenItemKind::File;

    sess->addProvider(makeProvider("tools", {tool}));
    sess->addProvider(makeProvider("files", {file1, file2}));

    // Wire submit observer
    std::string submittedId;
    mgr.addObserver([&](const std::string&, const NF::QuickOpenItem& item) {
        submittedId = item.id;
    });

    // Open and query
    sess->open();
    NF::QuickOpenQuery q;
    q.text = "level";
    sess->query(q);
    REQUIRE(sess->results().size() == 2);

    // Submit first result
    const std::string& firstId = sess->results()[0].id;
    REQUIRE(sess->submit(firstId));
    mgr.notifySubmit(sess->id(), *sess->submitted());

    REQUIRE_FALSE(sess->isOpen());
    REQUIRE(submittedId == firstId);
}

TEST_CASE("QuickOpen integration: kind filter narrows results", "[quickopen][integration]") {
    NF::QuickOpenManager mgr;
    auto* sess = mgr.createSession("palette");

    NF::QuickOpenItem t, f;
    t.id = "t"; t.label = "scene editor"; t.kind = NF::QuickOpenItemKind::Tool;
    f.id = "f"; f.label = "scene.scene";  f.kind = NF::QuickOpenItemKind::File;

    sess->addProvider(makeProvider("all", {t, f}));
    sess->open();

    NF::QuickOpenQuery q;
    q.text         = "scene";
    q.filterByKind = true;
    q.filterKind   = NF::QuickOpenItemKind::File;

    sess->query(q);
    REQUIRE(sess->results().size() == 1);
    REQUIRE(sess->results()[0].id == "f");
}

TEST_CASE("QuickOpen integration: multiple providers merged and ranked", "[quickopen][integration]") {
    NF::QuickOpenSession sess("s");

    // Provider 1: exact match + one miss
    NF::QuickOpenItem exact, prefix, noMatch;
    exact.id   = "e"; exact.label   = "scene";
    prefix.id  = "p"; prefix.label  = "scene_editor";
    noMatch.id = "n"; noMatch.label = "physics_editor";

    sess.addProvider(makeProvider("p1", {exact, noMatch}));
    sess.addProvider(makeProvider("p2", {prefix}));

    sess.open();
    NF::QuickOpenQuery q;
    q.text = "scene";
    size_t count = sess.query(q);

    // exact + prefix match; noMatch excluded
    REQUIRE(count == 2);
    REQUIRE(sess.results()[0].id == "e"); // exact score wins
    REQUIRE(sess.results()[1].id == "p");
}

TEST_CASE("QuickOpen integration: empty query returns all items", "[quickopen][integration]") {
    NF::QuickOpenSession sess("s");
    std::vector<NF::QuickOpenItem> items;
    for (int i = 0; i < 5; ++i) {
        NF::QuickOpenItem it;
        it.id    = std::to_string(i);
        it.label = "Item " + std::to_string(i);
        items.push_back(it);
    }
    sess.addProvider(makeProvider("p", items));
    sess.open();
    NF::QuickOpenQuery q; // empty text
    REQUIRE(sess.query(q) == 5);
}
