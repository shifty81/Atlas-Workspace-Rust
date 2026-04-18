// Phase 20: DesyncReproducer, QoSScheduler, ServerAssetValidator tests

#include "../engine/sim/DesyncReproducer.h"
#include "../engine/net/QoSScheduler.h"
#include "../engine/assets/ServerAssetValidator.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

using namespace atlas::sim;
using namespace atlas::net;
using namespace atlas::asset;

// =============================================================
// DesyncReproducer Tests
// =============================================================

void test_p20_desync_reproducer_initial_state() {
    DesyncReproducer reproducer;
    assert(reproducer.CaptureCount() == 0);
    assert(reproducer.Captures().empty());
    assert(reproducer.OutputDirectory() == "/tmp/atlas_repro");
    assert(reproducer.EngineVersion() == "dev");
    assert(reproducer.PlatformId() == "unknown");
    std::cout << "  [PASS] test_desync_reproducer_initial_state" << std::endl;
}

void test_p20_desync_reproducer_set_output_dir() {
    DesyncReproducer reproducer;
    reproducer.SetOutputDirectory("/tmp/atlas_test_repro");
    assert(reproducer.OutputDirectory() == "/tmp/atlas_test_repro");
    std::cout << "  [PASS] test_desync_reproducer_set_output_dir" << std::endl;
}

void test_p20_desync_reproducer_set_engine_version() {
    DesyncReproducer reproducer;
    reproducer.SetEngineVersion("1.2.3");
    assert(reproducer.EngineVersion() == "1.2.3");
    std::cout << "  [PASS] test_desync_reproducer_set_engine_version" << std::endl;
}

void test_p20_desync_reproducer_set_platform_id() {
    DesyncReproducer reproducer;
    reproducer.SetPlatformId("linux-x86_64");
    assert(reproducer.PlatformId() == "linux-x86_64");
    std::cout << "  [PASS] test_desync_reproducer_set_platform_id" << std::endl;
}

void test_p20_desync_reproducer_capture() {
    DesyncReproducer reproducer;
    reproducer.SetOutputDirectory("/tmp/atlas_test_phase20");

    std::vector<uint8_t> ecsData = {0x01, 0x02, 0x03, 0x04};
    std::vector<uint8_t> replayData = {0xAA, 0xBB, 0xCC};

    auto capture = reproducer.CaptureDesync(
        100, 0xDEAD, 0xBEEF, ecsData, 60, 42, replayData);

    assert(capture.valid);
    assert(capture.tick == 100);
    assert(capture.localHash == 0xDEAD);
    assert(capture.remoteHash == 0xBEEF);
    assert(!capture.savePath.empty());
    assert(!capture.replayPath.empty());
    assert(!capture.reportPath.empty());
    assert(reproducer.CaptureCount() == 1);

    // Verify files were created
    assert(std::filesystem::exists(capture.savePath));
    assert(std::filesystem::exists(capture.replayPath));
    assert(std::filesystem::exists(capture.reportPath));

    // Clean up
    std::filesystem::remove_all("/tmp/atlas_test_phase20");
    std::cout << "  [PASS] test_desync_reproducer_capture" << std::endl;
}

void test_p20_desync_reproducer_repro_command() {
    DesyncCapture capture;
    capture.savePath = "/tmp/test.asav";
    capture.replayPath = "/tmp/test.rply";
    auto cmd = DesyncReproducer::GenerateReproCommand(capture);
    assert(!cmd.empty());
    assert(cmd.find("--repro") != std::string::npos);
    assert(cmd.find("test.asav") != std::string::npos);
    assert(cmd.find("test.rply") != std::string::npos);
    std::cout << "  [PASS] test_desync_reproducer_repro_command" << std::endl;
}

void test_p20_desync_reproducer_crash_bundle() {
    DesyncReproducer reproducer;
    reproducer.SetOutputDirectory("/tmp/atlas_test_phase20_bundle");
    reproducer.SetEngineVersion("0.5.0");
    reproducer.SetPlatformId("linux-x86_64");

    std::vector<uint8_t> ecsData = {0x01, 0x02};
    std::vector<uint8_t> replayData = {0xAA};

    reproducer.CaptureDesync(50, 0x1111, 0x2222, ecsData, 60, 99, replayData);

    auto bundle = reproducer.BuildCrashBundle(60, 99);
    assert(bundle.valid);
    assert(bundle.engineVersion == "0.5.0");
    assert(bundle.platformId == "linux-x86_64");
    assert(bundle.tickRate == 60);
    assert(bundle.seed == 99);
    assert(!bundle.bundlePath.empty());
    assert(std::filesystem::exists(bundle.bundlePath));
    assert(reproducer.Bundles().size() == 1);

    // Clean up
    std::filesystem::remove_all("/tmp/atlas_test_phase20_bundle");
    std::cout << "  [PASS] test_desync_reproducer_crash_bundle" << std::endl;
}

void test_p20_desync_reproducer_empty_crash_bundle() {
    DesyncReproducer reproducer;
    auto bundle = reproducer.BuildCrashBundle(60, 42);
    assert(!bundle.valid);
    std::cout << "  [PASS] test_desync_reproducer_empty_crash_bundle" << std::endl;
}

// =============================================================
// QoSScheduler Tests
// =============================================================

void test_p20_qos_scheduler_initial_state() {
    QoSScheduler scheduler;
    assert(scheduler.QueueSize() == 0);
    assert(!scheduler.HasPending());
    assert(scheduler.DroppedCount() == 0);
    assert(scheduler.BytesSentThisWindow() == 0);
    assert(scheduler.Congestion() == CongestionState::Clear);
    std::cout << "  [PASS] test_qos_scheduler_initial_state" << std::endl;
}

void test_p20_qos_scheduler_enqueue_dequeue() {
    QoSScheduler scheduler;
    QoSPacket pkt;
    pkt.priority = PacketPriority::Normal;
    pkt.sizeBytes = 100;

    assert(scheduler.Enqueue(pkt));
    assert(scheduler.QueueSize() == 1);
    assert(scheduler.HasPending());

    QoSPacket out;
    assert(scheduler.Dequeue(out));
    assert(out.sizeBytes == 100);
    assert(scheduler.QueueSize() == 0);
    std::cout << "  [PASS] test_qos_scheduler_enqueue_dequeue" << std::endl;
}

void test_p20_qos_scheduler_priority_ordering() {
    QoSScheduler scheduler;

    QoSPacket low, high, critical;
    low.priority = PacketPriority::Low;
    low.sizeBytes = 10;
    high.priority = PacketPriority::High;
    high.sizeBytes = 20;
    critical.priority = PacketPriority::Critical;
    critical.sizeBytes = 30;

    scheduler.Enqueue(low);
    scheduler.Enqueue(high);
    scheduler.Enqueue(critical);

    QoSPacket out;
    scheduler.Dequeue(out);
    assert(out.priority == PacketPriority::Critical);
    scheduler.Dequeue(out);
    assert(out.priority == PacketPriority::High);
    scheduler.Dequeue(out);
    assert(out.priority == PacketPriority::Low);
    std::cout << "  [PASS] test_qos_scheduler_priority_ordering" << std::endl;
}

void test_p20_qos_scheduler_drain_all() {
    QoSScheduler scheduler;

    QoSPacket p1, p2;
    p1.priority = PacketPriority::Low;
    p2.priority = PacketPriority::High;
    scheduler.Enqueue(p1);
    scheduler.Enqueue(p2);

    auto all = scheduler.DrainAll();
    assert(all.size() == 2);
    assert(all[0].priority == PacketPriority::High);
    assert(all[1].priority == PacketPriority::Low);
    assert(scheduler.QueueSize() == 0);
    std::cout << "  [PASS] test_qos_scheduler_drain_all" << std::endl;
}

void test_p20_qos_scheduler_queue_size_limit() {
    QoSScheduler scheduler;
    QoSConfig cfg;
    cfg.maxQueueSize = 2;
    scheduler.Configure(cfg);

    QoSPacket p;
    p.priority = PacketPriority::Normal;
    assert(scheduler.Enqueue(p));
    assert(scheduler.Enqueue(p));
    // Third enqueue should fail (same priority, can't drop lower)
    assert(!scheduler.Enqueue(p));
    assert(scheduler.DroppedCount() == 1);
    assert(scheduler.QueueSize() == 2);
    std::cout << "  [PASS] test_qos_scheduler_queue_size_limit" << std::endl;
}

void test_p20_qos_scheduler_drop_lower_priority() {
    QoSScheduler scheduler;
    QoSConfig cfg;
    cfg.maxQueueSize = 2;
    scheduler.Configure(cfg);

    QoSPacket low, high;
    low.priority = PacketPriority::Low;
    high.priority = PacketPriority::High;

    scheduler.Enqueue(low);
    scheduler.Enqueue(low);
    // High priority should cause a low to be dropped
    assert(scheduler.Enqueue(high));
    assert(scheduler.QueueSize() == 2);
    assert(scheduler.DroppedCount() == 1);
    std::cout << "  [PASS] test_qos_scheduler_drop_lower_priority" << std::endl;
}

void test_p20_qos_scheduler_congestion_detection() {
    QoSScheduler scheduler;
    QoSConfig cfg;
    cfg.bandwidthBudgetBytesPerSec = 1000;
    cfg.congestionThreshold = 0.8f;
    scheduler.Configure(cfg);

    assert(scheduler.Congestion() == CongestionState::Clear);

    scheduler.RecordBytesSent(500);
    assert(scheduler.Congestion() == CongestionState::Clear);

    scheduler.RecordBytesSent(400); // 900 total >= 80% of 1000
    assert(scheduler.Congestion() == CongestionState::Approaching);

    scheduler.RecordBytesSent(200); // 1100 total >= 100% of 1000
    assert(scheduler.Congestion() == CongestionState::Congested);
    std::cout << "  [PASS] test_qos_scheduler_congestion_detection" << std::endl;
}

void test_p20_qos_scheduler_window_reset() {
    QoSScheduler scheduler;
    QoSConfig cfg;
    cfg.bandwidthBudgetBytesPerSec = 1000;
    scheduler.Configure(cfg);

    scheduler.RecordBytesSent(500);
    assert(scheduler.BytesSentThisWindow() == 500);

    // Advance past 1 second window
    scheduler.UpdateWindow(1001.0f);
    assert(scheduler.BytesSentThisWindow() == 0);
    std::cout << "  [PASS] test_qos_scheduler_window_reset" << std::endl;
}

void test_p20_qos_scheduler_reset() {
    QoSScheduler scheduler;
    QoSPacket p;
    p.priority = PacketPriority::Normal;
    scheduler.Enqueue(p);
    scheduler.RecordBytesSent(100);

    scheduler.Reset();
    assert(scheduler.QueueSize() == 0);
    assert(scheduler.BytesSentThisWindow() == 0);
    assert(scheduler.DroppedCount() == 0);
    std::cout << "  [PASS] test_qos_scheduler_reset" << std::endl;
}

void test_p20_qos_scheduler_dequeue_empty() {
    QoSScheduler scheduler;
    QoSPacket out;
    assert(!scheduler.Dequeue(out));
    std::cout << "  [PASS] test_qos_scheduler_dequeue_empty" << std::endl;
}

// =============================================================
// ServerAssetValidator Tests
// =============================================================

void test_p20_server_validator_register_asset() {
    ServerAssetValidator validator;
    validator.RegisterAsset("mesh_01", 0xAABB, 1);
    validator.RegisterAsset("tex_01", 0xCCDD, 2);
    assert(validator.ManifestSize() == 2);
    assert(validator.HasAsset("mesh_01"));
    assert(validator.HasAsset("tex_01"));
    assert(!validator.HasAsset("unknown"));
    std::cout << "  [PASS] test_server_validator_register_asset" << std::endl;
}

void test_p20_server_validator_validate_hash_pass() {
    ServerAssetValidator validator;
    validator.RegisterAsset("asset_a", 12345);

    auto result = validator.ValidateHash("asset_a", 12345);
    assert(result.accepted);
    assert(result.assetId == "asset_a");
    std::cout << "  [PASS] test_server_validator_validate_hash_pass" << std::endl;
}

void test_p20_server_validator_validate_hash_fail() {
    ServerAssetValidator validator;
    validator.RegisterAsset("asset_a", 12345);

    auto result = validator.ValidateHash("asset_a", 99999);
    assert(!result.accepted);
    assert(!result.reason.empty());
    std::cout << "  [PASS] test_server_validator_validate_hash_fail" << std::endl;
}

void test_p20_server_validator_validate_unknown_asset() {
    ServerAssetValidator validator;
    auto result = validator.ValidateHash("nonexistent", 12345);
    assert(!result.accepted);
    assert(result.reason.find("not in server manifest") != std::string::npos);
    std::cout << "  [PASS] test_server_validator_validate_unknown_asset" << std::endl;
}

void test_p20_server_validator_manifest() {
    ServerAssetValidator validator;
    validator.RegisterAsset("a", 100, 1);
    validator.RegisterAsset("b", 200, 2);
    validator.RegisterAsset("c", 300, 3);

    auto manifest = validator.Manifest();
    assert(manifest.size() == 3);

    // All entries should be present
    bool foundA = false, foundB = false, foundC = false;
    for (const auto& e : manifest) {
        if (e.assetId == "a" && e.expectedHash == 100 && e.expectedVersion == 1) foundA = true;
        if (e.assetId == "b" && e.expectedHash == 200 && e.expectedVersion == 2) foundB = true;
        if (e.assetId == "c" && e.expectedHash == 300 && e.expectedVersion == 3) foundC = true;
    }
    assert(foundA && foundB && foundC);
    std::cout << "  [PASS] test_server_validator_manifest" << std::endl;
}

void test_p20_server_validator_overwrite_entry() {
    ServerAssetValidator validator;
    validator.RegisterAsset("asset_x", 100);
    validator.RegisterAsset("asset_x", 200);
    assert(validator.ManifestSize() == 1);

    auto result = validator.ValidateHash("asset_x", 200);
    assert(result.accepted);
    std::cout << "  [PASS] test_server_validator_overwrite_entry" << std::endl;
}

void test_p20_server_validator_validate_missing_file() {
    ServerAssetValidator validator;
    validator.RegisterAsset("asset_y", 555);

    auto result = validator.ValidateAsset("asset_y", "/tmp/nonexistent_file_phase20.atlasb");
    assert(!result.accepted);
    assert(result.reason.find("not found") != std::string::npos);
    std::cout << "  [PASS] test_server_validator_validate_missing_file" << std::endl;
}

// =============================================================
// Runner
// =============================================================

void register_next_tasks_phase20() {
    std::cout << "\n--- Phase 20: DesyncReproducer ---" << std::endl;
    test_p20_desync_reproducer_initial_state();
    test_p20_desync_reproducer_set_output_dir();
    test_p20_desync_reproducer_set_engine_version();
    test_p20_desync_reproducer_set_platform_id();
    test_p20_desync_reproducer_capture();
    test_p20_desync_reproducer_repro_command();
    test_p20_desync_reproducer_crash_bundle();
    test_p20_desync_reproducer_empty_crash_bundle();

    std::cout << "\n--- Phase 20: QoSScheduler ---" << std::endl;
    test_p20_qos_scheduler_initial_state();
    test_p20_qos_scheduler_enqueue_dequeue();
    test_p20_qos_scheduler_priority_ordering();
    test_p20_qos_scheduler_drain_all();
    test_p20_qos_scheduler_queue_size_limit();
    test_p20_qos_scheduler_drop_lower_priority();
    test_p20_qos_scheduler_congestion_detection();
    test_p20_qos_scheduler_window_reset();
    test_p20_qos_scheduler_reset();
    test_p20_qos_scheduler_dequeue_empty();

    std::cout << "\n--- Phase 20: ServerAssetValidator ---" << std::endl;
    test_p20_server_validator_register_asset();
    test_p20_server_validator_validate_hash_pass();
    test_p20_server_validator_validate_hash_fail();
    test_p20_server_validator_validate_unknown_asset();
    test_p20_server_validator_manifest();
    test_p20_server_validator_overwrite_entry();
    test_p20_server_validator_validate_missing_file();
}
