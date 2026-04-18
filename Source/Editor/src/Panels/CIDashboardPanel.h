#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

enum class CICheckStatus {
    Unknown,
    Pending,
    Running,
    Passed,
    Failed,
    Skipped
};

struct CICheckResult {
    std::string name;
    CICheckStatus status = CICheckStatus::Unknown;
    double durationSeconds = 0.0;
    std::string message;
    uint64_t timestamp = 0;
};

enum class CIPipelineStatus {
    Idle,
    Running,
    Passed,
    Failed,
    PartialFailure
};

struct CIPipelineRun {
    uint64_t runId = 0;
    CIPipelineStatus status = CIPipelineStatus::Idle;
    std::vector<CICheckResult> checks;
    double totalDuration = 0.0;
    uint64_t startTimestamp = 0;
    std::string commitHash;
    std::string branch;
};

class CIDashboardPanel : public EditorPanel {
public:
    CIDashboardPanel();
    const char* Name() const override { return "CIDashboard"; }
    void Draw() override;

    // Start a new pipeline run
    uint64_t StartPipelineRun(const std::string& commitHash = "",
                               const std::string& branch = "");

    // Add a check result to the current run
    void AddCheckResult(const CICheckResult& result);

    // Complete the current pipeline run
    void CompletePipelineRun();

    // Get current run status
    CIPipelineStatus CurrentStatus() const;

    // Get current pipeline run
    const CIPipelineRun* CurrentRun() const;

    // Get a specific pipeline run by ID
    const CIPipelineRun* GetRun(uint64_t runId) const;

    // Get all pipeline runs (history)
    const std::vector<CIPipelineRun>& RunHistory() const;

    // Get number of pipeline runs
    size_t RunCount() const;

    // Get pass rate across all runs (0.0 - 1.0)
    double PassRate() const;

    // Get total checks across all runs
    int TotalChecks() const;

    // Get total passed checks
    int TotalPassed() const;

    // Get total failed checks
    int TotalFailed() const;

    // Get checks for current run
    std::vector<CICheckResult> CurrentChecks() const;

    // Get failed checks for current run
    std::vector<CICheckResult> CurrentFailedChecks() const;

    // Clear history
    void ClearHistory();

    // Set max history size
    void SetMaxHistory(size_t max);
    size_t GetMaxHistory() const;

    // Summary string for current state
    std::string Summary() const;

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::vector<CIPipelineRun> m_runs;
    size_t m_maxHistory = 100;
    uint64_t m_nextRunId = 1;
    atlas::ui::UIDrawList m_drawList;

    void UpdatePipelineStatus(CIPipelineRun& run);
};

}
