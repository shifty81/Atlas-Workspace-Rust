// NovaForge Project Validation Tests
// Validates that the novaforge project structure is complete and correct.
// Run via: atlas_runtime --project projects/novaforge/novaforge.atlas --validate-only

#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string projectRoot;

static bool FileExists(const std::string& relativePath) {
    return fs::exists(projectRoot + "/" + relativePath);
}

static bool FileContains(const std::string& relativePath, const std::string& needle) {
    std::ifstream f(projectRoot + "/" + relativePath);
    if (!f.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    return content.find(needle) != std::string::npos;
}

void test_manifest_exists() {
    assert(FileExists("novaforge.atlas"));
    assert(FileContains("novaforge.atlas", "atlas.project.v1"));
    assert(FileContains("novaforge.atlas", "NOVAFORGE"));
    std::cout << "[PASS] test_manifest_exists" << std::endl;
}

void test_plugin_toml_exists() {
    assert(FileExists("Plugin.toml"));
    assert(FileContains("Plugin.toml", "id = \"novaforge\""));
    assert(FileContains("Plugin.toml", "type = \"game\""));
    std::cout << "[PASS] test_plugin_toml_exists" << std::endl;
}

void test_world_graphs_exist() {
    assert(FileExists("worlds/galaxy.worldgraph"));
    assert(FileContains("worlds/galaxy.worldgraph", "atlas.worldgraph.v1"));
    assert(FileExists("worlds/station_interior.tilegraph"));
    assert(FileContains("worlds/station_interior.tilegraph", "atlas.tilegraph.v1"));
    std::cout << "[PASS] test_world_graphs_exist" << std::endl;
}

void test_strategy_graphs_exist() {
    assert(FileExists("strategy/faction_ai.strategygraph"));
    assert(FileContains("strategy/faction_ai.strategygraph", "atlas.strategygraph.v1"));
    std::cout << "[PASS] test_strategy_graphs_exist" << std::endl;
}

void test_conversation_graphs_exist() {
    assert(FileExists("conversations/captain_greeting.conversation"));
    assert(FileContains("conversations/captain_greeting.conversation", "atlas.conversation.v1"));
    std::cout << "[PASS] test_conversation_graphs_exist" << std::endl;
}

void test_behavior_graphs_exist() {
    assert(FileExists("ai/combat.behaviorgraph"));
    assert(FileContains("ai/combat.behaviorgraph", "atlas.behaviorgraph.v1"));
    assert(FileExists("ai/trade.behaviorgraph"));
    assert(FileContains("ai/trade.behaviorgraph", "atlas.behaviorgraph.v1"));
    assert(FileExists("ai/patrol.behaviorgraph"));
    assert(FileContains("ai/patrol.behaviorgraph", "atlas.behaviorgraph.v1"));
    std::cout << "[PASS] test_behavior_graphs_exist" << std::endl;
}

void test_ai_config_exists() {
    assert(FileExists("ai/npc_config.json"));
    assert(FileContains("ai/npc_config.json", "atlas.ai.v1"));
    std::cout << "[PASS] test_ai_config_exists" << std::endl;
}

void test_data_manifest_exists() {
    assert(FileExists("data/manifest.json"));
    assert(FileContains("data/manifest.json", "atlas.data.v1"));
    assert(FileContains("data/manifest.json", "contentRoot"));
    std::cout << "[PASS] test_data_manifest_exists" << std::endl;
}

void test_data_manifest_harvesting_entries() {
    assert(FileContains("data/manifest.json", "gas_types.json"));
    assert(FileContains("data/manifest.json", "ice_types.json"));
    std::cout << "[PASS] test_data_manifest_harvesting_entries" << std::endl;
}

void test_runtime_config_exists() {
    assert(FileExists("config/runtime.json"));
    assert(FileContains("config/runtime.json", "atlas.config.v1"));
    std::cout << "[PASS] test_runtime_config_exists" << std::endl;
}

void test_code_entry_point_exists() {
    assert(FileExists("Code/GameInit.cpp"));
    assert(FileExists("Code/GameInit.h"));
    std::cout << "[PASS] test_code_entry_point_exists" << std::endl;
}

void test_adapters_exist() {
    assert(FileExists("Code/Adapters/ECSAdapter.h"));
    assert(FileExists("Code/Adapters/ECSAdapter.cpp"));
    std::cout << "[PASS] test_adapters_exist" << std::endl;
}

void test_schemas_exist() {
    assert(FileExists("Schemas/novaforge.ship.v1.json"));
    assert(FileExists("Schemas/novaforge.module.v1.json"));
    assert(FileExists("Schemas/novaforge.npc.v1.json"));
    std::cout << "[PASS] test_schemas_exist" << std::endl;
}

void test_assets_directory_exists() {
    assert(fs::is_directory(projectRoot + "/assets"));
    std::cout << "[PASS] test_assets_directory_exists" << std::endl;
}

void test_required_directories() {
    std::vector<std::string> dirs = {
        "worlds", "strategy", "conversations", "ai",
        "data", "config", "assets", "Code", "Code/Adapters",
        "Schemas", "Tests"
    };
    for (const auto& d : dirs) {
        assert(fs::is_directory(projectRoot + "/" + d));
    }
    std::cout << "[PASS] test_required_directories" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        projectRoot = argv[1];
    } else {
        projectRoot = ".";
    }

    std::cout << "NOVAFORGE Project Validation Tests" << std::endl;
    std::cout << "Project root: " << projectRoot << std::endl;
    std::cout << std::endl;

    test_manifest_exists();
    test_plugin_toml_exists();
    test_world_graphs_exist();
    test_strategy_graphs_exist();
    test_conversation_graphs_exist();
    test_behavior_graphs_exist();
    test_ai_config_exists();
    test_data_manifest_exists();
    test_data_manifest_harvesting_entries();
    test_runtime_config_exists();
    test_code_entry_point_exists();
    test_adapters_exist();
    test_schemas_exist();
    test_assets_directory_exists();
    test_required_directories();

    std::cout << std::endl;
    std::cout << "All project validation tests PASSED" << std::endl;
    return 0;
}
