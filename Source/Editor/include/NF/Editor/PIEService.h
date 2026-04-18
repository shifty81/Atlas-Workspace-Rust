#pragma once
// NF::PIEService — Play-In-Editor runtime lifecycle manager.
//
// Manages the embedded runtime instance that powers Play-In-Editor (PIE).
// The service controls state transitions: Stopped → Playing → Paused → Stopped.
// It snapshots the editor world on enter and restores it on exit so that
// authoring state is not corrupted by gameplay simulation.
//
// Phase F.1 — Embedded PIE Runtime
//   enter()   — snapshot editor world, start runtime simulation
//   exit()    — destroy runtime, restore editor world from snapshot
//   pause()   — suspend simulation tick
//   resume()  — resume simulation tick
//   step()    — advance exactly one simulation tick while paused
//   reset()   — restart from last snapshot (without full exit+enter)
//
// Phase F.4 — PIE Diagnostics
//   error/warning surfacing via callbacks
//   performance counter access (FPS, entity count, memory)
//   session recording control

#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── PIEState ──────────────────────────────────────────────────────────────────

enum class PIEState : uint8_t {
    Stopped,  ///< No PIE session active
    Playing,  ///< Runtime is running at full speed
    Paused,   ///< Runtime is suspended (step mode)
};

inline const char* pieStateName(PIEState s) {
    switch (s) {
    case PIEState::Stopped: return "Stopped";
    case PIEState::Playing: return "Playing";
    case PIEState::Paused:  return "Paused";
    }
    return "Unknown";
}

// ── PIEDiagnosticEvent ────────────────────────────────────────────────────────

enum class PIEDiagnosticSeverity : uint8_t { Info, Warning, Error, Critical };

inline const char* pieDiagnosticSeverityName(PIEDiagnosticSeverity s) {
    switch (s) {
    case PIEDiagnosticSeverity::Info:     return "Info";
    case PIEDiagnosticSeverity::Warning:  return "Warning";
    case PIEDiagnosticSeverity::Error:    return "Error";
    case PIEDiagnosticSeverity::Critical: return "Critical";
    }
    return "Unknown";
}

struct PIEDiagnosticEvent {
    PIEDiagnosticSeverity severity = PIEDiagnosticSeverity::Info;
    std::string           message;
    std::string           source;   ///< subsystem that generated the event
    uint64_t              tickIndex = 0;
};

// ── PIEPerformanceCounters ────────────────────────────────────────────────────

struct PIEPerformanceCounters {
    float    fps              = 0.f;
    uint32_t entityCount      = 0;
    uint32_t drawCallCount    = 0;
    uint64_t memoryBytes      = 0; ///< peak runtime memory in current session
    uint32_t tickIndex        = 0; ///< simulation ticks since PIE enter
    float    lastFrameMs      = 0.f;
};

// ── PIESessionRecord ─────────────────────────────────────────────────────────

struct PIESessionRecord {
    uint32_t                       sessionId   = 0;
    uint64_t                       enterTimeMs = 0;
    uint64_t                       exitTimeMs  = 0;
    uint32_t                       totalTicks  = 0;
    uint32_t                       errorCount  = 0;
    std::vector<PIEDiagnosticEvent> events;

    [[nodiscard]] bool isRecorded()     const { return exitTimeMs > enterTimeMs; }
    [[nodiscard]] uint64_t durationMs() const {
        return exitTimeMs > enterTimeMs ? exitTimeMs - enterTimeMs : 0;
    }
};

// ── PIEService ────────────────────────────────────────────────────────────────

class PIEService {
public:
    static constexpr uint32_t kMaxDiagnosticHistory = 1024;

    PIEService()  = default;
    ~PIEService() = default;

    // ── Lifecycle ─────────────────────────────────────────────────────────

    /// Enter PIE: snapshot the editor world and start simulation.
    /// No-op if already in Playing or Paused state.
    /// Returns true on success.
    bool enter() {
        if (m_state != PIEState::Stopped) return false;
        m_sessionId++;
        m_currentRecord = PIESessionRecord{};
        m_currentRecord.sessionId   = m_sessionId;
        m_currentRecord.enterTimeMs = m_clockMs;
        m_counters = PIEPerformanceCounters{};
        m_diagnostics.clear();
        m_state = PIEState::Playing;
        if (m_onEnter) m_onEnter();
        return true;
    }

    /// Exit PIE: destroy runtime and restore editor world.
    /// No-op if already Stopped.
    bool exit() {
        if (m_state == PIEState::Stopped) return false;
        m_currentRecord.exitTimeMs  = m_clockMs;
        m_currentRecord.totalTicks  = m_counters.tickIndex;
        m_currentRecord.events      = m_diagnostics;
        m_sessionHistory.push_back(m_currentRecord);
        m_state = PIEState::Stopped;
        if (m_onExit) m_onExit();
        return true;
    }

    /// Pause: suspend the simulation tick.
    bool pause() {
        if (m_state != PIEState::Playing) return false;
        m_state = PIEState::Paused;
        if (m_onPause) m_onPause();
        return true;
    }

    /// Resume from pause.
    bool resume() {
        if (m_state != PIEState::Paused) return false;
        m_state = PIEState::Playing;
        if (m_onResume) m_onResume();
        return true;
    }

    /// Advance exactly one simulation tick while paused.
    bool step() {
        if (m_state != PIEState::Paused) return false;
        ++m_counters.tickIndex;
        if (m_onStep) m_onStep(m_counters.tickIndex);
        return true;
    }

    /// Restart from the last snapshot without a full exit+enter cycle.
    bool reset() {
        if (m_state == PIEState::Stopped) return false;
        m_counters = PIEPerformanceCounters{};
        m_diagnostics.clear();
        m_currentRecord.errorCount = 0;
        m_state = PIEState::Playing;
        if (m_onReset) m_onReset();
        return true;
    }

    // ── State query ───────────────────────────────────────────────────────

    [[nodiscard]] PIEState state()     const { return m_state; }
    [[nodiscard]] bool     isPlaying() const { return m_state == PIEState::Playing; }
    [[nodiscard]] bool     isPaused()  const { return m_state == PIEState::Paused; }
    [[nodiscard]] bool     isStopped() const { return m_state == PIEState::Stopped; }
    [[nodiscard]] uint32_t sessionId() const { return m_sessionId; }

    // ── Performance counters ──────────────────────────────────────────────

    [[nodiscard]] const PIEPerformanceCounters& counters() const { return m_counters; }

    /// Simulate one frame tick (called by the editor frame loop during PIE).
    void tickFrame(float deltaMs, float fps, uint32_t entityCount,
                   uint32_t drawCalls, uint64_t memBytes) {
        if (m_state != PIEState::Playing) return;
        m_counters.tickIndex++;
        m_counters.fps           = fps;
        m_counters.lastFrameMs   = deltaMs;
        m_counters.entityCount   = entityCount;
        m_counters.drawCallCount = drawCalls;
        m_counters.memoryBytes   = memBytes;
    }

    // ── Diagnostics ───────────────────────────────────────────────────────

    void pushDiagnostic(const PIEDiagnosticEvent& evt) {
        if (m_diagnostics.size() >= kMaxDiagnosticHistory) return;
        if (evt.severity == PIEDiagnosticSeverity::Error ||
            evt.severity == PIEDiagnosticSeverity::Critical) {
            ++m_currentRecord.errorCount;
        }
        m_diagnostics.push_back(evt);
        if (m_onDiagnostic) m_onDiagnostic(evt);
    }

    [[nodiscard]] const std::vector<PIEDiagnosticEvent>& diagnostics() const {
        return m_diagnostics;
    }

    [[nodiscard]] uint32_t diagnosticCount() const {
        return static_cast<uint32_t>(m_diagnostics.size());
    }

    [[nodiscard]] uint32_t errorCount() const {
        return m_currentRecord.errorCount;
    }

    [[nodiscard]] uint32_t countBySeverity(PIEDiagnosticSeverity sev) const {
        uint32_t n = 0;
        for (const auto& e : m_diagnostics) if (e.severity == sev) ++n;
        return n;
    }

    // ── Session history ───────────────────────────────────────────────────

    [[nodiscard]] const std::vector<PIESessionRecord>& sessionHistory() const {
        return m_sessionHistory;
    }

    [[nodiscard]] uint32_t sessionCount() const {
        return static_cast<uint32_t>(m_sessionHistory.size());
    }

    // ── Simulated clock (for testing) ─────────────────────────────────────

    void advanceClock(uint64_t ms) { m_clockMs += ms; }
    [[nodiscard]] uint64_t clockMs() const { return m_clockMs; }

    // ── Lifecycle callbacks ───────────────────────────────────────────────

    using SimpleCallback  = std::function<void()>;
    using StepCallback    = std::function<void(uint32_t tick)>;
    using DiagCallback    = std::function<void(const PIEDiagnosticEvent&)>;

    void setOnEnter(SimpleCallback cb)    { m_onEnter    = std::move(cb); }
    void setOnExit(SimpleCallback cb)     { m_onExit     = std::move(cb); }
    void setOnPause(SimpleCallback cb)    { m_onPause    = std::move(cb); }
    void setOnResume(SimpleCallback cb)   { m_onResume   = std::move(cb); }
    void setOnStep(StepCallback cb)       { m_onStep     = std::move(cb); }
    void setOnReset(SimpleCallback cb)    { m_onReset    = std::move(cb); }
    void setOnDiagnostic(DiagCallback cb) { m_onDiagnostic = std::move(cb); }

private:
    PIEState                      m_state   = PIEState::Stopped;
    uint32_t                      m_sessionId = 0;
    uint64_t                      m_clockMs   = 0;
    PIEPerformanceCounters        m_counters;
    PIESessionRecord              m_currentRecord;
    std::vector<PIEDiagnosticEvent> m_diagnostics;
    std::vector<PIESessionRecord>  m_sessionHistory;

    SimpleCallback  m_onEnter;
    SimpleCallback  m_onExit;
    SimpleCallback  m_onPause;
    SimpleCallback  m_onResume;
    StepCallback    m_onStep;
    SimpleCallback  m_onReset;
    DiagCallback    m_onDiagnostic;
};

} // namespace NF
