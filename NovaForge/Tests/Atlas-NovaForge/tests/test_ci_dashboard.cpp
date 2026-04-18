#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cmath>

#include "../editor/panels/CIDashboardPanel.h"

void test_ci_dashboard_init() {
    atlas::editor::CIDashboardPanel panel;
    assert(std::string(panel.Name()) == "CIDashboard");
    assert(panel.RunCount() == 0);
    assert(panel.CurrentStatus() == atlas::editor::CIPipelineStatus::Idle);
    std::cout << "[PASS] test_ci_dashboard_init" << std::endl;
}

void test_ci_dashboard_start_run() {
    atlas::editor::CIDashboardPanel panel;
    uint64_t id = panel.StartPipelineRun("abc123", "main");
    assert(id > 0);
    assert(panel.RunCount() == 1);
    assert(panel.CurrentStatus() == atlas::editor::CIPipelineStatus::Running);
    auto* run = panel.CurrentRun();
    assert(run != nullptr);
    assert(run->commitHash == "abc123");
    assert(run->branch == "main");
    std::cout << "[PASS] test_ci_dashboard_start_run" << std::endl;
}

void test_ci_dashboard_add_checks() {
    atlas::editor::CIDashboardPanel panel;
    panel.StartPipelineRun();

    atlas::editor::CICheckResult c1;
    c1.name = "build";
    c1.status = atlas::editor::CICheckStatus::Passed;
    c1.durationSeconds = 1.0;
    panel.AddCheckResult(c1);

    atlas::editor::CICheckResult c2;
    c2.name = "test";
    c2.status = atlas::editor::CICheckStatus::Passed;
    c2.durationSeconds = 2.0;
    panel.AddCheckResult(c2);

    auto checks = panel.CurrentChecks();
    assert(checks.size() == 2);
    std::cout << "[PASS] test_ci_dashboard_add_checks" << std::endl;
}

void test_ci_dashboard_complete_run_passed() {
    atlas::editor::CIDashboardPanel panel;
    panel.StartPipelineRun();

    atlas::editor::CICheckResult c1;
    c1.name = "build";
    c1.status = atlas::editor::CICheckStatus::Passed;
    panel.AddCheckResult(c1);

    atlas::editor::CICheckResult c2;
    c2.name = "test";
    c2.status = atlas::editor::CICheckStatus::Passed;
    panel.AddCheckResult(c2);

    panel.CompletePipelineRun();
    assert(panel.CurrentStatus() == atlas::editor::CIPipelineStatus::Passed);
    std::cout << "[PASS] test_ci_dashboard_complete_run_passed" << std::endl;
}

void test_ci_dashboard_complete_run_failed() {
    atlas::editor::CIDashboardPanel panel;
    panel.StartPipelineRun();

    atlas::editor::CICheckResult c1;
    c1.name = "build";
    c1.status = atlas::editor::CICheckStatus::Failed;
    panel.AddCheckResult(c1);

    atlas::editor::CICheckResult c2;
    c2.name = "test";
    c2.status = atlas::editor::CICheckStatus::Failed;
    panel.AddCheckResult(c2);

    panel.CompletePipelineRun();
    assert(panel.CurrentStatus() == atlas::editor::CIPipelineStatus::Failed);
    std::cout << "[PASS] test_ci_dashboard_complete_run_failed" << std::endl;
}

void test_ci_dashboard_partial_failure() {
    atlas::editor::CIDashboardPanel panel;
    panel.StartPipelineRun();

    atlas::editor::CICheckResult c1;
    c1.name = "build";
    c1.status = atlas::editor::CICheckStatus::Passed;
    panel.AddCheckResult(c1);

    atlas::editor::CICheckResult c2;
    c2.name = "test";
    c2.status = atlas::editor::CICheckStatus::Failed;
    panel.AddCheckResult(c2);

    panel.CompletePipelineRun();
    auto status = panel.CurrentStatus();
    assert(status == atlas::editor::CIPipelineStatus::PartialFailure ||
           status == atlas::editor::CIPipelineStatus::Failed);
    std::cout << "[PASS] test_ci_dashboard_partial_failure" << std::endl;
}

void test_ci_dashboard_pass_rate() {
    atlas::editor::CIDashboardPanel panel;

    // Run 1: passed
    panel.StartPipelineRun();
    atlas::editor::CICheckResult c1;
    c1.name = "build";
    c1.status = atlas::editor::CICheckStatus::Passed;
    panel.AddCheckResult(c1);
    panel.CompletePipelineRun();

    // Run 2: failed
    panel.StartPipelineRun();
    atlas::editor::CICheckResult c2;
    c2.name = "build";
    c2.status = atlas::editor::CICheckStatus::Failed;
    panel.AddCheckResult(c2);
    panel.CompletePipelineRun();

    double rate = panel.PassRate();
    assert(std::fabs(rate - 0.5) < 0.01);
    std::cout << "[PASS] test_ci_dashboard_pass_rate" << std::endl;
}

void test_ci_dashboard_history() {
    atlas::editor::CIDashboardPanel panel;
    panel.StartPipelineRun("a", "main");
    panel.CompletePipelineRun();
    panel.StartPipelineRun("b", "dev");
    panel.CompletePipelineRun();
    panel.StartPipelineRun("c", "main");
    panel.CompletePipelineRun();

    assert(panel.RunCount() == 3);
    const auto& history = panel.RunHistory();
    assert(history.size() == 3);
    assert(history[0].commitHash == "a");
    assert(history[2].commitHash == "c");
    std::cout << "[PASS] test_ci_dashboard_history" << std::endl;
}

void test_ci_dashboard_max_history() {
    atlas::editor::CIDashboardPanel panel;
    panel.SetMaxHistory(3);
    assert(panel.GetMaxHistory() == 3);

    for (int i = 0; i < 5; ++i) {
        panel.StartPipelineRun();
        panel.CompletePipelineRun();
    }
    assert(panel.RunCount() == 3);
    std::cout << "[PASS] test_ci_dashboard_max_history" << std::endl;
}

void test_ci_dashboard_summary() {
    atlas::editor::CIDashboardPanel panel;
    std::string empty = panel.Summary();
    assert(empty.find("no runs") != std::string::npos);

    panel.StartPipelineRun();
    atlas::editor::CICheckResult c1;
    c1.name = "build";
    c1.status = atlas::editor::CICheckStatus::Passed;
    panel.AddCheckResult(c1);
    panel.CompletePipelineRun();

    std::string s = panel.Summary();
    assert(s.find("CI:") != std::string::npos);
    assert(s.find("1/1") != std::string::npos);
    std::cout << "[PASS] test_ci_dashboard_summary" << std::endl;
}

void test_ci_dashboard_clear() {
    atlas::editor::CIDashboardPanel panel;
    panel.StartPipelineRun();
    panel.CompletePipelineRun();
    assert(panel.RunCount() == 1);
    panel.ClearHistory();
    assert(panel.RunCount() == 0);
    std::cout << "[PASS] test_ci_dashboard_clear" << std::endl;
}
