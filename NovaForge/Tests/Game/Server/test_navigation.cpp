/**
 * Navigation tests
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

// ==================== WarpAnomalySystem Tests ====================

void testWarpAnomalyNoneIfNotCruising() {
    std::cout << "\n=== Warp Anomaly None If Not Cruising ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Align;
    warp->warp_time = 5.0f;
    // tryTriggerAnomaly checks warp_time < 20, not phase; update() checks phase
    // With short warp_time and non-cruise phase, no anomaly via update
    sys.update(1.0f);
    assertTrue(sys.getAnomalyCount("ship1") == 0, "No anomaly when not in Cruise phase");
}

void testWarpAnomalyNoneIfShortWarp() {
    std::cout << "\n=== Warp Anomaly None If Short Warp ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    warp->warp_time = 5.0f;
    bool triggered = sys.tryTriggerAnomaly("ship1");
    assertTrue(!triggered, "No anomaly when warp_time < 20");
}

void testWarpAnomalyTriggersOnLongWarp() {
    std::cout << "\n=== Warp Anomaly Triggers On Long Warp ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    // Try many different warp_time values to find one that triggers
    bool any_triggered = false;
    for (int i = 20; i < 300; i++) {
        warp->warp_time = static_cast<float>(i);
        if (sys.tryTriggerAnomaly("ship1")) {
            any_triggered = true;
            break;
        }
    }
    assertTrue(any_triggered, "At least one anomaly triggered on long warp");
}

void testWarpAnomalyCount() {
    std::cout << "\n=== Warp Anomaly Count ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    int triggered_count = 0;
    for (int i = 20; i < 500; i++) {
        warp->warp_time = static_cast<float>(i);
        if (sys.tryTriggerAnomaly("ship1")) {
            triggered_count++;
        }
    }
    assertTrue(sys.getAnomalyCount("ship1") == triggered_count,
               "getAnomalyCount matches triggered count");
}

void testWarpAnomalyClear() {
    std::cout << "\n=== Warp Anomaly Clear ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    for (int i = 20; i < 300; i++) {
        warp->warp_time = static_cast<float>(i);
        if (sys.tryTriggerAnomaly("ship1")) break;
    }
    sys.clearAnomaly("ship1");
    auto cleared = sys.getLastAnomaly("ship1");
    assertTrue(cleared.name.empty(), "Anomaly cleared successfully");
}

// ==================== Warp State Phase Tests ====================

void testWarpStatePhaseAlign() {
    std::cout << "\n=== Warp State Phase Align ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp initiated");
    assertTrue(warpState->phase == components::WarpState::WarpPhase::Align, "Initial phase is Align");
}

void testWarpStatePhaseCruise() {
    std::cout << "\n=== Warp State Phase Cruise ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Warp duration ~2.5s at 200km with no Ship component (align_time=2.5 + ~0 travel)
    // Progress rate = 1/2.5 = 0.4/s; Cruise phase at progress 0.2-0.85
    // Need 0.5s+ elapsed for progress >= 0.2 (entry to cruise)
    for (int i = 0; i < 8; i++) {
        moveSys.update(0.1f);  // 0.8 seconds total, progress ~0.32
    }

    assertTrue(warpState->phase == components::WarpState::WarpPhase::Cruise, "Phase is Cruise at 30% progress");
    assertTrue(warpState->warp_time > 0.0f, "Warp time is tracking");
}

void testWarpStatePhaseExit() {
    std::cout << "\n=== Warp State Phase Exit ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Warp duration ~2.5s; Exit phase at progress > 0.85 = 2.125s elapsed
    // 22 updates of 0.1s = 2.2s, progress ~0.88 (in exit phase)
    for (int i = 0; i < 22; i++) {
        moveSys.update(0.1f);  // 2.2 seconds total
    }

    assertTrue(warpState->phase == components::WarpState::WarpPhase::Exit, "Phase is Exit near completion");
}

void testWarpStateResetOnArrival() {
    std::cout << "\n=== Warp State Reset On Arrival ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Complete the warp (10+ seconds at 0.1 progress/s)
    for (int i = 0; i < 110; i++) {
        moveSys.update(0.1f);  // 11.0 seconds total
    }

    assertTrue(warpState->phase == components::WarpState::WarpPhase::None, "Phase reset to None after arrival");
    assertTrue(approxEqual(warpState->warp_time, 0.0f), "Warp time reset");
    assertTrue(approxEqual(pos->x, 200000.0f), "Ship arrived at destination X");
}

void testWarpStateIntensity() {
    std::cout << "\n=== Warp State Intensity ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    warpState->mass_norm = 0.5f;  // medium-mass ship

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Advance a few ticks
    moveSys.update(1.0f);

    assertTrue(warpState->intensity > 0.0f, "Intensity increases during warp");
    assertTrue(warpState->intensity <= 1.0f, "Intensity clamped to max 1.0");
}

// ── Warp Disruption Tests ──────────────────────────────────────────

void testWarpDisruptionPreventsWarp() {
    std::cout << "\n=== Warp Disruption Prevents Warp ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_strength = 1;  // default warp core strength

    // Apply disruption equal to warp strength
    warpState->warp_disrupt_strength = 1;

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(!warped, "Warp rejected when disrupted (strength 1 >= warp core 1)");
}

void testWarpDisruptionInsufficientStrength() {
    std::cout << "\n=== Warp Disruption Insufficient Strength ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_strength = 3;  // high warp core strength (e.g., warp core stabilizer fitted)

    // Apply disruption less than warp strength
    warpState->warp_disrupt_strength = 2;

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp succeeds when disruption (2) < warp core strength (3)");
}

void testIsWarpDisruptedQuery() {
    std::cout << "\n=== Is Warp Disrupted Query ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_strength = 1;

    warpState->warp_disrupt_strength = 0;
    assertTrue(!moveSys.isWarpDisrupted("ship1"), "Not disrupted with 0 disrupt strength");

    warpState->warp_disrupt_strength = 1;
    assertTrue(moveSys.isWarpDisrupted("ship1"), "Disrupted when disrupt strength >= warp core");

    warpState->warp_disrupt_strength = 3;
    assertTrue(moveSys.isWarpDisrupted("ship1"), "Disrupted when disrupt strength exceeds warp core");
}

// ── Ship-Class Warp Speed Tests ────────────────────────────────────

void testWarpSpeedFromShipComponent() {
    std::cout << "\n=== Warp Speed From Ship Component ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_speed_au = 5.0f;   // frigate
    shipComp->align_time = 2.5f;

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp initiated with frigate speed");
    assertTrue(warpState->warp_speed == 5.0f, "WarpState.warp_speed set from Ship component");
}

void testBattleshipSlowerWarp() {
    std::cout << "\n=== Battleship Slower Warp ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    // Frigate ship
    auto* frig = world.createEntity("frigate1");
    auto* fpos = addComp<components::Position>(frig);
    fpos->x = 0.0f; fpos->y = 0.0f; fpos->z = 0.0f;
    addComp<components::Velocity>(frig);
    auto* fws = addComp<components::WarpState>(frig);
    auto* fs = addComp<components::Ship>(frig);
    fs->ship_class = "Frigate";
    fs->warp_speed_au = 5.0f;
    fs->align_time = 2.5f;

    // Battleship
    auto* bs = world.createEntity("bs1");
    auto* bpos = addComp<components::Position>(bs);
    bpos->x = 0.0f; bpos->y = 0.0f; bpos->z = 0.0f;
    addComp<components::Velocity>(bs);
    auto* bws = addComp<components::WarpState>(bs);
    auto* bsc = addComp<components::Ship>(bs);
    bsc->ship_class = "Battleship";
    bsc->warp_speed_au = 2.0f;
    bsc->align_time = 10.0f;

    moveSys.commandWarp("frigate1", 200000.0f, 0.0f, 0.0f);
    moveSys.commandWarp("bs1", 200000.0f, 0.0f, 0.0f);

    // Advance enough for frigate to complete warp (align_time=2.5s + ~0 travel ≈ 2.5s)
    for (int i = 0; i < 30; i++) {
        moveSys.update(0.1f);  // 3.0 seconds total
    }

    // Frigate should have arrived (warp_duration ≈ 2.5s, 3.0s > 2.5s)
    assertTrue(fws->phase == components::WarpState::WarpPhase::None,
               "Frigate arrived (faster warp)");

    // Battleship should still be in warp (warp_duration ≈ 10.0s, 3.0s < 10.0s)
    assertTrue(bws->phase != components::WarpState::WarpPhase::None,
               "Battleship still warping (slower warp)");
}

void testWarpNoDisruptionWithoutWarpState() {
    std::cout << "\n=== Warp No Disruption Without WarpState ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    // No WarpState component - disruption check should return false

    assertTrue(!moveSys.isWarpDisrupted("ship1"), "No disruption without WarpState component");
}

// ── AI Dynamic Orbit Tests ──────────────────────────────────────────

void testAIDynamicOrbitFrigate() {
    std::cout << "\n=== AI Dynamic Orbit Frigate ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("Frigate");
    assertTrue(dist == 5000.0f, "Frigate orbit distance is 5000m");

    float dist2 = systems::AISystem::orbitDistanceForClass("Destroyer");
    assertTrue(dist2 == 5000.0f, "Destroyer orbit distance is 5000m");
}

void testAIDynamicOrbitCruiser() {
    std::cout << "\n=== AI Dynamic Orbit Cruiser ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("Cruiser");
    assertTrue(dist == 15000.0f, "Cruiser orbit distance is 15000m");

    float dist2 = systems::AISystem::orbitDistanceForClass("Battlecruiser");
    assertTrue(dist2 == 15000.0f, "Battlecruiser orbit distance is 15000m");
}

void testAIDynamicOrbitBattleship() {
    std::cout << "\n=== AI Dynamic Orbit Battleship ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("Battleship");
    assertTrue(dist == 30000.0f, "Battleship orbit distance is 30000m");
}

void testAIDynamicOrbitCapital() {
    std::cout << "\n=== AI Dynamic Orbit Capital ===" << std::endl;

    float distCarrier = systems::AISystem::orbitDistanceForClass("Carrier");
    assertTrue(distCarrier == 50000.0f, "Carrier orbit distance is 50000m");

    float distTitan = systems::AISystem::orbitDistanceForClass("Titan");
    assertTrue(distTitan == 50000.0f, "Titan orbit distance is 50000m");
}

void testAIDynamicOrbitUnknown() {
    std::cout << "\n=== AI Dynamic Orbit Unknown ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("SomeUnknownClass");
    assertTrue(dist == 10000.0f, "Unknown class orbit distance is 10000m default");
}

void testAIEngagementRangeFromWeapon() {
    std::cout << "\n=== AI Engagement Range From Weapon ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("npc_test");
    auto* weapon = addComp<components::Weapon>(npc);
    weapon->optimal_range = 5000.0f;
    weapon->falloff_range = 2500.0f;

    float range = systems::AISystem::engagementRangeFromWeapon(npc);
    assertTrue(range == 7500.0f, "Engagement range is optimal + falloff");
}

void testAIEngagementRangeNoWeapon() {
    std::cout << "\n=== AI Engagement Range No Weapon ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("npc_no_weapon");

    float range = systems::AISystem::engagementRangeFromWeapon(npc);
    assertTrue(range == 0.0f, "No weapon returns 0 engagement range");
}

void testAITargetSelectionClosest() {
    std::cout << "\n=== AI Target Selection Closest ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create NPC
    auto* npc = world.createEntity("npc_selector");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->target_selection = components::AI::TargetSelection::Closest;
    ai->awareness_range = 50000.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(npc);

    // Create two players at different distances
    auto* far_player = world.createEntity("player_far");
    auto* far_pos = addComp<components::Position>(far_player);
    far_pos->x = 10000.0f;
    addComp<components::Player>(far_player);

    auto* near_player = world.createEntity("player_near");
    auto* near_pos = addComp<components::Position>(near_player);
    near_pos->x = 3000.0f;
    addComp<components::Player>(near_player);

    // Run AI - should pick nearest
    aiSys.update(1.0f);
    assertTrue(ai->target_entity_id == "player_near",
               "Closest selection picks nearest player");
}

void testAITargetSelectionLowestHP() {
    std::cout << "\n=== AI Target Selection Lowest HP ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc_hp_select");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->target_selection = components::AI::TargetSelection::LowestHP;
    ai->awareness_range = 50000.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f;
    addComp<components::Velocity>(npc);

    // Player at 5000m with full HP
    auto* full_hp = world.createEntity("player_full");
    auto* fpos = addComp<components::Position>(full_hp);
    fpos->x = 5000.0f;
    addComp<components::Player>(full_hp);
    auto* fh = addComp<components::Health>(full_hp);
    fh->shield_hp = 100.0f; fh->shield_max = 100.0f;
    fh->armor_hp = 100.0f; fh->armor_max = 100.0f;
    fh->hull_hp = 100.0f; fh->hull_max = 100.0f;

    // Player at 3000m with low HP
    auto* low_hp = world.createEntity("player_low");
    auto* lpos = addComp<components::Position>(low_hp);
    lpos->x = 3000.0f;
    addComp<components::Player>(low_hp);
    auto* lh = addComp<components::Health>(low_hp);
    lh->shield_hp = 0.0f; lh->shield_max = 100.0f;
    lh->armor_hp = 10.0f; lh->armor_max = 100.0f;
    lh->hull_hp = 50.0f; lh->hull_max = 100.0f;

    aiSys.update(1.0f);
    assertTrue(ai->target_entity_id == "player_low",
               "LowestHP selection picks damaged player");
}

void testAIDynamicOrbitApplied() {
    std::cout << "\n=== AI Dynamic Orbit Applied ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc_orbit_test");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->use_dynamic_orbit = true;
    ai->orbit_distance = 1000.0f;  // default, should be overridden
    ai->awareness_range = 50000.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f;
    addComp<components::Velocity>(npc);
    auto* ship = addComp<components::Ship>(npc);
    ship->ship_class = "Battleship";

    // Create a player in range
    auto* player = world.createEntity("player_orb");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 40000.0f;
    addComp<components::Player>(player);

    aiSys.update(1.0f);
    assertTrue(ai->orbit_distance == 30000.0f,
               "Dynamic orbit updates to battleship distance (30000m)");
}

// ── Protocol Message Tests ──────────────────────────────────────────

void testProtocolDockMessages() {
    std::cout << "\n=== Protocol Dock Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Dock success
    std::string dockSuccess = proto.createDockSuccess("station_01");
    assertTrue(dockSuccess.find("dock_success") != std::string::npos,
               "Dock success contains message type");
    assertTrue(dockSuccess.find("station_01") != std::string::npos,
               "Dock success contains station ID");

    // Dock failed
    std::string dockFailed = proto.createDockFailed("out of range");
    assertTrue(dockFailed.find("dock_failed") != std::string::npos,
               "Dock failed contains message type");
    assertTrue(dockFailed.find("out of range") != std::string::npos,
               "Dock failed contains reason");
}

void testProtocolUndockMessage() {
    std::cout << "\n=== Protocol Undock Message ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string undock = proto.createUndockSuccess();
    assertTrue(undock.find("undock_success") != std::string::npos,
               "Undock success contains message type");
}

void testProtocolRepairMessage() {
    std::cout << "\n=== Protocol Repair Message ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string repair = proto.createRepairResult(5000.0f, 100.0f, 100.0f, 100.0f);
    assertTrue(repair.find("repair_result") != std::string::npos,
               "Repair result contains message type");
    assertTrue(repair.find("5000") != std::string::npos,
               "Repair result contains cost");
}

void testProtocolDamageEventMessage() {
    std::cout << "\n=== Protocol Damage Event Message ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string dmg = proto.createDamageEvent("target_01", 150.0f, "kinetic",
                                               "shield", true, false, false);
    assertTrue(dmg.find("damage_event") != std::string::npos,
               "Damage event contains message type");
    assertTrue(dmg.find("target_01") != std::string::npos,
               "Damage event contains target ID");
    assertTrue(dmg.find("shield") != std::string::npos,
               "Damage event contains layer_hit");
    assertTrue(dmg.find("\"shield_depleted\":true") != std::string::npos,
               "Damage event has shield_depleted flag");
}

void testProtocolDockRequestParse() {
    std::cout << "\n=== Protocol Dock Request Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"dock_request\",\"data\":{\"station_id\":\"s1\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Dock request parses successfully");
    assertTrue(type == atlas::network::MessageType::DOCK_REQUEST, "Parsed type is DOCK_REQUEST");
}

void testProtocolWarpMessages() {
    std::cout << "\n=== Protocol Warp Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse warp_request message
    std::string msg = "{\"message_type\":\"warp_request\",\"data\":{\"dest_x\":1000,\"dest_y\":0,\"dest_z\":5000}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Warp request parses successfully");
    assertTrue(type == atlas::network::MessageType::WARP_REQUEST, "Parsed type is WARP_REQUEST");

    // Warp result creation
    std::string result = proto.createWarpResult(true);
    assertTrue(result.find("warp_result") != std::string::npos, "Warp result contains message type");
    assertTrue(result.find("true") != std::string::npos, "Warp result contains success");

    // Warp result with failure reason
    std::string fail = proto.createWarpResult(false, "Warp drive disrupted");
    assertTrue(fail.find("false") != std::string::npos, "Warp fail contains false");
    assertTrue(fail.find("Warp drive disrupted") != std::string::npos, "Warp fail contains reason");
}

void testProtocolMovementMessages() {
    std::cout << "\n=== Protocol Movement Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse approach message
    std::string msg = "{\"message_type\":\"approach\",\"data\":{\"target_id\":\"npc_1\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Approach message parses successfully");
    assertTrue(type == atlas::network::MessageType::APPROACH, "Parsed type is APPROACH");

    // Parse orbit message
    msg = "{\"message_type\":\"orbit\",\"data\":{\"target_id\":\"npc_1\",\"distance\":5000}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Orbit message parses successfully");
    assertTrue(type == atlas::network::MessageType::ORBIT, "Parsed type is ORBIT");

    // Parse stop message
    msg = "{\"message_type\":\"stop\",\"data\":{}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Stop message parses successfully");
    assertTrue(type == atlas::network::MessageType::STOP, "Parsed type is STOP");

    // Movement acknowledgement creation
    std::string ack = proto.createMovementAck("approach", true);
    assertTrue(ack.find("approach") != std::string::npos, "Movement ack contains command");
    assertTrue(ack.find("true") != std::string::npos, "Movement ack contains success");
}

// ==================== Warp Cinematic System Tests ====================

void testWarpCinematicCompositeIntensityFrigate() {
    std::cout << "\n=== Warp Cinematic: Composite Intensity Frigate ===" << std::endl;
    // Frigate mass_norm=0, cruise phase_frac=0.85
    float ci = systems::WarpCinematicSystem::computeCompositeIntensity(0.0f, 0.85f);
    assertTrue(ci > 0.8f && ci <= 1.0f, "Frigate cruise composite ~ 0.85");
}

void testWarpCinematicCompositeIntensityCapital() {
    std::cout << "\n=== Warp Cinematic: Composite Intensity Capital ===" << std::endl;
    // Capital mass_norm=1.0, cruise phase_frac=0.85
    float ci = systems::WarpCinematicSystem::computeCompositeIntensity(1.0f, 0.85f);
    assertTrue(ci > 0.85f, "Capital cruise composite > frigate");
    assertTrue(ci <= 1.0f, "Composite clamped to 1.0");
}

void testWarpCinematicCompositeIntensityNone() {
    std::cout << "\n=== Warp Cinematic: Composite Intensity None Phase ===" << std::endl;
    float ci = systems::WarpCinematicSystem::computeCompositeIntensity(0.5f, 0.0f);
    assertTrue(approxEqual(ci, 0.0f), "No warp phase => zero composite");
}

void testWarpCinematicLayersFrigate() {
    std::cout << "\n=== Warp Cinematic: Layers Frigate ===" << std::endl;
    float radial, bloom, skin, vignette;
    systems::WarpCinematicSystem::computeLayers(0.85f, 0.0f, radial, bloom, skin, vignette);
    assertTrue(radial > 0.0f, "Radial distortion > 0");
    assertTrue(bloom > radial, "Bloom > radial for frigate");
    assertTrue(skin > 0.0f, "Tunnel skin > 0");
    assertTrue(vignette > 0.0f, "Vignette > 0");
}

void testWarpCinematicLayersCapitalMoreDistortion() {
    std::cout << "\n=== Warp Cinematic: Capital has more radial distortion ===" << std::endl;
    float r_frig, b_f, s_f, v_f;
    float r_cap, b_c, s_c, v_c;
    systems::WarpCinematicSystem::computeLayers(0.85f, 0.0f, r_frig, b_f, s_f, v_f);
    systems::WarpCinematicSystem::computeLayers(0.85f, 1.0f, r_cap, b_c, s_c, v_c);
    assertTrue(r_cap > r_frig, "Capital radial > frigate radial at same composite");
    assertTrue(s_c > s_f, "Capital tunnel skin > frigate at same composite");
}

void testWarpCinematicAudioFrigate() {
    std::cout << "\n=== Warp Cinematic: Audio Frigate ===" << std::endl;
    float ev, hv, sv, ep, hp;
    systems::WarpCinematicSystem::computeAudio(0.85f, 0.0f, ev, hv, sv, ep, hp);
    assertTrue(ev > 0.0f, "Engine volume > 0");
    assertTrue(hv > 0.0f, "Harmonics volume > 0");
    assertTrue(sv > 0.0f, "Shimmer volume > 0");
    assertTrue(approxEqual(ep, 1.0f), "Frigate engine pitch = 1.0");
    assertTrue(approxEqual(hp, 1.0f), "Frigate harmonics pitch = 1.0");
}

void testWarpCinematicAudioCapitalLowerPitch() {
    std::cout << "\n=== Warp Cinematic: Capital lower pitch ===" << std::endl;
    float ev, hv, sv, ep, hp;
    systems::WarpCinematicSystem::computeAudio(0.85f, 1.0f, ev, hv, sv, ep, hp);
    assertTrue(ep < 1.0f, "Capital engine pitch < 1.0");
    assertTrue(hp < 1.0f, "Capital harmonics pitch < 1.0");
    assertTrue(ev > 0.0f, "Capital engine volume > 0");
}

void testWarpCinematicSystemUpdate() {
    std::cout << "\n=== Warp Cinematic: System update writes tunnel config ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    auto* ship = world.createEntity("ship1");
    auto* ws = addComp<components::WarpState>(ship);
    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->mass_norm = 0.3f;

    auto* tc = addComp<components::WarpTunnelConfig>(ship);
    auto* ap = addComp<components::WarpAudioProfile>(ship);

    sys.update(0.1f);

    assertTrue(tc->composite_intensity > 0.0f, "Composite intensity set after update");
    assertTrue(tc->radial_distortion > 0.0f, "Radial distortion set");
    assertTrue(tc->starfield_bloom > 0.0f, "Starfield bloom set");
    assertTrue(ap->engine_core_volume > 0.0f, "Engine audio volume set");
}

void testWarpCinematicAccessibilityReducesMotion() {
    std::cout << "\n=== Warp Cinematic: Accessibility reduces motion ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    auto* ship = world.createEntity("ship1");
    auto* ws = addComp<components::WarpState>(ship);
    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->mass_norm = 0.5f;
    auto* tc = addComp<components::WarpTunnelConfig>(ship);
    auto* ap = addComp<components::WarpAudioProfile>(ship);
    auto* acc = addComp<components::WarpAccessibility>(ship);
    acc->motion_intensity = 0.5f;
    acc->bass_intensity = 0.5f;
    acc->blur_intensity = 0.5f;

    sys.update(0.1f);

    // Compare with a second entity without accessibility scaling
    auto* ship2 = world.createEntity("ship2");
    auto* ws2 = addComp<components::WarpState>(ship2);
    ws2->phase = components::WarpState::WarpPhase::Cruise;
    ws2->mass_norm = 0.5f;
    auto* tc2 = addComp<components::WarpTunnelConfig>(ship2);
    auto* ap2 = addComp<components::WarpAudioProfile>(ship2);

    sys.update(0.1f);

    assertTrue(tc->starfield_bloom < tc2->starfield_bloom, "Accessibility reduces bloom");
    assertTrue(tc->radial_distortion < tc2->radial_distortion, "Accessibility reduces distortion");
    assertTrue(ap->engine_core_volume < ap2->engine_core_volume, "Accessibility reduces bass");
}

void testWarpCinematicNonePhaseZeroIntensity() {
    std::cout << "\n=== Warp Cinematic: None phase => zero intensity ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    auto* ship = world.createEntity("ship1");
    auto* ws = addComp<components::WarpState>(ship);
    ws->phase = components::WarpState::WarpPhase::None;
    ws->mass_norm = 0.5f;
    auto* tc = addComp<components::WarpTunnelConfig>(ship);
    auto* ap = addComp<components::WarpAudioProfile>(ship);

    sys.update(0.1f);

    assertTrue(approxEqual(tc->composite_intensity, 0.0f), "None phase => zero composite");
    assertTrue(approxEqual(tc->radial_distortion, 0.0f), "None phase => zero radial");
    assertTrue(approxEqual(ap->engine_core_volume, 0.0f), "None phase => zero engine audio");
}

void testWarpCinematicAlignPhaseSubtle() {
    std::cout << "\n=== Warp Cinematic: Align phase is subtle ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    auto* ship = world.createEntity("ship1");
    auto* ws = addComp<components::WarpState>(ship);
    ws->phase = components::WarpState::WarpPhase::Align;
    ws->mass_norm = 0.0f;
    auto* tc = addComp<components::WarpTunnelConfig>(ship);

    sys.update(0.1f);

    assertTrue(tc->composite_intensity > 0.0f, "Align phase has some effect");
    assertTrue(tc->composite_intensity < 0.2f, "Align phase is subtle (< 0.2)");
}

void testWarpCinematicExitPhaseFades() {
    std::cout << "\n=== Warp Cinematic: Exit phase fades out ===" << std::endl;
    ecs::World world;
    systems::WarpCinematicSystem sys(&world);

    // Cruise entity
    auto* s1 = world.createEntity("s1");
    auto* ws1 = addComp<components::WarpState>(s1);
    ws1->phase = components::WarpState::WarpPhase::Cruise;
    ws1->mass_norm = 0.5f;
    auto* tc1 = addComp<components::WarpTunnelConfig>(s1);

    // Exit entity
    auto* s2 = world.createEntity("s2");
    auto* ws2 = addComp<components::WarpState>(s2);
    ws2->phase = components::WarpState::WarpPhase::Exit;
    ws2->mass_norm = 0.5f;
    auto* tc2 = addComp<components::WarpTunnelConfig>(s2);

    sys.update(0.1f);

    assertTrue(tc2->composite_intensity < tc1->composite_intensity,
               "Exit phase intensity < cruise intensity");
}

// ==================== Anomaly System Tests ====================

void testAnomalyGenerateCreatesEntities() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    int count = sys.generateAnomalies("sys1", 42, 0.5f);
    assertTrue(count > 0, "generateAnomalies returns positive count");

    auto anomalies = sys.getAnomaliesInSystem("sys1");
    assertTrue(static_cast<int>(anomalies.size()) == count,
               "getAnomaliesInSystem matches generated count");
}

void testAnomalyHighsecFewerThanNullsec() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    int highsec = sys.generateAnomalies("high", 42, 1.0f);
    int nullsec = sys.generateAnomalies("null", 42, 0.0f);
    assertTrue(nullsec > highsec,
               "Nullsec generates more anomalies than highsec");
}

void testAnomalyDeterministicSeed() {
    ecs::World world1;
    systems::AnomalySystem sys1(&world1);
    int c1 = sys1.generateAnomalies("sys1", 12345, 0.5f);

    ecs::World world2;
    systems::AnomalySystem sys2(&world2);
    int c2 = sys2.generateAnomalies("sys1", 12345, 0.5f);

    assertTrue(c1 == c2, "Same seed produces same anomaly count");
}

void testAnomalyDifficultyFromSecurity() {
    using D = components::Anomaly::Difficulty;
    assertTrue(systems::AnomalySystem::difficultyFromSecurity(1.0f) == D::Trivial,
               "Highsec = Trivial difficulty");
    assertTrue(systems::AnomalySystem::difficultyFromSecurity(0.5f) == D::Medium,
               "Midsec = Medium difficulty");
    assertTrue(systems::AnomalySystem::difficultyFromSecurity(0.0f) == D::Deadly,
               "Nullsec = Deadly difficulty");
}

void testAnomalyNpcCountScales() {
    using D = components::Anomaly::Difficulty;
    int trivial = systems::AnomalySystem::npcCountFromDifficulty(D::Trivial);
    int deadly  = systems::AnomalySystem::npcCountFromDifficulty(D::Deadly);
    assertTrue(deadly > trivial, "Deadly has more NPCs than Trivial");
}

void testAnomalyLootMultiplierScales() {
    using D = components::Anomaly::Difficulty;
    float trivial = systems::AnomalySystem::lootMultiplierFromDifficulty(D::Trivial);
    float deadly  = systems::AnomalySystem::lootMultiplierFromDifficulty(D::Deadly);
    assertTrue(deadly > trivial, "Deadly has higher loot multiplier");
}

void testAnomalyCompleteAnomaly() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    sys.generateAnomalies("sys1", 42, 0.5f);
    auto anomalies = sys.getAnomaliesInSystem("sys1");
    assertTrue(!anomalies.empty(), "System has anomalies");

    bool completed = sys.completeAnomaly(anomalies[0]);
    assertTrue(completed, "completeAnomaly returns true");

    int after = sys.getActiveAnomalyCount("sys1");
    assertTrue(after == static_cast<int>(anomalies.size()) - 1,
               "Active count decreased by 1");
}

void testAnomalyDespawnOnTimer() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    sys.generateAnomalies("sys1", 42, 0.5f);
    auto anomalies = sys.getAnomaliesInSystem("sys1");
    int before = static_cast<int>(anomalies.size());
    assertTrue(before > 0, "Has anomalies to despawn");

    // Tick past the despawn timer (max is 7200s)
    sys.update(8000.0f);

    int after = sys.getActiveAnomalyCount("sys1");
    assertTrue(after == 0, "All anomalies despawned after timer");
}

void testAnomalySignatureStrength() {
    ecs::World world;
    systems::AnomalySystem sys(&world);

    sys.generateAnomalies("sys1", 42, 0.5f);
    auto anomalies = sys.getAnomaliesInSystem("sys1");
    for (const auto& id : anomalies) {
        auto* entity = world.getEntity(id);
        auto* anom = entity->getComponent<components::Anomaly>();
        assertTrue(anom->signature_strength > 0.0f && anom->signature_strength <= 1.0f,
                   "Signature strength in valid range");
    }
}

// ==================== Scanner System Tests ====================

void testScannerStartScan() {
    ecs::World world;
    systems::ScannerSystem sys(&world);

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    scanner_entity->addComponent(std::move(sc));

    bool started = sys.startScan("scanner1", "sys1");
    assertTrue(started, "startScan returns true");
    assertTrue(sys.getActiveScannerCount() == 1, "One active scanner");
}

void testScannerStopScan() {
    ecs::World world;
    systems::ScannerSystem sys(&world);

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    scanner_entity->addComponent(std::move(sc));

    sys.startScan("scanner1", "sys1");
    bool stopped = sys.stopScan("scanner1");
    assertTrue(stopped, "stopScan returns true");
    assertTrue(sys.getActiveScannerCount() == 0, "No active scanners");
}

void testScannerDetectsAnomaly() {
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys1", 42, 0.8f);  // easy to scan (highsec)

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 100.0f;  // strong scanner
    sc->scan_duration = 1.0f;    // fast scans
    scanner_entity->addComponent(std::move(sc));

    scanSys.startScan("scanner1", "sys1");

    // Run enough scan cycles to detect
    for (int i = 0; i < 5; ++i) {
        scanSys.update(1.1f);  // complete one cycle
    }

    auto results = scanSys.getScanResults("scanner1");
    assertTrue(!results.empty(), "Scanner detected anomalies");
    assertTrue(results[0].signal_strength > 0.0f, "Signal strength is positive");
}

void testScannerSignalAccumulates() {
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys1", 42, 0.8f);

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 80.0f;
    sc->scan_duration = 1.0f;
    scanner_entity->addComponent(std::move(sc));

    scanSys.startScan("scanner1", "sys1");
    scanSys.update(1.1f);  // first cycle

    auto results1 = scanSys.getScanResults("scanner1");
    float sig1 = results1.empty() ? 0.0f : results1[0].signal_strength;

    scanSys.update(1.1f);  // second cycle
    auto results2 = scanSys.getScanResults("scanner1");
    float sig2 = results2.empty() ? 0.0f : results2[0].signal_strength;

    assertTrue(sig2 > sig1, "Signal strength increases with scans");
}

void testScannerEffectiveScanStrength() {
    float s8 = systems::ScannerSystem::effectiveScanStrength(50.0f, 8);
    float s4 = systems::ScannerSystem::effectiveScanStrength(50.0f, 4);
    float s1 = systems::ScannerSystem::effectiveScanStrength(50.0f, 1);

    assertTrue(s8 > s4, "8 probes stronger than 4");
    assertTrue(s4 > s1, "4 probes stronger than 1");
    assertTrue(approxEqual(s8, 50.0f, 0.1f), "8 probes at base 50 = 50 effective");
}

void testScannerSignalGainPerCycle() {
    float gain_strong = systems::ScannerSystem::signalGainPerCycle(100.0f, 1.0f);
    float gain_weak   = systems::ScannerSystem::signalGainPerCycle(20.0f, 0.2f);

    assertTrue(gain_strong > gain_weak,
               "Strong scanner + strong signal > weak scanner + weak signal");
}

void testScannerWarpableAtFullSignal() {
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys1", 42, 0.9f);  // trivial difficulty = strong signal

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 200.0f;   // very strong
    sc->scan_duration = 0.5f;
    scanner_entity->addComponent(std::move(sc));

    scanSys.startScan("scanner1", "sys1");

    // Scan many times to get full signal
    for (int i = 0; i < 20; ++i) {
        scanSys.update(0.6f);
    }

    auto results = scanSys.getScanResults("scanner1");
    bool any_warpable = false;
    for (const auto& r : results) {
        if (r.warpable) any_warpable = true;
    }
    assertTrue(any_warpable, "At least one anomaly is warpable after many scans");
}

void testScannerNoResultWithoutAnomalies() {
    ecs::World world;
    systems::ScannerSystem scanSys(&world);

    auto* scanner_entity = world.createEntity("scanner1");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_duration = 1.0f;
    scanner_entity->addComponent(std::move(sc));

    scanSys.startScan("scanner1", "empty_sys");
    scanSys.update(1.1f);

    auto results = scanSys.getScanResults("scanner1");
    assertTrue(results.empty(), "No results in system without anomalies");
}

// ==================== Scan → Discover → Warp Integration Tests ====================

void testScanDiscoverWarpFlow() {
    std::cout << "\n=== Scan Discover Warp Flow ===" << std::endl;
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    // 1. Generate anomalies in a system
    int created = anomSys.generateAnomalies("sys_flow", 777, 0.3f);  // low-sec = more anomalies
    assertTrue(created > 0, "Anomalies generated for flow test");

    // 2. Create a scanner entity
    auto* scanner = world.createEntity("player_scanner");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 150.0f;
    sc->scan_duration = 0.5f;
    sc->probe_count = 8;
    scanner->addComponent(std::move(sc));

    // 3. Start scanning
    bool started = scanSys.startScan("player_scanner", "sys_flow");
    assertTrue(started, "Scan started successfully");

    // 4. Verify scanner is active
    assertTrue(scanSys.getActiveScannerCount() == 1, "One active scanner");

    // 5. Run enough scan cycles to discover anomalies
    for (int i = 0; i < 30; ++i) {
        scanSys.update(0.6f);
    }

    // 6. Check results - should have discovered at least one anomaly
    auto results = scanSys.getScanResults("player_scanner");
    assertTrue(!results.empty(), "Scanner discovered anomalies");

    // 7. Verify at least one anomaly has signal strength > 0
    bool has_signal = false;
    for (const auto& r : results) {
        if (r.signal_strength > 0.0f) has_signal = true;
    }
    assertTrue(has_signal, "At least one anomaly has signal strength");

    // 8. Check warpable status - strong scanner should get full signal
    bool any_warpable = false;
    for (const auto& r : results) {
        if (r.warpable) any_warpable = true;
    }
    assertTrue(any_warpable, "At least one anomaly is warpable");

    // 9. Get anomaly IDs from results
    std::string warpable_id;
    for (const auto& r : results) {
        if (r.warpable) {
            warpable_id = r.anomaly_id;
            break;
        }
    }
    assertTrue(!warpable_id.empty(), "Warpable anomaly has a valid ID");

    // 10. Verify anomaly still exists in the system
    auto system_anomalies = anomSys.getAnomaliesInSystem("sys_flow");
    bool found = false;
    for (const auto& id : system_anomalies) {
        if (id == warpable_id) found = true;
    }
    assertTrue(found, "Warpable anomaly exists in system anomaly list");
}

void testScanStopPreservesResults() {
    std::cout << "\n=== Scan Stop Preserves Results ===" << std::endl;
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys_stop", 888, 0.5f);

    auto* scanner = world.createEntity("scanner_stop");
    auto sc = std::make_unique<components::Scanner>();
    sc->scan_strength = 100.0f;
    sc->scan_duration = 0.5f;
    scanner->addComponent(std::move(sc));

    scanSys.startScan("scanner_stop", "sys_stop");
    // Run a few cycles
    for (int i = 0; i < 5; ++i) {
        scanSys.update(0.6f);
    }

    auto results_before = scanSys.getScanResults("scanner_stop");
    scanSys.stopScan("scanner_stop");
    auto results_after = scanSys.getScanResults("scanner_stop");

    assertTrue(results_before.size() == results_after.size(),
               "Results preserved after stopping scan");
}

void testScanAnomalyComplete() {
    std::cout << "\n=== Scan Anomaly Complete ===" << std::endl;
    ecs::World world;
    systems::AnomalySystem anomSys(&world);
    systems::ScannerSystem scanSys(&world);

    anomSys.generateAnomalies("sys_complete", 999, 0.8f);

    // Get anomaly count before completion
    int before = anomSys.getActiveAnomalyCount("sys_complete");
    assertTrue(before > 0, "System has active anomalies");

    // Complete the first anomaly
    auto ids = anomSys.getAnomaliesInSystem("sys_complete");
    assertTrue(!ids.empty(), "Anomaly IDs available");
    bool completed = anomSys.completeAnomaly(ids[0]);
    assertTrue(completed, "Anomaly marked as completed");

    // Active count should decrease
    int after = anomSys.getActiveAnomalyCount("sys_complete");
    assertTrue(after < before, "Active anomaly count decreased after completion");
}

void testScanProtocolRoundTrip() {
    std::cout << "\n=== Scan Protocol Round Trip ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Create scan start message and verify it round-trips
    std::string scan_msg = "{\"message_type\":\"scan_start\",\"data\":{\"system_id\":\"sys_rt\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(scan_msg, type, data);
    assertTrue(ok, "Scan start message parses");
    assertTrue(type == atlas::network::MessageType::SCAN_START, "Type is SCAN_START");
    assertTrue(data.find("sys_rt") != std::string::npos, "Data contains system_id");

    // Create scan result and verify it contains all expected fields
    std::string result = proto.createScanResult("sc_rt", 1,
        "[{\"anomaly_id\":\"a_rt\",\"signal_strength\":1.0,\"warpable\":true}]");
    assertTrue(result.find("scan_result") != std::string::npos, "Result type is scan_result");
    assertTrue(result.find("sc_rt") != std::string::npos, "Result contains scanner_id");
    assertTrue(result.find("a_rt") != std::string::npos, "Result contains anomaly_id");
    assertTrue(result.find("warpable") != std::string::npos, "Result contains warpable field");

    // Create anomaly list and verify it round-trips
    std::string anom_list = proto.createAnomalyList("sys_rt", 2, "[\"a1\",\"a2\"]");
    assertTrue(anom_list.find("anomaly_list") != std::string::npos, "List type is anomaly_list");
    assertTrue(anom_list.find("sys_rt") != std::string::npos, "List contains system_id");
}

// ==================== Difficulty Scaling System Tests ====================

void testDifficultyInitializeZone() {
    ecs::World world;
    systems::DifficultyScalingSystem sys(&world);

    auto* sys_entity = world.createEntity("sys1");
    auto dz = std::make_unique<components::DifficultyZone>();
    sys_entity->addComponent(std::move(dz));

    bool ok = sys.initializeZone("sys1", 0.5f);
    assertTrue(ok, "initializeZone returns true");

    auto* zone = sys_entity->getComponent<components::DifficultyZone>();
    assertTrue(approxEqual(zone->security_status, 0.5f, 0.01f),
               "Security status set correctly");
    assertTrue(zone->npc_hp_multiplier > 1.0f,
               "HP multiplier > 1 for midsec");
}

void testDifficultyHighsecLowMultipliers() {
    float hp = systems::DifficultyScalingSystem::hpMultiplierFromSecurity(1.0f);
    float dmg = systems::DifficultyScalingSystem::damageMultiplierFromSecurity(1.0f);
    assertTrue(approxEqual(hp, 1.0f, 0.01f), "Highsec HP multiplier = 1.0");
    assertTrue(approxEqual(dmg, 1.0f, 0.01f), "Highsec damage multiplier = 1.0");
}

void testDifficultyNullsecHighMultipliers() {
    float hp = systems::DifficultyScalingSystem::hpMultiplierFromSecurity(0.0f);
    float dmg = systems::DifficultyScalingSystem::damageMultiplierFromSecurity(0.0f);
    assertTrue(hp > 2.0f, "Nullsec HP multiplier > 2.0");
    assertTrue(dmg > 1.5f, "Nullsec damage multiplier > 1.5");
}

void testDifficultyLootScaling() {
    float highsec = systems::DifficultyScalingSystem::lootMultiplierFromSecurity(1.0f);
    float nullsec = systems::DifficultyScalingSystem::lootMultiplierFromSecurity(0.0f);
    assertTrue(nullsec > highsec, "Nullsec has better loot than highsec");
}

void testDifficultyOreScaling() {
    float highsec = systems::DifficultyScalingSystem::oreMultiplierFromSecurity(1.0f);
    float nullsec = systems::DifficultyScalingSystem::oreMultiplierFromSecurity(0.0f);
    assertTrue(nullsec > highsec, "Nullsec has richer ore than highsec");
}

void testDifficultyMaxTierFromSecurity() {
    int highsec = systems::DifficultyScalingSystem::maxTierFromSecurity(1.0f);
    int nullsec = systems::DifficultyScalingSystem::maxTierFromSecurity(0.0f);
    assertTrue(highsec == 1, "Highsec max tier = 1");
    assertTrue(nullsec == 5, "Nullsec max tier = 5");
}

void testDifficultyApplyToNPC() {
    ecs::World world;
    systems::DifficultyScalingSystem sys(&world);

    auto* sys_entity = world.createEntity("sys1");
    auto dz = std::make_unique<components::DifficultyZone>();
    sys_entity->addComponent(std::move(dz));
    sys.initializeZone("sys1", 0.0f);  // nullsec

    auto* npc = world.createEntity("npc1");
    auto hp = std::make_unique<components::Health>();
    hp->hull_hp = 100.0f;
    hp->hull_max = 100.0f;
    hp->armor_hp = 100.0f;
    hp->armor_max = 100.0f;
    hp->shield_hp = 100.0f;
    hp->shield_max = 100.0f;
    npc->addComponent(std::move(hp));

    auto wpn = std::make_unique<components::Weapon>();
    wpn->damage = 10.0f;
    npc->addComponent(std::move(wpn));

    bool applied = sys.applyToNPC("npc1", "sys1");
    assertTrue(applied, "applyToNPC returns true");

    auto* health = npc->getComponent<components::Health>();
    assertTrue(health->hull_hp > 100.0f, "NPC hull HP scaled up in nullsec");
    assertTrue(health->shield_hp > 100.0f, "NPC shield HP scaled up in nullsec");

    auto* weapon = npc->getComponent<components::Weapon>();
    assertTrue(weapon->damage > 10.0f, "NPC damage scaled up in nullsec");
}

void testDifficultySpawnRateScaling() {
    float highsec = systems::DifficultyScalingSystem::spawnRateFromSecurity(1.0f);
    float nullsec = systems::DifficultyScalingSystem::spawnRateFromSecurity(0.0f);
    assertTrue(approxEqual(highsec, 1.0f, 0.01f), "Highsec spawn rate = 1.0");
    assertTrue(nullsec > highsec, "Nullsec has higher spawn rate");
}

// ==================== LOD Priority Component Tests ====================

void testLODPriorityDefaults() {
    std::cout << "\n=== LOD Priority Defaults ===" << std::endl;
    components::LODPriority lod;
    assertTrue(approxEqual(lod.priority, 1.0f), "Default priority is 1.0");
    assertTrue(!lod.force_visible, "Default force_visible is false");
    assertTrue(approxEqual(lod.impostor_distance, 500.0f), "Default impostor_distance is 500.0");
}

void testLODPriorityPlayerShip() {
    std::cout << "\n=== LOD Priority Player Ship ===" << std::endl;
    ecs::World world;
    auto* ship = world.createEntity("player_ship_lod");
    auto* lod = addComp<components::LODPriority>(ship);
    lod->priority = 2.0f;
    lod->force_visible = true;

    auto* retrieved = ship->getComponent<components::LODPriority>();
    assertTrue(retrieved != nullptr, "LODPriority component retrievable");
    assertTrue(approxEqual(retrieved->priority, 2.0f), "Priority set to 2.0 for player ship");
    assertTrue(retrieved->force_visible, "Player ship forced visible");
}

// ==================== 100+ Ship Fleet Stress Test ====================

void testFleetStress150Ships() {
    std::cout << "\n=== Fleet Stress Test: 150 Ships ===" << std::endl;
    ecs::World world;

    // Create 150 ship entities with Position, Health, Ship, and AI components
    std::vector<ecs::Entity*> ships;
    for (int i = 0; i < 150; ++i) {
        std::string id = "stress_ship_" + std::to_string(i);
        auto* e = world.createEntity(id);
        auto* pos = addComp<components::Position>(e);
        pos->x = static_cast<float>(i * 100);
        pos->y = 0.0f;
        pos->z = static_cast<float>((i % 10) * 50);

        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 500.0f;
        hp->shield_max = 500.0f;
        hp->armor_hp = 300.0f;
        hp->armor_max = 300.0f;
        hp->hull_hp = 200.0f;
        hp->hull_max = 200.0f;

        auto* ship = addComp<components::Ship>(e);
        ship->ship_type = "Frigate";

        auto* ai = addComp<components::AI>(e);
        ai->state = components::AI::State::Idle;

        auto* lod = addComp<components::LODPriority>(e);
        lod->priority = (i < 10) ? 2.0f : 0.5f;  // first 10 ships high priority

        ships.push_back(e);
    }

    assertTrue(ships.size() == 150, "Created 150 ship entities");

    // Verify all entities were created and have components
    int valid = 0;
    for (auto* e : ships) {
        if (e->getComponent<components::Position>() &&
            e->getComponent<components::Health>() &&
            e->getComponent<components::Ship>() &&
            e->getComponent<components::AI>() &&
            e->getComponent<components::LODPriority>()) {
            ++valid;
        }
    }
    assertTrue(valid == 150, "All 150 ships have all required components");

    // Count high-priority vs low-priority
    int highPriority = 0;
    int lowPriority = 0;
    for (auto* e : ships) {
        auto* lod = e->getComponent<components::LODPriority>();
        if (lod->priority >= 1.5f) ++highPriority;
        else ++lowPriority;
    }
    assertTrue(highPriority == 10, "10 ships are high priority");
    assertTrue(lowPriority == 140, "140 ships are low priority");
}

void testFleetStressSystemUpdates() {
    std::cout << "\n=== Fleet Stress Test: System Updates ===" << std::endl;
    ecs::World world;
    systems::ShieldRechargeSystem shieldSys(&world);

    // Create 100 ships with Health components for shield recharge
    for (int i = 0; i < 100; ++i) {
        std::string id = "stress_shield_" + std::to_string(i);
        auto* e = world.createEntity(id);
        auto* hp = addComp<components::Health>(e);
        hp->shield_hp = 250.0f;
        hp->shield_max = 500.0f;
        hp->shield_recharge_rate = 10.0f;
    }

    // Run 10 update ticks
    for (int tick = 0; tick < 10; ++tick) {
        shieldSys.update(1.0f);
    }

    // Verify shields recharged across all entities
    int recharged = 0;
    auto entities = world.getEntities();
    for (auto* e : entities) {
        auto* hp = e->getComponent<components::Health>();
        if (hp && hp->shield_hp > 250.0f) {
            ++recharged;
        }
    }
    assertTrue(recharged == 100, "All 100 ships recharged shields");
}

// ==================== Persistence Round-Trip for New Components ====================

void testPersistenceAnomalyVisualCue() {
    std::cout << "\n=== Persistence: AnomalyVisualCue Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("anom_vis1");
    auto* cue = addComp<components::AnomalyVisualCue>(e);
    cue->anomaly_id = "anom_42";
    cue->cue_type = components::AnomalyVisualCue::CueType::GravityLens;
    cue->intensity = 0.8f;
    cue->radius = 750.0f;
    cue->pulse_frequency = 1.5f;
    cue->r = 0.2f; cue->g = 0.5f; cue->b = 0.9f;
    cue->distortion_strength = 0.7f;
    cue->active = true;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with AnomalyVisualCue");

    auto* e2 = world2.getEntity("anom_vis1");
    assertTrue(e2 != nullptr, "Entity found after deserialization");
    auto* cue2 = e2->getComponent<components::AnomalyVisualCue>();
    assertTrue(cue2 != nullptr, "AnomalyVisualCue component present");
    assertTrue(cue2->anomaly_id == "anom_42", "anomaly_id preserved");
    assertTrue(cue2->cue_type == components::AnomalyVisualCue::CueType::GravityLens, "cue_type preserved");
    assertTrue(approxEqual(cue2->intensity, 0.8f), "intensity preserved");
    assertTrue(approxEqual(cue2->radius, 750.0f), "radius preserved");
    assertTrue(approxEqual(cue2->pulse_frequency, 1.5f), "pulse_frequency preserved");
    assertTrue(approxEqual(cue2->r, 0.2f), "r preserved");
    assertTrue(approxEqual(cue2->g, 0.5f), "g preserved");
    assertTrue(approxEqual(cue2->b, 0.9f), "b preserved");
    assertTrue(approxEqual(cue2->distortion_strength, 0.7f), "distortion_strength preserved");
    assertTrue(cue2->active, "active preserved");
}

void testPersistenceLODPriority() {
    std::cout << "\n=== Persistence: LODPriority Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("lod_ent1");
    auto* lod = addComp<components::LODPriority>(e);
    lod->priority = 2.5f;
    lod->force_visible = true;
    lod->impostor_distance = 800.0f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with LODPriority");

    auto* e2 = world2.getEntity("lod_ent1");
    assertTrue(e2 != nullptr, "Entity found after deserialization");
    auto* lod2 = e2->getComponent<components::LODPriority>();
    assertTrue(lod2 != nullptr, "LODPriority component present");
    assertTrue(approxEqual(lod2->priority, 2.5f), "priority preserved");
    assertTrue(lod2->force_visible, "force_visible preserved");
    assertTrue(approxEqual(lod2->impostor_distance, 800.0f), "impostor_distance preserved");
}

// ==================== Phase 5 Continued: LOD System Tests ====================

void testLODSystemDefaults() {
    std::cout << "\n=== LOD System: Default Thresholds ===" << std::endl;
    ecs::World world;
    systems::LODSystem lodSys(&world);

    assertTrue(approxEqual(lodSys.getNearThreshold(), 5000.0f), "Default near threshold is 5000");
    assertTrue(approxEqual(lodSys.getMidThreshold(), 20000.0f), "Default mid threshold is 20000");
    assertTrue(approxEqual(lodSys.getFarThreshold(), 80000.0f), "Default far threshold is 80000");
}

void testLODSystemPriorityComputation() {
    std::cout << "\n=== LOD System: Priority Computation ===" << std::endl;
    ecs::World world;
    systems::LODSystem lodSys(&world);
    lodSys.setReferencePoint(0.0f, 0.0f, 0.0f);

    // Entity within near threshold (1000m)
    auto* eNear = world.createEntity("lod_near");
    auto* pNear = addComp<components::Position>(eNear);
    pNear->x = 1000.0f; pNear->y = 0.0f; pNear->z = 0.0f;
    addComp<components::LODPriority>(eNear);

    // Entity at mid range (10000m)
    auto* eMid = world.createEntity("lod_mid");
    auto* pMid = addComp<components::Position>(eMid);
    pMid->x = 10000.0f; pMid->y = 0.0f; pMid->z = 0.0f;
    addComp<components::LODPriority>(eMid);

    // Entity at far range (50000m)
    auto* eFar = world.createEntity("lod_far");
    auto* pFar = addComp<components::Position>(eFar);
    pFar->x = 50000.0f; pFar->y = 0.0f; pFar->z = 0.0f;
    addComp<components::LODPriority>(eFar);

    // Entity beyond far (100000m)
    auto* eImp = world.createEntity("lod_impostor");
    auto* pImp = addComp<components::Position>(eImp);
    pImp->x = 100000.0f; pImp->y = 0.0f; pImp->z = 0.0f;
    addComp<components::LODPriority>(eImp);

    lodSys.update(1.0f);

    assertTrue(approxEqual(eNear->getComponent<components::LODPriority>()->priority, 2.0f),
               "Near entity gets full detail priority");
    assertTrue(approxEqual(eMid->getComponent<components::LODPriority>()->priority, 1.0f),
               "Mid entity gets reduced priority");
    assertTrue(approxEqual(eFar->getComponent<components::LODPriority>()->priority, 0.5f),
               "Far entity gets merged priority");
    assertTrue(approxEqual(eImp->getComponent<components::LODPriority>()->priority, 0.1f),
               "Impostor entity gets lowest priority");

    assertTrue(lodSys.getFullDetailCount() == 1, "1 entity at full detail");
    assertTrue(lodSys.getReducedCount() == 1, "1 entity at reduced");
    assertTrue(lodSys.getMergedCount() == 1, "1 entity at merged");
    assertTrue(lodSys.getImpostorCount() == 1, "1 entity at impostor");
}

void testLODSystemForceVisible() {
    std::cout << "\n=== LOD System: Force Visible Override ===" << std::endl;
    ecs::World world;
    systems::LODSystem lodSys(&world);
    lodSys.setReferencePoint(0.0f, 0.0f, 0.0f);

    // Very far entity but force_visible
    auto* e = world.createEntity("lod_forced");
    auto* p = addComp<components::Position>(e);
    p->x = 200000.0f; p->y = 0.0f; p->z = 0.0f;
    auto* lod = addComp<components::LODPriority>(e);
    lod->force_visible = true;

    lodSys.update(1.0f);

    assertTrue(approxEqual(lod->priority, 2.0f),
               "Force-visible entity keeps full detail even at extreme range");
    assertTrue(lodSys.getFullDetailCount() == 1, "Force-visible counted as full detail");
}

void testLODSystemDistanceQuery() {
    std::cout << "\n=== LOD System: Distance Query ===" << std::endl;
    ecs::World world;
    systems::LODSystem lodSys(&world);
    lodSys.setReferencePoint(0.0f, 0.0f, 0.0f);

    auto* e = world.createEntity("lod_dist");
    auto* p = addComp<components::Position>(e);
    p->x = 3000.0f; p->y = 4000.0f; p->z = 0.0f;

    float distSq = lodSys.distanceSqToEntity("lod_dist");
    assertTrue(approxEqual(distSq, 25000000.0f), "Distance squared correct (3-4-5 triangle)");

    float noEntity = lodSys.distanceSqToEntity("nonexistent");
    assertTrue(noEntity < 0.0f, "Non-existent entity returns negative");
}

// ==================== Phase 5 Continued: Spatial Hash System Tests ====================

void testSpatialHashBasicIndex() {
    std::cout << "\n=== Spatial Hash: Basic Indexing ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);
    spatial.setCellSize(1000.0f);

    auto* e1 = world.createEntity("sh_e1");
    auto* p1 = addComp<components::Position>(e1);
    p1->x = 500.0f; p1->y = 0.0f; p1->z = 0.0f;

    auto* e2 = world.createEntity("sh_e2");
    auto* p2 = addComp<components::Position>(e2);
    p2->x = 600.0f; p2->y = 0.0f; p2->z = 0.0f;

    auto* e3 = world.createEntity("sh_e3");
    auto* p3 = addComp<components::Position>(e3);
    p3->x = 5000.0f; p3->y = 5000.0f; p3->z = 5000.0f;

    spatial.update(0.0f);

    assertTrue(spatial.getIndexedEntityCount() == 3, "3 entities indexed");
    assertTrue(spatial.getOccupiedCellCount() >= 1, "At least 1 cell occupied");
}

void testSpatialHashQueryNear() {
    std::cout << "\n=== Spatial Hash: Query Near ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);
    spatial.setCellSize(1000.0f);

    // Two close entities, one far away
    auto* e1 = world.createEntity("near_a");
    auto* p1 = addComp<components::Position>(e1);
    p1->x = 100.0f; p1->y = 0.0f; p1->z = 0.0f;

    auto* e2 = world.createEntity("near_b");
    auto* p2 = addComp<components::Position>(e2);
    p2->x = 200.0f; p2->y = 0.0f; p2->z = 0.0f;

    auto* e3 = world.createEntity("far_c");
    auto* p3 = addComp<components::Position>(e3);
    p3->x = 50000.0f; p3->y = 0.0f; p3->z = 0.0f;

    spatial.update(0.0f);

    auto nearby = spatial.queryNear(150.0f, 0.0f, 0.0f, 500.0f);
    assertTrue(nearby.size() == 2, "Two entities within 500m of query point");

    auto farNearby = spatial.queryNear(90.0f, 0.0f, 0.0f, 30.0f);
    assertTrue(farNearby.size() == 1, "One entity within 30m of offset query point");
}

void testSpatialHashQueryNeighbours() {
    std::cout << "\n=== Spatial Hash: Query Neighbours ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);
    spatial.setCellSize(1000.0f);

    // Two entities in same cell
    auto* e1 = world.createEntity("nb_a");
    auto* p1 = addComp<components::Position>(e1);
    p1->x = 100.0f; p1->y = 100.0f; p1->z = 100.0f;

    auto* e2 = world.createEntity("nb_b");
    auto* p2 = addComp<components::Position>(e2);
    p2->x = 200.0f; p2->y = 200.0f; p2->z = 200.0f;

    // Entity far away (different cell neighbourhood)
    auto* e3 = world.createEntity("nb_far");
    auto* p3 = addComp<components::Position>(e3);
    p3->x = 50000.0f; p3->y = 50000.0f; p3->z = 50000.0f;

    spatial.update(0.0f);

    auto neighbours = spatial.queryNeighbours("nb_a");
    bool foundB = false;
    bool foundFar = false;
    for (const auto& id : neighbours) {
        if (id == "nb_b") foundB = true;
        if (id == "nb_far") foundFar = true;
    }
    assertTrue(foundB, "Same-cell neighbour found");
    assertTrue(!foundFar, "Far entity not in neighbour set");
}

void testSpatialHashEmptyWorld() {
    std::cout << "\n=== Spatial Hash: Empty World ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);

    spatial.update(0.0f);

    assertTrue(spatial.getIndexedEntityCount() == 0, "Empty world has 0 indexed entities");
    assertTrue(spatial.getOccupiedCellCount() == 0, "Empty world has 0 occupied cells");

    auto nearby = spatial.queryNear(0.0f, 0.0f, 0.0f, 1000.0f);
    assertTrue(nearby.empty(), "No results from empty world");
}

void testSpatialHashCellSizeConfig() {
    std::cout << "\n=== Spatial Hash: Cell Size Configuration ===" << std::endl;
    ecs::World world;
    systems::SpatialHashSystem spatial(&world);

    spatial.setCellSize(500.0f);
    assertTrue(approxEqual(spatial.getCellSize(), 500.0f), "Cell size set to 500");

    // Negative size should be rejected
    spatial.setCellSize(-100.0f);
    assertTrue(approxEqual(spatial.getCellSize(), 500.0f), "Negative cell size rejected");
}

// ==================== WarpHUDTravelMode Tests ====================

void testWarpHUDTravelModeDefaults() {
    std::cout << "\n=== WarpHUDTravelMode defaults ===" << std::endl;
    components::WarpHUDTravelMode mode;
    assertTrue(approxEqual(mode.edge_softness, 0.0f), "edge_softness default 0");
    assertTrue(approxEqual(mode.color_desaturation, 0.0f), "desaturation default 0");
    assertTrue(approxEqual(mode.warning_mute, 0.0f), "warning_mute default 0");
    assertTrue(approxEqual(mode.safe_area_padding, 0.0f), "safe_area_padding default 0");
    assertTrue(approxEqual(mode.hud_scale, 1.0f), "hud_scale default 1");
    assertTrue(!mode.ui_flair_enabled, "ui_flair disabled by default");
    assertTrue(approxEqual(mode.bracket_animation, 0.0f), "bracket_animation default 0");
    assertTrue(approxEqual(mode.ui_glow_intensity, 0.0f), "ui_glow default 0");
    assertTrue(approxEqual(mode.hud_parallax_offset, 0.0f), "parallax default 0");
}

void testHUDTargetsNonePhase() {
    std::cout << "\n=== HUD targets: None phase ===" << std::endl;
    float e, d, w, p, s;
    systems::WarpHUDTravelModeSystem::computeTargets(0, e, d, w, p, s);
    assertTrue(approxEqual(e, 0.0f), "edge_softness 0 for None");
    assertTrue(approxEqual(d, 0.0f), "desaturation 0 for None");
    assertTrue(approxEqual(w, 0.0f), "warning_mute 0 for None");
    assertTrue(approxEqual(p, 0.0f), "padding 0 for None");
    assertTrue(approxEqual(s, 1.0f), "scale 1.0 for None");
}

void testHUDTargetsCruisePhase() {
    std::cout << "\n=== HUD targets: Cruise phase ===" << std::endl;
    float e, d, w, p, s;
    systems::WarpHUDTravelModeSystem::computeTargets(3, e, d, w, p, s);
    assertTrue(e > 0.5f, "cruise edge_softness high");
    assertTrue(d > 0.2f, "cruise desaturation noticeable");
    assertTrue(w > 0.5f, "cruise warnings muted");
    assertTrue(p >= 32.0f && p <= 48.0f, "cruise padding 32-48px");
    assertTrue(s < 1.0f && s >= 0.95f, "cruise scale inward");
}

void testHUDTargetsExitPhase() {
    std::cout << "\n=== HUD targets: Exit phase ===" << std::endl;
    float e, d, w, p, s;
    systems::WarpHUDTravelModeSystem::computeTargets(5, e, d, w, p, s);
    assertTrue(e < 0.3f, "exit edge_softness reducing");
    assertTrue(d < 0.15f, "exit desaturation low");
    assertTrue(approxEqual(w, 0.0f), "exit warnings not muted");
    assertTrue(p < 16.0f, "exit padding almost gone");
    assertTrue(s > 0.97f, "exit scale nearly normal");
}

void testHUDTargetsEventPhase() {
    std::cout << "\n=== HUD targets: Event phase ===" << std::endl;
    float e_event, d_event, w_event, p_event, s_event;
    systems::WarpHUDTravelModeSystem::computeTargets(4, e_event, d_event, w_event, p_event, s_event);
    float e_cruise, d_cruise, w_cruise, p_cruise, s_cruise;
    systems::WarpHUDTravelModeSystem::computeTargets(3, e_cruise, d_cruise, w_cruise, p_cruise, s_cruise);
    assertTrue(w_event < w_cruise, "event warning_mute less than cruise");
}

void testUIFlairComputation() {
    std::cout << "\n=== UI flair computation ===" << std::endl;
    float bracket, glow, parallax;
    systems::WarpHUDTravelModeSystem::computeUIFlair(0.0f, 0.5f, bracket, glow, parallax);
    assertTrue(approxEqual(bracket, 0.5f, 0.02f), "bracket ~0.5 at time 0");
    assertTrue(glow > 0.0f && glow <= 0.5f, "glow tracks bass (0.5)");
    assertTrue(std::fabs(parallax) < 0.1f, "parallax ~0 at time 0");
}

void testUIFlairGlowTracksBass() {
    std::cout << "\n=== UI flair glow tracks bass ===" << std::endl;
    float bracket, glow, parallax;
    systems::WarpHUDTravelModeSystem::computeUIFlair(5.0f, 0.0f, bracket, glow, parallax);
    assertTrue(approxEqual(glow, 0.0f), "no bass = no glow");
    systems::WarpHUDTravelModeSystem::computeUIFlair(5.0f, 1.0f, bracket, glow, parallax);
    assertTrue(glow > 0.7f, "full bass = high glow");
}

void testHUDSystemRampsDuringCruise() {
    std::cout << "\n=== HUD system ramps during cruise ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_warp_hud_cruise");
    auto* ws = addComp<components::WarpState>(e);
    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->warp_time = 10.0f;
    addComp<components::WarpHUDTravelMode>(e);

    systems::WarpHUDTravelModeSystem sys(&world);
    for (int i = 0; i < 60; ++i) sys.update(0.05f);

    auto* hud = e->getComponent<components::WarpHUDTravelMode>();
    assertTrue(hud->edge_softness > 0.4f, "edge_softness ramped up");
    assertTrue(hud->color_desaturation > 0.2f, "desaturation ramped up");
    assertTrue(hud->safe_area_padding > 30.0f, "padding ramped up");
    assertTrue(hud->hud_scale < 0.98f, "scale ramped down");
}

void testHUDSystemReturnsToNormalOnNone() {
    std::cout << "\n=== HUD system returns to normal on None ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_warp_hud_return");
    auto* ws = addComp<components::WarpState>(e);
    addComp<components::WarpHUDTravelMode>(e);

    systems::WarpHUDTravelModeSystem sys(&world);

    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->warp_time = 10.0f;
    for (int i = 0; i < 60; ++i) sys.update(0.05f);

    ws->phase = components::WarpState::WarpPhase::None;
    for (int i = 0; i < 80; ++i) sys.update(0.05f);

    auto* hud = e->getComponent<components::WarpHUDTravelMode>();
    assertTrue(hud->edge_softness < 0.05f, "edge_softness back to ~0");
    assertTrue(hud->color_desaturation < 0.05f, "desaturation back to ~0");
    assertTrue(hud->safe_area_padding < 2.0f, "padding back to ~0");
    assertTrue(hud->hud_scale > 0.99f, "scale back to ~1.0");
}

void testHUDSystemUIFlairOnlyDuringWarp() {
    std::cout << "\n=== HUD UI flair only during warp ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_warp_hud_flair");
    auto* ws = addComp<components::WarpState>(e);
    ws->phase = components::WarpState::WarpPhase::None;
    ws->warp_time = 5.0f;
    auto* hud = addComp<components::WarpHUDTravelMode>(e);
    hud->ui_flair_enabled = true;

    systems::WarpHUDTravelModeSystem sys(&world);
    sys.update(0.016f);

    assertTrue(approxEqual(hud->bracket_animation, 0.0f), "no flair when not in warp");
    assertTrue(approxEqual(hud->ui_glow_intensity, 0.0f), "no glow when not in warp");

    ws->phase = components::WarpState::WarpPhase::Cruise;
    auto* audio = addComp<components::WarpAudioProfile>(e);
    audio->engine_core_volume = 0.6f;
    sys.update(0.016f);

    assertTrue(hud->bracket_animation > 0.0f || hud->ui_glow_intensity > 0.0f,
               "flair active during cruise");
}

// ==================== WarpAutoComfort Tests ====================

void testWarpAutoComfortDefaults() {
    std::cout << "\n=== WarpAutoComfort defaults ===" << std::endl;
    components::WarpAutoComfort c;
    assertTrue(approxEqual(c.target_fps, 60.0f), "target_fps default 60");
    assertTrue(approxEqual(c.current_fps, 60.0f), "current_fps default 60");
    assertTrue(approxEqual(c.comfort_reduction, 0.0f), "comfort_reduction default 0");
    assertTrue(!c.ultrawide_detected, "ultrawide default false");
    assertTrue(approxEqual(c.max_distortion_ultrawide, 0.5f), "max_distortion_uw default 0.5");
}

void testComfortReductionIncreasesOnLowFPS() {
    std::cout << "\n=== Comfort reduction increases on low FPS ===" << std::endl;
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        30.0f, 60.0f, 0.0f, 1.0f);
    assertTrue(result > 0.0f, "reduction increased on low FPS");
}

void testComfortReductionDecreasesOnGoodFPS() {
    std::cout << "\n=== Comfort reduction decreases on good FPS ===" << std::endl;
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        60.0f, 60.0f, 0.5f, 1.0f);
    assertTrue(result < 0.5f, "reduction decreased on good FPS");
}

void testComfortReductionClamped() {
    std::cout << "\n=== Comfort reduction clamped ===" << std::endl;
    float r = 0.0f;
    for (int i = 0; i < 100; ++i) {
        r = systems::WarpAutoComfortSystem::computeComfortReduction(20.0f, 60.0f, r, 0.5f);
    }
    assertTrue(r <= 1.0f && r >= 0.0f, "reduction stays in [0,1]");
}

void testComfortHysteresis() {
    std::cout << "\n=== Comfort hysteresis ===" << std::endl;
    float result = systems::WarpAutoComfortSystem::computeComfortReduction(
        52.0f, 60.0f, 0.3f, 1.0f);
    assertTrue(approxEqual(result, 0.3f), "no change in hysteresis band");
}

void testApplyComfortReducesIntensity() {
    std::cout << "\n=== Apply comfort reduces intensity ===" << std::endl;
    float motion = 1.0f, blur = 1.0f;
    systems::WarpAutoComfortSystem::applyComfort(0.5f, false, 0.5f, motion, blur);
    assertTrue(approxEqual(motion, 0.7f), "motion scaled to 0.7");
    assertTrue(approxEqual(blur, 0.7f), "blur scaled to 0.7");
}

void testApplyComfortUltrawideClamp() {
    std::cout << "\n=== Apply comfort ultrawide clamp ===" << std::endl;
    float motion = 1.0f, blur = 1.0f;
    systems::WarpAutoComfortSystem::applyComfort(0.0f, true, 0.5f, motion, blur);
    assertTrue(approxEqual(motion, 1.0f), "motion unchanged without FPS reduction");
    assertTrue(approxEqual(blur, 0.5f), "blur clamped to 0.5 on ultrawide");
}

void testApplyComfortCombined() {
    std::cout << "\n=== Apply comfort combined ===" << std::endl;
    float motion = 1.0f, blur = 0.8f;
    systems::WarpAutoComfortSystem::applyComfort(0.3f, true, 0.5f, motion, blur);
    assertTrue(motion < 1.0f, "motion reduced by comfort");
    assertTrue(blur <= 0.5f, "blur clamped by ultrawide after comfort reduction");
}

void testAutoComfortSystemReducesOnLowFPS() {
    std::cout << "\n=== AutoComfort system reduces on low FPS ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_auto_comfort");
    auto* comfort = addComp<components::WarpAutoComfort>(e);
    comfort->target_fps = 60.0f;
    comfort->current_fps = 30.0f;
    auto* access = addComp<components::WarpAccessibility>(e);
    access->motion_intensity = 1.0f;
    access->blur_intensity = 1.0f;

    systems::WarpAutoComfortSystem sys(&world);
    for (int i = 0; i < 10; ++i) {
        access->motion_intensity = 1.0f;
        access->blur_intensity = 1.0f;
        sys.update(0.033f);
    }

    assertTrue(comfort->comfort_reduction > 0.0f, "comfort_reduction increased");
    assertTrue(access->motion_intensity < 1.0f, "motion reduced after system tick");
}

// ==================== WarpAccessibility tunnel_geometry Tests ====================

void testAccessibilityTunnelGeometryToggle() {
    std::cout << "\n=== Accessibility tunnel_geometry toggle ===" << std::endl;
    components::WarpAccessibility access;
    assertTrue(access.tunnel_geometry_enabled, "tunnel geometry on by default");
    access.tunnel_geometry_enabled = false;
    assertTrue(!access.tunnel_geometry_enabled, "tunnel geometry toggled off");
}

void testCinematicSystemTunnelGeometryDisabled() {
    std::cout << "\n=== Cinematic system with tunnel geometry disabled ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("test_tunnel_geom");
    auto* ws = addComp<components::WarpState>(e);
    ws->phase = components::WarpState::WarpPhase::Cruise;
    ws->mass_norm = 0.5f;
    addComp<components::WarpTunnelConfig>(e);
    auto* access = addComp<components::WarpAccessibility>(e);
    access->tunnel_geometry_enabled = false;

    systems::WarpCinematicSystem sys(&world);
    sys.update(0.016f);

    auto* tc = e->getComponent<components::WarpTunnelConfig>();
    assertTrue(approxEqual(tc->tunnel_skin, 0.0f), "tunnel_skin zeroed when geometry disabled");
    assertTrue(tc->starfield_bloom > 0.0f, "star streaks still active");
}

// ==================== Warp Meditation System Tests ====================

void testWarpMeditationDefaults() {
    std::cout << "\n=== Warp Meditation Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med1");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    assertTrue(!med->active, "Meditation not active by default");
    assertTrue(approxEqual(med->volume, 0.0f), "Volume is 0 by default");
    assertTrue(approxEqual(med->activation_delay, 15.0f), "Activation delay default 15s");
    assertTrue(approxEqual(med->fade_duration, 5.0f), "Fade duration default 5s");
}

void testMeditationActivatesAfterDelay() {
    std::cout << "\n=== Meditation Activates After Delay ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med2");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    med->activation_delay = 10.0f;

    systems::WarpMeditationSystem sys(&world);
    // Not enough time yet
    sys.update(5.0f);
    assertTrue(!med->active, "Not active before delay");
    // Enough time
    sys.update(6.0f);
    assertTrue(med->active, "Active after delay reached");
}

void testMeditationFadesInDuringCruise() {
    std::cout << "\n=== Meditation Fades In During Cruise ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med3");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    med->activation_delay = 0.0f;
    med->fade_duration = 10.0f;

    systems::WarpMeditationSystem sys(&world);
    sys.update(1.0f);
    assertTrue(med->active, "Active after activation");
    sys.update(5.0f);
    assertTrue(med->volume > 0.0f && med->volume < 1.0f, "Volume ramping up");
    sys.update(10.0f);
    assertTrue(approxEqual(med->volume, 1.0f), "Volume reaches 1.0");
}

void testMeditationFadesOutOnExit() {
    std::cout << "\n=== Meditation Fades Out On Exit ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med4");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    med->activation_delay = 0.0f;
    med->fade_duration = 5.0f;

    systems::WarpMeditationSystem sys(&world);
    sys.update(1.0f);
    sys.update(10.0f); // fully faded in
    assertTrue(approxEqual(med->volume, 1.0f), "Volume at 1.0");

    warp->phase = components::WarpState::WarpPhase::None;
    sys.update(2.0f);
    assertTrue(med->volume < 1.0f, "Volume decreasing after exit");
}

void testMeditationResetOnNone() {
    std::cout << "\n=== Meditation Reset On None ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("med5");
    auto* med = addComp<components::WarpMeditationLayer>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    med->activation_delay = 5.0f;

    systems::WarpMeditationSystem sys(&world);
    sys.update(6.0f);
    assertTrue(med->warp_cruise_time > 0.0f, "Cruise time accumulated");

    warp->phase = components::WarpState::WarpPhase::None;
    sys.update(10.0f);
    assertTrue(approxEqual(med->warp_cruise_time, 0.0f), "Cruise time reset to 0");
}

void testAudioProgressionPhases() {
    std::cout << "\n=== Audio Progression Phases ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("prog1");
    auto* prog = addComp<components::WarpAudioProgression>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    prog->tension_duration = 3.0f;
    prog->stabilize_duration = 5.0f;
    prog->bloom_duration = 4.0f;

    systems::WarpMeditationSystem sys(&world);
    using Phase = components::WarpAudioProgression::Phase;

    sys.update(1.0f);
    assertTrue(prog->current_phase == Phase::Tension, "Phase is Tension at 1s");
    sys.update(3.0f); // total 4s
    assertTrue(prog->current_phase == Phase::Stabilize, "Phase is Stabilize at 4s");
    sys.update(5.0f); // total 9s
    assertTrue(prog->current_phase == Phase::Bloom, "Phase is Bloom at 9s");
    sys.update(4.0f); // total 13s
    assertTrue(prog->current_phase == Phase::Meditative, "Phase is Meditative at 13s");
}

void testAudioProgressionReset() {
    std::cout << "\n=== Audio Progression Reset ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("prog2");
    auto* prog = addComp<components::WarpAudioProgression>(e);
    auto* warp = addComp<components::WarpState>(e);
    warp->phase = components::WarpState::WarpPhase::Cruise;

    systems::WarpMeditationSystem sys(&world);
    sys.update(20.0f);
    using Phase = components::WarpAudioProgression::Phase;
    assertTrue(prog->current_phase == Phase::Meditative, "Reached Meditative");

    warp->phase = components::WarpState::WarpPhase::None;
    sys.update(0.1f);
    assertTrue(prog->current_phase == Phase::Tension, "Reset to Tension on None");
    assertTrue(approxEqual(prog->phase_timer, 0.0f), "Timer reset to 0");
}

void testAudioProgressionComputeOverall() {
    std::cout << "\n=== Audio Progression Compute Overall ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("prog3");
    auto* prog = addComp<components::WarpAudioProgression>(e);
    prog->tension_duration = 3.0f;
    prog->stabilize_duration = 5.0f;
    prog->bloom_duration = 4.0f;

    prog->phase_timer = 0.0f;
    assertTrue(approxEqual(prog->computeOverallProgression(), 0.0f), "Overall 0 at start");

    prog->phase_timer = 6.0f; // halfway through total 12s
    assertTrue(approxEqual(prog->computeOverallProgression(), 0.5f), "Overall 0.5 at midpoint");

    prog->phase_timer = 12.0f;
    assertTrue(approxEqual(prog->computeOverallProgression(), 1.0f), "Overall 1.0 at end");

    prog->phase_timer = 20.0f; // past end
    assertTrue(approxEqual(prog->computeOverallProgression(), 1.0f), "Overall clamped to 1.0");
}

// ==================== JumpDriveSystem Tests ====================

void testJumpDriveInitiate() {
    std::cout << "\n=== Jump Drive: Initiate Jump ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->max_range_ly = 5.0f;
    jd->fuel_per_ly = 500.0f;
    jd->current_fuel = 5000.0f;
    jd->requires_cyno = true;

    systems::JumpDriveSystem sys(&world);
    assertTrue(sys.getPhase("capital1") == "idle", "Initial phase is idle");
    assertTrue(sys.initiateJump("capital1", "SystemB", 3.0f, "cyno1"), "Jump initiated with cyno");
    assertTrue(sys.getPhase("capital1") == "spooling_up", "Phase is spooling_up");
    assertTrue(!sys.initiateJump("capital1", "SystemC", 2.0f, "cyno2"), "Cannot initiate while spooling");
}

void testJumpDriveNoCyno() {
    std::cout << "\n=== Jump Drive: No Cyno Required ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->requires_cyno = true;
    jd->current_fuel = 5000.0f;

    systems::JumpDriveSystem sys(&world);
    assertTrue(!sys.initiateJump("capital1", "SystemB", 3.0f, ""), "Jump fails without cyno when required");
    jd->requires_cyno = false;
    assertTrue(sys.initiateJump("capital1", "SystemB", 3.0f, ""), "Jump succeeds without cyno when not required");
}

void testJumpDriveFullCycle() {
    std::cout << "\n=== Jump Drive: Full Cycle ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->spool_time = 5.0f;
    jd->cooldown_time = 10.0f;
    jd->fuel_per_ly = 500.0f;
    jd->current_fuel = 5000.0f;
    jd->max_range_ly = 5.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    sys.initiateJump("capital1", "SystemB", 3.0f);
    sys.update(5.0f);  // spool complete → Jumping phase, fuel consumed
    assertTrue(sys.getPhase("capital1") == "jumping", "Phase is jumping after spool");
    assertTrue(sys.getTotalJumps("capital1") == 1, "1 jump completed");
    assertTrue(approxEqual(sys.getFuel("capital1"), 3500.0f), "Fuel consumed: 5000 - 500*3 = 3500");

    sys.update(0.1f);  // Jumping → Cooldown
    assertTrue(sys.getPhase("capital1") == "cooldown", "Phase is cooldown");

    sys.update(10.0f);  // Cooldown complete
    assertTrue(sys.getPhase("capital1") == "idle", "Phase returns to idle");
}

void testJumpDriveFuelCheck() {
    std::cout << "\n=== Jump Drive: Fuel Check ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->fuel_per_ly = 500.0f;
    jd->current_fuel = 1000.0f;
    jd->max_range_ly = 5.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    assertTrue(!sys.initiateJump("capital1", "SystemFar", 3.0f), "Jump fails: not enough fuel for 3 LY");
    assertTrue(sys.initiateJump("capital1", "SystemNear", 2.0f), "Jump succeeds: enough fuel for 2 LY");
}

void testJumpDriveFatigue() {
    std::cout << "\n=== Jump Drive: Fatigue ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->spool_time = 1.0f;
    jd->cooldown_time = 1.0f;
    jd->fuel_per_ly = 100.0f;
    jd->current_fuel = 10000.0f;
    jd->max_range_ly = 5.0f;
    jd->max_fatigue = 10.0f;
    jd->fatigue_per_jump = 3.0f;
    jd->fatigue_decay_rate = 0.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    // Jump 1: fatigue 0 → 3
    sys.initiateJump("capital1", "S1", 1.0f);
    sys.update(1.0f);  // spool → jump
    sys.update(0.1f);  // jump → cooldown
    sys.update(1.0f);  // cooldown → idle
    assertTrue(approxEqual(sys.getFatigue("capital1"), 3.0f), "Fatigue after 1 jump");

    // Jump 2: fatigue 3 → 6
    sys.initiateJump("capital1", "S2", 1.0f);
    sys.update(1.0f);
    sys.update(0.1f);
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getFatigue("capital1"), 6.0f), "Fatigue after 2 jumps");

    // Jump 3: fatigue 6 → 9
    sys.initiateJump("capital1", "S3", 1.0f);
    sys.update(1.0f);
    sys.update(0.1f);
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getFatigue("capital1"), 9.0f), "Fatigue after 3 jumps");

    // Jump 4: fatigue 9 → would be 12 but capped at 10, also effective range reduced
    // effective range = 5 * (1 - 9/10) = 0.5 LY — can still jump 0.4 LY
    assertTrue(sys.canJump("capital1", 0.4f), "Can jump short distance with high fatigue");
}

void testJumpDriveFatigueDecay() {
    std::cout << "\n=== Jump Drive: Fatigue Decay ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->fatigue_hours = 5.0f;
    jd->fatigue_decay_rate = 1.0f;
    jd->max_fatigue = 10.0f;

    systems::JumpDriveSystem sys(&world);
    sys.update(3.0f);  // decay: 5 - 1*3 = 2
    assertTrue(approxEqual(sys.getFatigue("capital1"), 2.0f), "Fatigue decayed to 2");
    sys.update(5.0f);  // decay: 2 - 5 → clamped to 0
    assertTrue(approxEqual(sys.getFatigue("capital1"), 0.0f), "Fatigue decayed to 0 (clamped)");
}

void testJumpDriveCancel() {
    std::cout << "\n=== Jump Drive: Cancel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->current_fuel = 5000.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    sys.initiateJump("capital1", "SystemB", 2.0f);
    assertTrue(sys.cancelJump("capital1"), "Cancel during spool succeeds");
    assertTrue(sys.getPhase("capital1") == "idle", "Phase back to idle");
    assertTrue(!sys.cancelJump("capital1"), "Cancel in idle fails");
}

void testJumpDriveRefuel() {
    std::cout << "\n=== Jump Drive: Refuel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->current_fuel = 2000.0f;
    jd->max_fuel = 10000.0f;

    systems::JumpDriveSystem sys(&world);
    assertTrue(sys.refuel("capital1", 3000.0f), "Refuel success");
    assertTrue(approxEqual(sys.getFuel("capital1"), 5000.0f), "Fuel is 5000 after refuel");
    sys.refuel("capital1", 20000.0f);
    assertTrue(approxEqual(sys.getFuel("capital1"), 10000.0f), "Fuel capped at max");
}

void testJumpDriveEffectiveRange() {
    std::cout << "\n=== Jump Drive: Effective Range ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->max_range_ly = 10.0f;
    jd->max_fatigue = 10.0f;
    jd->fatigue_hours = 0.0f;

    systems::JumpDriveSystem sys(&world);
    assertTrue(approxEqual(sys.getEffectiveRange("capital1"), 10.0f), "Full range with no fatigue");
    jd->fatigue_hours = 5.0f;
    assertTrue(approxEqual(sys.getEffectiveRange("capital1"), 5.0f), "Half range at 50% fatigue");
    jd->fatigue_hours = 10.0f;
    assertTrue(approxEqual(sys.getEffectiveRange("capital1"), 0.0f), "Zero range at max fatigue");
}

void testJumpDriveCooldownRemaining() {
    std::cout << "\n=== Jump Drive: Cooldown Remaining ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("capital1");
    auto* jd = addComp<components::JumpDriveState>(e);
    jd->spool_time = 1.0f;
    jd->cooldown_time = 10.0f;
    jd->current_fuel = 5000.0f;
    jd->requires_cyno = false;

    systems::JumpDriveSystem sys(&world);
    assertTrue(approxEqual(sys.getCooldownRemaining("capital1"), 0.0f), "No cooldown in idle");
    sys.initiateJump("capital1", "S1", 1.0f);
    sys.update(1.0f);  // spool → jump
    sys.update(0.1f);  // jump → cooldown
    assertTrue(sys.getCooldownRemaining("capital1") > 9.0f, "Cooldown remaining after jump");
}

void testJumpDriveMissing() {
    std::cout << "\n=== Jump Drive: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::JumpDriveSystem sys(&world);
    assertTrue(sys.getPhase("nonexistent") == "unknown", "Default phase for missing");
    assertTrue(approxEqual(sys.getFuel("nonexistent"), 0.0f), "Default fuel for missing");
    assertTrue(approxEqual(sys.getMaxFuel("nonexistent"), 0.0f), "Default max fuel for missing");
    assertTrue(approxEqual(sys.getFatigue("nonexistent"), 0.0f), "Default fatigue for missing");
    assertTrue(approxEqual(sys.getMaxRange("nonexistent"), 0.0f), "Default max range for missing");
    assertTrue(approxEqual(sys.getEffectiveRange("nonexistent"), 0.0f), "Default effective range for missing");
    assertTrue(!sys.canJump("nonexistent", 1.0f), "Cannot jump for missing");
    assertTrue(sys.getTotalJumps("nonexistent") == 0, "Default total jumps for missing");
    assertTrue(approxEqual(sys.getCooldownRemaining("nonexistent"), 0.0f), "Default cooldown for missing");
}

// ==================== NavigationBookmark System Tests ====================

void testNavBookmarkCreate() {
    std::cout << "\n=== NavBookmark: Create ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initializeBookmarks("player1"), "Init bookmarks succeeds");
    assertTrue(sys.getBookmarkCount("player1") == 0, "No bookmarks initially");
    assertTrue(sys.getFavoriteCount("player1") == 0, "No favorites initially");
}

void testNavBookmarkAdd() {
    std::cout << "\n=== NavBookmark: Add ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    assertTrue(sys.addBookmark("player1", "bm1", "Mining Spot", "system_alpha", 100.0f, 200.0f, 300.0f), "Add bookmark succeeds");
    assertTrue(sys.getBookmarkCount("player1") == 1, "1 bookmark");
    assertTrue(sys.getLabel("player1", "bm1") == "Mining Spot", "Label is Mining Spot");
}

void testNavBookmarkDuplicate() {
    std::cout << "\n=== NavBookmark: Duplicate ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Spot A", "sys1", 0, 0, 0);
    assertTrue(!sys.addBookmark("player1", "bm1", "Spot B", "sys2", 1, 1, 1), "Duplicate ID rejected");
    assertTrue(sys.getBookmarkCount("player1") == 1, "Still 1 bookmark");
}

void testNavBookmarkRemove() {
    std::cout << "\n=== NavBookmark: Remove ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Spot A", "sys1", 0, 0, 0);
    sys.addBookmark("player1", "bm2", "Spot B", "sys2", 1, 1, 1);
    assertTrue(sys.removeBookmark("player1", "bm1"), "Remove succeeds");
    assertTrue(sys.getBookmarkCount("player1") == 1, "1 bookmark remains");
    assertTrue(!sys.removeBookmark("player1", "bm1"), "Remove nonexistent fails");
}

void testNavBookmarkCategory() {
    std::cout << "\n=== NavBookmark: Category ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Spot A", "sys1", 0, 0, 0);
    sys.addBookmark("player1", "bm2", "Spot B", "sys2", 1, 1, 1);
    assertTrue(sys.getCategoryCount("player1", "Personal") == 2, "Both default to Personal");
    assertTrue(sys.setCategory("player1", "bm1", "Corp"), "Set category succeeds");
    assertTrue(sys.getCategoryCount("player1", "Corp") == 1, "1 Corp bookmark");
    assertTrue(sys.getCategoryCount("player1", "Personal") == 1, "1 Personal bookmark");
}

void testNavBookmarkNotes() {
    std::cout << "\n=== NavBookmark: Notes ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Asteroid Belt", "sys1", 0, 0, 0);
    assertTrue(sys.setNotes("player1", "bm1", "Rich in Ferrite"), "Set notes succeeds");
    assertTrue(!sys.setNotes("player1", "bm_bad", "test"), "Set notes on nonexistent fails");
}

void testNavBookmarkFavorite() {
    std::cout << "\n=== NavBookmark: Favorite ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Home", "sys1", 0, 0, 0);
    sys.addBookmark("player1", "bm2", "Mine", "sys2", 1, 1, 1);
    assertTrue(sys.getFavoriteCount("player1") == 0, "No favorites yet");
    assertTrue(sys.toggleFavorite("player1", "bm1"), "Toggle favorite succeeds");
    assertTrue(sys.getFavoriteCount("player1") == 1, "1 favorite");
    assertTrue(sys.toggleFavorite("player1", "bm1"), "Toggle back succeeds");
    assertTrue(sys.getFavoriteCount("player1") == 0, "0 favorites after untoggle");
}

void testNavBookmarkMaxLimit() {
    std::cout << "\n=== NavBookmark: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    // Set a low max for testing
    auto* entity = world.getEntity("player1");
    entity->getComponent<components::NavigationBookmark>()->max_bookmarks = 3;
    sys.addBookmark("player1", "bm1", "A", "s1", 0, 0, 0);
    sys.addBookmark("player1", "bm2", "B", "s2", 0, 0, 0);
    sys.addBookmark("player1", "bm3", "C", "s3", 0, 0, 0);
    assertTrue(!sys.addBookmark("player1", "bm4", "D", "s4", 0, 0, 0), "Max limit enforced");
    assertTrue(sys.getBookmarkCount("player1") == 3, "Still 3 bookmarks");
}

void testNavBookmarkMultiple() {
    std::cout << "\n=== NavBookmark: Multiple ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBookmarks("player1");
    sys.addBookmark("player1", "bm1", "Station Alpha", "sys1", 10, 20, 30);
    sys.addBookmark("player1", "bm2", "Asteroid Belt", "sys1", 40, 50, 60);
    sys.addBookmark("player1", "bm3", "Pirate Zone", "sys2", 70, 80, 90);
    assertTrue(sys.getBookmarkCount("player1") == 3, "3 bookmarks added");
    sys.setCategory("player1", "bm3", "Shared");
    sys.toggleFavorite("player1", "bm1");
    sys.toggleFavorite("player1", "bm2");
    assertTrue(sys.getFavoriteCount("player1") == 2, "2 favorites");
    assertTrue(sys.getCategoryCount("player1", "Shared") == 1, "1 shared");
}

void testNavBookmarkMissing() {
    std::cout << "\n=== NavBookmark: Missing ===" << std::endl;
    ecs::World world;
    systems::NavigationBookmarkSystem sys(&world);
    assertTrue(!sys.initializeBookmarks("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.addBookmark("nonexistent", "bm1", "A", "s1", 0, 0, 0), "Add fails on missing");
    assertTrue(!sys.removeBookmark("nonexistent", "bm1"), "Remove fails on missing");
    assertTrue(!sys.setCategory("nonexistent", "bm1", "Corp"), "SetCategory fails on missing");
    assertTrue(!sys.toggleFavorite("nonexistent", "bm1"), "Toggle fails on missing");
    assertTrue(sys.getBookmarkCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getFavoriteCount("nonexistent") == 0, "0 favorites on missing");
    assertTrue(sys.getCategoryCount("nonexistent", "Personal") == 0, "0 category on missing");
    assertTrue(sys.getLabel("nonexistent", "bm1") == "", "Empty label on missing");
}

// ==================== ScanProbe System Tests ====================

void testScanProbeCreate() {
    std::cout << "\n=== ScanProbe: Create ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeProbes("ship1", "player1"), "Init probes succeeds");
    assertTrue(sys.getProbeCount("ship1") == 0, "No probes initially");
    assertTrue(sys.getTotalScansCompleted("ship1") == 0, "No scans initially");
    assertTrue(sys.getTotalSitesFound("ship1") == 0, "No sites initially");
}

void testScanProbeLaunch() {
    std::cout << "\n=== ScanProbe: Launch ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    assertTrue(sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f), "Launch probe succeeds");
    assertTrue(sys.getProbeCount("ship1") == 1, "1 probe launched");
    assertTrue(approxEqual(sys.getScanProgress("ship1", "p1"), 0.0f), "Initial progress 0");
}

void testScanProbeDuplicate() {
    std::cout << "\n=== ScanProbe: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    assertTrue(!sys.launchProbe("ship1", "p1", 1, 0.0f, 0.0f, 0.0f), "Duplicate probe rejected");
    assertTrue(sys.getProbeCount("ship1") == 1, "Still 1 probe");
}

void testScanProbeScan() {
    std::cout << "\n=== ScanProbe: Scan ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    assertTrue(sys.startScan("ship1", "p1"), "Start scan succeeds");
    assertTrue(sys.getActiveProbeCount("ship1") == 1, "1 active probe");
    // scan_time=10, scan_strength=1, delta=5 → progress = 0.5
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getScanProgress("ship1", "p1"), 0.5f), "Progress at 0.5");
}

void testScanProbeComplete() {
    std::cout << "\n=== ScanProbe: Complete ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    sys.startScan("ship1", "p1");
    sys.update(10.0f); // completes scan
    assertTrue(approxEqual(sys.getScanProgress("ship1", "p1"), 1.0f), "Scan complete at 1.0");
    assertTrue(sys.getTotalScansCompleted("ship1") == 1, "1 scan completed");
    assertTrue(sys.getActiveProbeCount("ship1") == 0, "No active probes after complete");
}

void testScanProbeExpiry() {
    std::cout << "\n=== ScanProbe: Expiry ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    // Default lifetime = 300s
    sys.update(300.0f);
    assertTrue(sys.getProbeCount("ship1") == 0, "Probe expired and removed");
}

void testScanProbeRecall() {
    std::cout << "\n=== ScanProbe: Recall ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    sys.launchProbe("ship1", "p1", 0, 10.0f, 20.0f, 30.0f);
    sys.launchProbe("ship1", "p2", 1, 50.0f, 60.0f, 70.0f);
    assertTrue(sys.recallProbe("ship1", "p1"), "Recall succeeds");
    assertTrue(sys.getProbeCount("ship1") == 1, "1 probe remaining");
    assertTrue(!sys.recallProbe("ship1", "p1"), "Recall nonexistent fails");
}

void testScanProbeResults() {
    std::cout << "\n=== ScanProbe: Results ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    assertTrue(sys.addResult("ship1", "sig1", "Anomaly", 0.75f), "Add result succeeds");
    assertTrue(sys.getResultCount("ship1") == 1, "1 result");
    assertTrue(sys.getTotalSitesFound("ship1") == 1, "1 site found");
    assertTrue(!sys.addResult("ship1", "sig1", "Wormhole", 0.5f), "Duplicate result rejected");
    assertTrue(sys.getResultCount("ship1") == 1, "Still 1 result");
}

void testScanProbeMaxLimit() {
    std::cout << "\n=== ScanProbe: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeProbes("ship1", "player1");
    // max_probes = 8
    for (int i = 0; i < 8; i++) {
        sys.launchProbe("ship1", "p" + std::to_string(i), 0, 0, 0, 0);
    }
    assertTrue(!sys.launchProbe("ship1", "p8", 0, 0, 0, 0), "Max probes enforced at 8");
    assertTrue(sys.getProbeCount("ship1") == 8, "Still 8 probes");
}

void testScanProbeMissing() {
    std::cout << "\n=== ScanProbe: Missing ===" << std::endl;
    ecs::World world;
    systems::ScanProbeSystem sys(&world);
    assertTrue(!sys.initializeProbes("nonexistent", "p1"), "Init fails on missing");
    assertTrue(!sys.launchProbe("nonexistent", "p1", 0, 0, 0, 0), "Launch fails on missing");
    assertTrue(!sys.recallProbe("nonexistent", "p1"), "Recall fails on missing");
    assertTrue(!sys.startScan("nonexistent", "p1"), "Start scan fails on missing");
    assertTrue(sys.getProbeCount("nonexistent") == 0, "0 probes on missing");
    assertTrue(sys.getActiveProbeCount("nonexistent") == 0, "0 active on missing");
    assertTrue(approxEqual(sys.getScanProgress("nonexistent", "p1"), 0.0f), "0 progress on missing");
    assertTrue(!sys.addResult("nonexistent", "r1", "Anomaly", 0.5f), "Add result fails on missing");
    assertTrue(sys.getResultCount("nonexistent") == 0, "0 results on missing");
    assertTrue(sys.getTotalScansCompleted("nonexistent") == 0, "0 scans on missing");
    assertTrue(sys.getTotalSitesFound("nonexistent") == 0, "0 sites on missing");
}

// ==================== Autopilot System Tests ====================

void testAutopilotCreate() {
    std::cout << "\n=== Autopilot: Create ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeAutopilot("ship1", "player1"), "Init autopilot succeeds");
    assertTrue(sys.getWaypointCount("ship1") == 0, "No waypoints initially");
    assertTrue(!sys.isEngaged("ship1"), "Not engaged initially");
    assertTrue(sys.getWaypointsReached("ship1") == 0, "No waypoints reached");
}

void testAutopilotAddWaypoint() {
    std::cout << "\n=== Autopilot: AddWaypoint ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    assertTrue(sys.addWaypoint("ship1", "wp1", "Station Alpha", 1000.0f, 0.0f, 0.0f), "Add waypoint succeeds");
    assertTrue(sys.getWaypointCount("ship1") == 1, "1 waypoint");
    assertTrue(!sys.addWaypoint("ship1", "wp1", "Dup", 0.0f, 0.0f, 0.0f), "Duplicate rejected");
}

void testAutopilotEngage() {
    std::cout << "\n=== Autopilot: Engage ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    assertTrue(!sys.engage("ship1"), "Engage fails with no waypoints");
    sys.addWaypoint("ship1", "wp1", "Gate A", 1000.0f, 0.0f, 0.0f);
    assertTrue(sys.engage("ship1"), "Engage succeeds with waypoints");
    assertTrue(sys.isEngaged("ship1"), "Autopilot engaged");
}

void testAutopilotDisengage() {
    std::cout << "\n=== Autopilot: Disengage ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 1000.0f, 0.0f, 0.0f);
    sys.engage("ship1");
    assertTrue(sys.disengage("ship1"), "Disengage succeeds");
    assertTrue(!sys.isEngaged("ship1"), "No longer engaged");
}

void testAutopilotNavigation() {
    std::cout << "\n=== Autopilot: Navigation ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 1000.0f, 0.0f, 0.0f);
    sys.engage("ship1");
    // speed=100 m/s, distance=1000m, travel for 10s → 1000m traveled
    // Arrival distance = 50m, so after 10s (1000m traveled), distance_to_next = 1000-1000 = 0 → arrived
    sys.update(10.0f);
    assertTrue(sys.getWaypointsReached("ship1") == 1, "Waypoint reached");
    assertTrue(sys.getCurrentWaypointIndex("ship1") == 1, "Index advanced to 1");
}

void testAutopilotRouteComplete() {
    std::cout << "\n=== Autopilot: RouteComplete ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 500.0f, 0.0f, 0.0f);
    sys.engage("ship1");
    sys.update(5.0f); // 100 m/s * 5s = 500m, arrives at wp1
    assertTrue(sys.isRouteComplete("ship1"), "Route complete after all waypoints");
}

void testAutopilotLoop() {
    std::cout << "\n=== Autopilot: Loop ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 500.0f, 0.0f, 0.0f);
    assertTrue(sys.setLoop("ship1", true), "Set loop succeeds");
    sys.engage("ship1");
    sys.update(5.0f); // reach wp1
    assertTrue(!sys.isRouteComplete("ship1"), "Route not complete in loop mode");
    assertTrue(sys.getCurrentWaypointIndex("ship1") == 0, "Index reset to 0 for loop");
}

void testAutopilotRemoveWaypoint() {
    std::cout << "\n=== Autopilot: RemoveWaypoint ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 1000.0f, 0.0f, 0.0f);
    sys.addWaypoint("ship1", "wp2", "Gate B", 2000.0f, 0.0f, 0.0f);
    assertTrue(sys.removeWaypoint("ship1", "wp1"), "Remove succeeds");
    assertTrue(sys.getWaypointCount("ship1") == 1, "1 waypoint remaining");
    assertTrue(!sys.removeWaypoint("ship1", "wp1"), "Remove nonexistent fails");
}

void testAutopilotMaxWaypoints() {
    std::cout << "\n=== Autopilot: MaxWaypoints ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    auto* entity = world.getEntity("ship1");
    auto* ap = entity->getComponent<components::Autopilot>();
    ap->max_waypoints = 3;
    sys.addWaypoint("ship1", "w1", "A", 100.0f, 0.0f, 0.0f);
    sys.addWaypoint("ship1", "w2", "B", 200.0f, 0.0f, 0.0f);
    sys.addWaypoint("ship1", "w3", "C", 300.0f, 0.0f, 0.0f);
    assertTrue(!sys.addWaypoint("ship1", "w4", "D", 400.0f, 0.0f, 0.0f), "Max waypoints enforced");
    assertTrue(sys.getWaypointCount("ship1") == 3, "Still 3 waypoints");
}

void testAutopilotMissing() {
    std::cout << "\n=== Autopilot: Missing ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    assertTrue(!sys.initializeAutopilot("nonexistent", "p1"), "Init fails on missing");
    assertTrue(!sys.addWaypoint("nonexistent", "w1", "A", 0, 0, 0), "Add fails on missing");
    assertTrue(!sys.removeWaypoint("nonexistent", "w1"), "Remove fails on missing");
    assertTrue(!sys.engage("nonexistent"), "Engage fails on missing");
    assertTrue(!sys.disengage("nonexistent"), "Disengage fails on missing");
    assertTrue(!sys.setLoop("nonexistent", true), "Set loop fails on missing");
    assertTrue(!sys.setSpeed("nonexistent", 200.0f), "Set speed fails on missing");
    assertTrue(sys.getWaypointCount("nonexistent") == 0, "0 waypoints on missing");
    assertTrue(sys.getCurrentWaypointIndex("nonexistent") == 0, "0 index on missing");
    assertTrue(sys.getWaypointsReached("nonexistent") == 0, "0 reached on missing");
    assertTrue(approxEqual(sys.getTotalDistanceTraveled("nonexistent"), 0.0f), "0 distance on missing");
    assertTrue(!sys.isEngaged("nonexistent"), "Not engaged on missing");
    assertTrue(!sys.isRouteComplete("nonexistent"), "Not complete on missing");
}

// ==================== JumpGate System Tests ====================

void testJumpGateCreate() {
    std::cout << "\n=== JumpGate: Create ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    assertTrue(sys.initializeGateNetwork("gate1", "system_jita"), "Init gate network succeeds");
    assertTrue(sys.getGateCount("gate1") == 0, "No gates initially");
    assertTrue(sys.getTotalJumps("gate1") == 0, "No jumps initially");
}

void testJumpGateAdd() {
    std::cout << "\n=== JumpGate: Add ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    assertTrue(sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f), "Add gate succeeds");
    assertTrue(sys.getGateCount("gate1") == 1, "1 gate");
    assertTrue(sys.getOnlineGateCount("gate1") == 1, "1 online gate");
}

void testJumpGateDuplicate() {
    std::cout << "\n=== JumpGate: Duplicate ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    assertTrue(!sys.addGate("gate1", "g1", "system_dodixie", "g_dod1", 60.0f, 0.7f), "Duplicate rejected");
    assertTrue(sys.getGateCount("gate1") == 1, "Still 1 gate");
}

void testJumpGateActivate() {
    std::cout << "\n=== JumpGate: Activate ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    assertTrue(sys.isGateReady("gate1", "g1"), "Gate ready before activation");
    assertTrue(sys.activateGate("gate1", "g1"), "Activation succeeds");
    assertTrue(!sys.isGateReady("gate1", "g1"), "Gate not ready during activation");
    assertTrue(!sys.activateGate("gate1", "g1"), "Double activation rejected");
}

void testJumpGateJump() {
    std::cout << "\n=== JumpGate: Jump ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    sys.activateGate("gate1", "g1");
    // Advance time to complete activation (default 10s)
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getActivationProgress("gate1", "g1"), 1.0f), "Activation complete");
    assertTrue(sys.completeJump("gate1", "g1"), "Jump completes");
    assertTrue(sys.getTotalJumps("gate1") == 1, "1 total jump");
}

void testJumpGateCooldown() {
    std::cout << "\n=== JumpGate: Cooldown ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    sys.activateGate("gate1", "g1");
    sys.update(10.0f); // complete activation
    sys.completeJump("gate1", "g1");
    assertTrue(!sys.isGateReady("gate1", "g1"), "Gate on cooldown");
    assertTrue(sys.getCooldownRemaining("gate1", "g1") > 0.0f, "Cooldown > 0");
    assertTrue(!sys.activateGate("gate1", "g1"), "Can't activate on cooldown");
    sys.update(30.0f); // wait out cooldown
    assertTrue(sys.isGateReady("gate1", "g1"), "Gate ready after cooldown");
}

void testJumpGateOffline() {
    std::cout << "\n=== JumpGate: Offline ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    assertTrue(sys.setGateOnline("gate1", "g1", false), "Set offline succeeds");
    assertTrue(!sys.activateGate("gate1", "g1"), "Can't activate offline gate");
    assertTrue(sys.getOnlineGateCount("gate1") == 0, "0 online gates");
    assertTrue(sys.setGateOnline("gate1", "g1", true), "Set online succeeds");
    assertTrue(sys.getOnlineGateCount("gate1") == 1, "1 online gate");
}

void testJumpGateRemove() {
    std::cout << "\n=== JumpGate: Remove ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    sys.addGate("gate1", "g1", "system_amarr", "g_amarr1", 50.0f, 0.9f);
    sys.addGate("gate1", "g2", "system_dodixie", "g_dod1", 60.0f, 0.7f);
    assertTrue(sys.removeGate("gate1", "g1"), "Remove succeeds");
    assertTrue(sys.getGateCount("gate1") == 1, "1 gate remaining");
    assertTrue(!sys.removeGate("gate1", "g1"), "Remove nonexistent fails");
}

void testJumpGateMaxLimit() {
    std::cout << "\n=== JumpGate: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    world.createEntity("gate1");
    sys.initializeGateNetwork("gate1", "system_jita");
    auto* entity = world.getEntity("gate1");
    auto* jg = entity->getComponent<components::JumpGate>();
    jg->max_gates = 2;
    sys.addGate("gate1", "g1", "sys_a", "ga", 50.0f, 1.0f);
    sys.addGate("gate1", "g2", "sys_b", "gb", 50.0f, 1.0f);
    assertTrue(!sys.addGate("gate1", "g3", "sys_c", "gc", 50.0f, 1.0f), "Max limit enforced");
    assertTrue(sys.getGateCount("gate1") == 2, "Still 2 gates");
}

void testJumpGateMissing() {
    std::cout << "\n=== JumpGate: Missing ===" << std::endl;
    ecs::World world;
    systems::JumpGateSystem sys(&world);
    assertTrue(!sys.initializeGateNetwork("nonexistent", "s1"), "Init fails on missing");
    assertTrue(!sys.addGate("nonexistent", "g1", "s", "g", 50.0f, 1.0f), "Add fails on missing");
    assertTrue(!sys.removeGate("nonexistent", "g1"), "Remove fails on missing");
    assertTrue(!sys.activateGate("nonexistent", "g1"), "Activate fails on missing");
    assertTrue(!sys.completeJump("nonexistent", "g1"), "Jump fails on missing");
    assertTrue(sys.getGateCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getOnlineGateCount("nonexistent") == 0, "0 online on missing");
    assertTrue(sys.getTotalJumps("nonexistent") == 0, "0 jumps on missing");
    assertTrue(!sys.isGateReady("nonexistent", "g1"), "Not ready on missing");
    assertTrue(approxEqual(sys.getActivationProgress("nonexistent", "g1"), 0.0f), "0 progress on missing");
}


void run_navigation_tests() {
    testWarpAnomalyNoneIfNotCruising();
    testWarpAnomalyNoneIfShortWarp();
    testWarpAnomalyTriggersOnLongWarp();
    testWarpAnomalyCount();
    testWarpAnomalyClear();
    testWarpStatePhaseAlign();
    testWarpStatePhaseCruise();
    testWarpStatePhaseExit();
    testWarpStateResetOnArrival();
    testWarpStateIntensity();
    testWarpDisruptionPreventsWarp();
    testWarpDisruptionInsufficientStrength();
    testIsWarpDisruptedQuery();
    testWarpSpeedFromShipComponent();
    testBattleshipSlowerWarp();
    testWarpNoDisruptionWithoutWarpState();
    testAIDynamicOrbitFrigate();
    testAIDynamicOrbitCruiser();
    testAIDynamicOrbitBattleship();
    testAIDynamicOrbitCapital();
    testAIDynamicOrbitUnknown();
    testAIEngagementRangeFromWeapon();
    testAIEngagementRangeNoWeapon();
    testAITargetSelectionClosest();
    testAITargetSelectionLowestHP();
    testAIDynamicOrbitApplied();
    testProtocolDockMessages();
    testProtocolUndockMessage();
    testProtocolRepairMessage();
    testProtocolDamageEventMessage();
    testProtocolDockRequestParse();
    testProtocolWarpMessages();
    testProtocolMovementMessages();
    testWarpCinematicCompositeIntensityFrigate();
    testWarpCinematicCompositeIntensityCapital();
    testWarpCinematicCompositeIntensityNone();
    testWarpCinematicLayersFrigate();
    testWarpCinematicLayersCapitalMoreDistortion();
    testWarpCinematicAudioFrigate();
    testWarpCinematicAudioCapitalLowerPitch();
    testWarpCinematicSystemUpdate();
    testWarpCinematicAccessibilityReducesMotion();
    testWarpCinematicNonePhaseZeroIntensity();
    testWarpCinematicAlignPhaseSubtle();
    testWarpCinematicExitPhaseFades();
    testAnomalyGenerateCreatesEntities();
    testAnomalyHighsecFewerThanNullsec();
    testAnomalyDeterministicSeed();
    testAnomalyDifficultyFromSecurity();
    testAnomalyNpcCountScales();
    testAnomalyLootMultiplierScales();
    testAnomalyCompleteAnomaly();
    testAnomalyDespawnOnTimer();
    testAnomalySignatureStrength();
    testScannerStartScan();
    testScannerStopScan();
    testScannerDetectsAnomaly();
    testScannerSignalAccumulates();
    testScannerEffectiveScanStrength();
    testScannerSignalGainPerCycle();
    testScannerWarpableAtFullSignal();
    testScannerNoResultWithoutAnomalies();
    testScanDiscoverWarpFlow();
    testScanStopPreservesResults();
    testScanAnomalyComplete();
    testScanProtocolRoundTrip();
    testDifficultyInitializeZone();
    testDifficultyHighsecLowMultipliers();
    testDifficultyNullsecHighMultipliers();
    testDifficultyLootScaling();
    testDifficultyOreScaling();
    testDifficultyMaxTierFromSecurity();
    testDifficultyApplyToNPC();
    testDifficultySpawnRateScaling();
    testLODPriorityDefaults();
    testLODPriorityPlayerShip();
    testFleetStress150Ships();
    testFleetStressSystemUpdates();
    testPersistenceAnomalyVisualCue();
    testPersistenceLODPriority();
    testLODSystemDefaults();
    testLODSystemPriorityComputation();
    testLODSystemForceVisible();
    testLODSystemDistanceQuery();
    testSpatialHashBasicIndex();
    testSpatialHashQueryNear();
    testSpatialHashQueryNeighbours();
    testSpatialHashEmptyWorld();
    testSpatialHashCellSizeConfig();
    testWarpHUDTravelModeDefaults();
    testHUDTargetsNonePhase();
    testHUDTargetsCruisePhase();
    testHUDTargetsExitPhase();
    testHUDTargetsEventPhase();
    testUIFlairComputation();
    testUIFlairGlowTracksBass();
    testHUDSystemRampsDuringCruise();
    testHUDSystemReturnsToNormalOnNone();
    testHUDSystemUIFlairOnlyDuringWarp();
    testWarpAutoComfortDefaults();
    testComfortReductionIncreasesOnLowFPS();
    testComfortReductionDecreasesOnGoodFPS();
    testComfortReductionClamped();
    testComfortHysteresis();
    testApplyComfortReducesIntensity();
    testApplyComfortUltrawideClamp();
    testApplyComfortCombined();
    testAutoComfortSystemReducesOnLowFPS();
    testAccessibilityTunnelGeometryToggle();
    testCinematicSystemTunnelGeometryDisabled();
    testWarpMeditationDefaults();
    testMeditationActivatesAfterDelay();
    testMeditationFadesInDuringCruise();
    testMeditationFadesOutOnExit();
    testMeditationResetOnNone();
    testAudioProgressionPhases();
    testAudioProgressionReset();
    testAudioProgressionComputeOverall();
    testJumpDriveInitiate();
    testJumpDriveNoCyno();
    testJumpDriveFullCycle();
    testJumpDriveFuelCheck();
    testJumpDriveFatigue();
    testJumpDriveFatigueDecay();
    testJumpDriveCancel();
    testJumpDriveRefuel();
    testJumpDriveEffectiveRange();
    testJumpDriveCooldownRemaining();
    testJumpDriveMissing();
    testNavBookmarkCreate();
    testNavBookmarkAdd();
    testNavBookmarkDuplicate();
    testNavBookmarkRemove();
    testNavBookmarkCategory();
    testNavBookmarkNotes();
    testNavBookmarkFavorite();
    testNavBookmarkMaxLimit();
    testNavBookmarkMultiple();
    testNavBookmarkMissing();
    testScanProbeCreate();
    testScanProbeLaunch();
    testScanProbeDuplicate();
    testScanProbeScan();
    testScanProbeComplete();
    testScanProbeExpiry();
    testScanProbeRecall();
    testScanProbeResults();
    testScanProbeMaxLimit();
    testScanProbeMissing();
    testAutopilotCreate();
    testAutopilotAddWaypoint();
    testAutopilotEngage();
    testAutopilotDisengage();
    testAutopilotNavigation();
    testAutopilotRouteComplete();
    testAutopilotLoop();
    testAutopilotRemoveWaypoint();
    testAutopilotMaxWaypoints();
    testAutopilotMissing();
    testJumpGateCreate();
    testJumpGateAdd();
    testJumpGateDuplicate();
    testJumpGateActivate();
    testJumpGateJump();
    testJumpGateCooldown();
    testJumpGateOffline();
    testJumpGateRemove();
    testJumpGateMaxLimit();
    testJumpGateMissing();
}
