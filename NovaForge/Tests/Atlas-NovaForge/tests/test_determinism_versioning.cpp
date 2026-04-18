#include "../engine/sim/DeterminismVersioning.h"
#include <iostream>
#include <cassert>

void test_dv_set_version() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::DeterminismVersion v;
    v.major = 1; v.minor = 2; v.patch = 3;
    v.hash = 12345; v.description = "release";
    reg.SetCurrentVersion(v);
    auto cur = reg.GetCurrentVersion();
    assert(cur.major == 1);
    assert(cur.minor == 2);
    assert(cur.patch == 3);
    assert(cur.hash == 12345);
    std::cout << "[PASS] test_dv_set_version" << std::endl;
}

void test_dv_register_fork() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::ForkInfo fork;
    fork.name = "community_fork";
    fork.baseVersion = {1, 0, 0, 0, ""};
    fork.currentVersion = {1, 0, 1, 0, ""};
    fork.isCompatible = true;
    reg.RegisterFork(fork);
    assert(reg.ForkCount() == 1);
    std::cout << "[PASS] test_dv_register_fork" << std::endl;
}

void test_dv_unregister_fork() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::ForkInfo fork;
    fork.name = "test_fork";
    reg.RegisterFork(fork);
    assert(reg.ForkCount() == 1);
    reg.UnregisterFork("test_fork");
    assert(reg.ForkCount() == 0);
    std::cout << "[PASS] test_dv_unregister_fork" << std::endl;
}

void test_dv_get_fork() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::ForkInfo fork;
    fork.name = "my_fork";
    fork.isCompatible = true;
    reg.RegisterFork(fork);
    auto* found = reg.GetFork("my_fork");
    assert(found != nullptr);
    assert(found->name == "my_fork");
    assert(reg.GetFork("no_such") == nullptr);
    std::cout << "[PASS] test_dv_get_fork" << std::endl;
}

void test_dv_list_forks() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::ForkInfo f1; f1.name = "a";
    atlas::sim::ForkInfo f2; f2.name = "b";
    reg.RegisterFork(f1);
    reg.RegisterFork(f2);
    auto list = reg.ListForks();
    assert(list.size() == 2);
    std::cout << "[PASS] test_dv_list_forks" << std::endl;
}

void test_dv_compatible() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::DeterminismVersion v; v.major = 1; v.minor = 0; v.patch = 0;
    reg.SetCurrentVersion(v);
    atlas::sim::ForkInfo fork;
    fork.name = "compat";
    fork.baseVersion = {1, 0, 0, 0, ""};
    fork.isCompatible = true;
    reg.RegisterFork(fork);
    assert(reg.CheckCompatibility("compat"));
    std::cout << "[PASS] test_dv_compatible" << std::endl;
}

void test_dv_incompatible() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::DeterminismVersion v; v.major = 2; v.minor = 0; v.patch = 0;
    reg.SetCurrentVersion(v);
    atlas::sim::ForkInfo fork;
    fork.name = "old_fork";
    fork.baseVersion = {1, 0, 0, 0, ""};
    fork.isCompatible = true;
    reg.RegisterFork(fork);
    assert(!reg.CheckCompatibility("old_fork"));
    std::cout << "[PASS] test_dv_incompatible" << std::endl;
}

void test_dv_check_all() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::DeterminismVersion v; v.major = 1; v.minor = 0; v.patch = 0;
    reg.SetCurrentVersion(v);
    atlas::sim::ForkInfo f1;
    f1.name = "good"; f1.baseVersion = {1, 0, 0, 0, ""}; f1.isCompatible = true;
    atlas::sim::ForkInfo f2;
    f2.name = "bad"; f2.baseVersion = {2, 0, 0, 0, ""}; f2.isCompatible = true;
    reg.RegisterFork(f1);
    reg.RegisterFork(f2);
    auto incompat = reg.CheckAllCompatibility();
    assert(incompat.size() == 1);
    assert(incompat[0] == "bad");
    std::cout << "[PASS] test_dv_check_all" << std::endl;
}

void test_dv_report() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::DeterminismVersion v; v.major = 1; v.minor = 0; v.patch = 0;
    reg.SetCurrentVersion(v);
    atlas::sim::ForkInfo fork;
    fork.name = "fork_a"; fork.baseVersion = {1, 0, 0, 0, ""}; fork.isCompatible = true;
    reg.RegisterFork(fork);
    std::string report = reg.GenerateReport();
    assert(!report.empty());
    assert(report.find("fork_a") != std::string::npos);
    std::cout << "[PASS] test_dv_report" << std::endl;
}

void test_dv_clear() {
    atlas::sim::DeterminismVersionRegistry reg;
    atlas::sim::DeterminismVersion v; v.major = 1; v.minor = 0; v.patch = 0;
    reg.SetCurrentVersion(v);
    atlas::sim::ForkInfo fork;
    fork.name = "f";
    reg.RegisterFork(fork);
    assert(reg.ForkCount() == 1);
    reg.Clear();
    assert(reg.ForkCount() == 0);
    assert(reg.GetCurrentVersion().major == 0);
    std::cout << "[PASS] test_dv_clear" << std::endl;
}
