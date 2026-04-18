#pragma once
// NF::Workspace — BrokerFlowController: formal broker → reasoner → action flow.
//
// Phase 4 component that wires the WorkspaceBroker (session/context) into the
// AtlasAIReasoner (rule evaluation) and then surfaces violations through the
// AIActionSurface and NotificationSystem. This formalizes the previously
// independent subsystems into a single coherent pipeline.
//
// Flow:
//   PipelineWatcher → BrokerFlowController
//     1. WorkspaceBroker indexes the event and produces an AnalysisResult
//     2. AtlasAIReasoner evaluates rules and produces RuleViolations
//     3. Violations are routed to AIActionSurface as proposed actions
//     4. Critical/error violations are posted to NotificationSystem
//     5. AtlasAIIntegration receives processed events for insight tracking

#include "NF/Pipeline/Pipeline.h"
#include "NF/Workspace/AIActionSurface.h"
#include "NF/Workspace/AIIntegration.h"
#include "NF/Workspace/NotificationSystem.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Flow Stage ────────────────────────────────────────────────────

enum class BrokerFlowStage : uint8_t {
    Idle,
    Indexing,      // broker is indexing the event
    Reasoning,     // reasoner is evaluating rules
    Surfacing,     // violations are being routed to actions/notifications
    Complete,
    Error,
};

inline const char* brokerFlowStageName(BrokerFlowStage s) {
    switch (s) {
        case BrokerFlowStage::Idle:       return "Idle";
        case BrokerFlowStage::Indexing:   return "Indexing";
        case BrokerFlowStage::Reasoning:  return "Reasoning";
        case BrokerFlowStage::Surfacing:  return "Surfacing";
        case BrokerFlowStage::Complete:   return "Complete";
        case BrokerFlowStage::Error:      return "Error";
    }
    return "Unknown";
}

// ── Flow Result ───────────────────────────────────────────────────
// Summarises what a single event flow produced.

struct BrokerFlowResult {
    std::string     eventPath;
    ChangeEventType eventType       = ChangeEventType::Unknown;
    size_t          violationCount  = 0;
    size_t          actionsProposed = 0;
    size_t          notificationsPosted = 0;
    BrokerFlowStage finalStage      = BrokerFlowStage::Idle;
    std::string     summary;
    bool            success         = false;
};

// ── Flow Config ───────────────────────────────────────────────────

struct BrokerFlowConfig {
    bool autoSurfaceActions    = true;   // propose AIActions for violations
    bool autoNotify            = true;   // post notifications for critical issues
    bool feedInsights          = true;   // route to AtlasAIIntegration
    RuleSeverity notifyMinSeverity = RuleSeverity::Error; // min severity for notification
};

// ── Broker Flow Controller ────────────────────────────────────────

class BrokerFlowController {
public:
    // Bind subsystems. All pointers must remain valid for the controller lifetime.
    void bind(WorkspaceBroker*     broker,
              AtlasAIReasoner*     reasoner,
              AIActionSurface*     actionSurface,
              NotificationSystem*  notifications,
              AtlasAIIntegration*  aiIntegration = nullptr) {
        m_broker        = broker;
        m_reasoner      = reasoner;
        m_actionSurface = actionSurface;
        m_notifications = notifications;
        m_aiIntegration = aiIntegration;
        // Ensure our notification channel exists
        if (m_notifications)
            m_notifications->createChannel(m_notifChannel);
    }

    void setConfig(const BrokerFlowConfig& cfg) { m_config = cfg; }
    [[nodiscard]] const BrokerFlowConfig& config() const { return m_config; }

    void setSessionId(const std::string& sessionId) { m_sessionId = sessionId; }
    [[nodiscard]] const std::string& sessionId() const { return m_sessionId; }

    // ── Core flow ─────────────────────────────────────────────────
    // Process a single pipeline event through the full broker flow.

    BrokerFlowResult processEvent(const ChangeEvent& event,
                                  const PipelineDirectories& dirs) {
        BrokerFlowResult result;
        result.eventPath = event.path;
        result.eventType = event.eventType;

        if (!m_broker || !m_reasoner) {
            result.finalStage = BrokerFlowStage::Error;
            result.summary    = "Broker or reasoner not bound";
            return result;
        }

        // Stage 1: Index event in broker
        m_stage = BrokerFlowStage::Indexing;
        AnalysisResult analysis = m_broker->analyzeEvent(m_sessionId, event, dirs);
        if (!analysis.success) {
            result.finalStage = BrokerFlowStage::Error;
            result.summary    = "Broker analysis failed: " + analysis.summary;
            return result;
        }

        // Stage 2: Evaluate rules
        m_stage = BrokerFlowStage::Reasoning;
        auto violations = m_reasoner->evaluate(event);
        result.violationCount = violations.size();

        // Stage 3: Surface violations as actions and notifications
        m_stage = BrokerFlowStage::Surfacing;

        if (m_actionSurface && m_config.autoSurfaceActions) {
            for (const auto& v : violations) {
                std::string label = "[" + std::string(ruleSeverityName(v.severity)) +
                                    "] " + v.description;
                std::string payload = v.suggestion;
                if (payload.empty()) payload = v.description;

                uint32_t id = m_actionSurface->propose(
                    AIActionType::ShowExplanation, label, payload, v.path);
                if (id != 0) ++result.actionsProposed;
            }
        }

        if (m_notifications && m_config.autoNotify) {
            for (const auto& v : violations) {
                if (static_cast<uint8_t>(v.severity) >=
                    static_cast<uint8_t>(m_config.notifyMinSeverity)) {
                    Notification n;
                    n.id       = "broker_" + v.ruleId + "_" + std::to_string(++m_notifSeq);
                    n.title    = "Rule " + v.ruleId + ": " + v.description;
                    n.message  = v.suggestion;
                    n.severity = severityFromRule(v.severity);
                    m_notifications->post(m_notifChannel, n);
                    ++result.notificationsPosted;
                }
            }
        }

        // Stage 4: Feed insights to AtlasAI integration
        if (m_aiIntegration && m_config.feedInsights) {
            m_aiIntegration->processEvent(
                changeEventTypeName(event.eventType), event.path, event.metadata);
        }

        m_stage = BrokerFlowStage::Complete;
        result.finalStage = BrokerFlowStage::Complete;
        result.summary    = analysis.summary;
        result.success    = true;
        ++m_flowsCompleted;
        m_totalViolations += violations.size();
        return result;
    }

    // ── Attach to watcher ─────────────────────────────────────────
    // Subscribe to a PipelineWatcher so events are automatically processed.

    void attachToWatcher(PipelineWatcher& watcher,
                         const PipelineDirectories& dirs) {
        watcher.subscribe([this, dirs](const ChangeEvent& event) {
            if (event.tool == "AtlasAI") return;
            processEvent(event, dirs);
        });
    }

    // ── Statistics ────────────────────────────────────────────────

    [[nodiscard]] BrokerFlowStage currentStage()      const { return m_stage; }
    [[nodiscard]] size_t          flowsCompleted()    const { return m_flowsCompleted; }
    [[nodiscard]] size_t          totalViolations()   const { return m_totalViolations; }

    [[nodiscard]] bool isBound() const {
        return m_broker && m_reasoner && m_actionSurface && m_notifications;
    }

private:
    static NotificationSeverity severityFromRule(RuleSeverity s) {
        switch (s) {
            case RuleSeverity::Info:     return NotificationSeverity::Info;
            case RuleSeverity::Warning:  return NotificationSeverity::Warning;
            case RuleSeverity::Error:    return NotificationSeverity::Error;
            case RuleSeverity::Critical: return NotificationSeverity::Error;
        }
        return NotificationSeverity::Info;
    }

    WorkspaceBroker*     m_broker        = nullptr;
    AtlasAIReasoner*     m_reasoner      = nullptr;
    AIActionSurface*     m_actionSurface = nullptr;
    NotificationSystem*  m_notifications = nullptr;
    AtlasAIIntegration*  m_aiIntegration = nullptr;

    BrokerFlowConfig     m_config;
    std::string          m_sessionId;
    std::string          m_notifChannel = "broker_flow";
    BrokerFlowStage      m_stage           = BrokerFlowStage::Idle;
    size_t               m_flowsCompleted  = 0;
    size_t               m_totalViolations = 0;
    uint32_t             m_notifSeq        = 0;
};

} // namespace NF
