/**
 * Station tests
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

// ==================== StationSystem Tests ====================

void testStationCreate() {
    std::cout << "\n=== Station Create ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    bool ok = stationSys.createStation("station_1", "Test Hub", 100.0f, 0.0f, 200.0f, 3000.0f, 2.0f);
    assertTrue(ok, "Station created successfully");

    auto* entity = world.getEntity("station_1");
    assertTrue(entity != nullptr, "Station entity exists");

    auto* station = entity->getComponent<components::Station>();
    assertTrue(station != nullptr, "Station component attached");
    assertTrue(station->station_name == "Test Hub", "Station name is correct");
    assertTrue(approxEqual(station->docking_range, 3000.0f), "Docking range is correct");
    assertTrue(approxEqual(station->repair_cost_per_hp, 2.0f), "Repair cost per HP is correct");
}

void testStationDuplicateCreate() {
    std::cout << "\n=== Station Duplicate Create ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub A", 0, 0, 0);
    bool dup = stationSys.createStation("station_1", "Hub B", 0, 0, 0);
    assertTrue(!dup, "Duplicate station creation rejected");
}

void testStationDockInRange() {
    std::cout << "\n=== Station Dock In Range ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 100.0f;
    addComp<components::Velocity>(ship);
    addComp<components::Player>(ship);

    bool ok = stationSys.dockAtStation("player_1", "station_1");
    assertTrue(ok, "Docking succeeds when in range");
    assertTrue(stationSys.isDocked("player_1"), "Player is docked");
    assertTrue(stationSys.getDockedStation("player_1") == "station_1", "Docked at correct station");
}

void testStationDockOutOfRange() {
    std::cout << "\n=== Station Dock Out Of Range ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 500.0f);

    auto* ship = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 9999.0f;
    addComp<components::Velocity>(ship);

    bool ok = stationSys.dockAtStation("player_1", "station_1");
    assertTrue(!ok, "Docking fails when out of range");
    assertTrue(!stationSys.isDocked("player_1"), "Player is not docked");
}

void testStationUndock() {
    std::cout << "\n=== Station Undock ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    stationSys.dockAtStation("player_1", "station_1");
    assertTrue(stationSys.isDocked("player_1"), "Docked before undock");

    bool ok = stationSys.undockFromStation("player_1");
    assertTrue(ok, "Undock succeeds");
    assertTrue(!stationSys.isDocked("player_1"), "No longer docked after undock");
}

void testStationUndockNotDocked() {
    std::cout << "\n=== Station Undock Not Docked ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);

    bool ok = stationSys.undockFromStation("player_1");
    assertTrue(!ok, "Undock fails when not docked");
}

void testStationRepair() {
    std::cout << "\n=== Station Repair ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f, 1.0f);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    auto* hp = addComp<components::Health>(ship);
    hp->shield_hp = 50.0f;  hp->shield_max = 100.0f;
    hp->armor_hp  = 30.0f;  hp->armor_max  = 100.0f;
    hp->hull_hp   = 80.0f;  hp->hull_max   = 100.0f;

    auto* player = addComp<components::Player>(ship);
    player->credits = 10000.0;

    stationSys.dockAtStation("player_1", "station_1");

    double cost = stationSys.repairShip("player_1");
    // Damage = (100-50) + (100-30) + (100-80) = 50+70+20 = 140 HP, at 1 Credits/hp = 140
    assertTrue(approxEqual(static_cast<float>(cost), 140.0f), "Repair cost is 140 Credits");
    assertTrue(approxEqual(hp->shield_hp, 100.0f), "Shield fully repaired");
    assertTrue(approxEqual(hp->armor_hp, 100.0f), "Armor fully repaired");
    assertTrue(approxEqual(hp->hull_hp, 100.0f), "Hull fully repaired");
    assertTrue(approxEqual(static_cast<float>(player->credits), 9860.0f), "Credits deducted");
}

void testStationRepairNoDamage() {
    std::cout << "\n=== Station Repair No Damage ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    auto* hp = addComp<components::Health>(ship);
    hp->shield_hp = hp->shield_max = 100.0f;
    hp->armor_hp  = hp->armor_max  = 100.0f;
    hp->hull_hp   = hp->hull_max   = 100.0f;

    addComp<components::Player>(ship);

    stationSys.dockAtStation("player_1", "station_1");

    double cost = stationSys.repairShip("player_1");
    assertTrue(approxEqual(static_cast<float>(cost), 0.0f), "No cost when no damage");
}

void testStationRepairNotDocked() {
    std::cout << "\n=== Station Repair Not Docked ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    auto* hp = addComp<components::Health>(ship);
    hp->shield_hp = 50.0f; hp->shield_max = 100.0f;

    double cost = stationSys.repairShip("player_1");
    assertTrue(approxEqual(static_cast<float>(cost), 0.0f), "No repair when not docked");
}

void testStationDockedCount() {
    std::cout << "\n=== Station Docked Count ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* s1 = world.createEntity("p1");
    addComp<components::Position>(s1);
    addComp<components::Velocity>(s1);

    auto* s2 = world.createEntity("p2");
    addComp<components::Position>(s2);
    addComp<components::Velocity>(s2);

    stationSys.dockAtStation("p1", "station_1");
    stationSys.dockAtStation("p2", "station_1");

    auto* station = world.getEntity("station_1")->getComponent<components::Station>();
    assertTrue(station->docked_count == 2, "Two ships docked");

    stationSys.undockFromStation("p1");
    assertTrue(station->docked_count == 1, "One ship after undock");
}

void testStationDoubleDock() {
    std::cout << "\n=== Station Double Dock ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("p1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    stationSys.dockAtStation("p1", "station_1");
    bool again = stationSys.dockAtStation("p1", "station_1");
    assertTrue(!again, "Cannot dock when already docked");
}

void testStationMovementStopsOnDock() {
    std::cout << "\n=== Station Movement Stops On Dock ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("p1");
    addComp<components::Position>(ship);
    auto* vel = addComp<components::Velocity>(ship);
    vel->vx = 100.0f;
    vel->vy = 50.0f;
    vel->vz = 200.0f;

    stationSys.dockAtStation("p1", "station_1");
    assertTrue(approxEqual(vel->vx, 0.0f), "Velocity X zeroed on dock");
    assertTrue(approxEqual(vel->vy, 0.0f), "Velocity Y zeroed on dock");
    assertTrue(approxEqual(vel->vz, 0.0f), "Velocity Z zeroed on dock");
}

// ==================== Phase 11: Station Deployment System Tests ====================

void testStationDeploymentBegin() {
    std::cout << "\n=== Station Deployment Begin ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    addComp<components::StationDeployment>(ship);
    bool ok = sys.beginDeployment("deployer1", "system_a", 100.0f, 200.0f, 300.0f);
    assertTrue(ok, "Deployment started successfully");
    assertTrue(sys.isDeploying("deployer1"), "Ship is deploying");
    assertTrue(!sys.isDeployed("deployer1"), "Ship is not yet deployed");
}

void testStationDeploymentComplete() {
    std::cout << "\n=== Station Deployment Complete ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    auto* dep = addComp<components::StationDeployment>(ship);
    dep->deploy_duration = 10.0f; // short for testing
    sys.beginDeployment("deployer1", "system_a", 100.0f, 200.0f, 300.0f);
    sys.update(5.0f);
    assertTrue(sys.isDeploying("deployer1"), "Still deploying at 5s");
    sys.update(6.0f);
    assertTrue(sys.isDeployed("deployer1"), "Deployed after 11s total");
}

void testStationDeploymentCancel() {
    std::cout << "\n=== Station Deployment Cancel ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    addComp<components::StationDeployment>(ship);
    sys.beginDeployment("deployer1", "system_a", 0, 0, 0);
    sys.cancelDeployment("deployer1");
    assertTrue(!sys.isDeploying("deployer1"), "Not deploying after cancel");
    assertTrue(!sys.isDeployed("deployer1"), "Not deployed after cancel");
}

void testStationDeploymentAttachModule() {
    std::cout << "\n=== Station Deployment Attach Module ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    auto* dep = addComp<components::StationDeployment>(ship);
    dep->deploy_state = components::StationDeployment::DeployState::Deployed;
    bool ok = sys.attachModule("deployer1", "security");
    assertTrue(ok, "Module attached successfully");
    assertTrue(sys.getAttachedModuleCount("deployer1") == 1, "1 module attached");
}

void testStationDeploymentModuleLimit() {
    std::cout << "\n=== Station Deployment Module Limit ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    auto* dep = addComp<components::StationDeployment>(ship);
    dep->deploy_state = components::StationDeployment::DeployState::Deployed;
    dep->max_module_slots = 2;
    sys.attachModule("deployer1", "security");
    sys.attachModule("deployer1", "market");
    bool ok = sys.attachModule("deployer1", "refinery");
    assertTrue(!ok, "Cannot attach beyond max module slots");
    assertTrue(sys.getAttachedModuleCount("deployer1") == 2, "Still 2 modules");
}

void testStationDeploymentSystemBonuses() {
    std::cout << "\n=== Station Deployment System Bonuses ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    auto* dep = addComp<components::StationDeployment>(ship);
    dep->deploy_state = components::StationDeployment::DeployState::Deployed;
    sys.attachModule("deployer1", "security");
    sys.attachModule("deployer1", "market");
    float sec = 0.0f, econ = 0.0f, res = 0.0f;
    sys.getSystemBonuses("deployer1", sec, econ, res);
    assertTrue(approxEqual(sec, 0.05f), "Security bonus from security module");
    assertTrue(approxEqual(econ, 0.10f), "Economy bonus from market module");
    assertTrue(approxEqual(res, 0.0f), "No resource bonus without refinery");
}

void testStationDeploymentNotDeployedCantAttach() {
    std::cout << "\n=== Station Deployment Not Deployed Can't Attach ===" << std::endl;
    ecs::World world;
    systems::StationDeploymentSystem sys(&world);
    auto* ship = world.createEntity("deployer1");
    addComp<components::StationDeployment>(ship);
    bool ok = sys.attachModule("deployer1", "security");
    assertTrue(!ok, "Cannot attach module to non-deployed station");
}

// ==================== Station News System Tests ====================

void testStationNewsEmpty() {
    std::cout << "\n=== Station News Empty ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("station1");
    addComp<components::StationNewsFeed>(e);

    systems::StationNewsSystem sys(&world);
    assertTrue(sys.getNewsCount("station1") == 0, "No news initially");
    auto news = sys.getNews("station1");
    assertTrue(news.empty(), "Empty news list");
}

void testStationNewsAddCombat() {
    std::cout << "\n=== Station News Add Combat ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("station2");
    addComp<components::StationNewsFeed>(e);

    systems::StationNewsSystem sys(&world);
    sys.reportCombatEvent("station2", "Pirates defeated", 100.0f);
    assertTrue(sys.getNewsCount("station2") == 1, "One news entry");
    auto news = sys.getNews("station2");
    assertTrue(news[0].category == "combat", "Category is combat");
}

void testStationNewsMaxEntries() {
    std::cout << "\n=== Station News Max Entries ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("station3");
    auto* feed = addComp<components::StationNewsFeed>(e);
    feed->max_entries = 5;

    systems::StationNewsSystem sys(&world);
    for (int i = 0; i < 10; i++) {
        sys.reportCombatEvent("station3", "Event " + std::to_string(i), static_cast<float>(i));
    }
    assertTrue(sys.getNewsCount("station3") == 5, "Entries trimmed to max");
}

// ==================== Docking System Tests ====================

void testDockingPortDefaults() {
    std::cout << "\n=== Docking Port Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("port1");
    auto* port = addComp<components::DockingPort>(e);
    assertTrue(!port->isOccupied(), "Not occupied by default");
}

void testDockingDock() {
    std::cout << "\n=== Docking Dock ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("port2");
    addComp<components::DockingPort>(e);

    systems::DockingSystem sys(&world);
    assertTrue(sys.dock("port2", "ship_1"), "Dock succeeds");
    assertTrue(sys.isOccupied("port2"), "Port is occupied");
    assertTrue(sys.getDockedEntity("port2") == "ship_1", "Correct docked entity");
}

void testDockingUndock() {
    std::cout << "\n=== Docking Undock ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("port3");
    addComp<components::DockingPort>(e);

    systems::DockingSystem sys(&world);
    sys.dock("port3", "ship_2");
    std::string undocked = sys.undock("port3");
    assertTrue(undocked == "ship_2", "Undocked correct entity");
    assertTrue(!sys.isOccupied("port3"), "Port is empty after undock");
}

// ==================== Rest Station System Tests ====================

void testRestStationDefaults() {
    std::cout << "\n=== Rest Station Defaults ===" << std::endl;
    ecs::World world;
    auto* station = world.createEntity("bed1");
    auto* rs = addComp<components::RestStation>(station);
    
    assertTrue(rs->isAvailable(), "New station is available");
    assertTrue(approxEqual(rs->quality, 1.0f), "Default quality is 1.0");
    assertTrue(rs->type == components::RestStation::StationType::Bed, "Default type is Bed");
}

void testRestStationStartRest() {
    std::cout << "\n=== Rest Station Start Rest ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char1");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 80.0f;
    
    auto* station = world.createEntity("bed2");
    addComp<components::RestStation>(station);
    
    bool started = restSys.startResting("char1", "bed2");
    assertTrue(started, "Rest started successfully");
    assertTrue(restSys.isResting("char1"), "Character is resting");
    assertTrue(!restSys.isStationAvailable("bed2"), "Station is now occupied");
}

void testRestStationRecovery() {
    std::cout << "\n=== Rest Station Recovery ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char2");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 50.0f;
    
    auto* station = world.createEntity("bed3");
    auto* rs = addComp<components::RestStation>(station);
    rs->quality = 2.0f;  // Luxury
    
    restSys.startResting("char2", "bed3");
    restSys.update(10.0f);  // 10 seconds of rest
    
    // Recovery = base_rate * quality * time = 1.0 * 2.0 * 10 = 20
    assertTrue(approxEqual(needs->fatigue, 30.0f), "Fatigue reduced by 20 (50 - 20 = 30)");
}

void testRestStationStopRest() {
    std::cout << "\n=== Rest Station Stop Rest ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char3");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 40.0f;
    
    auto* station = world.createEntity("bed4");
    addComp<components::RestStation>(station);
    
    restSys.startResting("char3", "bed4");
    restSys.update(5.0f);
    
    float finalFatigue = restSys.stopResting("char3");
    assertTrue(finalFatigue < 40.0f, "Fatigue reduced after rest");
    assertTrue(!restSys.isResting("char3"), "Character no longer resting");
    assertTrue(restSys.isStationAvailable("bed4"), "Station available again");
}

void testRestStationProgress() {
    std::cout << "\n=== Rest Station Progress ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char4");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 100.0f;
    
    auto* station = world.createEntity("bed5");
    addComp<components::RestStation>(station);
    
    restSys.startResting("char4", "bed5");
    restSys.update(50.0f);  // Should recover 50 fatigue points
    
    float progress = restSys.getRestProgress("char4");
    assertTrue(approxEqual(progress, 0.5f), "50% rest progress (50/100)");
}

void testRestStationQualityName() {
    std::cout << "\n=== Rest Station Quality Name ===" << std::endl;
    assertTrue(systems::RestStationSystem::getQualityName(2.0f) == "Luxury", "Quality 2.0 is Luxury");
    assertTrue(systems::RestStationSystem::getQualityName(1.5f) == "Premium", "Quality 1.5 is Premium");
    assertTrue(systems::RestStationSystem::getQualityName(1.0f) == "Standard", "Quality 1.0 is Standard");
    assertTrue(systems::RestStationSystem::getQualityName(0.5f) == "Basic", "Quality 0.5 is Basic");
    assertTrue(systems::RestStationSystem::getQualityName(0.3f) == "Poor", "Quality 0.3 is Poor");
}

void testRestStationAutoStop() {
    std::cout << "\n=== Rest Station Auto Stop ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* character = world.createEntity("char5");
    auto* needs = addComp<components::SurvivalNeeds>(character);
    needs->fatigue = 10.0f;
    
    auto* station = world.createEntity("bed6");
    addComp<components::RestStation>(station);
    
    restSys.startResting("char5", "bed6");
    restSys.update(15.0f);  // Should fully recover and auto-stop
    
    assertTrue(!restSys.isResting("char5"), "Character auto-stopped resting when fully rested");
    assertTrue(needs->fatigue <= 0.0f, "Fatigue at 0 when fully rested");
}

void testRestStationCount() {
    std::cout << "\n=== Rest Station Count ===" << std::endl;
    ecs::World world;
    systems::RestStationSystem restSys(&world);
    
    auto* char1 = world.createEntity("c1");
    addComp<components::SurvivalNeeds>(char1)->fatigue = 50.0f;
    auto* char2 = world.createEntity("c2");
    addComp<components::SurvivalNeeds>(char2)->fatigue = 50.0f;
    
    auto* bed1 = world.createEntity("b1");
    addComp<components::RestStation>(bed1);
    auto* bed2 = world.createEntity("b2");
    addComp<components::RestStation>(bed2);
    
    assertTrue(restSys.getRestingCount() == 0, "No one resting initially");
    
    restSys.startResting("c1", "b1");
    assertTrue(restSys.getRestingCount() == 1, "1 person resting");
    
    restSys.startResting("c2", "b2");
    assertTrue(restSys.getRestingCount() == 2, "2 people resting");
}

// ==================== Station Hangar System Tests ====================

void testHangarCreate() {
    std::cout << "\n=== Hangar Create ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);

    // Create a station first.
    auto* station = world.createEntity("station1");
    auto stationComp = std::make_unique<components::Station>();
    stationComp->station_name = "Test Station";
    station->addComponent(std::move(stationComp));

    assertTrue(sys.createHangar("hangar1", "station1", "player1",
                                components::StationHangar::HangarType::Personal),
               "Hangar created");
    assertTrue(!sys.createHangar("hangar1", "station1", "player1"),
               "Duplicate hangar fails");
}

void testHangarStoreShip() {
    std::cout << "\n=== Hangar Store Ship ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1");

    assertTrue(sys.storeShip("hangar1", "frigate_1"), "Store ship succeeds");
    assertTrue(sys.getStoredShipCount("hangar1") == 1, "One ship stored");
    assertTrue(!sys.storeShip("hangar1", "frigate_2"), "Hangar full at basic level");
}

void testHangarRetrieveShip() {
    std::cout << "\n=== Hangar Retrieve Ship ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1");
    sys.storeShip("hangar1", "frigate_1");

    assertTrue(sys.retrieveShip("hangar1", "frigate_1"), "Retrieve succeeds");
    assertTrue(sys.getStoredShipCount("hangar1") == 0, "Hangar empty after retrieve");
    assertTrue(!sys.retrieveShip("hangar1", "frigate_1"), "Double retrieve fails");
}

void testHangarUpgrade() {
    std::cout << "\n=== Hangar Upgrade ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1",
                     components::StationHangar::HangarType::Corporation);

    double cost = sys.upgradeHangar("hangar1");
    assertTrue(approxEqual(static_cast<float>(cost), 10000.0f), "Upgrade to Standard costs 10000");

    // After upgrade we should have 2 slots.
    assertTrue(sys.storeShip("hangar1", "ship_a"), "Slot 1");
    assertTrue(sys.storeShip("hangar1", "ship_b"), "Slot 2");
    assertTrue(!sys.storeShip("hangar1", "ship_c"), "Full at standard (2 slots)");
}

void testHangarMaxUpgrade() {
    std::cout << "\n=== Hangar Max Upgrade ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1",
                     components::StationHangar::HangarType::Corporation);

    sys.upgradeHangar("hangar1"); // Basic -> Standard
    sys.upgradeHangar("hangar1"); // Standard -> Advanced
    sys.upgradeHangar("hangar1"); // Advanced -> Premium
    double cost = sys.upgradeHangar("hangar1"); // Already max
    assertTrue(approxEqual(static_cast<float>(cost), 0.0f), "No upgrade past Premium");
}

void testHangarRentalAccrual() {
    std::cout << "\n=== Hangar Rental Accrual ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1",
                     components::StationHangar::HangarType::Leased);

    // Simulate 1 day (86400 seconds).
    sys.update(86400.0f);

    double balance = sys.getRentalBalance("hangar1");
    assertTrue(balance > 4999.0 && balance < 5001.0, "One day rental accrued ~5000 credits");
}

void testHangarShouldUseHangarFrigate() {
    std::cout << "\n=== Should Use Hangar (Frigate) ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);

    auto* ship = world.createEntity("frig1");
    auto shipComp = std::make_unique<components::Ship>();
    shipComp->ship_class = "Frigate";
    ship->addComponent(std::move(shipComp));

    assertTrue(sys.shouldUseHangar("frig1"), "Frigate uses hangar");
}

void testHangarShouldUseTetherCapital() {
    std::cout << "\n=== Should Use Tether (Capital) ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);

    auto* ship = world.createEntity("cap1");
    auto shipComp = std::make_unique<components::Ship>();
    shipComp->ship_class = "Capital";
    ship->addComponent(std::move(shipComp));

    assertTrue(!sys.shouldUseHangar("cap1"), "Capital uses tether (not hangar)");
}

void testHangarShouldUseTetherTitan() {
    std::cout << "\n=== Should Use Tether (Titan) ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);

    auto* ship = world.createEntity("titan1");
    auto shipComp = std::make_unique<components::Ship>();
    shipComp->ship_class = "Titan";
    ship->addComponent(std::move(shipComp));

    assertTrue(!sys.shouldUseHangar("titan1"), "Titan uses tether (not hangar)");
}

void testHangarSpawnPosition() {
    std::cout << "\n=== Hangar Spawn Position ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    sys.createHangar("hangar1", "station1", "player1");

    auto [x, y, z] = sys.getSpawnPosition("hangar1");
    assertTrue(approxEqual(x, 5.0f), "Spawn X is 5");
    assertTrue(approxEqual(y, 0.0f), "Spawn Y is 0");
    assertTrue(approxEqual(z, 2.0f), "Spawn Z is 2");
}

void testHangarDuplicateShip() {
    std::cout << "\n=== Hangar Duplicate Ship ===" << std::endl;
    ecs::World world;
    systems::StationHangarSystem sys(&world);
    // Upgrade to 2 slots.
    sys.createHangar("hangar1", "station1", "player1");
    sys.upgradeHangar("hangar1");

    assertTrue(sys.storeShip("hangar1", "ship1"), "First store succeeds");
    assertTrue(!sys.storeShip("hangar1", "ship1"), "Duplicate ship blocked");
}

void testHangarComponentDefaults() {
    std::cout << "\n=== Hangar Component Defaults ===" << std::endl;
    components::StationHangar h;
    assertTrue(h.type == components::StationHangar::HangarType::Leased, "Default type Leased");
    assertTrue(h.upgrade_level == components::StationHangar::UpgradeLevel::Basic, "Default Basic level");
    assertTrue(h.max_ship_slots == 1, "Default 1 slot");
    assertTrue(h.hasRoom(), "Has room at 0/1");
    assertTrue(h.isLeased(), "Is leased");
}

// ==================== Tether Docking System Tests ====================

void testTetherArmCreate() {
    std::cout << "\n=== Tether Arm Create ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);

    assertTrue(sys.createArm("arm1", "station1"), "Arm created");
    assertTrue(!sys.createArm("arm1", "station1"), "Duplicate arm fails");
    assertTrue(!sys.isOccupied("arm1"), "Arm empty");
}

void testTetherBeginTether() {
    std::cout << "\n=== Tether Begin ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");

    assertTrue(sys.beginTether("arm1", "capital_ship"), "Tether started");
    assertTrue(sys.isOccupied("arm1"), "Arm occupied");
    assertTrue(sys.getTetheredShip("arm1") == "capital_ship", "Correct ship tethered");
    assertTrue(sys.getArmState("arm1") ==
               components::TetherDockingArm::ArmState::Extending, "Arm extending");
}

void testTetherExtendToLock() {
    std::cout << "\n=== Tether Extend to Lock ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "capital_ship");

    // Run enough time for the arm to fully extend (default 0.5/s → 2 seconds).
    sys.update(3.0f);

    assertTrue(sys.getArmState("arm1") ==
               components::TetherDockingArm::ArmState::Locked, "Arm locked after extending");
    assertTrue(sys.isCrewTransferEnabled("arm1"), "Crew transfer enabled when locked");
    assertTrue(approxEqual(sys.getExtendProgress("arm1"), 1.0f), "Fully extended");
}

void testTetherUndockRetract() {
    std::cout << "\n=== Tether Undock Retract ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "capital_ship");
    sys.update(3.0f); // Lock

    assertTrue(sys.beginUndock("arm1"), "Undock started");
    assertTrue(!sys.isCrewTransferEnabled("arm1"), "Crew transfer disabled during retract");
    assertTrue(sys.getArmState("arm1") ==
               components::TetherDockingArm::ArmState::Retracting, "Arm retracting");

    sys.update(3.0f); // Retract

    assertTrue(sys.getArmState("arm1") ==
               components::TetherDockingArm::ArmState::Retracted, "Arm retracted");
    assertTrue(!sys.isOccupied("arm1"), "Arm empty after retraction");
    assertTrue(sys.getTetheredShip("arm1").empty(), "No ship tethered");
}

void testTetherDoubleOccupy() {
    std::cout << "\n=== Tether Double Occupy ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "ship_a");

    assertTrue(!sys.beginTether("arm1", "ship_b"), "Can't tether second ship");
}

void testTetherUndockOnlyWhenLocked() {
    std::cout << "\n=== Tether Undock Only When Locked ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "capital_ship");

    // Still extending, not locked yet.
    sys.update(0.5f);
    assertTrue(!sys.beginUndock("arm1"), "Can't undock while extending");
}

void testTetherPartialExtend() {
    std::cout << "\n=== Tether Partial Extend ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "capital_ship");

    sys.update(1.0f); // 0.5 progress/s * 1.0s = 0.5 progress
    float progress = sys.getExtendProgress("arm1");
    assertTrue(progress > 0.4f && progress < 0.6f, "Partially extended at ~0.5");
    assertTrue(!sys.isCrewTransferEnabled("arm1"), "No crew transfer while extending");
}

void testTetherComponentDefaults() {
    std::cout << "\n=== Tether Component Defaults ===" << std::endl;
    components::TetherDockingArm arm;
    assertTrue(arm.state == components::TetherDockingArm::ArmState::Retracted, "Default retracted");
    assertTrue(!arm.isOccupied(), "Default not occupied");
    assertTrue(arm.isFullyRetracted(), "Default fully retracted");
    assertTrue(!arm.isFullyExtended(), "Default not extended");
    assertTrue(arm.station_shield_active, "Shield active by default");
    assertTrue(!arm.crew_transfer_enabled, "Crew transfer disabled by default");
}

// ==================== Docking Ring Extension System Tests ====================

void testDockingRingInit() {
    std::cout << "\n=== Docking Ring: Init ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    auto* entity = world.createEntity("test_ship");
    assertTrue(sys.initializeRing("test_ship", 12.0f), "Ring initialized");
    auto* ring = entity->getComponent<components::DockingRingExtension>();
    assertTrue(ring != nullptr, "Ring component exists");
    assertTrue(approxEqual(ring->ring_diameter, 12.0f), "Ring diameter set");
    assertTrue(ring->state == components::DockingRingExtension::RingState::Retracted, "Initial state retracted");
    assertTrue(!sys.initializeRing("test_ship", 10.0f), "Duplicate init fails");
}

void testDockingRingExtend() {
    std::cout << "\n=== Docking Ring: Extend ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    assertTrue(sys.extendRing("test_ship"), "Extend starts");
    assertTrue(sys.getState("test_ship") == "extending", "State is extending");
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getProgress("test_ship"), 0.5f), "Progress 0.5 after 1s");
    sys.update(1.0f);
    assertTrue(sys.getState("test_ship") == "extended", "State extended after 2s");
    assertTrue(approxEqual(sys.getProgress("test_ship"), 1.0f), "Progress 1.0 when extended");
}

void testDockingRingRetract() {
    std::cout << "\n=== Docking Ring: Retract ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.extendRing("test_ship");
    sys.update(2.0f);
    assertTrue(sys.retractRing("test_ship"), "Retract starts");
    assertTrue(sys.getState("test_ship") == "retracting", "State is retracting");
    sys.update(2.0f);
    assertTrue(sys.getState("test_ship") == "retracted", "State retracted after 2s");
}

void testDockingRingConnect() {
    std::cout << "\n=== Docking Ring: Connect ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.extendRing("test_ship");
    sys.update(2.0f);
    sys.setAlignment("test_ship", 3.0f);
    assertTrue(sys.connectRing("test_ship", "target_ship", components::DockingRingExtension::ConnectionType::ShipToShip), "Connect succeeds");
    assertTrue(sys.isConnected("test_ship"), "Is connected");
}

void testDockingRingDisconnect() {
    std::cout << "\n=== Docking Ring: Disconnect ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.extendRing("test_ship");
    sys.update(2.0f);
    sys.setAlignment("test_ship", 2.0f);
    sys.connectRing("test_ship", "target", components::DockingRingExtension::ConnectionType::ShipToShip);
    assertTrue(sys.disconnectRing("test_ship"), "Disconnect succeeds");
    assertTrue(!sys.isConnected("test_ship"), "No longer connected");
    assertTrue(!sys.disconnectRing("test_ship"), "Double disconnect fails");
}

void testDockingRingAlignment() {
    std::cout << "\n=== Docking Ring: Alignment ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.extendRing("test_ship");
    sys.update(2.0f);
    sys.setAlignment("test_ship", 10.0f);
    assertTrue(!sys.connectRing("test_ship", "target", components::DockingRingExtension::ConnectionType::ShipToShip), "Connect fails with bad alignment");
    sys.setAlignment("test_ship", 4.0f);
    assertTrue(sys.connectRing("test_ship", "target", components::DockingRingExtension::ConnectionType::ShipToShip), "Connect succeeds with good alignment");
}

void testDockingRingPressure() {
    std::cout << "\n=== Docking Ring: Pressure ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    assertTrue(!sys.sealPressure("test_ship"), "Cannot seal when retracted");
    sys.extendRing("test_ship");
    sys.update(2.0f);
    assertTrue(sys.sealPressure("test_ship"), "Seal when extended");
    assertTrue(!sys.sealPressure("test_ship"), "Cannot double seal");
    assertTrue(sys.unsealPressure("test_ship"), "Unseal succeeds");
}

void testDockingRingPower() {
    std::cout << "\n=== Docking Ring: Power ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    sys.setPowerEnabled("test_ship", false);
    assertTrue(!sys.extendRing("test_ship"), "Cannot extend without power");
    sys.setPowerEnabled("test_ship", true);
    assertTrue(sys.extendRing("test_ship"), "Can extend with power");
}

void testDockingRingIntegrity() {
    std::cout << "\n=== Docking Ring: Integrity ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeRing("test_ship", 10.0f);
    assertTrue(approxEqual(sys.getIntegrity("test_ship"), 1.0f), "Initial integrity 1.0");
    sys.extendRing("test_ship");
    sys.update(2.0f);
    sys.setAlignment("test_ship", 0.0f);
    sys.connectRing("test_ship", "target", components::DockingRingExtension::ConnectionType::ShipToShip);
    sys.update(10.0f);
    assertTrue(sys.getIntegrity("test_ship") < 1.0f, "Integrity degrades when connected");
    sys.repairRing("test_ship", 1.0f);
    assertTrue(approxEqual(sys.getIntegrity("test_ship"), 1.0f), "Repair restores integrity");
}

void testDockingRingMissing() {
    std::cout << "\n=== Docking Ring: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::DockingRingExtensionSystem sys(&world);
    assertTrue(!sys.initializeRing("nonexistent", 10.0f), "Init fails on missing");
    assertTrue(!sys.extendRing("nonexistent"), "Extend fails on missing");
    assertTrue(sys.getState("nonexistent") == "unknown", "State unknown on missing");
    assertTrue(approxEqual(sys.getProgress("nonexistent"), 0.0f), "Progress 0 on missing");
}

// ==================== DockNodeLayout Tests ====================

void testDockNodeInit() {
    std::cout << "\n=== DockNodeLayout: Init ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    assertTrue(sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f), "Layout initialized");
    assertTrue(sys.getWindowCount("layout_1") == 0, "No windows initially");
    assertTrue(sys.getNodeType("layout_1", "root") == "root" || sys.getNodeType("layout_1", "root") == "leaf", "Root node exists");
    assertTrue(!sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f), "Duplicate init fails");
}

void testDockNodeAddWindow() {
    std::cout << "\n=== DockNodeLayout: Add Window ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    assertTrue(sys.addWindow("layout_1", "win_1"), "Window added");
    assertTrue(sys.getWindowCount("layout_1") == 1, "Window count is 1");
    assertTrue(sys.addWindow("layout_1", "win_2"), "Second window added");
    assertTrue(sys.getWindowCount("layout_1") == 2, "Window count is 2");
}

void testDockNodeRemoveWindow() {
    std::cout << "\n=== DockNodeLayout: Remove Window ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    assertTrue(sys.getWindowCount("layout_1") == 1, "1 window before remove");
    assertTrue(sys.removeWindow("layout_1", "win_1"), "Window removed");
    assertTrue(sys.getWindowCount("layout_1") == 0, "0 windows after remove");
    assertTrue(!sys.removeWindow("layout_1", "win_1"), "Double remove fails");
}

void testDockNodeSplit() {
    std::cout << "\n=== DockNodeLayout: Split ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    // Find the node with win_1 - it should be root which became a leaf
    assertTrue(sys.splitNode("layout_1", "root",
        components::DockNodeLayout::SplitDirection::Horizontal, 0.5f), "Node split");
    assertTrue(sys.getNodeType("layout_1", "root") == "split", "Root is now split");
}

void testDockNodeDock() {
    std::cout << "\n=== DockNodeLayout: Dock ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    // Find leaf node with win_1
    // Dock win_2 next to root
    assertTrue(sys.dockWindow("layout_1", "win_2", "root",
        components::DockNodeLayout::SplitDirection::Horizontal), "Window docked");
    assertTrue(sys.getWindowCount("layout_1") == 2, "2 windows after dock");
}

void testDockNodeUndock() {
    std::cout << "\n=== DockNodeLayout: Undock ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    assertTrue(sys.undockWindow("layout_1", "win_1"), "Window undocked");
    assertTrue(sys.getWindowCount("layout_1") == 0, "0 windows after undock");
    assertTrue(!sys.undockWindow("layout_1", "win_1"), "Double undock fails");
}

void testDockNodeMaxWindows() {
    std::cout << "\n=== DockNodeLayout: Max Windows ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    for (int i = 0; i < 20; i++) {
        sys.addWindow("layout_1", "win_" + std::to_string(i));
    }
    assertTrue(sys.getWindowCount("layout_1") == 20, "20 windows added");
    assertTrue(!sys.addWindow("layout_1", "win_overflow"), "Cannot exceed max windows");
}

void testDockNodeLayout() {
    std::cout << "\n=== DockNodeLayout: Layout Calculation ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1000.0f, 500.0f);
    sys.addWindow("layout_1", "win_1");
    sys.dockWindow("layout_1", "win_2", "root",
        components::DockNodeLayout::SplitDirection::Horizontal);
    sys.update(0.016f);
    auto bounds1 = sys.getWindowBounds("layout_1", "win_1");
    auto bounds2 = sys.getWindowBounds("layout_1", "win_2");
    assertTrue(std::get<2>(bounds1) > 0.0f, "Win1 has width");
    assertTrue(std::get<2>(bounds2) > 0.0f, "Win2 has width");
    assertTrue(approxEqual(std::get<2>(bounds1) + std::get<2>(bounds2), 1000.0f), "Widths sum to total");
}

void testDockNodeNestedSplit() {
    std::cout << "\n=== DockNodeLayout: Nested Split ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    world.createEntity("layout_1");
    sys.initializeLayout("layout_1", "owner_1", 1920.0f, 1080.0f);
    sys.addWindow("layout_1", "win_1");
    sys.splitNode("layout_1", "root",
        components::DockNodeLayout::SplitDirection::Horizontal, 0.5f);
    // The left child should have win_1, right is empty leaf
    assertTrue(sys.getNodeType("layout_1", "root") == "split", "Root is split");
    // Add window to the right child
    sys.addWindow("layout_1", "win_2");
    assertTrue(sys.getWindowCount("layout_1") == 2, "2 windows after nested ops");
}

void testDockNodeMissing() {
    std::cout << "\n=== DockNodeLayout: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::DockNodeLayoutSystem sys(&world);
    assertTrue(!sys.initializeLayout("nonexistent", "o", 100.0f, 100.0f), "Init fails on missing");
    assertTrue(!sys.addWindow("nonexistent", "win"), "Add fails on missing");
    assertTrue(sys.getWindowCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getNodeType("nonexistent", "root") == "unknown", "Type unknown on missing");
}


void run_station_tests() {
    testStationCreate();
    testStationDuplicateCreate();
    testStationDockInRange();
    testStationDockOutOfRange();
    testStationUndock();
    testStationUndockNotDocked();
    testStationRepair();
    testStationRepairNoDamage();
    testStationRepairNotDocked();
    testStationDockedCount();
    testStationDoubleDock();
    testStationMovementStopsOnDock();
    testStationDeploymentBegin();
    testStationDeploymentComplete();
    testStationDeploymentCancel();
    testStationDeploymentAttachModule();
    testStationDeploymentModuleLimit();
    testStationDeploymentSystemBonuses();
    testStationDeploymentNotDeployedCantAttach();
    testStationNewsEmpty();
    testStationNewsAddCombat();
    testStationNewsMaxEntries();
    testDockingPortDefaults();
    testDockingDock();
    testDockingUndock();
    testRestStationDefaults();
    testRestStationStartRest();
    testRestStationRecovery();
    testRestStationStopRest();
    testRestStationProgress();
    testRestStationQualityName();
    testRestStationAutoStop();
    testRestStationCount();
    testHangarCreate();
    testHangarStoreShip();
    testHangarRetrieveShip();
    testHangarUpgrade();
    testHangarMaxUpgrade();
    testHangarRentalAccrual();
    testHangarShouldUseHangarFrigate();
    testHangarShouldUseTetherCapital();
    testHangarShouldUseTetherTitan();
    testHangarSpawnPosition();
    testHangarDuplicateShip();
    testHangarComponentDefaults();
    testTetherArmCreate();
    testTetherBeginTether();
    testTetherExtendToLock();
    testTetherUndockRetract();
    testTetherDoubleOccupy();
    testTetherUndockOnlyWhenLocked();
    testTetherPartialExtend();
    testTetherComponentDefaults();
    testDockingRingInit();
    testDockingRingExtend();
    testDockingRingRetract();
    testDockingRingConnect();
    testDockingRingDisconnect();
    testDockingRingAlignment();
    testDockingRingPressure();
    testDockingRingPower();
    testDockingRingIntegrity();
    testDockingRingMissing();
    testDockNodeInit();
    testDockNodeAddWindow();
    testDockNodeRemoveWindow();
    testDockNodeSplit();
    testDockNodeDock();
    testDockNodeUndock();
    testDockNodeMaxWindows();
    testDockNodeLayout();
    testDockNodeNestedSplit();
    testDockNodeMissing();
}
