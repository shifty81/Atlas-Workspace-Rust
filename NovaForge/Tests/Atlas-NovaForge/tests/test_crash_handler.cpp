#include "../engine/core/CrashHandler.h"
#include <iostream>
#include <cassert>
#include <filesystem>

using namespace atlas::core;

void test_crash_breadcrumb() {
    CrashHandler handler;

    assert(handler.Breadcrumbs().empty());

    handler.AddBreadcrumb("engine", "Init complete");
    handler.AddBreadcrumb("net", "Connected to server");
    assert(handler.Breadcrumbs().size() == 2);
    assert(handler.Breadcrumbs()[0].category == "engine");
    assert(handler.Breadcrumbs()[1].message == "Connected to server");

    std::cout << "[PASS] test_crash_breadcrumb" << std::endl;
}

void test_crash_breadcrumb_limit() {
    CrashHandler handler;
    handler.SetMaxBreadcrumbs(3);
    assert(handler.MaxBreadcrumbs() == 3);

    handler.AddBreadcrumb("a", "msg1");
    handler.AddBreadcrumb("b", "msg2");
    handler.AddBreadcrumb("c", "msg3");
    handler.AddBreadcrumb("d", "msg4");

    assert(handler.Breadcrumbs().size() == 3);
    assert(handler.Breadcrumbs()[0].category == "b");
    assert(handler.Breadcrumbs()[2].category == "d");

    std::cout << "[PASS] test_crash_breadcrumb_limit" << std::endl;
}

void test_crash_clear_breadcrumbs() {
    CrashHandler handler;
    handler.AddBreadcrumb("test", "message");
    assert(!handler.Breadcrumbs().empty());

    handler.ClearBreadcrumbs();
    assert(handler.Breadcrumbs().empty());

    std::cout << "[PASS] test_crash_clear_breadcrumbs" << std::endl;
}

void test_crash_generate_report() {
    CrashHandler handler;
    handler.SetEngineVersion("1.0.0");
    handler.SetPlatform("Linux");
    handler.SetCurrentTick(1234);
    handler.AddBreadcrumb("engine", "loading");

    CrashReport report = handler.GenerateReport("Segfault in PhysicsWorld", "PhysicsWorld::Step");
    assert(report.reason == "Segfault in PhysicsWorld");
    assert(report.stackHint == "PhysicsWorld::Step");
    assert(report.engineVersion == "1.0.0");
    assert(report.platform == "Linux");
    assert(report.tickAtCrash == 1234);
    assert(report.breadcrumbs.size() == 1);

    std::cout << "[PASS] test_crash_generate_report" << std::endl;
}

void test_crash_save_report() {
    std::string tmpPath = (std::filesystem::temp_directory_path() / "atlas_crash_test.txt").string();

    CrashHandler handler;
    handler.SetEngineVersion("0.5.0");
    handler.SetPlatform("Linux");
    handler.AddBreadcrumb("init", "bootstrap");

    CrashReport report = handler.GenerateReport("Test crash");
    bool saved = handler.SaveReport(report, tmpPath);
    assert(saved);
    assert(std::filesystem::exists(tmpPath));

    std::filesystem::remove(tmpPath);

    std::cout << "[PASS] test_crash_save_report" << std::endl;
}

void test_crash_callback() {
    CrashHandler handler;
    handler.SetEngineVersion("1.0.0");

    bool callbackFired = false;
    std::string capturedReason;

    handler.SetCrashCallback([&](const CrashReport& report) {
        callbackFired = true;
        capturedReason = report.reason;
    });

    handler.ReportCrash("Out of memory");
    assert(callbackFired);
    assert(capturedReason == "Out of memory");

    std::cout << "[PASS] test_crash_callback" << std::endl;
}

void test_crash_report_to_file() {
    std::string tmpPath = (std::filesystem::temp_directory_path() / "atlas_crash_report_test.txt").string();

    CrashHandler handler;
    handler.SetEngineVersion("1.0.0");
    handler.SetPlatform("Linux");
    handler.SetCurrentTick(500);
    handler.AddBreadcrumb("world", "loading chunk 42");

    handler.ReportCrash("Null pointer", tmpPath);
    assert(std::filesystem::exists(tmpPath));

    std::filesystem::remove(tmpPath);

    std::cout << "[PASS] test_crash_report_to_file" << std::endl;
}
