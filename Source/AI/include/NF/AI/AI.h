#pragma once
// NF::AI — Generic behavior AI, memory, utility scoring, agents,
//          tool services, editor AI assistant (project-agnostic)
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace NF {

// ── AIBehavior ───────────────────────────────────────────────────

enum class AIBehavior : uint8_t {
    Idle,
    Patrol,
    Attack,
    Flee,
    Explore
};

// ── AIMemory ─────────────────────────────────────────────────────

struct AIMemory {
    float decayRate = 0.01f;
    float threat = 0.f;
    float friendliness = 0.5f;
};

// ── AI Blackboard ────────────────────────────────────────────────

using BlackboardValue = std::variant<int32_t, float, bool, std::string>;

class Blackboard {
public:
    void set(const std::string& key, BlackboardValue val) {
        m_entries[key] = std::move(val);
    }

    [[nodiscard]] const BlackboardValue* get(const std::string& key) const {
        auto it = m_entries.find(key);
        return (it != m_entries.end()) ? &it->second : nullptr;
    }

    [[nodiscard]] bool has(const std::string& key) const {
        return m_entries.count(key) > 0;
    }

    void remove(const std::string& key) {
        m_entries.erase(key);
    }

    void clear() { m_entries.clear(); }

    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

private:
    std::unordered_map<std::string, BlackboardValue> m_entries;
};

// ── Utility AI ───────────────────────────────────────────────────

struct UtilityAction {
    std::string            name;
    float                  score    = 0.f;
    std::function<void()>  callback;
};

class UtilitySelector {
public:
    void addAction(UtilityAction action) {
        m_actions.push_back(std::move(action));
    }

    [[nodiscard]] const UtilityAction* evaluate() const {
        if (m_actions.empty()) return nullptr;
        const UtilityAction* best = &m_actions[0];
        for (size_t i = 1; i < m_actions.size(); ++i) {
            if (m_actions[i].score > best->score) best = &m_actions[i];
        }
        return best;
    }

    [[nodiscard]] size_t actionCount() const { return m_actions.size(); }

    void clear() { m_actions.clear(); }

private:
    std::vector<UtilityAction> m_actions;
};

enum class UtilityCurve : uint8_t {
    Linear,
    Quadratic,
    Logistic,
    Inverse
};

inline float evaluateCurve(UtilityCurve curve, float input, float weight) {
    switch (curve) {
        case UtilityCurve::Linear:    return input * weight;
        case UtilityCurve::Quadratic: return input * input * weight;
        case UtilityCurve::Logistic:  return weight / (1.f + std::exp(-10.f * (input - 0.5f)));
        case UtilityCurve::Inverse: {
            float denom = input + 0.01f;
            return weight / denom;
        }
    }
    return 0.f;
}

// ── AI Agent ─────────────────────────────────────────────────────

class AIAgent {
public:
    explicit AIAgent(EntityID entityId = INVALID_ENTITY)
        : m_entityId(entityId) {}

    void setBehavior(AIBehavior b) { m_behavior = b; }
    [[nodiscard]] AIBehavior currentBehavior() const { return m_behavior; }

    [[nodiscard]] EntityID entityId() const { return m_entityId; }

    AIMemory&       memory()       { return m_memory; }
    [[nodiscard]] const AIMemory& memory() const { return m_memory; }

    void setBlackboard(Blackboard* bb) { m_blackboard = bb; }
    [[nodiscard]] Blackboard* blackboard() const { return m_blackboard; }

    void update(float dt) {
        // Decay threat over time
        m_memory.threat -= m_memory.decayRate * dt;
        if (m_memory.threat < 0.f) m_memory.threat = 0.f;
    }

private:
    EntityID           m_entityId   = INVALID_ENTITY;
    AIBehavior         m_behavior   = AIBehavior::Idle;
    AIMemory           m_memory;
    Blackboard*        m_blackboard = nullptr;
};

// ── AISystem ─────────────────────────────────────────────────────

class AISystem {
public:
    void init() { NF_LOG_INFO("AI", "AI system initialized"); }

    void shutdown() {
        m_agents.clear();
        NF_LOG_INFO("AI", "AI system shutdown");
    }

    void update(float dt) {
        for (auto& agent : m_agents) {
            agent.update(dt);
        }
    }

    void registerAgent(const AIAgent& agent) {
        m_agents.push_back(agent);
    }

    void unregisterAgent(EntityID id) {
        m_agents.erase(
            std::remove_if(m_agents.begin(), m_agents.end(),
                           [id](const AIAgent& a) { return a.entityId() == id; }),
            m_agents.end());
    }

    [[nodiscard]] AIAgent* findAgent(EntityID id) {
        for (auto& a : m_agents) {
            if (a.entityId() == id) return &a;
        }
        return nullptr;
    }

    [[nodiscard]] size_t agentCount() const { return m_agents.size(); }

private:
    std::vector<AIAgent> m_agents;
};

// ── ITool Interface ──────────────────────────────────────────────

class ITool {
public:
    virtual ~ITool() = default;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::string description() const = 0;
    virtual void init() = 0;
    virtual void shutdown() = 0;
    [[nodiscard]] virtual bool isInitialized() const = 0;
};

// ── AtlasAIQueryTool ────────────────────────────────────────────

class AtlasAIQueryTool : public ITool {
public:
    [[nodiscard]] std::string name() const override { return "AtlasAI.Query"; }
    [[nodiscard]] std::string description() const override { return "Multi-purpose AI assistant agent"; }

    void init() override {
        m_initialized = true;
        NF_LOG_INFO("AI", "AtlasAIQueryTool initialized");
    }

    void shutdown() override {
        m_initialized = false;
        NF_LOG_INFO("AI", "AtlasAIQueryTool shutdown");
    }

    [[nodiscard]] bool isInitialized() const override { return m_initialized; }

    [[nodiscard]] std::string processQuery(const std::string& query) const {
        if (!m_initialized) return "";
        return "AtlasAI response to: " + query;
    }

private:
    bool m_initialized = false;
};

// ── AtlasAIRulesTool ────────────────────────────────────────────

class AtlasAIRulesTool : public ITool {
public:
    [[nodiscard]] std::string name() const override { return "AtlasAI.Rules"; }
    [[nodiscard]] std::string description() const override { return "Rule-based AI evaluation engine"; }

    void init() override {
        m_initialized = true;
        NF_LOG_INFO("AI", "AtlasAIRulesTool initialized");
    }

    void shutdown() override {
        m_initialized = false;
        m_rules.clear();
        NF_LOG_INFO("AI", "AtlasAIRulesTool shutdown");
    }

    [[nodiscard]] bool isInitialized() const override { return m_initialized; }

    [[nodiscard]] JsonValue evaluate(const JsonValue& context) const {
        if (!m_initialized) return JsonValue();
        auto result = JsonValue::object();
        result.set("status", JsonValue("evaluated"));
        result.set("ruleCount", JsonValue(static_cast<int32_t>(m_rules.size())));
        result.set("input", context);
        return result;
    }

    void addRule(const std::string& ruleName, const std::string& condition,
                 const std::string& action) {
        Rule r;
        r.name      = ruleName;
        r.condition  = condition;
        r.action     = action;
        m_rules.push_back(std::move(r));
    }

    [[nodiscard]] size_t ruleCount() const { return m_rules.size(); }

private:
    struct Rule {
        std::string name;
        std::string condition;
        std::string action;
    };

    bool m_initialized = false;
    std::vector<Rule> m_rules;
};

// ── BlenderGeneratorTool ─────────────────────────────────────────

class BlenderGeneratorTool : public ITool {
public:
    [[nodiscard]] std::string name() const override { return "BlenderGenerator"; }
    [[nodiscard]] std::string description() const override { return "Procedural mesh generation tool"; }

    void init() override {
        m_initialized = true;
        NF_LOG_INFO("AI", "BlenderGeneratorTool initialized");
    }

    void shutdown() override {
        m_initialized = false;
        NF_LOG_INFO("AI", "BlenderGeneratorTool shutdown");
    }

    [[nodiscard]] bool isInitialized() const override { return m_initialized; }

    [[nodiscard]] JsonValue generateMesh(const std::string& type,
                                         const JsonValue& params) const {
        if (!m_initialized) return JsonValue();
        auto mesh = JsonValue::object();
        mesh.set("type", JsonValue(type));
        mesh.set("params", params);
        mesh.set("vertices", JsonValue(0));
        mesh.set("generated", JsonValue(true));
        return mesh;
    }

    [[nodiscard]] std::vector<std::string> supportedTypes() const {
        return {"cube", "sphere", "cylinder", "plane", "torus"};
    }

private:
    bool m_initialized = false;
};

// ── ContractScannerTool ──────────────────────────────────────────

class ContractScannerTool : public ITool {
public:
    [[nodiscard]] std::string name() const override { return "ContractScanner"; }
    [[nodiscard]] std::string description() const override { return "Code contract and invariant scanner"; }

    void init() override {
        m_initialized = true;
        m_issues.clear();
        NF_LOG_INFO("AI", "ContractScannerTool initialized");
    }

    void shutdown() override {
        m_initialized = false;
        m_issues.clear();
        NF_LOG_INFO("AI", "ContractScannerTool shutdown");
    }

    [[nodiscard]] bool isInitialized() const override { return m_initialized; }

    std::vector<std::string> scan(const std::string& code) {
        m_issues.clear();
        if (!m_initialized) return m_issues;
        if (code.empty()) {
            m_issues.push_back("empty input");
        }
        return m_issues;
    }

    [[nodiscard]] size_t issueCount() const { return m_issues.size(); }

private:
    bool m_initialized = false;
    std::vector<std::string> m_issues;
};

// ── ReplayMinimizerTool ──────────────────────────────────────────

class ReplayMinimizerTool : public ITool {
public:
    [[nodiscard]] std::string name() const override { return "ReplayMinimizer"; }
    [[nodiscard]] std::string description() const override { return "Replay trace minimization tool"; }

    void init() override {
        m_initialized = true;
        m_compressionRatio = 1.f;
        NF_LOG_INFO("AI", "ReplayMinimizerTool initialized");
    }

    void shutdown() override {
        m_initialized = false;
        m_compressionRatio = 1.f;
        NF_LOG_INFO("AI", "ReplayMinimizerTool shutdown");
    }

    [[nodiscard]] bool isInitialized() const override { return m_initialized; }

    std::vector<JsonValue> minimize(const std::vector<JsonValue>& frames) {
        if (!m_initialized || frames.empty()) {
            m_compressionRatio = 1.f;
            return frames;
        }
        // Simple minimization: keep only every other frame
        std::vector<JsonValue> result;
        for (size_t i = 0; i < frames.size(); i += 2) {
            result.push_back(frames[i]);
        }
        m_compressionRatio = static_cast<float>(result.size())
                           / static_cast<float>(frames.size());
        return result;
    }

    [[nodiscard]] float compressionRatio() const { return m_compressionRatio; }

private:
    bool  m_initialized     = false;
    float m_compressionRatio = 1.f;
};

// ── ToolRegistry ─────────────────────────────────────────────────

class ToolRegistry {
public:
    void registerTool(std::unique_ptr<ITool> tool) {
        if (tool) {
            NF_LOG_INFO("AI", "Registered tool: " + tool->name());
            m_tools.push_back(std::move(tool));
        }
    }

    [[nodiscard]] ITool* findTool(const std::string& toolName) const {
        for (auto& t : m_tools) {
            if (t->name() == toolName) return t.get();
        }
        return nullptr;
    }

    [[nodiscard]] size_t toolCount() const { return m_tools.size(); }

    [[nodiscard]] std::vector<std::string> allToolNames() const {
        std::vector<std::string> names;
        names.reserve(m_tools.size());
        for (auto& t : m_tools) names.push_back(t->name());
        return names;
    }

private:
    std::vector<std::unique_ptr<ITool>> m_tools;
};

// ── EditorAIAssistant ────────────────────────────────────────────

class EditorAIAssistant {
public:
    void setToolRegistry(ToolRegistry* reg) { m_registry = reg; }
    [[nodiscard]] ToolRegistry* toolRegistry() const { return m_registry; }

    [[nodiscard]] std::string query(const std::string& input) const {
        if (!isAvailable()) return "";
        return "AI assistant response to: " + input;
    }

    [[nodiscard]] bool isAvailable() const { return m_registry != nullptr; }

    [[nodiscard]] size_t registeredToolCount() const {
        return m_registry ? m_registry->toolCount() : 0;
    }

private:
    ToolRegistry* m_registry = nullptr;
};

} // namespace NF
