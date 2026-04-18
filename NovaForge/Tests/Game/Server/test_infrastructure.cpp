/**
 * Infrastructure tests
 */

#include "test_framework.h"
#include "systems/capacitor_system.h"
#include "systems/shield_recharge_system.h"
#include "systems/weapon_system.h"
#include "systems/targeting_system.h"
#include "data/ship_database.h"
#include "data/wormhole_database.h"
#include "systems/wormhole_system.h"
#include "systems/fleet_system.h"
#include "systems/mission_system.h"
#include "systems/skill_system.h"
#include "systems/module_system.h"
#include "systems/inventory_system.h"
#include "systems/loot_system.h"
#include "systems/drone_system.h"
#include "systems/insurance_system.h"
#include "systems/corporation_system.h"
#include "systems/bounty_system.h"
#include "systems/market_system.h"
#include "systems/contract_system.h"
#include "systems/pi_system.h"
#include "systems/manufacturing_system.h"
#include "systems/research_system.h"
#include "systems/chat_system.h"
#include "systems/character_creation_system.h"
#include "systems/tournament_system.h"
#include "systems/leaderboard_system.h"
#include "data/world_persistence.h"
#include "data/npc_database.h"
#include "systems/movement_system.h"
#include "systems/station_system.h"
#include "systems/wreck_salvage_system.h"
#include "systems/fleet_morale_system.h"
#include "systems/captain_personality_system.h"
#include "systems/fleet_chatter_system.h"
#include "systems/warp_anomaly_system.h"
#include "systems/captain_relationship_system.h"
#include "systems/emotional_arc_system.h"
#include "systems/fleet_cargo_system.h"
#include "systems/tactical_overlay_system.h"
#include "systems/combat_system.h"
#include "systems/ai_system.h"
#include "systems/mining_system.h"
#include "systems/fleet_formation_system.h"
#include "systems/captain_memory_system.h"
#include "systems/ship_fitting_system.h"
#include "systems/warp_cinematic_system.h"
#include "systems/warp_hud_travel_mode_system.h"
#include "systems/warp_auto_comfort_system.h"
#include "systems/refining_system.h"
#include "systems/anomaly_system.h"
#include "systems/scanner_system.h"
#include "systems/difficulty_scaling_system.h"
#include "systems/mission_template_system.h"
#include "systems/mission_generator_system.h"
#include "systems/reputation_system.h"
#include "systems/lod_system.h"
#include "systems/spatial_hash_system.h"
#include "systems/background_simulation_system.h"
#include "systems/npc_intent_system.h"
#include "systems/npc_behavior_tree_system.h"
#include "systems/combat_threat_system.h"
#include "systems/security_response_system.h"
#include "systems/ambient_traffic_system.h"
#include "systems/snapshot_replication_system.h"
#include "systems/interest_management_system.h"
#include "systems/fleet_progression_system.h"
#include "systems/station_deployment_system.h"
#include "systems/fleet_warp_formation_system.h"
#include "systems/warp_meditation_system.h"
#include "systems/rumor_questline_system.h"
#include "systems/captain_departure_system.h"
#include "systems/captain_transfer_system.h"
#include "systems/npc_rerouting_system.h"
#include "systems/local_reputation_system.h"
#include "systems/station_news_system.h"
#include "systems/wreck_persistence_system.h"
#include "systems/fleet_history_system.h"
#include "systems/rig_system.h"
#include "systems/legend_system.h"
#include "systems/ancient_tech_system.h"
#include "systems/docking_system.h"
#include "systems/survival_system.h"
#include "systems/menu_system.h"
#include "systems/crew_system.h"
#include "systems/salvage_exploration_system.h"
#include "systems/market_order_system.h"
#include "systems/ai_economic_actor_system.h"
#include "systems/turret_ai_system.h"
#include "systems/titan_assembly_system.h"
#include "systems/task_scheduler_system.h"
#include "systems/mod_manager_system.h"
#include "systems/ship_designer_system.h"
#include "systems/fleet_morale_resolution_system.h"
#include "systems/persistence_delta_system.h"
#include "systems/outer_rim_logistics_distortion_system.h"
#include "systems/rumor_propagation_system.h"
#include "systems/galactic_response_curve_system.h"
#include "systems/ship_capability_rating_system.h"
#include "systems/module_cascading_failure_system.h"
#include "systems/navigation_bookmark_system.h"
#include "systems/fleet_supply_line_system.h"
#include "systems/crew_training_system.h"
#include "systems/asteroid_belt_system.h"
#include "systems/scan_probe_system.h"
#include "systems/autopilot_system.h"
#include "network/protocol_handler.h"
#include "ui/server_console.h"
#include "utils/logger.h"
#include "utils/server_metrics.h"
#include "pcg/deterministic_rng.h"
#include "pcg/hash_utils.h"
#include "pcg/pcg_context.h"
#include "pcg/pcg_manager.h"
#include "pcg/ship_generator.h"
#include "pcg/fleet_doctrine.h"
#include "pcg/room_graph.h"
#include "pcg/deck_graph.h"
#include "pcg/elevator_system.h"
#include "pcg/hull_mesher.h"
#include "pcg/capital_ship_generator.h"
#include "pcg/ship_designer.h"
#include "pcg/snappable_grid.h"
#include "pcg/station_generator.h"
#include "pcg/salvage_system.h"
#include "pcg/rover_system.h"
#include "pcg/character_mesh_system.h"
#include "pcg/turret_generator.h"
#include "pcg/planet_generator.h"
#include "pcg/habitat_generator.h"
#include "pcg/grav_bike_generator.h"
#include "pcg/spine_hull_generator.h"
#include "pcg/terrain_generator.h"
#include "pcg/turret_placement_system.h"
#include "pcg/damage_state_generator.h"
#include "pcg/procedural_texture_generator.h"
#include "pcg/shield_effect_generator.h"
#include "pcg/economy_driven_generator.h"
#include "pcg/collision_manager.h"
#include "pcg/asteroid_field_generator.h"
#include "pcg/anomaly_generator.h"
#include "pcg/npc_encounter_generator.h"
#include "pcg/star_system_generator.h"
#include "pcg/galaxy_generator.h"
#include "systems/alliance_system.h"
#include "systems/sovereignty_system.h"
#include "systems/war_declaration_system.h"
#include "systems/convoy_ambush_system.h"
#include "systems/npc_dialogue_system.h"
#include "systems/station_monument_system.h"
#include "systems/information_propagation_system.h"
#include "systems/crew_activity_system.h"
#include "systems/visual_cue_system.h"
#include "systems/supply_demand_system.h"
#include "systems/black_market_system.h"
#include "systems/price_history_system.h"
#include "systems/propaganda_system.h"
#include "systems/rest_station_system.h"
#include "systems/myth_boss_system.h"
#include "systems/ancient_tech_upgrade_system.h"
#include "systems/mod_manifest_system.h"
#include "systems/ancient_ai_remnant_system.h"
#include "systems/character_creation_screen_system.h"
#include "systems/view_mode_transition_system.h"
#include "systems/station_hangar_system.h"
#include "systems/tether_docking_system.h"
#include "systems/fps_spawn_system.h"
#include "systems/fps_character_controller_system.h"
#include "systems/interior_door_system.h"
#include "systems/eva_airlock_system.h"
#include "systems/fps_interaction_system.h"
#include "systems/fps_combat_system.h"
#include "systems/fps_inventory_system.h"
#include "systems/ship_interior_layout_system.h"
#include "systems/environmental_hazard_system.h"
#include "systems/fps_objective_system.h"
#include "systems/wing_management_system.h"
#include "systems/economic_flow_system.h"
#include "systems/resource_production_chain_system.h"
#include "systems/fleet_doctrine_system.h"
#include "systems/player_progression_system.h"
#include "systems/terraforming_system.h"
#include "systems/food_processor_system.h"
#include "systems/fleet_squad_system.h"
#include "systems/cloaking_system.h"
#include "systems/jump_drive_system.h"
#include "systems/cargo_scan_system.h"
#include "systems/commander_disagreement_system.h"
#include "systems/imperfect_information_system.h"
#include "systems/captain_background_system.h"
#include "systems/rover_interior_system.h"
#include "systems/bike_garage_system.h"
#include "systems/visual_rig_system.h"
#include "systems/planetary_traversal_system.h"
#include "systems/solar_panel_system.h"
#include "systems/farming_deck_system.h"
#include "systems/docking_ring_extension_system.h"
#include "systems/rover_bay_ramp_system.h"
#include "systems/grid_construction_system.h"
#include "systems/space_planet_transition_system.h"
#include "systems/fleet_hangar_system.h"
#include "systems/hangar_environment_system.h"
#include "systems/dock_node_layout_system.h"
#include "systems/mission_consequence_system.h"
#include "systems/server_performance_monitor_system.h"
#include "systems/atlas_ui_panel_system.h"
#include "systems/keyboard_navigation_system.h"
#include "systems/data_binding_system.h"
#include "systems/entity_stress_test_system.h"
#include "systems/rig_locker_system.h"
#include "systems/visual_coupling_system.h"
#include "systems/fps_salvage_path_system.h"
#include "systems/lavatory_interaction_system.h"
#include "systems/eva_airlock_exit_system.h"
#include "systems/ancient_module_discovery_system.h"
#include "systems/client_prediction_system.h"
#include "systems/ship_template_mod_system.h"
#include "systems/database_persistence_system.h"
#include "systems/mission_editor_system.h"
#include "systems/content_validation_system.h"
#include "systems/cloud_deployment_config_system.h"
#include "systems/interest_management_priority_system.h"
#include "systems/visual_feedback_queue_system.h"
#include "systems/mod_doc_generator_system.h"
#include "systems/community_content_repository_system.h"
#include "systems/pvp_toggle_system.h"
#include "systems/dynamic_event_system.h"
#include "systems/jump_gate_system.h"
#include "systems/snapshot_replication_system2.h"
#include "systems/procedural_mission_generator_system.h"
#include "systems/incursion_system.h"
#include "systems/clone_bay_system.h"
#include "systems/loyalty_point_store_system.h"
#include <fstream>
#include <thread>
#include <sys/stat.h>

using namespace atlas;
using atlas::test::assertTrue;
using atlas::test::approxEqual;
using atlas::test::addComp;

// ==================== Logger Tests ====================

void testLoggerLevels() {
    std::cout << "\n=== Logger Levels ===" << std::endl;
    
    auto& log = utils::Logger::instance();
    
    // Disable console output so tests don't clutter the terminal
    log.setConsoleOutput(false);
    
    log.setLevel(utils::LogLevel::INFO);
    assertTrue(log.getLevel() == utils::LogLevel::INFO, "Log level set to INFO");
    
    log.setLevel(utils::LogLevel::DEBUG);
    assertTrue(log.getLevel() == utils::LogLevel::DEBUG, "Log level set to DEBUG");
    
    log.setLevel(utils::LogLevel::ERROR);
    assertTrue(log.getLevel() == utils::LogLevel::ERROR, "Log level set to ERROR");
    
    log.setLevel(utils::LogLevel::WARN);
    assertTrue(log.getLevel() == utils::LogLevel::WARN, "Log level set to WARN");
    
    log.setLevel(utils::LogLevel::FATAL);
    assertTrue(log.getLevel() == utils::LogLevel::FATAL, "Log level set to FATAL");
    
    // Re-enable console output
    log.setConsoleOutput(true);
    // Reset to INFO for other tests
    log.setLevel(utils::LogLevel::INFO);
}

void testLoggerFileOutput() {
    std::cout << "\n=== Logger File Output ===" << std::endl;
    
    auto& log = utils::Logger::instance();
    log.setConsoleOutput(false);
    
    // Shut down any previously opened file
    log.shutdown();
    assertTrue(!log.isFileOpen(), "No file open after shutdown");
    
    // Init with a temp directory
    bool ok = log.init("/tmp/eve_test_logs");
    assertTrue(ok, "Logger init succeeds");
    assertTrue(log.isFileOpen(), "Log file is open after init");
    
    // Write some log entries
    log.setLevel(utils::LogLevel::DEBUG);
    log.debug("test debug message");
    log.info("test info message");
    log.warn("test warn message");
    log.error("test error message");
    
    log.shutdown();
    assertTrue(!log.isFileOpen(), "Log file closed after shutdown");
    
    // Verify the file was actually written
    std::ifstream f("/tmp/eve_test_logs/server.log");
    assertTrue(f.is_open(), "Log file exists on disk");
    
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assertTrue(content.find("[DEBUG]") != std::string::npos, "Log contains DEBUG entry");
    assertTrue(content.find("[INFO]") != std::string::npos, "Log contains INFO entry");
    assertTrue(content.find("[WARN]") != std::string::npos, "Log contains WARN entry");
    assertTrue(content.find("[ERROR]") != std::string::npos, "Log contains ERROR entry");
    assertTrue(content.find("test debug message") != std::string::npos, "Log contains debug text");
    assertTrue(content.find("test info message") != std::string::npos, "Log contains info text");
    f.close();
    
    // Clean up
    std::remove("/tmp/eve_test_logs/server.log");
    
    // Re-enable console
    log.setConsoleOutput(true);
    log.setLevel(utils::LogLevel::INFO);
}

void testLoggerLevelFiltering() {
    std::cout << "\n=== Logger Level Filtering ===" << std::endl;
    
    auto& log = utils::Logger::instance();
    log.setConsoleOutput(false);
    log.shutdown();
    
    bool ok = log.init("/tmp/eve_test_logs", "filter_test.log");
    assertTrue(ok, "Logger init for filter test succeeds");
    
    // Set level to WARN — DEBUG and INFO should be filtered out
    log.setLevel(utils::LogLevel::WARN);
    log.debug("should_not_appear_debug");
    log.info("should_not_appear_info");
    log.warn("should_appear_warn");
    log.error("should_appear_error");
    
    log.shutdown();
    
    std::ifstream f("/tmp/eve_test_logs/filter_test.log");
    assertTrue(f.is_open(), "Filter test log file exists");
    
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assertTrue(content.find("should_not_appear_debug") == std::string::npos,
               "DEBUG filtered out at WARN level");
    assertTrue(content.find("should_not_appear_info") == std::string::npos,
               "INFO filtered out at WARN level");
    assertTrue(content.find("should_appear_warn") != std::string::npos,
               "WARN passes at WARN level");
    assertTrue(content.find("should_appear_error") != std::string::npos,
               "ERROR passes at WARN level");
    f.close();
    
    std::remove("/tmp/eve_test_logs/filter_test.log");
    log.setConsoleOutput(true);
    log.setLevel(utils::LogLevel::INFO);
}

// ==================== ServerMetrics Tests ====================

void testMetricsTickTiming() {
    std::cout << "\n=== Metrics Tick Timing ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    assertTrue(metrics.getTotalTicks() == 0, "No ticks recorded initially");
    assertTrue(metrics.getAvgTickMs() == 0.0, "Avg tick 0 with no data");
    assertTrue(metrics.getMaxTickMs() == 0.0, "Max tick 0 with no data");
    assertTrue(metrics.getMinTickMs() == 0.0, "Min tick 0 with no data");
    
    // Record a few ticks with a known sleep
    for (int i = 0; i < 5; ++i) {
        metrics.recordTickStart();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        metrics.recordTickEnd();
    }
    
    assertTrue(metrics.getTotalTicks() == 5, "5 ticks recorded");
    assertTrue(metrics.getAvgTickMs() >= 1.0, "Average tick >= 1ms");
    assertTrue(metrics.getMaxTickMs() >= 1.0, "Max tick >= 1ms");
    assertTrue(metrics.getMinTickMs() >= 1.0, "Min tick >= 1ms");
    assertTrue(metrics.getMaxTickMs() >= metrics.getMinTickMs(), "Max >= Min");
}

void testMetricsCounters() {
    std::cout << "\n=== Metrics Counters ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    assertTrue(metrics.getEntityCount() == 0, "Entity count starts at 0");
    assertTrue(metrics.getPlayerCount() == 0, "Player count starts at 0");
    
    metrics.setEntityCount(42);
    metrics.setPlayerCount(3);
    
    assertTrue(metrics.getEntityCount() == 42, "Entity count set to 42");
    assertTrue(metrics.getPlayerCount() == 3, "Player count set to 3");
}

void testMetricsUptime() {
    std::cout << "\n=== Metrics Uptime ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    assertTrue(metrics.getUptimeSeconds() >= 0.0, "Uptime is non-negative");
    
    std::string uptime = metrics.getUptimeString();
    assertTrue(!uptime.empty(), "Uptime string is not empty");
    assertTrue(uptime.find("d") != std::string::npos, "Uptime contains 'd'");
    assertTrue(uptime.find("h") != std::string::npos, "Uptime contains 'h'");
    assertTrue(uptime.find("m") != std::string::npos, "Uptime contains 'm'");
    assertTrue(uptime.find("s") != std::string::npos, "Uptime contains 's'");
}

void testMetricsSummary() {
    std::cout << "\n=== Metrics Summary ===" << std::endl;
    
    utils::ServerMetrics metrics;
    metrics.setEntityCount(10);
    metrics.setPlayerCount(2);
    
    metrics.recordTickStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    metrics.recordTickEnd();
    
    std::string s = metrics.summary();
    assertTrue(!s.empty(), "Summary is not empty");
    assertTrue(s.find("[Metrics]") != std::string::npos, "Summary contains [Metrics]");
    assertTrue(s.find("entities=10") != std::string::npos, "Summary contains entity count");
    assertTrue(s.find("players=2") != std::string::npos, "Summary contains player count");
    assertTrue(s.find("uptime") != std::string::npos, "Summary contains uptime");
    assertTrue(s.find("ticks=") != std::string::npos, "Summary contains tick count");
}

void testMetricsResetWindow() {
    std::cout << "\n=== Metrics Reset Window ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    // Record some ticks
    for (int i = 0; i < 3; ++i) {
        metrics.recordTickStart();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        metrics.recordTickEnd();
    }
    
    assertTrue(metrics.getTotalTicks() == 3, "3 ticks before reset");
    assertTrue(metrics.getAvgTickMs() > 0.0, "Avg > 0 before reset");
    
    metrics.resetWindow();
    
    // Total ticks should remain, but window stats reset
    assertTrue(metrics.getTotalTicks() == 3, "Total ticks preserved after reset");
    assertTrue(metrics.getAvgTickMs() == 0.0, "Avg reset to 0 after window reset");
    assertTrue(metrics.getMaxTickMs() == 0.0, "Max reset to 0 after window reset");
    assertTrue(metrics.getMinTickMs() == 0.0, "Min reset to 0 after window reset");
}

// ==================== ServerConsole Tests ====================

void testConsoleInit() {
    std::cout << "\n=== Console Init ===" << std::endl;
    ServerConsole console;
    // Pass dummy references — the init only stores a flag
    bool ok = console.init();
    assertTrue(ok, "Console initializes successfully");
    assertTrue(console.getCommandCount() >= 2, "Built-in commands registered (help, status)");
}

void testConsoleHelpCommand() {
    std::cout << "\n=== Console Help Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("help");
    assertTrue(output.find("help") != std::string::npos, "Help output lists 'help' command");
    assertTrue(output.find("status") != std::string::npos, "Help output lists 'status' command");
}

void testConsoleStatusCommand() {
    std::cout << "\n=== Console Status Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("status");
    assertTrue(output.find("Server Status") != std::string::npos, "Status output has header");
    assertTrue(output.find("Commands registered") != std::string::npos, "Status shows command count");
}

void testConsoleUnknownCommand() {
    std::cout << "\n=== Console Unknown Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("foobar");
    assertTrue(output.find("Unknown command") != std::string::npos, "Unknown command error message");
}

void testConsoleCustomCommand() {
    std::cout << "\n=== Console Custom Command ===" << std::endl;
    ServerConsole console;
    console.init();

    console.registerCommand("ping", "Reply with pong",
        [](const std::vector<std::string>& /*args*/) -> std::string {
            return "pong";
        });

    std::string output = console.executeCommand("ping");
    assertTrue(output == "pong", "Custom command returns expected output");
    assertTrue(console.getCommandCount() >= 3, "Custom command registered");
}

void testConsoleLogBuffer() {
    std::cout << "\n=== Console Log Buffer ===" << std::endl;
    ServerConsole console;
    console.init();

    console.addLogMessage(utils::LogLevel::INFO, "Test message 1");
    console.addLogMessage(utils::LogLevel::INFO, "Test message 2");

    assertTrue(console.getLogBuffer().size() == 2, "Two log entries buffered");
    assertTrue(console.getLogBuffer()[0] == "Test message 1", "First log entry correct");
}

void testConsoleEmptyCommand() {
    std::cout << "\n=== Console Empty Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("");
    assertTrue(output.empty(), "Empty command returns empty string");
}

void testConsoleNotInitialized() {
    std::cout << "\n=== Console Not Initialized ===" << std::endl;
    ServerConsole console;

    std::string output = console.executeCommand("help");
    assertTrue(output.find("not initialized") != std::string::npos, "Not-initialized message");
}

void testConsoleShutdown() {
    std::cout << "\n=== Console Shutdown ===" << std::endl;
    ServerConsole console;
    console.init();
    assertTrue(console.getCommandCount() >= 2, "Commands before shutdown");

    console.shutdown();
    assertTrue(console.getCommandCount() == 0, "Commands cleared after shutdown");
}

void testConsoleInteractiveMode() {
    std::cout << "\n=== Console Interactive Mode ===" << std::endl;
    ServerConsole console;
    assertTrue(!console.isInteractive(), "Default is non-interactive");
    console.setInteractive(true);
    assertTrue(console.isInteractive(), "Interactive mode set");
}

// ==================== TacticalOverlaySystem Tests ====================

void testTacticalOverlayToggle() {
    std::cout << "\n=== Tactical Overlay Toggle ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    sys.toggleOverlay("player1");
    assertTrue(sys.isEnabled("player1"), "Overlay enabled after toggle");
}

void testTacticalOverlayToggleTwice() {
    std::cout << "\n=== Tactical Overlay Toggle Twice ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    sys.toggleOverlay("player1");
    sys.toggleOverlay("player1");
    assertTrue(!sys.isEnabled("player1"), "Overlay disabled after double toggle");
}

void testTacticalOverlaySetToolRange() {
    std::cout << "\n=== Tactical Overlay Set Tool Range ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    auto* overlay = addComp<components::TacticalOverlayState>(entity);
    sys.setToolRange("player1", 5000.0f, "weapon");
    assertTrue(approxEqual(overlay->tool_range, 5000.0f), "Tool range set to 5000");
}

void testTacticalOverlayRingDistances() {
    std::cout << "\n=== Tactical Overlay Ring Distances ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    std::vector<float> custom = {10.0f, 25.0f, 50.0f};
    sys.setRingDistances("player1", custom);
    auto result = sys.getRingDistances("player1");
    assertTrue(result.size() == 3, "Ring distances has 3 entries");
    assertTrue(approxEqual(result[0], 10.0f), "First ring distance is 10");
    assertTrue(approxEqual(result[2], 50.0f), "Third ring distance is 50");
}

void testTacticalOverlayDefaultRings() {
    std::cout << "\n=== Tactical Overlay Default Rings ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    auto rings = sys.getRingDistances("player1");
    assertTrue(rings.size() == 6, "Default ring distances has 6 entries");
    assertTrue(approxEqual(rings[0], 5.0f), "Default first ring is 5.0");
    assertTrue(approxEqual(rings[5], 100.0f), "Default last ring is 100.0");
}

// ==================== New Protocol Message Tests ====================

void testProtocolSalvageMessages() {
    std::cout << "\n=== Protocol: Salvage Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse salvage request
    std::string msg = "{\"message_type\":\"salvage_request\",\"data\":{\"wreck_id\":\"wreck_1\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Salvage request parses successfully");
    assertTrue(type == atlas::network::MessageType::SALVAGE_REQUEST, "Type is SALVAGE_REQUEST");

    // Create salvage result
    std::string result = proto.createSalvageResult(true, "wreck_1", 3);
    assertTrue(result.find("salvage_result") != std::string::npos, "Result has correct type");
    assertTrue(result.find("wreck_1") != std::string::npos, "Result contains wreck_id");
    assertTrue(result.find("3") != std::string::npos, "Result contains items_recovered");
}

void testProtocolLootMessages() {
    std::cout << "\n=== Protocol: Loot Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"loot_all\",\"data\":{\"wreck_id\":\"wreck_2\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Loot all parses successfully");
    assertTrue(type == atlas::network::MessageType::LOOT_ALL, "Type is LOOT_ALL");

    std::string result = proto.createLootResult(true, "wreck_2", 5, 10000.0);
    assertTrue(result.find("loot_result") != std::string::npos, "Result has correct type");
    assertTrue(result.find("wreck_2") != std::string::npos, "Result contains wreck_id");
    assertTrue(result.find("10000") != std::string::npos, "Result contains isk_gained");
}

void testProtocolMiningMessages() {
    std::cout << "\n=== Protocol: Mining Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse mining start
    std::string msg = "{\"message_type\":\"mining_start\",\"data\":{\"deposit_id\":\"deposit_0\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Mining start parses successfully");
    assertTrue(type == atlas::network::MessageType::MINING_START, "Type is MINING_START");

    // Parse mining stop
    msg = "{\"message_type\":\"mining_stop\",\"data\":{}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Mining stop parses successfully");
    assertTrue(type == atlas::network::MessageType::MINING_STOP, "Type is MINING_STOP");

    // Create mining result
    std::string result = proto.createMiningResult(true, "deposit_0", "Ferrite", 100);
    assertTrue(result.find("mining_result") != std::string::npos, "Result has correct type");
    assertTrue(result.find("Ferrite") != std::string::npos, "Result contains mineral_type");
    assertTrue(result.find("100") != std::string::npos, "Result contains quantity_mined");
}

void testProtocolScannerMessages() {
    std::cout << "\n=== Protocol: Scanner Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse scan start
    std::string msg = "{\"message_type\":\"scan_start\",\"data\":{\"system_id\":\"sys_01\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Scan start parses successfully");
    assertTrue(type == atlas::network::MessageType::SCAN_START, "Type is SCAN_START");

    // Parse scan stop
    msg = "{\"message_type\":\"scan_stop\",\"data\":{}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Scan stop parses successfully");
    assertTrue(type == atlas::network::MessageType::SCAN_STOP, "Type is SCAN_STOP");

    // Create scan result
    std::string result = proto.createScanResult("scanner_1", 2, "[{\"anomaly_id\":\"a1\"},{\"anomaly_id\":\"a2\"}]");
    assertTrue(result.find("scan_result") != std::string::npos, "Scan result has correct type");
    assertTrue(result.find("scanner_1") != std::string::npos, "Scan result contains scanner_id");
    assertTrue(result.find("\"anomalies_found\":2") != std::string::npos, "Scan result contains anomalies_found");
    assertTrue(result.find("a1") != std::string::npos, "Scan result contains first anomaly");
    assertTrue(result.find("a2") != std::string::npos, "Scan result contains second anomaly");
}

void testProtocolAnomalyListMessages() {
    std::cout << "\n=== Protocol: Anomaly List Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse anomaly list request
    std::string msg = "{\"message_type\":\"anomaly_list\",\"data\":{\"system_id\":\"sys_02\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Anomaly list request parses successfully");
    assertTrue(type == atlas::network::MessageType::ANOMALY_LIST, "Type is ANOMALY_LIST");

    // Create anomaly list response
    std::string result = proto.createAnomalyList("sys_02", 3, "[\"anom_1\",\"anom_2\",\"anom_3\"]");
    assertTrue(result.find("anomaly_list") != std::string::npos, "Anomaly list has correct type");
    assertTrue(result.find("sys_02") != std::string::npos, "Anomaly list contains system_id");
    assertTrue(result.find("\"count\":3") != std::string::npos, "Anomaly list contains count");
    assertTrue(result.find("anom_1") != std::string::npos, "Anomaly list contains first anomaly");
    assertTrue(result.find("anom_3") != std::string::npos, "Anomaly list contains third anomaly");
}

void testProtocolScanResultParse() {
    std::cout << "\n=== Protocol: Scan Result Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"scan_result\",\"data\":{\"scanner_id\":\"sc1\",\"anomalies_found\":1,\"results\":[{\"anomaly_id\":\"a1\",\"signal_strength\":0.75}]}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Scan result parses successfully");
    assertTrue(type == atlas::network::MessageType::SCAN_RESULT, "Parsed type is SCAN_RESULT");
    assertTrue(data.find("sc1") != std::string::npos, "Data contains scanner_id");
}

void testProtocolLootResultParse() {
    std::cout << "\n=== Protocol: Loot Result Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"loot_result\",\"data\":{\"success\":true,\"wreck_id\":\"wreck_3\",\"items_collected\":2,\"isk_gained\":5000}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Loot result parses successfully");
    assertTrue(type == atlas::network::MessageType::LOOT_RESULT, "Type is LOOT_RESULT");
}

void testProtocolMiningResultParse() {
    std::cout << "\n=== Protocol: Mining Result Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"mining_result\",\"data\":{\"success\":true,\"deposit_id\":\"deposit_1\",\"mineral_type\":\"Galvite\",\"quantity_mined\":50}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Mining result parses successfully");
    assertTrue(type == atlas::network::MessageType::MINING_RESULT, "Type is MINING_RESULT");
}

// ==================== AI Mining State Test ====================

void testAIMiningState() {
    std::cout << "\n=== AI: Mining State ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("miner_npc");
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Mining;

    assertTrue(ai->state == components::AI::State::Mining, "AI state set to Mining");
    // Mining state is distinct from other states
    assertTrue(ai->state != components::AI::State::Idle, "Mining != Idle");
    assertTrue(ai->state != components::AI::State::Attacking, "Mining != Attacking");
}

void testAIMiningBehaviorActivatesLaser() {
    std::cout << "\n=== AI: Mining Behavior Activates Laser ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);
    systems::MiningSystem miningSys(&world);

    // Create a mineral deposit nearby
    auto* deposit = world.createEntity("deposit1");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 5000.0f;

    // Create an AI miner with MiningLaser
    auto* npc = world.createEntity("miner_npc");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 50.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Passive;
    ai->state = components::AI::State::Mining;
    ai->target_entity_id = "deposit1";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = false;
    auto* inv = addComp<components::Inventory>(npc);
    inv->max_capacity = 1000.0f;

    // Run AI update — mining behavior should activate the laser
    aiSys.update(1.0f);

    assertTrue(laser->active, "Mining behavior activates laser");
    assertTrue(laser->target_deposit_id == "deposit1", "Laser targets the deposit");
}

void testAIMiningIdleFindsDeposit() {
    std::cout << "\n=== AI: Idle Miner Finds Deposit ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create a mineral deposit within awareness range
    auto* deposit = world.createEntity("deposit_nearby");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 5000.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Galvite";
    dep->quantity_remaining = 3000.0f;

    // Create a passive AI miner at origin, idle
    auto* npc = world.createEntity("miner2");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Passive;
    ai->state = components::AI::State::Idle;
    ai->awareness_range = 50000.0f;
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = false;

    // Idle behavior should find the deposit and start approaching
    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Approaching, "Idle miner transitions to Approaching");
    assertTrue(ai->target_entity_id == "deposit_nearby", "Miner targets the deposit");
}

void testAIMiningApproachTransitions() {
    std::cout << "\n=== AI: Miner Approach Transitions to Mining ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create deposit very close
    auto* deposit = world.createEntity("dep_close");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 1000.0f;

    // Create NPC approaching the deposit (within mining range)
    auto* npc = world.createEntity("miner3");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 50.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Passive;
    ai->state = components::AI::State::Approaching;
    ai->target_entity_id = "dep_close";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = false;

    // Approach should transition to Mining when within range
    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Mining, "Approach transitions to Mining near deposit");
}

void testAIMiningStopsOnDepletedDeposit() {
    std::cout << "\n=== AI: Mining Stops When Deposit Depleted ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create an empty deposit
    auto* deposit = world.createEntity("empty_dep");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 0.0f;

    auto* npc = world.createEntity("miner4");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 100.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Mining;
    ai->target_entity_id = "empty_dep";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = true;
    laser->target_deposit_id = "empty_dep";

    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Idle, "Mining stops on depleted deposit");
    assertTrue(ai->target_entity_id.empty(), "Target cleared on depleted deposit");
}

void testAIMiningStopsOnCargoFull() {
    std::cout << "\n=== AI: Mining Stops When Cargo Full ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* deposit = world.createEntity("dep_full");
    auto* dpos = addComp<components::Position>(deposit);
    dpos->x = 100.0f; dpos->y = 0.0f; dpos->z = 0.0f;
    auto* dep = addComp<components::MineralDeposit>(deposit);
    dep->mineral_type = "Ferrite";
    dep->quantity_remaining = 5000.0f;

    auto* npc = world.createEntity("miner5");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 100.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Mining;
    ai->target_entity_id = "dep_full";
    auto* laser = addComp<components::MiningLaser>(npc);
    laser->active = true;
    // Fill cargo completely
    auto* inv = addComp<components::Inventory>(npc);
    inv->max_capacity = 100.0f;
    components::Inventory::Item ore;
    ore.item_id = "Ferrite";
    ore.name = "Ferrite";
    ore.type = "ore";
    ore.quantity = 1000;
    ore.volume = 0.1f;
    inv->items.push_back(ore);

    aiSys.update(1.0f);

    assertTrue(ai->state == components::AI::State::Idle, "Mining stops on cargo full");
}

void testAIFindNearestDeposit() {
    std::cout << "\n=== AI: Find Nearest Deposit ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create two deposits at different distances
    auto* far_dep = world.createEntity("far_dep");
    auto* fdpos = addComp<components::Position>(far_dep);
    fdpos->x = 30000.0f; fdpos->y = 0.0f; fdpos->z = 0.0f;
    auto* fd = addComp<components::MineralDeposit>(far_dep);
    fd->mineral_type = "Galvite";
    fd->quantity_remaining = 5000.0f;

    auto* near_dep = world.createEntity("near_dep");
    auto* ndpos = addComp<components::Position>(near_dep);
    ndpos->x = 5000.0f; ndpos->y = 0.0f; ndpos->z = 0.0f;
    auto* nd = addComp<components::MineralDeposit>(near_dep);
    nd->mineral_type = "Ferrite";
    nd->quantity_remaining = 5000.0f;

    auto* npc = world.createEntity("searcher");
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;
    auto* ai = addComp<components::AI>(npc);
    ai->awareness_range = 50000.0f;

    auto* found = aiSys.findNearestDeposit(npc);
    assertTrue(found != nullptr, "Found a deposit");
    assertTrue(found->getId() == "near_dep", "Found nearest deposit");
}

// ==================== Phase 10: Tactical Overlay Shared Filters Tests ====================

void testOverlaySharedFilters() {
    std::cout << "\n=== Overlay Shared Filters ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);

    auto defaults = sys.getFilterCategories("player1");
    assertTrue(defaults.size() == 4, "Default has 4 filter categories");

    std::vector<std::string> custom = {"hostile", "structure"};
    sys.setFilterCategories("player1", custom);
    auto updated = sys.getFilterCategories("player1");
    assertTrue(updated.size() == 2, "Updated to 2 filter categories");
    assertTrue(updated[0] == "hostile", "First filter is hostile");
    assertTrue(updated[1] == "structure", "Second filter is structure");
}

void testOverlayPassiveDisplayOnly() {
    std::cout << "\n=== Overlay Passive Display Only ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);

    assertTrue(sys.isPassiveDisplayOnly("player1"), "Overlay is passive by default");
}

void testOverlayEntityPriority() {
    std::cout << "\n=== Overlay Entity Priority ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);

    assertTrue(approxEqual(sys.getEntityDisplayPriority("player1"), 1.0f),
               "Default entity priority is 1.0");

    sys.setEntityDisplayPriority("player1", 5.0f);
    assertTrue(approxEqual(sys.getEntityDisplayPriority("player1"), 5.0f),
               "Entity priority updated to 5.0");
}

void testOverlayEntityPriorityHostileHighAsteroidLow() {
    std::cout << "\n=== Overlay Hostile High / Asteroid Low Priority ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* hostile = world.createEntity("hostile1");
    addComp<components::TacticalOverlayState>(hostile);
    sys.setEntityDisplayPriority("hostile1", 10.0f);

    auto* asteroid = world.createEntity("asteroid1");
    addComp<components::TacticalOverlayState>(asteroid);
    sys.setEntityDisplayPriority("asteroid1", 0.5f);

    assertTrue(sys.getEntityDisplayPriority("hostile1") > sys.getEntityDisplayPriority("asteroid1"),
               "Hostile higher priority than asteroid");
}

// ==================== Mission Protocol Tests ====================

void testMissionProtocolRoundTrip() {
    std::cout << "\n=== Mission Protocol Round Trip ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Test mission_list message parses correctly
    std::string list_msg = "{\"message_type\":\"mission_list\",\"data\":{\"system_id\":\"sys_01\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(list_msg, type, data);
    assertTrue(ok, "Mission list message parses");
    assertTrue(type == atlas::network::MessageType::MISSION_LIST, "Type is MISSION_LIST");

    // Test accept_mission message parses correctly
    std::string accept_msg = "{\"message_type\":\"accept_mission\",\"data\":{\"system_id\":\"sys_01\",\"mission_index\":0}}";
    ok = proto.parseMessage(accept_msg, type, data);
    assertTrue(ok, "Accept mission message parses");
    assertTrue(type == atlas::network::MessageType::ACCEPT_MISSION, "Type is ACCEPT_MISSION");

    // Test abandon_mission message parses correctly
    std::string abandon_msg = "{\"message_type\":\"abandon_mission\",\"data\":{\"mission_id\":\"m_001\"}}";
    ok = proto.parseMessage(abandon_msg, type, data);
    assertTrue(ok, "Abandon mission message parses");
    assertTrue(type == atlas::network::MessageType::ABANDON_MISSION, "Type is ABANDON_MISSION");

    // Test mission_progress message parses correctly
    std::string progress_msg = "{\"message_type\":\"mission_progress\",\"data\":{\"mission_id\":\"m_001\",\"objective_type\":\"destroy\"}}";
    ok = proto.parseMessage(progress_msg, type, data);
    assertTrue(ok, "Mission progress message parses");
    assertTrue(type == atlas::network::MessageType::MISSION_PROGRESS, "Type is MISSION_PROGRESS");

    // Test createMissionList output
    std::string ml = proto.createMissionList("sys_01", 2, "[{\"name\":\"Patrol\"},{\"name\":\"Mining\"}]");
    assertTrue(ml.find("mission_list") != std::string::npos, "MissionList contains type");
    assertTrue(ml.find("sys_01") != std::string::npos, "MissionList contains system_id");
    assertTrue(ml.find("Patrol") != std::string::npos, "MissionList contains mission data");

    // Test createMissionResult output
    std::string mr = proto.createMissionResult(true, "m_001", "accept", "Mission accepted");
    assertTrue(mr.find("mission_result") != std::string::npos, "MissionResult contains type");
    assertTrue(mr.find("m_001") != std::string::npos, "MissionResult contains mission_id");
    assertTrue(mr.find("accept") != std::string::npos, "MissionResult contains action");
}

void testMissionGeneratorNetworkFlow() {
    std::cout << "\n=== Mission Generator Network Flow ===" << std::endl;

    ecs::World world;
    systems::MissionTemplateSystem templates(&world);
    systems::MissionGeneratorSystem generator(&world, &templates);

    templates.installDefaultTemplates();

    // Create a system entity
    auto* sys = world.createEntity("system_01");
    auto* dz = addComp<components::DifficultyZone>(sys);
    dz->security_status = 0.5f;

    // Generate missions for the system
    int count = generator.generateMissionsForSystem("system_01", 42);
    assertTrue(count > 0, "Missions generated for system");

    auto available = generator.getAvailableMissions("system_01");
    assertTrue(!available.empty(), "Available missions list is not empty");

    // Create a player and offer a mission
    auto* player = world.createEntity("player_01");
    addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    bool offered = generator.offerMissionToPlayer("player_01", "system_01", 0);
    assertTrue(offered, "Mission offered to player successfully");

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(tracker->active_missions.size() == 1, "Player has one active mission");
    assertTrue(!tracker->active_missions[0].mission_id.empty(), "Mission has a valid ID");
}

void testMissionAcceptAbandonNetworkFlow() {
    std::cout << "\n=== Mission Accept/Abandon Network Flow ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missions(&world);
    systems::MissionTemplateSystem templates(&world);
    systems::MissionGeneratorSystem generator(&world, &templates);

    templates.installDefaultTemplates();

    auto* sys_entity = world.createEntity("system_01");
    auto* dz = addComp<components::DifficultyZone>(sys_entity);
    dz->security_status = 0.5f;

    generator.generateMissionsForSystem("system_01", 42);

    auto* player = world.createEntity("player_01");
    addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    // Accept a mission
    generator.offerMissionToPlayer("player_01", "system_01", 0);
    auto* tracker = player->getComponent<components::MissionTracker>();
    std::string mid = tracker->active_missions[0].mission_id;
    assertTrue(!mid.empty(), "Accepted mission has valid ID");

    // Abandon the mission
    missions.abandonMission("player_01", mid);
    assertTrue(tracker->active_missions.empty(), "Mission abandoned — active list empty");
}

void testAIDefensiveBehavior() {
    std::cout << "\n=== AI Defensive Behavior ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create a defensive NPC (patrol)
    auto* patrol = world.createEntity("patrol_01");
    auto* ai = addComp<components::AI>(patrol);
    ai->behavior = components::AI::Behavior::Defensive;
    ai->awareness_range = 100000.0f;
    addComp<components::Position>(patrol);
    addComp<components::Velocity>(patrol);
    auto* patrolFaction = addComp<components::Faction>(patrol);
    patrolFaction->faction_name = "Solari";
    patrolFaction->standings["Veyren"] = -5.0f;  // hostile to Veyren

    // Create a friendly player (positive standing with Solari)
    auto* player = world.createEntity("player_01");
    addComp<components::Player>(player);
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 200.0f;
    auto* standings = addComp<components::Standings>(player);
    standings->faction_standings["Solari"] = 3.0f;
    // Player has damage events (being attacked)
    auto* dmg = addComp<components::DamageEvent>(player);
    components::DamageEvent::HitRecord hit;
    hit.damage_amount = 50.0f;
    hit.layer_hit = "shield";
    hit.damage_type = "em";
    dmg->recent_hits.push_back(hit);

    // Create a hostile NPC attacking the player
    auto* pirate = world.createEntity("pirate_01");
    auto* pirateAI = addComp<components::AI>(pirate);
    pirateAI->behavior = components::AI::Behavior::Aggressive;
    pirateAI->target_entity_id = "player_01";  // targeting the player
    auto* piratePos = addComp<components::Position>(pirate);
    piratePos->x = 300.0f;
    addComp<components::Velocity>(pirate);
    auto* pirateFaction = addComp<components::Faction>(pirate);
    pirateFaction->faction_name = "Veyren";

    // The patrol should find the pirate attacking its friendly player
    ecs::Entity* attacker = aiSys.findAttackerOfFriendly(patrol);
    assertTrue(attacker == pirate, "Defensive NPC finds attacker of friendly player");
}

void testAIDefensiveNoFriendly() {
    std::cout << "\n=== AI Defensive No Friendly ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* patrol = world.createEntity("patrol_01");
    auto* ai = addComp<components::AI>(patrol);
    ai->behavior = components::AI::Behavior::Defensive;
    ai->awareness_range = 100000.0f;
    addComp<components::Position>(patrol);
    addComp<components::Velocity>(patrol);
    auto* patrolFaction = addComp<components::Faction>(patrol);
    patrolFaction->faction_name = "Solari";

    // No friendly entities in range → should find no attacker
    ecs::Entity* attacker = aiSys.findAttackerOfFriendly(patrol);
    assertTrue(attacker == nullptr, "No attacker found when no friendlies present");
}

void testAIDefensiveIdleTransition() {
    std::cout << "\n=== AI Defensive Idle Transition ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create a defensive NPC
    auto* patrol = world.createEntity("patrol_01");
    auto* ai = addComp<components::AI>(patrol);
    ai->behavior = components::AI::Behavior::Defensive;
    ai->awareness_range = 100000.0f;
    ai->state = components::AI::State::Idle;
    addComp<components::Position>(patrol);
    addComp<components::Velocity>(patrol);
    auto* patrolFaction = addComp<components::Faction>(patrol);
    patrolFaction->faction_name = "Solari";
    patrolFaction->standings["Veyren"] = -5.0f;

    // Create friendly player under attack
    auto* player = world.createEntity("player_01");
    addComp<components::Player>(player);
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 200.0f;
    auto* standings = addComp<components::Standings>(player);
    standings->faction_standings["Solari"] = 3.0f;
    auto* dmg = addComp<components::DamageEvent>(player);
    components::DamageEvent::HitRecord hit;
    hit.damage_amount = 50.0f;
    hit.layer_hit = "shield";
    hit.damage_type = "em";
    dmg->recent_hits.push_back(hit);

    auto* pirate = world.createEntity("pirate_01");
    auto* pirateAI = addComp<components::AI>(pirate);
    pirateAI->behavior = components::AI::Behavior::Aggressive;
    pirateAI->target_entity_id = "player_01";
    auto* piratePos = addComp<components::Position>(pirate);
    piratePos->x = 300.0f;
    addComp<components::Velocity>(pirate);
    auto* pirateFaction = addComp<components::Faction>(pirate);
    pirateFaction->faction_name = "Veyren";

    // Run AI update — patrol should transition to Approaching the pirate
    aiSys.update(0.1f);

    assertTrue(ai->state == components::AI::State::Approaching,
               "Defensive NPC transitions from Idle to Approaching when friendly attacked");
    assertTrue(ai->target_entity_id == "pirate_01",
               "Defensive NPC targets the attacker");
}

// ==================== Phase 5: Persistence & Stress Testing ====================

void testPersistenceStress100Ships() {
    std::cout << "\n=== Persistence Stress: 100 Ships Save/Load ===" << std::endl;

    ecs::World world;

    // Create 100 ship entities with multiple components
    for (int i = 0; i < 100; ++i) {
        std::string id = "stress_persist_ship_" + std::to_string(i);
        auto* e = world.createEntity(id);

        auto* pos = addComp<components::Position>(e);
        pos->x = static_cast<float>(i * 1000);
        pos->y = static_cast<float>((i % 10) * 200);
        pos->z = static_cast<float>((i / 10) * 300);

        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 400.0f + static_cast<float>(i);
        hp->shield_max = 500.0f;
        hp->armor_hp = 250.0f + static_cast<float>(i % 50);
        hp->armor_max = 300.0f;
        hp->hull_hp = 150.0f + static_cast<float>(i % 30);
        hp->hull_max = 200.0f;

        auto* ship = addComp<components::Ship>(e);
        ship->ship_type = (i % 3 == 0) ? "Frigate" : ((i % 3 == 1) ? "Cruiser" : "Battleship");

        auto* ai = addComp<components::AI>(e);
        ai->state = components::AI::State::Idle;

        auto* lod = addComp<components::LODPriority>(e);
        lod->priority = (i < 5) ? 2.0f : 0.5f;
        lod->force_visible = (i < 5);
    }

    assertTrue(world.getEntityCount() == 100, "Created 100 entities for stress test");

    // Save to file
    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_stress_100ships.json";
    bool saved = persistence.saveWorld(&world, filepath);
    assertTrue(saved, "100-ship world saved to file");

    // Load into fresh world
    ecs::World world2;
    bool loaded = persistence.loadWorld(&world2, filepath);
    assertTrue(loaded, "100-ship world loaded from file");
    assertTrue(world2.getEntityCount() == 100, "Loaded world has 100 entities");

    // Verify a sample of entities
    auto* e0 = world2.getEntity("stress_persist_ship_0");
    assertTrue(e0 != nullptr, "First ship entity found after load");
    auto* pos0 = e0->getComponent<components::Position>();
    assertTrue(pos0 != nullptr, "Position component on first ship");
    assertTrue(approxEqual(pos0->x, 0.0f), "First ship x position correct");

    auto* e50 = world2.getEntity("stress_persist_ship_50");
    assertTrue(e50 != nullptr, "Ship 50 entity found after load");
    auto* hp50 = e50->getComponent<components::Health>();
    assertTrue(hp50 != nullptr, "Health component on ship 50");
    assertTrue(approxEqual(hp50->shield_hp, 450.0f), "Ship 50 shield_hp preserved");

    auto* e99 = world2.getEntity("stress_persist_ship_99");
    assertTrue(e99 != nullptr, "Last ship entity found after load");
    auto* ship99 = e99->getComponent<components::Ship>();
    assertTrue(ship99 != nullptr, "Ship component on last entity");
    assertTrue(ship99->ship_type == "Frigate", "Last ship type preserved");

    // Verify LOD priorities survived
    auto* lod0 = e0->getComponent<components::LODPriority>();
    assertTrue(lod0 != nullptr, "LOD on first ship");
    assertTrue(approxEqual(lod0->priority, 2.0f), "High-priority LOD preserved");
    assertTrue(lod0->force_visible, "force_visible preserved on player ship");

    auto* lod50 = e50->getComponent<components::LODPriority>();
    assertTrue(lod50 != nullptr, "LOD on ship 50");
    assertTrue(approxEqual(lod50->priority, 0.5f), "Low-priority LOD preserved");

    // Clean up
    std::remove(filepath.c_str());
}

void testPersistenceFleetStateFile() {
    std::cout << "\n=== Persistence: Fleet State File Save/Load ===" << std::endl;

    ecs::World world;

    // Create a fleet commander entity
    auto* fc = world.createEntity("fleet_fc");
    auto* fm_fc = addComp<components::FleetMembership>(fc);
    fm_fc->fleet_id = "fleet_alpha";
    fm_fc->role = "FleetCommander";
    fm_fc->squad_id = "squad_1";
    fm_fc->wing_id = "wing_1";
    fm_fc->active_bonuses["armor_hp_bonus"] = 0.10f;

    auto* ff_fc = addComp<components::FleetFormation>(fc);
    ff_fc->formation = components::FleetFormation::FormationType::Arrow;
    ff_fc->slot_index = 0;
    ff_fc->offset_x = 0.0f;
    ff_fc->offset_y = 0.0f;
    ff_fc->offset_z = 0.0f;

    auto* morale = addComp<components::FleetMorale>(fc);
    morale->morale_score = 42.0f;
    morale->wins = 8;
    morale->losses = 2;
    morale->ships_lost = 1;
    morale->morale_state = "Inspired";

    auto* cargo = addComp<components::FleetCargoPool>(fc);
    cargo->total_capacity = 50000;
    cargo->used_capacity = 12000;
    cargo->pooled_items["Stellium"] = 5000;
    cargo->pooled_items["Vanthium"] = 3000;
    cargo->contributor_ship_ids.push_back("fleet_fc");
    cargo->contributor_ship_ids.push_back("fleet_member_1");

    // Create a fleet member entity
    auto* member = world.createEntity("fleet_member_1");
    auto* fm_m = addComp<components::FleetMembership>(member);
    fm_m->fleet_id = "fleet_alpha";
    fm_m->role = "Member";
    fm_m->squad_id = "squad_1";
    fm_m->wing_id = "wing_1";

    auto* ff_m = addComp<components::FleetFormation>(member);
    ff_m->formation = components::FleetFormation::FormationType::Arrow;
    ff_m->slot_index = 1;
    ff_m->offset_x = -50.0f;
    ff_m->offset_y = 0.0f;
    ff_m->offset_z = 25.0f;
    ff_m->spacing_modifier = 0.8f;

    // Save to file
    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_fleet_state.json";
    bool saved = persistence.saveWorld(&world, filepath);
    assertTrue(saved, "Fleet state saved to file");

    // Load into fresh world
    ecs::World world2;
    bool loaded = persistence.loadWorld(&world2, filepath);
    assertTrue(loaded, "Fleet state loaded from file");
    assertTrue(world2.getEntityCount() == 2, "Loaded 2 fleet entities");

    // Verify FC
    auto* fc2 = world2.getEntity("fleet_fc");
    assertTrue(fc2 != nullptr, "FC entity found after load");

    auto* fm_fc2 = fc2->getComponent<components::FleetMembership>();
    assertTrue(fm_fc2 != nullptr, "FleetMembership on FC after load");
    assertTrue(fm_fc2->fleet_id == "fleet_alpha", "FC fleet_id preserved");
    assertTrue(fm_fc2->role == "FleetCommander", "FC role preserved");

    auto* ff_fc2 = fc2->getComponent<components::FleetFormation>();
    assertTrue(ff_fc2 != nullptr, "FleetFormation on FC after load");
    assertTrue(ff_fc2->slot_index == 0, "FC slot_index preserved");

    auto* morale2 = fc2->getComponent<components::FleetMorale>();
    assertTrue(morale2 != nullptr, "FleetMorale on FC after load");
    assertTrue(approxEqual(morale2->morale_score, 42.0f), "morale_score preserved in file");
    assertTrue(morale2->wins == 8, "wins preserved in file");
    assertTrue(morale2->morale_state == "Inspired", "morale_state preserved in file");

    auto* cargo2 = fc2->getComponent<components::FleetCargoPool>();
    assertTrue(cargo2 != nullptr, "FleetCargoPool on FC after load");
    assertTrue(cargo2->total_capacity == 50000, "cargo total_capacity preserved");
    assertTrue(cargo2->used_capacity == 12000, "cargo used_capacity preserved");
    assertTrue(cargo2->pooled_items["Stellium"] == 5000, "cargo Stellium preserved");
    assertTrue(cargo2->pooled_items["Vanthium"] == 3000, "cargo Vanthium preserved");
    assertTrue(cargo2->contributor_ship_ids.size() == 2, "cargo contributors preserved");

    // Verify member
    auto* m2 = world2.getEntity("fleet_member_1");
    assertTrue(m2 != nullptr, "Member entity found after load");

    auto* fm_m2 = m2->getComponent<components::FleetMembership>();
    assertTrue(fm_m2 != nullptr, "FleetMembership on member after load");
    assertTrue(fm_m2->role == "Member", "Member role preserved");

    auto* ff_m2 = m2->getComponent<components::FleetFormation>();
    assertTrue(ff_m2 != nullptr, "FleetFormation on member after load");
    assertTrue(ff_m2->slot_index == 1, "Member slot_index preserved");
    assertTrue(approxEqual(ff_m2->offset_x, -50.0f), "Member offset_x preserved");
    assertTrue(approxEqual(ff_m2->spacing_modifier, 0.8f), "Member spacing_modifier preserved");

    // Clean up
    std::remove(filepath.c_str());
}

void testPersistenceEconomyFile() {
    std::cout << "\n=== Persistence: Economy State File Save/Load ===" << std::endl;

    ecs::World world;

    // Create market hub with multiple orders
    auto* hub = world.createEntity("market_hub_1");
    auto* market = addComp<components::MarketHub>(hub);
    market->station_id = "station_jita";
    market->broker_fee_rate = 0.025;
    market->sales_tax_rate = 0.04;

    // Add sell order
    components::MarketHub::Order sell;
    sell.order_id = "sell_001";
    sell.item_id = "trit";
    sell.item_name = "Stellium";
    sell.owner_id = "npc_trader_1";
    sell.is_buy_order = false;
    sell.price_per_unit = 6.0;
    sell.quantity = 50000;
    sell.quantity_remaining = 45000;
    sell.duration_remaining = 172800.0f;
    sell.fulfilled = false;
    market->orders.push_back(sell);

    // Add buy order
    components::MarketHub::Order buy;
    buy.order_id = "buy_001";
    buy.item_id = "pye";
    buy.item_name = "Vanthium";
    buy.owner_id = "npc_trader_2";
    buy.is_buy_order = true;
    buy.price_per_unit = 12.0;
    buy.quantity = 20000;
    buy.quantity_remaining = 20000;
    buy.duration_remaining = 86400.0f;
    buy.fulfilled = false;
    market->orders.push_back(buy);

    // Create mineral deposits
    auto* belt = world.createEntity("asteroid_belt_1");
    auto* deposit = addComp<components::MineralDeposit>(belt);
    deposit->mineral_type = "Ferrite";
    deposit->quantity_remaining = 25000.0f;
    deposit->max_quantity = 50000.0f;
    deposit->yield_rate = 1.0f;
    deposit->volume_per_unit = 0.1f;

    // Create system resources
    auto* sys = world.createEntity("system_res_1");
    auto* sysres = addComp<components::SystemResources>(sys);
    components::SystemResources::ResourceEntry r1;
    r1.mineral_type = "Stellium";
    r1.total_quantity = 100000.0f;
    r1.remaining_quantity = 75000.0f;
    sysres->resources.push_back(r1);
    components::SystemResources::ResourceEntry r2;
    r2.mineral_type = "Cydrium";
    r2.total_quantity = 30000.0f;
    r2.remaining_quantity = 28000.0f;
    sysres->resources.push_back(r2);

    // Save to file
    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_economy_state.json";
    bool saved = persistence.saveWorld(&world, filepath);
    assertTrue(saved, "Economy state saved to file");

    // Load into fresh world
    ecs::World world2;
    bool loaded = persistence.loadWorld(&world2, filepath);
    assertTrue(loaded, "Economy state loaded from file");
    assertTrue(world2.getEntityCount() == 3, "Loaded 3 economy entities");

    // Verify market hub
    auto* hub2 = world2.getEntity("market_hub_1");
    assertTrue(hub2 != nullptr, "Market hub found after load");
    auto* market2 = hub2->getComponent<components::MarketHub>();
    assertTrue(market2 != nullptr, "MarketHub component after load");
    assertTrue(market2->station_id == "station_jita", "station_id preserved");
    assertTrue(market2->orders.size() == 2, "Both orders preserved");
    assertTrue(market2->orders[0].order_id == "sell_001", "Sell order preserved");
    assertTrue(!market2->orders[0].is_buy_order, "Sell order type preserved");
    assertTrue(market2->orders[0].quantity_remaining == 45000, "Sell order qty remaining preserved");
    assertTrue(market2->orders[1].order_id == "buy_001", "Buy order preserved");
    assertTrue(market2->orders[1].is_buy_order, "Buy order type preserved");

    // Verify mineral deposit
    auto* belt2 = world2.getEntity("asteroid_belt_1");
    assertTrue(belt2 != nullptr, "Asteroid belt found after load");
    auto* deposit2 = belt2->getComponent<components::MineralDeposit>();
    assertTrue(deposit2 != nullptr, "MineralDeposit after load");
    assertTrue(deposit2->mineral_type == "Ferrite", "mineral_type preserved");
    assertTrue(approxEqual(deposit2->quantity_remaining, 25000.0f), "deposit qty remaining preserved");

    // Verify system resources
    auto* sys2 = world2.getEntity("system_res_1");
    assertTrue(sys2 != nullptr, "System resources found after load");
    auto* sysres2 = sys2->getComponent<components::SystemResources>();
    assertTrue(sysres2 != nullptr, "SystemResources after load");
    assertTrue(sysres2->resources.size() == 2, "Resource entries preserved");
    assertTrue(sysres2->resources[0].mineral_type == "Stellium", "Stellium entry preserved");
    assertTrue(approxEqual(sysres2->resources[0].remaining_quantity, 75000.0f), "Stellium remaining preserved");

    // Clean up
    std::remove(filepath.c_str());
}

// ==================== Phase 2: Background Simulation System Tests ====================

void testSimStarSystemStateDefaults() {
    std::cout << "\n=== SimStarSystemState Defaults ===" << std::endl;
    ecs::World world;
    auto* sys = world.createEntity("system_alpha");
    auto* state = addComp<components::SimStarSystemState>(sys);

    assertTrue(approxEqual(state->traffic_level, 0.5f), "Default traffic 0.5");
    assertTrue(approxEqual(state->economic_index, 0.5f), "Default economy 0.5");
    assertTrue(approxEqual(state->security_level, 0.5f), "Default security 0.5");
    assertTrue(approxEqual(state->threat_level, 0.0f), "Default threat 0.0");
    assertTrue(approxEqual(state->pirate_activity, 0.0f), "Default pirate activity 0.0");
    assertTrue(approxEqual(state->resource_availability, 1.0f), "Default resources 1.0");
    assertTrue(!state->pirate_surge, "No pirate surge by default");
    assertTrue(!state->resource_shortage, "No shortage by default");
    assertTrue(!state->lockdown, "No lockdown by default");
}

void testBackgroundSimThreatDecay() {
    std::cout << "\n=== Background Sim: Threat Decay ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_beta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.5f;

    bgSim.update(10.0f);  // 10 seconds

    assertTrue(state->threat_level < 0.5f, "Threat decayed after tick");
    assertTrue(state->threat_level > 0.0f, "Threat not fully gone after short tick");
}

void testBackgroundSimEconomyRecovery() {
    std::cout << "\n=== Background Sim: Economy Recovery ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_gamma");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.2f;

    bgSim.update(10.0f);

    assertTrue(state->economic_index > 0.2f, "Economy recovered from low");
    assertTrue(state->economic_index <= 0.5f, "Economy doesn't exceed baseline");
}

void testBackgroundSimResourceRegen() {
    std::cout << "\n=== Background Sim: Resource Regen ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_delta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->resource_availability = 0.3f;

    bgSim.update(10.0f);

    assertTrue(state->resource_availability > 0.3f, "Resources regenerated");
    assertTrue(state->resource_availability <= 1.0f, "Resources don't exceed max");
}

void testBackgroundSimPirateSurge() {
    std::cout << "\n=== Background Sim: Pirate Surge Event ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_epsilon");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->pirate_activity = 0.8f;  // above threshold (0.7)

    bgSim.update(1.0f);

    assertTrue(state->pirate_surge, "Pirate surge triggered at high activity");
    assertTrue(state->event_timer > 0.0f, "Event timer set");
    assertTrue(bgSim.isEventActive("system_epsilon", "pirate_surge"),
               "isEventActive returns true for pirate_surge");
}

void testBackgroundSimResourceShortage() {
    std::cout << "\n=== Background Sim: Resource Shortage Event ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_zeta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->resource_availability = 0.1f;  // below threshold (0.2)

    bgSim.update(1.0f);

    assertTrue(state->resource_shortage, "Resource shortage triggered");
    assertTrue(bgSim.isEventActive("system_zeta", "resource_shortage"),
               "isEventActive returns true for shortage");
}

void testBackgroundSimLockdown() {
    std::cout << "\n=== Background Sim: Lockdown Event ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_eta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.9f;  // above threshold (0.8)

    bgSim.update(1.0f);

    assertTrue(state->lockdown, "Lockdown triggered at extreme threat");
    assertTrue(bgSim.isEventActive("system_eta", "lockdown"),
               "isEventActive returns true for lockdown");
}

void testBackgroundSimEventTimerExpiry() {
    std::cout << "\n=== Background Sim: Event Timer Expiry ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);
    bgSim.event_duration = 10.0f;  // short duration for test

    auto* sys = world.createEntity("system_theta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->pirate_activity = 0.8f;

    bgSim.update(1.0f);
    assertTrue(state->pirate_surge, "Surge active initially");

    // Lower activity and wait for timer to expire
    state->pirate_activity = 0.1f;
    bgSim.update(15.0f);  // exceeds event_duration

    assertTrue(!state->pirate_surge, "Surge cleared after timer + conditions subsided");
}

void testBackgroundSimQuerySystems() {
    std::cout << "\n=== Background Sim: Query Systems with Event ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys1 = world.createEntity("system_1");
    auto* state1 = addComp<components::SimStarSystemState>(sys1);
    state1->pirate_activity = 0.9f;

    auto* sys2 = world.createEntity("system_2");
    auto* state2 = addComp<components::SimStarSystemState>(sys2);
    state2->pirate_activity = 0.1f;

    bgSim.update(1.0f);

    auto surged = bgSim.getSystemsWithEvent("pirate_surge");
    assertTrue(surged.size() == 1, "Only one system has pirate surge");
    assertTrue(surged[0] == "system_1", "Correct system has surge");
}

void testBackgroundSimPirateGrowth() {
    std::cout << "\n=== Background Sim: Pirate Growth in Low-Sec ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_lowsec");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.1f;  // low security
    state->pirate_activity = 0.3f;

    bgSim.update(10.0f);

    assertTrue(state->pirate_activity > 0.3f, "Pirate activity grew in low-sec");
}

void testBackgroundSimPriceModifier() {
    std::cout << "\n=== Background Sim: Price Modifier ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    auto* sys = world.createEntity("system_market");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->resource_availability = 0.3f;  // scarce
    state->trade_volume = 0.2f;           // low trade

    bgSim.update(1.0f);

    assertTrue(state->price_modifier > 1.0f, "Prices increase when resources scarce");
    assertTrue(state->price_modifier <= 2.0f, "Price modifier capped at 2.0");
}

void testBackgroundSimNoEventOnNonEntity() {
    std::cout << "\n=== Background Sim: No Event on Missing Entity ===" << std::endl;
    ecs::World world;
    systems::BackgroundSimulationSystem bgSim(&world);

    assertTrue(!bgSim.isEventActive("nonexistent", "pirate_surge"),
               "No event on missing entity");
    assertTrue(bgSim.getSystemState("nonexistent") == nullptr,
               "Null state for missing entity");
}

// ==================== Phase 2: NPC Intent System Tests ====================

void testSimNPCIntentDefaults() {
    std::cout << "\n=== SimNPCIntent Defaults ===" << std::endl;
    ecs::World world;
    auto* npc = world.createEntity("npc_trader_1");
    auto* intent = addComp<components::SimNPCIntent>(npc);

    assertTrue(intent->current_intent == components::SimNPCIntent::Intent::Idle,
               "Default intent is Idle");
    assertTrue(intent->archetype == components::SimNPCIntent::Archetype::Trader,
               "Default archetype is Trader");
    assertTrue(approxEqual(intent->wallet, 10000.0f, 1.0f), "Default wallet 10000");
    assertTrue(approxEqual(intent->cargo_fill, 0.0f), "Default cargo empty");
    assertTrue(!intent->intent_complete, "Intent not complete by default");
}

void testNPCIntentArchetypeWeights() {
    std::cout << "\n=== NPC Intent: Archetype Weights ===" << std::endl;

    // Test Trader archetype
    components::SimNPCIntent trader_intent;
    trader_intent.archetype = components::SimNPCIntent::Archetype::Trader;
    systems::NPCIntentSystem::applyArchetypeWeights(&trader_intent);
    assertTrue(trader_intent.trade_weight > 0.8f, "Trader has high trade weight");
    assertTrue(trader_intent.haul_weight > 0.5f, "Trader has decent haul weight");

    // Test Pirate archetype
    components::SimNPCIntent pirate_intent;
    pirate_intent.archetype = components::SimNPCIntent::Archetype::Pirate;
    systems::NPCIntentSystem::applyArchetypeWeights(&pirate_intent);
    assertTrue(pirate_intent.hunt_weight > 0.8f, "Pirate has high hunt weight");
    assertTrue(pirate_intent.salvage_weight > 0.4f, "Pirate has decent salvage weight");

    // Test Miner archetype
    components::SimNPCIntent miner_intent;
    miner_intent.archetype = components::SimNPCIntent::Archetype::Miner;
    systems::NPCIntentSystem::applyArchetypeWeights(&miner_intent);
    assertTrue(miner_intent.mine_weight > 0.8f, "Miner has high mine weight");
    assertTrue(miner_intent.flee_weight > 0.6f, "Miner has high flee weight (cautious)");

    // Test Patrol archetype
    components::SimNPCIntent patrol_intent;
    patrol_intent.archetype = components::SimNPCIntent::Archetype::Patrol;
    systems::NPCIntentSystem::applyArchetypeWeights(&patrol_intent);
    assertTrue(patrol_intent.patrol_weight > 0.8f, "Patrol has high patrol weight");
    assertTrue(patrol_intent.escort_weight > 0.6f, "Patrol has decent escort weight");

    // Test Hauler archetype
    components::SimNPCIntent hauler_intent;
    hauler_intent.archetype = components::SimNPCIntent::Archetype::Hauler;
    systems::NPCIntentSystem::applyArchetypeWeights(&hauler_intent);
    assertTrue(hauler_intent.haul_weight > 0.8f, "Hauler has high haul weight");

    // Test Industrialist archetype
    components::SimNPCIntent ind_intent;
    ind_intent.archetype = components::SimNPCIntent::Archetype::Industrialist;
    systems::NPCIntentSystem::applyArchetypeWeights(&ind_intent);
    assertTrue(ind_intent.trade_weight > 0.6f, "Industrialist has decent trade weight");
    assertTrue(ind_intent.mine_weight > 0.5f, "Industrialist has decent mine weight");
}

void testNPCIntentFleeOnLowHealth() {
    std::cout << "\n=== NPC Intent: Flee on Low Health ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 0.0f;  // instant re-eval for testing

    auto* npc = world.createEntity("npc_hurt");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Patrol;
    intent->archetype = components::SimNPCIntent::Archetype::Patrol;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);

    auto* health = addComp<components::Health>(npc);
    health->hull_hp = 10.0f;
    health->hull_max = 100.0f;  // 10% hull

    intentSys.update(1.0f);

    assertTrue(intent->current_intent == components::SimNPCIntent::Intent::Flee,
               "NPC flees when hull below 25%");
}

void testNPCIntentTraderInGoodEconomy() {
    std::cout << "\n=== NPC Intent: Trader Favors Trade in Good Economy ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 0.0f;

    // Create system with good economy
    auto* sys = world.createEntity("system_rich");
    auto* sysState = addComp<components::SimStarSystemState>(sys);
    sysState->economic_index = 0.9f;
    sysState->security_level = 0.8f;
    sysState->resource_availability = 0.8f;
    sysState->trade_volume = 0.7f;

    auto* npc = world.createEntity("npc_trader");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->archetype = components::SimNPCIntent::Archetype::Trader;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);
    intent->target_system_id = "system_rich";
    intent->cargo_fill = 0.6f;  // has cargo to sell
    addComp<components::Health>(npc);

    intentSys.update(1.0f);

    // Trader should choose Trade (high econ + cargo)
    auto scores = intentSys.scoreIntents("npc_trader");
    assertTrue(!scores.empty(), "Score intents returns results");
    assertTrue(scores[0].first == components::SimNPCIntent::Intent::Trade,
               "Trade scores highest for trader in good economy with cargo");
}

void testNPCIntentMinerInResourceSystem() {
    std::cout << "\n=== NPC Intent: Miner Prefers Mining in Rich System ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 0.0f;

    auto* sys = world.createEntity("system_ore");
    auto* sysState = addComp<components::SimStarSystemState>(sys);
    sysState->resource_availability = 0.9f;
    sysState->security_level = 0.8f;

    auto* npc = world.createEntity("npc_miner");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->archetype = components::SimNPCIntent::Archetype::Miner;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);
    intent->target_system_id = "system_ore";
    addComp<components::Health>(npc);

    auto scores = intentSys.scoreIntents("npc_miner");
    assertTrue(!scores.empty(), "Miner score intents not empty");

    // Find mine score
    float mine_score = 0.0f;
    for (auto& [i, s] : scores) {
        if (i == components::SimNPCIntent::Intent::Mine) mine_score = s;
    }
    assertTrue(mine_score > 0.5f, "Mining scores high in resource-rich system");
}

void testNPCIntentForceIntent() {
    std::cout << "\n=== NPC Intent: Force Intent ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);

    auto* npc = world.createEntity("npc_forced");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Idle;

    intentSys.forceIntent("npc_forced", components::SimNPCIntent::Intent::Dock);

    assertTrue(intent->current_intent == components::SimNPCIntent::Intent::Dock,
               "Intent forced to Dock");
    assertTrue(intent->previous_intent == components::SimNPCIntent::Intent::Idle,
               "Previous intent recorded");
    assertTrue(intent->intent_duration == 0.0f, "Intent duration reset");
}

void testNPCIntentQueryByIntent() {
    std::cout << "\n=== NPC Intent: Query NPCs by Intent ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);

    auto* npc1 = world.createEntity("npc_patrol_1");
    auto* i1 = addComp<components::SimNPCIntent>(npc1);
    i1->current_intent = components::SimNPCIntent::Intent::Patrol;

    auto* npc2 = world.createEntity("npc_patrol_2");
    auto* i2 = addComp<components::SimNPCIntent>(npc2);
    i2->current_intent = components::SimNPCIntent::Intent::Patrol;

    auto* npc3 = world.createEntity("npc_trade_1");
    auto* i3 = addComp<components::SimNPCIntent>(npc3);
    i3->current_intent = components::SimNPCIntent::Intent::Trade;

    auto patrollers = intentSys.getNPCsWithIntent(components::SimNPCIntent::Intent::Patrol);
    assertTrue(patrollers.size() == 2, "Two patrolling NPCs found");

    auto traders = intentSys.getNPCsWithIntent(components::SimNPCIntent::Intent::Trade);
    assertTrue(traders.size() == 1, "One trading NPC found");
}

void testNPCIntentQueryByArchetype() {
    std::cout << "\n=== NPC Intent: Query NPCs by Archetype ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);

    auto* npc1 = world.createEntity("npc_m1");
    auto* i1 = addComp<components::SimNPCIntent>(npc1);
    i1->archetype = components::SimNPCIntent::Archetype::Miner;

    auto* npc2 = world.createEntity("npc_m2");
    auto* i2 = addComp<components::SimNPCIntent>(npc2);
    i2->archetype = components::SimNPCIntent::Archetype::Miner;

    auto* npc3 = world.createEntity("npc_p1");
    auto* i3 = addComp<components::SimNPCIntent>(npc3);
    i3->archetype = components::SimNPCIntent::Archetype::Pirate;

    auto miners = intentSys.getNPCsByArchetype(components::SimNPCIntent::Archetype::Miner);
    assertTrue(miners.size() == 2, "Two miners found");

    auto pirates = intentSys.getNPCsByArchetype(components::SimNPCIntent::Archetype::Pirate);
    assertTrue(pirates.size() == 1, "One pirate found");
}

void testNPCIntentCooldownPreventsReeval() {
    std::cout << "\n=== NPC Intent: Cooldown Prevents Re-evaluation ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 30.0f;  // 30 second cooldown

    auto* npc = world.createEntity("npc_cooldown");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->archetype = components::SimNPCIntent::Archetype::Patrol;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);
    addComp<components::Health>(npc);

    // First update sets cooldown
    intentSys.update(1.0f);
    auto first_intent = intent->current_intent;

    // Second update within cooldown — intent should not change
    intentSys.update(1.0f);
    assertTrue(intent->current_intent == first_intent,
               "Intent unchanged during cooldown");
}

void testNPCIntentDockOnFullCargo() {
    std::cout << "\n=== NPC Intent: Dock on Full Cargo ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);
    intentSys.re_eval_interval = 0.0f;

    auto* npc = world.createEntity("npc_full_cargo");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->archetype = components::SimNPCIntent::Archetype::Hauler;
    systems::NPCIntentSystem::applyArchetypeWeights(intent);
    intent->cargo_fill = 0.95f;  // nearly full
    addComp<components::Health>(npc);

    auto scores = intentSys.scoreIntents("npc_full_cargo");
    float dock_score = 0.0f;
    for (auto& [i, s] : scores) {
        if (i == components::SimNPCIntent::Intent::Dock) dock_score = s;
    }
    assertTrue(dock_score > 0.5f, "Dock scores high when cargo full");
}

void testNPCIntentGetIntentMissing() {
    std::cout << "\n=== NPC Intent: Get Intent on Missing Entity ===" << std::endl;
    ecs::World world;
    systems::NPCIntentSystem intentSys(&world);

    auto result = intentSys.getIntent("nonexistent");
    assertTrue(result == components::SimNPCIntent::Intent::Idle,
               "Missing entity returns Idle");
}

// ==================== NPC Behavior Tree System Tests ====================

void testNPCBehaviorTreeDefaults() {
    std::cout << "\n=== NPC Behavior Tree: Defaults ===" << std::endl;
    ecs::World world;

    auto* npc = world.createEntity("bt_npc");
    auto* behavior = addComp<components::NPCBehaviorState>(npc);

    assertTrue(behavior->phases.empty(), "Phases initially empty");
    assertTrue(behavior->current_phase_index == 0, "Phase index starts at 0");
    assertTrue(behavior->tree_complete == false, "Tree not complete initially");
    assertTrue(behavior->phase_duration == 10.0f, "Default phase duration 10s");
}

void testNPCBehaviorTreeTraderPhases() {
    std::cout << "\n=== NPC Behavior Tree: Trader Trade Phases ===" << std::endl;
    using A = components::SimNPCIntent::Archetype;
    using I = components::SimNPCIntent::Intent;

    auto phases = systems::NPCBehaviorTreeSystem::getPhasesForIntent(A::Trader, I::Trade);
    assertTrue(phases.size() == 5, "Trader trade has 5 phases");
    assertTrue(phases[0] == "CheckMarketPrices", "First phase is CheckMarketPrices");
    assertTrue(phases[4] == "EvaluateProfit", "Last phase is EvaluateProfit");
}

void testNPCBehaviorTreeMinerPhases() {
    std::cout << "\n=== NPC Behavior Tree: Miner Mine Phases ===" << std::endl;
    using A = components::SimNPCIntent::Archetype;
    using I = components::SimNPCIntent::Intent;

    auto phases = systems::NPCBehaviorTreeSystem::getPhasesForIntent(A::Miner, I::Mine);
    assertTrue(phases.size() == 5, "Miner mine has 5 phases");
    assertTrue(phases[0] == "FindDeposit", "First phase is FindDeposit");
    assertTrue(phases[2] == "ActivateLasers", "Third phase is ActivateLasers");
}

void testNPCBehaviorTreePiratePhases() {
    std::cout << "\n=== NPC Behavior Tree: Pirate Hunt Phases ===" << std::endl;
    using A = components::SimNPCIntent::Archetype;
    using I = components::SimNPCIntent::Intent;

    auto phases = systems::NPCBehaviorTreeSystem::getPhasesForIntent(A::Pirate, I::Hunt);
    assertTrue(phases.size() == 5, "Pirate hunt has 5 phases");
    assertTrue(phases[0] == "ScanForPrey", "First phase is ScanForPrey");
    assertTrue(phases[3] == "Engage", "Fourth phase is Engage");
}

void testNPCBehaviorTreePhaseAdvancement() {
    std::cout << "\n=== NPC Behavior Tree: Phase Advancement ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    auto* npc = world.createEntity("bt_advance");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Trade;
    intent->archetype = components::SimNPCIntent::Archetype::Trader;

    auto* behavior = addComp<components::NPCBehaviorState>(npc);
    behavior->phase_duration = 5.0f;

    // First tick builds the tree
    btSys.update(0.0f);
    assertTrue(behavior->phases.size() == 5, "Phases populated from intent");
    assertTrue(btSys.getCurrentPhase("bt_advance") == "CheckMarketPrices",
               "Starts at first phase");

    // Advance time past phase_duration
    btSys.update(6.0f);
    assertTrue(behavior->current_phase_index == 1, "Advanced to phase 1");
    assertTrue(btSys.getCurrentPhase("bt_advance") == "BuyGoods",
               "Now in BuyGoods phase");
}

void testNPCBehaviorTreeCompletion() {
    std::cout << "\n=== NPC Behavior Tree: Completion ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    auto* npc = world.createEntity("bt_complete");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Dock;
    intent->archetype = components::SimNPCIntent::Archetype::Trader;

    auto* behavior = addComp<components::NPCBehaviorState>(npc);
    behavior->phase_duration = 1.0f;

    // Build tree
    btSys.update(0.0f);
    int numPhases = static_cast<int>(behavior->phases.size());
    assertTrue(numPhases == 3, "Dock has 3 phases");

    // Advance through all phases
    for (int i = 0; i < numPhases; i++) {
        btSys.update(2.0f);
    }

    assertTrue(behavior->tree_complete, "Tree marked complete");
    assertTrue(intent->intent_complete, "Intent marked complete");
    assertTrue(btSys.isTreeComplete("bt_complete"), "Query confirms complete");
}

void testNPCBehaviorTreeIntentChange() {
    std::cout << "\n=== NPC Behavior Tree: Intent Change Rebuilds Tree ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    auto* npc = world.createEntity("bt_change");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Trade;
    intent->archetype = components::SimNPCIntent::Archetype::Trader;

    auto* behavior = addComp<components::NPCBehaviorState>(npc);
    behavior->phase_duration = 1.0f;

    btSys.update(0.0f);
    assertTrue(behavior->phases.size() == 5, "Trade has 5 phases");

    // Change intent
    intent->current_intent = components::SimNPCIntent::Intent::Flee;
    btSys.update(0.0f);
    assertTrue(behavior->current_phase_index == 0, "Phase reset to 0");
    assertTrue(behavior->phases[0] == "SelectSafespot", "Flee starts at SelectSafespot");
    assertTrue(!behavior->tree_complete, "Tree no longer complete");
}

void testNPCBehaviorTreeReset() {
    std::cout << "\n=== NPC Behavior Tree: Reset ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    auto* npc = world.createEntity("bt_reset");
    auto* intent = addComp<components::SimNPCIntent>(npc);
    intent->current_intent = components::SimNPCIntent::Intent::Patrol;
    intent->archetype = components::SimNPCIntent::Archetype::Patrol;

    auto* behavior = addComp<components::NPCBehaviorState>(npc);
    behavior->phase_duration = 1.0f;

    btSys.update(0.0f);
    btSys.update(2.0f);
    assertTrue(behavior->current_phase_index > 0, "Advanced past phase 0");

    btSys.resetTree("bt_reset");
    assertTrue(behavior->current_phase_index == 0, "Reset to phase 0");
    assertTrue(behavior->phase_elapsed == 0.0f, "Elapsed reset to 0");
    assertTrue(!behavior->tree_complete, "Tree not complete after reset");
}

void testNPCBehaviorTreeGenericPhases() {
    std::cout << "\n=== NPC Behavior Tree: Generic Phase Fallback ===" << std::endl;
    using A = components::SimNPCIntent::Archetype;
    using I = components::SimNPCIntent::Intent;

    // Trader exploring uses generic explore phases
    auto phases = systems::NPCBehaviorTreeSystem::getPhasesForIntent(A::Trader, I::Explore);
    assertTrue(phases.size() == 3, "Generic explore has 3 phases");
    assertTrue(phases[0] == "SelectDestination", "Generic explore starts with SelectDestination");
}

void testNPCBehaviorTreeMissingEntity() {
    std::cout << "\n=== NPC Behavior Tree: Missing Entity Queries ===" << std::endl;
    ecs::World world;
    systems::NPCBehaviorTreeSystem btSys(&world);

    assertTrue(btSys.getCurrentPhase("nobody") == "", "Missing entity returns empty phase");
    assertTrue(btSys.isTreeComplete("nobody"), "Missing entity returns true for complete");
    assertTrue(btSys.getPhaseElapsed("nobody") == 0.0f, "Missing entity returns 0 elapsed");
}

// ==================== Ambient Traffic System Tests ====================

void testAmbientTrafficDefaults() {
    std::cout << "\n=== Ambient Traffic: Defaults ===" << std::endl;
    ecs::World world;

    auto* sys = world.createEntity("traffic_sys");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);

    assertTrue(traffic->spawn_timer == 60.0f, "Default spawn timer is 60s");
    assertTrue(traffic->active_traffic_count == 0, "No active traffic initially");
    assertTrue(traffic->pending_spawns.empty(), "No pending spawns initially");
}

void testAmbientTrafficSpawnOnTimer() {
    std::cout << "\n=== Ambient Traffic: Spawns On Timer ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 5.0f;  // fast for testing

    auto* sys = world.createEntity("traffic_eco");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 1.0f;  // about to fire
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;
    state->resource_availability = 0.7f;
    state->pirate_activity = 0.5f;
    state->security_level = 0.6f;
    state->trade_volume = 0.6f;

    atSys.update(2.0f);  // timer fires
    auto spawns = atSys.getPendingSpawns("traffic_eco");
    assertTrue(!spawns.empty(), "Spawns generated after timer fires");
    assertTrue(traffic->active_traffic_count > 0, "Active traffic count increased");
}

void testAmbientTrafficTraderSpawn() {
    std::cout << "\n=== Ambient Traffic: Trader Spawn on Good Economy ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_trader");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;
    state->resource_availability = 0.0f;
    state->pirate_activity = 0.0f;
    state->security_level = 0.0f;
    state->trade_volume = 0.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_trader");
    bool has_trader = false;
    for (auto& s : spawns) if (s == "trader") has_trader = true;
    assertTrue(has_trader, "Trader spawned in good economy");
}

void testAmbientTrafficMinerSpawn() {
    std::cout << "\n=== Ambient Traffic: Miner Spawn on Rich Resources ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_miner");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.0f;
    state->resource_availability = 0.9f;
    state->pirate_activity = 0.0f;
    state->security_level = 0.0f;
    state->trade_volume = 0.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_miner");
    bool has_miner = false;
    for (auto& s : spawns) if (s == "miner") has_miner = true;
    assertTrue(has_miner, "Miner spawned in resource-rich system");
}

void testAmbientTrafficCapReached() {
    std::cout << "\n=== Ambient Traffic: No Spawn When Cap Reached ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;
    atSys.max_traffic_per_system = 5;

    auto* sys = world.createEntity("traffic_full");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    traffic->active_traffic_count = 5;  // already at cap
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 1.0f;

    atSys.update(1.0f);
    auto spawns = atSys.getPendingSpawns("traffic_full");
    assertTrue(spawns.empty(), "No spawns when at traffic cap");
}

void testAmbientTrafficClearPending() {
    std::cout << "\n=== Ambient Traffic: Clear Pending Spawns ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);
    atSys.spawn_interval = 1.0f;

    auto* sys = world.createEntity("traffic_clear");
    auto* traffic = addComp<components::AmbientTrafficState>(sys);
    traffic->spawn_timer = 0.5f;
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->economic_index = 0.8f;

    atSys.update(1.0f);
    assertTrue(!traffic->pending_spawns.empty(), "Pending spawns exist");

    atSys.clearPendingSpawns("traffic_clear");
    assertTrue(traffic->pending_spawns.empty(), "Pending spawns cleared");
}

void testAmbientTrafficMissingEntity() {
    std::cout << "\n=== Ambient Traffic: Missing Entity Queries ===" << std::endl;
    ecs::World world;
    systems::AmbientTrafficSystem atSys(&world);

    auto spawns = atSys.getPendingSpawns("nobody");
    assertTrue(spawns.empty(), "No spawns for missing entity");
    assertTrue(atSys.getActiveTrafficCount("nobody") == 0, "Zero traffic for missing entity");
}

// ==================== Tactical Overlay Stage 4 Tests ====================

void testOverlayAnchorRing() {
    std::cout << "\n=== Tactical Overlay: Anchor Ring ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    auto* player = world.createEntity("overlay_anchor");

    toSys.setAnchorRing("overlay_anchor", "fc_ship", 25.0f);
    assertTrue(approxEqual(toSys.getAnchorRingRadius("overlay_anchor"), 25.0f),
               "Anchor ring radius set to 25");
}

void testOverlayAnchorRingDisabled() {
    std::cout << "\n=== Tactical Overlay: Anchor Ring Disabled ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    auto* player = world.createEntity("overlay_noanchor");
    addComp<components::TacticalOverlayState>(player);

    assertTrue(toSys.getAnchorRingRadius("overlay_noanchor") == 0.0f,
               "Anchor ring disabled by default");
}

void testOverlayWingBands() {
    std::cout << "\n=== Tactical Overlay: Wing Bands ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    auto* player = world.createEntity("overlay_wings");

    std::vector<float> offsets = {10.0f, 20.0f, 30.0f};
    toSys.setWingBands("overlay_wings", true, offsets);
    assertTrue(toSys.areWingBandsEnabled("overlay_wings"), "Wing bands enabled");

    auto result = toSys.getWingBandOffsets("overlay_wings");
    assertTrue(result.size() == 3, "Three wing band offsets");
    assertTrue(approxEqual(result[0], 10.0f), "First offset correct");
    assertTrue(approxEqual(result[2], 30.0f), "Third offset correct");
}

void testOverlayWingBandsDisabledByDefault() {
    std::cout << "\n=== Tactical Overlay: Wing Bands Disabled Default ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    auto* player = world.createEntity("overlay_nowings");
    addComp<components::TacticalOverlayState>(player);

    assertTrue(!toSys.areWingBandsEnabled("overlay_nowings"), "Wing bands disabled by default");
    assertTrue(toSys.getWingBandOffsets("overlay_nowings").empty(), "No offsets by default");
}

void testOverlayFleetExtensionsMissing() {
    std::cout << "\n=== Tactical Overlay: Fleet Extensions Missing Entity ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem toSys(&world);

    assertTrue(toSys.getAnchorRingRadius("nobody") == 0.0f, "Missing entity anchor ring is 0");
    assertTrue(!toSys.areWingBandsEnabled("nobody"), "Missing entity wing bands disabled");
    assertTrue(toSys.getWingBandOffsets("nobody").empty(), "Missing entity no offsets");
}

// ==================== InterestManagementSystem Tests ====================

void testInterestRegisterClient() {
    std::cout << "\n=== Interest: Register Client ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);

    auto* player = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(player);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;

    ims.registerClient(1, "player_1");
    assertTrue(ims.getClientCount() == 1, "One client registered");
}

void testInterestNearEntityIncluded() {
    std::cout << "\n=== Interest: Near Entity Included ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* player = world.createEntity("player_1");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 0.0f; ppos->y = 0.0f; ppos->z = 0.0f;

    auto* npc = world.createEntity("npc_1");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 50.0f; npos->y = 0.0f; npos->z = 0.0f;

    ims.registerClient(1, "player_1");
    ims.update(0.0f);

    assertTrue(ims.isRelevant(1, "npc_1"), "Near entity is relevant");
    assertTrue(ims.isRelevant(1, "player_1"), "Own entity is always relevant");
}

void testInterestFarEntityExcluded() {
    std::cout << "\n=== Interest: Far Entity Excluded ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* player = world.createEntity("player_1");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 0.0f; ppos->y = 0.0f; ppos->z = 0.0f;

    auto* npc = world.createEntity("npc_far");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 200.0f; npos->y = 0.0f; npos->z = 0.0f;

    ims.registerClient(1, "player_1");
    ims.update(0.0f);

    assertTrue(!ims.isRelevant(1, "npc_far"), "Far entity excluded");
}

void testInterestForceVisible() {
    std::cout << "\n=== Interest: Force Visible ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* player = world.createEntity("player_1");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 0.0f; ppos->y = 0.0f; ppos->z = 0.0f;

    auto* npc = world.createEntity("fleet_member");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 500.0f; npos->y = 0.0f; npos->z = 0.0f;  // beyond range

    ims.registerClient(1, "player_1");
    ims.addForceVisible(1, "fleet_member");
    ims.update(0.0f);

    assertTrue(ims.isRelevant(1, "fleet_member"),
               "Force-visible entity included despite distance");
}

void testInterestForceVisibleRemove() {
    std::cout << "\n=== Interest: Remove Force Visible ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* player = world.createEntity("player_1");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 0.0f; ppos->y = 0.0f; ppos->z = 0.0f;

    auto* npc = world.createEntity("fleet_member");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 500.0f; npos->y = 0.0f; npos->z = 0.0f;

    ims.registerClient(1, "player_1");
    ims.addForceVisible(1, "fleet_member");
    ims.update(0.0f);
    assertTrue(ims.isRelevant(1, "fleet_member"), "Force visible before remove");

    ims.removeForceVisible(1, "fleet_member");
    ims.update(0.0f);
    assertTrue(!ims.isRelevant(1, "fleet_member"), "Not relevant after removing force visible");
}

void testInterestUnregisterClient() {
    std::cout << "\n=== Interest: Unregister Client ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);

    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);

    ims.registerClient(1, "player_1");
    assertTrue(ims.getClientCount() == 1, "Client registered");

    ims.unregisterClient(1);
    assertTrue(ims.getClientCount() == 0, "Client unregistered");
    assertTrue(ims.getRelevantCount(1) == 0, "No relevant entities for unregistered client");
}

void testInterestMultipleClients() {
    std::cout << "\n=== Interest: Multiple Clients ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);
    ims.setFarRange(100.0f);

    auto* p1 = world.createEntity("player_1");
    auto* p1pos = addComp<components::Position>(p1);
    p1pos->x = 0.0f; p1pos->y = 0.0f; p1pos->z = 0.0f;

    auto* p2 = world.createEntity("player_2");
    auto* p2pos = addComp<components::Position>(p2);
    p2pos->x = 200.0f; p2pos->y = 0.0f; p2pos->z = 0.0f;

    auto* npc = world.createEntity("npc_1");
    auto* npos = addComp<components::Position>(npc);
    npos->x = 50.0f; npos->y = 0.0f; npos->z = 0.0f;

    ims.registerClient(1, "player_1");
    ims.registerClient(2, "player_2");
    ims.update(0.0f);

    assertTrue(ims.isRelevant(1, "npc_1"), "NPC near player 1 is relevant to client 1");
    assertTrue(!ims.isRelevant(2, "npc_1"), "NPC far from player 2 is not relevant to client 2");
}

void testInterestEntityNoPosition() {
    std::cout << "\n=== Interest: Entity Without Position Included ===" << std::endl;

    ecs::World world;
    systems::InterestManagementSystem ims(&world);

    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);

    // System entity without position
    world.createEntity("system_entity");

    ims.registerClient(1, "player_1");
    ims.update(0.0f);

    assertTrue(ims.isRelevant(1, "system_entity"),
               "Entity without position is always relevant");
}

// ==================== NPC Rerouting System Tests ====================

void testNPCReroutingNoDanger() {
    std::cout << "\n=== NPC Rerouting No Danger ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("sys_safe");
    auto* sys_state = addComp<components::SimStarSystemState>(sys_entity);
    sys_state->threat_level = 0.2f;

    auto* npc = world.createEntity("npc1");
    auto* route = addComp<components::NPCRouteState>(npc);
    route->planned_route = {"sys_safe"};

    systems::NPCReroutingSystem sys(&world);
    sys.update(1.0f);
    assertTrue(!route->rerouting, "No rerouting when threat is low");
    assertTrue(route->planned_route.size() == 1, "Route unchanged");
}

void testNPCReroutingDangerousSystem() {
    std::cout << "\n=== NPC Rerouting Dangerous System ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("sys_danger");
    auto* sys_state = addComp<components::SimStarSystemState>(sys_entity);
    sys_state->threat_level = 0.9f;

    auto* npc = world.createEntity("npc2");
    auto* route = addComp<components::NPCRouteState>(npc);
    route->planned_route = {"sys_danger"};

    systems::NPCReroutingSystem sys(&world);
    sys.update(1.0f);
    assertTrue(route->rerouting, "Rerouting triggered by dangerous system");
    assertTrue(route->planned_route.empty(), "Dangerous system removed from route");
}

void testNPCReroutingCooldown() {
    std::cout << "\n=== NPC Rerouting Cooldown ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("sys_mid");
    auto* sys_state = addComp<components::SimStarSystemState>(sys_entity);
    sys_state->threat_level = 0.9f;

    auto* npc = world.createEntity("npc3");
    auto* route = addComp<components::NPCRouteState>(npc);
    route->planned_route = {"sys_mid"};
    route->reroute_cooldown = 100.0f;

    systems::NPCReroutingSystem sys(&world);
    sys.update(1.0f);
    assertTrue(!route->rerouting, "Cooldown prevents rerouting");
}

// ==================== ServerPerformanceMonitor Tests ====================

void testPerfMonitorInit() {
    std::cout << "\n=== PerfMonitor: Init ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    assertTrue(sys.initializeMonitor("mon_1", "server_1", 50.0f), "Monitor initialized");
    assertTrue(approxEqual(sys.getAverageTickTime("mon_1"), 0.0f), "Avg tick 0 initially");
    assertTrue(approxEqual(sys.getBudgetUtilization("mon_1"), 0.0f), "Budget 0 initially");
    assertTrue(!sys.isAlertActive("mon_1"), "No alert initially");
    assertTrue(!sys.initializeMonitor("mon_1", "server_1", 50.0f), "Duplicate init fails");
}

void testPerfMonitorRecordTiming() {
    std::cout << "\n=== PerfMonitor: Record Timing ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    assertTrue(sys.recordSystemTiming("mon_1", "PhysicsSystem", 10.0f), "Timing recorded");
    assertTrue(sys.recordSystemTiming("mon_1", "RenderSystem", 5.0f), "Second timing recorded");
    sys.update(0.016f);
    assertTrue(sys.getSlowestSystem("mon_1") == "PhysicsSystem", "Physics is slowest");
}

void testPerfMonitorTickComplete() {
    std::cout << "\n=== PerfMonitor: Tick Complete ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    assertTrue(sys.recordTickComplete("mon_1", 30.0f, 100), "Tick recorded");
    assertTrue(approxEqual(sys.getAverageTickTime("mon_1"), 30.0f), "Avg is 30ms");
    assertTrue(approxEqual(sys.getBudgetUtilization("mon_1"), 0.6f), "Budget util is 0.6");
}

void testPerfMonitorAverage() {
    std::cout << "\n=== PerfMonitor: Average ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordTickComplete("mon_1", 20.0f, 100);
    sys.recordTickComplete("mon_1", 40.0f, 100);
    float avg = sys.getAverageTickTime("mon_1");
    assertTrue(approxEqual(avg, 30.0f), "Average of 20 and 40 is 30");
}

void testPerfMonitorBudget() {
    std::cout << "\n=== PerfMonitor: Budget ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordTickComplete("mon_1", 25.0f, 50);
    float util = sys.getBudgetUtilization("mon_1");
    assertTrue(approxEqual(util, 0.5f), "25/50 = 50% utilization");
}

void testPerfMonitorAlert() {
    std::cout << "\n=== PerfMonitor: Alert ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordTickComplete("mon_1", 45.0f, 100);
    sys.update(0.016f);
    assertTrue(sys.isAlertActive("mon_1"), "Alert active at 90% budget");
    sys.resetMetrics("mon_1");
    sys.recordTickComplete("mon_1", 10.0f, 100);
    sys.update(0.016f);
    assertTrue(!sys.isAlertActive("mon_1"), "No alert at 20% budget");
}

void testPerfMonitorHotPath() {
    std::cout << "\n=== PerfMonitor: Hot Path ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    // Hot path threshold: 25% of 50ms = 12.5ms
    sys.recordSystemTiming("mon_1", "PhysicsSystem", 15.0f);
    sys.recordSystemTiming("mon_1", "AISystem", 14.0f);
    sys.recordSystemTiming("mon_1", "RenderSystem", 5.0f);
    sys.update(0.016f);
    assertTrue(sys.getHotPathCount("mon_1") == 2, "2 hot paths (Physics + AI)");
}

void testPerfMonitorSlowest() {
    std::cout << "\n=== PerfMonitor: Slowest ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordSystemTiming("mon_1", "SystemA", 5.0f);
    sys.recordSystemTiming("mon_1", "SystemB", 20.0f);
    sys.recordSystemTiming("mon_1", "SystemC", 8.0f);
    sys.update(0.016f);
    assertTrue(sys.getSlowestSystem("mon_1") == "SystemB", "SystemB is slowest");
}

void testPerfMonitorReset() {
    std::cout << "\n=== PerfMonitor: Reset ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    world.createEntity("mon_1");
    sys.initializeMonitor("mon_1", "server_1", 50.0f);
    sys.recordTickComplete("mon_1", 40.0f, 200);
    sys.recordSystemTiming("mon_1", "SysA", 15.0f);
    assertTrue(sys.resetMetrics("mon_1"), "Reset succeeds");
    assertTrue(approxEqual(sys.getAverageTickTime("mon_1"), 0.0f), "Avg 0 after reset");
    assertTrue(approxEqual(sys.getBudgetUtilization("mon_1"), 0.0f), "Budget 0 after reset");
    assertTrue(sys.getSlowestSystem("mon_1").empty(), "No slowest after reset");
}

void testPerfMonitorMissing() {
    std::cout << "\n=== PerfMonitor: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::ServerPerformanceMonitorSystem sys(&world);
    assertTrue(!sys.initializeMonitor("nonexistent", "s", 50.0f), "Init fails on missing");
    assertTrue(approxEqual(sys.getAverageTickTime("nonexistent"), 0.0f), "Avg 0 on missing");
    assertTrue(approxEqual(sys.getBudgetUtilization("nonexistent"), 0.0f), "Budget 0 on missing");
    assertTrue(!sys.isAlertActive("nonexistent"), "No alert on missing");
    assertTrue(sys.getSlowestSystem("nonexistent").empty(), "Empty slowest on missing");
}

// ==================== ClientPredictionSystem Tests ====================

void testClientPredictionInit() {
    std::cout << "\n=== ClientPrediction: Init ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    assertTrue(sys.initPrediction("ship1", "client_abc"), "Init prediction succeeds");
    auto* cp = e->getComponent<components::ClientPrediction>();
    assertTrue(cp != nullptr, "Component exists");
    assertTrue(cp->client_id == "client_abc", "Client ID set");
    assertTrue(cp->active == true, "Active by default");
}

void testClientPredictionServerState() {
    std::cout << "\n=== ClientPrediction: Server State ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    assertTrue(sys.setServerState("ship1", 10.0f, 20.0f, 30.0f, 5), "Set server state");
    assertTrue(sys.setServerState("nonexistent", 0, 0, 0, 0) == false, "Fails on missing");
}

void testClientPredictionApplyInput() {
    std::cout << "\n=== ClientPrediction: Apply Input ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    assertTrue(sys.applyInput("ship1", 1.0f, 0.0f, 0.0f, 10), "Apply input succeeds");
    assertTrue(sys.getPredictionFrame("ship1") == 10, "Frame set to 10");
}

void testClientPredictionUpdate() {
    std::cout << "\n=== ClientPrediction: Update ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    sys.applyInput("ship1", 10.0f, 0.0f, 0.0f, 1);
    sys.update(0.1f);
    auto* cp = e->getComponent<components::ClientPrediction>();
    assertTrue(cp->predicted_x > 0.0f, "Predicted X advanced by velocity");
}

void testClientPredictionError() {
    std::cout << "\n=== ClientPrediction: Error Calculation ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    auto* cp = e->getComponent<components::ClientPrediction>();
    cp->predicted_x = 10.0f;
    cp->server_x = 0.0f;
    sys.update(0.0f);
    assertTrue(sys.getPredictionError("ship1") > 0.0f, "Error is non-zero");
}

void testClientPredictionReconciliation() {
    std::cout << "\n=== ClientPrediction: Reconciliation ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    auto* cp = e->getComponent<components::ClientPrediction>();
    cp->predicted_x = 100.0f;
    cp->server_x = 0.0f;
    cp->correction_speed = 5.0f;
    sys.update(0.1f);
    assertTrue(sys.isReconciling("ship1"), "Is reconciling after error");
    assertTrue(sys.getCorrectionBlend("ship1") > 0.0f, "Blend advanced");
}

void testClientPredictionSnap() {
    std::cout << "\n=== ClientPrediction: Snap to Server ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    auto* cp = e->getComponent<components::ClientPrediction>();
    cp->predicted_x = 10.0f;
    cp->server_x = 0.0f;
    cp->correction_speed = 100.0f;
    sys.update(1.0f);
    assertTrue(approxEqual(cp->predicted_x, cp->server_x, 0.1f), "Snapped to server after full blend");
    assertTrue(approxEqual(cp->correction_blend, 0.0f), "Blend reset after snap");
}

void testClientPredictionInactive() {
    std::cout << "\n=== ClientPrediction: Inactive ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    auto* cp = e->getComponent<components::ClientPrediction>();
    cp->active = false;
    cp->velocity_x = 100.0f;
    sys.update(1.0f);
    assertTrue(approxEqual(cp->predicted_x, 0.0f), "Inactive entity not updated");
}

void testClientPredictionFrame() {
    std::cout << "\n=== ClientPrediction: Frame Tracking ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    world.createEntity("ship1");
    sys.initPrediction("ship1", "c1");
    sys.applyInput("ship1", 0, 0, 0, 42);
    assertTrue(sys.getPredictionFrame("ship1") == 42, "Prediction frame is 42");
    sys.setServerState("ship1", 0, 0, 0, 40);
    assertTrue(sys.getPredictionFrame("ship1") == 42, "Frame unchanged after server state");
}

void testClientPredictionMissing() {
    std::cout << "\n=== ClientPrediction: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::ClientPredictionSystem sys(&world);
    assertTrue(!sys.initPrediction("nonexistent", "c1"), "Init fails on missing");
    assertTrue(!sys.setServerState("nonexistent", 0, 0, 0, 0), "SetServer fails on missing");
    assertTrue(!sys.applyInput("nonexistent", 0, 0, 0, 0), "ApplyInput fails on missing");
    assertTrue(sys.getPredictionError("nonexistent") == 0.0f, "Error 0 on missing");
    assertTrue(!sys.isReconciling("nonexistent"), "Not reconciling on missing");
    assertTrue(sys.getCorrectionBlend("nonexistent") == 0.0f, "Blend 0 on missing");
    assertTrue(sys.getPredictionFrame("nonexistent") == 0, "Frame 0 on missing");
}

// ==================== CloudDeploymentConfig System Tests ====================

void testCloudDeploymentCreate() {
    std::cout << "\n=== CloudDeploymentConfig: Create ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    auto* e = world.createEntity("cloud1");
    assertTrue(sys.createConfig("cloud1"), "Create config succeeds");
    auto* cfg = e->getComponent<components::CloudDeploymentConfig>();
    assertTrue(cfg != nullptr, "Component exists");
    assertTrue(cfg->active, "Config is active by default");
    assertTrue(cfg->provider == 0, "Default provider is AWS (0)");
    assertTrue(cfg->max_players == 20, "Default max players is 20");
    assertTrue(!cfg->deployed, "Not deployed by default");
}

void testCloudDeploymentProvider() {
    std::cout << "\n=== CloudDeploymentConfig: Provider ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    auto* e = world.createEntity("cloud1");
    sys.createConfig("cloud1");
    assertTrue(sys.setProvider("cloud1", 1), "Set GCP succeeds");
    assertTrue(sys.getProvider("cloud1") == 1, "Provider is GCP (1)");
    auto* cfg = e->getComponent<components::CloudDeploymentConfig>();
    assertTrue(cfg->estimated_monthly_cost > 0.0f, "Cost calculated");
    assertTrue(!sys.setProvider("cloud1", 5), "Invalid provider rejected");
}

void testCloudDeploymentRegion() {
    std::cout << "\n=== CloudDeploymentConfig: Region ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    assertTrue(sys.setRegion("cloud1", "us-east-1"), "Set region succeeds");
    assertTrue(sys.getRegion("cloud1") == "us-east-1", "Region stored");
    assertTrue(!sys.setRegion("cloud1", ""), "Empty region rejected");
}

void testCloudDeploymentValidate() {
    std::cout << "\n=== CloudDeploymentConfig: Validate ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    assertTrue(!sys.validate("cloud1"), "Empty config fails validation");
    sys.setRegion("cloud1", "eu-west-1");
    assertTrue(!sys.validate("cloud1"), "Missing instance type fails");
    sys.setInstanceType("cloud1", "t3.medium");
    assertTrue(sys.validate("cloud1"), "Complete config validates");
}

void testCloudDeploymentDeploy() {
    std::cout << "\n=== CloudDeploymentConfig: Deploy ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    assertTrue(!sys.deploy("cloud1"), "Deploy fails without valid config");
    sys.setRegion("cloud1", "us-west-2");
    sys.setInstanceType("cloud1", "c5.large");
    assertTrue(sys.deploy("cloud1"), "Deploy succeeds with valid config");
    assertTrue(sys.isDeployed("cloud1"), "Is deployed");
}

void testCloudDeploymentUptime() {
    std::cout << "\n=== CloudDeploymentConfig: Uptime ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    sys.setRegion("cloud1", "us-east-1");
    sys.setInstanceType("cloud1", "t3.large");
    sys.deploy("cloud1");
    sys.update(10.0f);
    assertTrue(sys.getUptime("cloud1") > 9.0f, "Uptime tracked");
}

void testCloudDeploymentHealthCheck() {
    std::cout << "\n=== CloudDeploymentConfig: HealthCheck ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    sys.setRegion("cloud1", "us-east-1");
    sys.setInstanceType("cloud1", "t3.large");
    sys.enableHealthCheck("cloud1", 10.0f);
    sys.deploy("cloud1");
    sys.update(25.0f);
    assertTrue(sys.getHealthCheckCount("cloud1") == 2, "2 health checks after 25s with 10s interval");
}

void testCloudDeploymentMaxPlayers() {
    std::cout << "\n=== CloudDeploymentConfig: MaxPlayers ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    auto* e = world.createEntity("cloud1");
    sys.createConfig("cloud1");
    sys.setMaxPlayers("cloud1", 50);
    assertTrue(sys.getMaxPlayers("cloud1") == 50, "Max players set to 50");
    auto* cfg = e->getComponent<components::CloudDeploymentConfig>();
    assertTrue(cfg->estimated_monthly_cost > 0.0f, "Cost recalculated");
    sys.setMaxPlayers("cloud1", 200);
    assertTrue(sys.getMaxPlayers("cloud1") == 100, "Max clamped to 100");
}

void testCloudDeploymentCost() {
    std::cout << "\n=== CloudDeploymentConfig: Cost ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    world.createEntity("cloud1");
    sys.createConfig("cloud1");
    sys.setProvider("cloud1", 0);  // AWS
    sys.setMaxPlayers("cloud1", 20);
    float aws_cost = sys.getEstimatedMonthlyCost("cloud1");
    assertTrue(aws_cost > 0.0f, "AWS cost is positive");
    sys.setProvider("cloud1", 1);  // GCP
    float gcp_cost = sys.getEstimatedMonthlyCost("cloud1");
    assertTrue(gcp_cost > 0.0f, "GCP cost is positive");
    assertTrue(gcp_cost != aws_cost, "Different providers have different costs");
}

void testCloudDeploymentMissing() {
    std::cout << "\n=== CloudDeploymentConfig: Missing ===" << std::endl;
    ecs::World world;
    systems::CloudDeploymentConfigSystem sys(&world);
    assertTrue(!sys.createConfig("nonexistent"), "Create fails on missing");
    assertTrue(!sys.setProvider("nonexistent", 0), "SetProvider fails on missing");
    assertTrue(!sys.setRegion("nonexistent", "us-east-1"), "SetRegion fails on missing");
    assertTrue(!sys.setInstanceType("nonexistent", "t3"), "SetInstanceType fails on missing");
    assertTrue(!sys.setMaxPlayers("nonexistent", 20), "SetMaxPlayers fails on missing");
    assertTrue(!sys.validate("nonexistent"), "Validate fails on missing");
    assertTrue(!sys.deploy("nonexistent"), "Deploy fails on missing");
    assertTrue(sys.getProvider("nonexistent") == -1, "-1 provider on missing");
    assertTrue(sys.getMaxPlayers("nonexistent") == 0, "0 max players on missing");
    assertTrue(!sys.isDeployed("nonexistent"), "Not deployed on missing");
}

// ==================== InterestManagementPriority Tests ====================

void testInterestPriorityCreate() {
    std::cout << "\n=== InterestManagementPriority: Create ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    assertTrue(sys.createPriority("ip1"), "Create priority succeeds");
    auto* ip = e->getComponent<components::InterestPriority>();
    assertTrue(ip != nullptr, "Component exists");
    assertTrue(ip->active, "Active by default");
    assertTrue(ip->priority_tier == 2, "Default tier is Medium (2)");
    assertTrue(ip->needs_update, "Needs update by default");
    assertTrue(ip->total_updates == 0, "No updates yet");
}

void testInterestPrioritySetTier() {
    std::cout << "\n=== InterestManagementPriority: SetTier ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    world.createEntity("ip1");
    sys.createPriority("ip1");
    assertTrue(sys.setPriorityTier("ip1", 0), "Set Critical tier");
    assertTrue(sys.getPriorityTier("ip1") == 0, "Tier is Critical (0)");
    assertTrue(sys.getBandwidthWeight("ip1") == 1.0f, "Critical bandwidth weight is 1.0");
    assertTrue(sys.setPriorityTier("ip1", 3), "Set Low tier");
    assertTrue(sys.getBandwidthWeight("ip1") == 0.25f, "Low bandwidth weight is 0.25");
}

void testInterestPriorityDistanceAuto() {
    std::cout << "\n=== InterestManagementPriority: DistanceAuto ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    world.createEntity("ip1");
    sys.createPriority("ip1");
    assertTrue(sys.setDistance("ip1", 500.0f), "Set distance 500");
    assertTrue(sys.getPriorityTier("ip1") == 0, "Distance 500 -> Critical");
    assertTrue(sys.setDistance("ip1", 3000.0f), "Set distance 3000");
    assertTrue(sys.getPriorityTier("ip1") == 1, "Distance 3000 -> High");
    assertTrue(sys.setDistance("ip1", 10000.0f), "Set distance 10000");
    assertTrue(sys.getPriorityTier("ip1") == 2, "Distance 10000 -> Medium");
    assertTrue(sys.setDistance("ip1", 25000.0f), "Set distance 25000");
    assertTrue(sys.getPriorityTier("ip1") == 3, "Distance 25000 -> Low");
}

void testInterestPriorityUpdate() {
    std::cout << "\n=== InterestManagementPriority: Update ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 2); // Medium: 0.25s interval
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    sys.update(0.3f);
    assertTrue(sys.needsUpdate("ip1"), "Update triggered after 0.3s >= 0.25s");
    assertTrue(sys.getTotalUpdates("ip1") == 1, "Total updates is 1");
}

void testInterestPriorityAccumulator() {
    std::cout << "\n=== InterestManagementPriority: Accumulator ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 2); // Medium: 0.25s
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    sys.update(0.1f);
    assertTrue(!sys.needsUpdate("ip1"), "No update after 0.1s < 0.25s");
    sys.update(0.1f);
    assertTrue(!sys.needsUpdate("ip1"), "No update after 0.2s < 0.25s");
    sys.update(0.1f);
    assertTrue(sys.needsUpdate("ip1"), "Update after 0.3s >= 0.25s");
}

void testInterestPriorityCritical() {
    std::cout << "\n=== InterestManagementPriority: Critical ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 0); // Critical: 0.05s
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    sys.update(0.05f);
    assertTrue(sys.needsUpdate("ip1"), "Critical updates every 0.05s");
    assertTrue(sys.getTotalUpdates("ip1") == 1, "One update counted");
}

void testInterestPriorityLow() {
    std::cout << "\n=== InterestManagementPriority: Low ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 3); // Low: 0.5s
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    sys.update(0.4f);
    assertTrue(!sys.needsUpdate("ip1"), "No update at 0.4s < 0.5s");
    sys.update(0.2f);
    assertTrue(sys.needsUpdate("ip1"), "Update after 0.6s >= 0.5s");
}

void testInterestPriorityBandwidth() {
    std::cout << "\n=== InterestManagementPriority: Bandwidth ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 0);
    assertTrue(sys.getEstimatedBandwidth("ip1") == 100.0f, "Critical bandwidth = 100");
    sys.setPriorityTier("ip1", 1);
    assertTrue(sys.getEstimatedBandwidth("ip1") == 75.0f, "High bandwidth = 75");
    sys.setPriorityTier("ip1", 2);
    assertTrue(sys.getEstimatedBandwidth("ip1") == 50.0f, "Medium bandwidth = 50");
    sys.setPriorityTier("ip1", 3);
    assertTrue(sys.getEstimatedBandwidth("ip1") == 25.0f, "Low bandwidth = 25");
}

void testInterestPriorityInactive() {
    std::cout << "\n=== InterestManagementPriority: Inactive ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    auto* e = world.createEntity("ip1");
    sys.createPriority("ip1");
    sys.setPriorityTier("ip1", 0); // Critical: 0.05s
    auto* ip = e->getComponent<components::InterestPriority>();
    ip->needs_update = false;
    ip->active = false;
    sys.update(1.0f);
    assertTrue(!sys.needsUpdate("ip1"), "Inactive entity skipped");
    assertTrue(sys.getTotalUpdates("ip1") == 0, "No updates on inactive");
}

void testInterestPriorityMissing() {
    std::cout << "\n=== InterestManagementPriority: Missing ===" << std::endl;
    ecs::World world;
    systems::InterestManagementPrioritySystem sys(&world);
    assertTrue(!sys.createPriority("nonexistent"), "Create fails on missing");
    assertTrue(!sys.setClientId("nonexistent", 1), "SetClientId fails on missing");
    assertTrue(!sys.setPriorityTier("nonexistent", 0), "SetTier fails on missing");
    assertTrue(!sys.setDistance("nonexistent", 100.0f), "SetDistance fails on missing");
    assertTrue(!sys.needsUpdate("nonexistent"), "needsUpdate false on missing");
    assertTrue(sys.getPriorityTier("nonexistent") == -1, "-1 tier on missing");
    assertTrue(sys.getBandwidthWeight("nonexistent") == 0.0f, "0 bandwidth on missing");
    assertTrue(sys.getTotalUpdates("nonexistent") == 0, "0 updates on missing");
    assertTrue(sys.getEstimatedBandwidth("nonexistent") == 0.0f, "0 est bandwidth on missing");
}


void run_infrastructure_tests() {
    testLoggerLevels();
    testLoggerFileOutput();
    testLoggerLevelFiltering();
    testMetricsTickTiming();
    testMetricsCounters();
    testMetricsUptime();
    testMetricsSummary();
    testMetricsResetWindow();
    testConsoleInit();
    testConsoleHelpCommand();
    testConsoleStatusCommand();
    testConsoleUnknownCommand();
    testConsoleCustomCommand();
    testConsoleLogBuffer();
    testConsoleEmptyCommand();
    testConsoleNotInitialized();
    testConsoleShutdown();
    testConsoleInteractiveMode();
    testTacticalOverlayToggle();
    testTacticalOverlayToggleTwice();
    testTacticalOverlaySetToolRange();
    testTacticalOverlayRingDistances();
    testTacticalOverlayDefaultRings();
    testProtocolSalvageMessages();
    testProtocolLootMessages();
    testProtocolMiningMessages();
    testProtocolScannerMessages();
    testProtocolAnomalyListMessages();
    testProtocolScanResultParse();
    testProtocolLootResultParse();
    testProtocolMiningResultParse();
    testAIMiningState();
    testAIMiningBehaviorActivatesLaser();
    testAIMiningIdleFindsDeposit();
    testAIMiningApproachTransitions();
    testAIMiningStopsOnDepletedDeposit();
    testAIMiningStopsOnCargoFull();
    testAIFindNearestDeposit();
    testOverlaySharedFilters();
    testOverlayPassiveDisplayOnly();
    testOverlayEntityPriority();
    testOverlayEntityPriorityHostileHighAsteroidLow();
    testMissionProtocolRoundTrip();
    testMissionGeneratorNetworkFlow();
    testMissionAcceptAbandonNetworkFlow();
    testAIDefensiveBehavior();
    testAIDefensiveNoFriendly();
    testAIDefensiveIdleTransition();
    testPersistenceStress100Ships();
    testPersistenceFleetStateFile();
    testPersistenceEconomyFile();
    testSimStarSystemStateDefaults();
    testBackgroundSimThreatDecay();
    testBackgroundSimEconomyRecovery();
    testBackgroundSimResourceRegen();
    testBackgroundSimPirateSurge();
    testBackgroundSimResourceShortage();
    testBackgroundSimLockdown();
    testBackgroundSimEventTimerExpiry();
    testBackgroundSimQuerySystems();
    testBackgroundSimPirateGrowth();
    testBackgroundSimPriceModifier();
    testBackgroundSimNoEventOnNonEntity();
    testSimNPCIntentDefaults();
    testNPCIntentArchetypeWeights();
    testNPCIntentFleeOnLowHealth();
    testNPCIntentTraderInGoodEconomy();
    testNPCIntentMinerInResourceSystem();
    testNPCIntentForceIntent();
    testNPCIntentQueryByIntent();
    testNPCIntentQueryByArchetype();
    testNPCIntentCooldownPreventsReeval();
    testNPCIntentDockOnFullCargo();
    testNPCIntentGetIntentMissing();
    testNPCBehaviorTreeDefaults();
    testNPCBehaviorTreeTraderPhases();
    testNPCBehaviorTreeMinerPhases();
    testNPCBehaviorTreePiratePhases();
    testNPCBehaviorTreePhaseAdvancement();
    testNPCBehaviorTreeCompletion();
    testNPCBehaviorTreeIntentChange();
    testNPCBehaviorTreeReset();
    testNPCBehaviorTreeGenericPhases();
    testNPCBehaviorTreeMissingEntity();
    testAmbientTrafficDefaults();
    testAmbientTrafficSpawnOnTimer();
    testAmbientTrafficTraderSpawn();
    testAmbientTrafficMinerSpawn();
    testAmbientTrafficCapReached();
    testAmbientTrafficClearPending();
    testAmbientTrafficMissingEntity();
    testOverlayAnchorRing();
    testOverlayAnchorRingDisabled();
    testOverlayWingBands();
    testOverlayWingBandsDisabledByDefault();
    testOverlayFleetExtensionsMissing();
    testInterestRegisterClient();
    testInterestNearEntityIncluded();
    testInterestFarEntityExcluded();
    testInterestForceVisible();
    testInterestForceVisibleRemove();
    testInterestUnregisterClient();
    testInterestMultipleClients();
    testInterestEntityNoPosition();
    testNPCReroutingNoDanger();
    testNPCReroutingDangerousSystem();
    testNPCReroutingCooldown();
    testPerfMonitorInit();
    testPerfMonitorRecordTiming();
    testPerfMonitorTickComplete();
    testPerfMonitorAverage();
    testPerfMonitorBudget();
    testPerfMonitorAlert();
    testPerfMonitorHotPath();
    testPerfMonitorSlowest();
    testPerfMonitorReset();
    testPerfMonitorMissing();
    testClientPredictionInit();
    testClientPredictionServerState();
    testClientPredictionApplyInput();
    testClientPredictionUpdate();
    testClientPredictionError();
    testClientPredictionReconciliation();
    testClientPredictionSnap();
    testClientPredictionInactive();
    testClientPredictionFrame();
    testClientPredictionMissing();
    testCloudDeploymentCreate();
    testCloudDeploymentProvider();
    testCloudDeploymentRegion();
    testCloudDeploymentValidate();
    testCloudDeploymentDeploy();
    testCloudDeploymentUptime();
    testCloudDeploymentHealthCheck();
    testCloudDeploymentMaxPlayers();
    testCloudDeploymentCost();
    testCloudDeploymentMissing();
    testInterestPriorityCreate();
    testInterestPrioritySetTier();
    testInterestPriorityDistanceAuto();
    testInterestPriorityUpdate();
    testInterestPriorityAccumulator();
    testInterestPriorityCritical();
    testInterestPriorityLow();
    testInterestPriorityBandwidth();
    testInterestPriorityInactive();
    testInterestPriorityMissing();
}
