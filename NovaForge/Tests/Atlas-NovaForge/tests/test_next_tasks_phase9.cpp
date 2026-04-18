#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cstdio>
#include <filesystem>
#include <fstream>
#if defined(_WIN32)
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

#include "../engine/sim/DesyncReproducer.h"
#include "../engine/sim/ReplayDivergenceInspector.h"
#include "../engine/sim/StateHasher.h"
#include "../engine/sim/ReplayRecorder.h"
#include "../engine/net/QoSScheduler.h"

// ============================================================
// Task 1: CMakePresets.json validation
// ============================================================

void test_cmake_presets_file_exists() {
    std::string path = std::string(CMAKE_SOURCE_DIR) + "/CMakePresets.json";
    assert(std::filesystem::exists(path));

    std::ifstream in(path);
    assert(in.is_open());

    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("\"version\"") != std::string::npos);
    assert(content.find("\"configurePresets\"") != std::string::npos);
    assert(content.find("\"buildPresets\"") != std::string::npos);
    assert(content.find("\"testPresets\"") != std::string::npos);

    std::cout << "[PASS] test_cmake_presets_file_exists" << std::endl;
}

void test_cmake_presets_has_debug() {
    std::string path = std::string(CMAKE_SOURCE_DIR) + "/CMakePresets.json";
    std::ifstream in(path);
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("\"name\": \"debug\"") != std::string::npos);
    assert(content.find("\"CMAKE_BUILD_TYPE\": \"Debug\"") != std::string::npos);

    std::cout << "[PASS] test_cmake_presets_has_debug" << std::endl;
}

void test_cmake_presets_has_release() {
    std::string path = std::string(CMAKE_SOURCE_DIR) + "/CMakePresets.json";
    std::ifstream in(path);
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("\"name\": \"release\"") != std::string::npos);
    assert(content.find("\"CMAKE_BUILD_TYPE\": \"Release\"") != std::string::npos);

    std::cout << "[PASS] test_cmake_presets_has_release" << std::endl;
}

void test_cmake_presets_has_ci() {
    std::string path = std::string(CMAKE_SOURCE_DIR) + "/CMakePresets.json";
    std::ifstream in(path);
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("\"name\": \"ci\"") != std::string::npos);
    assert(content.find("ATLAS_DETERMINISM_STRICT") != std::string::npos);

    std::cout << "[PASS] test_cmake_presets_has_ci" << std::endl;
}

void test_cmake_presets_has_development() {
    std::string path = std::string(CMAKE_SOURCE_DIR) + "/CMakePresets.json";
    std::ifstream in(path);
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("\"name\": \"development\"") != std::string::npos);
    assert(content.find("\"CMAKE_BUILD_TYPE\": \"RelWithDebInfo\"") != std::string::npos);

    std::cout << "[PASS] test_cmake_presets_has_development" << std::endl;
}

// ============================================================
// Task 2: Determinism Crash Report Bundle
// ============================================================

void test_crash_bundle_empty_captures() {
    atlas::sim::DesyncReproducer reproducer;
    auto bundle = reproducer.BuildCrashBundle(60, 12345);
    assert(!bundle.valid);
    assert(reproducer.Bundles().empty());

    std::cout << "[PASS] test_crash_bundle_empty_captures" << std::endl;
}

void test_crash_bundle_engine_version() {
    atlas::sim::DesyncReproducer reproducer;
    assert(reproducer.EngineVersion() == "dev");

    reproducer.SetEngineVersion("1.0.0");
    assert(reproducer.EngineVersion() == "1.0.0");

    reproducer.SetPlatformId("linux-x86_64");
    assert(reproducer.PlatformId() == "linux-x86_64");

    std::cout << "[PASS] test_crash_bundle_engine_version" << std::endl;
}

void test_crash_bundle_from_capture() {
    std::string tmpDir = (std::filesystem::temp_directory_path() / ("atlas_crash_test_" + std::to_string(::getpid()))).string();
    std::filesystem::create_directories(tmpDir);

    atlas::sim::DesyncReproducer reproducer;
    reproducer.SetOutputDirectory(tmpDir);
    reproducer.SetEngineVersion("2.1.0");
    reproducer.SetPlatformId("linux-x86_64");

    std::vector<uint8_t> ecsData = {1, 2, 3, 4};
    std::vector<uint8_t> replayData = {5, 6, 7, 8};

    auto capture = reproducer.CaptureDesync(100, 0xAABB, 0xCCDD,
                                             ecsData, 60, 42, replayData);
    assert(capture.valid);

    auto bundle = reproducer.BuildCrashBundle(60, 42);
    assert(bundle.valid);
    assert(bundle.engineVersion == "2.1.0");
    assert(bundle.platformId == "linux-x86_64");
    assert(bundle.tickRate == 60);
    assert(bundle.seed == 42);
    assert(bundle.capture.tick == 100);
    assert(!bundle.bundlePath.empty());
    assert(std::filesystem::exists(bundle.bundlePath));
    assert(reproducer.Bundles().size() == 1);

    // Verify manifest content
    std::ifstream in(bundle.bundlePath);
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("atlas_crash_bundle_v1") != std::string::npos);
    assert(content.find("engine_version=2.1.0") != std::string::npos);
    assert(content.find("platform=linux-x86_64") != std::string::npos);
    assert(content.find("tick=100") != std::string::npos);
    assert(content.find("tick_rate=60") != std::string::npos);

    std::filesystem::remove_all(tmpDir);
    std::cout << "[PASS] test_crash_bundle_from_capture" << std::endl;
}

void test_crash_bundle_at_index() {
    std::string tmpDir = (std::filesystem::temp_directory_path() / ("atlas_crash_idx_" + std::to_string(::getpid()))).string();
    std::filesystem::create_directories(tmpDir);

    atlas::sim::DesyncReproducer reproducer;
    reproducer.SetOutputDirectory(tmpDir);

    std::vector<uint8_t> data = {1, 2};

    reproducer.CaptureDesync(10, 0x10, 0x20, data, 30, 1, data);
    reproducer.CaptureDesync(20, 0x30, 0x40, data, 30, 1, data);

    assert(reproducer.CaptureCount() == 2);

    auto bundle0 = reproducer.BuildCrashBundleAt(0, 30, 1);
    assert(bundle0.valid);
    assert(bundle0.capture.tick == 10);

    auto bundle1 = reproducer.BuildCrashBundleAt(1, 30, 1);
    assert(bundle1.valid);
    assert(bundle1.capture.tick == 20);

    // Out of bounds
    auto bundleOob = reproducer.BuildCrashBundleAt(99, 30, 1);
    assert(!bundleOob.valid);

    assert(reproducer.Bundles().size() == 2);

    std::filesystem::remove_all(tmpDir);
    std::cout << "[PASS] test_crash_bundle_at_index" << std::endl;
}

void test_crash_bundle_contains_repro_command() {
    std::string tmpDir = (std::filesystem::temp_directory_path() / ("atlas_crash_repro_" + std::to_string(::getpid()))).string();
    std::filesystem::create_directories(tmpDir);

    atlas::sim::DesyncReproducer reproducer;
    reproducer.SetOutputDirectory(tmpDir);

    std::vector<uint8_t> data = {9, 8, 7};
    reproducer.CaptureDesync(50, 0xFF, 0xEE, data, 60, 99, data);

    auto bundle = reproducer.BuildCrashBundle(60, 99);
    assert(bundle.valid);

    std::ifstream in(bundle.bundlePath);
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("repro=") != std::string::npos);
    assert(content.find("--repro") != std::string::npos);

    std::filesystem::remove_all(tmpDir);
    std::cout << "[PASS] test_crash_bundle_contains_repro_command" << std::endl;
}

// ============================================================
// Task 3: Replay Divergence Minimizer
// ============================================================

void test_minimizer_no_divergence() {
    std::vector<atlas::sim::ReplayFrame> a, b;
    for (uint32_t i = 0; i < 100; ++i) {
        atlas::sim::ReplayFrame f;
        f.tick = i;
        f.stateHash = 1000 + i;
        a.push_back(f);
        b.push_back(f);
    }

    auto result = atlas::sim::ReplayDivergenceInspector::MinimizeDivergence(a, b);
    assert(!result.minimized);
    assert(result.firstDivergentTick == -1);

    std::cout << "[PASS] test_minimizer_no_divergence" << std::endl;
}

void test_minimizer_first_tick_divergence() {
    std::vector<atlas::sim::ReplayFrame> a, b;
    for (uint32_t i = 0; i < 50; ++i) {
        atlas::sim::ReplayFrame fa, fb;
        fa.tick = i;
        fb.tick = i;
        fa.stateHash = 100 + i;
        fb.stateHash = (i == 0) ? 999 : (100 + i);
        a.push_back(fa);
        b.push_back(fb);
    }

    auto result = atlas::sim::ReplayDivergenceInspector::MinimizeDivergence(a, b);
    assert(result.minimized);
    assert(result.firstDivergentTick == 0);
    assert(result.lastMatchingTick == -1);
    assert(result.windowSize == 1);
    assert(result.iterationsUsed > 0);

    std::cout << "[PASS] test_minimizer_first_tick_divergence" << std::endl;
}

void test_minimizer_mid_stream_divergence() {
    std::vector<atlas::sim::ReplayFrame> a, b;
    const uint32_t divergeAt = 42;
    for (uint32_t i = 0; i < 100; ++i) {
        atlas::sim::ReplayFrame fa, fb;
        fa.tick = i;
        fb.tick = i;
        fa.stateHash = 500 + i;
        fb.stateHash = (i >= divergeAt) ? (700 + i) : (500 + i);
        a.push_back(fa);
        b.push_back(fb);
    }

    auto result = atlas::sim::ReplayDivergenceInspector::MinimizeDivergence(a, b);
    assert(result.minimized);
    assert(result.firstDivergentTick == 42);
    assert(result.lastMatchingTick == 41);
    assert(result.windowSize == 1);
    assert(result.iterationsUsed > 0);
    assert(result.iterationsUsed <= 10); // log2(100) ~ 7

    std::cout << "[PASS] test_minimizer_mid_stream_divergence" << std::endl;
}

void test_minimizer_last_tick_divergence() {
    std::vector<atlas::sim::ReplayFrame> a, b;
    for (uint32_t i = 0; i < 64; ++i) {
        atlas::sim::ReplayFrame fa, fb;
        fa.tick = i;
        fb.tick = i;
        fa.stateHash = 200 + i;
        fb.stateHash = (i == 63) ? 999 : (200 + i);
        a.push_back(fa);
        b.push_back(fb);
    }

    auto result = atlas::sim::ReplayDivergenceInspector::MinimizeDivergence(a, b);
    assert(result.minimized);
    assert(result.firstDivergentTick == 63);
    assert(result.lastMatchingTick == 62);

    std::cout << "[PASS] test_minimizer_last_tick_divergence" << std::endl;
}

void test_minimizer_empty_streams() {
    std::vector<atlas::sim::ReplayFrame> a, b;
    auto result = atlas::sim::ReplayDivergenceInspector::MinimizeDivergence(a, b);
    assert(!result.minimized);
    assert(result.firstDivergentTick == -1);

    std::cout << "[PASS] test_minimizer_empty_streams" << std::endl;
}

void test_minimizer_from_hashers() {
    atlas::sim::StateHasher local, remote;
    local.Reset(1);
    remote.Reset(1);

    // First 20 ticks: same state/inputs
    for (uint64_t t = 0; t < 20; ++t) {
        std::vector<uint8_t> state = {static_cast<uint8_t>(t)};
        std::vector<uint8_t> input = {static_cast<uint8_t>(t + 100)};
        local.AdvanceTick(t, state, input);
        remote.AdvanceTick(t, state, input);
    }

    // Tick 20: diverge
    {
        std::vector<uint8_t> state = {20};
        std::vector<uint8_t> inputA = {120};
        std::vector<uint8_t> inputB = {255};
        local.AdvanceTick(20, state, inputA);
        remote.AdvanceTick(20, state, inputB);
    }

    auto result = atlas::sim::ReplayDivergenceInspector::MinimizeDivergenceFromHashers(local, remote);
    assert(result.minimized);
    assert(result.firstDivergentTick == 20);
    assert(result.lastMatchingTick == 19);
    assert(result.iterationsUsed > 0);

    std::cout << "[PASS] test_minimizer_from_hashers" << std::endl;
}

void test_minimizer_single_frame() {
    std::vector<atlas::sim::ReplayFrame> a(1), b(1);
    a[0].tick = 0;
    a[0].stateHash = 42;
    b[0].tick = 0;
    b[0].stateHash = 99;

    auto result = atlas::sim::ReplayDivergenceInspector::MinimizeDivergence(a, b);
    assert(result.minimized);
    assert(result.firstDivergentTick == 0);
    assert(result.lastMatchingTick == -1);

    std::cout << "[PASS] test_minimizer_single_frame" << std::endl;
}

// ============================================================
// Task 4: QoS Packet Scheduler
// ============================================================

void test_qos_default_config() {
    atlas::net::QoSScheduler scheduler;
    assert(scheduler.QueueSize() == 0);
    assert(!scheduler.HasPending());
    assert(scheduler.DroppedCount() == 0);
    assert(scheduler.Congestion() == atlas::net::CongestionState::Clear);

    std::cout << "[PASS] test_qos_default_config" << std::endl;
}

void test_qos_enqueue_dequeue() {
    atlas::net::QoSScheduler scheduler;
    atlas::net::QoSConfig config;
    config.maxQueueSize = 10;
    scheduler.Configure(config);

    atlas::net::QoSPacket pkt;
    pkt.priority = atlas::net::PacketPriority::Normal;
    pkt.sizeBytes = 100;

    assert(scheduler.Enqueue(pkt));
    assert(scheduler.QueueSize() == 1);
    assert(scheduler.HasPending());

    atlas::net::QoSPacket out;
    assert(scheduler.Dequeue(out));
    assert(out.sizeBytes == 100);
    assert(out.id >= 1);
    assert(scheduler.QueueSize() == 0);

    std::cout << "[PASS] test_qos_enqueue_dequeue" << std::endl;
}

void test_qos_priority_ordering() {
    atlas::net::QoSScheduler scheduler;
    atlas::net::QoSConfig config;
    config.maxQueueSize = 10;
    scheduler.Configure(config);

    atlas::net::QoSPacket low, normal, high, critical;
    low.priority = atlas::net::PacketPriority::Low;
    low.sizeBytes = 10;
    normal.priority = atlas::net::PacketPriority::Normal;
    normal.sizeBytes = 20;
    high.priority = atlas::net::PacketPriority::High;
    high.sizeBytes = 30;
    critical.priority = atlas::net::PacketPriority::Critical;
    critical.sizeBytes = 40;

    // Enqueue in worst-first order
    scheduler.Enqueue(low);
    scheduler.Enqueue(normal);
    scheduler.Enqueue(high);
    scheduler.Enqueue(critical);

    assert(scheduler.QueueSize() == 4);

    // Dequeue should give Critical first, then High, Normal, Low
    atlas::net::QoSPacket out;
    assert(scheduler.Dequeue(out));
    assert(out.sizeBytes == 40); // Critical

    assert(scheduler.Dequeue(out));
    assert(out.sizeBytes == 30); // High

    assert(scheduler.Dequeue(out));
    assert(out.sizeBytes == 20); // Normal

    assert(scheduler.Dequeue(out));
    assert(out.sizeBytes == 10); // Low

    assert(!scheduler.Dequeue(out)); // empty

    std::cout << "[PASS] test_qos_priority_ordering" << std::endl;
}

void test_qos_queue_overflow_drops_low_priority() {
    atlas::net::QoSScheduler scheduler;
    atlas::net::QoSConfig config;
    config.maxQueueSize = 2;
    scheduler.Configure(config);

    atlas::net::QoSPacket low, normal, high;
    low.priority = atlas::net::PacketPriority::Low;
    low.sizeBytes = 10;
    normal.priority = atlas::net::PacketPriority::Normal;
    normal.sizeBytes = 20;
    high.priority = atlas::net::PacketPriority::High;
    high.sizeBytes = 30;

    scheduler.Enqueue(low);
    scheduler.Enqueue(normal);
    assert(scheduler.QueueSize() == 2);

    // Enqueue high should evict the low-priority packet
    assert(scheduler.Enqueue(high));
    assert(scheduler.QueueSize() == 2);
    assert(scheduler.DroppedCount() == 1);

    // Queue should contain Normal and High
    auto drained = scheduler.DrainAll();
    assert(drained.size() == 2);
    assert(drained[0].sizeBytes == 30); // High
    assert(drained[1].sizeBytes == 20); // Normal

    std::cout << "[PASS] test_qos_queue_overflow_drops_low_priority" << std::endl;
}

void test_qos_congestion_detection() {
    atlas::net::QoSScheduler scheduler;
    atlas::net::QoSConfig config;
    config.bandwidthBudgetBytesPerSec = 1000;
    config.congestionThreshold = 0.8f;
    scheduler.Configure(config);

    assert(scheduler.Congestion() == atlas::net::CongestionState::Clear);

    // Send 700 bytes — still Clear (70%)
    scheduler.RecordBytesSent(700);
    assert(scheduler.Congestion() == atlas::net::CongestionState::Clear);

    // Send 100 more → 800 bytes (80%) — Approaching
    scheduler.RecordBytesSent(100);
    assert(scheduler.Congestion() == atlas::net::CongestionState::Approaching);

    // Send 200 more → 1000 bytes (100%) — Congested
    scheduler.RecordBytesSent(200);
    assert(scheduler.Congestion() == atlas::net::CongestionState::Congested);

    std::cout << "[PASS] test_qos_congestion_detection" << std::endl;
}

void test_qos_window_reset() {
    atlas::net::QoSScheduler scheduler;
    atlas::net::QoSConfig config;
    config.bandwidthBudgetBytesPerSec = 1000;
    scheduler.Configure(config);

    scheduler.RecordBytesSent(500);
    assert(scheduler.BytesSentThisWindow() == 500);

    // Advance 1 full second
    scheduler.UpdateWindow(1000.0f);
    assert(scheduler.BytesSentThisWindow() == 0);
    assert(scheduler.Congestion() == atlas::net::CongestionState::Clear);

    std::cout << "[PASS] test_qos_window_reset" << std::endl;
}

void test_qos_drain_all() {
    atlas::net::QoSScheduler scheduler;
    atlas::net::QoSConfig config;
    config.maxQueueSize = 100;
    scheduler.Configure(config);

    for (int i = 0; i < 5; ++i) {
        atlas::net::QoSPacket pkt;
        pkt.priority = static_cast<atlas::net::PacketPriority>(i % 4);
        pkt.sizeBytes = 100 + static_cast<uint32_t>(i);
        scheduler.Enqueue(pkt);
    }

    assert(scheduler.QueueSize() == 5);

    auto all = scheduler.DrainAll();
    assert(all.size() == 5);
    assert(scheduler.QueueSize() == 0);

    // Should be sorted by priority (highest first)
    for (size_t i = 1; i < all.size(); ++i) {
        assert(static_cast<uint8_t>(all[i - 1].priority) >=
               static_cast<uint8_t>(all[i].priority));
    }

    std::cout << "[PASS] test_qos_drain_all" << std::endl;
}

void test_qos_reset() {
    atlas::net::QoSScheduler scheduler;
    atlas::net::QoSConfig config;
    config.maxQueueSize = 10;
    config.bandwidthBudgetBytesPerSec = 5000;
    scheduler.Configure(config);

    atlas::net::QoSPacket pkt;
    pkt.priority = atlas::net::PacketPriority::Normal;
    pkt.sizeBytes = 100;
    scheduler.Enqueue(pkt);
    scheduler.RecordBytesSent(500);

    scheduler.Reset();
    assert(scheduler.QueueSize() == 0);
    assert(scheduler.BytesSentThisWindow() == 0);
    assert(scheduler.DroppedCount() == 0);
    assert(scheduler.Congestion() == atlas::net::CongestionState::Clear);

    std::cout << "[PASS] test_qos_reset" << std::endl;
}

// ============================================================
// Runner function — called from main.cpp
// ============================================================

void run_next_tasks_phase9_tests() {
    std::cout << "\n--- Phase 9: CMakePresets Validation ---" << std::endl;
    test_cmake_presets_file_exists();
    test_cmake_presets_has_debug();
    test_cmake_presets_has_release();
    test_cmake_presets_has_ci();
    test_cmake_presets_has_development();

    std::cout << "\n--- Phase 9: Determinism Crash Report Bundle ---" << std::endl;
    test_crash_bundle_empty_captures();
    test_crash_bundle_engine_version();
    test_crash_bundle_from_capture();
    test_crash_bundle_at_index();
    test_crash_bundle_contains_repro_command();

    std::cout << "\n--- Phase 9: Replay Divergence Minimizer ---" << std::endl;
    test_minimizer_no_divergence();
    test_minimizer_first_tick_divergence();
    test_minimizer_mid_stream_divergence();
    test_minimizer_last_tick_divergence();
    test_minimizer_empty_streams();
    test_minimizer_from_hashers();
    test_minimizer_single_frame();

    std::cout << "\n--- Phase 9: QoS Packet Scheduler ---" << std::endl;
    test_qos_default_config();
    test_qos_enqueue_dequeue();
    test_qos_priority_ordering();
    test_qos_queue_overflow_drops_low_priority();
    test_qos_congestion_detection();
    test_qos_window_reset();
    test_qos_drain_all();
    test_qos_reset();
}
