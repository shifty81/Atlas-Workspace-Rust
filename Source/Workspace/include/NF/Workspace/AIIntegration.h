#pragma once
// NF::Editor — AI analysis engine, proactive suggester, pipeline bridge, Atlas AI
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class AIInsightType : uint8_t {
    CodeQuality,
    PerformanceHint,
    AssetOptimization,
    LogicBug,
    SecurityRisk,
    Refactoring,
    Documentation,
    General
};

inline const char* aiInsightTypeName(AIInsightType t) noexcept {
    switch (t) {
        case AIInsightType::CodeQuality:       return "CodeQuality";
        case AIInsightType::PerformanceHint:   return "PerformanceHint";
        case AIInsightType::AssetOptimization: return "AssetOptimization";
        case AIInsightType::LogicBug:          return "LogicBug";
        case AIInsightType::SecurityRisk:      return "SecurityRisk";
        case AIInsightType::Refactoring:       return "Refactoring";
        case AIInsightType::Documentation:     return "Documentation";
        case AIInsightType::General:           return "General";
        default:                                return "Unknown";
    }
}

struct AIInsight {
    std::string id;
    AIInsightType type = AIInsightType::General;
    std::string title;
    std::string description;
    std::string sourceTool;
    std::string affectedPath;
    float confidence = 0.f;
    float severity = 0.f;
    int64_t timestamp = 0;
    bool dismissed = false;
    bool applied = false;

    [[nodiscard]] bool isActionable() const { return !dismissed && !applied && confidence > 0.5f; }
    void dismiss() { dismissed = true; }
    void markApplied() { applied = true; }
};

enum class AIQueryPriority : uint8_t { Low, Normal, High, Critical };

struct AIQueryRequest {
    std::string queryId;
    std::string prompt;
    std::string context;
    AIQueryPriority priority = AIQueryPriority::Normal;
    std::string targetTool;
    float timeoutSeconds = 30.f;

    [[nodiscard]] bool isValid() const { return !queryId.empty() && !prompt.empty(); }
};

struct AIQueryResponse {
    std::string queryId;
    bool success = false;
    std::string result;
    std::string error;
    float processingTimeSec = 0.f;
    std::vector<AIInsight> insights;

    [[nodiscard]] bool hasInsights() const { return !insights.empty(); }
    [[nodiscard]] size_t insightCount() const { return insights.size(); }
};

class AIAnalysisEngine {
public:
    void analyzeEvent(const std::string& eventType, const std::string& path,
                      const std::string& /*metadata*/) {
        AIInsight insight;
        insight.id = "insight_" + std::to_string(m_nextId++);
        insight.timestamp = static_cast<int64_t>(m_nextId);
        insight.affectedPath = path;
        insight.sourceTool = "AIAnalysisEngine";

        if (eventType == "AssetImported") {
            insight.type = AIInsightType::AssetOptimization;
            insight.title = "Asset import analysis";
            insight.description = "Imported asset at " + path + " may benefit from optimization";
            insight.confidence = 0.7f;
            insight.severity = 0.3f;
        } else if (eventType == "ScriptUpdated") {
            insight.type = AIInsightType::CodeQuality;
            insight.title = "Script quality check";
            insight.description = "Updated script at " + path + " should be reviewed";
            insight.confidence = 0.6f;
            insight.severity = 0.4f;
        } else if (eventType == "ContractIssue") {
            insight.type = AIInsightType::SecurityRisk;
            insight.title = "Contract issue detected";
            insight.description = "Contract issue at " + path + " requires attention";
            insight.confidence = 0.9f;
            insight.severity = 0.8f;
        } else {
            insight.type = AIInsightType::General;
            insight.title = "Pipeline event analysis";
            insight.description = "Event " + eventType + " at " + path;
            insight.confidence = 0.5f;
            insight.severity = 0.2f;
        }

        m_insights.push_back(insight);
    }

    [[nodiscard]] const std::vector<AIInsight>& insights() const { return m_insights; }
    [[nodiscard]] size_t insightCount() const { return m_insights.size(); }

    [[nodiscard]] std::vector<AIInsight> actionableInsights() const {
        std::vector<AIInsight> result;
        for (const auto& i : m_insights) {
            if (i.isActionable()) result.push_back(i);
        }
        return result;
    }

    [[nodiscard]] std::vector<AIInsight> insightsByType(AIInsightType type) const {
        std::vector<AIInsight> result;
        for (const auto& i : m_insights) {
            if (i.type == type) result.push_back(i);
        }
        return result;
    }

    void dismissInsight(const std::string& id) {
        for (auto& i : m_insights) {
            if (i.id == id) { i.dismiss(); return; }
        }
    }

    void clearDismissed() {
        m_insights.erase(
            std::remove_if(m_insights.begin(), m_insights.end(),
                [](const AIInsight& i) { return i.dismissed; }),
            m_insights.end());
    }

    void clear() { m_insights.clear(); m_nextId = 1; }

    static constexpr size_t kMaxInsights = 256;

private:
    std::vector<AIInsight> m_insights;
    size_t m_nextId = 1;
};

struct AISuggestion {
    std::string id;
    std::string title;
    std::string description;
    AIInsightType category = AIInsightType::General;
    float priority = 0.f;
    bool accepted = false;
    bool rejected = false;

    [[nodiscard]] bool isPending() const { return !accepted && !rejected; }
    void accept() { accepted = true; }
    void reject() { rejected = true; }
};

class AIProactiveSuggester {
public:
    void generateSuggestions(const AIAnalysisEngine& engine) {
        auto actionable = engine.actionableInsights();
        for (const auto& insight : actionable) {
            if (m_suggestions.size() >= kMaxSuggestions) break;
            bool alreadySuggested = false;
            for (const auto& s : m_suggestions) {
                if (s.title == insight.title) { alreadySuggested = true; break; }
            }
            if (alreadySuggested) continue;

            AISuggestion suggestion;
            suggestion.id = "sug_" + std::to_string(m_nextId++);
            suggestion.title = insight.title;
            suggestion.description = insight.description;
            suggestion.category = insight.type;
            suggestion.priority = insight.severity * insight.confidence;
            m_suggestions.push_back(suggestion);
        }
    }

    [[nodiscard]] const std::vector<AISuggestion>& suggestions() const { return m_suggestions; }
    [[nodiscard]] size_t suggestionCount() const { return m_suggestions.size(); }

    [[nodiscard]] size_t pendingCount() const {
        size_t count = 0;
        for (const auto& s : m_suggestions) {
            if (s.isPending()) ++count;
        }
        return count;
    }

    [[nodiscard]] AISuggestion* findSuggestion(const std::string& id) {
        for (auto& s : m_suggestions) {
            if (s.id == id) return &s;
        }
        return nullptr;
    }

    void acceptSuggestion(const std::string& id) {
        auto* s = findSuggestion(id);
        if (s) s->accept();
    }

    void rejectSuggestion(const std::string& id) {
        auto* s = findSuggestion(id);
        if (s) s->reject();
    }

    void clearResolved() {
        m_suggestions.erase(
            std::remove_if(m_suggestions.begin(), m_suggestions.end(),
                [](const AISuggestion& s) { return !s.isPending(); }),
            m_suggestions.end());
    }

    void clear() { m_suggestions.clear(); m_nextId = 1; }

    static constexpr size_t kMaxSuggestions = 64;

private:
    std::vector<AISuggestion> m_suggestions;
    size_t m_nextId = 1;
};

class AIPipelineBridge {
public:
    explicit AIPipelineBridge(const std::string& pipelineDir = ".atlas/pipeline")
        : m_pipelineDir(pipelineDir) {}

    void processEvent(const std::string& eventType, const std::string& path,
                      const std::string& metadata = "") {
        m_engine.analyzeEvent(eventType, path, metadata);
        m_eventsProcessed++;
        if (m_eventsProcessed % m_suggestionInterval == 0) {
            m_suggester.generateSuggestions(m_engine);
        }
    }

    AIQueryResponse submitQuery(const AIQueryRequest& request) {
        AIQueryResponse response;
        response.queryId = request.queryId;
        if (!request.isValid()) {
            response.success = false;
            response.error = "Invalid query request";
            return response;
        }
        m_queriesProcessed++;
        response.success = true;
        response.result = "Processed query: " + request.prompt;
        response.processingTimeSec = 0.01f;
        m_engine.analyzeEvent("AIAnalysis", request.context, request.prompt);
        response.insights = m_engine.insightsByType(AIInsightType::General);
        return response;
    }

    [[nodiscard]] const AIAnalysisEngine& engine() const { return m_engine; }
    [[nodiscard]] AIAnalysisEngine& engine() { return m_engine; }
    [[nodiscard]] const AIProactiveSuggester& suggester() const { return m_suggester; }
    [[nodiscard]] AIProactiveSuggester& suggester() { return m_suggester; }
    [[nodiscard]] size_t eventsProcessed() const { return m_eventsProcessed; }
    [[nodiscard]] size_t queriesProcessed() const { return m_queriesProcessed; }
    [[nodiscard]] const std::string& pipelineDir() const { return m_pipelineDir; }

    void setSuggestionInterval(size_t interval) { m_suggestionInterval = interval > 0 ? interval : 1; }

private:
    std::string m_pipelineDir;
    AIAnalysisEngine m_engine;
    AIProactiveSuggester m_suggester;
    size_t m_eventsProcessed = 0;
    size_t m_queriesProcessed = 0;
    size_t m_suggestionInterval = 5;
};

struct AtlasAIConfig {
    std::string pipelineDir = ".atlas/pipeline";
    bool proactiveSuggestions = true;
    size_t suggestionInterval = 5;
    float analysisTickRate = 1.f;
    size_t maxInsights = 256;
    size_t maxSuggestions = 64;
};

class AtlasAIIntegration {
public:
    void init(const AtlasAIConfig& config = {}) {
        m_config = config;
        m_bridge = AIPipelineBridge(config.pipelineDir);
        m_bridge.setSuggestionInterval(config.suggestionInterval);
        m_initialized = true;
    }

    void shutdown() {
        m_bridge.engine().clear();
        m_bridge.suggester().clear();
        m_initialized = false;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    void processEvent(const std::string& eventType, const std::string& path,
                      const std::string& metadata = "") {
        if (!m_initialized) return;
        m_bridge.processEvent(eventType, path, metadata);
    }

    AIQueryResponse submitQuery(const AIQueryRequest& request) {
        if (!m_initialized) {
            AIQueryResponse r;
            r.queryId = request.queryId;
            r.success = false;
            r.error = "AtlasAI not initialized";
            return r;
        }
        return m_bridge.submitQuery(request);
    }

    void tick(float dt) {
        if (!m_initialized) return;
        m_tickAccumulator += dt;
        m_tickCount++;
        if (m_config.proactiveSuggestions && m_tickAccumulator >= m_config.analysisTickRate) {
            m_bridge.suggester().generateSuggestions(m_bridge.engine());
            m_tickAccumulator -= m_config.analysisTickRate;
        }
    }

    [[nodiscard]] const AIPipelineBridge& bridge() const { return m_bridge; }
    [[nodiscard]] AIPipelineBridge& bridge() { return m_bridge; }
    [[nodiscard]] const AtlasAIConfig& config() const { return m_config; }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] size_t totalInsights() const { return m_bridge.engine().insightCount(); }
    [[nodiscard]] size_t totalSuggestions() const { return m_bridge.suggester().suggestionCount(); }
    [[nodiscard]] size_t pendingSuggestions() const { return m_bridge.suggester().pendingCount(); }
    [[nodiscard]] size_t totalEvents() const { return m_bridge.eventsProcessed(); }
    [[nodiscard]] size_t totalQueries() const { return m_bridge.queriesProcessed(); }

private:
    AtlasAIConfig m_config;
    AIPipelineBridge m_bridge;
    bool m_initialized = false;
    size_t m_tickCount = 0;
    float m_tickAccumulator = 0.f;
};

// ── S10 — Performance Profiler ───────────────────────────────────


} // namespace NF
