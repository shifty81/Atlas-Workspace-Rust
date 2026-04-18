#include "../engine/sim/JobTracer.h"
#include "../editor/panels/JobTracePanel.h"
#include <iostream>
#include <cassert>

using namespace atlas::sim;
using namespace atlas::editor;

void test_job_tracer_empty() {
    JobTracer tracer;
    assert(tracer.History().empty());
    assert(tracer.LatestTrace() == nullptr);
    assert(tracer.TraceAtTick(0) == nullptr);
    std::cout << "[PASS] test_job_tracer_empty" << std::endl;
}

void test_job_tracer_single_tick() {
    JobTracer tracer;
    tracer.BeginTick(1);
    tracer.RecordSystemStart("Physics");
    tracer.RecordSystemEnd();
    tracer.RecordSystemStart("AI");
    tracer.RecordSystemEnd();
    tracer.EndTick();

    assert(tracer.History().size() == 1);
    const auto* trace = tracer.LatestTrace();
    assert(trace != nullptr);
    assert(trace->tick == 1);
    assert(trace->entries.size() == 2);
    assert(trace->entries[0].systemName == "Physics");
    assert(trace->entries[0].orderIndex == 0);
    assert(trace->entries[1].systemName == "AI");
    assert(trace->entries[1].orderIndex == 1);
    assert(trace->orderHash != 0);

    std::cout << "[PASS] test_job_tracer_single_tick" << std::endl;
}

void test_job_tracer_multiple_ticks() {
    JobTracer tracer;
    for (uint64_t t = 1; t <= 3; ++t) {
        tracer.BeginTick(t);
        tracer.RecordSystemStart("Physics");
        tracer.RecordSystemEnd();
        tracer.EndTick();
    }

    assert(tracer.History().size() == 3);
    assert(tracer.TraceAtTick(1) != nullptr);
    assert(tracer.TraceAtTick(2) != nullptr);
    assert(tracer.TraceAtTick(3) != nullptr);
    assert(tracer.TraceAtTick(4) == nullptr);

    std::cout << "[PASS] test_job_tracer_multiple_ticks" << std::endl;
}

void test_job_tracer_deterministic_hash() {
    // Two tracers with the same execution order should produce the same hash
    JobTracer tracer1;
    JobTracer tracer2;

    auto runTick = [](JobTracer& t, uint64_t tick) {
        t.BeginTick(tick);
        t.RecordSystemStart("Physics");
        t.RecordSystemEnd();
        t.RecordSystemStart("AI");
        t.RecordSystemEnd();
        t.RecordSystemStart("Gameplay");
        t.RecordSystemEnd();
        t.EndTick();
    };

    runTick(tracer1, 1);
    runTick(tracer2, 1);

    assert(tracer1.LatestTrace()->orderHash == tracer2.LatestTrace()->orderHash);

    std::cout << "[PASS] test_job_tracer_deterministic_hash" << std::endl;
}

void test_job_tracer_different_order_different_hash() {
    JobTracer tracer1;
    JobTracer tracer2;

    // tracer1: Physics then AI
    tracer1.BeginTick(1);
    tracer1.RecordSystemStart("Physics");
    tracer1.RecordSystemEnd();
    tracer1.RecordSystemStart("AI");
    tracer1.RecordSystemEnd();
    tracer1.EndTick();

    // tracer2: AI then Physics (different order)
    tracer2.BeginTick(1);
    tracer2.RecordSystemStart("AI");
    tracer2.RecordSystemEnd();
    tracer2.RecordSystemStart("Physics");
    tracer2.RecordSystemEnd();
    tracer2.EndTick();

    assert(tracer1.LatestTrace()->orderHash != tracer2.LatestTrace()->orderHash);

    std::cout << "[PASS] test_job_tracer_different_order_different_hash" << std::endl;
}

void test_job_tracer_compare_order() {
    TickTrace a, b;
    a.entries.push_back({"Physics", 1, 0, 0, 0});
    a.entries.push_back({"AI", 1, 1, 0, 0});

    b.entries.push_back({"Physics", 1, 0, 0, 0});
    b.entries.push_back({"AI", 1, 1, 0, 0});

    assert(JobTracer::CompareOrder(a, b) == -1);

    // Change order in b
    b.entries[1].systemName = "Gameplay";
    assert(JobTracer::CompareOrder(a, b) == 1);

    std::cout << "[PASS] test_job_tracer_compare_order" << std::endl;
}

void test_job_tracer_max_history() {
    JobTracer tracer;
    tracer.SetMaxHistory(5);
    assert(tracer.MaxHistory() == 5);

    for (uint64_t t = 1; t <= 10; ++t) {
        tracer.BeginTick(t);
        tracer.RecordSystemStart("Sys");
        tracer.RecordSystemEnd();
        tracer.EndTick();
    }

    assert(tracer.History().size() == 5);
    assert(tracer.TraceAtTick(6) != nullptr);
    assert(tracer.TraceAtTick(5) == nullptr); // pruned

    std::cout << "[PASS] test_job_tracer_max_history" << std::endl;
}

void test_job_tracer_clear() {
    JobTracer tracer;
    tracer.BeginTick(1);
    tracer.RecordSystemStart("Sys");
    tracer.RecordSystemEnd();
    tracer.EndTick();

    tracer.Clear();
    assert(tracer.History().empty());
    assert(tracer.LatestTrace() == nullptr);

    std::cout << "[PASS] test_job_tracer_clear" << std::endl;
}

// --- Panel tests ---

void test_job_trace_panel_no_tracer() {
    JobTracePanel panel;
    panel.Refresh();
    assert(panel.Summaries().empty());
    assert(!panel.HasOrderMismatch());
    assert(panel.Summary() == "No tracer attached");

    std::cout << "[PASS] test_job_trace_panel_no_tracer" << std::endl;
}

void test_job_trace_panel_consistent() {
    JobTracer tracer;
    for (uint64_t t = 1; t <= 3; ++t) {
        tracer.BeginTick(t);
        tracer.RecordSystemStart("Physics");
        tracer.RecordSystemEnd();
        tracer.EndTick();
    }

    JobTracePanel panel;
    panel.SetTracer(&tracer);
    panel.Refresh();

    assert(panel.Summaries().size() == 3);
    assert(!panel.HasOrderMismatch());
    assert(panel.Summary() == "Execution order consistent");

    std::cout << "[PASS] test_job_trace_panel_consistent" << std::endl;
}

void test_job_trace_panel_mismatch() {
    JobTracer tracer;
    JobTracer reference;

    // Same order at tick 1
    tracer.BeginTick(1);
    tracer.RecordSystemStart("Physics");
    tracer.RecordSystemEnd();
    tracer.EndTick();

    reference.BeginTick(1);
    reference.RecordSystemStart("Physics");
    reference.RecordSystemEnd();
    reference.EndTick();

    // Different order at tick 2
    tracer.BeginTick(2);
    tracer.RecordSystemStart("AI");
    tracer.RecordSystemEnd();
    tracer.EndTick();

    reference.BeginTick(2);
    reference.RecordSystemStart("Physics");
    reference.RecordSystemEnd();
    reference.EndTick();

    JobTracePanel panel;
    panel.SetTracer(&tracer);
    panel.SetReferenceTracer(&reference);
    panel.Refresh();

    assert(panel.HasOrderMismatch());
    assert(panel.FirstMismatchTick() == 2);
    std::string summary = panel.Summary();
    assert(summary.find("mismatch") != std::string::npos);
    assert(summary.find("2") != std::string::npos);

    std::cout << "[PASS] test_job_trace_panel_mismatch" << std::endl;
}

void test_job_trace_panel_entries_at_tick() {
    JobTracer tracer;
    tracer.BeginTick(5);
    tracer.RecordSystemStart("Physics");
    tracer.RecordSystemEnd();
    tracer.RecordSystemStart("AI");
    tracer.RecordSystemEnd();
    tracer.EndTick();

    JobTracePanel panel;
    panel.SetTracer(&tracer);

    auto entries = panel.EntriesAtTick(5);
    assert(entries.size() == 2);
    assert(entries[0].systemName == "Physics");
    assert(entries[1].systemName == "AI");

    auto empty = panel.EntriesAtTick(99);
    assert(empty.empty());

    std::cout << "[PASS] test_job_trace_panel_entries_at_tick" << std::endl;
}
