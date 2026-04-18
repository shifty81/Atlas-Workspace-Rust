#include "../editor/panels/ProfilerPanel.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::editor;

void test_profiler_empty() {
    ProfilerPanel panel;

    assert(std::string(panel.Name()) == "Profiler");
    assert(panel.History().empty());
    assert(panel.CurrentMetrics().empty());
    assert(panel.FrameCount() == 0);
    assert(panel.AverageFrameTime() == 0.0);
    assert(panel.PeakFrameTime() == 0.0);

    std::cout << "[PASS] test_profiler_empty" << std::endl;
}

void test_profiler_record_frame() {
    ProfilerPanel panel;

    FrameTiming timing;
    timing.frameNumber = 1;
    timing.frameDurationMs = 16.6;
    timing.tickDurationMs = 8.0;
    timing.renderDurationMs = 6.0;
    timing.idleMs = 2.6;

    panel.RecordFrame(timing);
    assert(panel.FrameCount() == 1);
    assert(panel.History()[0].frameNumber == 1);
    assert(std::abs(panel.History()[0].frameDurationMs - 16.6) < 0.01);

    std::cout << "[PASS] test_profiler_record_frame" << std::endl;
}

void test_profiler_average_and_peak() {
    ProfilerPanel panel;

    FrameTiming t1;
    t1.frameDurationMs = 10.0;
    panel.RecordFrame(t1);

    FrameTiming t2;
    t2.frameDurationMs = 20.0;
    panel.RecordFrame(t2);

    FrameTiming t3;
    t3.frameDurationMs = 30.0;
    panel.RecordFrame(t3);

    assert(std::abs(panel.AverageFrameTime() - 20.0) < 0.01);
    assert(std::abs(panel.PeakFrameTime() - 30.0) < 0.01);

    std::cout << "[PASS] test_profiler_average_and_peak" << std::endl;
}

void test_profiler_max_history() {
    ProfilerPanel panel;
    panel.SetMaxHistory(5);
    assert(panel.MaxHistory() == 5);

    for (int i = 0; i < 10; ++i) {
        FrameTiming t;
        t.frameNumber = i;
        t.frameDurationMs = static_cast<double>(i);
        panel.RecordFrame(t);
    }

    assert(panel.FrameCount() == 5);
    assert(panel.History()[0].frameNumber == 5);

    std::cout << "[PASS] test_profiler_max_history" << std::endl;
}

void test_profiler_system_metrics() {
    ProfilerPanel panel;

    SystemMetric m1;
    m1.systemName = "Physics";
    m1.durationMs = 3.5;
    m1.entityCount = 100;

    SystemMetric m2;
    m2.systemName = "AI";
    m2.durationMs = 2.0;
    m2.entityCount = 50;

    panel.RecordSystemMetric(m1);
    panel.RecordSystemMetric(m2);
    assert(panel.CurrentMetrics().size() == 2);
    assert(panel.CurrentMetrics()[0].systemName == "Physics");

    // Recording a new frame clears current metrics
    FrameTiming t;
    t.frameDurationMs = 16.0;
    panel.RecordFrame(t);
    assert(panel.CurrentMetrics().empty());

    std::cout << "[PASS] test_profiler_system_metrics" << std::endl;
}

void test_profiler_pause() {
    ProfilerPanel panel;

    assert(!panel.IsPaused());
    panel.SetPaused(true);
    assert(panel.IsPaused());

    FrameTiming t;
    t.frameDurationMs = 16.0;
    panel.RecordFrame(t);
    assert(panel.FrameCount() == 0);

    SystemMetric m;
    m.systemName = "Test";
    panel.RecordSystemMetric(m);
    assert(panel.CurrentMetrics().empty());

    panel.SetPaused(false);
    panel.RecordFrame(t);
    assert(panel.FrameCount() == 1);

    std::cout << "[PASS] test_profiler_pause" << std::endl;
}

void test_profiler_clear() {
    ProfilerPanel panel;

    FrameTiming t;
    t.frameDurationMs = 16.0;
    panel.RecordFrame(t);

    SystemMetric m;
    m.systemName = "Test";
    panel.RecordSystemMetric(m);

    panel.ClearHistory();
    assert(panel.FrameCount() == 0);
    assert(panel.CurrentMetrics().empty());

    std::cout << "[PASS] test_profiler_clear" << std::endl;
}
