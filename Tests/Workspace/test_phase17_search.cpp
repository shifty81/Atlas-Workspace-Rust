// Tests/Workspace/test_phase17_search.cpp
// Phase 17 — Workspace Search and Indexing
//
// Tests for:
//   1. SearchScope — enum name helpers
//   2. SearchResultType — enum name helpers
//   3. SearchMatchKind — enum name helpers
//   4. SearchQuery — typed query with filters and scope
//   5. SearchResult — ranked result with match context
//   6. SearchIndex — in-memory content index
//   7. SearchEngine — cross-index search
//   8. Integration — full search pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceSearch.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — Enums
// ═════════════════════════════════════════════════════════════════

TEST_CASE("searchScopeName returns correct strings", "[Phase17][Scope]") {
    CHECK(std::string(searchScopeName(SearchScope::All))      == "All");
    CHECK(std::string(searchScopeName(SearchScope::Project))  == "Project");
    CHECK(std::string(searchScopeName(SearchScope::Assets))   == "Assets");
    CHECK(std::string(searchScopeName(SearchScope::Tools))    == "Tools");
    CHECK(std::string(searchScopeName(SearchScope::Panels))   == "Panels");
    CHECK(std::string(searchScopeName(SearchScope::Commands)) == "Commands");
    CHECK(std::string(searchScopeName(SearchScope::Settings)) == "Settings");
    CHECK(std::string(searchScopeName(SearchScope::Plugins))  == "Plugins");
    CHECK(std::string(searchScopeName(SearchScope::Scripts))  == "Scripts");
    CHECK(std::string(searchScopeName(SearchScope::Custom))   == "Custom");
}

TEST_CASE("searchResultTypeName returns correct strings", "[Phase17][ResultType]") {
    CHECK(std::string(searchResultTypeName(SearchResultType::File))    == "File");
    CHECK(std::string(searchResultTypeName(SearchResultType::Asset))   == "Asset");
    CHECK(std::string(searchResultTypeName(SearchResultType::Tool))    == "Tool");
    CHECK(std::string(searchResultTypeName(SearchResultType::Panel))   == "Panel");
    CHECK(std::string(searchResultTypeName(SearchResultType::Command)) == "Command");
    CHECK(std::string(searchResultTypeName(SearchResultType::Setting)) == "Setting");
    CHECK(std::string(searchResultTypeName(SearchResultType::Plugin))  == "Plugin");
    CHECK(std::string(searchResultTypeName(SearchResultType::Script))  == "Script");
    CHECK(std::string(searchResultTypeName(SearchResultType::Text))    == "Text");
    CHECK(std::string(searchResultTypeName(SearchResultType::Symbol))  == "Symbol");
    CHECK(std::string(searchResultTypeName(SearchResultType::Custom))  == "Custom");
}

TEST_CASE("searchMatchKindName returns correct strings", "[Phase17][MatchKind]") {
    CHECK(std::string(searchMatchKindName(SearchMatchKind::Exact))    == "Exact");
    CHECK(std::string(searchMatchKindName(SearchMatchKind::Prefix))   == "Prefix");
    CHECK(std::string(searchMatchKindName(SearchMatchKind::Contains)) == "Contains");
    CHECK(std::string(searchMatchKindName(SearchMatchKind::Fuzzy))    == "Fuzzy");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — SearchQuery
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SearchQuery default is invalid", "[Phase17][Query]") {
    SearchQuery q;
    CHECK_FALSE(q.isValid());
    CHECK(q.scope() == SearchScope::All);
    CHECK_FALSE(q.caseSensitive());
    CHECK(q.maxResults() == 100);
}

TEST_CASE("SearchQuery valid construction", "[Phase17][Query]") {
    SearchQuery q("hello", SearchScope::Assets);
    CHECK(q.isValid());
    CHECK(q.text() == "hello");
    CHECK(q.scope() == SearchScope::Assets);
}

TEST_CASE("SearchQuery case sensitivity", "[Phase17][Query]") {
    SearchQuery q("test");
    CHECK_FALSE(q.caseSensitive());
    q.setCaseSensitive(true);
    CHECK(q.caseSensitive());
}

TEST_CASE("SearchQuery maxResults", "[Phase17][Query]") {
    SearchQuery q("test");
    q.setMaxResults(10);
    CHECK(q.maxResults() == 10);
}

TEST_CASE("SearchQuery type filters", "[Phase17][Query]") {
    SearchQuery q("test");
    CHECK_FALSE(q.hasTypeFilters());

    q.addTypeFilter(SearchResultType::File);
    q.addTypeFilter(SearchResultType::Asset);
    CHECK(q.hasTypeFilters());
    CHECK(q.hasTypeFilter(SearchResultType::File));
    CHECK(q.hasTypeFilter(SearchResultType::Asset));
    CHECK_FALSE(q.hasTypeFilter(SearchResultType::Tool));
    CHECK(q.typeFilters().size() == 2);

    // Duplicate rejection
    q.addTypeFilter(SearchResultType::File);
    CHECK(q.typeFilters().size() == 2);

    q.clearTypeFilters();
    CHECK_FALSE(q.hasTypeFilters());
}

TEST_CASE("SearchQuery source filter", "[Phase17][Query]") {
    SearchQuery q("test");
    CHECK(q.sourceFilter().empty());
    q.setSourceFilter("assets");
    CHECK(q.sourceFilter() == "assets");
}

TEST_CASE("SearchQuery equality", "[Phase17][Query]") {
    SearchQuery a("hello", SearchScope::Assets);
    SearchQuery b("hello", SearchScope::Assets);
    SearchQuery c("world", SearchScope::Assets);
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — SearchResult
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SearchResult default is invalid", "[Phase17][Result]") {
    SearchResult r;
    CHECK_FALSE(r.isValid());
    CHECK(r.type == SearchResultType::Text);
    CHECK(r.matchKind == SearchMatchKind::Contains);
    CHECK(r.score == 0.0f);
}

TEST_CASE("SearchResult valid construction", "[Phase17][Result]") {
    SearchResult r;
    r.id = "r1";
    r.title = "Test Result";
    r.description = "A test";
    r.source = "idx1";
    r.type = SearchResultType::File;
    r.matchKind = SearchMatchKind::Exact;
    r.score = 95.0f;

    CHECK(r.isValid());
    CHECK(r.id == "r1");
    CHECK(r.title == "Test Result");
}

TEST_CASE("SearchResult sorting by score descending", "[Phase17][Result]") {
    SearchResult a, b;
    a.id = "a"; a.title = "A"; a.score = 80.0f;
    b.id = "b"; b.title = "B"; b.score = 90.0f;

    CHECK(b < a); // Higher score sorts first
}

TEST_CASE("SearchResult equality by id and source", "[Phase17][Result]") {
    SearchResult a, b;
    a.id = "r1"; a.title = "A"; a.source = "idx1"; a.score = 80.0f;
    b.id = "r1"; b.title = "B"; b.source = "idx1"; b.score = 90.0f;
    CHECK(a == b); // Same id and source

    b.source = "idx2";
    CHECK(a != b);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — SearchIndex
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SearchIndex default state", "[Phase17][Index]") {
    SearchIndex idx("assets", SearchScope::Assets);
    CHECK(idx.isValid());
    CHECK(idx.name() == "assets");
    CHECK(idx.scope() == SearchScope::Assets);
    CHECK(idx.entryCount() == 0);
    CHECK(idx.isEmpty());
}

TEST_CASE("SearchIndex unnamed is invalid", "[Phase17][Index]") {
    SearchIndex idx;
    CHECK_FALSE(idx.isValid());
}

TEST_CASE("SearchIndex addEntry and findEntry", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "Entry One"; e.content = "Some searchable content";
    e.type = SearchResultType::File;

    CHECK(idx.addEntry(e));
    CHECK(idx.entryCount() == 1);
    CHECK_FALSE(idx.isEmpty());

    auto* found = idx.findEntry("e1");
    REQUIRE(found);
    CHECK(found->title == "Entry One");
    CHECK(found->type == SearchResultType::File);

    CHECK(idx.findEntry("missing") == nullptr);
}

TEST_CASE("SearchIndex rejects duplicate entry", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "Entry";
    CHECK(idx.addEntry(e));
    CHECK_FALSE(idx.addEntry(e));
    CHECK(idx.entryCount() == 1);
}

TEST_CASE("SearchIndex rejects invalid entry", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e; // empty id
    CHECK_FALSE(idx.addEntry(e));
}

TEST_CASE("SearchIndex removeEntry", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "Entry";
    idx.addEntry(e);

    CHECK(idx.removeEntry("e1"));
    CHECK(idx.entryCount() == 0);
    CHECK_FALSE(idx.removeEntry("e1")); // already gone
}

TEST_CASE("SearchIndex updateEntry", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "Original"; e.content = "old";
    idx.addEntry(e);

    e.title = "Updated"; e.content = "new";
    CHECK(idx.updateEntry(e));

    auto* found = idx.findEntry("e1");
    REQUIRE(found);
    CHECK(found->title == "Updated");
    CHECK(found->content == "new");

    SearchIndex::Entry missing;
    missing.id = "nope"; missing.title = "X";
    CHECK_FALSE(idx.updateEntry(missing));
}

TEST_CASE("SearchIndex entries() returns all", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e1; e1.id = "a"; e1.title = "A";
    SearchIndex::Entry e2; e2.id = "b"; e2.title = "B";
    idx.addEntry(e1);
    idx.addEntry(e2);

    auto& all = idx.entries();
    REQUIRE(all.size() == 2);
    CHECK(all[0].id == "a");
    CHECK(all[1].id == "b");
}

TEST_CASE("SearchIndex query exact title match", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "PlayerController"; e.content = "Handles player input";
    idx.addEntry(e);

    SearchQuery q("PlayerController");
    auto results = idx.query(q);
    REQUIRE(results.size() == 1);
    CHECK(results[0].matchKind == SearchMatchKind::Exact);
    CHECK(results[0].score == 100.0f);
    CHECK(results[0].id == "e1");
}

TEST_CASE("SearchIndex query prefix match", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "PlayerController"; e.content = "";
    idx.addEntry(e);

    SearchQuery q("Player");
    auto results = idx.query(q);
    REQUIRE(results.size() == 1);
    CHECK(results[0].matchKind == SearchMatchKind::Prefix);
    CHECK(results[0].score == 80.0f);
}

TEST_CASE("SearchIndex query title contains match", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "MyPlayerController"; e.content = "";
    idx.addEntry(e);

    SearchQuery q("Player");
    auto results = idx.query(q);
    REQUIRE(results.size() == 1);
    CHECK(results[0].matchKind == SearchMatchKind::Contains);
    CHECK(results[0].score == 60.0f);
}

TEST_CASE("SearchIndex query content match", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "Controller"; e.content = "Handles player input and movement";
    idx.addEntry(e);

    SearchQuery q("movement");
    auto results = idx.query(q);
    REQUIRE(results.size() == 1);
    CHECK(results[0].matchKind == SearchMatchKind::Contains);
    CHECK(results[0].score == 40.0f);
    CHECK_FALSE(results[0].context.empty());
}

TEST_CASE("SearchIndex query case insensitive by default", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "PlayerController"; e.content = "";
    idx.addEntry(e);

    SearchQuery q("playercontroller");
    auto results = idx.query(q);
    REQUIRE(results.size() == 1);
    CHECK(results[0].matchKind == SearchMatchKind::Exact);
}

TEST_CASE("SearchIndex query case sensitive", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "PlayerController"; e.content = "";
    idx.addEntry(e);

    SearchQuery q("playercontroller");
    q.setCaseSensitive(true);
    auto results = idx.query(q);
    CHECK(results.empty()); // No match in case-sensitive mode
}

TEST_CASE("SearchIndex query no match", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "Enemy"; e.content = "An enemy object";
    idx.addEntry(e);

    SearchQuery q("Wizard");
    auto results = idx.query(q);
    CHECK(results.empty());
}

TEST_CASE("SearchIndex query type filter", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e1; e1.id = "f1"; e1.title = "PlayerMesh"; e1.type = SearchResultType::Asset;
    SearchIndex::Entry e2; e2.id = "f2"; e2.title = "PlayerScript"; e2.type = SearchResultType::Script;
    idx.addEntry(e1);
    idx.addEntry(e2);

    SearchQuery q("Player");
    q.addTypeFilter(SearchResultType::Asset);
    auto results = idx.query(q);
    REQUIRE(results.size() == 1);
    CHECK(results[0].id == "f1");
}

TEST_CASE("SearchIndex query invalid returns empty", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchQuery q; // invalid (no text)
    auto results = idx.query(q);
    CHECK(results.empty());
}

TEST_CASE("SearchIndex query maxResults", "[Phase17][Index]") {
    SearchIndex idx("test");
    for (int i = 0; i < 10; ++i) {
        SearchIndex::Entry e;
        e.id = "e" + std::to_string(i);
        e.title = "Item" + std::to_string(i);
        idx.addEntry(e);
    }

    SearchQuery q("Item");
    q.setMaxResults(3);
    auto results = idx.query(q);
    CHECK(results.size() == 3);
}

TEST_CASE("SearchIndex query results sorted by score", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e1; e1.id = "e1"; e1.title = "PlayerManager"; e1.content = "";
    SearchIndex::Entry e2; e2.id = "e2"; e2.title = "Player"; e2.content = "";
    SearchIndex::Entry e3; e3.id = "e3"; e3.title = "Controller"; e3.content = "Player movement";
    idx.addEntry(e1);
    idx.addEntry(e2);
    idx.addEntry(e3);

    SearchQuery q("Player");
    auto results = idx.query(q);
    REQUIRE(results.size() == 3);
    // Exact match first, then prefix, then contains
    CHECK(results[0].id == "e2"); // exact
    CHECK(results[1].id == "e1"); // prefix
    CHECK(results[2].id == "e3"); // content match
    CHECK(results[0].score >= results[1].score);
    CHECK(results[1].score >= results[2].score);
}

TEST_CASE("SearchIndex clear", "[Phase17][Index]") {
    SearchIndex idx("test");
    SearchIndex::Entry e;
    e.id = "e1"; e.title = "Entry";
    idx.addEntry(e);

    idx.clear();
    CHECK(idx.entryCount() == 0);
    CHECK(idx.isEmpty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — SearchEngine
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SearchEngine starts empty", "[Phase17][Engine]") {
    SearchEngine eng;
    CHECK(eng.indexCount() == 0);
    CHECK(eng.totalSearches() == 0);
    CHECK(eng.totalResults() == 0);
    CHECK(eng.totalEntries() == 0);
}

TEST_CASE("SearchEngine register and find index", "[Phase17][Engine]") {
    SearchEngine eng;
    SearchIndex idx("assets", SearchScope::Assets);

    CHECK(eng.registerIndex(idx));
    CHECK(eng.isRegistered("assets"));
    CHECK(eng.indexCount() == 1);

    auto* found = eng.findIndex("assets");
    REQUIRE(found);
    CHECK(found->scope() == SearchScope::Assets);
}

TEST_CASE("SearchEngine rejects duplicate index", "[Phase17][Engine]") {
    SearchEngine eng;
    SearchIndex idx("assets");
    CHECK(eng.registerIndex(idx));
    CHECK_FALSE(eng.registerIndex(idx));
    CHECK(eng.indexCount() == 1);
}

TEST_CASE("SearchEngine rejects invalid index", "[Phase17][Engine]") {
    SearchEngine eng;
    SearchIndex idx; // no name
    CHECK_FALSE(eng.registerIndex(idx));
}

TEST_CASE("SearchEngine unregister index", "[Phase17][Engine]") {
    SearchEngine eng;
    SearchIndex idx("assets");
    eng.registerIndex(idx);

    CHECK(eng.unregisterIndex("assets"));
    CHECK_FALSE(eng.isRegistered("assets"));
    CHECK(eng.indexCount() == 0);
    CHECK_FALSE(eng.unregisterIndex("assets")); // already gone
}

TEST_CASE("SearchEngine search across indices", "[Phase17][Engine]") {
    SearchEngine eng;

    SearchIndex assets("assets", SearchScope::Assets);
    SearchIndex::Entry ae; ae.id = "a1"; ae.title = "PlayerMesh"; ae.type = SearchResultType::Asset;
    assets.addEntry(ae);
    eng.registerIndex(assets);

    SearchIndex tools("tools", SearchScope::Tools);
    SearchIndex::Entry te; te.id = "t1"; te.title = "PlayerInspector"; te.type = SearchResultType::Tool;
    tools.addEntry(te);
    eng.registerIndex(tools);

    SearchQuery q("Player");
    auto results = eng.search(q);
    CHECK(results.size() == 2);
    CHECK(eng.totalSearches() == 1);
    CHECK(eng.totalResults() == 2);
}

TEST_CASE("SearchEngine search with scope filter", "[Phase17][Engine]") {
    SearchEngine eng;

    SearchIndex assets("assets", SearchScope::Assets);
    SearchIndex::Entry ae; ae.id = "a1"; ae.title = "PlayerMesh";
    assets.addEntry(ae);
    eng.registerIndex(assets);

    SearchIndex tools("tools", SearchScope::Tools);
    SearchIndex::Entry te; te.id = "t1"; te.title = "PlayerInspector";
    tools.addEntry(te);
    eng.registerIndex(tools);

    SearchQuery q("Player", SearchScope::Assets);
    auto results = eng.search(q);
    REQUIRE(results.size() == 1);
    CHECK(results[0].id == "a1");
}

TEST_CASE("SearchEngine search with source filter", "[Phase17][Engine]") {
    SearchEngine eng;

    SearchIndex idx1("index_a");
    SearchIndex::Entry e1; e1.id = "e1"; e1.title = "Widget";
    idx1.addEntry(e1);
    eng.registerIndex(idx1);

    SearchIndex idx2("index_b");
    SearchIndex::Entry e2; e2.id = "e2"; e2.title = "Widget";
    idx2.addEntry(e2);
    eng.registerIndex(idx2);

    SearchQuery q("Widget");
    q.setSourceFilter("index_a");
    auto results = eng.search(q);
    REQUIRE(results.size() == 1);
    CHECK(results[0].source == "index_a");
}

TEST_CASE("SearchEngine search invalid query returns empty", "[Phase17][Engine]") {
    SearchEngine eng;
    SearchQuery q; // invalid
    auto results = eng.search(q);
    CHECK(results.empty());
}

TEST_CASE("SearchEngine search enforces maxResults across indices", "[Phase17][Engine]") {
    SearchEngine eng;

    for (int i = 0; i < 5; ++i) {
        SearchIndex idx("idx" + std::to_string(i));
        for (int j = 0; j < 5; ++j) {
            SearchIndex::Entry e;
            e.id = "e" + std::to_string(i) + "_" + std::to_string(j);
            e.title = "Item" + std::to_string(j);
            idx.addEntry(e);
        }
        eng.registerIndex(idx);
    }

    SearchQuery q("Item");
    q.setMaxResults(7);
    auto results = eng.search(q);
    CHECK(results.size() == 7);
}

TEST_CASE("SearchEngine totalEntries across indices", "[Phase17][Engine]") {
    SearchEngine eng;

    SearchIndex idx1("a");
    SearchIndex::Entry e1; e1.id = "1"; e1.title = "A";
    idx1.addEntry(e1);
    eng.registerIndex(idx1);

    SearchIndex idx2("b");
    SearchIndex::Entry e2; e2.id = "2"; e2.title = "B";
    SearchIndex::Entry e3; e3.id = "3"; e3.title = "C";
    idx2.addEntry(e2);
    idx2.addEntry(e3);
    eng.registerIndex(idx2);

    CHECK(eng.totalEntries() == 3);
}

TEST_CASE("SearchEngine allIndices returns all", "[Phase17][Engine]") {
    SearchEngine eng;
    SearchIndex idx1("a");
    SearchIndex idx2("b");
    eng.registerIndex(idx1);
    eng.registerIndex(idx2);

    auto& all = eng.allIndices();
    REQUIRE(all.size() == 2);
    CHECK(all[0].name() == "a");
    CHECK(all[1].name() == "b");
}

TEST_CASE("SearchEngine clear resets everything", "[Phase17][Engine]") {
    SearchEngine eng;
    SearchIndex idx("test");
    SearchIndex::Entry e; e.id = "1"; e.title = "X";
    idx.addEntry(e);
    eng.registerIndex(idx);

    SearchQuery q("X");
    eng.search(q);

    eng.clear();
    CHECK(eng.indexCount() == 0);
    CHECK(eng.totalSearches() == 0);
    CHECK(eng.totalResults() == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-index search with ranking", "[Phase17][Integration]") {
    SearchEngine eng;

    // Create asset index with player entries
    SearchIndex assets("assets", SearchScope::Assets);
    SearchIndex::Entry ae1; ae1.id = "mesh_player"; ae1.title = "Player"; ae1.type = SearchResultType::Asset;
    SearchIndex::Entry ae2; ae2.id = "mesh_enemy"; ae2.title = "PlayerEnemy"; ae2.type = SearchResultType::Asset;
    assets.addEntry(ae1);
    assets.addEntry(ae2);
    eng.registerIndex(assets);

    // Create command index
    SearchIndex cmds("commands", SearchScope::Commands);
    SearchIndex::Entry ce1; ce1.id = "cmd_spawn"; ce1.title = "SpawnPlayer"; ce1.type = SearchResultType::Command;
    cmds.addEntry(ce1);
    eng.registerIndex(cmds);

    SearchQuery q("Player");
    auto results = eng.search(q);
    REQUIRE(results.size() == 3);

    // Exact match should be first (score 100)
    CHECK(results[0].id == "mesh_player");
    CHECK(results[0].matchKind == SearchMatchKind::Exact);
    // Prefix matches next
    CHECK(results[1].matchKind == SearchMatchKind::Prefix);
}

TEST_CASE("Integration: add entries to registered index then search", "[Phase17][Integration]") {
    SearchEngine eng;
    SearchIndex idx("tools", SearchScope::Tools);
    eng.registerIndex(idx);

    // Add entries after registration via findIndex
    auto* tools = eng.findIndex("tools");
    REQUIRE(tools);

    SearchIndex::Entry e1; e1.id = "t1"; e1.title = "Inspector"; e1.type = SearchResultType::Tool;
    SearchIndex::Entry e2; e2.id = "t2"; e2.title = "Profiler"; e2.type = SearchResultType::Tool;
    SearchIndex::Entry e3; e3.id = "t3"; e3.title = "Debugger"; e3.type = SearchResultType::Tool;
    tools->addEntry(e1);
    tools->addEntry(e2);
    tools->addEntry(e3);

    CHECK(eng.totalEntries() == 3);

    SearchQuery q("Profiler");
    auto results = eng.search(q);
    REQUIRE(results.size() == 1);
    CHECK(results[0].title == "Profiler");
    CHECK(results[0].matchKind == SearchMatchKind::Exact);
}

TEST_CASE("Integration: scope-filtered search across mixed indices", "[Phase17][Integration]") {
    SearchEngine eng;

    SearchIndex assets("assets", SearchScope::Assets);
    SearchIndex::Entry ae; ae.id = "a1"; ae.title = "Texture_Wood"; ae.type = SearchResultType::Asset;
    assets.addEntry(ae);
    eng.registerIndex(assets);

    SearchIndex cmds("commands", SearchScope::Commands);
    SearchIndex::Entry ce; ce.id = "c1"; ce.title = "Apply_Wood"; ce.type = SearchResultType::Command;
    cmds.addEntry(ce);
    eng.registerIndex(cmds);

    SearchIndex settings("settings", SearchScope::Settings);
    SearchIndex::Entry se; se.id = "s1"; se.title = "Wood_Quality"; se.type = SearchResultType::Setting;
    settings.addEntry(se);
    eng.registerIndex(settings);

    // All scope — finds all 3
    SearchQuery qAll("Wood");
    CHECK(eng.search(qAll).size() == 3);

    // Assets scope — finds only asset
    SearchQuery qAssets("Wood", SearchScope::Assets);
    auto assetResults = eng.search(qAssets);
    REQUIRE(assetResults.size() == 1);
    CHECK(assetResults[0].id == "a1");

    // Commands scope — finds only command
    SearchQuery qCmds("Wood", SearchScope::Commands);
    auto cmdResults = eng.search(qCmds);
    REQUIRE(cmdResults.size() == 1);
    CHECK(cmdResults[0].id == "c1");
}

TEST_CASE("Integration: search statistics accumulate", "[Phase17][Integration]") {
    SearchEngine eng;

    SearchIndex idx("data");
    for (int i = 0; i < 5; ++i) {
        SearchIndex::Entry e;
        e.id = "e" + std::to_string(i);
        e.title = "Widget" + std::to_string(i);
        idx.addEntry(e);
    }
    eng.registerIndex(idx);

    SearchQuery q1("Widget");
    q1.setMaxResults(3);
    eng.search(q1); // 3 results

    SearchQuery q2("Widget0");
    eng.search(q2); // 1 result

    CHECK(eng.totalSearches() == 2);
    CHECK(eng.totalResults() == 4); // 3 + 1
}
