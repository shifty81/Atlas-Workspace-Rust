// Tests/Workspace/test_phase35_project_file.cpp
// Phase 35 — Workspace Project File
//
// Tests for:
//   1. ProjectFileVersion    — construction, toString, isCompatible, parse, current
//   2. ProjectFileSection    — set/get/getOr/has/remove/clear/count/empty
//   3. WorkspaceProjectFile  — identity setters, isValid, section management
//   4. Serialization         — serialize + parse round-trip
//   5. Integration           — full pipeline with sections and version checks

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceProjectFile.h"
#include <string>

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// 1. ProjectFileVersion
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ProjectFileVersion default is 1.0.0", "[ProjectFileVersion]") {
    ProjectFileVersion v;
    CHECK(v.major == 1u);
    CHECK(v.minor == 0u);
    CHECK(v.patch == 0u);
}

TEST_CASE("ProjectFileVersion toString formats major.minor.patch", "[ProjectFileVersion]") {
    ProjectFileVersion v;
    v.major = 2; v.minor = 3; v.patch = 4;
    CHECK(v.toString() == "2.3.4");
}

TEST_CASE("ProjectFileVersion isCompatible same version", "[ProjectFileVersion]") {
    ProjectFileVersion file = {1, 0, 0};
    ProjectFileVersion reader = {1, 0, 0};
    CHECK(file.isCompatible(reader));
}

TEST_CASE("ProjectFileVersion isCompatible file minor older than reader", "[ProjectFileVersion]") {
    ProjectFileVersion file   = {1, 0, 0};
    ProjectFileVersion reader = {1, 2, 0};
    CHECK(file.isCompatible(reader)); // reader 1.2 can read file 1.0
}

TEST_CASE("ProjectFileVersion incompatible when file minor is newer than reader", "[ProjectFileVersion]") {
    ProjectFileVersion file   = {1, 3, 0};
    ProjectFileVersion reader = {1, 0, 0};
    CHECK_FALSE(file.isCompatible(reader));
}

TEST_CASE("ProjectFileVersion incompatible when major version differs", "[ProjectFileVersion]") {
    ProjectFileVersion file   = {2, 0, 0};
    ProjectFileVersion reader = {1, 0, 0};
    CHECK_FALSE(file.isCompatible(reader));
}

TEST_CASE("ProjectFileVersion parse valid string", "[ProjectFileVersion]") {
    ProjectFileVersion v;
    CHECK(ProjectFileVersion::parse("3.1.7", v));
    CHECK(v.major == 3u);
    CHECK(v.minor == 1u);
    CHECK(v.patch == 7u);
}

TEST_CASE("ProjectFileVersion parse fails on invalid string", "[ProjectFileVersion]") {
    ProjectFileVersion v;
    CHECK_FALSE(ProjectFileVersion::parse("abc", v));
    CHECK_FALSE(ProjectFileVersion::parse("", v));
}

TEST_CASE("ProjectFileVersion current returns 1.0.0", "[ProjectFileVersion]") {
    auto c = ProjectFileVersion::current();
    CHECK(c.major == 1u);
    CHECK(c.minor == 0u);
    CHECK(c.patch == 0u);
}

TEST_CASE("ProjectFileVersion equality operator", "[ProjectFileVersion]") {
    ProjectFileVersion a = {1, 2, 3};
    ProjectFileVersion b = {1, 2, 3};
    ProjectFileVersion c = {1, 2, 4};
    CHECK(a == b);
    CHECK(a != c);
}

// ─────────────────────────────────────────────────────────────────
// 2. ProjectFileSection
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ProjectFileSection default is empty", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    CHECK(sec.empty());
    CHECK(sec.count() == 0u);
    CHECK(sec.name() == "Core");
}

TEST_CASE("ProjectFileSection set and get value", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    sec.set("key", "value");
    const std::string* v = sec.get("key");
    REQUIRE(v != nullptr);
    CHECK(*v == "value");
}

TEST_CASE("ProjectFileSection set overwrites existing key", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    sec.set("key", "first");
    sec.set("key", "second");
    CHECK(*sec.get("key") == "second");
    CHECK(sec.count() == 1u);
}

TEST_CASE("ProjectFileSection get returns nullptr for missing key", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    CHECK(sec.get("missing") == nullptr);
}

TEST_CASE("ProjectFileSection getOr returns default for missing key", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    CHECK(sec.getOr("missing", "fallback") == "fallback");
}

TEST_CASE("ProjectFileSection getOr returns value for existing key", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    sec.set("k", "v");
    CHECK(sec.getOr("k", "fallback") == "v");
}

TEST_CASE("ProjectFileSection has returns correct result", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    sec.set("k", "v");
    CHECK(sec.has("k"));
    CHECK_FALSE(sec.has("missing"));
}

TEST_CASE("ProjectFileSection remove existing key succeeds", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    sec.set("k", "v");
    CHECK(sec.remove("k"));
    CHECK_FALSE(sec.has("k"));
    CHECK(sec.empty());
}

TEST_CASE("ProjectFileSection remove missing key fails", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    CHECK_FALSE(sec.remove("nope"));
}

TEST_CASE("ProjectFileSection clear empties all entries", "[ProjectFileSection]") {
    ProjectFileSection sec("Core");
    sec.set("a", "1");
    sec.set("b", "2");
    sec.clear();
    CHECK(sec.empty());
    CHECK(sec.count() == 0u);
}

// ─────────────────────────────────────────────────────────────────
// 3. WorkspaceProjectFile
// ─────────────────────────────────────────────────────────────────
TEST_CASE("WorkspaceProjectFile default is invalid", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    CHECK_FALSE(pf.isValid());
}

TEST_CASE("WorkspaceProjectFile valid with id and name", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    pf.setProjectId("proj-001");
    pf.setProjectName("My Project");
    CHECK(pf.isValid());
}

TEST_CASE("WorkspaceProjectFile invalid without id", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    pf.setProjectName("My Project");
    CHECK_FALSE(pf.isValid());
}

TEST_CASE("WorkspaceProjectFile invalid without name", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    pf.setProjectId("proj-001");
    CHECK_FALSE(pf.isValid());
}

TEST_CASE("WorkspaceProjectFile setContentRoot stores path", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    pf.setContentRoot("/assets/project");
    CHECK(pf.contentRoot() == "/assets/project");
}

TEST_CASE("WorkspaceProjectFile section() creates section on demand", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    pf.section("Core").set("engine", "AtlasEngine");
    CHECK(pf.hasSection("Core"));
    CHECK(pf.sectionCount() == 1u);
}

TEST_CASE("WorkspaceProjectFile section() returns existing section", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    pf.section("Layout").set("width", "1920");
    pf.section("Layout").set("height", "1080");
    CHECK(pf.sectionCount() == 1u); // only one section
    CHECK(*pf.findSection("Layout")->get("width") == "1920");
}

TEST_CASE("WorkspaceProjectFile findSection returns nullptr for missing", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    CHECK(pf.findSection("Missing") == nullptr);
}

TEST_CASE("WorkspaceProjectFile removeSection removes correctly", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    pf.section("Temp").set("x", "1");
    CHECK(pf.hasSection("Temp"));
    CHECK(pf.removeSection("Temp"));
    CHECK_FALSE(pf.hasSection("Temp"));
}

TEST_CASE("WorkspaceProjectFile removeSection fails for unknown name", "[WorkspaceProjectFile]") {
    WorkspaceProjectFile pf;
    CHECK_FALSE(pf.removeSection("Ghost"));
}

// ─────────────────────────────────────────────────────────────────
// 4. Serialization
// ─────────────────────────────────────────────────────────────────
TEST_CASE("WorkspaceProjectFile serialize produces magic header", "[Serialization]") {
    WorkspaceProjectFile pf;
    pf.setProjectId("p1");
    pf.setProjectName("Test");
    std::string out = pf.serialize();
    CHECK(out.find("#atlasproject") != std::string::npos);
}

TEST_CASE("WorkspaceProjectFile serialize and parse round-trip", "[Serialization]") {
    WorkspaceProjectFile original;
    original.setProjectId("proj-42");
    original.setProjectName("Atlas Demo");
    original.setContentRoot("/Content/AtlasDemo");

    std::string data = original.serialize();

    WorkspaceProjectFile parsed;
    CHECK(WorkspaceProjectFile::parse(data, parsed));
    CHECK(parsed.projectId()   == "proj-42");
    CHECK(parsed.projectName() == "Atlas Demo");
    CHECK(parsed.contentRoot() == "/Content/AtlasDemo");
}

TEST_CASE("WorkspaceProjectFile serialize and parse with sections", "[Serialization]") {
    WorkspaceProjectFile original;
    original.setProjectId("proj-1");
    original.setProjectName("Project");
    original.section("Layout").set("theme", "dark");
    original.section("Layout").set("scale", "1.5");
    original.section("Build").set("target", "windows");

    std::string data = original.serialize();

    WorkspaceProjectFile parsed;
    REQUIRE(WorkspaceProjectFile::parse(data, parsed));
    CHECK(parsed.hasSection("Layout"));
    CHECK(parsed.hasSection("Build"));
    CHECK(*parsed.findSection("Layout")->get("theme") == "dark");
    CHECK(*parsed.findSection("Build")->get("target") == "windows");
}

TEST_CASE("WorkspaceProjectFile parse fails on missing magic", "[Serialization]") {
    std::string bad = "project.id=foo\nproject.name=bar\n";
    WorkspaceProjectFile out;
    CHECK_FALSE(WorkspaceProjectFile::parse(bad, out));
}

TEST_CASE("WorkspaceProjectFile parse fails on empty input", "[Serialization]") {
    WorkspaceProjectFile out;
    CHECK_FALSE(WorkspaceProjectFile::parse("", out));
}

// ─────────────────────────────────────────────────────────────────
// 5. Integration
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ProjectFile integration: full round-trip preserves all data", "[ProjectFileIntegration]") {
    WorkspaceProjectFile pf;
    pf.setProjectId("atlas-001");
    pf.setProjectName("Atlas Workspace Demo");
    pf.setContentRoot("Content/AtlasDemo");
    pf.setVersion({1, 2, 0});
    pf.section("Core").set("engine",   "AtlasEngine");
    pf.section("Core").set("renderer", "D3D11");
    pf.section("Settings").set("vsync", "true");

    std::string data = pf.serialize();
    WorkspaceProjectFile parsed;
    REQUIRE(WorkspaceProjectFile::parse(data, parsed));

    CHECK(parsed.projectId()   == "atlas-001");
    CHECK(parsed.projectName() == "Atlas Workspace Demo");
    CHECK(parsed.contentRoot() == "Content/AtlasDemo");
    CHECK(parsed.version().minor == 2u);
    CHECK(*parsed.findSection("Core")->get("engine")     == "AtlasEngine");
    CHECK(*parsed.findSection("Core")->get("renderer")   == "D3D11");
    CHECK(*parsed.findSection("Settings")->get("vsync")  == "true");
}

TEST_CASE("ProjectFile integration: version compatibility check", "[ProjectFileIntegration]") {
    WorkspaceProjectFile pf;
    pf.setProjectId("p");
    pf.setProjectName("N");
    pf.setVersion({1, 0, 0});

    std::string data = pf.serialize();
    WorkspaceProjectFile parsed;
    REQUIRE(WorkspaceProjectFile::parse(data, parsed));

    // A reader at 1.1.0 should be able to read a 1.0.0 file
    ProjectFileVersion reader = {1, 1, 0};
    CHECK(parsed.version().isCompatible(reader));

    // A reader at 2.0.0 should not be compatible with a 1.x file
    ProjectFileVersion reader2 = {2, 0, 0};
    CHECK_FALSE(parsed.version().isCompatible(reader2));
}

TEST_CASE("ProjectFile integration: multiple sections independent", "[ProjectFileIntegration]") {
    WorkspaceProjectFile pf;
    pf.setProjectId("p");
    pf.setProjectName("N");
    pf.section("A").set("x", "1");
    pf.section("B").set("x", "2");

    std::string data = pf.serialize();
    WorkspaceProjectFile parsed;
    REQUIRE(WorkspaceProjectFile::parse(data, parsed));

    CHECK(*parsed.findSection("A")->get("x") == "1");
    CHECK(*parsed.findSection("B")->get("x") == "2");
}
