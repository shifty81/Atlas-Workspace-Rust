/**
 * Fps tests
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

// ==================== Interior-Exterior Coupling Tests ====================

void testInteriorExteriorDefaults() {
    std::cout << "\n=== Interior-Exterior Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("link1");
    auto* link = addComp<components::InteriorExteriorLink>(e);
    assertTrue(link->effects.empty(), "No effects");
    assertTrue(link->visible_module_count == 0, "No visible modules");
    assertTrue(approxEqual(link->total_hull_deformation, 0.0f), "No hull deformation");
}

void testInteriorExteriorAddEffect() {
    std::cout << "\n=== Interior-Exterior Add Effect ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("link2");
    auto* link = addComp<components::InteriorExteriorLink>(e);
    link->addEffect("refinery", 0.3f, true, 1.5f);
    link->addEffect("cargo_rack", 0.1f, false, 1.0f);
    assertTrue(static_cast<int>(link->effects.size()) == 2, "Two effects");
    assertTrue(link->visible_module_count == 1, "One visible module");
    assertTrue(approxEqual(link->total_hull_deformation, 0.4f), "Total deformation 0.4");
}

void testInteriorExteriorClear() {
    std::cout << "\n=== Interior-Exterior Clear ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("link3");
    auto* link = addComp<components::InteriorExteriorLink>(e);
    link->addEffect("solar_panel", 0.2f, true, 1.0f);
    link->clearEffects();
    assertTrue(link->effects.empty(), "Effects cleared");
    assertTrue(link->visible_module_count == 0, "Visible count reset");
    assertTrue(approxEqual(link->total_hull_deformation, 0.0f), "Deformation reset");
}

// ==================== FPS Spawn System Tests ====================

void testFPSSpawnCreate() {
    std::cout << "\n=== FPS Spawn Create ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);

    assertTrue(sys.createSpawnPoint("spawn1", "station1",
                                     components::FPSSpawnPoint::SpawnContext::Hangar,
                                     10.0f, 0.0f, 2.0f, 90.0f),
               "Spawn point created");
    assertTrue(!sys.createSpawnPoint("spawn1", "station1",
                                      components::FPSSpawnPoint::SpawnContext::Hangar,
                                      0, 0, 0),
               "Duplicate spawn fails");
}

void testFPSSpawnTransform() {
    std::cout << "\n=== FPS Spawn Transform ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);
    sys.createSpawnPoint("spawn1", "station1",
                         components::FPSSpawnPoint::SpawnContext::Hangar,
                         10.0f, 5.0f, 2.0f, 90.0f);

    auto [x, y, z, yaw] = sys.getSpawnTransform("spawn1");
    assertTrue(approxEqual(x, 10.0f), "Spawn X");
    assertTrue(approxEqual(y, 5.0f), "Spawn Y");
    assertTrue(approxEqual(z, 2.0f), "Spawn Z");
    assertTrue(approxEqual(yaw, 90.0f), "Spawn Yaw");
}

void testFPSSpawnContext() {
    std::cout << "\n=== FPS Spawn Context ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);
    sys.createSpawnPoint("spawn1", "station1",
                         components::FPSSpawnPoint::SpawnContext::TetherAirlock,
                         0, 0, 0);

    assertTrue(sys.getSpawnContext("spawn1") ==
               components::FPSSpawnPoint::SpawnContext::TetherAirlock,
               "Context is TetherAirlock");
}

void testFPSSpawnActivation() {
    std::cout << "\n=== FPS Spawn Activation ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);
    sys.createSpawnPoint("spawn1", "station1",
                         components::FPSSpawnPoint::SpawnContext::Hangar,
                         0, 0, 0);

    assertTrue(sys.setSpawnActive("spawn1", false), "Deactivate succeeds");
    assertTrue(sys.setSpawnActive("spawn1", true), "Reactivate succeeds");
    assertTrue(!sys.setSpawnActive("nonexistent", false), "Nonexistent fails");
}

void testFPSSpawnFindForDockedPlayer() {
    std::cout << "\n=== FPS Spawn Find For Docked Player ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);

    // Create station and player.
    world.createEntity("station1");
    auto* player = world.createEntity("player1");
    auto docked = std::make_unique<components::Docked>();
    docked->station_id = "station1";
    player->addComponent(std::move(docked));

    // Create a hangar owned by this player at the station.
    auto* hangarEnt = world.createEntity("hangar1");
    auto hangarComp = std::make_unique<components::StationHangar>();
    hangarComp->station_id = "station1";
    hangarComp->owner_id = "player1";
    hangarEnt->addComponent(std::move(hangarComp));

    // Create hangar and lobby spawn points.
    sys.createSpawnPoint("sp_hangar", "station1",
                         components::FPSSpawnPoint::SpawnContext::Hangar,
                         10.0f, 0.0f, 2.0f);
    sys.createSpawnPoint("sp_lobby", "station1",
                         components::FPSSpawnPoint::SpawnContext::StationLobby,
                         0.0f, 0.0f, 0.0f);

    std::string spawn = sys.findSpawnForPlayer("player1");
    assertTrue(spawn == "sp_hangar", "Docked player with hangar spawns in hangar");
}

void testFPSSpawnFindForDockedPlayerNoHangar() {
    std::cout << "\n=== FPS Spawn Find For Docked Player (No Hangar) ===" << std::endl;
    ecs::World world;
    systems::FPSSpawnSystem sys(&world);

    world.createEntity("station1");
    auto* player = world.createEntity("player1");
    auto docked = std::make_unique<components::Docked>();
    docked->station_id = "station1";
    player->addComponent(std::move(docked));

    // Only a lobby spawn, no hangar.
    sys.createSpawnPoint("sp_lobby", "station1",
                         components::FPSSpawnPoint::SpawnContext::StationLobby,
                         0.0f, 0.0f, 0.0f);

    std::string spawn = sys.findSpawnForPlayer("player1");
    assertTrue(spawn == "sp_lobby", "Docked player without hangar spawns in lobby");
}

void testFPSSpawnContextNames() {
    std::cout << "\n=== FPS Spawn Context Names ===" << std::endl;
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::Hangar) == "Hangar", "Hangar name");
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::StationLobby) == "StationLobby", "StationLobby name");
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::ShipBridge) == "ShipBridge", "ShipBridge name");
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::TetherAirlock) == "TetherAirlock", "TetherAirlock name");
    assertTrue(systems::FPSSpawnSystem::contextName(
        components::FPSSpawnPoint::SpawnContext::EVAHatch) == "EVAHatch", "EVAHatch name");
}

void testFPSSpawnComponentDefaults() {
    std::cout << "\n=== FPS Spawn Component Defaults ===" << std::endl;
    components::FPSSpawnPoint sp;
    assertTrue(sp.context == components::FPSSpawnPoint::SpawnContext::Hangar, "Default context Hangar");
    assertTrue(sp.is_active, "Default active");
    assertTrue(approxEqual(sp.pos_x, 0.0f), "Default pos_x 0");
    assertTrue(approxEqual(sp.yaw, 0.0f), "Default yaw 0");
}

// ==================== FPS Character Controller System Tests ====================

void testFPSCharControllerSpawn() {
    std::cout << "\n=== FPS Character Controller Spawn ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);

    assertTrue(sys.spawnCharacter("player1", "ship_interior_1", 5.0f, 0.0f, 3.0f, 90.0f),
               "Spawn character");
    assertTrue(!sys.spawnCharacter("player1", "ship_interior_1", 0, 0, 0),
               "Duplicate spawn fails");

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(x, 5.0f), "Spawn X");
    assertTrue(approxEqual(y, 0.0f), "Spawn Y");
    assertTrue(approxEqual(z, 3.0f), "Spawn Z");
}

void testFPSCharControllerMovement() {
    std::cout << "\n=== FPS Character Controller Movement ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    // Walk forward (positive Z)
    sys.setMoveInput("player1", 0.0f, 1.0f);
    sys.update(1.0f);  // 1 second at walk speed 4 m/s

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(z, 4.0f), "Walked forward 4m in 1s");
    assertTrue(approxEqual(x, 0.0f), "No lateral movement");
}

void testFPSCharControllerSprint() {
    std::cout << "\n=== FPS Character Controller Sprint ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.setStance("player1", static_cast<int>(components::FPSCharacterState::Stance::Sprinting));
    sys.setMoveInput("player1", 0.0f, 1.0f);
    sys.update(1.0f);  // 1 second at sprint speed 7 m/s

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(z, 7.0f), "Sprinted forward 7m in 1s");
    assertTrue(sys.getStance("player1") ==
               static_cast<int>(components::FPSCharacterState::Stance::Sprinting),
               "Still sprinting");
}

void testFPSCharControllerCrouch() {
    std::cout << "\n=== FPS Character Controller Crouch ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.setStance("player1", static_cast<int>(components::FPSCharacterState::Stance::Crouching));
    sys.setMoveInput("player1", 1.0f, 0.0f);
    sys.update(1.0f);  // 1 second at crouch speed 2 m/s

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(x, 2.0f), "Crouch-walked sideways 2m in 1s");
}

void testFPSCharControllerJump() {
    std::cout << "\n=== FPS Character Controller Jump ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    assertTrue(sys.isGrounded("player1"), "Starts grounded");
    sys.requestJump("player1");
    sys.update(0.1f);  // Short tick

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(y > 0.0f, "Character is above ground after jump");
    assertTrue(!sys.isGrounded("player1"), "Not grounded during jump");
}

void testFPSCharControllerGravityLanding() {
    std::cout << "\n=== FPS Character Controller Gravity Landing ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.requestJump("player1");
    // Simulate enough time for jump arc to complete
    for (int i = 0; i < 30; ++i) {
        sys.update(0.1f);
    }

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(y, 0.0f), "Landed back on ground");
    assertTrue(sys.isGrounded("player1"), "Grounded after landing");
}

void testFPSCharControllerZeroG() {
    std::cout << "\n=== FPS Character Controller Zero-G ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 5.0f, 0.0f);

    sys.setGravity("player1", 0.0f);  // Zero gravity
    sys.update(1.0f);

    auto [x, y, z] = sys.getPosition("player1");
    assertTrue(approxEqual(y, 5.0f), "No falling in zero-g");
    assertTrue(!sys.isGrounded("player1"), "Never grounded in zero-g");
}

void testFPSCharControllerStaminaDrain() {
    std::cout << "\n=== FPS Character Controller Stamina Drain ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    assertTrue(approxEqual(sys.getStaminaFraction("player1"), 1.0f), "Full stamina initially");

    sys.setStance("player1", static_cast<int>(components::FPSCharacterState::Stance::Sprinting));
    sys.setMoveInput("player1", 0.0f, 1.0f);
    sys.update(3.0f);  // 3 seconds of sprinting, drains 60 stamina

    assertTrue(sys.getStaminaFraction("player1") < 1.0f, "Stamina drained by sprinting");
    assertTrue(sys.getStaminaFraction("player1") > 0.0f, "Stamina not fully depleted");
}

void testFPSCharControllerStaminaExhaustion() {
    std::cout << "\n=== FPS Character Controller Stamina Exhaustion ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.setStance("player1", static_cast<int>(components::FPSCharacterState::Stance::Sprinting));
    sys.setMoveInput("player1", 0.0f, 1.0f);
    sys.update(6.0f);  // 6 seconds, drains 120 stamina (max 100) → exhausted

    assertTrue(sys.getStance("player1") ==
               static_cast<int>(components::FPSCharacterState::Stance::Standing),
               "Reverts to standing when exhausted");
}

void testFPSCharControllerLookDirection() {
    std::cout << "\n=== FPS Character Controller Look Direction ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    sys.setLookDirection("player1", 180.0f, 45.0f);
    auto [yaw, pitch] = sys.getLookDirection("player1");
    assertTrue(approxEqual(yaw, 180.0f), "Yaw set");
    assertTrue(approxEqual(pitch, 45.0f), "Pitch set");

    // Pitch clamp
    sys.setLookDirection("player1", 0.0f, 95.0f);
    auto [yaw2, pitch2] = sys.getLookDirection("player1");
    assertTrue(approxEqual(pitch2, 89.0f), "Pitch clamped to 89");
}

void testFPSCharControllerRemove() {
    std::cout << "\n=== FPS Character Controller Remove ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);
    sys.spawnCharacter("player1", "interior1", 0.0f, 0.0f, 0.0f);

    assertTrue(sys.removeCharacter("player1"), "Remove succeeds");
    assertTrue(!sys.removeCharacter("player1"), "Double remove fails");
    assertTrue(!sys.removeCharacter("nonexistent"), "Remove nonexistent fails");
}

void testFPSCharControllerStanceNames() {
    std::cout << "\n=== FPS Character Controller Stance Names ===" << std::endl;
    assertTrue(systems::FPSCharacterControllerSystem::stanceName(0) == "Standing", "Standing name");
    assertTrue(systems::FPSCharacterControllerSystem::stanceName(1) == "Crouching", "Crouching name");
    assertTrue(systems::FPSCharacterControllerSystem::stanceName(2) == "Sprinting", "Sprinting name");
    assertTrue(systems::FPSCharacterControllerSystem::stanceName(99) == "Unknown", "Unknown stance");
}

void testFPSCharControllerComponentDefaults() {
    std::cout << "\n=== FPS Character Controller Component Defaults ===" << std::endl;
    components::FPSCharacterState state;
    assertTrue(state.stance == 0, "Default stance standing");
    assertTrue(state.grounded, "Default grounded");
    assertTrue(approxEqual(state.walk_speed, 4.0f), "Default walk speed 4");
    assertTrue(approxEqual(state.sprint_speed, 7.0f), "Default sprint speed 7");
    assertTrue(approxEqual(state.gravity, 9.81f), "Default gravity 9.81");
    assertTrue(approxEqual(state.stamina, 100.0f), "Default stamina 100");
}

// ==================== Interior Door System Tests ====================

void testInteriorDoorCreate() {
    std::cout << "\n=== Interior Door Create ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);

    assertTrue(sys.createDoor("door1", "interior1", "roomA", "roomB",
               static_cast<int>(components::InteriorDoor::DoorType::Standard)),
               "Door created");
    assertTrue(!sys.createDoor("door1", "interior1", "roomA", "roomB"),
               "Duplicate door fails");
}

void testInteriorDoorOpenClose() {
    std::cout << "\n=== Interior Door Open/Close ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("door1", "interior1", "roomA", "roomB");

    assertTrue(sys.getDoorState("door1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Closed),
               "Starts closed");
    assertTrue(sys.requestOpen("door1"), "Open request succeeds");
    assertTrue(sys.getDoorState("door1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Opening),
               "State is Opening");

    // Simulate until fully open
    for (int i = 0; i < 30; ++i) sys.update(0.1f);

    assertTrue(sys.getDoorState("door1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Open),
               "Door fully open");
    assertTrue(approxEqual(sys.getOpenProgress("door1"), 1.0f), "Progress is 1.0");
}

void testInteriorDoorAutoClose() {
    std::cout << "\n=== Interior Door Auto-Close ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("door1", "interior1", "roomA", "roomB");

    sys.requestOpen("door1");
    // Open fully
    for (int i = 0; i < 30; ++i) sys.update(0.1f);

    // Wait for auto-close (default 5 seconds)
    for (int i = 0; i < 60; ++i) sys.update(0.1f);

    // Should be closing or closed by now
    int state = sys.getDoorState("door1");
    assertTrue(state == static_cast<int>(components::InteriorDoor::DoorState::Closing) ||
               state == static_cast<int>(components::InteriorDoor::DoorState::Closed),
               "Door auto-closing after delay");
}

void testInteriorDoorLock() {
    std::cout << "\n=== Interior Door Lock ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("door1", "interior1", "roomA", "roomB");

    assertTrue(sys.lockDoor("door1"), "Lock succeeds when closed");
    assertTrue(sys.isLocked("door1"), "Door is locked");
    assertTrue(sys.getDoorState("door1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Locked),
               "State is Locked");
    assertTrue(!sys.requestOpen("door1"), "Can't open locked door");

    assertTrue(sys.unlockDoor("door1"), "Unlock succeeds");
    assertTrue(!sys.isLocked("door1"), "Door is unlocked");
    assertTrue(sys.requestOpen("door1"), "Can open after unlock");
}

void testInteriorDoorAirlockPressure() {
    std::cout << "\n=== Interior Door Airlock Pressure ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("airlock1", "interior1", "roomA", "roomB",
                   static_cast<int>(components::InteriorDoor::DoorType::Airlock));

    // Normal pressure — should open
    sys.setPressure("airlock1", 1.0f, 1.0f);
    assertTrue(sys.requestOpen("airlock1"), "Opens with equal pressure");

    // Reset to closed for next test
    ecs::World world2;
    systems::InteriorDoorSystem sys2(&world2);
    sys2.createDoor("airlock2", "interior1", "roomA", "roomB",
                    static_cast<int>(components::InteriorDoor::DoorType::Airlock));

    // Vacuum on one side — should fail
    sys2.setPressure("airlock2", 1.0f, 0.0f);
    assertTrue(!sys2.requestOpen("airlock2"), "Refuses to open with pressure diff");
    assertTrue(sys2.hasPressureWarning("airlock2") == false,
               "No warning until update");

    sys2.update(0.0f);  // Trigger pressure warning calculation
    assertTrue(sys2.hasPressureWarning("airlock2"), "Pressure warning after update");
}

void testInteriorDoorSecurityAccess() {
    std::cout << "\n=== Interior Door Security Access ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("secDoor1", "interior1", "roomA", "roomB",
                   static_cast<int>(components::InteriorDoor::DoorType::Security));

    // Set access requirement
    auto* entity = world.getEntity("secDoor1");
    auto* door = entity->getComponent<components::InteriorDoor>();
    door->required_access = "engineer";

    assertTrue(!sys.requestOpen("secDoor1", ""), "No access denied");
    assertTrue(!sys.requestOpen("secDoor1", "civilian"), "Wrong access denied");
    assertTrue(sys.requestOpen("secDoor1", "engineer"), "Correct access granted");
}

void testInteriorDoorStateNames() {
    std::cout << "\n=== Interior Door State Names ===" << std::endl;
    assertTrue(systems::InteriorDoorSystem::stateName(0) == "Closed", "Closed name");
    assertTrue(systems::InteriorDoorSystem::stateName(1) == "Opening", "Opening name");
    assertTrue(systems::InteriorDoorSystem::stateName(2) == "Open", "Open name");
    assertTrue(systems::InteriorDoorSystem::stateName(3) == "Closing", "Closing name");
    assertTrue(systems::InteriorDoorSystem::stateName(4) == "Locked", "Locked name");
}

void testInteriorDoorTypeNames() {
    std::cout << "\n=== Interior Door Type Names ===" << std::endl;
    assertTrue(systems::InteriorDoorSystem::typeName(0) == "Standard", "Standard name");
    assertTrue(systems::InteriorDoorSystem::typeName(1) == "Airlock", "Airlock name");
    assertTrue(systems::InteriorDoorSystem::typeName(2) == "Security", "Security name");
    assertTrue(systems::InteriorDoorSystem::typeName(99) == "Unknown", "Unknown type");
}

void testInteriorDoorComponentDefaults() {
    std::cout << "\n=== Interior Door Component Defaults ===" << std::endl;
    components::InteriorDoor d;
    assertTrue(d.door_type == 0, "Default type Standard");
    assertTrue(d.door_state == 0, "Default state Closed");
    assertTrue(approxEqual(d.open_progress, 0.0f), "Default progress 0");
    assertTrue(approxEqual(d.pressure_a, 1.0f), "Default pressure_a 1.0");
    assertTrue(approxEqual(d.pressure_b, 1.0f), "Default pressure_b 1.0");
    assertTrue(!d.is_locked, "Default not locked");
}

// ==================== EVA Airlock System Tests ====================

void testEVAAirlockCreate() {
    std::cout << "\n=== EVA Airlock Create ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);

    assertTrue(sys.createAirlock("airlock1", "ship1", 1.0f), "Airlock created");
    assertTrue(!sys.createAirlock("airlock1", "ship1"), "Duplicate fails");
    assertTrue(sys.getPhase("airlock1") ==
               static_cast<int>(components::EVAAirlockState::Phase::Idle),
               "Starts idle");
}

void testEVAAirlockBeginEVA() {
    std::cout << "\n=== EVA Airlock Begin EVA ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 0.5f);

    // Insufficient oxygen
    assertTrue(!sys.beginEVA("airlock1", "player1", 5.0f),
               "EVA denied with low oxygen");

    // Sufficient oxygen
    assertTrue(sys.beginEVA("airlock1", "player1", 50.0f),
               "EVA started with enough oxygen");
    assertTrue(sys.getPhase("airlock1") ==
               static_cast<int>(components::EVAAirlockState::Phase::EnterChamber),
               "Phase is EnterChamber");
}

void testEVAAirlockFullSequence() {
    std::cout << "\n=== EVA Airlock Full EVA Sequence ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 0.5f);

    sys.beginEVA("airlock1", "player1", 100.0f);

    // Run through all phases: EnterChamber → InnerSeal → Depressurize → OuterOpen → EVAActive
    for (int i = 0; i < 50; ++i) sys.update(0.1f);

    assertTrue(sys.isInEVA("airlock1"), "Player is in EVA");
    assertTrue(approxEqual(sys.getChamberPressure("airlock1"), 0.0f),
               "Chamber depressurized");
}

void testEVAAirlockReentry() {
    std::cout << "\n=== EVA Airlock Reentry ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 0.5f);

    sys.beginEVA("airlock1", "player1", 100.0f);
    for (int i = 0; i < 50; ++i) sys.update(0.1f);  // Reach EVA

    assertTrue(sys.beginReentry("airlock1", "player1"), "Reentry started");

    // Run through re-entry: OuterSeal → Repressurize → InnerOpen → Complete
    for (int i = 0; i < 50; ++i) sys.update(0.1f);

    assertTrue(sys.getPhase("airlock1") ==
               static_cast<int>(components::EVAAirlockState::Phase::Complete),
               "Reentry complete");
    assertTrue(approxEqual(sys.getChamberPressure("airlock1"), 1.0f),
               "Chamber repressurized");
}

void testEVAAirlockAbort() {
    std::cout << "\n=== EVA Airlock Abort ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 1.0f);

    sys.beginEVA("airlock1", "player1", 100.0f);
    sys.update(0.5f);  // Partial progress in EnterChamber

    assertTrue(sys.abortSequence("airlock1"), "Abort succeeds during pre-EVA");

    sys.update(0.01f);  // Process abort
    assertTrue(sys.getPhase("airlock1") ==
               static_cast<int>(components::EVAAirlockState::Phase::Idle),
               "Reverted to Idle after abort");
}

void testEVAAirlockReentryWrongPlayer() {
    std::cout << "\n=== EVA Airlock Reentry Wrong Player ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 0.5f);

    sys.beginEVA("airlock1", "player1", 100.0f);
    for (int i = 0; i < 50; ++i) sys.update(0.1f);

    assertTrue(!sys.beginReentry("airlock1", "player2"),
               "Wrong player can't reenter");
}

void testEVAAirlockPhaseNames() {
    std::cout << "\n=== EVA Airlock Phase Names ===" << std::endl;
    assertTrue(systems::EVAAirlockSystem::phaseName(0) == "Idle", "Idle name");
    assertTrue(systems::EVAAirlockSystem::phaseName(1) == "EnterChamber", "EnterChamber name");
    assertTrue(systems::EVAAirlockSystem::phaseName(3) == "Depressurize", "Depressurize name");
    assertTrue(systems::EVAAirlockSystem::phaseName(5) == "EVAActive", "EVAActive name");
    assertTrue(systems::EVAAirlockSystem::phaseName(9) == "Complete", "Complete name");
}

void testEVAAirlockComponentDefaults() {
    std::cout << "\n=== EVA Airlock Component Defaults ===" << std::endl;
    components::EVAAirlockState state;
    assertTrue(state.phase == 0, "Default phase Idle");
    assertTrue(approxEqual(state.chamber_pressure, 1.0f), "Default chamber pressure 1.0");
    assertTrue(!state.inner_door_open, "Default inner door closed");
    assertTrue(!state.outer_door_open, "Default outer door closed");
    assertTrue(approxEqual(state.min_suit_oxygen, 10.0f), "Default min suit oxygen 10");
    assertTrue(!state.abort_requested, "Default no abort");
}

// ==================== FPS Interaction System Tests ====================

void testFPSInteractionCreate() {
    std::cout << "\n=== FPS Interaction Create ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);

    bool ok = sys.createInteractable("ia_door1", "interior_1", "door_1",
                                      components::FPSInteractable::InteractionType::Door,
                                      5.0f, 0.0f, 3.0f, 2.0f, "Open Door");
    assertTrue(ok, "Create interactable succeeds");
    assertTrue(!sys.createInteractable("ia_door1", "interior_1", "door_1",
                                        components::FPSInteractable::InteractionType::Door,
                                        5.0f, 0.0f, 3.0f),
               "Duplicate create fails");
}

void testFPSInteractionFindNearest() {
    std::cout << "\n=== FPS Interaction Find Nearest ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);

    sys.createInteractable("ia_near", "interior_1", "door_near",
                            components::FPSInteractable::InteractionType::Door,
                            5.5f, 0.0f, 3.0f, 2.0f);
    sys.createInteractable("ia_far", "interior_1", "door_far",
                            components::FPSInteractable::InteractionType::Door,
                            50.0f, 0.0f, 3.0f, 2.0f);

    std::string nearest = sys.findNearestInteractable("p1");
    assertTrue(nearest == "ia_near", "Nearest interactable found");
}

void testFPSInteractionOutOfRange() {
    std::cout << "\n=== FPS Interaction Out Of Range ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);

    sys.createInteractable("ia_far", "interior_1", "door_far",
                            components::FPSInteractable::InteractionType::Door,
                            100.0f, 0.0f, 0.0f, 2.0f);

    assertTrue(!sys.isInRange("p1", "ia_far"), "Out of range detected");
    assertTrue(sys.findNearestInteractable("p1").empty(), "No nearest when all out of range");
}

void testFPSInteractionDoorToggle() {
    std::cout << "\n=== FPS Interaction Door Toggle ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);
    systems::InteriorDoorSystem doorSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);
    doorSys.createDoor("door_1", "interior_1", "roomA", "roomB",
                        static_cast<int>(components::InteriorDoor::DoorType::Standard));

    sys.createInteractable("ia_door1", "interior_1", "door_1",
                            components::FPSInteractable::InteractionType::Door,
                            5.5f, 0.0f, 3.0f, 2.0f);

    bool ok = sys.interact("p1", "ia_door1");
    assertTrue(ok, "Door opened via interaction");
    assertTrue(doorSys.getDoorState("door_1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Opening),
               "Door is now Opening");

    // Fully open the door
    doorSys.update(10.0f);
    assertTrue(doorSys.getDoorState("door_1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Open),
               "Door fully opened");

    // Close via interaction
    bool ok2 = sys.interact("p1", "ia_door1");
    assertTrue(ok2, "Door closed via interaction");
    assertTrue(doorSys.getDoorState("door_1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Closing),
               "Door is now Closing");
}

void testFPSInteractionAirlock() {
    std::cout << "\n=== FPS Interaction Airlock ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);
    systems::EVAAirlockSystem alSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);
    // Add survival needs for oxygen check
    auto* charEnt = world.getEntity("fpschar_p1");
    auto needs = std::make_unique<components::SurvivalNeeds>();
    needs->oxygen = 80.0f;
    charEnt->addComponent(std::move(needs));

    alSys.createAirlock("airlock_1", "ship_1", 2.0f);

    sys.createInteractable("ia_airlock1", "interior_1", "airlock_1",
                            components::FPSInteractable::InteractionType::Airlock,
                            5.5f, 0.0f, 3.0f, 2.0f);

    bool ok = sys.interact("p1", "ia_airlock1");
    assertTrue(ok, "Airlock EVA initiated via interaction");
    assertTrue(alSys.getPhase("airlock_1") ==
               static_cast<int>(components::EVAAirlockState::Phase::EnterChamber),
               "Airlock in EnterChamber phase");
}

void testFPSInteractionMedicalBay() {
    std::cout << "\n=== FPS Interaction Medical Bay ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);
    auto* charEnt = world.getEntity("fpschar_p1");
    auto needs = std::make_unique<components::SurvivalNeeds>();
    needs->oxygen = 30.0f;
    needs->hunger = 80.0f;
    needs->fatigue = 70.0f;
    charEnt->addComponent(std::move(needs));

    sys.createInteractable("ia_med1", "interior_1", "",
                            components::FPSInteractable::InteractionType::MedicalBay,
                            5.5f, 0.0f, 3.0f, 2.0f);

    bool ok = sys.interact("p1", "ia_med1");
    assertTrue(ok, "Medical bay used");

    auto* n = charEnt->getComponent<components::SurvivalNeeds>();
    assertTrue(approxEqual(n->oxygen, 100.0f), "Oxygen refilled");
    assertTrue(n->hunger < 80.0f, "Hunger reduced");
    assertTrue(n->fatigue < 70.0f, "Fatigue reduced");
}

void testFPSInteractionAccessControl() {
    std::cout << "\n=== FPS Interaction Access Control ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);

    sys.createInteractable("ia_sec", "interior_1", "terminal_sec",
                            components::FPSInteractable::InteractionType::Terminal,
                            5.5f, 0.0f, 3.0f, 2.0f);

    // Set access requirement
    auto* iaEnt = world.getEntity("ia_sec");
    auto* ia = iaEnt->getComponent<components::FPSInteractable>();
    ia->required_access = "captain";

    assertTrue(!sys.interact("p1", "ia_sec", "crew"), "Wrong access denied");
    assertTrue(sys.interact("p1", "ia_sec", "captain"), "Correct access granted");
}

void testFPSInteractionEnable() {
    std::cout << "\n=== FPS Interaction Enable/Disable ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 5.0f, 0.0f, 3.0f, 0.0f);

    sys.createInteractable("ia_t", "interior_1", "term_1",
                            components::FPSInteractable::InteractionType::Terminal,
                            5.5f, 0.0f, 3.0f, 2.0f);

    assertTrue(sys.interact("p1", "ia_t"), "Interact when enabled");
    sys.setEnabled("ia_t", false);
    assertTrue(!sys.interact("p1", "ia_t"), "Interact fails when disabled");
    sys.setEnabled("ia_t", true);
    assertTrue(sys.interact("p1", "ia_t"), "Interact succeeds after re-enable");
}

void testFPSInteractionTypeNames() {
    std::cout << "\n=== FPS Interaction Type Names ===" << std::endl;
    assertTrue(systems::FPSInteractionSystem::typeName(0) == "Door", "Door name");
    assertTrue(systems::FPSInteractionSystem::typeName(1) == "Airlock", "Airlock name");
    assertTrue(systems::FPSInteractionSystem::typeName(2) == "Terminal", "Terminal name");
    assertTrue(systems::FPSInteractionSystem::typeName(3) == "LootContainer", "LootContainer name");
    assertTrue(systems::FPSInteractionSystem::typeName(4) == "Fabricator", "Fabricator name");
    assertTrue(systems::FPSInteractionSystem::typeName(5) == "MedicalBay", "MedicalBay name");
    assertTrue(systems::FPSInteractionSystem::typeName(99) == "Unknown", "Unknown type");
}

void testFPSInteractionComponentDefaults() {
    std::cout << "\n=== FPS Interaction Component Defaults ===" << std::endl;
    components::FPSInteractable ia;
    assertTrue(ia.interaction_type == 0, "Default type Door");
    assertTrue(approxEqual(ia.interact_range, 2.0f), "Default range 2.0");
    assertTrue(ia.is_enabled, "Default enabled");
    assertTrue(ia.required_access.empty(), "Default no access required");
}

void testFPSInteractionDifferentInteriors() {
    std::cout << "\n=== FPS Interaction Different Interiors ===" << std::endl;
    ecs::World world;
    systems::FPSInteractionSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_A", 5.0f, 0.0f, 3.0f, 0.0f);

    sys.createInteractable("ia_other", "interior_B", "door_other",
                            components::FPSInteractable::InteractionType::Door,
                            5.5f, 0.0f, 3.0f, 2.0f);

    assertTrue(sys.findNearestInteractable("p1").empty(),
               "Cannot find interactable in different interior");
}

// ==================== ShipInteriorLayoutSystem Tests ====================

void testInteriorLayoutGenerateFrigate() {
    std::cout << "\n=== Interior Layout Generate Frigate ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    bool ok = sys.generateLayout("ship_1", "frigate");
    assertTrue(ok, "Generate frigate layout succeeds");
    assertTrue(sys.getRoomCount("ship_1") == 5, "Frigate has 5 rooms");
}

void testInteriorLayoutGenerateBattleship() {
    std::cout << "\n=== Interior Layout Generate Battleship ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    bool ok = sys.generateLayout("ship_bs", "battleship");
    assertTrue(ok, "Generate battleship layout succeeds");
    assertTrue(sys.getRoomCount("ship_bs") == 10, "Battleship has 10 rooms");
}

void testInteriorLayoutGenerateCapital() {
    std::cout << "\n=== Interior Layout Generate Capital ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    bool ok = sys.generateLayout("ship_cap", "capital");
    assertTrue(ok, "Generate capital layout succeeds");
    assertTrue(sys.getRoomCount("ship_cap") == 12, "Capital has 12 rooms");
}

void testInteriorLayoutDuplicate() {
    std::cout << "\n=== Interior Layout Duplicate ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "frigate");
    bool dup = sys.generateLayout("ship_1", "frigate");
    assertTrue(!dup, "Duplicate generation fails");
}

void testInteriorLayoutFindBridge() {
    std::cout << "\n=== Interior Layout Find Bridge ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "cruiser");
    std::string bridge = sys.findRoomByType("ship_1",
        components::ShipInteriorLayout::RoomType::Bridge);
    assertTrue(!bridge.empty(), "Bridge room found");
}

void testInteriorLayoutFindEngineering() {
    std::cout << "\n=== Interior Layout Find Engineering ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "cruiser");
    std::string eng = sys.findRoomByType("ship_1",
        components::ShipInteriorLayout::RoomType::Engineering);
    assertTrue(!eng.empty(), "Engineering room found");
}

void testInteriorLayoutConnections() {
    std::cout << "\n=== Interior Layout Connections ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "frigate");
    auto* layout = sys.getLayout("ship_1");
    assertTrue(layout != nullptr, "Layout exists");
    assertTrue(layout->connectionCount() > 0, "Has connections");

    // First two rooms should be connected
    if (layout->rooms.size() >= 2) {
        assertTrue(layout->areConnected(layout->rooms[0].room_id,
                                         layout->rooms[1].room_id),
                   "Adjacent rooms are connected");
    }
}

void testInteriorLayoutAdjacentRooms() {
    std::cout << "\n=== Interior Layout Adjacent Rooms ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "cruiser");
    auto* layout = sys.getLayout("ship_1");
    assertTrue(layout != nullptr, "Layout exists");

    if (layout->rooms.size() >= 3) {
        // Middle room should have at least 2 neighbors
        auto adj = sys.getAdjacentRooms("ship_1", layout->rooms[1].room_id);
        assertTrue(adj.size() >= 2, "Middle room has at least 2 adjacent rooms");
    }
}

void testInteriorLayoutUnknownShip() {
    std::cout << "\n=== Interior Layout Unknown Ship ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    assertTrue(sys.getRoomCount("nonexistent") == 0, "No rooms for unknown ship");
    assertTrue(sys.getLayout("nonexistent") == nullptr, "No layout for unknown ship");
}

void testInteriorLayoutRoomTypeName() {
    std::cout << "\n=== Interior Layout Room Type Names ===" << std::endl;
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(0) == "Bridge", "Bridge name");
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(1) == "Engineering", "Engineering name");
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(6) == "Corridor", "Corridor name");
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(7) == "Airlock", "Airlock name");
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(99) == "Unknown", "Unknown name");
}

void testInteriorLayoutAirlockExteriorConnection() {
    std::cout << "\n=== Interior Layout Airlock Exterior Connection ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "frigate");
    auto* layout = sys.getLayout("ship_1");
    assertTrue(layout != nullptr, "Layout exists");

    // Should have at least one connection to "exterior"
    bool hasExterior = false;
    for (const auto& c : layout->connections) {
        if (c.to_room_id == "exterior" || c.from_room_id == "exterior") {
            hasExterior = true;
            break;
        }
    }
    assertTrue(hasExterior, "Airlock has exterior connection");
}

void testInteriorLayoutComponentDefaults() {
    std::cout << "\n=== Interior Layout Component Defaults ===" << std::endl;
    components::ShipInteriorLayout layout;
    assertTrue(layout.rooms.empty(), "Default empty rooms");
    assertTrue(layout.connections.empty(), "Default empty connections");
    assertTrue(layout.roomCount() == 0, "Default room count 0");
    assertTrue(layout.connectionCount() == 0, "Default connection count 0");
    assertTrue(!layout.hasRoom("any"), "Default has no rooms");
    assertTrue(layout.getRoom("any") == nullptr, "Default getRoom null");
}

// ==================== EnvironmentalHazardSystem Tests ====================

void testEnvHazardCreate() {
    std::cout << "\n=== Environmental Hazard Create ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    bool ok = sys.createHazard("hz_1", "room_eng", "interior_1",
                                components::EnvironmentalHazard::HazardType::Fire,
                                components::EnvironmentalHazard::Severity::Moderate);
    assertTrue(ok, "Create hazard succeeds");
    assertTrue(!sys.createHazard("hz_1", "room_eng", "interior_1",
                                  components::EnvironmentalHazard::HazardType::Fire,
                                  components::EnvironmentalHazard::Severity::Moderate),
               "Duplicate create fails");
}

void testEnvHazardRoomSafety() {
    std::cout << "\n=== Environmental Hazard Room Safety ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    assertTrue(sys.isRoomSafe("room_a"), "Empty room is safe");

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::HullBreach,
                      components::EnvironmentalHazard::Severity::Minor);
    assertTrue(!sys.isRoomSafe("room_a"), "Room with hazard is unsafe");
    assertTrue(sys.isRoomSafe("room_b"), "Other room still safe");
}

void testEnvHazardDPS() {
    std::cout << "\n=== Environmental Hazard DPS ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    float dps = sys.getRoomDPS("room_a");
    assertTrue(dps > 0.0f, "Minor hazard has positive DPS");
    assertTrue(approxEqual(dps, 2.0f), "Minor hazard DPS is 2.0");
}

void testEnvHazardDPSSeverity() {
    std::cout << "\n=== Environmental Hazard DPS Severity ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_cat", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Catastrophic);
    float dps = sys.getRoomDPS("room_a");
    assertTrue(approxEqual(dps, 25.0f), "Catastrophic hazard DPS is 25.0");
}

void testEnvHazardMultipleInRoom() {
    std::cout << "\n=== Environmental Hazard Multiple In Room ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    sys.createHazard("hz_2", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Radiation,
                      components::EnvironmentalHazard::Severity::Moderate);

    auto hazards = sys.getHazardsInRoom("room_a");
    assertTrue(hazards.size() == 2, "Two hazards in room");
    float dps = sys.getRoomDPS("room_a");
    assertTrue(approxEqual(dps, 7.0f), "Combined DPS is 7.0 (2+5)");
}

void testEnvHazardRepair() {
    std::cout << "\n=== Environmental Hazard Repair ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    assertTrue(sys.startRepair("hz_1"), "Start repair succeeds");
    assertTrue(approxEqual(sys.getRepairProgress("hz_1"), 0.0f), "Initial repair progress 0");

    // Simulate 10 seconds of repair (rate = 0.1/s → 1.0 progress)
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getRepairProgress("hz_1"), 1.0f), "Repair complete after 10s");
    assertTrue(sys.isRoomSafe("room_a"), "Room safe after repair");
}

void testEnvHazardStopRepair() {
    std::cout << "\n=== Environmental Hazard Stop Repair ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    sys.startRepair("hz_1");
    sys.update(3.0f);  // Partial repair
    sys.stopRepair("hz_1");
    float prog = sys.getRepairProgress("hz_1");
    assertTrue(prog > 0.0f && prog < 1.0f, "Partial repair progress");

    sys.update(5.0f);  // No further repair progress
    assertTrue(approxEqual(sys.getRepairProgress("hz_1"), prog), "No progress after stop");
}

void testEnvHazardRemove() {
    std::cout << "\n=== Environmental Hazard Remove ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    assertTrue(!sys.isRoomSafe("room_a"), "Room unsafe with hazard");
    assertTrue(sys.removeHazard("hz_1"), "Remove hazard succeeds");
    assertTrue(sys.isRoomSafe("room_a"), "Room safe after removal");
}

void testEnvHazardSpreadTimer() {
    std::cout << "\n=== Environmental Hazard Spread Timer ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Catastrophic);

    auto* entity = world.getEntity("hz_1");
    auto* h = entity->getComponent<components::EnvironmentalHazard>();
    assertTrue(!h->is_spreading, "Not spreading initially");

    // Catastrophic spread interval = 5.0s
    sys.update(5.0f);
    assertTrue(h->is_spreading, "Spreading after interval");
}

void testEnvHazardActiveList() {
    std::cout << "\n=== Environmental Hazard Active List ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem sys(&world);

    sys.createHazard("hz_1", "room_a", "interior_1",
                      components::EnvironmentalHazard::HazardType::Fire,
                      components::EnvironmentalHazard::Severity::Minor);
    sys.createHazard("hz_2", "room_b", "interior_1",
                      components::EnvironmentalHazard::HazardType::Radiation,
                      components::EnvironmentalHazard::Severity::Moderate);
    sys.createHazard("hz_3", "room_c", "interior_2",
                      components::EnvironmentalHazard::HazardType::ToxicLeak,
                      components::EnvironmentalHazard::Severity::Critical);

    auto active1 = sys.getActiveHazards("interior_1");
    assertTrue(active1.size() == 2, "2 active hazards in interior_1");

    auto active2 = sys.getActiveHazards("interior_2");
    assertTrue(active2.size() == 1, "1 active hazard in interior_2");
}

void testEnvHazardTypeName() {
    std::cout << "\n=== Environmental Hazard Type Names ===" << std::endl;
    assertTrue(systems::EnvironmentalHazardSystem::hazardTypeName(0) == "HullBreach", "HullBreach name");
    assertTrue(systems::EnvironmentalHazardSystem::hazardTypeName(1) == "Fire", "Fire name");
    assertTrue(systems::EnvironmentalHazardSystem::hazardTypeName(2) == "Radiation", "Radiation name");
    assertTrue(systems::EnvironmentalHazardSystem::severityName(0) == "Minor", "Minor severity");
    assertTrue(systems::EnvironmentalHazardSystem::severityName(3) == "Catastrophic", "Catastrophic severity");
}

void testEnvHazardComponentDefaults() {
    std::cout << "\n=== Environmental Hazard Component Defaults ===" << std::endl;
    components::EnvironmentalHazard h;
    assertTrue(h.is_active == true, "Default active");
    assertTrue(!h.is_spreading, "Default not spreading");
    assertTrue(!h.is_being_repaired, "Default not being repaired");
    assertTrue(approxEqual(h.repair_progress, 0.0f), "Default repair 0");
    assertTrue(approxEqual(h.damage_per_second, 5.0f), "Default DPS 5.0");
    assertTrue(approxEqual(h.spread_interval, 30.0f), "Default spread interval 30");
}

void testEnvHazardRoomLevelDamage() {
    std::cout << "\n=== Environmental Hazard Room-Level Damage ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem hazSys(&world);
    systems::FPSCharacterControllerSystem fpsSys(&world);

    // Create a fire hazard in room_eng
    hazSys.createHazard("hz_fire", "room_eng", "interior_ship",
                         components::EnvironmentalHazard::HazardType::Fire,
                         components::EnvironmentalHazard::Severity::Minor);

    // Spawn two characters in the same interior
    fpsSys.spawnCharacter("player_in_room", "interior_ship", 0, 0, 0);
    fpsSys.spawnCharacter("player_other_room", "interior_ship", 0, 0, 0);

    // Set rooms: one in the hazard room, one elsewhere
    fpsSys.setCurrentRoom("player_in_room", "room_eng");
    fpsSys.setCurrentRoom("player_other_room", "room_bridge");

    // Give both characters health
    auto* e1 = world.getEntity("fpschar_player_in_room");
    auto h1 = std::make_unique<components::FPSHealth>();
    h1->health = 100.0f; h1->shield = 0.0f;
    e1->addComponent(std::move(h1));

    auto* e2 = world.getEntity("fpschar_player_other_room");
    auto h2 = std::make_unique<components::FPSHealth>();
    h2->health = 100.0f; h2->shield = 0.0f;
    e2->addComponent(std::move(h2));

    // Update for 1 second (Minor DPS = 2.0)
    hazSys.update(1.0f);

    auto* hp1 = e1->getComponent<components::FPSHealth>();
    auto* hp2 = e2->getComponent<components::FPSHealth>();
    assertTrue(hp1->health < 100.0f, "Player in hazard room takes damage");
    assertTrue(approxEqual(hp1->health, 98.0f), "Player takes 2.0 damage in 1s");
    assertTrue(approxEqual(hp2->health, 100.0f), "Player in other room takes no damage");
}

void testEnvHazardNoRoomNoDamage() {
    std::cout << "\n=== Environmental Hazard No Room No Damage ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem hazSys(&world);
    systems::FPSCharacterControllerSystem fpsSys(&world);

    hazSys.createHazard("hz_1", "room_eng", "interior_ship",
                         components::EnvironmentalHazard::HazardType::Fire,
                         components::EnvironmentalHazard::Severity::Moderate);

    // Spawn character with empty current_room_id (default)
    fpsSys.spawnCharacter("player_no_room", "interior_ship", 0, 0, 0);

    auto* entity = world.getEntity("fpschar_player_no_room");
    auto hp = std::make_unique<components::FPSHealth>();
    hp->health = 100.0f; hp->shield = 0.0f;
    entity->addComponent(std::move(hp));

    hazSys.update(1.0f);

    auto* health = entity->getComponent<components::FPSHealth>();
    assertTrue(approxEqual(health->health, 100.0f), "Character without room takes no damage");
}

void testEnvHazardRoomDamageShieldCascade() {
    std::cout << "\n=== Environmental Hazard Room Damage Shield Cascade ===" << std::endl;
    ecs::World world;
    systems::EnvironmentalHazardSystem hazSys(&world);
    systems::FPSCharacterControllerSystem fpsSys(&world);

    hazSys.createHazard("hz_1", "room_eng", "interior_ship",
                         components::EnvironmentalHazard::HazardType::Radiation,
                         components::EnvironmentalHazard::Severity::Catastrophic);

    fpsSys.spawnCharacter("shielded", "interior_ship", 0, 0, 0);
    fpsSys.setCurrentRoom("shielded", "room_eng");

    auto* entity = world.getEntity("fpschar_shielded");
    auto hp = std::make_unique<components::FPSHealth>();
    hp->health = 100.0f; hp->shield = 10.0f;
    entity->addComponent(std::move(hp));

    // Catastrophic DPS = 25.0, 1 second → 25 damage
    // Shield absorbs 10, remaining 15 hits health
    hazSys.update(1.0f);

    auto* health = entity->getComponent<components::FPSHealth>();
    assertTrue(approxEqual(health->shield, 0.0f), "Shield depleted");
    assertTrue(approxEqual(health->health, 85.0f), "Health takes overflow damage");
}

void testFPSCharControllerCurrentRoom() {
    std::cout << "\n=== FPS Character Controller Current Room ===" << std::endl;
    ecs::World world;
    systems::FPSCharacterControllerSystem sys(&world);

    sys.spawnCharacter("player1", "interior_ship", 0, 0, 0);
    assertTrue(sys.getCurrentRoom("player1").empty(), "Default room is empty");

    assertTrue(sys.setCurrentRoom("player1", "room_bridge"), "Set room succeeds");
    assertTrue(sys.getCurrentRoom("player1") == "room_bridge", "Room is bridge");

    assertTrue(sys.setCurrentRoom("player1", "room_eng"), "Change room succeeds");
    assertTrue(sys.getCurrentRoom("player1") == "room_eng", "Room is engineering");

    assertTrue(!sys.setCurrentRoom("nonexistent", "room_a"), "Set room fails for missing player");
    assertTrue(sys.getCurrentRoom("nonexistent").empty(), "Get room empty for missing player");
}

// ==================== FPSObjectiveSystem Tests ====================

void testFPSObjectiveCreate() {
    std::cout << "\n=== FPS Objective Create ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    bool ok = sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                                   components::FPSObjective::ObjectiveType::EliminateHostiles,
                                   "Clear the bridge of enemies");
    assertTrue(ok, "Create objective succeeds");
    assertTrue(!sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                                     components::FPSObjective::ObjectiveType::EliminateHostiles),
               "Duplicate create fails");
}

void testFPSObjectiveActivate() {
    std::cout << "\n=== FPS Objective Activate ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles);
    assertTrue(sys.getObjectiveState("obj_1") ==
               static_cast<int>(components::FPSObjective::ObjectiveState::Inactive),
               "Starts inactive");

    assertTrue(sys.activateObjective("obj_1"), "Activate succeeds");
    assertTrue(sys.getObjectiveState("obj_1") ==
               static_cast<int>(components::FPSObjective::ObjectiveState::Active),
               "Now active");

    // Can't activate again
    assertTrue(!sys.activateObjective("obj_1"), "Double activate fails");
}

void testFPSObjectiveEliminateHostiles() {
    std::cout << "\n=== FPS Objective Eliminate Hostiles ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles);
    sys.setHostileCount("obj_1", 3);
    sys.activateObjective("obj_1");

    sys.reportHostileKill("obj_1");
    assertTrue(approxEqual(sys.getProgress("obj_1"), 1.0f / 3.0f), "1/3 progress");

    sys.reportHostileKill("obj_1");
    assertTrue(approxEqual(sys.getProgress("obj_1"), 2.0f / 3.0f), "2/3 progress");

    sys.reportHostileKill("obj_1");
    assertTrue(sys.isComplete("obj_1"), "Objective complete after 3 kills");
    assertTrue(approxEqual(sys.getProgress("obj_1"), 1.0f), "Progress at 1.0");
}

void testFPSObjectiveRetrieveItem() {
    std::cout << "\n=== FPS Objective Retrieve Item ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_cargo", "player1",
                         components::FPSObjective::ObjectiveType::RetrieveItem);
    sys.setTargetItem("obj_1", "data_core_alpha");
    sys.activateObjective("obj_1");

    // Wrong item
    assertTrue(!sys.reportItemCollected("obj_1", "wrong_item"), "Wrong item fails");
    assertTrue(!sys.isComplete("obj_1"), "Not complete yet");

    // Correct item
    assertTrue(sys.reportItemCollected("obj_1", "data_core_alpha"), "Correct item succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

void testFPSObjectiveDefendPoint() {
    std::cout << "\n=== FPS Objective Defend Point ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_eng", "player1",
                         components::FPSObjective::ObjectiveType::DefendPoint);
    sys.setDefendDuration("obj_1", 60.0f);
    sys.activateObjective("obj_1");

    sys.update(30.0f);
    assertTrue(approxEqual(sys.getProgress("obj_1"), 0.5f), "50% progress at 30s");
    assertTrue(!sys.isComplete("obj_1"), "Not complete at 30s");

    sys.update(30.0f);
    assertTrue(sys.isComplete("obj_1"), "Complete after 60s");
}

void testFPSObjectiveSabotage() {
    std::cout << "\n=== FPS Objective Sabotage ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_eng", "player1",
                         components::FPSObjective::ObjectiveType::Sabotage,
                         "Destroy the reactor core");
    sys.activateObjective("obj_1");

    assertTrue(sys.reportSabotageComplete("obj_1"), "Sabotage report succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

void testFPSObjectiveEscape() {
    std::cout << "\n=== FPS Objective Escape ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_airlock", "player1",
                         components::FPSObjective::ObjectiveType::Escape);
    sys.activateObjective("obj_1");

    assertTrue(sys.reportExtraction("obj_1"), "Extraction succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

void testFPSObjectiveRescueVIP() {
    std::cout << "\n=== FPS Objective Rescue VIP ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_quarters", "player1",
                         components::FPSObjective::ObjectiveType::RescueVIP,
                         "Rescue the ambassador");
    sys.activateObjective("obj_1");

    assertTrue(sys.reportVIPRescued("obj_1"), "VIP rescue succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

void testFPSObjectiveRepairSystem() {
    std::cout << "\n=== FPS Objective Repair System ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_eng", "player1",
                         components::FPSObjective::ObjectiveType::RepairSystem,
                         "Fix the reactor coolant system");
    sys.activateObjective("obj_1");

    assertTrue(sys.reportRepairComplete("obj_1"), "Repair report succeeds");
    assertTrue(sys.isComplete("obj_1"), "Objective complete");
}

void testFPSObjectiveTimeLimit() {
    std::cout << "\n=== FPS Objective Time Limit ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles,
                         "Clear the bridge", 10.0f);
    sys.setHostileCount("obj_1", 5);
    sys.activateObjective("obj_1");

    sys.update(10.0f);
    assertTrue(sys.isFailed("obj_1"), "Objective failed due to time limit");
}

void testFPSObjectiveFailManual() {
    std::cout << "\n=== FPS Objective Manual Fail ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_bridge", "player1",
                         components::FPSObjective::ObjectiveType::DefendPoint);
    sys.activateObjective("obj_1");

    assertTrue(sys.failObjective("obj_1"), "Manual fail succeeds");
    assertTrue(sys.isFailed("obj_1"), "Objective is failed");
}

void testFPSObjectivePlayerList() {
    std::cout << "\n=== FPS Objective Player List ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_a", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles);
    sys.createObjective("obj_2", "interior_1", "room_b", "player1",
                         components::FPSObjective::ObjectiveType::Sabotage);
    sys.createObjective("obj_3", "interior_1", "room_c", "player2",
                         components::FPSObjective::ObjectiveType::Escape);

    sys.activateObjective("obj_1");
    sys.activateObjective("obj_2");
    sys.activateObjective("obj_3");

    auto p1_objs = sys.getPlayerObjectives("player1");
    assertTrue(p1_objs.size() == 2, "Player1 has 2 objectives");

    auto p2_objs = sys.getPlayerObjectives("player2");
    assertTrue(p2_objs.size() == 1, "Player2 has 1 objective");
}

void testFPSObjectiveWrongType() {
    std::cout << "\n=== FPS Objective Wrong Type Report ===" << std::endl;
    ecs::World world;
    systems::FPSObjectiveSystem sys(&world);

    sys.createObjective("obj_1", "interior_1", "room_a", "player1",
                         components::FPSObjective::ObjectiveType::EliminateHostiles);
    sys.activateObjective("obj_1");

    // Can't report sabotage on an eliminate objective
    assertTrue(!sys.reportSabotageComplete("obj_1"), "Wrong type report fails");
    assertTrue(!sys.reportExtraction("obj_1"), "Wrong type extraction fails");
    assertTrue(!sys.isComplete("obj_1"), "Not complete from wrong reports");
}

void testFPSObjectiveTypeName() {
    std::cout << "\n=== FPS Objective Type Names ===" << std::endl;
    assertTrue(systems::FPSObjectiveSystem::objectiveTypeName(0) == "EliminateHostiles", "EliminateHostiles name");
    assertTrue(systems::FPSObjectiveSystem::objectiveTypeName(1) == "RescueVIP", "RescueVIP name");
    assertTrue(systems::FPSObjectiveSystem::objectiveTypeName(2) == "Sabotage", "Sabotage name");
    assertTrue(systems::FPSObjectiveSystem::objectiveTypeName(3) == "DefendPoint", "DefendPoint name");
    assertTrue(systems::FPSObjectiveSystem::stateName(0) == "Inactive", "Inactive state name");
    assertTrue(systems::FPSObjectiveSystem::stateName(2) == "Completed", "Completed state name");
}

void testFPSObjectiveComponentDefaults() {
    std::cout << "\n=== FPS Objective Component Defaults ===" << std::endl;
    components::FPSObjective obj;
    assertTrue(obj.state == 0, "Default state inactive");
    assertTrue(obj.objective_type == 0, "Default type eliminate");
    assertTrue(approxEqual(obj.progress, 0.0f), "Default progress 0");
    assertTrue(approxEqual(obj.time_limit, 0.0f), "Default no time limit");
    assertTrue(obj.hostiles_required == 0, "Default 0 hostiles required");
    assertTrue(obj.hostiles_killed == 0, "Default 0 hostiles killed");
    assertTrue(!obj.item_collected, "Default item not collected");
}

// ==================== RoverInteriorSystem Tests ====================

void testRoverInteriorInit() {
    std::cout << "\n=== Rover Interior: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    assertTrue(sys.initializeInterior("rover1", "rover_001"), "Interior initialized");
    assertTrue(sys.isPressurized("rover1"), "Initially pressurized");
    assertTrue(approxEqual(sys.getOxygenLevel("rover1"), 100.0f), "Oxygen at 100%");
    assertTrue(!sys.initializeInterior("rover1", "rover_002"), "Duplicate init rejected");
}

void testRoverInteriorAddRoom() {
    std::cout << "\n=== Rover Interior: Add Room ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");

    assertTrue(sys.addRoom("rover1", "cockpit", components::RoverInterior::RoomType::Cockpit), "Cockpit added");
    assertTrue(sys.getRoomCount("rover1") == 1, "1 room");
    assertTrue(sys.getRoomType("rover1", "cockpit") == "cockpit", "Room type is cockpit");
    assertTrue(!sys.addRoom("rover1", "cockpit", components::RoverInterior::RoomType::CargoHold), "Duplicate room rejected");
}

void testRoverInteriorRigLocker() {
    std::cout << "\n=== Rover Interior: Rig Locker ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");

    assertTrue(!sys.hasRigLocker("rover1"), "No rig locker initially");
    assertTrue(!sys.storeRig("rover1", "rig1"), "Cannot store without locker");

    sys.addRoom("rover1", "locker", components::RoverInterior::RoomType::RigLocker);
    sys.update(0.1f);  // Update to detect rig locker
    assertTrue(sys.hasRigLocker("rover1"), "Has rig locker after adding");
    assertTrue(sys.storeRig("rover1", "rig1"), "Rig stored");
    assertTrue(sys.getStoredRigCount("rover1") == 1, "1 rig stored");
    assertTrue(sys.retrieveRig("rover1", "rig1"), "Rig retrieved");
    assertTrue(sys.getStoredRigCount("rover1") == 0, "0 rigs after retrieval");
}

void testRoverInteriorEquipment() {
    std::cout << "\n=== Rover Interior: Equipment ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");
    sys.addRoom("rover1", "bay", components::RoverInterior::RoomType::EquipmentBay);

    assertTrue(sys.installEquipment("rover1", "bay", "equip1"), "Equipment installed");
    assertTrue(sys.getEquipmentCount("rover1", "bay") == 1, "1 equipment");
    assertTrue(sys.removeEquipment("rover1", "bay", "equip1"), "Equipment removed");
    assertTrue(sys.getEquipmentCount("rover1", "bay") == 0, "0 equipment after removal");
}

void testRoverInteriorPressurization() {
    std::cout << "\n=== Rover Interior: Pressurization ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");
    sys.addRoom("rover1", "room", components::RoverInterior::RoomType::CargoHold);

    assertTrue(sys.isPressurized("rover1"), "Initially pressurized");
    assertTrue(sys.setPressurized("rover1", false), "Depressurize");
    assertTrue(!sys.isPressurized("rover1"), "Not pressurized");

    sys.update(10.0f); // 10 seconds of oxygen leak
    assertTrue(sys.getOxygenLevel("rover1") < 100.0f, "Oxygen decreased");
}

void testRoverInteriorVolume() {
    std::cout << "\n=== Rover Interior: Volume ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");
    sys.addRoom("rover1", "cargo", components::RoverInterior::RoomType::CargoHold);
    sys.update(0.1f);

    assertTrue(sys.getTotalVolume("rover1") > 0.0f, "Volume > 0 after adding room");
}

void testRoverInteriorMaxRooms() {
    std::cout << "\n=== Rover Interior: Max Rooms ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");

    // Default max is 4
    sys.addRoom("rover1", "r1", components::RoverInterior::RoomType::Cockpit);
    sys.addRoom("rover1", "r2", components::RoverInterior::RoomType::CargoHold);
    sys.addRoom("rover1", "r3", components::RoverInterior::RoomType::Scanner);
    sys.addRoom("rover1", "r4", components::RoverInterior::RoomType::Airlock);
    assertTrue(sys.getRoomCount("rover1") == 4, "4 rooms");
    assertTrue(!sys.addRoom("rover1", "r5", components::RoverInterior::RoomType::EquipmentBay), "5th room rejected");
}

void testRoverInteriorRemoveRoom() {
    std::cout << "\n=== Rover Interior: Remove Room ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");
    sys.addRoom("rover1", "cargo", components::RoverInterior::RoomType::CargoHold);
    assertTrue(sys.getRoomCount("rover1") == 1, "1 room");
    assertTrue(sys.removeRoom("rover1", "cargo"), "Room removed");
    assertTrue(sys.getRoomCount("rover1") == 0, "0 rooms");
    assertTrue(!sys.removeRoom("rover1", "cargo"), "Double remove fails");
}

void testRoverInteriorMissing() {
    std::cout << "\n=== Rover Interior: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::RoverInteriorSystem sys(&world);
    assertTrue(sys.getRoomCount("nonexistent") == 0, "No rooms on missing");
    assertTrue(!sys.isPressurized("nonexistent"), "Not pressurized on missing");
    assertTrue(approxEqual(sys.getOxygenLevel("nonexistent"), 0.0f), "Zero oxygen on missing");
    assertTrue(!sys.hasRigLocker("nonexistent"), "No locker on missing");
}

// ==================== BikeGarageSystem Tests ====================

void testBikeGarageInit() {
    std::cout << "\n=== Bike Garage: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    assertTrue(sys.initializeGarage("ship1", "ship_owner", 2), "Garage initialized");
    assertTrue(sys.getCapacity("ship1") == 2, "Capacity is 2");
    assertTrue(sys.getBikeCount("ship1") == 0, "No bikes initially");
    assertTrue(!sys.isFull("ship1"), "Not full");
    assertTrue(!sys.initializeGarage("ship1", "other_owner", 3), "Duplicate init rejected");
}

void testBikeGarageStore() {
    std::cout << "\n=== Bike Garage: Store Bike ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);

    assertTrue(sys.storeBike("ship1", "bike1", 12345, "Solari"), "Bike stored");
    assertTrue(sys.getBikeCount("ship1") == 1, "1 bike");
    assertTrue(sys.hasBike("ship1", "bike1"), "Has bike1");
    assertTrue(!sys.storeBike("ship1", "bike1", 67890, "Veyren"), "Duplicate bike rejected");
}

void testBikeGarageRetrieve() {
    std::cout << "\n=== Bike Garage: Retrieve Bike ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 12345, "Solari");

    assertTrue(sys.retrieveBike("ship1", "bike1"), "Bike retrieved");
    assertTrue(sys.getBikeCount("ship1") == 0, "0 bikes");
    assertTrue(!sys.hasBike("ship1", "bike1"), "No longer has bike1");
    assertTrue(!sys.retrieveBike("ship1", "bike1"), "Double retrieve fails");
}

void testBikeGarageFull() {
    std::cout << "\n=== Bike Garage: Full ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 111, "Solari");
    sys.storeBike("ship1", "bike2", 222, "Veyren");

    assertTrue(sys.isFull("ship1"), "Garage is full");
    assertTrue(!sys.storeBike("ship1", "bike3", 333, "Aurelian"), "Third bike rejected");
}

void testBikeGarageLock() {
    std::cout << "\n=== Bike Garage: Lock/Unlock ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 12345, "Solari");

    assertTrue(!sys.isBikeLocked("ship1", "bike1"), "Not locked initially");
    assertTrue(sys.lockBike("ship1", "bike1"), "Locked");
    assertTrue(sys.isBikeLocked("ship1", "bike1"), "Is locked");
    assertTrue(!sys.retrieveBike("ship1", "bike1"), "Cannot retrieve locked");
    assertTrue(sys.unlockBike("ship1", "bike1"), "Unlocked");
    assertTrue(sys.retrieveBike("ship1", "bike1"), "Can retrieve after unlock");
}

void testBikeGarageFuel() {
    std::cout << "\n=== Bike Garage: Fuel ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 12345, "Solari");

    assertTrue(approxEqual(sys.getBikeFuel("ship1", "bike1"), 100.0f), "Full fuel");
    assertTrue(sys.setBikeFuel("ship1", "bike1", 50.0f), "Set fuel");
    assertTrue(approxEqual(sys.getBikeFuel("ship1", "bike1"), 50.0f), "Fuel at 50%");
}

void testBikeGarageHull() {
    std::cout << "\n=== Bike Garage: Hull Integrity ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 12345, "Solari");

    assertTrue(approxEqual(sys.getBikeHullIntegrity("ship1", "bike1"), 100.0f), "Full hull");
    assertTrue(sys.setBikeHullIntegrity("ship1", "bike1", 75.0f), "Set hull");
    assertTrue(approxEqual(sys.getBikeHullIntegrity("ship1", "bike1"), 75.0f), "Hull at 75%");
}

void testBikeGarageDoor() {
    std::cout << "\n=== Bike Garage: Door ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);

    assertTrue(!sys.isDoorOpen("ship1"), "Door closed initially");
    assertTrue(sys.openDoor("ship1"), "Door opened");
    assertTrue(sys.isDoorOpen("ship1"), "Door is open");

    sys.update(2.0f); // Animate door
    assertTrue(sys.getDoorProgress("ship1") > 0.0f, "Door progress > 0");

    assertTrue(sys.closeDoor("ship1"), "Door closed");
    assertTrue(!sys.isDoorOpen("ship1"), "Door is closed");
}

void testBikeGaragePower() {
    std::cout << "\n=== Bike Garage: Power ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);

    assertTrue(sys.isPowerEnabled("ship1"), "Power enabled initially");
    assertTrue(sys.setPowerEnabled("ship1", false), "Power disabled");
    assertTrue(!sys.isPowerEnabled("ship1"), "Power is off");
    assertTrue(!sys.openDoor("ship1"), "Cannot open door without power");
    assertTrue(!sys.storeBike("ship1", "bike1", 111, "Solari"), "Cannot store without power");
}

void testBikeGarageMissing() {
    std::cout << "\n=== Bike Garage: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::BikeGarageSystem sys(&world);
    assertTrue(sys.getBikeCount("nonexistent") == 0, "No bikes on missing");
    assertTrue(sys.getCapacity("nonexistent") == 0, "No capacity on missing");
    assertTrue(sys.isFull("nonexistent"), "Full on missing (default true)");
    assertTrue(!sys.hasBike("nonexistent", "bike1"), "No bike on missing");
}

// ==================== Planetary Traversal System Tests ====================

void testTraversalInit() {
    std::cout << "\n=== Planetary Traversal: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    assertTrue(sys.initializeTraversal("explorer1", "planet_001", 10.0f, 20.0f), "Traversal initialized");
    assertTrue(approxEqual(sys.getPositionX("explorer1"), 10.0f), "Start X correct");
    assertTrue(approxEqual(sys.getPositionY("explorer1"), 20.0f), "Start Y correct");
    assertTrue(!sys.initializeTraversal("explorer1", "planet_002", 0.0f, 0.0f), "Duplicate init rejected");
}

void testTraversalSetDestination() {
    std::cout << "\n=== Planetary Traversal: Set Destination ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);

    assertTrue(sys.setDestination("explorer1", 100.0f, 100.0f), "Destination set");
    assertTrue(sys.isTraversing("explorer1"), "Is traversing");
    assertTrue(sys.getDistanceToDestination("explorer1") > 0.0f, "Distance to dest > 0");
}

void testTraversalMovement() {
    std::cout << "\n=== Planetary Traversal: Movement ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setDestination("explorer1", 100.0f, 0.0f);

    sys.update(1.0f);
    assertTrue(sys.getPositionX("explorer1") > 0.0f, "Position X advanced");
    assertTrue(sys.isTraversing("explorer1"), "Still traversing");
    assertTrue(sys.getSpeed("explorer1") > 0.0f, "Speed > 0 while moving");
}

void testTraversalArrival() {
    std::cout << "\n=== Planetary Traversal: Arrival ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setDestination("explorer1", 0.5f, 0.0f);  // very close

    sys.update(1.0f);
    assertTrue(!sys.isTraversing("explorer1"), "Arrived at destination");
    assertTrue(approxEqual(sys.getPositionX("explorer1"), 0.5f), "At destination X");
}

void testTraversalVehicle() {
    std::cout << "\n=== Planetary Traversal: Vehicle ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);

    assertTrue(sys.setVehicle("explorer1", "rover_001", 20.0f), "Vehicle set");
    sys.setDestination("explorer1", 100.0f, 0.0f);
    sys.update(1.0f);
    assertTrue(sys.getSpeed("explorer1") > 5.0f, "Speed increased with vehicle");
}

void testTraversalDismount() {
    std::cout << "\n=== Planetary Traversal: Dismount ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setVehicle("explorer1", "rover_001", 20.0f);

    assertTrue(sys.dismountVehicle("explorer1"), "Dismounted");
    sys.setDestination("explorer1", 100.0f, 0.0f);
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getSpeed("explorer1"), 5.0f), "Speed reset to foot speed");
}

void testTraversalTerrain() {
    std::cout << "\n=== Planetary Traversal: Terrain ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);

    assertTrue(sys.setTerrainType("explorer1", components::PlanetaryTraversal::TerrainType::Mountains), "Terrain set");
    assertTrue(sys.getTerrainTypeStr("explorer1") == "mountains", "Terrain is mountains");
    sys.setDestination("explorer1", 100.0f, 0.0f);
    sys.update(1.0f);
    assertTrue(sys.getSpeed("explorer1") < 5.0f, "Speed reduced on mountains");
}

void testTraversalDistance() {
    std::cout << "\n=== Planetary Traversal: Distance ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setDestination("explorer1", 100.0f, 0.0f);

    sys.update(1.0f);
    sys.update(1.0f);
    assertTrue(sys.getDistanceTraveled("explorer1") > 0.0f, "Distance accumulated");
    float dist = sys.getDistanceTraveled("explorer1");
    sys.update(1.0f);
    assertTrue(sys.getDistanceTraveled("explorer1") > dist, "Distance keeps increasing");
}

void testTraversalClear() {
    std::cout << "\n=== Planetary Traversal: Clear Destination ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setDestination("explorer1", 100.0f, 100.0f);

    assertTrue(sys.isTraversing("explorer1"), "Traversing after set");
    assertTrue(sys.clearDestination("explorer1"), "Cleared destination");
    assertTrue(!sys.isTraversing("explorer1"), "Not traversing after clear");
}

void testTraversalMissing() {
    std::cout << "\n=== Planetary Traversal: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::PlanetaryTraversalSystem sys(&world);
    assertTrue(!sys.initializeTraversal("nonexistent", "planet", 0.0f, 0.0f), "Init fails on missing");
    assertTrue(!sys.setDestination("nonexistent", 10.0f, 10.0f), "SetDest fails on missing");
    assertTrue(approxEqual(sys.getPositionX("nonexistent"), 0.0f), "PosX 0 on missing");
    assertTrue(approxEqual(sys.getDistanceTraveled("nonexistent"), 0.0f), "Distance 0 on missing");
    assertTrue(!sys.isTraversing("nonexistent"), "Not traversing on missing");
}

// ==================== Rover Bay Ramp System Tests ====================

void testRoverBayInit() {
    std::cout << "\n=== Rover Bay: Init ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    auto* entity = world.createEntity("test_ship");
    assertTrue(sys.initializeBay("test_ship", 3), "Bay initialized");
    auto* bay = entity->getComponent<components::RoverBayRamp>();
    assertTrue(bay != nullptr, "Bay component exists");
    assertTrue(bay->max_rovers == 3, "Max rovers set");
    assertTrue(bay->state == components::RoverBayRamp::RampState::Closed, "Initial state closed");
    assertTrue(!sys.initializeBay("test_ship", 2), "Duplicate init fails");
}

void testRoverBayOpen() {
    std::cout << "\n=== Rover Bay: Open ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    assertTrue(sys.openRamp("test_ship"), "Open starts");
    assertTrue(sys.getRampState("test_ship") == "opening", "State is opening");
    sys.update(2.0f);
    assertTrue(approxEqual(sys.getRampProgress("test_ship"), 0.6f), "Progress 0.6 after 2s");
    sys.update(2.0f);
    assertTrue(sys.getRampState("test_ship") == "open", "State open after 4s");
}

void testRoverBayClose() {
    std::cout << "\n=== Rover Bay: Close ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.openRamp("test_ship");
    sys.update(4.0f);
    assertTrue(sys.closeRamp("test_ship"), "Close starts");
    assertTrue(sys.getRampState("test_ship") == "closing", "State is closing");
    sys.update(4.0f);
    assertTrue(sys.getRampState("test_ship") == "closed", "State closed after 4s");
}

void testRoverBayStore() {
    std::cout << "\n=== Rover Bay: Store ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    assertTrue(!sys.storeRover("test_ship", "rover1"), "Cannot store when closed");
    sys.openRamp("test_ship");
    sys.update(4.0f);
    assertTrue(sys.storeRover("test_ship", "rover1"), "Store rover1");
    assertTrue(sys.storeRover("test_ship", "rover2"), "Store rover2");
    assertTrue(!sys.storeRover("test_ship", "rover3"), "Cannot exceed capacity");
    assertTrue(sys.getStoredCount("test_ship") == 2, "Stored count is 2");
}

void testRoverBayDeploy() {
    std::cout << "\n=== Rover Bay: Deploy ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.openRamp("test_ship");
    sys.update(4.0f);
    sys.storeRover("test_ship", "rover1");
    assertTrue(sys.deployRover("test_ship", "rover1"), "Deploy rover1");
    assertTrue(sys.getStoredCount("test_ship") == 0, "Stored count 0 after deploy");
    assertTrue(sys.getDeployedCount("test_ship") == 1, "Deployed count 1");
    assertTrue(!sys.deployRover("test_ship", "rover1"), "Cannot deploy already deployed");
}

void testRoverBayRetrieve() {
    std::cout << "\n=== Rover Bay: Retrieve ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.openRamp("test_ship");
    sys.update(4.0f);
    sys.storeRover("test_ship", "rover1");
    sys.deployRover("test_ship", "rover1");
    assertTrue(sys.retrieveRover("test_ship", "rover1"), "Retrieve rover1");
    assertTrue(sys.getStoredCount("test_ship") == 1, "Stored count 1 after retrieve");
    assertTrue(sys.getDeployedCount("test_ship") == 0, "Deployed count 0 after retrieve");
}

void testRoverBaySafety() {
    std::cout << "\n=== Rover Bay: Safety ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.setExternalAtmosphere("test_ship", components::RoverBayRamp::AtmosphereType::Corrosive);
    assertTrue(!sys.openRamp("test_ship"), "Cannot open in corrosive atmosphere");
    sys.setExternalAtmosphere("test_ship", components::RoverBayRamp::AtmosphereType::Toxic);
    assertTrue(sys.openRamp("test_ship"), "Can open in toxic atmosphere");
}

void testRoverBayPower() {
    std::cout << "\n=== Rover Bay: Power ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    sys.setPowerEnabled("test_ship", false);
    assertTrue(!sys.openRamp("test_ship"), "Cannot open without power");
    sys.setPowerEnabled("test_ship", true);
    assertTrue(sys.openRamp("test_ship"), "Can open with power");
}

void testRoverBayAtmosphere() {
    std::cout << "\n=== Rover Bay: Atmosphere ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    auto* entity = world.createEntity("test_ship");
    sys.initializeBay("test_ship", 2);
    auto* bay = entity->getComponent<components::RoverBayRamp>();
    assertTrue(bay->is_pressurized, "Initially pressurized");
    sys.openRamp("test_ship");
    sys.update(4.0f);
    assertTrue(!bay->is_pressurized, "Depressurized when open");
    sys.closeRamp("test_ship");
    sys.update(4.0f);
    assertTrue(bay->is_pressurized, "Repressurized when closed");
}

void testRoverBayMissing() {
    std::cout << "\n=== Rover Bay: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::RoverBayRampSystem sys(&world);
    assertTrue(!sys.initializeBay("nonexistent", 2), "Init fails on missing");
    assertTrue(!sys.openRamp("nonexistent"), "Open fails on missing");
    assertTrue(sys.getRampState("nonexistent") == "unknown", "State unknown on missing");
    assertTrue(sys.getStoredCount("nonexistent") == 0, "Stored 0 on missing");
}

// ==================== LavatoryInteraction Tests ====================

void testLavatoryInit() {
    std::cout << "\n=== LavatoryInteraction: Init ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    assertTrue(sys.createLavatory("lav_1", "room_bathroom", 5.0f, 15.0f), "Lavatory created");
    assertTrue(sys.getPhase("lav_1") == 0, "Phase is idle");
    assertTrue(!sys.isOccupied("lav_1"), "Not occupied");
    assertTrue(!sys.createLavatory("lav_1", "room_2"), "Duplicate creation fails");
}

void testLavatoryBeginInteraction() {
    std::cout << "\n=== LavatoryInteraction: Begin ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom");
    assertTrue(sys.beginInteraction("lav_1", "player_1"), "Interaction started");
    assertTrue(sys.isOccupied("lav_1"), "Now occupied");
    assertTrue(sys.getPhase("lav_1") == 1, "Phase is approaching");
    assertTrue(!sys.beginInteraction("lav_1", "player_2"), "Can't use occupied lavatory");
}

void testLavatoryDoorTransition() {
    std::cout << "\n=== LavatoryInteraction: Door Transition ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 5.0f, 15.0f);
    sys.beginInteraction("lav_1", "player_1");
    // Advance through Approaching phase (phase_duration = 1.5s)
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.isDoorOpen("lav_1"), "Door opened during sequence");
}

void testLavatoryThirdPerson() {
    std::cout << "\n=== LavatoryInteraction: Third Person ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 5.0f, 15.0f);
    sys.beginInteraction("lav_1", "player_1");
    // Advance through Approaching + DoorOpening + into TransitionToThirdPerson
    for (int i = 0; i < 6; i++) sys.update(1.0f);
    // Should reach or pass through TransitionToThirdPerson
    assertTrue(sys.getPhase("lav_1") >= 3, "Advanced past door opening");
}

void testLavatoryFullCycle() {
    std::cout << "\n=== LavatoryInteraction: Full Cycle ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 2.0f, 15.0f);
    sys.beginInteraction("lav_1", "player_1");
    // Run enough ticks to complete the full cycle
    // 7 phases × 1.5s each (except UsingFacility which is 2.0s) ≈ 12.5s
    for (int i = 0; i < 30; i++) sys.update(1.0f);
    assertTrue(sys.getPhase("lav_1") == 7, "Reached complete phase");
    assertTrue(!sys.isOccupied("lav_1"), "No longer occupied after completion");
}

void testLavatoryCancel() {
    std::cout << "\n=== LavatoryInteraction: Cancel ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom");
    sys.beginInteraction("lav_1", "player_1");
    sys.update(1.0f);
    assertTrue(sys.cancelInteraction("lav_1"), "Interaction cancelled");
    assertTrue(!sys.isOccupied("lav_1"), "Not occupied after cancel");
    assertTrue(sys.getPhase("lav_1") == 0, "Phase reset to idle");
    assertTrue(!sys.cancelInteraction("lav_1"), "Can't cancel idle");
}

void testLavatoryAudio() {
    std::cout << "\n=== LavatoryInteraction: Audio ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 2.0f, 15.0f);
    sys.beginInteraction("lav_1", "player_1");
    // Initially no audio
    assertTrue(!sys.isAudioPlaying("lav_1"), "No audio initially");
    // Advance past approaching into door opening
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.isAudioPlaying("lav_1"), "Audio playing during sequence");
}

void testLavatoryHygiene() {
    std::cout << "\n=== LavatoryInteraction: Hygiene Bonus ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    world.createEntity("lav_1");
    sys.createLavatory("lav_1", "room_bathroom", 5.0f, 20.0f);
    assertTrue(approxEqual(sys.getHygieneBonus("lav_1"), 20.0f), "Custom hygiene bonus");
    world.createEntity("lav_2");
    sys.createLavatory("lav_2", "room_2");
    assertTrue(approxEqual(sys.getHygieneBonus("lav_2"), 15.0f), "Default hygiene bonus");
}

void testLavatoryPhaseNames() {
    std::cout << "\n=== LavatoryInteraction: Phase Names ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    assertTrue(sys.phaseName(0) == "idle", "Phase 0 name");
    assertTrue(sys.phaseName(1) == "approaching", "Phase 1 name");
    assertTrue(sys.phaseName(4) == "using_facility", "Phase 4 name");
    assertTrue(sys.phaseName(7) == "complete", "Phase 7 name");
}

void testLavatoryMissing() {
    std::cout << "\n=== LavatoryInteraction: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::LavatoryInteractionSystem sys(&world);
    assertTrue(!sys.createLavatory("nonexistent", "room"), "Create fails on missing");
    assertTrue(!sys.beginInteraction("nonexistent", "p"), "Begin fails on missing");
    assertTrue(sys.getPhase("nonexistent") == 0, "Phase 0 on missing");
    assertTrue(!sys.isOccupied("nonexistent"), "Not occupied on missing");
}

// ==================== EVAAirlockExit Tests ====================

void testEVAExitInit() {
    std::cout << "\n=== EVAAirlockExit: Init ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    assertTrue(sys.createExitPoint("airlock_1", "ship_1", 200.0f), "Exit point created");
    assertTrue(sys.getState("airlock_1") == 0, "State is inactive");
    assertTrue(!sys.isInSpace("airlock_1"), "Not in space");
    assertTrue(!sys.createExitPoint("airlock_1", "ship_1"), "Duplicate creation fails");
}

void testEVAExitRequest() {
    std::cout << "\n=== EVAAirlockExit: Request Exit ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    assertTrue(sys.requestExit("airlock_1", "player_1", 100.0f), "Exit requested");
    assertTrue(sys.getState("airlock_1") == 1, "State is requesting_exit");
    assertTrue(!sys.requestExit("airlock_1", "player_2", 100.0f), "Can't request twice");
}

void testEVAExitDockedBlocked() {
    std::cout << "\n=== EVAAirlockExit: Docked Blocked ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.setDockState("airlock_1", true);
    sys.requestExit("airlock_1", "player_1", 100.0f);
    // Advance through RequestingExit to CheckingDockState
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.isExitBlocked("airlock_1"), "Exit blocked while docked");
    assertTrue(sys.getState("airlock_1") == 0, "Returned to inactive");
}

void testEVAExitLowOxygen() {
    std::cout << "\n=== EVAAirlockExit: Low Oxygen ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.requestExit("airlock_1", "player_1", 5.0f); // Below min 10.0
    // Advance through RequestingExit to CheckingDockState
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.isExitBlocked("airlock_1"), "Exit blocked with low oxygen");
}

void testEVAExitFullCycle() {
    std::cout << "\n=== EVAAirlockExit: Full Cycle ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.setDockState("airlock_1", false);
    sys.requestExit("airlock_1", "player_1", 100.0f);
    // Advance through all phases to InSpace (states 1-5, 2s each = 10s)
    for (int i = 0; i < 15; i++) sys.update(1.0f);
    assertTrue(sys.isInSpace("airlock_1"), "Player is in space");
}

void testEVAExitTether() {
    std::cout << "\n=== EVAAirlockExit: Tether ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1", 100.0f);
    sys.setDockState("airlock_1", false);
    sys.requestExit("airlock_1", "player_1", 100.0f);
    for (int i = 0; i < 15; i++) sys.update(1.0f);
    assertTrue(sys.isInSpace("airlock_1"), "In space");
    sys.moveAway("airlock_1", 50.0f);
    assertTrue(approxEqual(sys.getDistanceFromShip("airlock_1"), 50.0f), "50m away");
    sys.moveAway("airlock_1", 200.0f);
    assertTrue(approxEqual(sys.getDistanceFromShip("airlock_1"), 100.0f), "Clamped to tether range");
    assertTrue(sys.isTetherActive("airlock_1"), "Tether is active");
}

void testEVAExitReturn() {
    std::cout << "\n=== EVAAirlockExit: Return ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.setDockState("airlock_1", false);
    sys.requestExit("airlock_1", "player_1", 100.0f);
    for (int i = 0; i < 15; i++) sys.update(1.0f);
    assertTrue(sys.beginReturn("airlock_1"), "Return started");
    // Advance through Returning to Complete
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.getState("airlock_1") == 7, "Reached complete state");
}

void testEVAExitCancel() {
    std::cout << "\n=== EVAAirlockExit: Cancel ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.requestExit("airlock_1", "player_1", 100.0f);
    assertTrue(sys.cancelExit("airlock_1"), "Exit cancelled");
    assertTrue(sys.getState("airlock_1") == 0, "State reset to inactive");
    assertTrue(!sys.cancelExit("airlock_1"), "Can't cancel inactive");
}

void testEVAExitStateNames() {
    std::cout << "\n=== EVAAirlockExit: State Names ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    assertTrue(sys.stateName(0) == "inactive", "State 0 name");
    assertTrue(sys.stateName(1) == "requesting_exit", "State 1 name");
    assertTrue(sys.stateName(5) == "in_space", "State 5 name");
    assertTrue(sys.stateName(7) == "complete", "State 7 name");
}

void testEVAExitMissing() {
    std::cout << "\n=== EVAAirlockExit: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    assertTrue(!sys.createExitPoint("nonexistent", "ship"), "Create fails on missing");
    assertTrue(!sys.requestExit("nonexistent", "p", 100.0f), "Request fails on missing");
    assertTrue(sys.getState("nonexistent") == 0, "State 0 on missing");
    assertTrue(!sys.isInSpace("nonexistent"), "Not in space on missing");
}

// ==================== AncientModuleDiscovery Tests ====================

void testAncientDiscoveryInit() {
    std::cout << "\n=== AncientModuleDiscovery: Init ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    assertTrue(sys.initializeSite("ruin_1", "ancient_site_01", "player_1", 75.0f), "Site initialized");
    assertTrue(approxEqual(sys.getScanRange("ruin_1"), 75.0f), "Scan range set");
    assertTrue(sys.getTotalModules("ruin_1") == 0, "No modules initially");
    assertTrue(!sys.initializeSite("ruin_1", "s", "p"), "Duplicate init fails");
}

void testAncientDiscoveryAddModule() {
    std::cout << "\n=== AncientModuleDiscovery: Add Module ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    assertTrue(sys.addHiddenModule("ruin_1", "mod_1", "shield_gen", 0.7f, 10.0f, 5000.0f), "Module added");
    assertTrue(sys.getTotalModules("ruin_1") == 1, "1 module total");
    assertTrue(!sys.addHiddenModule("ruin_1", "mod_1", "shield_gen", 0.5f, 5.0f, 1000.0f), "Duplicate rejected");
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 0, "Module is undiscovered");
}

void testAncientDiscoveryScan() {
    std::cout << "\n=== AncientModuleDiscovery: Scan ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "power_core", 0.5f, 8.0f, 3000.0f);
    assertTrue(sys.beginScan("ruin_1", "mod_1"), "Scan started");
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 1, "Module is scanning");
    assertTrue(!sys.beginScan("ruin_1", "mod_1"), "Can't re-scan");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 2, "Module discovered after scan");
    assertTrue(sys.getDiscoveredCount("ruin_1") == 1, "1 discovered");
}

void testAncientDiscoveryExtract() {
    std::cout << "\n=== AncientModuleDiscovery: Extract ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "weapon_array", 0.8f, 5.0f, 8000.0f);
    assertTrue(!sys.beginExtraction("ruin_1", "mod_1"), "Can't extract undiscovered");
    sys.beginScan("ruin_1", "mod_1");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f); // Complete scan
    assertTrue(sys.beginExtraction("ruin_1", "mod_1"), "Extraction started");
    for (int i = 0; i < 10; i++) sys.update(1.0f); // Complete extraction (5.0s required)
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 4, "Module extracted");
    assertTrue(sys.getExtractedCount("ruin_1") == 1, "1 extracted");
}

void testAncientDiscoveryAnalyze() {
    std::cout << "\n=== AncientModuleDiscovery: Analyze ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "nav_system", 0.3f, 3.0f, 2000.0f);
    sys.beginScan("ruin_1", "mod_1");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    sys.beginExtraction("ruin_1", "mod_1");
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.analyzeModule("ruin_1", "mod_1"), "Module analyzed");
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 5, "Module in analyzed state");
    assertTrue(!sys.analyzeModule("ruin_1", "mod_1"), "Can't re-analyze");
}

void testAncientDiscoveryMultiple() {
    std::cout << "\n=== AncientModuleDiscovery: Multiple Modules ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "shield_gen", 0.5f, 5.0f, 3000.0f);
    sys.addHiddenModule("ruin_1", "mod_2", "power_core", 0.6f, 8.0f, 5000.0f);
    sys.addHiddenModule("ruin_1", "mod_3", "nav_comp", 0.4f, 4.0f, 2000.0f);
    assertTrue(sys.getTotalModules("ruin_1") == 3, "3 modules total");
    sys.beginScan("ruin_1", "mod_1");
    sys.beginScan("ruin_1", "mod_2");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.getDiscoveredCount("ruin_1") == 2, "2 discovered");
}

void testAncientDiscoveryMaxModules() {
    std::cout << "\n=== AncientModuleDiscovery: Max Modules ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    for (int i = 0; i < 10; i++) {
        std::string id = "mod_" + std::to_string(i);
        assertTrue(sys.addHiddenModule("ruin_1", id, "tech", 0.5f, 5.0f, 1000.0f),
            ("Module " + std::to_string(i) + " added").c_str());
    }
    assertTrue(!sys.addHiddenModule("ruin_1", "mod_extra", "tech", 0.5f, 5.0f, 1000.0f),
        "11th module rejected (max 10)");
}

void testAncientDiscoveryActive() {
    std::cout << "\n=== AncientModuleDiscovery: Active State ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    world.createEntity("ruin_1");
    sys.initializeSite("ruin_1", "site_01", "player_1");
    sys.addHiddenModule("ruin_1", "mod_1", "shield_gen", 0.5f, 5.0f, 3000.0f);
    sys.beginScan("ruin_1", "mod_1");
    sys.update(2.0f); // Not active, shouldn't advance
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 1, "Still scanning when inactive");
    sys.setActive("ruin_1", true);
    for (int i = 0; i < 3; i++) sys.update(1.0f);
    assertTrue(sys.getModuleState("ruin_1", "mod_1") == 2, "Discovered when active");
}

void testAncientDiscoveryStateNames() {
    std::cout << "\n=== AncientModuleDiscovery: State Names ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    assertTrue(sys.stateName(0) == "undiscovered", "State 0 name");
    assertTrue(sys.stateName(1) == "scanning", "State 1 name");
    assertTrue(sys.stateName(2) == "discovered", "State 2 name");
    assertTrue(sys.stateName(5) == "analyzed", "State 5 name");
}

void testAncientDiscoveryMissing() {
    std::cout << "\n=== AncientModuleDiscovery: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::AncientModuleDiscoverySystem sys(&world);
    assertTrue(!sys.initializeSite("nonexistent", "s", "p"), "Init fails on missing");
    assertTrue(!sys.addHiddenModule("nonexistent", "m", "t", 0.5f, 5.0f, 100.0f), "Add fails on missing");
    assertTrue(sys.getDiscoveredCount("nonexistent") == 0, "0 discovered on missing");
    assertTrue(sys.getTotalModules("nonexistent") == 0, "0 modules on missing");
}


void run_fps_tests() {
    testInteriorExteriorDefaults();
    testInteriorExteriorAddEffect();
    testInteriorExteriorClear();
    testFPSSpawnCreate();
    testFPSSpawnTransform();
    testFPSSpawnContext();
    testFPSSpawnActivation();
    testFPSSpawnFindForDockedPlayer();
    testFPSSpawnFindForDockedPlayerNoHangar();
    testFPSSpawnContextNames();
    testFPSSpawnComponentDefaults();
    testFPSCharControllerSpawn();
    testFPSCharControllerMovement();
    testFPSCharControllerSprint();
    testFPSCharControllerCrouch();
    testFPSCharControllerJump();
    testFPSCharControllerGravityLanding();
    testFPSCharControllerZeroG();
    testFPSCharControllerStaminaDrain();
    testFPSCharControllerStaminaExhaustion();
    testFPSCharControllerLookDirection();
    testFPSCharControllerRemove();
    testFPSCharControllerStanceNames();
    testFPSCharControllerComponentDefaults();
    testInteriorDoorCreate();
    testInteriorDoorOpenClose();
    testInteriorDoorAutoClose();
    testInteriorDoorLock();
    testInteriorDoorAirlockPressure();
    testInteriorDoorSecurityAccess();
    testInteriorDoorStateNames();
    testInteriorDoorTypeNames();
    testInteriorDoorComponentDefaults();
    testEVAAirlockCreate();
    testEVAAirlockBeginEVA();
    testEVAAirlockFullSequence();
    testEVAAirlockReentry();
    testEVAAirlockAbort();
    testEVAAirlockReentryWrongPlayer();
    testEVAAirlockPhaseNames();
    testEVAAirlockComponentDefaults();
    testFPSInteractionCreate();
    testFPSInteractionFindNearest();
    testFPSInteractionOutOfRange();
    testFPSInteractionDoorToggle();
    testFPSInteractionAirlock();
    testFPSInteractionMedicalBay();
    testFPSInteractionAccessControl();
    testFPSInteractionEnable();
    testFPSInteractionTypeNames();
    testFPSInteractionComponentDefaults();
    testFPSInteractionDifferentInteriors();
    testInteriorLayoutGenerateFrigate();
    testInteriorLayoutGenerateBattleship();
    testInteriorLayoutGenerateCapital();
    testInteriorLayoutDuplicate();
    testInteriorLayoutFindBridge();
    testInteriorLayoutFindEngineering();
    testInteriorLayoutConnections();
    testInteriorLayoutAdjacentRooms();
    testInteriorLayoutUnknownShip();
    testInteriorLayoutRoomTypeName();
    testInteriorLayoutAirlockExteriorConnection();
    testInteriorLayoutComponentDefaults();
    testEnvHazardCreate();
    testEnvHazardRoomSafety();
    testEnvHazardDPS();
    testEnvHazardDPSSeverity();
    testEnvHazardMultipleInRoom();
    testEnvHazardRepair();
    testEnvHazardStopRepair();
    testEnvHazardRemove();
    testEnvHazardSpreadTimer();
    testEnvHazardActiveList();
    testEnvHazardTypeName();
    testEnvHazardComponentDefaults();
    testEnvHazardRoomLevelDamage();
    testEnvHazardNoRoomNoDamage();
    testEnvHazardRoomDamageShieldCascade();
    testFPSCharControllerCurrentRoom();
    testFPSObjectiveCreate();
    testFPSObjectiveActivate();
    testFPSObjectiveEliminateHostiles();
    testFPSObjectiveRetrieveItem();
    testFPSObjectiveDefendPoint();
    testFPSObjectiveSabotage();
    testFPSObjectiveEscape();
    testFPSObjectiveRescueVIP();
    testFPSObjectiveRepairSystem();
    testFPSObjectiveTimeLimit();
    testFPSObjectiveFailManual();
    testFPSObjectivePlayerList();
    testFPSObjectiveWrongType();
    testFPSObjectiveTypeName();
    testFPSObjectiveComponentDefaults();
    testRoverInteriorInit();
    testRoverInteriorAddRoom();
    testRoverInteriorRigLocker();
    testRoverInteriorEquipment();
    testRoverInteriorPressurization();
    testRoverInteriorVolume();
    testRoverInteriorMaxRooms();
    testRoverInteriorRemoveRoom();
    testRoverInteriorMissing();
    testBikeGarageInit();
    testBikeGarageStore();
    testBikeGarageRetrieve();
    testBikeGarageFull();
    testBikeGarageLock();
    testBikeGarageFuel();
    testBikeGarageHull();
    testBikeGarageDoor();
    testBikeGaragePower();
    testBikeGarageMissing();
    testTraversalInit();
    testTraversalSetDestination();
    testTraversalMovement();
    testTraversalArrival();
    testTraversalVehicle();
    testTraversalDismount();
    testTraversalTerrain();
    testTraversalDistance();
    testTraversalClear();
    testTraversalMissing();
    testRoverBayInit();
    testRoverBayOpen();
    testRoverBayClose();
    testRoverBayStore();
    testRoverBayDeploy();
    testRoverBayRetrieve();
    testRoverBaySafety();
    testRoverBayPower();
    testRoverBayAtmosphere();
    testRoverBayMissing();
    testLavatoryInit();
    testLavatoryBeginInteraction();
    testLavatoryDoorTransition();
    testLavatoryThirdPerson();
    testLavatoryFullCycle();
    testLavatoryCancel();
    testLavatoryAudio();
    testLavatoryHygiene();
    testLavatoryPhaseNames();
    testLavatoryMissing();
    testEVAExitInit();
    testEVAExitRequest();
    testEVAExitDockedBlocked();
    testEVAExitLowOxygen();
    testEVAExitFullCycle();
    testEVAExitTether();
    testEVAExitReturn();
    testEVAExitCancel();
    testEVAExitStateNames();
    testEVAExitMissing();
    testAncientDiscoveryInit();
    testAncientDiscoveryAddModule();
    testAncientDiscoveryScan();
    testAncientDiscoveryExtract();
    testAncientDiscoveryAnalyze();
    testAncientDiscoveryMultiple();
    testAncientDiscoveryMaxModules();
    testAncientDiscoveryActive();
    testAncientDiscoveryStateNames();
    testAncientDiscoveryMissing();
}
