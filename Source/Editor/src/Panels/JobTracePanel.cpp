#include "JobTracePanel.h"
#include <cstdio>

namespace atlas::editor {

void JobTracePanel::Draw() {
    Refresh();

    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title bar
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 300, 20}, "Job Trace", {220, 220, 220, 255});

    int32_t y = 28;

    if (!m_tracer) {
        m_drawList.DrawText({4, y, 590, 16}, "No tracer attached", {160, 160, 160, 255});
        return;
    }

    if (m_summaries.empty()) {
        m_drawList.DrawText({4, y, 590, 16}, "No traces recorded", {160, 160, 160, 255});
        return;
    }

    // Status line
    std::string status = HasOrderMismatch()
        ? "Order mismatch detected"
        : "Execution order consistent";
    atlas::ui::UIColor statusColor = HasOrderMismatch()
        ? atlas::ui::UIColor{255, 100, 100, 255}
        : atlas::ui::UIColor{100, 200, 100, 255};
    m_drawList.DrawText({4, y, 590, 16}, status, statusColor);
    y += 20;

    // Column header
    m_drawList.DrawRect({0, y, 600, 20}, {45, 45, 45, 255});
    m_drawList.DrawText({4, y + 2, 590, 16}, "Tick       Systems  Hash             Match",
                        {180, 200, 220, 255});
    y += 22;

    // Trace entries
    char buf[128];
    for (const auto& s : m_summaries) {
        atlas::ui::UIColor rowColor = s.orderMatches
            ? atlas::ui::UIColor{200, 200, 200, 255}
            : atlas::ui::UIColor{255, 100, 100, 255};
        std::snprintf(buf, sizeof(buf), "%-10llu %-8u 0x%012llX  %s",
                      (unsigned long long)s.tick,
                      s.systemCount,
                      (unsigned long long)s.orderHash,
                      s.orderMatches ? "OK" : "MISMATCH");
        m_drawList.DrawText({4, y, 590, 16}, buf, rowColor);
        y += 20;
    }
}

void JobTracePanel::SetTracer(const sim::JobTracer* tracer) {
    m_tracer = tracer;
}

void JobTracePanel::SetReferenceTracer(const sim::JobTracer* reference) {
    m_reference = reference;
}

void JobTracePanel::Refresh() {
    m_summaries.clear();
    m_firstMismatch = -1;

    if (!m_tracer) return;

    const auto& history = m_tracer->History();

    for (const auto& trace : history) {
        JobTraceSummary summary;
        summary.tick = trace.tick;
        summary.systemCount = static_cast<uint32_t>(trace.entries.size());
        summary.orderHash = trace.orderHash;
        summary.orderMatches = true;

        // Compare against reference if available
        if (m_reference) {
            const auto* refTrace = m_reference->TraceAtTick(trace.tick);
            if (refTrace) {
                summary.orderMatches = (trace.orderHash == refTrace->orderHash);
            }
        }

        if (!summary.orderMatches && m_firstMismatch < 0) {
            m_firstMismatch = static_cast<int64_t>(trace.tick);
        }

        m_summaries.push_back(summary);
    }
}

const std::vector<JobTraceSummary>& JobTracePanel::Summaries() const {
    return m_summaries;
}

bool JobTracePanel::HasOrderMismatch() const {
    return m_firstMismatch >= 0;
}

int64_t JobTracePanel::FirstMismatchTick() const {
    return m_firstMismatch;
}

std::string JobTracePanel::Summary() const {
    if (!m_tracer) return "No tracer attached";
    if (m_summaries.empty()) return "No traces recorded";
    if (!HasOrderMismatch()) return "Execution order consistent";

    char buf[128];
    std::snprintf(buf, sizeof(buf),
        "Order mismatch at tick %lld",
        (long long)m_firstMismatch);
    return std::string(buf);
}

std::vector<sim::JobTraceEntry> JobTracePanel::EntriesAtTick(uint64_t tick) const {
    if (!m_tracer) return {};
    const auto* trace = m_tracer->TraceAtTick(tick);
    if (!trace) return {};
    return trace->entries;
}

}  // namespace atlas::editor
