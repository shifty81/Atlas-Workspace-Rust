#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <filesystem>
#include <cstdlib>

// Test the crash reporter tool and include firewall enhancements.

namespace fs = std::filesystem;

// Helper: resolve path trying both CWD and parent
static std::string resolveTool(const std::string& relative) {
    if (fs::exists(relative)) return relative;
    std::string parent = "../" + relative;
    if (fs::exists(parent)) return parent;
    return relative;
}

// Helper: run crash_reporter.py command with path fallback
static int runCrashReporter(const std::string& args) {
    std::string tool = resolveTool("tools/crash_reporter.py");
    std::string cmd = "python3 '" + tool + "' " + args + " > /dev/null 2>&1";
    return std::system(cmd.c_str());
}

// ============================================================
// Crash Reporter Tests
// ============================================================

void test_crash_reporter_tool_exists() {
    std::string path = resolveTool("tools/crash_reporter.py");
    assert(fs::exists(path));
    std::cout << "[PASS] test_crash_reporter_tool_exists" << std::endl;
}

void test_crash_reporter_help() {
    int result = runCrashReporter("--help");
    assert(result == 0);
    std::cout << "[PASS] test_crash_reporter_help" << std::endl;
}

void test_crash_reporter_validate_empty_dir() {
    auto tmpDir = fs::temp_directory_path() / "atlas_test_crash_empty";
    fs::create_directories(tmpDir);

    int result = runCrashReporter("--dir '" + tmpDir.string() + "' --validate-only");
    // Should return non-zero (no manifests found)
    assert(result != 0);

    fs::remove_all(tmpDir);
    std::cout << "[PASS] test_crash_reporter_validate_empty_dir" << std::endl;
}

void test_crash_reporter_validate_manifest() {
    auto tmpDir = fs::temp_directory_path() / "atlas_test_crash_manifest";
    fs::create_directories(tmpDir);

    // Create mock save and replay files
    auto savePath = tmpDir / "desync_tick_100.asav";
    auto replayPath = tmpDir / "desync_tick_100.rply";
    auto reportPath = tmpDir / "desync_tick_100_report.txt";
    {
        std::ofstream(savePath) << "mock save data";
        std::ofstream(replayPath) << "mock replay data";
        std::ofstream(reportPath) << "mock report";
    }

    // Create manifest
    auto manifestPath = tmpDir / "desync_tick_100.atlascrash";
    {
        std::ofstream out(manifestPath);
        out << "atlas_crash_bundle_v1\n";
        out << "engine_version=dev\n";
        out << "platform=linux\n";
        out << "tick=100\n";
        out << "tick_rate=60\n";
        out << "seed=42\n";
        out << "local_hash=0xabc123\n";
        out << "remote_hash=0xdef456\n";
        out << "save=" << savePath.string() << "\n";
        out << "replay=" << replayPath.string() << "\n";
        out << "report=" << reportPath.string() << "\n";
        out << "repro=./AtlasServer --repro --save \"" << savePath.string() << "\"\n";
    }

    int result = runCrashReporter("--dir '" + tmpDir.string() + "' --validate-only");
    assert(result == 0);

    fs::remove_all(tmpDir);
    std::cout << "[PASS] test_crash_reporter_validate_manifest" << std::endl;
}

void test_crash_reporter_bundle() {
    auto tmpDir = fs::temp_directory_path() / "atlas_test_crash_bundle";
    fs::create_directories(tmpDir);

    auto savePath = tmpDir / "desync_tick_200.asav";
    auto replayPath = tmpDir / "desync_tick_200.rply";
    auto reportPath = tmpDir / "desync_tick_200_report.txt";
    {
        std::ofstream(savePath) << "save data for tick 200";
        std::ofstream(replayPath) << "replay data for tick 200";
        std::ofstream(reportPath) << "report for tick 200";
    }

    auto manifestPath = tmpDir / "desync_tick_200.atlascrash";
    {
        std::ofstream out(manifestPath);
        out << "atlas_crash_bundle_v1\n";
        out << "engine_version=0.1.0\n";
        out << "platform=linux\n";
        out << "tick=200\n";
        out << "tick_rate=60\n";
        out << "seed=1234\n";
        out << "local_hash=0x111\n";
        out << "remote_hash=0x222\n";
        out << "save=" << savePath.string() << "\n";
        out << "replay=" << replayPath.string() << "\n";
        out << "report=" << reportPath.string() << "\n";
        out << "repro=./AtlasServer --repro\n";
    }

    auto outputPath = tmpDir / "test_bundle.tar.gz";
    int result = runCrashReporter("--dir '" + tmpDir.string() + "' --output '" + outputPath.string() + "'");
    assert(result == 0);
    assert(fs::exists(outputPath));
    assert(fs::file_size(outputPath) > 0);

    fs::remove_all(tmpDir);
    std::cout << "[PASS] test_crash_reporter_bundle" << std::endl;
}

// ============================================================
// Include Firewall Enhancement Tests
// ============================================================

void test_include_firewall_header_exists() {
    std::string path = resolveTool("engine/core/contract/IncludeFirewall.h");
    assert(fs::exists(path));
    std::cout << "[PASS] test_include_firewall_header_exists" << std::endl;
}

void test_include_firewall_defines_marker() {
    std::string path = resolveTool("engine/core/contract/IncludeFirewall.h");

    std::ifstream file(path);
    assert(file.is_open());
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    assert(content.find("ATLAS_INCLUDE_FIREWALL_ACTIVE") != std::string::npos);
    std::cout << "[PASS] test_include_firewall_defines_marker" << std::endl;
}

void test_include_firewall_guards_simulation() {
    std::string path = resolveTool("engine/core/contract/IncludeFirewall.h");

    std::ifstream file(path);
    assert(file.is_open());
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    assert(content.find("ATLAS_LAYER_SIMULATION") != std::string::npos);
    assert(content.find("GL_VERSION") != std::string::npos);
    assert(content.find("VULKAN_CORE_H_") != std::string::npos);
    assert(content.find("ATLAS_GL_RENDERER_H") != std::string::npos);
    assert(content.find("ATLAS_VULKAN_RENDERER_H") != std::string::npos);
    std::cout << "[PASS] test_include_firewall_guards_simulation" << std::endl;
}

void test_include_firewall_guards_core() {
    std::string path = resolveTool("engine/core/contract/IncludeFirewall.h");

    std::ifstream file(path);
    assert(file.is_open());
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    assert(content.find("ATLAS_LAYER_CORE") != std::string::npos);
    assert(content.find("ATLAS_TICK_SCHEDULER_H") != std::string::npos);
    assert(content.find("ATLAS_REPLAY_RECORDER_H") != std::string::npos);
    std::cout << "[PASS] test_include_firewall_guards_core" << std::endl;
}

void test_include_firewall_ecs_no_render() {
    // Verify ecs/ source files don't include render headers
    std::string ecsDir = resolveTool("engine/ecs");
    if (!fs::exists(ecsDir)) {
        std::cout << "[PASS] test_include_firewall_ecs_no_render (skipped - dir not found)" << std::endl;
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(ecsDir)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        if (ext != ".h" && ext != ".cpp") continue;

        std::ifstream file(entry.path().string());
        std::string line;
        while (std::getline(file, line)) {
            auto includePos = line.find("#include");
            if (includePos == std::string::npos) continue;
            auto commentPos = line.find("//");
            if (commentPos != std::string::npos && commentPos < includePos) continue;
            assert(line.find("GLRenderer") == std::string::npos);
            assert(line.find("VulkanRenderer") == std::string::npos);
        }
    }

    std::cout << "[PASS] test_include_firewall_ecs_no_render" << std::endl;
}

void test_include_firewall_physics_no_render() {
    // Verify physics/ source files don't include render headers
    std::string physicsDir = resolveTool("engine/physics");
    if (!fs::exists(physicsDir)) {
        std::cout << "[PASS] test_include_firewall_physics_no_render (skipped - dir not found)" << std::endl;
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(physicsDir)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        if (ext != ".h" && ext != ".cpp") continue;

        std::ifstream file(entry.path().string());
        std::string line;
        while (std::getline(file, line)) {
            auto includePos = line.find("#include");
            if (includePos == std::string::npos) continue;
            auto commentPos = line.find("//");
            if (commentPos != std::string::npos && commentPos < includePos) continue;
            assert(line.find("GLRenderer") == std::string::npos);
            assert(line.find("VulkanRenderer") == std::string::npos);
        }
    }

    std::cout << "[PASS] test_include_firewall_physics_no_render" << std::endl;
}

// ============================================================
// CI Workflow Tests
// ============================================================

void test_crash_reporter_workflow_exists() {
    std::string path = resolveTool(".github/workflows/atlas_crash_reporter.yml");
    assert(fs::exists(path));
    std::cout << "[PASS] test_crash_reporter_workflow_exists" << std::endl;
}
