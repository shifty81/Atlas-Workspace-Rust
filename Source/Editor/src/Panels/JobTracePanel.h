#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/sim/JobTracer.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>

namespace atlas::editor {

/// Summary of a single tick for the trace viewer.
struct JobTraceSummary {
    uint64_t tick = 0;
    uint32_t systemCount = 0;
    uint64_t orderHash = 0;
    bool orderMatches = true;  ///< True if order matches reference trace
};

/// Editor panel for visualizing job execution traces.
/// Shows which systems ran in what order each tick and detects
/// non-deterministic execution ordering.
class JobTracePanel : public EditorPanel {
public:
    const char* Name() const override { return "Job Trace"; }
    void Draw() override;

    void SetTracer(const sim::JobTracer* tracer);
    void SetReferenceTracer(const sim::JobTracer* reference);
    void Refresh();

    const std::vector<JobTraceSummary>& Summaries() const;
    bool HasOrderMismatch() const;
    int64_t FirstMismatchTick() const;
    std::string Summary() const;

    /// Get detailed entries for a specific tick.
    std::vector<sim::JobTraceEntry> EntriesAtTick(uint64_t tick) const;

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    const sim::JobTracer* m_tracer = nullptr;
    const sim::JobTracer* m_reference = nullptr;
    std::vector<JobTraceSummary> m_summaries;
    int64_t m_firstMismatch = -1;
    atlas::ui::UIDrawList m_drawList;
};

}  // namespace atlas::editor
