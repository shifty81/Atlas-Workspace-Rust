#include "../editor/panels/TruthUIPanel.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;

void test_truth_ui_name() {
    TruthUIPanel panel;
    assert(std::string(panel.Name()) == "Truth UI");
    std::cout << "[PASS] test_truth_ui_name" << std::endl;
}

void test_truth_ui_defaults() {
    TruthUIPanel panel;
    assert(!panel.IsSimulating());
    assert(!panel.IsPaused());
    assert(!panel.HasDivergence());
    assert(!panel.HasOrderMismatch());
    assert(panel.GetPlayInEditor() == nullptr);
    assert(panel.GetReplayTimeline() == nullptr);
    assert(panel.GetStateHashDiff() == nullptr);
    assert(panel.GetJobTrace() == nullptr);
    std::cout << "[PASS] test_truth_ui_defaults" << std::endl;
}

void test_truth_ui_wiring() {
    TruthUIPanel panel;
    PlayInEditor pie;
    ReplayTimelinePanel timeline;
    StateHashDiffPanel hashDiff;
    JobTracePanel jobTrace;

    panel.SetPlayInEditor(&pie);
    panel.SetReplayTimeline(&timeline);
    panel.SetStateHashDiff(&hashDiff);
    panel.SetJobTrace(&jobTrace);

    assert(panel.GetPlayInEditor() == &pie);
    assert(panel.GetReplayTimeline() == &timeline);
    assert(panel.GetStateHashDiff() == &hashDiff);
    assert(panel.GetJobTrace() == &jobTrace);

    std::cout << "[PASS] test_truth_ui_wiring" << std::endl;
}

void test_truth_ui_summary_stopped() {
    TruthUIPanel panel;
    PlayInEditor pie;
    panel.SetPlayInEditor(&pie);

    std::string summary = panel.Summary();
    assert(summary.find("Stopped") != std::string::npos);
    assert(summary.find("Ticks: 0") != std::string::npos);

    std::cout << "[PASS] test_truth_ui_summary_stopped" << std::endl;
}

void test_truth_ui_summary_no_pie() {
    TruthUIPanel panel;
    std::string summary = panel.Summary();
    assert(summary == "Truth UI");

    std::cout << "[PASS] test_truth_ui_summary_no_pie" << std::endl;
}

void test_truth_ui_draw() {
    TruthUIPanel panel;
    // Draw should not crash even with no wiring
    panel.Draw();
    std::cout << "[PASS] test_truth_ui_draw" << std::endl;
}
