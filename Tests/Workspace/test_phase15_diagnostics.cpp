// Tests/Workspace/test_phase15_diagnostics.cpp
// Phase 15 — Workspace Diagnostics and Telemetry
//
// Tests for:
//   1. DiagnosticSeverity / DiagnosticCategory — enum name helpers
//   2. DiagnosticEntry — structured diagnostic record
//   3. DiagnosticCollector — accumulate/query/acknowledge diagnostics
//   4. TelemetryEventType — enum name helpers
//   5. TelemetryEvent — typed telemetry event with properties
//   6. TelemetryCollector — session-scoped event accumulation
//   7. DiagnosticSnapshot / TelemetrySnapshot — point-in-time captures
//   8. Integration — diagnostics + telemetry working together

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceDiagnostics.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — Enums
// ═════════════════════════════════════════════════════════════════

TEST_CASE("diagnosticSeverityName returns correct strings", "[Phase15][Severity]") {
    CHECK(std::string(diagnosticSeverityName(DiagnosticSeverity::Info))    == "Info");
    CHECK(std::string(diagnosticSeverityName(DiagnosticSeverity::Warning)) == "Warning");
    CHECK(std::string(diagnosticSeverityName(DiagnosticSeverity::Error))   == "Error");
    CHECK(std::string(diagnosticSeverityName(DiagnosticSeverity::Fatal))   == "Fatal");
}

TEST_CASE("diagnosticCategoryName returns correct strings", "[Phase15][Category]") {
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::Build))       == "Build");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::Asset))       == "Asset");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::Plugin))      == "Plugin");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::Project))     == "Project");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::Tool))        == "Tool");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::Render))      == "Render");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::Performance)) == "Performance");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::IO))          == "IO");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::Network))     == "Network");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::System))      == "System");
    CHECK(std::string(diagnosticCategoryName(DiagnosticCategory::Custom))      == "Custom");
}

TEST_CASE("telemetryEventTypeName returns correct strings", "[Phase15][TelemetryType]") {
    CHECK(std::string(telemetryEventTypeName(TelemetryEventType::FeatureUsage)) == "FeatureUsage");
    CHECK(std::string(telemetryEventTypeName(TelemetryEventType::Performance))  == "Performance");
    CHECK(std::string(telemetryEventTypeName(TelemetryEventType::Error))        == "Error");
    CHECK(std::string(telemetryEventTypeName(TelemetryEventType::Navigation))   == "Navigation");
    CHECK(std::string(telemetryEventTypeName(TelemetryEventType::Session))      == "Session");
    CHECK(std::string(telemetryEventTypeName(TelemetryEventType::Command))      == "Command");
    CHECK(std::string(telemetryEventTypeName(TelemetryEventType::Asset))        == "Asset");
    CHECK(std::string(telemetryEventTypeName(TelemetryEventType::Plugin))       == "Plugin");
    CHECK(std::string(telemetryEventTypeName(TelemetryEventType::Custom))       == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — DiagnosticEntry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("DiagnosticEntry default is invalid", "[Phase15][Entry]") {
    DiagnosticEntry entry;
    CHECK_FALSE(entry.isValid());
    CHECK_FALSE(entry.isError());
    CHECK_FALSE(entry.acknowledged);
}

TEST_CASE("DiagnosticEntry valid construction", "[Phase15][Entry]") {
    DiagnosticEntry entry;
    entry.id = "BUILD_001";
    entry.category = DiagnosticCategory::Build;
    entry.severity = DiagnosticSeverity::Error;
    entry.source = "BuildPipeline";
    entry.message = "Compilation failed";
    entry.timestampMs = 1000;

    CHECK(entry.isValid());
    CHECK(entry.isError());
    CHECK(entry.id == "BUILD_001");
    CHECK(entry.source == "BuildPipeline");
}

TEST_CASE("DiagnosticEntry isError for Error and Fatal", "[Phase15][Entry]") {
    DiagnosticEntry e;
    e.id = "X"; e.source = "S"; e.message = "M";

    e.severity = DiagnosticSeverity::Info;
    CHECK_FALSE(e.isError());

    e.severity = DiagnosticSeverity::Warning;
    CHECK_FALSE(e.isError());

    e.severity = DiagnosticSeverity::Error;
    CHECK(e.isError());

    e.severity = DiagnosticSeverity::Fatal;
    CHECK(e.isError());
}

TEST_CASE("DiagnosticEntry equality", "[Phase15][Entry]") {
    DiagnosticEntry a;
    a.id = "A"; a.source = "S"; a.message = "M"; a.timestampMs = 100;
    DiagnosticEntry b = a;

    CHECK(a == b);
    b.timestampMs = 200;
    CHECK(a != b);
}

TEST_CASE("DiagnosticEntry requires id, source, and message", "[Phase15][Entry]") {
    DiagnosticEntry e;
    e.source = "S"; e.message = "M";
    CHECK_FALSE(e.isValid()); // missing id

    e.id = "X"; e.source = ""; e.message = "M";
    CHECK_FALSE(e.isValid()); // missing source

    e.id = "X"; e.source = "S"; e.message = "";
    CHECK_FALSE(e.isValid()); // missing message

    e.id = "X"; e.source = "S"; e.message = "M";
    CHECK(e.isValid());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — DiagnosticCollector
// ═════════════════════════════════════════════════════════════════

TEST_CASE("DiagnosticCollector starts empty", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    CHECK(dc.empty());
    CHECK(dc.count() == 0);
    CHECK_FALSE(dc.hasErrors());
    CHECK(dc.errorCount() == 0);
    CHECK(dc.unacknowledgedCount() == 0);
}

TEST_CASE("DiagnosticCollector submit and count", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    CHECK(dc.submitInfo("I1", "Sys", "All good"));
    CHECK(dc.submitWarning("W1", "Sys", "Watch out"));
    CHECK(dc.submitError("E1", "Sys", "Broken"));
    CHECK(dc.count() == 3);
    CHECK_FALSE(dc.empty());
}

TEST_CASE("DiagnosticCollector rejects invalid entry", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    DiagnosticEntry bad;
    CHECK_FALSE(dc.submit(bad));
    CHECK(dc.count() == 0);
}

TEST_CASE("DiagnosticCollector findById", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("FIND_ME", "Src", "Hello");
    auto* found = dc.findById("FIND_ME");
    REQUIRE(found);
    CHECK(found->message == "Hello");
    CHECK(dc.findById("NOPE") == nullptr);
}

TEST_CASE("DiagnosticCollector findByCategory", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("A1", "Src", "Info", DiagnosticCategory::Build);
    dc.submitWarning("A2", "Src", "Warn", DiagnosticCategory::Asset);
    dc.submitError("A3", "Src", "Err", DiagnosticCategory::Build);

    auto builds = dc.findByCategory(DiagnosticCategory::Build);
    CHECK(builds.size() == 2);
    auto assets = dc.findByCategory(DiagnosticCategory::Asset);
    CHECK(assets.size() == 1);
    auto plugins = dc.findByCategory(DiagnosticCategory::Plugin);
    CHECK(plugins.empty());
}

TEST_CASE("DiagnosticCollector findBySeverity", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("I1", "Src", "Info1");
    dc.submitInfo("I2", "Src", "Info2");
    dc.submitError("E1", "Src", "Error1");

    auto infos = dc.findBySeverity(DiagnosticSeverity::Info);
    CHECK(infos.size() == 2);
    auto errors = dc.findBySeverity(DiagnosticSeverity::Error);
    CHECK(errors.size() == 1);
}

TEST_CASE("DiagnosticCollector findBySource", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("A1", "BuildPipeline", "Msg1");
    dc.submitInfo("A2", "AssetPipeline", "Msg2");
    dc.submitInfo("A3", "BuildPipeline", "Msg3");

    auto builds = dc.findBySource("BuildPipeline");
    CHECK(builds.size() == 2);
}

TEST_CASE("DiagnosticCollector countBySeverity and countByCategory", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("I1", "Src", "Info1", DiagnosticCategory::Build);
    dc.submitWarning("W1", "Src", "Warn1", DiagnosticCategory::Asset);
    dc.submitError("E1", "Src", "Err1", DiagnosticCategory::Build);

    CHECK(dc.countBySeverity(DiagnosticSeverity::Info) == 1);
    CHECK(dc.countBySeverity(DiagnosticSeverity::Warning) == 1);
    CHECK(dc.countBySeverity(DiagnosticSeverity::Error) == 1);
    CHECK(dc.countByCategory(DiagnosticCategory::Build) == 2);
    CHECK(dc.countByCategory(DiagnosticCategory::Asset) == 1);
}

TEST_CASE("DiagnosticCollector hasErrors and errorCount", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("I1", "Src", "Info");
    CHECK_FALSE(dc.hasErrors());
    CHECK(dc.errorCount() == 0);

    dc.submitError("E1", "Src", "Error");
    CHECK(dc.hasErrors());
    CHECK(dc.errorCount() == 1);

    // Fatal counts as error too
    DiagnosticEntry fatal;
    fatal.id = "F1"; fatal.source = "Src"; fatal.message = "Fatal";
    fatal.severity = DiagnosticSeverity::Fatal;
    dc.submit(fatal);
    CHECK(dc.errorCount() == 2);
}

TEST_CASE("DiagnosticCollector acknowledge", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("A1", "Src", "Msg1");
    dc.submitError("A2", "Src", "Msg2");
    CHECK(dc.unacknowledgedCount() == 2);

    CHECK(dc.acknowledge("A1"));
    CHECK(dc.unacknowledgedCount() == 1);

    CHECK_FALSE(dc.acknowledge("NONEXISTENT"));
    CHECK(dc.unacknowledgedCount() == 1);
}

TEST_CASE("DiagnosticCollector acknowledgeAll", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("A1", "Src", "Msg1");
    dc.submitInfo("A2", "Src", "Msg2");
    dc.submitInfo("A3", "Src", "Msg3");
    CHECK(dc.unacknowledgedCount() == 3);

    dc.acknowledgeAll();
    CHECK(dc.unacknowledgedCount() == 0);
}

TEST_CASE("DiagnosticCollector observer notification", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    std::vector<std::string> observed;
    dc.addObserver([&](const DiagnosticEntry& e) { observed.push_back(e.id); });

    dc.submitInfo("O1", "Src", "Msg1");
    dc.submitError("O2", "Src", "Msg2");
    REQUIRE(observed.size() == 2);
    CHECK(observed[0] == "O1");
    CHECK(observed[1] == "O2");
}

TEST_CASE("DiagnosticCollector clearObservers", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    int callCount = 0;
    dc.addObserver([&](const DiagnosticEntry&) { ++callCount; });
    dc.submitInfo("X", "S", "M");
    CHECK(callCount == 1);

    dc.clearObservers();
    dc.submitInfo("Y", "S", "M");
    CHECK(callCount == 1); // No more calls
}

TEST_CASE("DiagnosticCollector clear removes all entries", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("A", "S", "M");
    dc.submitError("B", "S", "M");
    CHECK(dc.count() == 2);
    dc.clear();
    CHECK(dc.empty());
    CHECK(dc.count() == 0);
}

TEST_CASE("DiagnosticCollector all returns entries", "[Phase15][Collector]") {
    DiagnosticCollector dc;
    dc.submitInfo("X1", "S", "M1");
    dc.submitWarning("X2", "S", "M2");
    auto& all = dc.all();
    REQUIRE(all.size() == 2);
    CHECK(all[0].id == "X1");
    CHECK(all[1].id == "X2");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — TelemetryEvent
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TelemetryEvent default is invalid", "[Phase15][TelemetryEvent]") {
    TelemetryEvent ev;
    CHECK_FALSE(ev.isValid());
    CHECK(ev.propertyCount() == 0);
}

TEST_CASE("TelemetryEvent valid construction", "[Phase15][TelemetryEvent]") {
    TelemetryEvent ev;
    ev.name = "tool.activated";
    ev.type = TelemetryEventType::FeatureUsage;
    ev.source = "SceneEditor";
    ev.timestampMs = 500;
    ev.durationMs = 12.5;

    CHECK(ev.isValid());
    CHECK(ev.name == "tool.activated");
    CHECK(ev.durationMs == 12.5);
}

TEST_CASE("TelemetryEvent property bag", "[Phase15][TelemetryEvent]") {
    TelemetryEvent ev;
    ev.name = "test"; ev.source = "src";

    CHECK(ev.setProperty("key1", "val1"));
    CHECK(ev.setProperty("key2", "val2"));
    CHECK(ev.propertyCount() == 2);

    CHECK(ev.hasProperty("key1"));
    CHECK(ev.getProperty("key1") == "val1");
    CHECK_FALSE(ev.hasProperty("key3"));
    CHECK(ev.getProperty("key3").empty());
}

TEST_CASE("TelemetryEvent property overwrite", "[Phase15][TelemetryEvent]") {
    TelemetryEvent ev;
    ev.name = "test"; ev.source = "src";

    ev.setProperty("k", "v1");
    CHECK(ev.getProperty("k") == "v1");

    ev.setProperty("k", "v2");
    CHECK(ev.getProperty("k") == "v2");
    CHECK(ev.propertyCount() == 1); // No duplicate
}

TEST_CASE("TelemetryEvent rejects empty key", "[Phase15][TelemetryEvent]") {
    TelemetryEvent ev;
    ev.name = "test"; ev.source = "src";
    CHECK_FALSE(ev.setProperty("", "val"));
    CHECK(ev.propertyCount() == 0);
}

TEST_CASE("TelemetryEvent properties() returns all", "[Phase15][TelemetryEvent]") {
    TelemetryEvent ev;
    ev.name = "test"; ev.source = "src";
    ev.setProperty("a", "1");
    ev.setProperty("b", "2");
    auto& props = ev.properties();
    REQUIRE(props.size() == 2);
    CHECK(props[0].key == "a");
    CHECK(props[1].key == "b");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — TelemetryCollector
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TelemetryCollector starts inactive", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    CHECK_FALSE(tc.isActive());
    CHECK(tc.empty());
    CHECK(tc.count() == 0);
    CHECK(tc.sessionId().empty());
}

TEST_CASE("TelemetryCollector beginSession/endSession", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("session-001");
    CHECK(tc.isActive());
    CHECK(tc.sessionId() == "session-001");

    tc.endSession();
    CHECK_FALSE(tc.isActive());
    CHECK(tc.sessionId() == "session-001"); // Id retained after end
}

TEST_CASE("TelemetryCollector rejects records when inactive", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    CHECK_FALSE(tc.recordFeature("f", "src"));
    CHECK(tc.count() == 0);
}

TEST_CASE("TelemetryCollector rejects invalid events", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    TelemetryEvent bad; // invalid: no name/source
    CHECK_FALSE(tc.record(bad));
    CHECK(tc.count() == 0);
}

TEST_CASE("TelemetryCollector record and count", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");

    CHECK(tc.recordFeature("f1", "ToolA", 100));
    CHECK(tc.recordPerformance("perf1", "ToolB", 16.5, 200));
    CHECK(tc.recordError("err1", "ToolC", 300));

    CHECK(tc.count() == 3);
    CHECK_FALSE(tc.empty());
}

TEST_CASE("TelemetryCollector findByType", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    tc.recordFeature("f1", "A");
    tc.recordFeature("f2", "B");
    tc.recordError("e1", "C");

    auto features = tc.findByType(TelemetryEventType::FeatureUsage);
    CHECK(features.size() == 2);
    auto errors = tc.findByType(TelemetryEventType::Error);
    CHECK(errors.size() == 1);
}

TEST_CASE("TelemetryCollector findBySource", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    tc.recordFeature("f1", "ToolA");
    tc.recordFeature("f2", "ToolB");
    tc.recordError("e1", "ToolA");

    auto fromA = tc.findBySource("ToolA");
    CHECK(fromA.size() == 2);
}

TEST_CASE("TelemetryCollector findByName", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    tc.recordFeature("tool.open", "A");
    tc.recordFeature("tool.open", "B");
    tc.recordFeature("tool.close", "A");

    auto opens = tc.findByName("tool.open");
    CHECK(opens.size() == 2);
}

TEST_CASE("TelemetryCollector countByType", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    tc.recordFeature("f1", "A");
    tc.recordPerformance("p1", "A", 10.0);
    tc.recordPerformance("p2", "B", 20.0);

    CHECK(tc.countByType(TelemetryEventType::FeatureUsage) == 1);
    CHECK(tc.countByType(TelemetryEventType::Performance) == 2);
    CHECK(tc.countByType(TelemetryEventType::Error) == 0);
}

TEST_CASE("TelemetryCollector observer notification", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    std::vector<std::string> names;
    tc.addObserver([&](const TelemetryEvent& e) { names.push_back(e.name); });

    tc.recordFeature("f1", "A");
    tc.recordError("e1", "B");
    REQUIRE(names.size() == 2);
    CHECK(names[0] == "f1");
    CHECK(names[1] == "e1");
}

TEST_CASE("TelemetryCollector clearObservers", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    int calls = 0;
    tc.addObserver([&](const TelemetryEvent&) { ++calls; });
    tc.recordFeature("f1", "A");
    CHECK(calls == 1);

    tc.clearObservers();
    tc.recordFeature("f2", "A");
    CHECK(calls == 1);
}

TEST_CASE("TelemetryCollector clear removes events", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    tc.recordFeature("f1", "A");
    tc.recordFeature("f2", "B");
    CHECK(tc.count() == 2);

    tc.clear();
    CHECK(tc.empty());
    CHECK(tc.isActive()); // Session stays active
}

TEST_CASE("TelemetryCollector all returns events", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    tc.recordFeature("f1", "A");
    tc.recordError("e1", "B");

    auto& all = tc.all();
    REQUIRE(all.size() == 2);
    CHECK(all[0].name == "f1");
    CHECK(all[1].name == "e1");
}

TEST_CASE("TelemetryCollector beginSession clears previous events", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    tc.recordFeature("f1", "A");
    CHECK(tc.count() == 1);

    tc.beginSession("s2");
    CHECK(tc.count() == 0);
    CHECK(tc.sessionId() == "s2");
}

TEST_CASE("TelemetryCollector recordPerformance captures duration", "[Phase15][TelCollector]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    tc.recordPerformance("frame.render", "Renderer", 16.67, 1000);

    auto perfs = tc.findByType(TelemetryEventType::Performance);
    REQUIRE(perfs.size() == 1);
    CHECK(perfs[0].durationMs == 16.67);
    CHECK(perfs[0].timestampMs == 1000);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — DiagnosticSnapshot
// ═════════════════════════════════════════════════════════════════

TEST_CASE("DiagnosticSnapshot captures collector state", "[Phase15][Snapshot]") {
    DiagnosticCollector dc;
    dc.submitInfo("I1", "S", "M1");
    dc.submitWarning("W1", "S", "M2");
    dc.submitError("E1", "S", "M3");
    DiagnosticEntry fatal;
    fatal.id = "F1"; fatal.source = "S"; fatal.message = "M4";
    fatal.severity = DiagnosticSeverity::Fatal;
    dc.submit(fatal);
    dc.acknowledge("I1");

    auto snap = DiagnosticSnapshot::capture(dc);
    CHECK(snap.totalEntries == 4);
    CHECK(snap.infoCount == 1);
    CHECK(snap.warningCount == 1);
    CHECK(snap.errorCount == 1);
    CHECK(snap.fatalCount == 1);
    CHECK(snap.unacknowledgedCount == 3);
    CHECK(snap.hasErrors);
}

TEST_CASE("DiagnosticSnapshot on empty collector", "[Phase15][Snapshot]") {
    DiagnosticCollector dc;
    auto snap = DiagnosticSnapshot::capture(dc);
    CHECK(snap.totalEntries == 0);
    CHECK(snap.infoCount == 0);
    CHECK(snap.warningCount == 0);
    CHECK(snap.errorCount == 0);
    CHECK(snap.fatalCount == 0);
    CHECK(snap.unacknowledgedCount == 0);
    CHECK_FALSE(snap.hasErrors);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 7 — TelemetrySnapshot
// ═════════════════════════════════════════════════════════════════

TEST_CASE("TelemetrySnapshot captures collector state", "[Phase15][TelSnapshot]") {
    TelemetryCollector tc;
    tc.beginSession("snap-session");
    tc.recordFeature("f1", "A");
    tc.recordFeature("f2", "B");
    tc.recordPerformance("p1", "A", 10.0);
    tc.recordError("e1", "C");

    auto snap = TelemetrySnapshot::capture(tc);
    CHECK(snap.sessionId == "snap-session");
    CHECK(snap.active);
    CHECK(snap.totalEvents == 4);
    CHECK(snap.featureCount == 2);
    CHECK(snap.perfCount == 1);
    CHECK(snap.errorCount == 1);
}

TEST_CASE("TelemetrySnapshot on inactive collector", "[Phase15][TelSnapshot]") {
    TelemetryCollector tc;
    auto snap = TelemetrySnapshot::capture(tc);
    CHECK(snap.sessionId.empty());
    CHECK_FALSE(snap.active);
    CHECK(snap.totalEvents == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 8 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: diagnostic triggers telemetry error recording", "[Phase15][Integration]") {
    DiagnosticCollector dc;
    TelemetryCollector tc;
    tc.beginSession("integration-1");

    // Wire: diagnostic observer records telemetry on errors
    dc.addObserver([&](const DiagnosticEntry& e) {
        if (e.isError()) {
            tc.recordError("diag." + e.id, e.source, e.timestampMs);
        }
    });

    dc.submitInfo("I1", "Build", "OK");
    dc.submitError("E1", "Build", "Failed", DiagnosticCategory::Build, 100);
    dc.submitWarning("W1", "Asset", "Stale");
    dc.submitError("E2", "Plugin", "Crash", DiagnosticCategory::Plugin, 200);

    // Telemetry should only have the 2 error events
    CHECK(tc.count() == 2);
    auto errors = tc.findByType(TelemetryEventType::Error);
    REQUIRE(errors.size() == 2);
    CHECK(errors[0].name == "diag.E1");
    CHECK(errors[1].name == "diag.E2");
}

TEST_CASE("Integration: snapshots reflect current state accurately", "[Phase15][Integration]") {
    DiagnosticCollector dc;
    TelemetryCollector tc;
    tc.beginSession("snap-int");

    dc.submitInfo("I1", "S", "M");
    dc.submitError("E1", "S", "M");
    tc.recordFeature("f1", "T");
    tc.recordPerformance("p1", "T", 8.0);

    auto dSnap = DiagnosticSnapshot::capture(dc);
    auto tSnap = TelemetrySnapshot::capture(tc);

    CHECK(dSnap.totalEntries == 2);
    CHECK(dSnap.hasErrors);
    CHECK(tSnap.totalEvents == 2);
    CHECK(tSnap.featureCount == 1);
    CHECK(tSnap.perfCount == 1);

    // Clear and re-snapshot
    dc.clear();
    tc.clear();
    auto dSnap2 = DiagnosticSnapshot::capture(dc);
    auto tSnap2 = TelemetrySnapshot::capture(tc);
    CHECK(dSnap2.totalEntries == 0);
    CHECK_FALSE(dSnap2.hasErrors);
    CHECK(tSnap2.totalEvents == 0);
}

TEST_CASE("Integration: full diagnostic lifecycle with acknowledge", "[Phase15][Integration]") {
    DiagnosticCollector dc;

    // Submit a batch of diagnostics
    dc.submitInfo("SYS_001", "System", "Workspace started", DiagnosticCategory::System, 0);
    dc.submitWarning("ASSET_001", "AssetPipeline", "Texture not found", DiagnosticCategory::Asset, 100);
    dc.submitError("BUILD_001", "BuildPipeline", "Link error", DiagnosticCategory::Build, 200);
    dc.submitError("PLUGIN_001", "PluginLoader", "Plugin crashed", DiagnosticCategory::Plugin, 300);

    CHECK(dc.count() == 4);
    CHECK(dc.errorCount() == 2);
    CHECK(dc.unacknowledgedCount() == 4);

    // User acknowledges the build error
    dc.acknowledge("BUILD_001");
    CHECK(dc.unacknowledgedCount() == 3);

    // Snapshot
    auto snap = DiagnosticSnapshot::capture(dc);
    CHECK(snap.totalEntries == 4);
    CHECK(snap.errorCount == 2);
    CHECK(snap.unacknowledgedCount == 3);

    // Query by category
    auto buildDiags = dc.findByCategory(DiagnosticCategory::Build);
    REQUIRE(buildDiags.size() == 1);
    CHECK(buildDiags[0].id == "BUILD_001");

    // Acknowledge all
    dc.acknowledgeAll();
    CHECK(dc.unacknowledgedCount() == 0);
}

TEST_CASE("Integration: telemetry session restart clears events", "[Phase15][Integration]") {
    TelemetryCollector tc;
    tc.beginSession("s1");
    tc.recordFeature("f1", "A");
    tc.recordFeature("f2", "B");
    CHECK(tc.count() == 2);

    // Start new session
    tc.endSession();
    tc.beginSession("s2");
    CHECK(tc.count() == 0);
    CHECK(tc.sessionId() == "s2");

    tc.recordPerformance("p1", "A", 33.0);
    CHECK(tc.count() == 1);
}
