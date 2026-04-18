#include "../engine/sim/ReplayDivergenceInspector.h"
#include <iostream>
#include <cassert>

using namespace atlas::sim;

void test_divergence_no_mismatch() {
    StateHasher a, b;
    a.Reset(42);
    b.Reset(42);

    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4, 5};

    for (uint64_t t = 1; t <= 10; ++t) {
        a.AdvanceTick(t, state, inputs);
        b.AdvanceTick(t, state, inputs);
    }

    auto report = ReplayDivergenceInspector::Compare(a, b);

    assert(report.divergeTick == -1);
    assert(report.totalTicksCompared == 10);
    assert(report.firstMatchingTicks == 10);
    assert(report.severity == DivergenceSeverity::None);
    assert(!ReplayDivergenceInspector::IsCritical(report));

    std::cout << "[PASS] test_divergence_no_mismatch" << std::endl;
}

void test_divergence_hash_mismatch() {
    StateHasher a, b;
    a.Reset(0);
    b.Reset(0);

    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4};

    // Identical for first 5 ticks
    for (uint64_t t = 1; t <= 5; ++t) {
        a.AdvanceTick(t, state, inputs);
        b.AdvanceTick(t, state, inputs);
    }

    // Diverge at tick 6
    a.AdvanceTick(6, state, inputs);
    std::vector<uint8_t> altState = {9, 9, 9};
    b.AdvanceTick(6, altState, inputs);

    auto report = ReplayDivergenceInspector::Compare(a, b);

    assert(report.divergeTick == 6);
    assert(report.localHash != report.remoteHash);
    assert(report.totalTicksCompared == 6);
    assert(report.firstMatchingTicks == 5);
    assert(report.severity != DivergenceSeverity::None);

    std::cout << "[PASS] test_divergence_hash_mismatch" << std::endl;
}

void test_divergence_frame_comparison() {
    std::vector<ReplayFrame> a, b;
    for (uint32_t t = 0; t < 8; ++t) {
        ReplayFrame f;
        f.tick = t;
        f.stateHash = 0xAA00 + t;
        a.push_back(f);
        b.push_back(f);
    }

    // Identical streams
    auto r1 = ReplayDivergenceInspector::CompareFrames(a, b);
    assert(r1.divergeTick == -1);
    assert(r1.totalTicksCompared == 8);
    assert(r1.firstMatchingTicks == 8);

    // Introduce mismatch at index 3 (tick 3)
    b[3].stateHash = 0xFFFF;
    auto r2 = ReplayDivergenceInspector::CompareFrames(a, b);
    assert(r2.divergeTick == 3);
    assert(r2.localHash == 0xAA03);
    assert(r2.remoteHash == 0xFFFF);
    assert(r2.firstMatchingTicks == 3);

    std::cout << "[PASS] test_divergence_frame_comparison" << std::endl;
}

void test_divergence_single_frame_check() {
    ReplayFrame expected;
    expected.tick = 10;
    expected.stateHash = 0xBEEF;

    ReplayFrame good;
    good.tick = 10;
    good.stateHash = 0xBEEF;

    ReplayFrame badHash;
    badHash.tick = 10;
    badHash.stateHash = 0xDEAD;

    ReplayFrame badTick;
    badTick.tick = 11;
    badTick.stateHash = 0xBEEF;

    assert(ReplayDivergenceInspector::CheckFrame(expected, good));
    assert(!ReplayDivergenceInspector::CheckFrame(expected, badHash));
    assert(!ReplayDivergenceInspector::CheckFrame(expected, badTick));

    std::cout << "[PASS] test_divergence_single_frame_check" << std::endl;
}

void test_divergence_report_format() {
    // No-divergence report
    DivergenceReport ok;
    auto okStr = ReplayDivergenceInspector::FormatReport(ok);
    assert(okStr.find("OK") != std::string::npos);

    // Diverged report
    DivergenceReport bad;
    bad.divergeTick = 42;
    bad.localHash = 0x1111;
    bad.remoteHash = 0x2222;
    bad.totalTicksCompared = 100;
    bad.firstMatchingTicks = 42;
    bad.severity = DivergenceSeverity::Warning;

    auto badStr = ReplayDivergenceInspector::FormatReport(bad);
    assert(badStr.find("DIVERGED") != std::string::npos);
    assert(badStr.find("42") != std::string::npos);
    assert(badStr.find("Warning") != std::string::npos);

    // AnalyzePattern produces meaningful text
    auto analysis = ReplayDivergenceInspector::AnalyzePattern(bad);
    assert(analysis.find("tick 42") != std::string::npos);

    auto okAnalysis = ReplayDivergenceInspector::AnalyzePattern(ok);
    assert(okAnalysis.find("No divergence") != std::string::npos);

    std::cout << "[PASS] test_divergence_report_format" << std::endl;
}

void test_divergence_history() {
    ReplayDivergenceInspector inspector;

    assert(inspector.ReportHistory().empty());

    DivergenceReport r1;
    r1.divergeTick = 10;
    inspector.RecordReport(r1);

    assert(inspector.ReportHistory().size() == 1);
    assert(inspector.LastReport().divergeTick == 10);

    DivergenceReport r2;
    r2.divergeTick = 20;
    inspector.RecordReport(r2);

    assert(inspector.ReportHistory().size() == 2);
    assert(inspector.LastReport().divergeTick == 20);

    // Clear
    inspector.ClearHistory();
    assert(inspector.ReportHistory().empty());
    assert(inspector.LastReport().divergeTick == -1);

    // Max reports cap (100)
    for (int i = 0; i < 110; ++i) {
        DivergenceReport r;
        r.divergeTick = i;
        inspector.RecordReport(r);
    }
    assert(inspector.ReportHistory().size() == 100);
    // Oldest entries are dropped; first remaining should be tick 10
    assert(inspector.ReportHistory().front().divergeTick == 10);
    assert(inspector.LastReport().divergeTick == 109);

    std::cout << "[PASS] test_divergence_history" << std::endl;
}

void test_divergence_severity() {
    // No divergence → None
    DivergenceReport none;
    assert(none.severity == DivergenceSeverity::None);
    assert(!ReplayDivergenceInspector::IsCritical(none));

    // Early divergence → Critical (via Compare)
    StateHasher a, b;
    a.Reset(0);
    b.Reset(0);

    std::vector<uint8_t> state = {1};
    std::vector<uint8_t> inputs = {2};
    std::vector<uint8_t> alt = {9};

    // Diverge immediately at tick 1
    a.AdvanceTick(1, state, inputs);
    b.AdvanceTick(1, alt, inputs);

    auto early = ReplayDivergenceInspector::Compare(a, b);
    assert(early.severity == DivergenceSeverity::Critical);
    assert(ReplayDivergenceInspector::IsCritical(early));

    // Late divergence → Warning
    StateHasher c, d;
    c.Reset(0);
    d.Reset(0);

    for (uint64_t t = 1; t <= 9; ++t) {
        c.AdvanceTick(t, state, inputs);
        d.AdvanceTick(t, state, inputs);
    }
    c.AdvanceTick(10, state, inputs);
    d.AdvanceTick(10, alt, inputs);

    auto late = ReplayDivergenceInspector::Compare(c, d);
    assert(late.severity == DivergenceSeverity::Warning);
    assert(!ReplayDivergenceInspector::IsCritical(late));

    std::cout << "[PASS] test_divergence_severity" << std::endl;
}
