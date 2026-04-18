// Tests/Workspace/test_phase42_logging_route.cpp
// Phase 42 — Logging Route V1
//
// Tests for:
//   1. LogLevel helpers   — logLevelName, logLevelAtLeast
//   2. LogEntry           — isValid, isError, isWarning
//   3. LogSink            — isValid, accepts (level filter, tag filter, enabled flag)
//   4. LogRoute           — isValid, matchesSource (prefix match, empty = all)
//   5. LoggingRouteV1     — addSink/removeSink/addRoute/removeRoute/log; buffer; countByLevel
//   6. Integration        — multi-sink routing, level filtering, route pass-through control

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/LoggingRouteV1.h"
#include <string>
#include <vector>

using namespace NF;

// ── Helpers ───────────────────────────────────────────────────────

static LogSink makeSink(uint32_t id, const std::string& name,
                         LogLevel minLevel, LogSinkCallback cb) {
    LogSink s;
    s.id       = id;
    s.name     = name;
    s.minLevel = minLevel;
    s.callback = std::move(cb);
    s.enabled  = true;
    return s;
}

static LogRoute makeRoute(uint32_t id, const std::string& name,
                           const std::string& sourcePattern,
                           std::vector<uint32_t> sinkIds) {
    LogRoute r;
    r.id            = id;
    r.name          = name;
    r.sourcePattern = sourcePattern;
    r.sinkIds       = std::move(sinkIds);
    r.passThrough   = true;
    return r;
}

// ─────────────────────────────────────────────────────────────────
// 1. LogLevel helpers
// ─────────────────────────────────────────────────────────────────

TEST_CASE("logLevelName – all levels have names", "[phase42][logLevel]") {
    CHECK(std::string(logLevelName(LogLevel::Trace)) == "Trace");
    CHECK(std::string(logLevelName(LogLevel::Debug)) == "Debug");
    CHECK(std::string(logLevelName(LogLevel::Info))  == "Info");
    CHECK(std::string(logLevelName(LogLevel::Warn))  == "Warn");
    CHECK(std::string(logLevelName(LogLevel::Error)) == "Error");
    CHECK(std::string(logLevelName(LogLevel::Fatal)) == "Fatal");
}

TEST_CASE("logLevelAtLeast – ordering is Trace < Debug < Info < Warn < Error < Fatal", "[phase42][logLevel]") {
    CHECK(logLevelAtLeast(LogLevel::Trace, LogLevel::Trace));
    CHECK_FALSE(logLevelAtLeast(LogLevel::Trace, LogLevel::Debug));
    CHECK(logLevelAtLeast(LogLevel::Error, LogLevel::Warn));
    CHECK(logLevelAtLeast(LogLevel::Fatal, LogLevel::Fatal));
    CHECK_FALSE(logLevelAtLeast(LogLevel::Info, LogLevel::Error));
}

// ─────────────────────────────────────────────────────────────────
// 2. LogEntry
// ─────────────────────────────────────────────────────────────────

TEST_CASE("LogEntry – default is invalid (seq 0, empty message)", "[phase42][LogEntry]") {
    LogEntry e;
    CHECK_FALSE(e.isValid());
}

TEST_CASE("LogEntry – valid when seq > 0 and message non-empty", "[phase42][LogEntry]") {
    LogEntry e;
    e.seq     = 1;
    e.message = "hello";
    CHECK(e.isValid());
}

TEST_CASE("LogEntry – isError for Error and Fatal", "[phase42][LogEntry]") {
    LogEntry e;
    e.seq = 1; e.message = "x";
    e.level = LogLevel::Error;
    CHECK(e.isError());
    e.level = LogLevel::Fatal;
    CHECK(e.isError());
    e.level = LogLevel::Warn;
    CHECK_FALSE(e.isError());
}

TEST_CASE("LogEntry – isWarning for Warn and above", "[phase42][LogEntry]") {
    LogEntry e;
    e.seq = 1; e.message = "x";
    e.level = LogLevel::Warn;
    CHECK(e.isWarning());
    e.level = LogLevel::Error;
    CHECK(e.isWarning());
    e.level = LogLevel::Info;
    CHECK_FALSE(e.isWarning());
}

// ─────────────────────────────────────────────────────────────────
// 3. LogSink
// ─────────────────────────────────────────────────────────────────

TEST_CASE("LogSink – invalid without id", "[phase42][LogSink]") {
    LogSink s;
    s.name = "sink";
    s.callback = [](const LogEntry&) {};
    CHECK_FALSE(s.isValid()); // id == 0
}

TEST_CASE("LogSink – invalid without name", "[phase42][LogSink]") {
    LogSink s;
    s.id = 1;
    s.callback = [](const LogEntry&) {};
    CHECK_FALSE(s.isValid());
}

TEST_CASE("LogSink – invalid without callback", "[phase42][LogSink]") {
    LogSink s;
    s.id   = 1;
    s.name = "sink";
    CHECK_FALSE(s.isValid());
}

TEST_CASE("LogSink – valid with id + name + callback", "[phase42][LogSink]") {
    LogSink s = makeSink(1, "console", LogLevel::Info, [](const LogEntry&) {});
    CHECK(s.isValid());
}

TEST_CASE("LogSink – accepts filters by minLevel", "[phase42][LogSink]") {
    LogSink s = makeSink(1, "warn_sink", LogLevel::Warn, [](const LogEntry&) {});
    LogEntry e; e.seq = 1; e.message = "x";
    e.level = LogLevel::Info;
    CHECK_FALSE(s.accepts(e));
    e.level = LogLevel::Warn;
    CHECK(s.accepts(e));
    e.level = LogLevel::Error;
    CHECK(s.accepts(e));
}

TEST_CASE("LogSink – accepts filters by tagFilter (empty = any tag)", "[phase42][LogSink]") {
    LogSink s = makeSink(1, "tagged", LogLevel::Debug, [](const LogEntry&) {});
    s.tagFilter = "renderer";
    LogEntry e; e.seq = 1; e.message = "x"; e.level = LogLevel::Debug;
    e.tag = "renderer";
    CHECK(s.accepts(e));
    e.tag = "audio";
    CHECK_FALSE(s.accepts(e));
}

TEST_CASE("LogSink – accepts returns false when disabled", "[phase42][LogSink]") {
    LogSink s = makeSink(1, "sink", LogLevel::Trace, [](const LogEntry&) {});
    LogEntry e; e.seq = 1; e.message = "x"; e.level = LogLevel::Info;
    s.enabled = false;
    CHECK_FALSE(s.accepts(e));
}

// ─────────────────────────────────────────────────────────────────
// 4. LogRoute
// ─────────────────────────────────────────────────────────────────

TEST_CASE("LogRoute – invalid without id", "[phase42][LogRoute]") {
    LogRoute r;
    r.name = "route";
    CHECK_FALSE(r.isValid());
}

TEST_CASE("LogRoute – invalid without name", "[phase42][LogRoute]") {
    LogRoute r;
    r.id = 1;
    CHECK_FALSE(r.isValid());
}

TEST_CASE("LogRoute – valid with id and name", "[phase42][LogRoute]") {
    LogRoute r = makeRoute(1, "route", "", {});
    CHECK(r.isValid());
}

TEST_CASE("LogRoute – matchesSource: empty pattern matches anything", "[phase42][LogRoute]") {
    LogRoute r = makeRoute(1, "all", "", {});
    CHECK(r.matchesSource("Renderer/Vulkan"));
    CHECK(r.matchesSource(""));
    CHECK(r.matchesSource("Audio"));
}

TEST_CASE("LogRoute – matchesSource: prefix match", "[phase42][LogRoute]") {
    LogRoute r = makeRoute(1, "renderer_route", "Renderer", {});
    CHECK(r.matchesSource("Renderer/Vulkan"));
    CHECK(r.matchesSource("Renderer"));
    CHECK_FALSE(r.matchesSource("Audio"));
    CHECK_FALSE(r.matchesSource("UI"));
}

// ─────────────────────────────────────────────────────────────────
// 5. LoggingRouteV1
// ─────────────────────────────────────────────────────────────────

TEST_CASE("LoggingRouteV1 – default empty state", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    CHECK(router.sinkCount()   == 0);
    CHECK(router.routeCount()  == 0);
    CHECK(router.bufferSize()  == 0);
    CHECK(router.logCount()    == 0);
}

TEST_CASE("LoggingRouteV1 – addSink stores valid sink", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    CHECK(router.addSink(makeSink(1, "console", LogLevel::Trace, [](const LogEntry&) {})));
    CHECK(router.sinkCount() == 1);
}

TEST_CASE("LoggingRouteV1 – addSink rejects invalid sink", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    LogSink bad; // id=0, no name, no callback
    CHECK_FALSE(router.addSink(bad));
    CHECK(router.sinkCount() == 0);
}

TEST_CASE("LoggingRouteV1 – addSink rejects duplicate id", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    router.addSink(makeSink(1, "a", LogLevel::Info, [](const LogEntry&) {}));
    CHECK_FALSE(router.addSink(makeSink(1, "b", LogLevel::Info, [](const LogEntry&) {})));
    CHECK(router.sinkCount() == 1);
}

TEST_CASE("LoggingRouteV1 – removeSink succeeds", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    router.addSink(makeSink(5, "s", LogLevel::Info, [](const LogEntry&) {}));
    CHECK(router.removeSink(5));
    CHECK(router.sinkCount() == 0);
}

TEST_CASE("LoggingRouteV1 – removeSink unknown returns false", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    CHECK_FALSE(router.removeSink(99));
}

TEST_CASE("LoggingRouteV1 – addRoute stores valid route", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    CHECK(router.addRoute(makeRoute(1, "r", "", {})));
    CHECK(router.routeCount() == 1);
}

TEST_CASE("LoggingRouteV1 – addRoute rejects invalid route (no id)", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    LogRoute bad; // id=0
    CHECK_FALSE(router.addRoute(bad));
}

TEST_CASE("LoggingRouteV1 – addRoute rejects duplicate id", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    router.addRoute(makeRoute(1, "first", "", {}));
    CHECK_FALSE(router.addRoute(makeRoute(1, "second", "", {})));
    CHECK(router.routeCount() == 1);
}

TEST_CASE("LoggingRouteV1 – removeRoute succeeds", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    router.addRoute(makeRoute(3, "r", "", {}));
    CHECK(router.removeRoute(3));
    CHECK(router.routeCount() == 0);
}

TEST_CASE("LoggingRouteV1 – log buffers entry", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    router.log(LogLevel::Info, "test", "hello");
    CHECK(router.bufferSize() == 1);
    CHECK(router.logCount()   == 1);
    const auto& e = router.buffer().front();
    CHECK(e.level   == LogLevel::Info);
    CHECK(e.tag     == "test");
    CHECK(e.message == "hello");
    CHECK(e.seq     == 1);
}

TEST_CASE("LoggingRouteV1 – convenience helpers (trace/debug/info/warn/error/fatal)", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    router.trace("t", "m");
    router.debug("t", "m");
    router.info("t",  "m");
    router.warn("t",  "m");
    router.error("t", "m");
    router.fatal("t", "m");
    CHECK(router.logCount() == 6);
    CHECK(router.countByLevel(LogLevel::Trace) == 1);
    CHECK(router.countByLevel(LogLevel::Fatal) == 1);
}

TEST_CASE("LoggingRouteV1 – countByLevel filters correctly", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    router.info("t", "a");
    router.info("t", "b");
    router.error("t", "c");
    CHECK(router.countByLevel(LogLevel::Info)  == 2);
    CHECK(router.countByLevel(LogLevel::Error) == 1);
    CHECK(router.countByLevel(LogLevel::Warn)  == 0);
}

TEST_CASE("LoggingRouteV1 – clearBuffer empties buffer but keeps logCount", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    router.info("t", "x");
    router.info("t", "y");
    CHECK(router.logCount() == 2);
    router.clearBuffer();
    CHECK(router.bufferSize() == 0);
    CHECK(router.logCount()   == 2);  // logCount is cumulative
}

TEST_CASE("LoggingRouteV1 – sink receives logged entries above minLevel", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    std::vector<LogEntry> received;
    router.addSink(makeSink(1, "warn_only", LogLevel::Warn,
        [&](const LogEntry& e) { received.push_back(e); }));

    router.info("t", "should be filtered");
    router.warn("t", "should be received");
    router.error("t", "also received");

    CHECK(received.size() == 2);
    CHECK(received[0].level == LogLevel::Warn);
    CHECK(received[1].level == LogLevel::Error);
}

TEST_CASE("LoggingRouteV1 – setSinkEnabled disables delivery", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    int count = 0;
    router.addSink(makeSink(1, "s", LogLevel::Trace,
        [&](const LogEntry&) { ++count; }));

    router.info("t", "first");
    CHECK(count == 1);
    router.setSinkEnabled(1, false);
    router.info("t", "second");
    CHECK(count == 1);   // not delivered when disabled
    router.setSinkEnabled(1, true);
    router.info("t", "third");
    CHECK(count == 2);
}

TEST_CASE("LoggingRouteV1 – setMinLevel updates sink threshold", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    std::vector<LogLevel> levels;
    router.addSink(makeSink(1, "s", LogLevel::Error,
        [&](const LogEntry& e) { levels.push_back(e.level); }));

    router.warn("t", "filtered out");
    CHECK(levels.empty());

    router.setMinLevel(1, LogLevel::Debug);
    router.warn("t", "now accepted");
    CHECK(levels.size() == 1);
}

TEST_CASE("LoggingRouteV1 – findSink returns const pointer", "[phase42][LoggingRouteV1]") {
    LoggingRouteV1 router;
    router.addSink(makeSink(7, "s7", LogLevel::Info, [](const LogEntry&) {}));
    const LoggingRouteV1& cr = router;
    const LogSink* s = cr.findSink(7);
    REQUIRE(s != nullptr);
    CHECK(s->name == "s7");
    CHECK(cr.findSink(99) == nullptr);
}

// ─────────────────────────────────────────────────────────────────
// 6. Integration
// ─────────────────────────────────────────────────────────────────

TEST_CASE("LoggingRouteV1 integration – multi-sink with different level filters", "[phase42][integration]") {
    LoggingRouteV1 router;

    std::vector<std::string> verboseLog;
    std::vector<std::string> errorLog;

    router.addSink(makeSink(1, "verbose", LogLevel::Trace,
        [&](const LogEntry& e) { verboseLog.push_back(e.message); }));
    router.addSink(makeSink(2, "errors", LogLevel::Error,
        [&](const LogEntry& e) { errorLog.push_back(e.message); }));

    router.debug("sys",  "debug msg");
    router.info("sys",   "info msg");
    router.error("sys",  "error msg");
    router.fatal("sys",  "fatal msg");

    CHECK(verboseLog.size() == 4);  // all levels
    CHECK(errorLog.size()   == 2);  // Error + Fatal only
}

TEST_CASE("LoggingRouteV1 integration – tag-filtered sink", "[phase42][integration]") {
    LoggingRouteV1 router;

    std::vector<std::string> renderLogs;
    LogSink renderSink = makeSink(1, "render_sink", LogLevel::Debug,
        [&](const LogEntry& e) { renderLogs.push_back(e.tag); });
    renderSink.tagFilter = "Renderer";
    router.addSink(renderSink);

    router.info("Renderer", "frame begin");
    router.info("Audio",    "buffer underrun");
    router.info("Renderer", "draw call");

    REQUIRE(renderLogs.size() == 2);
    CHECK(renderLogs[0] == "Renderer");
    CHECK(renderLogs[1] == "Renderer");
}

TEST_CASE("LoggingRouteV1 integration – buffer accumulates multiple messages", "[phase42][integration]") {
    LoggingRouteV1 router;

    for (int i = 0; i < 10; ++i) {
        router.info("loop", "message " + std::to_string(i));
    }

    CHECK(router.bufferSize()  == 10);
    CHECK(router.logCount()    == 10);
    CHECK(router.countByLevel(LogLevel::Info) == 10);

    router.clearBuffer();
    CHECK(router.bufferSize() == 0);
    CHECK(router.logCount()   == 10);  // cumulative unchanged
}

TEST_CASE("LoggingRouteV1 integration – sequential seq numbers", "[phase42][integration]") {
    LoggingRouteV1 router;
    router.info("t", "a");
    router.warn("t", "b");
    router.error("t", "c");

    const auto& buf = router.buffer();
    REQUIRE(buf.size() == 3);
    CHECK(buf[0].seq == 1);
    CHECK(buf[1].seq == 2);
    CHECK(buf[2].seq == 3);
}
