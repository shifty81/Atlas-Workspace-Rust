#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S9 Tests: AtlasAI Integration ──────────────────────────────

TEST_CASE("AIInsightType names", "[Editor][S9]") {
    REQUIRE(std::string(aiInsightTypeName(AIInsightType::CodeQuality))       == "CodeQuality");
    REQUIRE(std::string(aiInsightTypeName(AIInsightType::PerformanceHint))   == "PerformanceHint");
    REQUIRE(std::string(aiInsightTypeName(AIInsightType::AssetOptimization)) == "AssetOptimization");
    REQUIRE(std::string(aiInsightTypeName(AIInsightType::LogicBug))          == "LogicBug");
    REQUIRE(std::string(aiInsightTypeName(AIInsightType::SecurityRisk))      == "SecurityRisk");
    REQUIRE(std::string(aiInsightTypeName(AIInsightType::Refactoring))       == "Refactoring");
    REQUIRE(std::string(aiInsightTypeName(AIInsightType::Documentation))     == "Documentation");
    REQUIRE(std::string(aiInsightTypeName(AIInsightType::General))           == "General");
}

TEST_CASE("AIInsight defaults", "[Editor][S9]") {
    AIInsight i;
    REQUIRE(i.type == AIInsightType::General);
    REQUIRE(i.confidence == 0.f);
    REQUIRE(i.severity == 0.f);
    REQUIRE_FALSE(i.dismissed);
    REQUIRE_FALSE(i.applied);
    REQUIRE_FALSE(i.isActionable()); // confidence 0 <= 0.5
}

TEST_CASE("AIInsight isActionable requires confidence > 0.5", "[Editor][S9]") {
    AIInsight i;
    i.confidence = 0.5f;
    REQUIRE_FALSE(i.isActionable()); // exactly 0.5 not actionable
    i.confidence = 0.51f;
    REQUIRE(i.isActionable());
}

TEST_CASE("AIInsight dismiss and markApplied", "[Editor][S9]") {
    AIInsight i;
    i.confidence = 0.8f;
    REQUIRE(i.isActionable());
    i.dismiss();
    REQUIRE(i.dismissed);
    REQUIRE_FALSE(i.isActionable());

    AIInsight j;
    j.confidence = 0.8f;
    j.markApplied();
    REQUIRE(j.applied);
    REQUIRE_FALSE(j.isActionable());
}

TEST_CASE("AIQueryRequest isValid", "[Editor][S9]") {
    AIQueryRequest r;
    REQUIRE_FALSE(r.isValid());
    r.queryId = "q1";
    REQUIRE_FALSE(r.isValid());
    r.prompt = "test";
    REQUIRE(r.isValid());
}

TEST_CASE("AIQueryPriority values", "[Editor][S9]") {
    REQUIRE(static_cast<int>(AIQueryPriority::Low)      == 0);
    REQUIRE(static_cast<int>(AIQueryPriority::Normal)   == 1);
    REQUIRE(static_cast<int>(AIQueryPriority::High)     == 2);
    REQUIRE(static_cast<int>(AIQueryPriority::Critical) == 3);
}

TEST_CASE("AIQueryResponse defaults", "[Editor][S9]") {
    AIQueryResponse r;
    REQUIRE_FALSE(r.success);
    REQUIRE_FALSE(r.hasInsights());
    REQUIRE(r.insightCount() == 0);
}

TEST_CASE("AIAnalysisEngine analyzeEvent AssetImported", "[Editor][S9]") {
    AIAnalysisEngine eng;
    eng.analyzeEvent("AssetImported", "models/ship.fbx", "");
    REQUIRE(eng.insightCount() == 1);
    auto& i = eng.insights()[0];
    REQUIRE(i.type == AIInsightType::AssetOptimization);
    REQUIRE(i.confidence == Catch::Approx(0.7f));
}

TEST_CASE("AIAnalysisEngine analyzeEvent ScriptUpdated", "[Editor][S9]") {
    AIAnalysisEngine eng;
    eng.analyzeEvent("ScriptUpdated", "scripts/main.lua", "");
    REQUIRE(eng.insightCount() == 1);
    REQUIRE(eng.insights()[0].type == AIInsightType::CodeQuality);
}

TEST_CASE("AIAnalysisEngine analyzeEvent ContractIssue", "[Editor][S9]") {
    AIAnalysisEngine eng;
    eng.analyzeEvent("ContractIssue", "src/foo.h", "");
    REQUIRE(eng.insightCount() == 1);
    REQUIRE(eng.insights()[0].type == AIInsightType::SecurityRisk);
    REQUIRE(eng.insights()[0].confidence == Catch::Approx(0.9f));
}

TEST_CASE("AIAnalysisEngine actionableInsights filtering", "[Editor][S9]") {
    AIAnalysisEngine eng;
    eng.analyzeEvent("AssetImported", "a.fbx", "");    // confidence 0.7 -> actionable
    eng.analyzeEvent("ScriptUpdated", "b.lua", "");    // confidence 0.6 -> actionable
    eng.analyzeEvent("Other", "c.txt", "");            // confidence 0.5 -> NOT actionable
    REQUIRE(eng.insightCount() == 3);
    REQUIRE(eng.actionableInsights().size() == 2);
}

TEST_CASE("AIAnalysisEngine insightsByType", "[Editor][S9]") {
    AIAnalysisEngine eng;
    eng.analyzeEvent("AssetImported", "a.fbx", "");
    eng.analyzeEvent("AssetImported", "b.fbx", "");
    eng.analyzeEvent("ScriptUpdated", "c.lua", "");
    REQUIRE(eng.insightsByType(AIInsightType::AssetOptimization).size() == 2);
    REQUIRE(eng.insightsByType(AIInsightType::CodeQuality).size() == 1);
}

TEST_CASE("AIAnalysisEngine dismissInsight + clearDismissed", "[Editor][S9]") {
    AIAnalysisEngine eng;
    eng.analyzeEvent("AssetImported", "a.fbx", "");
    eng.analyzeEvent("ScriptUpdated", "b.lua", "");
    eng.dismissInsight(eng.insights()[0].id);
    REQUIRE(eng.insights()[0].dismissed);
    eng.clearDismissed();
    REQUIRE(eng.insightCount() == 1);
}

TEST_CASE("AIProactiveSuggester generateSuggestions", "[Editor][S9]") {
    AIAnalysisEngine eng;
    eng.analyzeEvent("AssetImported", "a.fbx", "");
    eng.analyzeEvent("ContractIssue", "b.h", "");

    AIProactiveSuggester sug;
    sug.generateSuggestions(eng);
    REQUIRE(sug.suggestionCount() == 2);
    REQUIRE(sug.pendingCount() == 2);
}

TEST_CASE("AIProactiveSuggester accept/reject + pendingCount", "[Editor][S9]") {
    AIAnalysisEngine eng;
    eng.analyzeEvent("AssetImported", "a.fbx", "");
    eng.analyzeEvent("ContractIssue", "b.h", "");

    AIProactiveSuggester sug;
    sug.generateSuggestions(eng);
    REQUIRE(sug.pendingCount() == 2);

    sug.acceptSuggestion(sug.suggestions()[0].id);
    REQUIRE(sug.pendingCount() == 1);

    sug.rejectSuggestion(sug.suggestions()[1].id);
    REQUIRE(sug.pendingCount() == 0);
}

TEST_CASE("AIProactiveSuggester clearResolved", "[Editor][S9]") {
    AIAnalysisEngine eng;
    eng.analyzeEvent("AssetImported", "a.fbx", "");

    AIProactiveSuggester sug;
    sug.generateSuggestions(eng);
    sug.acceptSuggestion(sug.suggestions()[0].id);
    sug.clearResolved();
    REQUIRE(sug.suggestionCount() == 0);
}

TEST_CASE("AIPipelineBridge processEvent + eventsProcessed", "[Editor][S9]") {
    AIPipelineBridge bridge;
    REQUIRE(bridge.eventsProcessed() == 0);
    bridge.processEvent("AssetImported", "a.fbx");
    bridge.processEvent("ScriptUpdated", "b.lua");
    REQUIRE(bridge.eventsProcessed() == 2);
    REQUIRE(bridge.engine().insightCount() == 2);
}

TEST_CASE("AIPipelineBridge submitQuery success + failure", "[Editor][S9]") {
    AIPipelineBridge bridge;

    AIQueryRequest bad;
    auto r1 = bridge.submitQuery(bad);
    REQUIRE_FALSE(r1.success);

    AIQueryRequest good;
    good.queryId = "q1";
    good.prompt = "analyze this";
    good.context = "src/test.cpp";
    auto r2 = bridge.submitQuery(good);
    REQUIRE(r2.success);
    REQUIRE(bridge.queriesProcessed() == 1);
}

TEST_CASE("AtlasAIIntegration init/shutdown", "[Editor][S9]") {
    AtlasAIIntegration ai;
    REQUIRE_FALSE(ai.isInitialized());
    ai.init();
    REQUIRE(ai.isInitialized());
    ai.shutdown();
    REQUIRE_FALSE(ai.isInitialized());
}

TEST_CASE("AtlasAIIntegration processEvent + totalInsights", "[Editor][S9]") {
    AtlasAIIntegration ai;
    ai.init();
    ai.processEvent("AssetImported", "a.fbx");
    ai.processEvent("ContractIssue", "b.h");
    REQUIRE(ai.totalInsights() == 2);
    REQUIRE(ai.totalEvents() == 2);
}

TEST_CASE("AtlasAIIntegration tick with proactive suggestions", "[Editor][S9]") {
    AtlasAIIntegration ai;
    AtlasAIConfig cfg;
    cfg.analysisTickRate = 1.f;
    cfg.proactiveSuggestions = true;
    ai.init(cfg);

    ai.processEvent("ContractIssue", "c.h");
    REQUIRE(ai.totalInsights() == 1);

    ai.tick(1.0f); // triggers suggestion generation
    REQUIRE(ai.tickCount() == 1);
    REQUIRE(ai.totalSuggestions() >= 1);
    REQUIRE(ai.pendingSuggestions() >= 1);
}
