#include <iostream>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>

#include "../engine/sim/TLCModelChecker.h"

// ============================================================
// TLC Model Checker Tests
// ============================================================

static std::string specsDir() {
    // CMAKE_SOURCE_DIR is defined by the test CMakeLists.txt
#ifdef CMAKE_SOURCE_DIR
    return std::string(CMAKE_SOURCE_DIR) + "/specs";
#else
    return "specs";
#endif
}

void test_tlc_register_spec() {
    atlas::sim::TLCModelChecker checker;
    bool ok = checker.RegisterSpec("replay", specsDir() + "/replay.tla");
    assert(ok);
    assert(checker.SpecCount() == 1);

    // Duplicate registration fails
    assert(!checker.RegisterSpec("replay", specsDir() + "/replay.tla"));
    assert(checker.SpecCount() == 1);

    std::cout << "[PASS] test_tlc_register_spec" << std::endl;
}

void test_tlc_register_multiple_specs() {
    atlas::sim::TLCModelChecker checker;
    assert(checker.RegisterSpec("ecs", specsDir() + "/ecs.tla"));
    assert(checker.RegisterSpec("replay", specsDir() + "/replay.tla"));
    assert(checker.RegisterSpec("layout", specsDir() + "/layout.tla"));
    assert(checker.SpecCount() == 3);

    auto names = checker.SpecNames();
    assert(names.size() == 3);
    // Map is sorted, so names should be alphabetical
    assert(names[0] == "ecs");
    assert(names[1] == "layout");
    assert(names[2] == "replay");

    std::cout << "[PASS] test_tlc_register_multiple_specs" << std::endl;
}

void test_tlc_register_bad_path() {
    atlas::sim::TLCModelChecker checker;
    assert(!checker.RegisterSpec("missing", "/nonexistent/path.tla"));
    assert(checker.SpecCount() == 0);
    assert(!checker.RegisterSpec("", specsDir() + "/ecs.tla"));
    assert(!checker.RegisterSpec("x", ""));

    std::cout << "[PASS] test_tlc_register_bad_path" << std::endl;
}

void test_tlc_unregister_spec() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("ecs", specsDir() + "/ecs.tla");
    assert(checker.SpecCount() == 1);
    assert(checker.UnregisterSpec("ecs"));
    assert(checker.SpecCount() == 0);
    assert(!checker.UnregisterSpec("ecs")); // already removed

    std::cout << "[PASS] test_tlc_unregister_spec" << std::endl;
}

void test_tlc_get_spec() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("replay", specsDir() + "/replay.tla");

    const auto* spec = checker.GetSpec("replay");
    assert(spec != nullptr);
    assert(spec->name == "replay");
    assert(!spec->content.empty());
    assert(!spec->invariantNames.empty());

    assert(checker.GetSpec("nonexistent") == nullptr);

    std::cout << "[PASS] test_tlc_get_spec" << std::endl;
}

void test_tlc_extract_invariants_replay() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("replay", specsDir() + "/replay.tla");
    const auto* spec = checker.GetSpec("replay");
    assert(spec != nullptr);

    // replay.tla should have: ChainIntegrity, NoGaps, MonotonicTicks,
    // ReplayDeterminism, FrameHashConsistency, RecordingTerminates,
    // PlaybackTerminates
    const auto& inv = spec->invariantNames;
    assert(std::find(inv.begin(), inv.end(), "ChainIntegrity") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "NoGaps") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "MonotonicTicks") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "ReplayDeterminism") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "FrameHashConsistency") != inv.end());

    std::cout << "[PASS] test_tlc_extract_invariants_replay" << std::endl;
}

void test_tlc_extract_invariants_ecs() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("ecs", specsDir() + "/ecs.tla");
    const auto* spec = checker.GetSpec("ecs");
    assert(spec != nullptr);

    const auto& inv = spec->invariantNames;
    assert(std::find(inv.begin(), inv.end(), "UniqueIDs") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "DeadClean") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "FixedOrder") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "ExclusiveOwnership") != inv.end());

    // Actions should NOT appear
    assert(std::find(inv.begin(), inv.end(), "CreateEntity") == inv.end());
    assert(std::find(inv.begin(), inv.end(), "DestroyEntity") == inv.end());
    assert(std::find(inv.begin(), inv.end(), "BeginTick") == inv.end());
    assert(std::find(inv.begin(), inv.end(), "Init") == inv.end());

    std::cout << "[PASS] test_tlc_extract_invariants_ecs" << std::endl;
}

void test_tlc_extract_invariants_layout() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("layout", specsDir() + "/layout.tla");
    const auto* spec = checker.GetSpec("layout");
    assert(spec != nullptr);

    const auto& inv = spec->invariantNames;
    assert(std::find(inv.begin(), inv.end(), "SizePartition") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "FullCoverage") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "MinimumSize") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "TreeConsistent") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "NoOrphanPanels") != inv.end());

    // Helpers and actions should be excluded
    assert(std::find(inv.begin(), inv.end(), "SplitPanel") == inv.end());
    assert(std::find(inv.begin(), inv.end(), "MergePanel") == inv.end());
    assert(std::find(inv.begin(), inv.end(), "ResizePanel") == inv.end());

    std::cout << "[PASS] test_tlc_extract_invariants_layout" << std::endl;
}

void test_tlc_validate_spec() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("replay", specsDir() + "/replay.tla");
    assert(checker.ValidateSpec("replay"));
    assert(!checker.ValidateSpec("nonexistent"));

    std::cout << "[PASS] test_tlc_validate_spec" << std::endl;
}

void test_tlc_check_spec() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("ecs", specsDir() + "/ecs.tla");
    auto report = checker.CheckSpec("ecs");

    assert(report.result == atlas::sim::TLCResult::Pass);
    assert(report.specName == "ecs");
    assert(!report.specPath.empty());
    assert(report.statesExplored > 0);
    assert(report.distinctStates > 0);
    assert(report.elapsedSeconds >= 0.0);
    assert(!report.invariants.empty());
    assert(report.errorMessage.empty());

    for (const auto& inv : report.invariants) {
        assert(inv.verified);
    }

    std::cout << "[PASS] test_tlc_check_spec" << std::endl;
}

void test_tlc_check_spec_not_found() {
    atlas::sim::TLCModelChecker checker;
    auto report = checker.CheckSpec("missing");
    assert(report.result == atlas::sim::TLCResult::NotFound);
    assert(!report.errorMessage.empty());

    std::cout << "[PASS] test_tlc_check_spec_not_found" << std::endl;
}

void test_tlc_check_all() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("ecs", specsDir() + "/ecs.tla");
    checker.RegisterSpec("replay", specsDir() + "/replay.tla");
    checker.RegisterSpec("layout", specsDir() + "/layout.tla");

    auto reports = checker.CheckAll();
    assert(reports.size() == 3);

    for (const auto& r : reports) {
        assert(r.result == atlas::sim::TLCResult::Pass);
        assert(!r.invariants.empty());
    }

    std::cout << "[PASS] test_tlc_check_all" << std::endl;
}

void test_tlc_ci_check_pass() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("ecs", specsDir() + "/ecs.tla");
    checker.RegisterSpec("replay", specsDir() + "/replay.tla");
    assert(checker.RunCICheck() == 0);

    std::cout << "[PASS] test_tlc_ci_check_pass" << std::endl;
}

void test_tlc_format_report() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("replay", specsDir() + "/replay.tla");
    auto report = checker.CheckSpec("replay");
    std::string formatted = atlas::sim::TLCModelChecker::FormatReport(report);

    assert(formatted.find("TLC Check Report") != std::string::npos);
    assert(formatted.find("replay") != std::string::npos);
    assert(formatted.find("PASS") != std::string::npos);
    assert(formatted.find("Invariants") != std::string::npos);
    assert(formatted.find("ChainIntegrity") != std::string::npos);

    std::cout << "[PASS] test_tlc_format_report" << std::endl;
}

void test_tlc_export_report() {
    atlas::sim::TLCModelChecker checker;
    checker.RegisterSpec("ecs", specsDir() + "/ecs.tla");
    auto report = checker.CheckSpec("ecs");

    std::string path = (std::filesystem::temp_directory_path() / "test_tlc_report.txt").string();
    assert(atlas::sim::TLCModelChecker::ExportReport(report, path));

    std::ifstream in(path);
    assert(in.good());
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("TLC Check Report") != std::string::npos);
    assert(content.find("ecs") != std::string::npos);

    std::remove(path.c_str());
    std::cout << "[PASS] test_tlc_export_report" << std::endl;
}

void test_tlc_path_management() {
    atlas::sim::TLCModelChecker checker;
    assert(checker.GetTLCPath().empty());
    assert(!checker.IsTLCAvailable());

    checker.SetTLCPath("/nonexistent/tlc");
    assert(checker.GetTLCPath() == "/nonexistent/tlc");
    assert(!checker.IsTLCAvailable());

    // Point to a file that exists
    checker.SetTLCPath(specsDir() + "/ecs.tla");
    assert(checker.IsTLCAvailable());

    std::cout << "[PASS] test_tlc_path_management" << std::endl;
}

void test_tlc_extract_invariants_static() {
    std::string content = R"(
---- MODULE test ----
EXTENDS Naturals

SafetyProp == x > 0
LivenessProp == y < 10
Init == TRUE
Next == TRUE
Spec == Init /\ [][Next]_vars
====
)";
    auto inv = atlas::sim::TLCModelChecker::ExtractInvariants(content);
    assert(std::find(inv.begin(), inv.end(), "SafetyProp") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "LivenessProp") != inv.end());
    assert(std::find(inv.begin(), inv.end(), "Init") == inv.end());
    assert(std::find(inv.begin(), inv.end(), "Next") == inv.end());
    assert(std::find(inv.begin(), inv.end(), "Spec") == inv.end());

    std::cout << "[PASS] test_tlc_extract_invariants_static" << std::endl;
}
