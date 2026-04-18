#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::sim {

/// Records when a system started and ended execution within a tick.
struct JobTraceEntry {
    std::string systemName;
    uint64_t tick = 0;
    uint32_t orderIndex = 0;   ///< Execution order within the tick (0-based)
    double startTimeUs = 0.0;  ///< Relative start time in microseconds
    double durationUs = 0.0;   ///< Duration in microseconds
};

/// Complete trace for a single tick.
struct TickTrace {
    uint64_t tick = 0;
    std::vector<JobTraceEntry> entries;
    uint64_t orderHash = 0;    ///< Hash of the execution order for determinism checks
};

/// Records system execution order and timing each tick.
/// Used for determinism verification: if two runs produce different
/// orderHash values for the same tick, execution is non-deterministic.
class JobTracer {
public:
    /// Start recording a new tick.
    void BeginTick(uint64_t tick);

    /// Record the start of a system's execution within the current tick.
    void RecordSystemStart(const std::string& systemName);

    /// Record the end of the current system's execution.
    void RecordSystemEnd();

    /// Finish the current tick and compute the order hash.
    void EndTick();

    /// The most recent completed tick trace.
    const TickTrace* LatestTrace() const;

    /// Trace at a specific tick (returns nullptr if not in history).
    const TickTrace* TraceAtTick(uint64_t tick) const;

    /// All recorded tick traces.
    const std::vector<TickTrace>& History() const;

    /// Compare execution order between two traces. Returns -1 if identical,
    /// or the index of the first entry that differs.
    static int CompareOrder(const TickTrace& a, const TickTrace& b);

    /// Maximum number of tick traces to retain (default: 120).
    void SetMaxHistory(size_t max);
    size_t MaxHistory() const;

    /// Clear all recorded traces.
    void Clear();

private:
    std::vector<TickTrace> m_history;
    TickTrace m_current;
    uint32_t m_currentOrder = 0;
    bool m_inTick = false;
    bool m_inSystem = false;
    size_t m_maxHistory = 120;
    double m_systemStartTime = 0.0;
};

}  // namespace atlas::sim
