// Tests/Workspace/test_phase58_output_panel.cpp
// Phase 58 — WorkspaceOutputPanel: OutputSeverity, OutputEntry,
//             OutputChannel, OutputPanel
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceOutputPanel.h"

// ═════════════════════════════════════════════════════════════════
// OutputSeverity
// ═════════════════════════════════════════════════════════════════

TEST_CASE("OutputSeverity: name helpers", "[output][severity]") {
    REQUIRE(std::string(NF::outputSeverityName(NF::OutputSeverity::Info))    == "Info");
    REQUIRE(std::string(NF::outputSeverityName(NF::OutputSeverity::Warning)) == "Warning");
    REQUIRE(std::string(NF::outputSeverityName(NF::OutputSeverity::Error))   == "Error");
    REQUIRE(std::string(NF::outputSeverityName(NF::OutputSeverity::Debug))   == "Debug");
    REQUIRE(std::string(NF::outputSeverityName(NF::OutputSeverity::Trace))   == "Trace");
}

// ═════════════════════════════════════════════════════════════════
// OutputEntry
// ═════════════════════════════════════════════════════════════════

static NF::OutputEntry makeEntry(const std::string& id, const std::string& text,
                                  NF::OutputSeverity sev = NF::OutputSeverity::Info,
                                  const std::string& channel = "build") {
    NF::OutputEntry e;
    e.id          = id;
    e.text        = text;
    e.severity    = sev;
    e.channel     = channel;
    e.timestampMs = 1000;
    return e;
}

TEST_CASE("OutputEntry: default invalid", "[output][entry]") {
    NF::OutputEntry e;
    REQUIRE_FALSE(e.isValid());
}

TEST_CASE("OutputEntry: valid with id and text", "[output][entry]") {
    auto e = makeEntry("e1", "Build started");
    REQUIRE(e.isValid());
}

TEST_CASE("OutputEntry: invalid without text", "[output][entry]") {
    NF::OutputEntry e;
    e.id = "x";
    REQUIRE_FALSE(e.isValid());
}

TEST_CASE("OutputEntry: equality by id", "[output][entry]") {
    auto a = makeEntry("a", "msg1");
    auto b = makeEntry("a", "msg2");
    auto c = makeEntry("c", "msg1");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ═════════════════════════════════════════════════════════════════
// OutputChannel
// ═════════════════════════════════════════════════════════════════

TEST_CASE("OutputChannel: default empty", "[output][channel]") {
    NF::OutputChannel ch;
    ch.id   = "ch1";
    ch.name = "Build";
    REQUIRE(ch.isValid());
    REQUIRE(ch.empty());
    REQUIRE(ch.count() == 0);
}

TEST_CASE("OutputChannel: invalid without id", "[output][channel]") {
    NF::OutputChannel ch;
    ch.name = "Build";
    REQUIRE_FALSE(ch.isValid());
}

TEST_CASE("OutputChannel: addEntry valid", "[output][channel]") {
    NF::OutputChannel ch;
    ch.id   = "ch1";
    ch.name = "Build";
    auto e = makeEntry("e1", "Build started");
    REQUIRE(ch.addEntry(e));
    REQUIRE(ch.count() == 1);
    REQUIRE_FALSE(ch.empty());
}

TEST_CASE("OutputChannel: addEntry rejects invalid", "[output][channel]") {
    NF::OutputChannel ch;
    ch.id   = "ch1";
    ch.name = "Build";
    NF::OutputEntry e;
    REQUIRE_FALSE(ch.addEntry(e));
}

TEST_CASE("OutputChannel: clear", "[output][channel]") {
    NF::OutputChannel ch;
    ch.id   = "ch1";
    ch.name = "Build";
    ch.addEntry(makeEntry("e1", "msg1"));
    ch.addEntry(makeEntry("e2", "msg2"));
    REQUIRE(ch.count() == 2);
    ch.clear();
    REQUIRE(ch.empty());
}

TEST_CASE("OutputChannel: filterBySeverity", "[output][channel]") {
    NF::OutputChannel ch;
    ch.id   = "ch1";
    ch.name = "Build";
    ch.addEntry(makeEntry("e1", "info msg", NF::OutputSeverity::Info));
    ch.addEntry(makeEntry("e2", "error msg", NF::OutputSeverity::Error));
    ch.addEntry(makeEntry("e3", "warning", NF::OutputSeverity::Warning));
    ch.addEntry(makeEntry("e4", "another error", NF::OutputSeverity::Error));

    auto errors = ch.filterBySeverity(NF::OutputSeverity::Error);
    REQUIRE(errors.size() == 2);
}

TEST_CASE("OutputChannel: countBySeverity", "[output][channel]") {
    NF::OutputChannel ch;
    ch.id   = "ch1";
    ch.name = "Build";
    ch.addEntry(makeEntry("e1", "info", NF::OutputSeverity::Info));
    ch.addEntry(makeEntry("e2", "warn", NF::OutputSeverity::Warning));
    ch.addEntry(makeEntry("e3", "warn2", NF::OutputSeverity::Warning));

    REQUIRE(ch.countBySeverity(NF::OutputSeverity::Warning) == 2);
    REQUIRE(ch.countBySeverity(NF::OutputSeverity::Info) == 1);
    REQUIRE(ch.countBySeverity(NF::OutputSeverity::Error) == 0);
}

TEST_CASE("OutputChannel: lastEntry", "[output][channel]") {
    NF::OutputChannel ch;
    ch.id   = "ch1";
    ch.name = "Build";
    REQUIRE(ch.lastEntry() == nullptr);
    ch.addEntry(makeEntry("e1", "first"));
    ch.addEntry(makeEntry("e2", "second"));
    REQUIRE(ch.lastEntry() != nullptr);
    REQUIRE(ch.lastEntry()->id == "e2");
}

TEST_CASE("OutputChannel: evicts oldest when full", "[output][channel]") {
    NF::OutputChannel ch;
    ch.id   = "ch1";
    ch.name = "Build";
    for (int i = 0; i < NF::OutputChannel::MAX_ENTRIES; ++i) {
        ch.addEntry(makeEntry("e" + std::to_string(i), "msg" + std::to_string(i)));
    }
    REQUIRE(ch.count() == NF::OutputChannel::MAX_ENTRIES);
    // Adding one more should evict the oldest
    ch.addEntry(makeEntry("extra", "overflow"));
    REQUIRE(ch.count() == NF::OutputChannel::MAX_ENTRIES);
    REQUIRE(ch.entries().front().id == "e1");
    REQUIRE(ch.lastEntry()->id == "extra");
}

// ═════════════════════════════════════════════════════════════════
// OutputPanel
// ═════════════════════════════════════════════════════════════════

static NF::OutputChannel makeChannel(const std::string& id, const std::string& name) {
    NF::OutputChannel ch;
    ch.id   = id;
    ch.name = name;
    return ch;
}

TEST_CASE("OutputPanel: default empty", "[output][panel]") {
    NF::OutputPanel panel;
    REQUIRE(panel.channelCount() == 0);
    REQUIRE(panel.totalEntries() == 0);
}

TEST_CASE("OutputPanel: addChannel", "[output][panel]") {
    NF::OutputPanel panel;
    REQUIRE(panel.addChannel(makeChannel("build", "Build Output")));
    REQUIRE(panel.channelCount() == 1);
    REQUIRE(panel.hasChannel("build"));
}

TEST_CASE("OutputPanel: addChannel rejects invalid", "[output][panel]") {
    NF::OutputPanel panel;
    NF::OutputChannel ch;
    REQUIRE_FALSE(panel.addChannel(ch));
}

TEST_CASE("OutputPanel: addChannel rejects duplicate", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    REQUIRE_FALSE(panel.addChannel(makeChannel("build", "Build 2")));
}

TEST_CASE("OutputPanel: removeChannel", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    REQUIRE(panel.removeChannel("build"));
    REQUIRE(panel.channelCount() == 0);
}

TEST_CASE("OutputPanel: removeChannel unknown", "[output][panel]") {
    NF::OutputPanel panel;
    REQUIRE_FALSE(panel.removeChannel("missing"));
}

TEST_CASE("OutputPanel: write to channel", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    auto e = makeEntry("e1", "Compiling...");
    REQUIRE(panel.write("build", e));
    REQUIRE(panel.totalEntries() == 1);
}

TEST_CASE("OutputPanel: write to unknown channel", "[output][panel]") {
    NF::OutputPanel panel;
    REQUIRE_FALSE(panel.write("missing", makeEntry("e1", "msg")));
}

TEST_CASE("OutputPanel: searchByText", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    panel.addChannel(makeChannel("test", "Tests"));
    panel.write("build", makeEntry("e1", "Compiling main.cpp"));
    panel.write("build", makeEntry("e2", "Linking binary"));
    panel.write("test", makeEntry("e3", "Running compile tests"));

    auto results = panel.searchByText("compil");
    REQUIRE(results.size() == 2);
}

TEST_CASE("OutputPanel: searchByText case-insensitive", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    panel.write("build", makeEntry("e1", "ERROR: Build failed"));

    auto results = panel.searchByText("error");
    REQUIRE(results.size() == 1);
}

TEST_CASE("OutputPanel: searchByText empty query", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    panel.write("build", makeEntry("e1", "msg"));

    auto results = panel.searchByText("");
    REQUIRE(results.empty());
}

TEST_CASE("OutputPanel: searchBySeverity", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    panel.write("build", makeEntry("e1", "info msg", NF::OutputSeverity::Info));
    panel.write("build", makeEntry("e2", "error msg", NF::OutputSeverity::Error));
    panel.write("build", makeEntry("e3", "another error", NF::OutputSeverity::Error));

    auto errors = panel.searchBySeverity(NF::OutputSeverity::Error);
    REQUIRE(errors.size() == 2);
}

TEST_CASE("OutputPanel: observer on write", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));

    int callCount = 0;
    std::string lastText;
    panel.addObserver([&](const NF::OutputEntry& e) {
        ++callCount;
        lastText = e.text;
    });

    panel.write("build", makeEntry("e1", "Build started"));
    REQUIRE(callCount == 1);
    REQUIRE(lastText == "Build started");
}

TEST_CASE("OutputPanel: clearObservers", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));

    int callCount = 0;
    panel.addObserver([&](const NF::OutputEntry&) { ++callCount; });
    panel.clearObservers();
    panel.write("build", makeEntry("e1", "msg"));
    REQUIRE(callCount == 0);
}

TEST_CASE("OutputPanel: clearAllEntries", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    panel.addChannel(makeChannel("test", "Tests"));
    panel.write("build", makeEntry("e1", "msg1"));
    panel.write("test", makeEntry("e2", "msg2"));
    REQUIRE(panel.totalEntries() == 2);
    panel.clearAllEntries();
    REQUIRE(panel.totalEntries() == 0);
    REQUIRE(panel.channelCount() == 2);
}

TEST_CASE("OutputPanel: serialize empty", "[output][serial]") {
    NF::OutputPanel panel;
    REQUIRE(panel.serialize().empty());
}

TEST_CASE("OutputPanel: serialize round-trip", "[output][serial]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build Output"));
    panel.write("build", makeEntry("e1", "Compiling...", NF::OutputSeverity::Info));
    panel.write("build", makeEntry("e2", "Error: missing semicolon", NF::OutputSeverity::Error));

    std::string data = panel.serialize();
    REQUIRE_FALSE(data.empty());

    NF::OutputPanel panel2;
    REQUIRE(panel2.deserialize(data));
    REQUIRE(panel2.channelCount() == 1);
    REQUIRE(panel2.totalEntries() == 2);
    auto* ch = panel2.findChannel("build");
    REQUIRE(ch != nullptr);
    REQUIRE(ch->name == "Build Output");
    REQUIRE(ch->entries()[0].text == "Compiling...");
    REQUIRE(ch->entries()[1].severity == NF::OutputSeverity::Error);
}

TEST_CASE("OutputPanel: serialize pipe in text", "[output][serial]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("ch", "Ch"));
    panel.write("ch", makeEntry("e1", "a|b|c"));

    std::string data = panel.serialize();
    NF::OutputPanel panel2;
    panel2.deserialize(data);
    auto* ch = panel2.findChannel("ch");
    REQUIRE(ch != nullptr);
    REQUIRE(ch->entries()[0].text == "a|b|c");
}

TEST_CASE("OutputPanel: deserialize empty", "[output][serial]") {
    NF::OutputPanel panel;
    REQUIRE(panel.deserialize(""));
    REQUIRE(panel.channelCount() == 0);
}

TEST_CASE("OutputPanel: clear", "[output][panel]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    panel.write("build", makeEntry("e1", "msg"));
    int calls = 0;
    panel.addObserver([&](const NF::OutputEntry&) { ++calls; });
    panel.clear();
    REQUIRE(panel.channelCount() == 0);
    REQUIRE(panel.totalEntries() == 0);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-channel output with search", "[output][integration]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    panel.addChannel(makeChannel("test", "Tests"));
    panel.addChannel(makeChannel("lint", "Linter"));

    panel.write("build", makeEntry("b1", "Compiling main.cpp", NF::OutputSeverity::Info));
    panel.write("build", makeEntry("b2", "Error: undefined reference", NF::OutputSeverity::Error));
    panel.write("test", makeEntry("t1", "Running test suite", NF::OutputSeverity::Info));
    panel.write("test", makeEntry("t2", "FAIL: test_math", NF::OutputSeverity::Error));
    panel.write("lint", makeEntry("l1", "Warning: unused variable", NF::OutputSeverity::Warning));

    REQUIRE(panel.totalEntries() == 5);
    REQUIRE(panel.searchBySeverity(NF::OutputSeverity::Error).size() == 2);
    REQUIRE(panel.searchByText("error").size() == 1);
    REQUIRE(panel.searchByText("FAIL").size() == 1);
}

TEST_CASE("Integration: serialize/deserialize multi-channel preserves data", "[output][integration]") {
    NF::OutputPanel panel;
    panel.addChannel(makeChannel("build", "Build"));
    panel.addChannel(makeChannel("test", "Tests"));
    panel.write("build", makeEntry("b1", "OK", NF::OutputSeverity::Info));
    panel.write("test", makeEntry("t1", "PASS|all", NF::OutputSeverity::Debug));

    std::string data = panel.serialize();
    NF::OutputPanel panel2;
    panel2.deserialize(data);

    REQUIRE(panel2.channelCount() == 2);
    REQUIRE(panel2.totalEntries() == 2);

    auto* build = panel2.findChannel("build");
    auto* test  = panel2.findChannel("test");
    REQUIRE(build != nullptr);
    REQUIRE(test != nullptr);
    REQUIRE(build->entries()[0].text == "OK");
    REQUIRE(test->entries()[0].text == "PASS|all");
    REQUIRE(test->entries()[0].severity == NF::OutputSeverity::Debug);
}
