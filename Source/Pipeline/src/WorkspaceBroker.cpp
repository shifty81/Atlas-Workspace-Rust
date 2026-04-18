// NF::Pipeline — S3 AtlasAI Broker (WorkspaceBroker)
//
// Implements WorkspaceBroker: the AI workspace broker that manages
// sessions, context indexing, and analysis routing for AtlasAI.
// All communication uses the .atlas/pipeline/ directory.

#include "NF/Pipeline/Pipeline.h"

#include <algorithm>
#include <chrono>

namespace NF {

namespace {

static int64_t nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
               system_clock::now().time_since_epoch()).count();
}

} // anonymous namespace

// ── WorkspaceBroker ───────────────────────────────────────────────────────

WorkspaceBroker::WorkspaceBroker() = default;

std::string WorkspaceBroker::generateSessionId() {
    static std::atomic<uint32_t> counter{0};
    int64_t ms  = nowMs();
    uint32_t seq = counter.fetch_add(1);
    return "sa-" + std::to_string(ms) + "-" + std::to_string(seq);
}

// ── SA-1: Session management ──────────────────────────────────────────────

std::string WorkspaceBroker::createSession(const std::string& projectName) {
    BrokerSession session;
    session.id           = generateSessionId();
    session.projectName  = projectName;
    session.createdAt    = nowMs();
    session.lastActiveAt = session.createdAt;
    session.active       = true;

    std::string id = session.id;
    m_sessions[id] = std::move(session);
    return id;
}

bool WorkspaceBroker::resumeSession(const std::string& sessionId) {
    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end()) return false;
    it->second.active       = true;
    it->second.lastActiveAt = nowMs();
    return true;
}

bool WorkspaceBroker::closeSession(const std::string& sessionId) {
    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end()) return false;
    it->second.active       = false;
    it->second.lastActiveAt = nowMs();
    return true;
}

const BrokerSession* WorkspaceBroker::session(const std::string& sessionId) const {
    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end()) return nullptr;
    return &it->second;
}

std::vector<std::string> WorkspaceBroker::activeSessions() const {
    std::vector<std::string> result;
    for (const auto& [id, s] : m_sessions) {
        if (s.active) result.push_back(id);
    }
    return result;
}

// ── SA-2: Context indexing ────────────────────────────────────────────────

void WorkspaceBroker::indexEvent(const std::string& sessionId,
                                  const ChangeEvent& event) {
    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end()) return;

    // Track in session.
    AnalysisRequest req;
    req.event       = event;
    req.sessionId   = sessionId;
    req.requestTime = nowMs();
    it->second.requests.push_back(std::move(req));
    it->second.lastActiveAt = nowMs();

    // Track in file index.
    m_fileIndex[event.path].events.push_back({event.eventType, event.timestamp});
    ++m_totalEventsIndexed;
}

size_t WorkspaceBroker::eventCountForPath(const std::string& path) const {
    auto it = m_fileIndex.find(path);
    if (it == m_fileIndex.end()) return 0;
    return it->second.events.size();
}

ChangeEventType WorkspaceBroker::lastEventTypeForPath(const std::string& path) const {
    auto it = m_fileIndex.find(path);
    if (it == m_fileIndex.end()) return ChangeEventType::Unknown;
    if (it->second.events.empty()) return ChangeEventType::Unknown;
    return it->second.events.back().first;
}

std::vector<std::string> WorkspaceBroker::trackedPaths() const {
    std::vector<std::string> result;
    result.reserve(m_fileIndex.size());
    for (const auto& [path, _] : m_fileIndex) {
        result.push_back(path);
    }
    std::sort(result.begin(), result.end());
    return result;
}

// ── SA-3: Analysis requests ───────────────────────────────────────────────

std::string WorkspaceBroker::generateSummary(const ChangeEvent& event) const {
    // Build a human-readable summary describing what happened.
    std::string summary = "Event: ";
    summary += changeEventTypeName(event.eventType);
    summary += " from ";
    summary += event.tool.empty() ? "unknown" : event.tool;
    summary += " on '";
    summary += event.path;
    summary += "'";
    if (!event.metadata.empty()) {
        summary += " [";
        summary += event.metadata;
        summary += "]";
    }
    return summary;
}

std::string WorkspaceBroker::generateRecommendation(const ChangeEvent& event) const {
    // Generate context-aware recommendations based on event type.
    switch (event.eventType) {
        case ChangeEventType::AssetImported:
            return "Verify asset in Content Browser; check LOD levels and material slots.";
        case ChangeEventType::AnimationExported:
            return "Preview animation in viewport; validate bone mapping against skeleton.";
        case ChangeEventType::ScriptUpdated:
            return "Run ContractScanner to check for violations; review logic graph.";
        case ChangeEventType::WorldChanged:
            return "Review world snapshot diff; check entity placement bounds.";
        case ChangeEventType::ContractIssue:
            return "Review contract violation details; consider AtlasAI for resolution.";
        case ChangeEventType::ReplayExported:
            return "Run ReplayMinimizer to reduce file size; archive for regression tests.";
        case ChangeEventType::AIAnalysis:
            return "Review AI analysis; apply recommendations if applicable.";
        default:
            return {};
    }
}

AnalysisResult WorkspaceBroker::analyzeEvent(const std::string& sessionId,
                                              const ChangeEvent& event,
                                              const PipelineDirectories& dirs) {
    AnalysisResult result;
    result.sessionId       = sessionId;
    result.requestPath     = event.path;
    result.sourceEventType = event.eventType;
    result.analysisTime    = nowMs();

    auto it = m_sessions.find(sessionId);
    if (it == m_sessions.end()) {
        result.success = false;
        result.summary = "Session not found: " + sessionId;
        return result;
    }

    // Index the event.
    indexEvent(sessionId, event);

    // Generate analysis.
    result.summary        = generateSummary(event);
    result.recommendation = generateRecommendation(event);
    result.success        = true;

    // Record in session.
    it->second.results.push_back(result);
    ++m_totalAnalyses;

    // Emit an AIAnalysis response event back into the pipeline.
    ChangeEvent response;
    response.tool      = "AtlasAI";
    response.eventType = ChangeEventType::AIAnalysis;
    response.path      = event.path;
    response.timestamp = result.analysisTime;
    response.metadata  = "session=" + sessionId +
                         ";summary=" + result.summary;
    response.writeToFile(dirs.changes);

    return result;
}

// ── SA-5: Full pipeline integration ───────────────────────────────────────

void WorkspaceBroker::attachToWatcher(PipelineWatcher& watcher,
                                       const std::string& sessionId,
                                       const PipelineDirectories& dirs) {
    // Subscribe: every incoming event gets indexed and analyzed.
    watcher.subscribe([this, sessionId, dirs](const ChangeEvent& event) {
        // Skip events emitted by AtlasAI itself to avoid feedback loops.
        if (event.tool == "AtlasAI") return;
        analyzeEvent(sessionId, event, dirs);
    });
}

} // namespace NF
