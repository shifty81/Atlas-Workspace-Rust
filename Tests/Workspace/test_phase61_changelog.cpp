// Tests/Workspace/test_phase61_changelog.cpp
// Phase 61 — WorkspaceChangelog: ChangeCategory, ChangeSeverity,
//             ChangeEntry, ChangeVersion, Changelog
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceChangelog.h"

// ═════════════════════════════════════════════════════════════════
// ChangeCategory / ChangeSeverity
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ChangeCategory: name helpers", "[changelog][category]") {
    REQUIRE(std::string(NF::changeCategoryName(NF::ChangeCategory::Feature))    == "Feature");
    REQUIRE(std::string(NF::changeCategoryName(NF::ChangeCategory::Bugfix))     == "Bugfix");
    REQUIRE(std::string(NF::changeCategoryName(NF::ChangeCategory::Breaking))   == "Breaking");
    REQUIRE(std::string(NF::changeCategoryName(NF::ChangeCategory::Deprecated)) == "Deprecated");
    REQUIRE(std::string(NF::changeCategoryName(NF::ChangeCategory::Internal))   == "Internal");
    REQUIRE(std::string(NF::changeCategoryName(NF::ChangeCategory::Security))   == "Security");
}

TEST_CASE("ChangeSeverity: name helpers", "[changelog][severity]") {
    REQUIRE(std::string(NF::changeSeverityName(NF::ChangeSeverity::Patch))    == "Patch");
    REQUIRE(std::string(NF::changeSeverityName(NF::ChangeSeverity::Minor))    == "Minor");
    REQUIRE(std::string(NF::changeSeverityName(NF::ChangeSeverity::Major))    == "Major");
    REQUIRE(std::string(NF::changeSeverityName(NF::ChangeSeverity::Critical)) == "Critical");
}

// ═════════════════════════════════════════════════════════════════
// ChangeEntry
// ═════════════════════════════════════════════════════════════════

static NF::ChangeEntry makeEntry(const std::string& id, const std::string& summary,
                                  NF::ChangeCategory cat = NF::ChangeCategory::Feature,
                                  NF::ChangeSeverity sev = NF::ChangeSeverity::Minor) {
    NF::ChangeEntry e;
    e.id          = id;
    e.summary     = summary;
    e.category    = cat;
    e.severity    = sev;
    e.version     = "1.0.0";
    e.timestampMs = 1000;
    return e;
}

TEST_CASE("ChangeEntry: default invalid", "[changelog][entry]") {
    NF::ChangeEntry e;
    REQUIRE_FALSE(e.isValid());
}

TEST_CASE("ChangeEntry: valid with id and summary", "[changelog][entry]") {
    auto e = makeEntry("e1", "Added new feature");
    REQUIRE(e.isValid());
}

TEST_CASE("ChangeEntry: invalid without summary", "[changelog][entry]") {
    NF::ChangeEntry e;
    e.id = "x";
    REQUIRE_FALSE(e.isValid());
}

TEST_CASE("ChangeEntry: equality by id", "[changelog][entry]") {
    auto a = makeEntry("a", "Summary A");
    auto b = makeEntry("a", "Summary B");
    auto c = makeEntry("c", "Summary C");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ═════════════════════════════════════════════════════════════════
// ChangeVersion
// ═════════════════════════════════════════════════════════════════

static NF::ChangeVersion makeVersion(const std::string& ver,
                                      const std::string& date = "2026-01-01") {
    NF::ChangeVersion v;
    v.version     = ver;
    v.releaseDate = date;
    return v;
}

TEST_CASE("ChangeVersion: default empty", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    REQUIRE(v.isValid());
    REQUIRE(v.empty());
    REQUIRE(v.count() == 0);
}

TEST_CASE("ChangeVersion: invalid without version string", "[changelog][version]") {
    NF::ChangeVersion v;
    REQUIRE_FALSE(v.isValid());
}

TEST_CASE("ChangeVersion: addEntry", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    REQUIRE(v.addEntry(makeEntry("e1", "Feature added")));
    REQUIRE(v.count() == 1);
    REQUIRE_FALSE(v.empty());
}

TEST_CASE("ChangeVersion: addEntry rejects invalid", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    NF::ChangeEntry e;
    REQUIRE_FALSE(v.addEntry(e));
}

TEST_CASE("ChangeVersion: addEntry rejects duplicate", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    v.addEntry(makeEntry("e1", "Feature"));
    REQUIRE_FALSE(v.addEntry(makeEntry("e1", "Feature2")));
}

TEST_CASE("ChangeVersion: removeEntry", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    v.addEntry(makeEntry("e1", "Feature"));
    REQUIRE(v.removeEntry("e1"));
    REQUIRE(v.empty());
}

TEST_CASE("ChangeVersion: removeEntry unknown", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    REQUIRE_FALSE(v.removeEntry("missing"));
}

TEST_CASE("ChangeVersion: findEntry", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    v.addEntry(makeEntry("e1", "Feature", NF::ChangeCategory::Bugfix));
    const auto* e = v.findEntry("e1");
    REQUIRE(e != nullptr);
    REQUIRE(e->category == NF::ChangeCategory::Bugfix);
}

TEST_CASE("ChangeVersion: filterByCategory", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    v.addEntry(makeEntry("e1", "Bug fix", NF::ChangeCategory::Bugfix));
    v.addEntry(makeEntry("e2", "New feature", NF::ChangeCategory::Feature));
    v.addEntry(makeEntry("e3", "Another bug", NF::ChangeCategory::Bugfix));
    auto bugs = v.filterByCategory(NF::ChangeCategory::Bugfix);
    REQUIRE(bugs.size() == 2);
}

TEST_CASE("ChangeVersion: filterBySeverity", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    v.addEntry(makeEntry("e1", "Patch fix", NF::ChangeCategory::Bugfix, NF::ChangeSeverity::Patch));
    v.addEntry(makeEntry("e2", "Major feature", NF::ChangeCategory::Feature, NF::ChangeSeverity::Major));
    v.addEntry(makeEntry("e3", "Minor tweak", NF::ChangeCategory::Internal, NF::ChangeSeverity::Patch));
    auto patches = v.filterBySeverity(NF::ChangeSeverity::Patch);
    REQUIRE(patches.size() == 2);
}

TEST_CASE("ChangeVersion: countByCategory", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    v.addEntry(makeEntry("e1", "f1", NF::ChangeCategory::Feature));
    v.addEntry(makeEntry("e2", "f2", NF::ChangeCategory::Feature));
    v.addEntry(makeEntry("e3", "b1", NF::ChangeCategory::Bugfix));
    REQUIRE(v.countByCategory(NF::ChangeCategory::Feature) == 2);
    REQUIRE(v.countByCategory(NF::ChangeCategory::Bugfix) == 1);
    REQUIRE(v.countByCategory(NF::ChangeCategory::Security) == 0);
}

TEST_CASE("ChangeVersion: clear", "[changelog][version]") {
    auto v = makeVersion("1.0.0");
    v.addEntry(makeEntry("e1", "A"));
    v.addEntry(makeEntry("e2", "B"));
    v.clear();
    REQUIRE(v.empty());
}

// ═════════════════════════════════════════════════════════════════
// Changelog
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Changelog: default empty", "[changelog][main]") {
    NF::Changelog cl;
    REQUIRE(cl.versionCount() == 0);
    REQUIRE(cl.totalEntries() == 0);
}

TEST_CASE("Changelog: addVersion", "[changelog][main]") {
    NF::Changelog cl;
    REQUIRE(cl.addVersion(makeVersion("1.0.0")));
    REQUIRE(cl.versionCount() == 1);
    REQUIRE(cl.hasVersion("1.0.0"));
}

TEST_CASE("Changelog: addVersion rejects invalid", "[changelog][main]") {
    NF::Changelog cl;
    NF::ChangeVersion v;
    REQUIRE_FALSE(cl.addVersion(v));
}

TEST_CASE("Changelog: addVersion rejects duplicate", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    REQUIRE_FALSE(cl.addVersion(makeVersion("1.0.0")));
}

TEST_CASE("Changelog: removeVersion", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    REQUIRE(cl.removeVersion("1.0.0"));
    REQUIRE(cl.versionCount() == 0);
}

TEST_CASE("Changelog: removeVersion unknown", "[changelog][main]") {
    NF::Changelog cl;
    REQUIRE_FALSE(cl.removeVersion("9.9.9"));
}

TEST_CASE("Changelog: addEntry shortcut", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    REQUIRE(cl.addEntry("1.0.0", makeEntry("e1", "Feature added")));
    REQUIRE(cl.totalEntries() == 1);
}

TEST_CASE("Changelog: addEntry unknown version", "[changelog][main]") {
    NF::Changelog cl;
    REQUIRE_FALSE(cl.addEntry("9.9.9", makeEntry("e1", "Feature")));
}

TEST_CASE("Changelog: searchByText", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    cl.addVersion(makeVersion("1.1.0"));
    cl.addEntry("1.0.0", makeEntry("e1", "Add new toolbar widget"));
    cl.addEntry("1.0.0", makeEntry("e2", "Fix crash on startup"));
    cl.addEntry("1.1.0", makeEntry("e3", "Improve toolbar performance"));

    auto results = cl.searchByText("toolbar");
    REQUIRE(results.size() == 2);
}

TEST_CASE("Changelog: searchByText case-insensitive", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    cl.addEntry("1.0.0", makeEntry("e1", "SECURITY fix for auth module"));

    auto results = cl.searchByText("security");
    REQUIRE(results.size() == 1);
}

TEST_CASE("Changelog: searchByText empty query", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    cl.addEntry("1.0.0", makeEntry("e1", "Something"));
    REQUIRE(cl.searchByText("").empty());
}

TEST_CASE("Changelog: filterByCategory", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    cl.addVersion(makeVersion("1.1.0"));
    cl.addEntry("1.0.0", makeEntry("e1", "Feature A", NF::ChangeCategory::Feature));
    cl.addEntry("1.0.0", makeEntry("e2", "Bug B", NF::ChangeCategory::Bugfix));
    cl.addEntry("1.1.0", makeEntry("e3", "Feature C", NF::ChangeCategory::Feature));

    auto features = cl.filterByCategory(NF::ChangeCategory::Feature);
    REQUIRE(features.size() == 2);
}

TEST_CASE("Changelog: filterBySeverity", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    cl.addEntry("1.0.0", makeEntry("e1", "Major update", NF::ChangeCategory::Feature, NF::ChangeSeverity::Major));
    cl.addEntry("1.0.0", makeEntry("e2", "Patch fix", NF::ChangeCategory::Bugfix, NF::ChangeSeverity::Patch));

    auto majors = cl.filterBySeverity(NF::ChangeSeverity::Major);
    REQUIRE(majors.size() == 1);
    REQUIRE(majors[0]->id == "e1");
}

TEST_CASE("Changelog: observer on addEntry", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));

    int callCount = 0;
    std::string lastId;
    cl.addObserver([&](const NF::ChangeEntry& e) {
        ++callCount;
        lastId = e.id;
    });

    cl.addEntry("1.0.0", makeEntry("e1", "New feature"));
    REQUIRE(callCount == 1);
    REQUIRE(lastId == "e1");
}

TEST_CASE("Changelog: clearObservers", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    int callCount = 0;
    cl.addObserver([&](const NF::ChangeEntry&) { ++callCount; });
    cl.clearObservers();
    cl.addEntry("1.0.0", makeEntry("e1", "Feature"));
    REQUIRE(callCount == 0);
}

TEST_CASE("Changelog: serialize empty", "[changelog][serial]") {
    NF::Changelog cl;
    REQUIRE(cl.serialize().empty());
}

TEST_CASE("Changelog: serialize round-trip", "[changelog][serial]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0", "2026-01-01"));
    cl.addEntry("1.0.0", makeEntry("e1", "New feature", NF::ChangeCategory::Feature, NF::ChangeSeverity::Minor));
    auto e2 = makeEntry("e2", "Critical bug fix", NF::ChangeCategory::Bugfix, NF::ChangeSeverity::Critical);
    e2.detail = "Fixed crash in renderer";
    cl.addEntry("1.0.0", e2);

    std::string data = cl.serialize();
    REQUIRE_FALSE(data.empty());

    NF::Changelog cl2;
    REQUIRE(cl2.deserialize(data));
    REQUIRE(cl2.versionCount() == 1);
    REQUIRE(cl2.totalEntries() == 2);

    const auto* ver = cl2.findVersion("1.0.0");
    REQUIRE(ver != nullptr);
    REQUIRE(ver->releaseDate == "2026-01-01");
    REQUIRE(ver->findEntry("e1")->category == NF::ChangeCategory::Feature);
    REQUIRE(ver->findEntry("e2")->severity == NF::ChangeSeverity::Critical);
    REQUIRE(ver->findEntry("e2")->detail == "Fixed crash in renderer");
}

TEST_CASE("Changelog: serialize pipe in summary", "[changelog][serial]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    cl.addEntry("1.0.0", makeEntry("e1", "Fix a|b|c issue"));

    std::string data = cl.serialize();
    NF::Changelog cl2;
    cl2.deserialize(data);
    REQUIRE(cl2.findVersion("1.0.0")->findEntry("e1")->summary == "Fix a|b|c issue");
}

TEST_CASE("Changelog: deserialize empty", "[changelog][serial]") {
    NF::Changelog cl;
    REQUIRE(cl.deserialize(""));
    REQUIRE(cl.versionCount() == 0);
}

TEST_CASE("Changelog: clear", "[changelog][main]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    cl.addEntry("1.0.0", makeEntry("e1", "Feature"));
    int calls = 0;
    cl.addObserver([&](const NF::ChangeEntry&) { ++calls; });
    cl.clear();
    REQUIRE(cl.versionCount() == 0);
    REQUIRE(cl.totalEntries() == 0);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-version changelog with search", "[changelog][integration]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0", "2025-01-01"));
    cl.addVersion(makeVersion("1.1.0", "2025-06-01"));
    cl.addVersion(makeVersion("2.0.0", "2026-01-01"));

    cl.addEntry("1.0.0", makeEntry("e1", "Initial release", NF::ChangeCategory::Feature, NF::ChangeSeverity::Major));
    cl.addEntry("1.1.0", makeEntry("e2", "Add toolbar widget", NF::ChangeCategory::Feature, NF::ChangeSeverity::Minor));
    cl.addEntry("1.1.0", makeEntry("e3", "Fix toolbar crash", NF::ChangeCategory::Bugfix, NF::ChangeSeverity::Patch));
    cl.addEntry("2.0.0", makeEntry("e4", "Breaking: new API", NF::ChangeCategory::Breaking, NF::ChangeSeverity::Major));
    cl.addEntry("2.0.0", makeEntry("e5", "Security patch", NF::ChangeCategory::Security, NF::ChangeSeverity::Critical));

    REQUIRE(cl.totalEntries() == 5);
    REQUIRE(cl.searchByText("toolbar").size() == 2);
    REQUIRE(cl.filterByCategory(NF::ChangeCategory::Feature).size() == 2);
    REQUIRE(cl.filterByCategory(NF::ChangeCategory::Bugfix).size() == 1);
    REQUIRE(cl.filterBySeverity(NF::ChangeSeverity::Major).size() == 2);
    REQUIRE(cl.filterBySeverity(NF::ChangeSeverity::Critical).size() == 1);
}

TEST_CASE("Integration: serialize/deserialize multi-version", "[changelog][integration]") {
    NF::Changelog cl;
    cl.addVersion(makeVersion("1.0.0"));
    cl.addVersion(makeVersion("2.0.0"));
    cl.addEntry("1.0.0", makeEntry("e1", "Feature one", NF::ChangeCategory::Feature));
    cl.addEntry("2.0.0", makeEntry("e2", "Breaking change|v2", NF::ChangeCategory::Breaking, NF::ChangeSeverity::Major));

    std::string data = cl.serialize();
    NF::Changelog cl2;
    cl2.deserialize(data);

    REQUIRE(cl2.versionCount() == 2);
    REQUIRE(cl2.totalEntries() == 2);
    REQUIRE(cl2.findVersion("2.0.0")->findEntry("e2")->summary == "Breaking change|v2");
    REQUIRE(cl2.findVersion("2.0.0")->findEntry("e2")->severity == NF::ChangeSeverity::Major);
}
