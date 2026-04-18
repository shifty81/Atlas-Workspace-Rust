#include <iostream>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
#include <filesystem>

#include "../engine/core/Engine.h"
#include "../engine/core/Logger.h"
#include "../engine/core/contract/DeterministicRNG.h"
#include "../engine/project/ProjectManager.h"

// ============================================================
// Task 1: TileEditor main loop uses Engine::Run()
// ============================================================

void test_tileeditor_engine_runs_with_max_ticks() {
    // The tile editor now delegates to Engine::Run(). Verify that an
    // engine configured with EngineMode::Editor + maxTicks actually
    // terminates after the expected number of ticks, which is the
    // same mechanism the tile editor main loop relies on.
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Editor;
    cfg.headless = true;
    cfg.maxTicks = 5;
    cfg.tickRate = 60;

    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.GetScheduler().SetFramePacing(false);
    engine.Run();

    // Engine should have stopped after maxTicks
    assert(!engine.Running());
    std::cout << "[PASS] test_tileeditor_engine_runs_with_max_ticks" << std::endl;
}

void test_tileeditor_editor_mode_capabilities() {
    // The tile editor runs in Editor mode which should have asset write
    // and hot reload capabilities.
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Editor;
    cfg.headless = true;
    cfg.maxTicks = 1;

    atlas::Engine engine(cfg);
    engine.InitCore();

    assert(engine.Can(atlas::Capability::AssetWrite));
    assert(engine.Can(atlas::Capability::HotReload));
    assert(engine.Can(atlas::Capability::GraphEdit));

    engine.Shutdown();
    std::cout << "[PASS] test_tileeditor_editor_mode_capabilities" << std::endl;
}

// ============================================================
// Task 2: Client and Server CLI argument support
// ============================================================

void test_client_project_loading() {
    // Verify that ProjectManager can load the sample project descriptor.
    const std::string samplePath = std::string(CMAKE_SOURCE_DIR)
        + "/projects/atlas-sample/sample.atlas";

    auto& pm = atlas::project::ProjectManager::Get();
    bool loaded = pm.Load(samplePath);
    assert(loaded);
    assert(pm.Descriptor().name == "Atlas Sample");
    assert(pm.Descriptor().runtime.tickRate == 60);
    assert(pm.Descriptor().runtime.maxPlayers == 1);
    assert(pm.IsLoaded());

    std::cout << "[PASS] test_client_project_loading" << std::endl;
}

void test_server_project_config_applied() {
    // Verify that project config can influence engine configuration.
    const std::string samplePath = std::string(CMAKE_SOURCE_DIR)
        + "/projects/atlas-sample/sample.atlas";

    auto& pm = atlas::project::ProjectManager::Get();
    bool loaded = pm.Load(samplePath);
    assert(loaded);

    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Server;
    cfg.tickRate = pm.Descriptor().runtime.tickRate;
    cfg.assetRoot = pm.Descriptor().assets.root;

    assert(cfg.tickRate == 60);
    assert(cfg.assetRoot == "assets");

    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    assert(engine.Can(atlas::Capability::NetAuthority));
    assert(!engine.Can(atlas::Capability::Rendering));

    engine.Shutdown();
    std::cout << "[PASS] test_server_project_config_applied" << std::endl;
}

void test_client_engine_with_project_tick_rate() {
    const std::string samplePath = std::string(CMAKE_SOURCE_DIR)
        + "/projects/atlas-sample/sample.atlas";

    auto& pm = atlas::project::ProjectManager::Get();
    pm.Load(samplePath);

    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Client;
    cfg.headless = true;
    cfg.maxTicks = 3;
    cfg.tickRate = pm.Descriptor().runtime.tickRate;

    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.Run();

    assert(!engine.Running());
    std::cout << "[PASS] test_client_engine_with_project_tick_rate" << std::endl;
}

// ============================================================
// Task 3: Sample project content
// ============================================================

void test_sample_project_has_plugin_toml() {
    const std::string pluginPath = std::string(CMAKE_SOURCE_DIR)
        + "/projects/atlas-sample/Plugin.toml";
    assert(std::filesystem::exists(pluginPath));

    std::ifstream f(pluginPath);
    assert(f.good());
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assert(content.find("atlas-sample") != std::string::npos);
    assert(content.find("Atlas Sample") != std::string::npos);

    std::cout << "[PASS] test_sample_project_has_plugin_toml" << std::endl;
}

void test_sample_project_has_runtime_config() {
    const std::string configPath = std::string(CMAKE_SOURCE_DIR)
        + "/projects/atlas-sample/config/runtime.json";
    assert(std::filesystem::exists(configPath));

    std::ifstream f(configPath);
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assert(content.find("atlas.config.v1") != std::string::npos);
    assert(content.find("\"deterministic\": true") != std::string::npos);

    std::cout << "[PASS] test_sample_project_has_runtime_config" << std::endl;
}

void test_sample_project_has_entities_data() {
    const std::string dataPath = std::string(CMAKE_SOURCE_DIR)
        + "/projects/atlas-sample/data/entities.json";
    assert(std::filesystem::exists(dataPath));

    std::ifstream f(dataPath);
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assert(content.find("atlas.entities.v1") != std::string::npos);
    assert(content.find("player_spawn") != std::string::npos);
    assert(content.find("terrain_origin") != std::string::npos);

    std::cout << "[PASS] test_sample_project_has_entities_data" << std::endl;
}

void test_sample_project_has_worldgraph() {
    const std::string worldPath = std::string(CMAKE_SOURCE_DIR)
        + "/projects/atlas-sample/worlds/terrain.worldgraph";
    assert(std::filesystem::exists(worldPath));

    std::ifstream f(worldPath);
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assert(content.find("atlas.worldgraph.v1") != std::string::npos);

    std::cout << "[PASS] test_sample_project_has_worldgraph" << std::endl;
}

// ============================================================
// Task 4: Cross-platform determinism verification enabled
// ============================================================

void test_determinism_config_cross_platform_enabled() {
    const std::string configPath = std::string(CMAKE_SOURCE_DIR)
        + "/engine/core/contract/determinism.json";

    std::ifstream f(configPath);
    assert(f.good());
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assert(content.find("\"crossPlatformVerification\": true") != std::string::npos);

    std::cout << "[PASS] test_determinism_config_cross_platform_enabled" << std::endl;
}

void test_determinism_config_golden_replay_gate() {
    const std::string configPath = std::string(CMAKE_SOURCE_DIR)
        + "/engine/core/contract/determinism.json";

    std::ifstream f(configPath);
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assert(content.find("\"goldenReplayGate\": true") != std::string::npos);
    assert(content.find("\"contractBotEnabled\": true") != std::string::npos);

    std::cout << "[PASS] test_determinism_config_golden_replay_gate" << std::endl;
}

// ============================================================
// Runner
// ============================================================

void run_next_tasks_phase4_tests() {
    std::cout << "\n--- Next Tasks Phase 4: TileEditor + CLI + Sample ---" << std::endl;

    // Task 1: TileEditor
    test_tileeditor_engine_runs_with_max_ticks();
    test_tileeditor_editor_mode_capabilities();

    // Task 2: Client/Server CLI
    test_client_project_loading();
    test_server_project_config_applied();
    test_client_engine_with_project_tick_rate();

    // Task 3: Sample project content
    test_sample_project_has_plugin_toml();
    test_sample_project_has_runtime_config();
    test_sample_project_has_entities_data();
    test_sample_project_has_worldgraph();

    // Task 4: Determinism config
    test_determinism_config_cross_platform_enabled();
    test_determinism_config_golden_replay_gate();
}
