/**
 * Tournament tests
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

// ==================== TournamentSystem Tests ====================

void testTournamentCreate() {
    std::cout << "\n=== Tournament Create ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    bool created = tourneySys.createTournament("tourney_1", "pvp_tourney_1", "Arena Championship", 8, 10000.0, 300.0f);
    assertTrue(created, "Tournament created");
    assertTrue(tourneySys.getStatus("tourney_1") == "registration", "Status is registration");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 0, "Zero participants initially");
}

void testTournamentRegister() {
    std::cout << "\n=== Tournament Register ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test Tournament", 4, 5000.0, 300.0f);

    assertTrue(tourneySys.registerPlayer("tourney_1", "player_1", "Alice"), "Player 1 registered");
    assertTrue(tourneySys.registerPlayer("tourney_1", "player_2", "Bob"), "Player 2 registered");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 2, "Two participants registered");
    assertTrue(approxEqual(static_cast<float>(tourneySys.getPrizePool("tourney_1")), 10000.0f), "Prize pool is 10K");
}

void testTournamentMaxParticipants() {
    std::cout << "\n=== Tournament Max Participants ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Small Tourney", 2, 1000.0, 300.0f);

    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    bool third = tourneySys.registerPlayer("tourney_1", "p3", "Charlie");
    assertTrue(!third, "Third player rejected (tournament full)");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 2, "Still 2 participants");
}

void testTournamentDuplicateRegister() {
    std::cout << "\n=== Tournament Duplicate Register ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);

    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    bool dup = tourneySys.registerPlayer("tourney_1", "p1", "Alice Again");
    assertTrue(!dup, "Duplicate registration rejected");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 1, "Still 1 participant");
}

void testTournamentStart() {
    std::cout << "\n=== Tournament Start ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");

    bool started = tourneySys.startTournament("tourney_1");
    assertTrue(started, "Tournament started");
    assertTrue(tourneySys.getStatus("tourney_1") == "active", "Status is active");
    assertTrue(tourneySys.getCurrentRound("tourney_1") == 1, "Round 1 started");
}

void testTournamentEmptyCannotStart() {
    std::cout << "\n=== Tournament Empty Cannot Start ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Empty", 8, 0.0, 300.0f);

    bool started = tourneySys.startTournament("tourney_1");
    assertTrue(!started, "Empty tournament cannot start");
    assertTrue(tourneySys.getStatus("tourney_1") == "registration", "Status stays registration");
}

void testTournamentScoring() {
    std::cout << "\n=== Tournament Scoring ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.startTournament("tourney_1");

    tourneySys.recordKill("tourney_1", "p1", 5);
    tourneySys.recordKill("tourney_1", "p2", 3);
    tourneySys.recordKill("tourney_1", "p1", 2);

    assertTrue(tourneySys.getPlayerScore("tourney_1", "p1") == 7, "Player 1 score is 7");
    assertTrue(tourneySys.getPlayerScore("tourney_1", "p2") == 3, "Player 2 score is 3");
}

void testTournamentElimination() {
    std::cout << "\n=== Tournament Elimination ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.registerPlayer("tourney_1", "p3", "Charlie");
    tourneySys.startTournament("tourney_1");

    assertTrue(tourneySys.getActiveParticipantCount("tourney_1") == 3, "3 active before elimination");
    tourneySys.eliminatePlayer("tourney_1", "p2");
    assertTrue(tourneySys.getActiveParticipantCount("tourney_1") == 2, "2 active after elimination");

    // Eliminated player cannot score
    bool scored = tourneySys.recordKill("tourney_1", "p2", 1);
    assertTrue(!scored, "Eliminated player cannot score");
}

void testTournamentRoundAdvance() {
    std::cout << "\n=== Tournament Round Advance ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 100.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.startTournament("tourney_1");

    tourneySys.recordKill("tourney_1", "p1", 5);
    assertTrue(tourneySys.getCurrentRound("tourney_1") == 1, "Still round 1 before update");

    // Advance past round 1
    tourneySys.update(101.0f);
    assertTrue(tourneySys.getCurrentRound("tourney_1") == 2, "Advanced to round 2");
}

void testTournamentCompletion() {
    std::cout << "\n=== Tournament Completion ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 1000.0, 50.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.startTournament("tourney_1");

    // Advance through all 3 rounds
    tourneySys.update(51.0f);  // end round 1 → start round 2
    tourneySys.update(51.0f);  // end round 2 → start round 3
    tourneySys.update(51.0f);  // end round 3 → completed

    assertTrue(tourneySys.getStatus("tourney_1") == "completed", "Tournament completed after 3 rounds");
}

void testTournamentRegisterAfterStart() {
    std::cout << "\n=== Tournament Register After Start ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.startTournament("tourney_1");

    bool late = tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    assertTrue(!late, "Cannot register after tournament starts");
}

// ==================== LeaderboardSystem Tests ====================

void testLeaderboardRecordKill() {
    std::cout << "\n=== Leaderboard Record Kill ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordKill("board_1", "p1", "Alice");
    lbSys.recordKill("board_1", "p1", "Alice");
    lbSys.recordKill("board_1", "p1", "Alice");

    assertTrue(lbSys.getPlayerKills("board_1", "p1") == 3, "Player has 3 kills");
    assertTrue(lbSys.getEntryCount("board_1") == 1, "One entry on board");
}

void testLeaderboardMultiplePlayers() {
    std::cout << "\n=== Leaderboard Multiple Players ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordKill("board_1", "p1", "Alice");
    lbSys.recordKill("board_1", "p2", "Bob");
    lbSys.recordKill("board_1", "p1", "Alice");

    assertTrue(lbSys.getEntryCount("board_1") == 2, "Two entries on board");
    assertTrue(lbSys.getPlayerKills("board_1", "p1") == 2, "Alice has 2 kills");
    assertTrue(lbSys.getPlayerKills("board_1", "p2") == 1, "Bob has 1 kill");
}

void testLeaderboardIskTracking() {
    std::cout << "\n=== Leaderboard Credits Tracking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordIskEarned("board_1", "p1", "Alice", 50000.0);
    lbSys.recordIskEarned("board_1", "p1", "Alice", 25000.0);

    assertTrue(approxEqual(static_cast<float>(lbSys.getPlayerIskEarned("board_1", "p1")), 75000.0f), "Credits earned is 75K");
}

void testLeaderboardMissionTracking() {
    std::cout << "\n=== Leaderboard Mission Tracking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordMissionComplete("board_1", "p1", "Alice");
    lbSys.recordMissionComplete("board_1", "p1", "Alice");

    assertTrue(lbSys.getPlayerMissions("board_1", "p1") == 2, "Player completed 2 missions");
}

void testLeaderboardRanking() {
    std::cout << "\n=== Leaderboard Ranking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordKill("board_1", "p1", "Alice");
    for (int i = 0; i < 5; ++i) lbSys.recordKill("board_1", "p2", "Bob");
    for (int i = 0; i < 3; ++i) lbSys.recordKill("board_1", "p3", "Charlie");

    auto ranking = lbSys.getRankingByKills("board_1");
    assertTrue(static_cast<int>(ranking.size()) == 3, "Ranking has 3 entries");
    assertTrue(ranking[0] == "p2", "Bob is rank 1 (5 kills)");
    assertTrue(ranking[1] == "p3", "Charlie is rank 2 (3 kills)");
    assertTrue(ranking[2] == "p1", "Alice is rank 3 (1 kill)");
}

void testLeaderboardAchievementDefine() {
    std::cout << "\n=== Leaderboard Achievement Define ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.defineAchievement("board_1", "first_blood", "First Blood", "Get your first kill", "combat", "total_kills", 1);
    lbSys.defineAchievement("board_1", "veteran", "Veteran", "Reach 100 kills", "combat", "total_kills", 100);

    auto* lb = board->getComponent<components::Leaderboard>();
    assertTrue(static_cast<int>(lb->achievements.size()) == 2, "Two achievements defined");
}

void testLeaderboardAchievementUnlock() {
    std::cout << "\n=== Leaderboard Achievement Unlock ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.defineAchievement("board_1", "first_blood", "First Blood", "Get your first kill", "combat", "total_kills", 1);
    lbSys.defineAchievement("board_1", "veteran", "Veteran", "Reach 100 kills", "combat", "total_kills", 100);

    lbSys.recordKill("board_1", "p1", "Alice");
    int unlocked = lbSys.checkAchievements("board_1", "p1", 1000.0f);

    assertTrue(unlocked == 1, "One achievement unlocked");
    assertTrue(lbSys.hasAchievement("board_1", "p1", "first_blood"), "First Blood unlocked");
    assertTrue(!lbSys.hasAchievement("board_1", "p1", "veteran"), "Veteran not unlocked yet");
}

void testLeaderboardAchievementNoDuplicate() {
    std::cout << "\n=== Leaderboard Achievement No Duplicate ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.defineAchievement("board_1", "first_blood", "First Blood", "Get first kill", "combat", "total_kills", 1);
    lbSys.recordKill("board_1", "p1", "Alice");

    lbSys.checkAchievements("board_1", "p1");
    int second = lbSys.checkAchievements("board_1", "p1");

    assertTrue(second == 0, "No duplicate unlock");
    assertTrue(lbSys.getPlayerAchievementCount("board_1", "p1") == 1, "Still 1 achievement total");
}

void testLeaderboardNonexistentPlayer() {
    std::cout << "\n=== Leaderboard Nonexistent Player ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    assertTrue(lbSys.getPlayerKills("board_1", "fake") == 0, "Zero kills for nonexistent");
    assertTrue(approxEqual(static_cast<float>(lbSys.getPlayerIskEarned("board_1", "fake")), 0.0f), "Zero Credits for nonexistent");
    assertTrue(lbSys.getPlayerMissions("board_1", "fake") == 0, "Zero missions for nonexistent");
}

void testLeaderboardDamageTracking() {
    std::cout << "\n=== Leaderboard Damage Tracking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordDamageDealt("board_1", "p1", "Alice", 5000.0);
    lbSys.recordDamageDealt("board_1", "p1", "Alice", 3000.0);

    auto* lb = board->getComponent<components::Leaderboard>();
    bool found = false;
    for (const auto& e : lb->entries) {
        if (e.player_id == "p1") {
            found = true;
            assertTrue(approxEqual(static_cast<float>(e.total_damage_dealt), 8000.0f), "Total damage is 8000");
        }
    }
    assertTrue(found, "Player entry found for damage tracking");
}


void run_tournament_tests() {
    testTournamentCreate();
    testTournamentRegister();
    testTournamentMaxParticipants();
    testTournamentDuplicateRegister();
    testTournamentStart();
    testTournamentEmptyCannotStart();
    testTournamentScoring();
    testTournamentElimination();
    testTournamentRoundAdvance();
    testTournamentCompletion();
    testTournamentRegisterAfterStart();
    testLeaderboardRecordKill();
    testLeaderboardMultiplePlayers();
    testLeaderboardIskTracking();
    testLeaderboardMissionTracking();
    testLeaderboardRanking();
    testLeaderboardAchievementDefine();
    testLeaderboardAchievementUnlock();
    testLeaderboardAchievementNoDuplicate();
    testLeaderboardNonexistentPlayer();
    testLeaderboardDamageTracking();
}
