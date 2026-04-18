/**
 * Movement tests
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

// ==================== Movement System & Collision Tests ====================

void testMovementBasicUpdate() {
    std::cout << "\n=== Movement Basic Update ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    auto* entity = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = 100.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    assertTrue(approxEqual(pos->x, 100.0f), "Position updated by velocity * dt");
    assertTrue(approxEqual(pos->y, 0.0f), "Y unchanged when vy = 0");
}

void testMovementSpeedLimit() {
    std::cout << "\n=== Movement Speed Limit ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    auto* entity = world.createEntity("ship2");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    pos->x = 0.0f;
    vel->vx = 500.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    float speed = std::sqrt(vel->vx * vel->vx + vel->vy * vel->vy + vel->vz * vel->vz);
    assertTrue(speed <= vel->max_speed + 0.01f, "Speed clamped to max_speed");
}

void testMovementCollisionZonePush() {
    std::cout << "\n=== Movement Collision Zone Push ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    // Set up a collision zone at origin (like a sun)
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});  // 500km radius sun
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship3");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Place ship inside the sun's collision zone
    pos->x = 100000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = -100.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    
    // After update, ship should be pushed outside the collision zone
    float dist = std::sqrt(pos->x * pos->x + pos->y * pos->y + pos->z * pos->z);
    assertTrue(dist >= 500000.0f, "Ship pushed outside collision zone (sun)");
}

void testMovementCollisionZoneVelocityKilled() {
    std::cout << "\n=== Movement Collision Zone Velocity Killed ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship4");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Ship inside zone moving toward center
    pos->x = 100000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = -200.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 300.0f;
    
    moveSys.update(1.0f);
    
    // Velocity toward the celestial should be killed
    assertTrue(vel->vx >= 0.0f, "Velocity toward celestial killed (bounce effect)");
}

void testMovementOutsideCollisionZoneUnaffected() {
    std::cout << "\n=== Movement Outside Collision Zone Unaffected ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship5");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Ship well outside the collision zone
    pos->x = 1000000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = 100.0f; vel->vy = 50.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    
    // Position should be updated normally (not pushed)
    assertTrue(approxEqual(pos->x, 1000100.0f), "Ship outside zone moves normally in X");
    assertTrue(approxEqual(pos->y, 50.0f), "Ship outside zone moves normally in Y");
}

void testMovementMultipleCollisionZones() {
    std::cout << "\n=== Movement Multiple Collision Zones ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    // Sun at origin, planet at 1M meters
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});        // Sun
    zones.push_back({1000000.0f, 0.0f, 0.0f, 6000.0f});     // Planet
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship6");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Ship inside planet's collision zone
    pos->x = 999000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = 100.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    
    // Ship should be pushed out of planet's collision zone
    float distToPlanet = std::sqrt((pos->x - 1000000.0f) * (pos->x - 1000000.0f) + 
                                    pos->y * pos->y + pos->z * pos->z);
    assertTrue(distToPlanet >= 6000.0f, "Ship pushed outside planet collision zone");
}

// ==================== Movement Command Tests ====================

void testMovementOrbitCommand() {
    std::cout << "\n=== Movement Orbit Command ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(ship);
    vel->max_speed = 200.0f;

    auto* target = world.createEntity("target1");
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 1000.0f; tpos->y = 0.0f; tpos->z = 0.0f;
    addComp<components::Velocity>(target);

    moveSys.commandOrbit("ship1", "target1", 500.0f);
    moveSys.update(1.0f);

    // Ship should be moving (velocity non-zero)
    float speed = std::sqrt(vel->vx * vel->vx + vel->vy * vel->vy + vel->vz * vel->vz);
    assertTrue(speed > 0.0f, "Ship has velocity after orbit command");
}

void testMovementApproachCommand() {
    std::cout << "\n=== Movement Approach Command ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(ship);
    vel->max_speed = 200.0f;

    auto* target = world.createEntity("target1");
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 1000.0f; tpos->y = 0.0f; tpos->z = 0.0f;
    addComp<components::Velocity>(target);

    moveSys.commandApproach("ship1", "target1");
    moveSys.update(1.0f);

    // Ship should be moving toward target (positive vx)
    assertTrue(vel->vx > 0.0f, "Ship moving toward target (positive X)");
    assertTrue(pos->x > 0.0f, "Ship position moved toward target");
}

void testMovementStopCommand() {
    std::cout << "\n=== Movement Stop Command ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    addComp<components::Position>(ship);
    auto* vel = addComp<components::Velocity>(ship);
    vel->vx = 100.0f;
    vel->vy = 50.0f;
    vel->max_speed = 200.0f;

    moveSys.commandStop("ship1");
    assertTrue(vel->vx == 0.0f && vel->vy == 0.0f && vel->vz == 0.0f,
               "Velocity zeroed after stop command");
}

void testMovementWarpDistance() {
    std::cout << "\n=== Movement Warp Distance Check ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);

    // Try to warp too close (< 150km)
    bool warped = moveSys.commandWarp("ship1", 100.0f, 0.0f, 0.0f);
    assertTrue(!warped, "Warp rejected (destination too close)");

    // Warp to valid distance
    warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp accepted (>150km)");
}


void run_movement_tests() {
    testMovementBasicUpdate();
    testMovementSpeedLimit();
    testMovementCollisionZonePush();
    testMovementCollisionZoneVelocityKilled();
    testMovementOutsideCollisionZoneUnaffected();
    testMovementMultipleCollisionZones();
    testMovementOrbitCommand();
    testMovementApproachCommand();
    testMovementStopCommand();
    testMovementWarpDistance();
}
