// Phase 18: TickStepDebugger, SimulationStateAuditor, FPDriftDetector tests

#include "../engine/sim/TickStepDebugger.h"
#include "../engine/sim/SimulationStateAuditor.h"
#include "../engine/sim/FPDriftDetector.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace atlas::sim;

// =============================================================
// TickStepDebugger Tests
// =============================================================

void test_p18_tick_debugger_initial_state() {
    TickStepDebugger dbg;
    assert(dbg.CurrentTick() == 0);
    assert(!dbg.IsPaused());
    assert(dbg.BreakpointCount() == 0);
    assert(dbg.TriggeredBreakpointId() == 0);
    std::cout << "  [PASS] test_tick_debugger_initial_state" << std::endl;
}

void test_p18_tick_debugger_step_forward() {
    TickStepDebugger dbg;
    dbg.StepForward();
    assert(dbg.CurrentTick() == 1);
    dbg.StepForward(5);
    assert(dbg.CurrentTick() == 6);
    std::cout << "  [PASS] test_tick_debugger_step_forward" << std::endl;
}

void test_p18_tick_debugger_step_backward() {
    TickStepDebugger dbg;
    dbg.SetCurrentTick(10);
    dbg.StepBackward(3);
    assert(dbg.CurrentTick() == 7);
    // Should not go below 0
    dbg.StepBackward(100);
    assert(dbg.CurrentTick() == 0);
    std::cout << "  [PASS] test_tick_debugger_step_backward" << std::endl;
}

void test_p18_tick_debugger_jump_to_tick() {
    TickStepDebugger dbg;
    dbg.JumpToTick(42);
    assert(dbg.CurrentTick() == 42);
    dbg.JumpToTick(0);
    assert(dbg.CurrentTick() == 0);
    std::cout << "  [PASS] test_tick_debugger_jump_to_tick" << std::endl;
}

void test_p18_tick_debugger_pause() {
    TickStepDebugger dbg;
    assert(!dbg.IsPaused());
    dbg.SetPaused(true);
    assert(dbg.IsPaused());
    dbg.SetPaused(false);
    assert(!dbg.IsPaused());
    std::cout << "  [PASS] test_tick_debugger_pause" << std::endl;
}

void test_p18_tick_debugger_add_breakpoint() {
    TickStepDebugger dbg;
    TickBreakpoint bp;
    bp.tick = 100;
    bp.label = "test_bp";
    uint32_t id = dbg.AddBreakpoint(bp);
    assert(id > 0);
    assert(dbg.BreakpointCount() == 1);
    assert(dbg.Breakpoints()[0].id == id);
    assert(dbg.Breakpoints()[0].tick == 100);
    assert(dbg.Breakpoints()[0].label == "test_bp");
    std::cout << "  [PASS] test_tick_debugger_add_breakpoint" << std::endl;
}

void test_p18_tick_debugger_remove_breakpoint() {
    TickStepDebugger dbg;
    TickBreakpoint bp;
    bp.tick = 50;
    uint32_t id = dbg.AddBreakpoint(bp);
    assert(dbg.BreakpointCount() == 1);
    bool removed = dbg.RemoveBreakpoint(id);
    assert(removed);
    assert(dbg.BreakpointCount() == 0);
    // Removing non-existent breakpoint
    assert(!dbg.RemoveBreakpoint(999));
    std::cout << "  [PASS] test_tick_debugger_remove_breakpoint" << std::endl;
}

void test_p18_tick_debugger_enable_disable_breakpoint() {
    TickStepDebugger dbg;
    TickBreakpoint bp;
    bp.tick = 100;
    uint32_t id = dbg.AddBreakpoint(bp);
    assert(dbg.Breakpoints()[0].enabled);

    dbg.EnableBreakpoint(id, false);
    assert(!dbg.Breakpoints()[0].enabled);

    dbg.EnableBreakpoint(id, true);
    assert(dbg.Breakpoints()[0].enabled);

    // Non-existent breakpoint
    assert(!dbg.EnableBreakpoint(999, false));
    std::cout << "  [PASS] test_tick_debugger_enable_disable_breakpoint" << std::endl;
}

void test_p18_tick_debugger_clear_breakpoints() {
    TickStepDebugger dbg;
    TickBreakpoint bp;
    bp.tick = 10;
    dbg.AddBreakpoint(bp);
    bp.tick = 20;
    dbg.AddBreakpoint(bp);
    assert(dbg.BreakpointCount() == 2);
    dbg.ClearBreakpoints();
    assert(dbg.BreakpointCount() == 0);
    std::cout << "  [PASS] test_tick_debugger_clear_breakpoints" << std::endl;
}

void test_p18_tick_debugger_check_tick_breakpoint() {
    TickStepDebugger dbg;
    TickBreakpoint bp;
    bp.tick = 10;
    dbg.AddBreakpoint(bp);

    dbg.SetCurrentTick(5);
    assert(!dbg.CheckBreakpoints());
    assert(dbg.TriggeredBreakpointId() == 0);

    dbg.SetCurrentTick(10);
    assert(dbg.CheckBreakpoints());
    assert(dbg.TriggeredBreakpointId() > 0);
    std::cout << "  [PASS] test_tick_debugger_check_tick_breakpoint" << std::endl;
}

void test_p18_tick_debugger_disabled_breakpoint_not_triggered() {
    TickStepDebugger dbg;
    TickBreakpoint bp;
    bp.tick = 10;
    uint32_t id = dbg.AddBreakpoint(bp);
    dbg.EnableBreakpoint(id, false);

    dbg.SetCurrentTick(10);
    assert(!dbg.CheckBreakpoints());
    std::cout << "  [PASS] test_tick_debugger_disabled_breakpoint_not_triggered" << std::endl;
}

void test_p18_tick_debugger_breakpoint_callback() {
    TickStepDebugger dbg;
    bool fired = false;
    uint32_t cbBpId = 0;
    uint64_t cbTick = 0;

    dbg.SetBreakpointCallback([&](uint32_t bpId, uint64_t tick) {
        fired = true;
        cbBpId = bpId;
        cbTick = tick;
    });

    TickBreakpoint bp;
    bp.tick = 25;
    uint32_t id = dbg.AddBreakpoint(bp);

    dbg.SetCurrentTick(25);
    dbg.CheckBreakpoints();
    assert(fired);
    assert(cbBpId == id);
    assert(cbTick == 25);
    std::cout << "  [PASS] test_tick_debugger_breakpoint_callback" << std::endl;
}

void test_p18_tick_debugger_hash_mismatch_breakpoint() {
    TickStepDebugger dbg;
    uint32_t id = dbg.AddHashMismatchBreakpoint(0xDEADBEEF, "hash_check");
    assert(dbg.BreakpointCount() == 1);
    assert(dbg.Breakpoints()[0].hashMismatch == 0xDEADBEEF);
    assert(dbg.Breakpoints()[0].label == "hash_check");
    (void)id;
    std::cout << "  [PASS] test_tick_debugger_hash_mismatch_breakpoint" << std::endl;
}

// =============================================================
// SimulationStateAuditor Tests
// =============================================================

void test_p18_auditor_register_system() {
    SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.RegisterSystem("AI");
    assert(auditor.RegisteredSystems().size() == 2);
    // Duplicate registration should be ignored
    auditor.RegisterSystem("Physics");
    assert(auditor.RegisteredSystems().size() == 2);
    std::cout << "  [PASS] test_auditor_register_system" << std::endl;
}

void test_p18_auditor_declare_state() {
    SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.DeclareState("Physics", "RigidBodies", StateCategory::Simulated);
    auditor.DeclareState("Physics", "Constraints", StateCategory::Simulated);
    assert(auditor.DeclaredStateCount("Physics") == 2);
    auto blocks = auditor.DeclaredBlocks("Physics");
    assert(blocks.size() == 2);
    assert(blocks[0] == "RigidBodies");
    assert(blocks[1] == "Constraints");
    std::cout << "  [PASS] test_auditor_declare_state" << std::endl;
}

void test_p18_auditor_audit_passes() {
    SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.RegisterSystem("AI");
    auditor.DeclareState("Physics", "Bodies", StateCategory::Simulated);
    auditor.DeclareState("AI", "Memory", StateCategory::Simulated);

    auto result = auditor.Audit();
    assert(result.passed);
    assert(result.systemsAudited == 2);
    assert(result.stateBlocksVerified == 2);
    assert(result.errorsFound == 0);
    std::cout << "  [PASS] test_auditor_audit_passes" << std::endl;
}

void test_p18_auditor_audit_fails_missing_state() {
    SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.RegisterSystem("AI");
    // Only declare state for Physics, not AI
    auditor.DeclareState("Physics", "Bodies", StateCategory::Simulated);

    auto result = auditor.Audit();
    assert(!result.passed);
    assert(result.errorsFound == 1);
    assert(result.findings.size() >= 1);
    assert(result.findings[0].severity == AuditFinding::Severity::Error);
    assert(result.findings[0].systemName == "AI");
    std::cout << "  [PASS] test_auditor_audit_fails_missing_state" << std::endl;
}

void test_p18_auditor_audit_warns_unregistered() {
    SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.DeclareState("Physics", "Bodies");
    // Declare state for an unregistered system
    auditor.DeclareState("UnknownSystem", "Data");

    auto result = auditor.Audit();
    assert(result.passed); // Unregistered is a warning, not error
    assert(result.warningsFound == 1);
    std::cout << "  [PASS] test_auditor_audit_warns_unregistered" << std::endl;
}

void test_p18_auditor_clear() {
    SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.DeclareState("Physics", "Bodies");
    auditor.Clear();
    assert(auditor.RegisteredSystems().empty());
    assert(auditor.DeclaredStateCount("Physics") == 0);
    std::cout << "  [PASS] test_auditor_clear" << std::endl;
}

void test_p18_auditor_multiple_systems() {
    SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.RegisterSystem("AI");
    auditor.RegisterSystem("Networking");
    auditor.RegisterSystem("Economy");

    auditor.DeclareState("Physics", "Bodies");
    auditor.DeclareState("AI", "Memory");
    auditor.DeclareState("AI", "Plans");
    auditor.DeclareState("Networking", "Peers");
    auditor.DeclareState("Economy", "Markets");

    auto result = auditor.Audit();
    assert(result.passed);
    assert(result.systemsAudited == 4);
    assert(result.stateBlocksVerified == 5);
    std::cout << "  [PASS] test_auditor_multiple_systems" << std::endl;
}

// =============================================================
// FPDriftDetector Tests
// =============================================================

void test_p18_fp_drift_no_drift() {
    StateHasher hasherA;
    hasherA.Reset(42);
    StateHasher hasherB;
    hasherB.Reset(42);

    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4, 5};

    for (uint64_t t = 0; t < 10; ++t) {
        hasherA.AdvanceTick(t, state, inputs);
        hasherB.AdvanceTick(t, state, inputs);
    }

    FPDriftDetector detector;
    detector.SetPlatformNames("Windows", "Linux");
    auto report = detector.RunComparison(hasherA, hasherB);

    assert(!report.hasDrift);
    assert(report.firstDivergenceTick == -1);
    assert(report.totalTicksCompared == 10);
    assert(report.matchingTicks == 10);
    assert(report.platformAName == "Windows");
    assert(report.platformBName == "Linux");
    std::cout << "  [PASS] test_fp_drift_no_drift" << std::endl;
}

void test_p18_fp_drift_detected() {
    StateHasher hasherA;
    hasherA.Reset(42);
    StateHasher hasherB;
    hasherB.Reset(42);

    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {4, 5};

    // First 5 ticks identical
    for (uint64_t t = 0; t < 5; ++t) {
        hasherA.AdvanceTick(t, state, inputs);
        hasherB.AdvanceTick(t, state, inputs);
    }
    // Tick 5 diverges
    std::vector<uint8_t> stateB = {1, 2, 99};
    hasherA.AdvanceTick(5, state, inputs);
    hasherB.AdvanceTick(5, stateB, inputs);

    FPDriftDetector detector;
    auto report = detector.RunComparison(hasherA, hasherB);

    assert(report.hasDrift);
    assert(report.firstDivergenceTick == 5);
    assert(report.matchingTicks == 5);
    assert(report.platformAHash != report.platformBHash);
    std::cout << "  [PASS] test_fp_drift_detected" << std::endl;
}

void test_p18_fp_drift_compare_system_hashes() {
    std::vector<std::pair<std::string, uint64_t>> sysA = {
        {"Physics", 100}, {"AI", 200}, {"Render", 300}
    };
    std::vector<std::pair<std::string, uint64_t>> sysB = {
        {"Physics", 100}, {"AI", 999}, {"Render", 300}
    };

    auto drifts = FPDriftDetector::CompareSystemHashes(sysA, sysB);
    assert(drifts.size() == 3);
    assert(drifts[0].matches);   // Physics OK
    assert(!drifts[1].matches);  // AI diverged
    assert(drifts[2].matches);   // Render OK
    assert(drifts[1].systemName == "AI");
    std::cout << "  [PASS] test_fp_drift_compare_system_hashes" << std::endl;
}

void test_p18_fp_drift_compare_system_hashes_unequal_sizes() {
    std::vector<std::pair<std::string, uint64_t>> sysA = {
        {"Physics", 100}, {"AI", 200}
    };
    std::vector<std::pair<std::string, uint64_t>> sysB = {
        {"Physics", 100}
    };

    auto drifts = FPDriftDetector::CompareSystemHashes(sysA, sysB);
    assert(drifts.size() == 2);
    assert(drifts[0].matches);
    assert(!drifts[1].matches); // extra entry in A has no match
    assert(drifts[1].hashB == 0);
    std::cout << "  [PASS] test_fp_drift_compare_system_hashes_unequal_sizes" << std::endl;
}

void test_p18_fp_drift_format_report_no_drift() {
    FPDriftReport report;
    report.platformAName = "Win";
    report.platformBName = "Linux";
    report.totalTicksCompared = 100;
    report.matchingTicks = 100;

    std::string formatted = FPDriftDetector::FormatReport(report);
    assert(!formatted.empty());
    assert(formatted.find("OK") != std::string::npos);
    assert(formatted.find("Win") != std::string::npos);
    assert(formatted.find("Linux") != std::string::npos);
    std::cout << "  [PASS] test_fp_drift_format_report_no_drift" << std::endl;
}

void test_p18_fp_drift_format_report_with_drift() {
    FPDriftReport report;
    report.hasDrift = true;
    report.platformAName = "Win";
    report.platformBName = "Mac";
    report.firstDivergenceTick = 42;
    report.platformAHash = 0x1234;
    report.platformBHash = 0x5678;
    report.totalTicksCompared = 50;
    report.matchingTicks = 42;

    std::string formatted = FPDriftDetector::FormatReport(report);
    assert(!formatted.empty());
    assert(formatted.find("DRIFT") != std::string::npos);
    assert(formatted.find("42") != std::string::npos);
    std::cout << "  [PASS] test_fp_drift_format_report_with_drift" << std::endl;
}

void test_p18_fp_drift_add_system_hashes() {
    FPDriftDetector detector;
    detector.SetPlatformNames("A", "B");

    std::vector<std::pair<std::string, uint64_t>> sysA = {{"Phys", 100}};
    std::vector<std::pair<std::string, uint64_t>> sysB = {{"Phys", 999}};

    detector.AddSystemHashes("Simulation", sysA, sysB);

    // Now run a comparison to see system drifts in the report
    StateHasher ha, hb;
    ha.Reset(1);
    hb.Reset(2); // different seed => different hashes
    std::vector<uint8_t> s = {1};
    std::vector<uint8_t> i = {2};
    ha.AdvanceTick(0, s, i);
    hb.AdvanceTick(0, s, i);

    auto report = detector.RunComparison(ha, hb);
    assert(report.hasDrift);
    assert(!report.systemDrifts.empty());
    assert(!report.systemDrifts[0].matches);
    std::cout << "  [PASS] test_fp_drift_add_system_hashes" << std::endl;
}

// =============================================================
// Runner
// =============================================================

void register_next_tasks_phase18() {
    std::cout << "\n--- Phase 18: TickStepDebugger ---" << std::endl;
    test_p18_tick_debugger_initial_state();
    test_p18_tick_debugger_step_forward();
    test_p18_tick_debugger_step_backward();
    test_p18_tick_debugger_jump_to_tick();
    test_p18_tick_debugger_pause();
    test_p18_tick_debugger_add_breakpoint();
    test_p18_tick_debugger_remove_breakpoint();
    test_p18_tick_debugger_enable_disable_breakpoint();
    test_p18_tick_debugger_clear_breakpoints();
    test_p18_tick_debugger_check_tick_breakpoint();
    test_p18_tick_debugger_disabled_breakpoint_not_triggered();
    test_p18_tick_debugger_breakpoint_callback();
    test_p18_tick_debugger_hash_mismatch_breakpoint();

    std::cout << "\n--- Phase 18: SimulationStateAuditor ---" << std::endl;
    test_p18_auditor_register_system();
    test_p18_auditor_declare_state();
    test_p18_auditor_audit_passes();
    test_p18_auditor_audit_fails_missing_state();
    test_p18_auditor_audit_warns_unregistered();
    test_p18_auditor_clear();
    test_p18_auditor_multiple_systems();

    std::cout << "\n--- Phase 18: FPDriftDetector ---" << std::endl;
    test_p18_fp_drift_no_drift();
    test_p18_fp_drift_detected();
    test_p18_fp_drift_compare_system_hashes();
    test_p18_fp_drift_compare_system_hashes_unequal_sizes();
    test_p18_fp_drift_format_report_no_drift();
    test_p18_fp_drift_format_report_with_drift();
    test_p18_fp_drift_add_system_hashes();
}
