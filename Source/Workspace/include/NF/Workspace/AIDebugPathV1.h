#pragma once
// NF::Editor — AI debug path v1: trace recording, step annotation, decision log, replay
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── AI Debug Step Type ────────────────────────────────────────────

enum class AIDebugStepType : uint8_t {
    Perception,     // sensory input processed
    Consideration,  // utility/decision scoring
    Decision,       // action selected
    Execution,      // action dispatched
    Interrupt,      // higher-priority goal interrupts
    Idle,           // no action taken
};

inline const char* aiDebugStepTypeName(AIDebugStepType t) {
    switch (t) {
        case AIDebugStepType::Perception:   return "Perception";
        case AIDebugStepType::Consideration:return "Consideration";
        case AIDebugStepType::Decision:     return "Decision";
        case AIDebugStepType::Execution:    return "Execution";
        case AIDebugStepType::Interrupt:    return "Interrupt";
        case AIDebugStepType::Idle:         return "Idle";
    }
    return "Unknown";
}

// ── AI Debug Annotation ────────────────────────────────────────────
// Arbitrary string key-value pair attached to a step.

struct AIDebugAnnotation {
    std::string key;
    std::string value;
    float       score = 0.f;  // optional utility score
};

// ── AI Debug Step ──────────────────────────────────────────────────
// Records one step in the AI decision trace.

struct AIDebugStep {
    uint32_t                        id         = 0;
    AIDebugStepType                 type       = AIDebugStepType::Idle;
    std::string                     label;
    std::string                     agentId;
    uint64_t                        tickIndex  = 0;
    float                           wallTimeMs = 0.f;
    std::vector<AIDebugAnnotation>  annotations;

    [[nodiscard]] bool isValid() const { return id != 0 && !agentId.empty(); }

    void addAnnotation(const std::string& key, const std::string& val, float score = 0.f) {
        annotations.push_back({key, val, score});
    }

    [[nodiscard]] const AIDebugAnnotation* findAnnotation(const std::string& key) const {
        for (const auto& a : annotations) if (a.key == key) return &a;
        return nullptr;
    }
};

// ── AI Decision Log Entry ──────────────────────────────────────────

struct AIDecisionLogEntry {
    uint32_t    id       = 0;
    std::string agentId;
    std::string chosenAction;
    float       chosenScore  = 0.f;
    std::string runnerUpAction;
    float       runnerUpScore = 0.f;
    uint64_t    tickIndex    = 0;

    [[nodiscard]] bool isValid() const { return id != 0 && !agentId.empty(); }
    [[nodiscard]] float scoreDelta() const { return chosenScore - runnerUpScore; }
};

// ── AI Debug Path V1 ──────────────────────────────────────────────

using AIStepCallback = std::function<void(const AIDebugStep&)>;

class AIDebugPathV1 {
public:
    static constexpr size_t MAX_STEPS       = 8192;
    static constexpr size_t MAX_DECISION_LOG = 1024;

    void beginRecording(const std::string& agentId) {
        m_recordingAgentId  = agentId;
        m_recording         = true;
        m_currentTickIndex  = 0;
    }

    void endRecording() {
        m_recording = false;
        m_recordingAgentId.clear();
    }

    bool addStep(AIDebugStep step) {
        if (!step.isValid()) return false;
        if (m_steps.size() >= MAX_STEPS) m_steps.erase(m_steps.begin());
        step.tickIndex = m_currentTickIndex;
        m_steps.push_back(step);
        ++m_stepCount;
        if (m_onStep) m_onStep(step);
        return true;
    }

    void tick() {
        ++m_currentTickIndex;
    }

    bool logDecision(const AIDecisionLogEntry& entry) {
        if (!entry.isValid()) return false;
        m_decisionLog.push_back(entry);
        if (m_decisionLog.size() > MAX_DECISION_LOG)
            m_decisionLog.erase(m_decisionLog.begin());
        return true;
    }

    // Replay: iterate steps for a given agent
    [[nodiscard]] std::vector<const AIDebugStep*> replayAgent(const std::string& agentId) const {
        std::vector<const AIDebugStep*> out;
        for (const auto& s : m_steps) if (s.agentId == agentId) out.push_back(&s);
        return out;
    }

    // Filter steps by type
    [[nodiscard]] std::vector<const AIDebugStep*> filterByType(AIDebugStepType type) const {
        std::vector<const AIDebugStep*> out;
        for (const auto& s : m_steps) if (s.type == type) out.push_back(&s);
        return out;
    }

    // Get decision log entries for a given agent
    [[nodiscard]] std::vector<const AIDecisionLogEntry*> decisionsForAgent(const std::string& agentId) const {
        std::vector<const AIDecisionLogEntry*> out;
        for (const auto& e : m_decisionLog) if (e.agentId == agentId) out.push_back(&e);
        return out;
    }

    void clearSteps()      { m_steps.clear(); }
    void clearDecisions()  { m_decisionLog.clear(); }
    void clear()           { clearSteps(); clearDecisions(); m_stepCount = 0; }

    void setOnStep(AIStepCallback cb) { m_onStep = std::move(cb); }

    [[nodiscard]] bool        isRecording()     const { return m_recording;           }
    [[nodiscard]] const std::string& recordingAgentId() const { return m_recordingAgentId; }
    [[nodiscard]] uint64_t    currentTick()     const { return m_currentTickIndex;    }
    [[nodiscard]] size_t      stepCount()       const { return m_stepCount;           }
    [[nodiscard]] size_t      bufferedSteps()   const { return m_steps.size();        }
    [[nodiscard]] size_t      decisionCount()   const { return m_decisionLog.size();  }
    [[nodiscard]] const std::vector<AIDebugStep>& steps() const { return m_steps; }
    [[nodiscard]] const std::vector<AIDecisionLogEntry>& decisionLog() const { return m_decisionLog; }

private:
    std::vector<AIDebugStep>         m_steps;
    std::vector<AIDecisionLogEntry>  m_decisionLog;
    AIStepCallback                   m_onStep;
    std::string                      m_recordingAgentId;
    uint64_t                         m_currentTickIndex = 0;
    size_t                           m_stepCount        = 0;
    bool                             m_recording        = false;
};

} // namespace NF
