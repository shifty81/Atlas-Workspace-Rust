// S145 editor tests: LoggingRouteV1, AIDebugPathV1, CodexSnippetMirror
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── LoggingRouteV1 ────────────────────────────────────────────────────────────

TEST_CASE("LogLevel names and comparison", "[Editor][S145]") {
    REQUIRE(std::string(logLevelName(LogLevel::Trace))   == "Trace");
    REQUIRE(std::string(logLevelName(LogLevel::Debug))   == "Debug");
    REQUIRE(std::string(logLevelName(LogLevel::Info))    == "Info");
    REQUIRE(std::string(logLevelName(LogLevel::Warn)) == "Warn");
    REQUIRE(std::string(logLevelName(LogLevel::Error))   == "Error");
    REQUIRE(std::string(logLevelName(LogLevel::Fatal))   == "Fatal");

    REQUIRE(logLevelAtLeast(LogLevel::Warn, LogLevel::Info));
    REQUIRE(!logLevelAtLeast(LogLevel::Debug, LogLevel::Warn));
}

TEST_CASE("LogEntry isValid and helpers", "[Editor][S145]") {
    LogEntry e;
    REQUIRE(!e.isValid());
    e.seq = 1; e.message = "hello"; e.level = LogLevel::Info;
    REQUIRE(e.isValid());
    REQUIRE(!e.isError());
    REQUIRE(!e.isWarning());
    e.level = LogLevel::Error;
    REQUIRE(e.isError());
    REQUIRE(e.isWarning());
}

TEST_CASE("LogSink accepts filtering by level and tag", "[Editor][S145]") {
    LogSink sink;
    sink.id       = 1;
    sink.name     = "console";
    sink.minLevel = LogLevel::Warn;
    sink.callback = [](const LogEntry&) {};

    LogEntry low; low.seq = 1; low.level = LogLevel::Debug; low.message = "d";
    LogEntry high; high.seq = 2; high.level = LogLevel::Error; high.message = "e";
    REQUIRE(!sink.accepts(low));
    REQUIRE(sink.accepts(high));

    sink.tagFilter = "network";
    LogEntry tagged; tagged.seq = 3; tagged.level = LogLevel::Error; tagged.message = "e"; tagged.tag = "network";
    LogEntry untagged; untagged.seq = 4; untagged.level = LogLevel::Error; untagged.message = "e"; untagged.tag = "ui";
    REQUIRE(sink.accepts(tagged));
    REQUIRE(!sink.accepts(untagged));

    sink.enabled = false;
    REQUIRE(!sink.accepts(tagged));
}

TEST_CASE("LoggingRouteV1 addSink and reject duplicate", "[Editor][S145]") {
    LoggingRouteV1 logger;
    LogSink s; s.id = 1; s.name = "console"; s.callback = [](const LogEntry&){};
    REQUIRE(logger.addSink(std::move(s)));
    REQUIRE(logger.sinkCount() == 1u);
    LogSink dup; dup.id = 1; dup.name = "dup"; dup.callback = [](const LogEntry&){};
    REQUIRE(!logger.addSink(std::move(dup)));
}

TEST_CASE("LoggingRouteV1 log delivers to matching sink", "[Editor][S145]") {
    LoggingRouteV1 logger;
    std::vector<LogEntry> received;
    LogSink s; s.id = 1; s.name = "test"; s.minLevel = LogLevel::Info;
    s.callback = [&](const LogEntry& e) { received.push_back(e); };
    logger.addSink(std::move(s));

    logger.info("app", "hello");
    REQUIRE(received.size() == 1u);
    REQUIRE(received[0].message == "hello");

    logger.trace("app", "low");  // below minLevel
    REQUIRE(received.size() == 1u);  // not delivered

    logger.error("app", "boom");
    REQUIRE(received.size() == 2u);
    REQUIRE(received[1].isError());
}

TEST_CASE("LoggingRouteV1 convenience helpers", "[Editor][S145]") {
    LoggingRouteV1 logger;
    size_t cnt = 0;
    LogSink s; s.id = 1; s.name = "t"; s.minLevel = LogLevel::Trace;
    s.callback = [&](const LogEntry&) { ++cnt; };
    logger.addSink(std::move(s));

    logger.trace("t", "msg");
    logger.debug("t", "msg");
    logger.info("t", "msg");
    logger.warn("t", "msg");
    logger.error("t", "msg");
    logger.fatal("t", "msg");
    REQUIRE(cnt == 6u);
    REQUIRE(logger.logCount() == 6u);
}

TEST_CASE("LoggingRouteV1 buffer and countByLevel", "[Editor][S145]") {
    LoggingRouteV1 logger;
    logger.info("t", "A");
    logger.warn("t", "B");
    logger.error("t", "C");
    logger.error("t", "D");
    REQUIRE(logger.bufferSize() == 4u);
    REQUIRE(logger.countByLevel(LogLevel::Error) == 2u);
    REQUIRE(logger.countByLevel(LogLevel::Info) == 1u);
    logger.clearBuffer();
    REQUIRE(logger.bufferSize() == 0u);
}

TEST_CASE("LoggingRouteV1 addRoute and removeSink", "[Editor][S145]") {
    LoggingRouteV1 logger;
    LogSink s; s.id = 1; s.name = "s"; s.callback = [](const LogEntry&){};
    logger.addSink(std::move(s));

    LogRoute r; r.id = 1; r.name = "engine-route"; r.sourcePattern = "engine";
    r.sinkIds = {1};
    REQUIRE(logger.addRoute(r));
    REQUIRE(logger.routeCount() == 1u);

    REQUIRE(logger.removeRoute(1));
    REQUIRE(logger.routeCount() == 0u);

    REQUIRE(logger.removeSink(1));
    REQUIRE(logger.sinkCount() == 0u);
}

TEST_CASE("LoggingRouteV1 setSinkEnabled and setMinLevel", "[Editor][S145]") {
    LoggingRouteV1 logger;
    std::vector<LogEntry> received;
    LogSink s; s.id = 1; s.name = "s"; s.minLevel = LogLevel::Debug;
    s.callback = [&](const LogEntry& e) { received.push_back(e); };
    logger.addSink(std::move(s));

    logger.info("t", "first");
    REQUIRE(received.size() == 1u);

    logger.setSinkEnabled(1, false);
    logger.info("t", "second");
    REQUIRE(received.size() == 1u);  // sink disabled

    logger.setSinkEnabled(1, true);
    logger.setMinLevel(1, LogLevel::Error);
    logger.info("t", "third");
    REQUIRE(received.size() == 1u);  // below new minLevel
    logger.error("t", "fourth");
    REQUIRE(received.size() == 2u);
}

// ── AIDebugPathV1 ─────────────────────────────────────────────────────────────

TEST_CASE("AIDebugStepType names", "[Editor][S145]") {
    REQUIRE(std::string(aiDebugStepTypeName(AIDebugStepType::Perception))    == "Perception");
    REQUIRE(std::string(aiDebugStepTypeName(AIDebugStepType::Decision))      == "Decision");
    REQUIRE(std::string(aiDebugStepTypeName(AIDebugStepType::Execution))     == "Execution");
    REQUIRE(std::string(aiDebugStepTypeName(AIDebugStepType::Interrupt))     == "Interrupt");
    REQUIRE(std::string(aiDebugStepTypeName(AIDebugStepType::Idle))          == "Idle");
}

TEST_CASE("AIDebugStep isValid and annotations", "[Editor][S145]") {
    AIDebugStep s;
    REQUIRE(!s.isValid());
    s.id = 1; s.agentId = "soldier-1";
    REQUIRE(s.isValid());
    s.addAnnotation("target", "enemy-3", 0.9f);
    s.addAnnotation("threat", "low", 0.2f);
    REQUIRE(s.annotations.size() == 2u);
    REQUIRE(s.findAnnotation("target") != nullptr);
    REQUIRE(s.findAnnotation("missing") == nullptr);
    REQUIRE(s.findAnnotation("target")->score == Approx(0.9f));
}

TEST_CASE("AIDecisionLogEntry isValid and scoreDelta", "[Editor][S145]") {
    AIDecisionLogEntry e;
    REQUIRE(!e.isValid());
    e.id = 1; e.agentId = "guard";
    REQUIRE(e.isValid());
    e.chosenScore = 0.8f; e.runnerUpScore = 0.5f;
    REQUIRE(e.scoreDelta() == Approx(0.3f));
}

TEST_CASE("AIDebugPathV1 beginRecording and endRecording", "[Editor][S145]") {
    AIDebugPathV1 path;
    REQUIRE(!path.isRecording());
    path.beginRecording("agent-1");
    REQUIRE(path.isRecording());
    REQUIRE(path.recordingAgentId() == "agent-1");
    path.endRecording();
    REQUIRE(!path.isRecording());
    REQUIRE(path.recordingAgentId().empty());
}

TEST_CASE("AIDebugPathV1 addStep and tick", "[Editor][S145]") {
    AIDebugPathV1 path;
    path.beginRecording("agent-A");

    AIDebugStep s1; s1.id = 1; s1.agentId = "agent-A"; s1.type = AIDebugStepType::Perception;
    AIDebugStep s2; s2.id = 2; s2.agentId = "agent-A"; s2.type = AIDebugStepType::Decision;
    path.tick();
    REQUIRE(path.addStep(s1));
    path.tick();
    REQUIRE(path.addStep(s2));

    REQUIRE(path.stepCount() == 2u);
    REQUIRE(path.currentTick() == 2u);
}

TEST_CASE("AIDebugPathV1 replayAgent filters by agentId", "[Editor][S145]") {
    AIDebugPathV1 path;
    AIDebugStep a; a.id = 1; a.agentId = "agent-A"; a.type = AIDebugStepType::Idle;
    AIDebugStep b; b.id = 2; b.agentId = "agent-B"; b.type = AIDebugStepType::Decision;
    AIDebugStep a2; a2.id = 3; a2.agentId = "agent-A"; a2.type = AIDebugStepType::Execution;
    path.addStep(a); path.addStep(b); path.addStep(a2);

    auto replayA = path.replayAgent("agent-A");
    REQUIRE(replayA.size() == 2u);
    auto replayB = path.replayAgent("agent-B");
    REQUIRE(replayB.size() == 1u);
}

TEST_CASE("AIDebugPathV1 filterByType", "[Editor][S145]") {
    AIDebugPathV1 path;
    AIDebugStep s1; s1.id = 1; s1.agentId = "a"; s1.type = AIDebugStepType::Decision;
    AIDebugStep s2; s2.id = 2; s2.agentId = "a"; s2.type = AIDebugStepType::Idle;
    AIDebugStep s3; s3.id = 3; s3.agentId = "a"; s3.type = AIDebugStepType::Decision;
    path.addStep(s1); path.addStep(s2); path.addStep(s3);
    REQUIRE(path.filterByType(AIDebugStepType::Decision).size() == 2u);
    REQUIRE(path.filterByType(AIDebugStepType::Idle).size() == 1u);
}

TEST_CASE("AIDebugPathV1 logDecision and decisionsForAgent", "[Editor][S145]") {
    AIDebugPathV1 path;
    AIDecisionLogEntry e; e.id = 1; e.agentId = "sniper"; e.chosenAction = "retreat";
    REQUIRE(path.logDecision(e));
    REQUIRE(path.decisionCount() == 1u);
    REQUIRE(path.decisionsForAgent("sniper").size() == 1u);
    REQUIRE(path.decisionsForAgent("guard").empty());
}

TEST_CASE("AIDebugPathV1 onStep callback fires", "[Editor][S145]") {
    AIDebugPathV1 path;
    uint32_t lastId = 0;
    path.setOnStep([&](const AIDebugStep& s) { lastId = s.id; });
    AIDebugStep s; s.id = 42; s.agentId = "bot";
    path.addStep(s);
    REQUIRE(lastId == 42u);
}

TEST_CASE("AIDebugPathV1 clear", "[Editor][S145]") {
    AIDebugPathV1 path;
    AIDebugStep s; s.id = 1; s.agentId = "a";
    path.addStep(s);
    AIDecisionLogEntry e; e.id = 1; e.agentId = "a";
    path.logDecision(e);
    path.clear();
    REQUIRE(path.bufferedSteps() == 0u);
    REQUIRE(path.decisionCount() == 0u);
}

// ── CodexSnippetMirror ────────────────────────────────────────────────────────

TEST_CASE("SnippetLanguage names", "[Editor][S145]") {
    REQUIRE(std::string(snippetLanguageName(SnippetLanguage::Cpp))      == "C++");
    REQUIRE(std::string(snippetLanguageName(SnippetLanguage::Lua))      == "Lua");
    REQUIRE(std::string(snippetLanguageName(SnippetLanguage::GLSL))     == "GLSL");
    REQUIRE(std::string(snippetLanguageName(SnippetLanguage::Markdown)) == "Markdown");
    REQUIRE(std::string(snippetLanguageName(SnippetLanguage::Any))      == "Any");
}

TEST_CASE("SnippetSyncState names", "[Editor][S145]") {
    REQUIRE(std::string(snippetSyncStateName(SnippetSyncState::Local))    == "Local");
    REQUIRE(std::string(snippetSyncStateName(SnippetSyncState::Synced))   == "Synced");
    REQUIRE(std::string(snippetSyncStateName(SnippetSyncState::Conflict)) == "Conflict");
    REQUIRE(std::string(snippetSyncStateName(SnippetSyncState::Pending))  == "Pending");
}

TEST_CASE("CodexSnippet validity and tag management", "[Editor][S145]") {
    CodexSnippet s;
    REQUIRE(!s.isValid());
    s.id = 1; s.title = "Hello World"; s.body = "print('hello')";
    REQUIRE(s.isValid());
    s.addTag("tutorial");
    s.addTag("python");
    REQUIRE(s.hasTag("tutorial"));
    REQUIRE(!s.hasTag("cpp"));
    s.addTag("tutorial");  // duplicate
    REQUIRE(s.tags.size() == 2u);
    REQUIRE(s.removeTag("tutorial"));
    REQUIRE(!s.hasTag("tutorial"));
    REQUIRE(!s.removeTag("nonexistent"));
}

TEST_CASE("CodexSnippetMirror addSnippet and reject duplicate", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CodexSnippet s; s.id = 1; s.title = "Vec3 init"; s.body = "Vec3(0,0,0);";
    REQUIRE(mirror.addSnippet(s));
    REQUIRE(mirror.snippetCount() == 1u);
    REQUIRE(!mirror.addSnippet(s));  // duplicate
}

TEST_CASE("CodexSnippetMirror removeSnippet", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CodexSnippet s; s.id = 1; s.title = "A"; s.body = "body";
    mirror.addSnippet(s);
    REQUIRE(mirror.removeSnippet(1));
    REQUIRE(mirror.snippetCount() == 0u);
    REQUIRE(!mirror.removeSnippet(99));
}

TEST_CASE("CodexSnippetMirror updateBody marks Modified", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CodexSnippet s; s.id = 1; s.title = "A"; s.body = "old"; s.syncState = SnippetSyncState::Synced;
    mirror.addSnippet(s);
    REQUIRE(mirror.updateBody(1, "new"));
    REQUIRE(mirror.findSnippet(1)->body == "new");
    REQUIRE(mirror.findSnippet(1)->syncState == SnippetSyncState::Modified);
    REQUIRE(!mirror.updateBody(1, ""));  // empty rejected
}

TEST_CASE("CodexSnippetMirror markSynced and markConflict", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CodexSnippet s; s.id = 1; s.title = "A"; s.body = "x";
    mirror.addSnippet(s);
    REQUIRE(mirror.markSynced(1, "codex-abc-123"));
    REQUIRE(mirror.findSnippet(1)->isSynced());
    REQUIRE(mirror.findSnippet(1)->codexId == "codex-abc-123");
    REQUIRE(mirror.syncCount() == 1u);

    REQUIRE(mirror.markConflict(1));
    REQUIRE(mirror.findSnippet(1)->hasConflict());
    REQUIRE(mirror.conflictCount() == 1u);
}

TEST_CASE("CodexSnippetMirror resolveConflict", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CodexSnippet s; s.id = 1; s.title = "A"; s.body = "x"; s.syncState = SnippetSyncState::Conflict;
    mirror.addSnippet(s);
    REQUIRE(mirror.resolveConflict(1, true));  // keep local
    REQUIRE(mirror.findSnippet(1)->syncState == SnippetSyncState::Modified);

    mirror.markConflict(1);
    REQUIRE(mirror.resolveConflict(1, false));  // accept remote
    REQUIRE(mirror.findSnippet(1)->syncState == SnippetSyncState::Synced);
}

TEST_CASE("CodexSnippetMirror search", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CodexSnippet a; a.id = 1; a.title = "Vec3 helper"; a.body = "Vec3(x,y,z)"; a.language = SnippetLanguage::Cpp;
    CodexSnippet b; b.id = 2; b.title = "Print debug"; b.body = "print(x)"; b.language = SnippetLanguage::Lua;
    b.addTag("debug");
    CodexSnippet c; c.id = 3; c.title = "Shader param"; c.body = "uniform float"; c.language = SnippetLanguage::GLSL;
    mirror.addSnippet(a); mirror.addSnippet(b); mirror.addSnippet(c);

    auto res = mirror.search("Vec");
    REQUIRE(res.size() == 1u);
    REQUIRE(res[0] == 1u);

    auto tagRes = mirror.search("debug");
    REQUIRE(tagRes.size() == 1u);
}

TEST_CASE("CodexSnippetMirror filterByLanguage", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CodexSnippet a; a.id = 1; a.title = "A"; a.body = "x"; a.language = SnippetLanguage::Cpp;
    CodexSnippet b; b.id = 2; b.title = "B"; b.body = "y"; b.language = SnippetLanguage::Lua;
    CodexSnippet c; c.id = 3; c.title = "C"; c.body = "z"; c.language = SnippetLanguage::Any;
    mirror.addSnippet(a); mirror.addSnippet(b); mirror.addSnippet(c);

    auto cpp = mirror.filterByLanguage(SnippetLanguage::Cpp);
    REQUIRE(cpp.size() == 2u);  // Cpp + Any

    auto lua = mirror.filterByLanguage(SnippetLanguage::Lua);
    REQUIRE(lua.size() == 2u);  // Lua + Any
}

TEST_CASE("CodexSnippetMirror filterByTag", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CodexSnippet a; a.id = 1; a.title = "A"; a.body = "x"; a.addTag("math");
    CodexSnippet b; b.id = 2; b.title = "B"; b.body = "y"; b.addTag("math"); b.addTag("geometry");
    CodexSnippet c; c.id = 3; c.title = "C"; c.body = "z";
    mirror.addSnippet(a); mirror.addSnippet(b); mirror.addSnippet(c);
    REQUIRE(mirror.filterByTag("math").size() == 2u);
    REQUIRE(mirror.filterByTag("geometry").size() == 1u);
    REQUIRE(mirror.filterByTag("nope").empty());
}

TEST_CASE("CodexSnippetMirror pendingSync lists Modified and Pending", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    CodexSnippet a; a.id = 1; a.title = "A"; a.body = "x"; a.syncState = SnippetSyncState::Local;
    CodexSnippet b; b.id = 2; b.title = "B"; b.body = "y"; b.syncState = SnippetSyncState::Modified;
    CodexSnippet c; c.id = 3; c.title = "C"; c.body = "z"; c.syncState = SnippetSyncState::Pending;
    mirror.addSnippet(a); mirror.addSnippet(b); mirror.addSnippet(c);
    REQUIRE(mirror.pendingSync().size() == 2u);
}

TEST_CASE("SnippetInsertTarget and register/unregister", "[Editor][S145]") {
    CodexSnippetMirror mirror;
    SnippetInsertTarget t; t.panelId = "code-editor"; t.line = 10;
    REQUIRE(t.isValid());
    mirror.registerInsertTarget(t);
    REQUIRE(mirror.insertTargetCount() == 1u);
    REQUIRE(mirror.unregisterInsertTarget("code-editor"));
    REQUIRE(mirror.insertTargetCount() == 0u);
}
