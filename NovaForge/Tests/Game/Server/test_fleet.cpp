/**
 * Fleet tests
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

// ==================== FleetSystem Tests ====================

void testFleetCreateAndDisband() {
    std::cout << "\n=== Fleet Create and Disband ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* player1 = world.createEntity("player_1");
    addComp<components::Player>(player1)->character_name = "Commander";
    
    // Create fleet
    std::string fleet_id = fleetSys.createFleet("player_1", "Alpha Fleet");
    assertTrue(!fleet_id.empty(), "Fleet created successfully");
    assertTrue(fleetSys.getFleetCount() == 1, "Fleet count is 1");
    assertTrue(fleetSys.getMemberCount(fleet_id) == 1, "Fleet has 1 member (FC)");
    
    const systems::Fleet* fleet = fleetSys.getFleet(fleet_id);
    assertTrue(fleet != nullptr, "Fleet retrievable");
    assertTrue(fleet->fleet_name == "Alpha Fleet", "Fleet name correct");
    assertTrue(fleet->commander_entity_id == "player_1", "Commander is player_1");
    
    // FC has FleetMembership component
    auto* fm = player1->getComponent<components::FleetMembership>();
    assertTrue(fm != nullptr, "FC has FleetMembership component");
    assertTrue(fm->role == "FleetCommander", "FC role is FleetCommander");
    
    // Cannot create another fleet while in one
    std::string fleet2 = fleetSys.createFleet("player_1", "Beta Fleet");
    assertTrue(fleet2.empty(), "Cannot create fleet while already in one");
    
    // Disband
    assertTrue(fleetSys.disbandFleet(fleet_id, "player_1"), "FC can disband fleet");
    assertTrue(fleetSys.getFleetCount() == 0, "No fleets after disband");
    assertTrue(player1->getComponent<components::FleetMembership>() == nullptr,
               "FleetMembership removed after disband");
}

void testFleetAddRemoveMembers() {
    std::cout << "\n=== Fleet Add/Remove Members ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Wing1";
    auto* p3 = world.createEntity("pilot_3");
    addComp<components::Player>(p3)->character_name = "Wing2";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Test Fleet");
    
    // Add members
    assertTrue(fleetSys.addMember(fleet_id, "pilot_2"), "Add pilot_2 succeeds");
    assertTrue(fleetSys.addMember(fleet_id, "pilot_3"), "Add pilot_3 succeeds");
    assertTrue(fleetSys.getMemberCount(fleet_id) == 3, "Fleet has 3 members");
    
    // Cannot add same member twice
    assertTrue(!fleetSys.addMember(fleet_id, "pilot_2"), "Cannot add duplicate member");
    
    // Cannot add nonexistent entity
    assertTrue(!fleetSys.addMember(fleet_id, "ghost"), "Cannot add nonexistent entity");
    
    // Entity fleet lookup
    assertTrue(fleetSys.getFleetForEntity("pilot_2") == fleet_id, "pilot_2 fleet lookup correct");
    assertTrue(fleetSys.getFleetForEntity("ghost").empty(), "Nonexistent entity has no fleet");
    
    // Remove member
    assertTrue(fleetSys.removeMember(fleet_id, "pilot_2"), "Remove pilot_2 succeeds");
    assertTrue(fleetSys.getMemberCount(fleet_id) == 2, "Fleet has 2 members after remove");
    assertTrue(fleetSys.getFleetForEntity("pilot_2").empty(), "Removed member has no fleet");
    assertTrue(p2->getComponent<components::FleetMembership>() == nullptr,
               "Removed member has no FleetMembership component");
}

void testFleetFCLeavePromotes() {
    std::cout << "\n=== Fleet FC Leave Auto-Promotes ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Test Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    
    // FC leaves
    fleetSys.removeMember(fleet_id, "fc");
    assertTrue(fleetSys.getFleetCount() == 1, "Fleet still exists after FC leave");
    
    const systems::Fleet* fleet = fleetSys.getFleet(fleet_id);
    assertTrue(fleet != nullptr, "Fleet still retrievable");
    assertTrue(fleet->commander_entity_id == "pilot_2", "pilot_2 auto-promoted to FC");
    
    auto* fm = p2->getComponent<components::FleetMembership>();
    assertTrue(fm != nullptr && fm->role == "FleetCommander", "Promoted member has FC role");
}

void testFleetDisbandOnEmpty() {
    std::cout << "\n=== Fleet Disbands When Empty ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Solo Fleet");
    assertTrue(fleetSys.getFleetCount() == 1, "Fleet exists");
    
    fleetSys.removeMember(fleet_id, "fc");
    assertTrue(fleetSys.getFleetCount() == 0, "Fleet auto-disbanded when last member leaves");
}

void testFleetPromoteMember() {
    std::cout << "\n=== Fleet Promote Member ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    auto* p3 = world.createEntity("pilot_3");
    addComp<components::Player>(p3)->character_name = "Pilot3";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Test Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    fleetSys.addMember(fleet_id, "pilot_3");
    
    // Promote to WingCommander
    assertTrue(fleetSys.promoteMember(fleet_id, "fc", "pilot_2", "WingCommander"),
               "Promote pilot_2 to WingCommander succeeds");
    auto* fm2 = p2->getComponent<components::FleetMembership>();
    assertTrue(fm2 != nullptr && fm2->role == "WingCommander", "pilot_2 role updated");
    
    // Promote to SquadCommander
    assertTrue(fleetSys.promoteMember(fleet_id, "fc", "pilot_3", "SquadCommander"),
               "Promote pilot_3 to SquadCommander succeeds");
    
    // Non-FC cannot promote
    assertTrue(!fleetSys.promoteMember(fleet_id, "pilot_2", "pilot_3", "Member"),
               "Non-FC cannot promote");
    
    // Invalid role
    assertTrue(!fleetSys.promoteMember(fleet_id, "fc", "pilot_2", "Admiral"),
               "Invalid role rejected");
    
    // Promote to FC transfers command
    assertTrue(fleetSys.promoteMember(fleet_id, "fc", "pilot_2", "FleetCommander"),
               "Transfer FC to pilot_2 succeeds");
    const systems::Fleet* fleet = fleetSys.getFleet(fleet_id);
    assertTrue(fleet->commander_entity_id == "pilot_2", "pilot_2 is now FC");
    auto* fm_fc = fc->getComponent<components::FleetMembership>();
    assertTrue(fm_fc->role == "Member", "Old FC demoted to Member");
}

void testFleetSquadAndWingOrganization() {
    std::cout << "\n=== Fleet Squad and Wing Organization ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("p2");
    addComp<components::Player>(p2)->character_name = "P2";
    auto* p3 = world.createEntity("p3");
    addComp<components::Player>(p3)->character_name = "P3";
    auto* p4 = world.createEntity("p4");
    addComp<components::Player>(p4)->character_name = "P4";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Organized Fleet");
    fleetSys.addMember(fleet_id, "p2");
    fleetSys.addMember(fleet_id, "p3");
    fleetSys.addMember(fleet_id, "p4");
    
    // Assign to squads
    assertTrue(fleetSys.assignToSquad(fleet_id, "p2", "squad_alpha"),
               "Assign p2 to squad_alpha");
    assertTrue(fleetSys.assignToSquad(fleet_id, "p3", "squad_alpha"),
               "Assign p3 to squad_alpha");
    assertTrue(fleetSys.assignToSquad(fleet_id, "p4", "squad_bravo"),
               "Assign p4 to squad_bravo");
    
    // Check squad membership
    auto* fm2 = p2->getComponent<components::FleetMembership>();
    assertTrue(fm2->squad_id == "squad_alpha", "p2 squad_id is squad_alpha");
    
    const systems::Fleet* fleet = fleetSys.getFleet(fleet_id);
    assertTrue(fleet->squads.at("squad_alpha").size() == 2, "squad_alpha has 2 members");
    assertTrue(fleet->squads.at("squad_bravo").size() == 1, "squad_bravo has 1 member");
    
    // Assign squads to wings
    assertTrue(fleetSys.assignSquadToWing(fleet_id, "squad_alpha", "wing_1"),
               "Assign squad_alpha to wing_1");
    assertTrue(fleetSys.assignSquadToWing(fleet_id, "squad_bravo", "wing_1"),
               "Assign squad_bravo to wing_1");
    
    assertTrue(fleet->wings.at("wing_1").size() == 2, "wing_1 has 2 squads");
    
    // Nonexistent squad cannot be assigned
    assertTrue(!fleetSys.assignSquadToWing(fleet_id, "ghost_squad", "wing_2"),
               "Cannot assign nonexistent squad to wing");
    
    // Non-member cannot be assigned to squad
    assertTrue(!fleetSys.assignToSquad(fleet_id, "ghost", "squad_alpha"),
               "Cannot assign non-member to squad");
}

void testFleetBonuses() {
    std::cout << "\n=== Fleet Bonuses ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("booster");
    addComp<components::Player>(p2)->character_name = "Booster";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Bonus Fleet");
    fleetSys.addMember(fleet_id, "booster");
    
    // Set booster
    assertTrue(fleetSys.setBooster(fleet_id, "armor", "booster"), "Set armor booster");
    assertTrue(fleetSys.setBooster(fleet_id, "shield", "booster"), "Set shield booster");
    
    // Invalid booster type
    assertTrue(!fleetSys.setBooster(fleet_id, "invalid", "booster"), "Invalid booster type rejected");
    
    // Non-member cannot be booster
    assertTrue(!fleetSys.setBooster(fleet_id, "armor", "ghost"), "Non-member cannot be booster");
    
    // Check bonus definitions
    auto armor_bonuses = fleetSys.getBonusesForType("armor");
    assertTrue(armor_bonuses.size() == 2, "Armor has 2 bonuses");
    assertTrue(approxEqual(armor_bonuses[0].value, 0.10f), "Armor HP bonus is 10%");
    assertTrue(approxEqual(armor_bonuses[1].value, 0.05f), "Armor resist bonus is 5%");
    
    auto skirmish_bonuses = fleetSys.getBonusesForType("skirmish");
    assertTrue(skirmish_bonuses.size() == 2, "Skirmish has 2 bonuses");
    assertTrue(approxEqual(skirmish_bonuses[0].value, 0.15f), "Skirmish speed bonus is 15%");
    
    auto info_bonuses = fleetSys.getBonusesForType("information");
    assertTrue(info_bonuses.size() == 2, "Information has 2 bonuses");
    assertTrue(approxEqual(info_bonuses[0].value, 0.20f), "Info targeting range bonus is 20%");
    
    // Update applies bonuses to FleetMembership components
    fleetSys.update(1.0f);
    auto* fm_fc = fc->getComponent<components::FleetMembership>();
    assertTrue(!fm_fc->active_bonuses.empty(), "FC has active bonuses after update");
    assertTrue(fm_fc->active_bonuses.find("armor_hp_bonus") != fm_fc->active_bonuses.end(),
               "FC has armor_hp_bonus");
}

void testFleetBroadcastTarget() {
    std::cout << "\n=== Fleet Broadcast Target ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    addComp<components::Target>(fc);
    addComp<components::Ship>(fc);
    
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    addComp<components::Target>(p2);
    addComp<components::Ship>(p2);
    
    auto* enemy = world.createEntity("enemy_1");
    addComp<components::Health>(enemy);
    
    std::string fleet_id = fleetSys.createFleet("fc", "Combat Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    
    // Broadcast target
    int notified = fleetSys.broadcastTarget(fleet_id, "fc", "enemy_1");
    assertTrue(notified == 2, "2 fleet members notified of target");
    
    // Both FC and pilot_2 should be locking
    auto* fc_target = fc->getComponent<components::Target>();
    assertTrue(fc_target->locking_targets.find("enemy_1") != fc_target->locking_targets.end(),
               "FC started locking broadcast target");
    
    auto* p2_target = p2->getComponent<components::Target>();
    assertTrue(p2_target->locking_targets.find("enemy_1") != p2_target->locking_targets.end(),
               "pilot_2 started locking broadcast target");
    
    // Broadcasting nonexistent target returns 0
    int none = fleetSys.broadcastTarget(fleet_id, "fc", "nonexistent");
    assertTrue(none == 0, "Broadcast nonexistent target returns 0");
}

void testFleetWarp() {
    std::cout << "\n=== Fleet Warp ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* fc_pos = addComp<components::Position>(fc);
    fc_pos->x = 0; fc_pos->y = 0; fc_pos->z = 0;
    auto* fc_vel = addComp<components::Velocity>(fc);
    fc_vel->max_speed = 1000.0f;
    
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    auto* p2_pos = addComp<components::Position>(p2);
    p2_pos->x = 100; p2_pos->y = 0; p2_pos->z = 0;
    auto* p2_vel = addComp<components::Velocity>(p2);
    p2_vel->max_speed = 800.0f;
    
    std::string fleet_id = fleetSys.createFleet("fc", "Warp Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    
    // FC can fleet warp
    int warped = fleetSys.fleetWarp(fleet_id, "fc", 10000.0f, 0.0f, 0.0f);
    assertTrue(warped == 2, "2 fleet members initiated warp");
    assertTrue(fc_vel->vx > 0.0f, "FC velocity set toward destination");
    assertTrue(p2_vel->vx > 0.0f, "pilot_2 velocity set toward destination");
    
    // Regular member cannot fleet warp
    int no_warp = fleetSys.fleetWarp(fleet_id, "pilot_2", 20000.0f, 0.0f, 0.0f);
    assertTrue(no_warp == 0, "Regular member cannot fleet warp");
}

void testFleetDisbandPermission() {
    std::cout << "\n=== Fleet Disband Permission ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Test Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    
    // Non-FC cannot disband
    assertTrue(!fleetSys.disbandFleet(fleet_id, "pilot_2"), "Non-FC cannot disband fleet");
    assertTrue(fleetSys.getFleetCount() == 1, "Fleet still exists");
    
    // Nonexistent fleet
    assertTrue(!fleetSys.disbandFleet("ghost_fleet", "fc"), "Cannot disband nonexistent fleet");
}

void testFleetMembershipComponent() {
    std::cout << "\n=== FleetMembership Component ===" << std::endl;
    
    ecs::World world;
    
    auto* entity = world.createEntity("test_pilot");
    auto* fm = addComp<components::FleetMembership>(entity);
    fm->fleet_id = "fleet_1";
    fm->role = "Member";
    fm->squad_id = "squad_alpha";
    fm->wing_id = "wing_1";
    fm->active_bonuses["armor_hp_bonus"] = 0.10f;
    
    assertTrue(fm->fleet_id == "fleet_1", "FleetMembership fleet_id correct");
    assertTrue(fm->role == "Member", "FleetMembership role correct");
    assertTrue(fm->squad_id == "squad_alpha", "FleetMembership squad_id correct");
    assertTrue(fm->wing_id == "wing_1", "FleetMembership wing_id correct");
    assertTrue(approxEqual(fm->active_bonuses["armor_hp_bonus"], 0.10f),
               "FleetMembership bonus value correct");
}

// ==================== FleetMoraleSystem Tests ====================

void testFleetMoraleRecordWin() {
    std::cout << "\n=== Fleet Morale Record Win ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    sys.recordWin("cap1");
    assertTrue(sys.getMoraleScore("cap1") > 0.0f, "Morale score positive after win");
    assertTrue(sys.getMoraleState("cap1") == "Steady", "Morale state is Steady after one win");
}

void testFleetMoraleRecordLoss() {
    std::cout << "\n=== Fleet Morale Record Loss ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    sys.recordLoss("cap1");
    assertTrue(sys.getMoraleScore("cap1") < 0.0f, "Morale score negative after loss");
}

void testFleetMoraleMultipleEvents() {
    std::cout << "\n=== Fleet Morale Multiple Events ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    for (int i = 0; i < 10; i++) {
        sys.recordWin("cap1");
    }
    // 10 wins * 1.0 = 10, but let's accumulate: each recordWin increments wins
    // After 10 wins: score = 10 * 1.0 = 10 ... need >= 50
    // Actually wins accumulate: after 10 calls, wins=10, score=10. Need 50 wins for 50.
    for (int i = 0; i < 40; i++) {
        sys.recordWin("cap1");
    }
    assertTrue(sys.getMoraleScore("cap1") >= 50.0f, "Morale >= 50 after 50 wins");
    assertTrue(sys.getMoraleState("cap1") == "Inspired", "Morale state Inspired at high morale");
}

void testFleetMoraleLossStreak() {
    std::cout << "\n=== Fleet Morale Loss Streak ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    for (int i = 0; i < 5; i++) {
        sys.recordLoss("cap1");
    }
    sys.recordShipLost("cap1");
    sys.recordShipLost("cap1");
    // score = 0*1 - 5*1.5 - 2*2.0 + 0 = -11.5 => Doubtful
    std::string state = sys.getMoraleState("cap1");
    assertTrue(state == "Doubtful" || state == "Disengaged",
               "Morale state Doubtful or Disengaged after losses");
}

void testFleetMoraleSavedByPlayer() {
    std::cout << "\n=== Fleet Morale Saved By Player ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    sys.recordSavedByPlayer("cap1");
    assertTrue(sys.getMoraleScore("cap1") > 0.0f, "Saved by player increases morale");
}

void testFleetMoraleMissionTogether() {
    std::cout << "\n=== Fleet Morale Mission Together ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    sys.recordMissionTogether("cap1");
    auto* morale = entity->getComponent<components::FleetMorale>();
    assertTrue(morale != nullptr, "FleetMorale component created");
    assertTrue(morale->missions_together == 1, "Missions together counter incremented");
}

// ==================== FleetCargoSystem Tests ====================

void testFleetCargoAddContributor() {
    std::cout << "\n=== Fleet Cargo Add Contributor ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;
    sys.addContributor("pool1", "ship1");
    sys.recalculate("pool1");
    assertTrue(sys.getTotalCapacity("pool1") == 400, "Total capacity is 400 after adding ship");
}

void testFleetCargoRemoveContributor() {
    std::cout << "\n=== Fleet Cargo Remove Contributor ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;
    sys.addContributor("pool1", "ship1");
    sys.removeContributor("pool1", "ship1");
    assertTrue(sys.getTotalCapacity("pool1") == 0, "Total capacity 0 after removing ship");
}

void testFleetCargoMultipleShips() {
    std::cout << "\n=== Fleet Cargo Multiple Ships ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    for (int i = 0; i < 3; i++) {
        std::string sid = "ship" + std::to_string(i);
        auto* ship = world.createEntity(sid);
        auto* inv = addComp<components::Inventory>(ship);
        inv->max_capacity = 200.0f;
        sys.addContributor("pool1", sid);
    }
    sys.recalculate("pool1");
    assertTrue(sys.getTotalCapacity("pool1") == 600, "Aggregate capacity of 3 ships is 600");
}

void testFleetCargoUsedCapacity() {
    std::cout << "\n=== Fleet Cargo Used Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;
    components::Inventory::Item item;
    item.item_id = "ore1";
    item.name = "Ferrite";
    item.type = "ore";
    item.quantity = 10;
    item.volume = 5.0f;
    inv->items.push_back(item);
    sys.addContributor("pool1", "ship1");
    sys.recalculate("pool1");
    assertTrue(sys.getUsedCapacity("pool1") == 50, "Used capacity reflects items (10*5=50)");
}

void testFleetCargoGetCapacity() {
    std::cout << "\n=== Fleet Cargo Get Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 300.0f;
    sys.addContributor("pool1", "ship1");
    assertTrue(sys.getTotalCapacity("pool1") == 300, "getTotalCapacity query returns 300");
}

// ==================== Fleet Formation System Tests ====================

void testFleetFormationSetFormation() {
    std::cout << "\n=== Fleet Formation: Set Formation ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("leader");
    world.createEntity("wing1");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("leader", FT::Arrow, 0);
    sys.setFormation("wing1", FT::Arrow, 1);

    assertTrue(sys.getFormation("leader") == FT::Arrow, "Leader has Arrow formation");
    assertTrue(sys.getFormation("wing1") == FT::Arrow, "Wing1 has Arrow formation");
}

void testFleetFormationLeaderAtOrigin() {
    std::cout << "\n=== Fleet Formation: Leader At Origin ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("leader");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("leader", FT::Arrow, 0);
    sys.computeOffsets();

    float ox = 0, oy = 0, oz = 0;
    assertTrue(sys.getOffset("leader", ox, oy, oz), "Leader has offset");
    assertTrue(approxEqual(ox, 0.0f) && approxEqual(oy, 0.0f) && approxEqual(oz, 0.0f),
               "Leader offset is (0,0,0)");
}

void testFleetFormationArrowOffsets() {
    std::cout << "\n=== Fleet Formation: Arrow Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("s0");
    world.createEntity("s1");
    world.createEntity("s2");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("s0", FT::Arrow, 0);
    sys.setFormation("s1", FT::Arrow, 1);
    sys.setFormation("s2", FT::Arrow, 2);
    sys.computeOffsets();

    float ox1 = 0, oy1 = 0, oz1 = 0;
    sys.getOffset("s1", ox1, oy1, oz1);
    assertTrue(ox1 < 0.0f, "Slot 1 is to the left");
    assertTrue(oz1 < 0.0f, "Slot 1 is behind");

    float ox2 = 0, oy2 = 0, oz2 = 0;
    sys.getOffset("s2", ox2, oy2, oz2);
    assertTrue(ox2 > 0.0f, "Slot 2 is to the right");
    assertTrue(oz2 < 0.0f, "Slot 2 is behind");
}

void testFleetFormationLineOffsets() {
    std::cout << "\n=== Fleet Formation: Line Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("s0");
    world.createEntity("s1");
    world.createEntity("s2");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("s0", FT::Line, 0);
    sys.setFormation("s1", FT::Line, 1);
    sys.setFormation("s2", FT::Line, 2);
    sys.computeOffsets();

    float ox0 = 0, oy0 = 0, oz0 = 0;
    sys.getOffset("s0", ox0, oy0, oz0);
    assertTrue(approxEqual(ox0, 0.0f) && approxEqual(oz0, 0.0f), "Line slot 0 at origin");

    float ox1 = 0, oy1 = 0, oz1 = 0;
    sys.getOffset("s1", ox1, oy1, oz1);
    assertTrue(approxEqual(ox1, 0.0f), "Line slot 1 aligned in X");
    assertTrue(oz1 < 0.0f, "Line slot 1 behind leader");

    float ox2 = 0, oy2 = 0, oz2 = 0;
    sys.getOffset("s2", ox2, oy2, oz2);
    assertTrue(oz2 < oz1, "Line slot 2 further behind than slot 1");
}

void testFleetFormationDiamondOffsets() {
    std::cout << "\n=== Fleet Formation: Diamond Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    for (int i = 0; i < 4; ++i)
        world.createEntity("d" + std::to_string(i));

    using FT = components::FleetFormation::FormationType;
    for (int i = 0; i < 4; ++i)
        sys.setFormation("d" + std::to_string(i), FT::Diamond, i);
    sys.computeOffsets();

    float ox0 = 0, oy0 = 0, oz0 = 0;
    sys.getOffset("d0", ox0, oy0, oz0);
    assertTrue(approxEqual(ox0, 0.0f) && approxEqual(oz0, 0.0f), "Diamond slot 0 at origin");

    float ox1 = 0, oy1 = 0, oz1 = 0;
    sys.getOffset("d1", ox1, oy1, oz1);
    assertTrue(ox1 < 0.0f, "Diamond slot 1 to the left");

    float ox2 = 0, oy2 = 0, oz2 = 0;
    sys.getOffset("d2", ox2, oy2, oz2);
    assertTrue(ox2 > 0.0f, "Diamond slot 2 to the right");

    float ox3 = 0, oy3 = 0, oz3 = 0;
    sys.getOffset("d3", ox3, oy3, oz3);
    assertTrue(approxEqual(ox3, 0.0f), "Diamond slot 3 centered in X");
    assertTrue(oz3 < oz1, "Diamond slot 3 behind slots 1 & 2");
}

void testFleetFormationNoneType() {
    std::cout << "\n=== Fleet Formation: None Type ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("e1");

    assertTrue(sys.getFormation("e1") == components::FleetFormation::FormationType::None,
               "Entity without formation returns None");

    float ox = 0, oy = 0, oz = 0;
    assertTrue(!sys.getOffset("e1", ox, oy, oz), "No offset for entity without component");
}

// ==================== Player Fleet Tests ====================

void testPlayerFleetCreate() {
    std::cout << "\n=== Player Fleet: Create ===" << std::endl;
    ecs::World world;
    systems::FleetSystem fleetSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    addComp<components::Position>(player);

    std::string fid = fleetSys.createPlayerFleet("player1", "My Fleet");
    assertTrue(!fid.empty(), "Player fleet created successfully");
    assertTrue(fleetSys.isPlayerFleet(fid), "Fleet is marked as player fleet");
    assertTrue(fleetSys.getMemberCount(fid) == 1, "Fleet has 1 member (player)");
}

void testPlayerFleetAssignCaptains() {
    std::cout << "\n=== Player Fleet: Assign Captains ===" << std::endl;
    ecs::World world;
    systems::FleetSystem fleetSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    addComp<components::Position>(player);

    std::string fid = fleetSys.createPlayerFleet("player1");

    // Add 4 captains
    for (int i = 1; i <= 4; ++i) {
        std::string cid = "captain" + std::to_string(i);
        auto* cap = world.createEntity(cid);
        addComp<components::Position>(cap);
        bool ok = fleetSys.assignCaptain(fid, cid, "Captain " + std::to_string(i));
        assertTrue(ok, "Captain " + std::to_string(i) + " assigned");
    }
    assertTrue(fleetSys.getMemberCount(fid) == 5, "Fleet has 5 members total");
}

void testPlayerFleetMaxCap() {
    std::cout << "\n=== Player Fleet: Max 5 Ships ===" << std::endl;
    ecs::World world;
    systems::FleetSystem fleetSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    addComp<components::Position>(player);

    std::string fid = fleetSys.createPlayerFleet("player1");

    for (int i = 1; i <= 4; ++i) {
        std::string cid = "captain" + std::to_string(i);
        auto* cap = world.createEntity(cid);
        addComp<components::Position>(cap);
        fleetSys.assignCaptain(fid, cid, "Captain " + std::to_string(i));
    }

    // Adding another captain should fail when fleet is at capacity (1 player + 4 captains = 5)
    auto* extra = world.createEntity("captain5");
    addComp<components::Position>(extra);
    bool ok = fleetSys.assignCaptain(fid, "captain5", "Extra Captain");
    assertTrue(!ok, "Cannot add 5th captain (fleet full at 5)");
    assertTrue(fleetSys.getMemberCount(fid) == 5, "Fleet still 5 members");
}

void testPlayerFleetNotPlayerFleet() {
    std::cout << "\n=== Player Fleet: assignCaptain on non-player fleet ===" << std::endl;
    ecs::World world;
    systems::FleetSystem fleetSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    addComp<components::Position>(player);

    // Create a normal fleet (not player fleet)
    std::string fid = fleetSys.createFleet("player1", "Normal Fleet");
    assertTrue(!fleetSys.isPlayerFleet(fid), "Normal fleet is not player fleet");

    auto* cap = world.createEntity("cap1");
    addComp<components::Position>(cap);
    bool ok = fleetSys.assignCaptain(fid, "cap1", "Captain");
    assertTrue(!ok, "assignCaptain fails on non-player fleet");
}

// ==================== Phase 11: Fleet Progression System Tests ====================

void testFleetProgressionEarlyStage() {
    std::cout << "\n=== Fleet Progression Early Stage ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::Early,
               "Fleet starts in Early stage");
    assertTrue(sys.getMaxShips("fleet1") == 5, "Early stage max ships is 5");
    assertTrue(sys.getMaxWings("fleet1") == 1, "Early stage max wings is 1");
}

void testFleetProgressionMidStage() {
    std::cout << "\n=== Fleet Progression Mid Stage ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    sys.addExperience("fleet1", 150.0f);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::Mid,
               "Fleet advances to Mid stage at 100+ XP");
    assertTrue(sys.getMaxShips("fleet1") == 15, "Mid stage max ships is 15");
    assertTrue(sys.getMaxWings("fleet1") == 3, "Mid stage max wings is 3");
}

void testFleetProgressionEndStage() {
    std::cout << "\n=== Fleet Progression End Stage ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    sys.addExperience("fleet1", 600.0f);
    assertTrue(sys.getStage("fleet1") == components::FleetProgression::Stage::End,
               "Fleet advances to End stage at 500+ XP");
    assertTrue(sys.getMaxShips("fleet1") == 25, "End stage max ships is 25");
    assertTrue(sys.getMaxWings("fleet1") == 5, "End stage max wings is 5");
}

void testFleetProgressionCanAddShip() {
    std::cout << "\n=== Fleet Progression Can Add Ship ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    auto* prog = addComp<components::FleetProgression>(fleet);
    prog->current_ship_count = 4;
    assertTrue(sys.canAddShip("fleet1"), "Can add ship when below max");
    assertTrue(sys.addShipToFleet("fleet1"), "Successfully added ship");
    assertTrue(!sys.canAddShip("fleet1"), "Cannot add ship at max (5)");
}

void testFleetProgressionRemoveShip() {
    std::cout << "\n=== Fleet Progression Remove Ship ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    auto* prog = addComp<components::FleetProgression>(fleet);
    prog->current_ship_count = 3;
    sys.removeShipFromFleet("fleet1");
    assertTrue(prog->current_ship_count == 2, "Ship count decremented");
}

void testFleetProgressionWingRoleUnlock() {
    std::cout << "\n=== Fleet Progression Wing Role Unlock ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "mining"), "Mining wing locked in Early");
    sys.addExperience("fleet1", 150.0f);
    sys.update(0.0f);
    assertTrue(sys.isWingRoleUnlocked("fleet1", "mining"), "Mining wing unlocked in Mid");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "combat"), "Combat wing unlocked in Mid");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "logistics"), "Logistics wing unlocked in Mid");
    assertTrue(!sys.isWingRoleUnlocked("fleet1", "salvage"), "Salvage wing locked in Mid");
}

void testFleetProgressionEndStageAllRoles() {
    std::cout << "\n=== Fleet Progression End Stage All Roles ===" << std::endl;
    ecs::World world;
    systems::FleetProgressionSystem sys(&world);
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);
    sys.addExperience("fleet1", 600.0f);
    sys.update(0.0f);
    assertTrue(sys.isWingRoleUnlocked("fleet1", "salvage"), "Salvage wing unlocked in End");
    assertTrue(sys.isWingRoleUnlocked("fleet1", "construction"), "Construction wing unlocked in End");
}

// ==================== Phase 11: Fleet Cargo Ship Loss Tests ====================

void testFleetCargoShipLoss() {
    std::cout << "\n=== Fleet Cargo Ship Loss ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship1 = world.createEntity("ship1");
    auto* inv1 = addComp<components::Inventory>(ship1);
    inv1->max_capacity = 400.0f;
    auto* ship2 = world.createEntity("ship2");
    auto* inv2 = addComp<components::Inventory>(ship2);
    inv2->max_capacity = 300.0f;
    sys.addContributor("pool1", "ship1");
    sys.addContributor("pool1", "ship2");
    assertTrue(sys.getTotalCapacity("pool1") == 700, "Total capacity before loss is 700");
    uint64_t lost = sys.handleShipLoss("pool1", "ship1");
    assertTrue(lost == 400, "Lost capacity is 400");
    assertTrue(sys.getTotalCapacity("pool1") == 300, "Total capacity after loss is 300");
}

void testFleetCargoScaledCapacity() {
    std::cout << "\n=== Fleet Cargo Scaled Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 1000.0f;
    sys.addContributor("pool1", "ship1");
    // logistics=1.2, skill=1.1, morale=1.0 → 1000 * 1.32 = 1320
    uint64_t scaled = sys.getScaledCapacity("pool1", 1.2f, 1.1f, 1.0f);
    assertTrue(scaled == 1320, "Scaled capacity is 1320 (1000*1.2*1.1*1.0)");
}

void testFleetCargoScaledWithMoralePenalty() {
    std::cout << "\n=== Fleet Cargo Scaled With Morale Penalty ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 1000.0f;
    sys.addContributor("pool1", "ship1");
    // logistics=1.0, skill=1.0, morale=0.75 → 1000 * 0.75 = 750
    uint64_t scaled = sys.getScaledCapacity("pool1", 1.0f, 1.0f, 0.75f);
    assertTrue(scaled == 750, "Scaled capacity penalized by low morale (750)");
}

// ==================== Phase 11: Fleet Warp Formation System Tests ====================

void testFleetWarpFormationBeginEnd() {
    std::cout << "\n=== Fleet Warp Formation Begin/End ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* ship = world.createEntity("ship1");
    addComp<components::FleetWarpState>(ship);
    sys.beginFleetWarp("ship1");
    assertTrue(sys.isInFleetWarp("ship1"), "Ship is in fleet warp");
    sys.endFleetWarp("ship1");
    assertTrue(!sys.isInFleetWarp("ship1"), "Ship no longer in fleet warp");
}

void testFleetWarpFormationShipClassSelection() {
    std::cout << "\n=== Fleet Warp Formation Ship Class Selection ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* frig = world.createEntity("frig1");
    addComp<components::FleetWarpState>(frig);
    sys.selectFormationByShipClass("frig1", "Frigate");
    auto* ws = frig->getComponent<components::FleetWarpState>();
    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::TightEchelon,
               "Frigates use TightEchelon");

    auto* cruiser = world.createEntity("cruiser1");
    addComp<components::FleetWarpState>(cruiser);
    sys.selectFormationByShipClass("cruiser1", "Cruiser");
    ws = cruiser->getComponent<components::FleetWarpState>();
    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::LooseDiamond,
               "Cruisers use LooseDiamond");

    auto* cap = world.createEntity("cap1");
    addComp<components::FleetWarpState>(cap);
    sys.selectFormationByShipClass("cap1", "Capital");
    ws = cap->getComponent<components::FleetWarpState>();
    assertTrue(ws->warp_formation == components::FleetWarpState::WarpFormationType::WideCapital,
               "Capitals use WideCapital");
}

void testFleetWarpFormationBreathing() {
    std::cout << "\n=== Fleet Warp Formation Breathing ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* ship = world.createEntity("ship1");
    addComp<components::FleetWarpState>(ship);
    sys.beginFleetWarp("ship1");
    float offset0 = sys.getBreathingOffset("ship1");
    assertTrue(approxEqual(offset0, 0.0f, 0.01f), "Breathing starts at 0");
    sys.update(5.0f); // advance time
    float offset1 = sys.getBreathingOffset("ship1");
    // After 5s with default breathing_frequency=0.03, phase should have advanced
    assertTrue(!approxEqual(offset0, offset1, 0.001f), "Breathing changes over time");
}

void testFleetWarpFormationDistortion() {
    std::cout << "\n=== Fleet Warp Formation Distortion ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* frig = world.createEntity("frig1");
    addComp<components::FleetWarpState>(frig);
    sys.selectFormationByShipClass("frig1", "Frigate");
    float frig_bend = sys.getDistortionBend("frig1");

    auto* cap = world.createEntity("cap1");
    addComp<components::FleetWarpState>(cap);
    sys.selectFormationByShipClass("cap1", "Capital");
    float cap_bend = sys.getDistortionBend("cap1");

    assertTrue(cap_bend > frig_bend, "Capitals bend space more than frigates");
}

void testFleetWarpFormationLeaderAtOrigin() {
    std::cout << "\n=== Fleet Warp Formation Leader At Origin ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* ship = world.createEntity("ship1");
    addComp<components::FleetWarpState>(ship);
    sys.selectFormationByShipClass("ship1", "Cruiser");
    float ox, oy, oz;
    sys.computeWarpOffset("ship1", 0, ox, oy, oz);
    assertTrue(approxEqual(ox, 0.0f) && approxEqual(oy, 0.0f) && approxEqual(oz, 0.0f),
               "Leader (slot 0) at origin");
}

void testFleetWarpFormationSlotOffsets() {
    std::cout << "\n=== Fleet Warp Formation Slot Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetWarpFormationSystem sys(&world);
    auto* ship = world.createEntity("ship1");
    addComp<components::FleetWarpState>(ship);
    sys.selectFormationByShipClass("ship1", "Cruiser");
    float ox, oy, oz;
    sys.computeWarpOffset("ship1", 1, ox, oy, oz);
    assertTrue(ox != 0.0f || oz != 0.0f, "Non-leader slots have non-zero offsets");
}

// ==================== Phase 11: Fleet Civilization Tests ====================

void testFleetCivilizationThresholdNotMet() {
    std::cout << "\n=== Fleet Civilization Threshold Not Met ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    auto* civ = addComp<components::FleetCivilization>(fleet);
    assertTrue(!civ->isCivilizationThresholdMet(), "Threshold not met initially");
}

void testFleetCivilizationThresholdMet() {
    std::cout << "\n=== Fleet Civilization Threshold Met ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    auto* civ = addComp<components::FleetCivilization>(fleet);
    civ->has_stable_logistics = true;
    civ->has_loyal_captains = true;
    civ->has_fleet_history = true;
    civ->has_fleet_industry = true;
    assertTrue(civ->isCivilizationThresholdMet(), "Threshold met when all criteria true");
}

// ==================== Fleet History System Tests ====================

void testFleetHistoryEmpty() {
    std::cout << "\n=== Fleet History Empty ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::FleetHistory>(e);

    systems::FleetHistorySystem sys(&world);
    assertTrue(sys.getEventCount("fleet1") == 0, "No events initially");
}

void testFleetHistoryRecordEvent() {
    std::cout << "\n=== Fleet History Record Event ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet2");
    addComp<components::FleetHistory>(e);

    systems::FleetHistorySystem sys(&world);
    sys.recordEvent("fleet2", "mission_complete", "Cleared pirates", 100.0f, "mission_1");
    assertTrue(sys.getEventCount("fleet2") == 1, "One event recorded");
    auto history = sys.getHistory("fleet2");
    assertTrue(history[0].event_type == "mission_complete", "Correct event type");
}

void testFleetHistoryMaxEvents() {
    std::cout << "\n=== Fleet History Max Events ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet3");
    auto* hist = addComp<components::FleetHistory>(e);
    hist->max_events = 5;

    systems::FleetHistorySystem sys(&world);
    for (int i = 0; i < 10; i++) {
        sys.recordEvent("fleet3", "event", "desc " + std::to_string(i), static_cast<float>(i));
    }
    assertTrue(sys.getEventCount("fleet3") == 5, "Events trimmed to max");
}

void testFleetHistoryByType() {
    std::cout << "\n=== Fleet History By Type ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet4");
    addComp<components::FleetHistory>(e);

    systems::FleetHistorySystem sys(&world);
    sys.recordEvent("fleet4", "mission_complete", "Mission 1", 1.0f);
    sys.recordEvent("fleet4", "ship_lost", "Lost a frigate", 2.0f);
    sys.recordEvent("fleet4", "mission_complete", "Mission 2", 3.0f);

    auto missions = sys.getEventsByType("fleet4", "mission_complete");
    assertTrue(static_cast<int>(missions.size()) == 2, "Two mission_complete events");
    auto losses = sys.getEventsByType("fleet4", "ship_lost");
    assertTrue(static_cast<int>(losses.size()) == 1, "One ship_lost event");
}

// ==================== FleetDoctrineSystem Tests ====================

void testFleetDoctrineCreate() {
    std::cout << "\n=== Fleet Doctrine: Create ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    assertTrue(sys.createDoctrine("fleet1", "doctrine1", "Shield Fleet"), "Doctrine created");
    assertTrue(!sys.createDoctrine("fleet1", "doctrine2", "Another"), "Duplicate doctrine rejected");
    assertTrue(sys.getDoctrineName("fleet1") == "Shield Fleet", "Doctrine name matches");
}

void testFleetDoctrineAddSlot() {
    std::cout << "\n=== Fleet Doctrine: Add Slot ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Armor Fleet");
    assertTrue(sys.addSlot("fleet1", "DPS", "Battleship", 3, 5, true), "DPS slot added");
    assertTrue(sys.addSlot("fleet1", "Logistics", "Cruiser", 2, 3, true), "Logi slot added");
    assertTrue(sys.addSlot("fleet1", "Tackle", "Frigate", 1, 4, false), "Tackle slot added");
    assertTrue(!sys.addSlot("fleet1", "DPS", "Frigate", 1, 2, false), "Duplicate role rejected");
    assertTrue(sys.getSlotCount("fleet1") == 3, "Slot count is 3");
}

void testFleetDoctrineRemoveSlot() {
    std::cout << "\n=== Fleet Doctrine: Remove Slot ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 3, 5, true);
    sys.addSlot("fleet1", "Logistics", "Cruiser", 2, 3, true);
    assertTrue(sys.removeSlot("fleet1", "DPS"), "Slot removed");
    assertTrue(sys.getSlotCount("fleet1") == 1, "Slot count after removal");
    assertTrue(!sys.removeSlot("fleet1", "nonexistent"), "Nonexistent slot removal fails");
}

void testFleetDoctrineAssignShip() {
    std::cout << "\n=== Fleet Doctrine: Assign Ship ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 2, 3, true);
    assertTrue(sys.assignShip("fleet1", "DPS"), "Ship assigned to DPS");
    assertTrue(sys.assignShip("fleet1", "DPS"), "Second ship assigned");
    assertTrue(sys.assignShip("fleet1", "DPS"), "Third ship assigned (max)");
    assertTrue(!sys.assignShip("fleet1", "DPS"), "Fourth ship rejected (over max)");
}

void testFleetDoctrineUnassignShip() {
    std::cout << "\n=== Fleet Doctrine: Unassign Ship ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 1, 3, true);
    sys.assignShip("fleet1", "DPS");
    sys.assignShip("fleet1", "DPS");
    assertTrue(sys.unassignShip("fleet1", "DPS"), "Ship unassigned");
    assertTrue(sys.unassignShip("fleet1", "DPS"), "Second ship unassigned");
    assertTrue(!sys.unassignShip("fleet1", "DPS"), "Unassign from empty fails");
}

void testFleetDoctrineReadiness() {
    std::cout << "\n=== Fleet Doctrine: Readiness ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 2, 5, true);
    sys.addSlot("fleet1", "Logistics", "Cruiser", 1, 3, true);
    sys.addSlot("fleet1", "Tackle", "Frigate", 0, 4, false);  // optional

    // Initially 0% ready (0 of 2 required slots filled)
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getReadiness("fleet1"), 0.0f), "Initially not ready");
    assertTrue(!sys.isReady("fleet1"), "isReady false initially");

    // Fill DPS (still missing Logistics)
    sys.assignShip("fleet1", "DPS");
    sys.assignShip("fleet1", "DPS");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getReadiness("fleet1"), 0.5f), "50% ready with DPS filled");

    // Fill Logistics
    sys.assignShip("fleet1", "Logistics");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getReadiness("fleet1"), 1.0f), "100% ready");
    assertTrue(sys.isReady("fleet1"), "isReady true when all required filled");
}

void testFleetDoctrineLock() {
    std::cout << "\n=== Fleet Doctrine: Lock ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 2, 5, true);

    sys.lockDoctrine("fleet1", true);
    assertTrue(sys.isLocked("fleet1"), "Doctrine is locked");
    assertTrue(!sys.addSlot("fleet1", "Logistics", "Cruiser", 1, 3, true), "Cannot add slot when locked");
    assertTrue(!sys.removeSlot("fleet1", "DPS"), "Cannot remove slot when locked");

    sys.lockDoctrine("fleet1", false);
    assertTrue(!sys.isLocked("fleet1"), "Doctrine unlocked");
    assertTrue(sys.addSlot("fleet1", "Logistics", "Cruiser", 1, 3, true), "Can add slot after unlock");
}

void testFleetDoctrineShipCounts() {
    std::cout << "\n=== Fleet Doctrine: Ship Counts ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 2, 5, true);
    sys.addSlot("fleet1", "Logistics", "Cruiser", 1, 3, true);
    sys.assignShip("fleet1", "DPS");
    sys.assignShip("fleet1", "DPS");
    sys.assignShip("fleet1", "Logistics");
    sys.update(0.0f);

    assertTrue(sys.getTargetShipCount("fleet1") == 8, "Target ship count is max_count sum (5+3)");
    assertTrue(sys.getCurrentShipCount("fleet1") == 3, "Current ship count is 3");
}

void testFleetDoctrineMissing() {
    std::cout << "\n=== Fleet Doctrine: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetDoctrineSystem sys(&world);
    assertTrue(approxEqual(sys.getReadiness("nonexistent"), 0.0f), "Default readiness for missing");
    assertTrue(sys.getSlotCount("nonexistent") == 0, "Default slot count for missing");
    assertTrue(!sys.isReady("nonexistent"), "Default not ready for missing");
    assertTrue(sys.getDoctrineName("nonexistent").empty(), "Default name empty for missing");
}

void testFleetDoctrineComponentDefaults() {
    std::cout << "\n=== Fleet Doctrine: Component Defaults ===" << std::endl;
    components::FleetDoctrine doctrine;
    assertTrue(doctrine.slots.empty(), "Default slots empty");
    assertTrue(approxEqual(doctrine.readiness, 0.0f), "Default readiness is 0.0");
    assertTrue(!doctrine.is_locked, "Default not locked");
    assertTrue(doctrine.total_ship_target == 0, "Default target 0");
    assertTrue(doctrine.current_ship_count == 0, "Default current 0");
}

// ==================== FleetSquadSystem Tests ====================

void testFleetSquadCreate() {
    std::cout << "\n=== Fleet Squad: Create ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    assertTrue(sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault), "Squad created");
    assertTrue(sys.getMemberCount("squad1") == 1, "Leader is first member");
    assertTrue(sys.getLeaderId("squad1") == "leader1", "Leader ID correct");
    assertTrue(sys.getRole("squad1") == "assault", "Role is assault");
    assertTrue(!sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault), "Duplicate create rejected");
}

void testFleetSquadDissolve() {
    std::cout << "\n=== Fleet Squad: Dissolve ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    assertTrue(sys.dissolveSquad("squad1"), "Squad dissolved");
    assertTrue(sys.getMemberCount("squad1") == 0, "No members after dissolve");
    assertTrue(!sys.dissolveSquad("squad1"), "Dissolve on missing fails");
}

void testFleetSquadAddMember() {
    std::cout << "\n=== Fleet Squad: Add Member ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    assertTrue(sys.addMember("squad1", "member2"), "Member added");
    assertTrue(sys.getMemberCount("squad1") == 2, "2 members now");
    assertTrue(!sys.addMember("squad1", "member2"), "Duplicate member rejected");
}

void testFleetSquadMaxMembers() {
    std::cout << "\n=== Fleet Squad: Max Members ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    sys.addMember("squad1", "m2");
    sys.addMember("squad1", "m3");
    sys.addMember("squad1", "m4");
    sys.addMember("squad1", "m5");
    assertTrue(sys.getMemberCount("squad1") == 5, "Squad is full");
    assertTrue(!sys.addMember("squad1", "m6"), "Cannot exceed max members");
}

void testFleetSquadRemoveMember() {
    std::cout << "\n=== Fleet Squad: Remove Member ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    sys.addMember("squad1", "member2");
    assertTrue(sys.removeMember("squad1", "member2"), "Member removed");
    assertTrue(sys.getMemberCount("squad1") == 1, "1 member remaining");
    assertTrue(!sys.removeMember("squad1", "member2"), "Remove missing member fails");
}

void testFleetSquadRemoveLeader() {
    std::cout << "\n=== Fleet Squad: Remove Leader ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    sys.addMember("squad1", "member2");
    sys.addMember("squad1", "member3");
    assertTrue(sys.removeMember("squad1", "leader1"), "Leader removed");
    assertTrue(sys.getLeaderId("squad1") == "member2", "First remaining member promoted to leader");
    assertTrue(sys.getMemberCount("squad1") == 2, "2 members remaining");
}

void testFleetSquadFormation() {
    std::cout << "\n=== Fleet Squad: Formation ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    assertTrue(sys.getFormation("squad1") == "line", "Default formation is line");
    sys.setFormation("squad1", components::FleetSquad::SquadFormation::Wedge);
    assertTrue(sys.getFormation("squad1") == "wedge", "Formation changed to wedge");
    sys.setFormation("squad1", components::FleetSquad::SquadFormation::Circle);
    assertTrue(sys.getFormation("squad1") == "circle", "Formation changed to circle");
}

void testFleetSquadRole() {
    std::cout << "\n=== Fleet Squad: Role ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    assertTrue(sys.getRole("squad1") == "assault", "Default role is assault");
    sys.setRole("squad1", components::FleetSquad::SquadRole::Defense);
    assertTrue(sys.getRole("squad1") == "defense", "Role changed to defense");
    sys.setRole("squad1", components::FleetSquad::SquadRole::Scout);
    assertTrue(sys.getRole("squad1") == "scout", "Role changed to scout");
}

void testFleetSquadEffectiveness() {
    std::cout << "\n=== Fleet Squad: Effectiveness ===" << std::endl;
    ecs::World world;
    world.createEntity("squad1");

    systems::FleetSquadSystem sys(&world);
    sys.createSquad("squad1", "alpha", "leader1", components::FleetSquad::SquadRole::Assault);
    sys.addMember("squad1", "m2");
    sys.addMember("squad1", "m3");
    sys.update(0.0f);
    // 3 members: 1.0 + 0.05 * (3-1) = 1.10
    assertTrue(approxEqual(sys.getEffectiveness("squad1"), 1.10f), "Effectiveness with 3 members");
    assertTrue(approxEqual(sys.getCohesion("squad1"), 1.0f), "Cohesion is 1.0 for active squad");
    assertTrue(sys.isSquadActive("squad1"), "Squad is active");
}

void testFleetSquadMissing() {
    std::cout << "\n=== Fleet Squad: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetSquadSystem sys(&world);
    assertTrue(sys.getMemberCount("nonexistent") == 0, "Default member count for missing");
    assertTrue(sys.getLeaderId("nonexistent") == "", "Default leader for missing");
    assertTrue(sys.getRole("nonexistent") == "", "Default role for missing");
    assertTrue(sys.getFormation("nonexistent") == "", "Default formation for missing");
    assertTrue(approxEqual(sys.getCohesion("nonexistent"), 0.0f), "Default cohesion for missing");
    assertTrue(approxEqual(sys.getEffectiveness("nonexistent"), 0.0f), "Default effectiveness for missing");
    assertTrue(!sys.isSquadActive("nonexistent"), "Default inactive for missing");
}

// ===== FleetMoraleResolution System Tests =====

void testFleetMoraleResCreate() {
    std::cout << "\n=== FleetMoraleRes: Create ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    assertTrue(sys.initializeFleet("fleet1"), "Init fleet succeeds");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 50.0f), "Initial morale is 50");
    assertTrue(approxEqual(sys.getIdeologyAlignment("fleet1"), 0.5f), "Initial ideology is 0.5");
    assertTrue(!sys.isCrisisActive("fleet1"), "No crisis initially");
    assertTrue(sys.getDepartures("fleet1") == 0, "No departures initially");
    assertTrue(sys.getResolutionCount("fleet1") == 0, "No resolutions initially");
}

void testFleetMoraleResCrisis() {
    std::cout << "\n=== FleetMoraleRes: Crisis ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    assertTrue(sys.triggerCrisis("fleet1"), "Trigger crisis succeeds");
    assertTrue(sys.isCrisisActive("fleet1"), "Crisis is active");
}

void testFleetMoraleResResolve() {
    std::cout << "\n=== FleetMoraleRes: Resolve ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.triggerCrisis("fleet1");
    assertTrue(sys.resolveWithMethod("fleet1", "Compromise"), "Resolve with Compromise");
    assertTrue(!sys.isCrisisActive("fleet1"), "Crisis resolved");
    assertTrue(sys.getResolutionCount("fleet1") == 1, "Resolution count is 1");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 60.0f), "Morale boosted by Compromise (+10)");

    sys.triggerCrisis("fleet1");
    assertTrue(sys.resolveWithMethod("fleet1", "Mediation"), "Resolve with Mediation");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 80.0f), "Morale boosted by Mediation (+20)");
    assertTrue(sys.getResolutionCount("fleet1") == 2, "Resolution count is 2");
    assertTrue(!sys.resolveWithMethod("fleet1", "InvalidMethod"), "Invalid method fails");
}

void testFleetMoraleResAdjust() {
    std::cout << "\n=== FleetMoraleRes: Adjust ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    assertTrue(sys.adjustMorale("fleet1", 30.0f), "Adjust morale up");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 80.0f), "Morale is 80");
    assertTrue(sys.adjustMorale("fleet1", 50.0f), "Adjust morale past max");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 100.0f), "Morale clamped at 100");
    assertTrue(sys.adjustMorale("fleet1", -150.0f), "Adjust morale below min");
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 0.0f), "Morale clamped at 0");
}

void testFleetMoraleResIdeology() {
    std::cout << "\n=== FleetMoraleRes: Ideology ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    assertTrue(sys.setIdeologyAlignment("fleet1", 0.8f), "Set ideology 0.8");
    assertTrue(approxEqual(sys.getIdeologyAlignment("fleet1"), 0.8f), "Ideology is 0.8");
    assertTrue(sys.setIdeologyAlignment("fleet1", 2.0f), "Set ideology past max");
    assertTrue(approxEqual(sys.getIdeologyAlignment("fleet1"), 1.0f), "Ideology clamped at 1.0");
    assertTrue(sys.setIdeologyAlignment("fleet1", -1.0f), "Set ideology below min");
    assertTrue(approxEqual(sys.getIdeologyAlignment("fleet1"), 0.0f), "Ideology clamped at 0.0");
}

void testFleetMoraleResRecovery() {
    std::cout << "\n=== FleetMoraleRes: Recovery ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.adjustMorale("fleet1", -20.0f); // morale = 30
    sys.setIdeologyAlignment("fleet1", 1.0f);
    sys.update(5.0f); // recovery = 1.0 * 5.0 * 1.0 = 5.0
    assertTrue(approxEqual(sys.getFleetMorale("fleet1"), 35.0f), "Morale recovered to 35");
}

void testFleetMoraleResAutoFracture() {
    std::cout << "\n=== FleetMoraleRes: AutoFracture ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.adjustMorale("fleet1", -35.0f); // morale = 15, below threshold of 20
    sys.update(0.1f); // should auto-trigger crisis
    assertTrue(sys.isCrisisActive("fleet1"), "Crisis auto-triggered below threshold");
}

void testFleetMoraleResDeparture() {
    std::cout << "\n=== FleetMoraleRes: Departure ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.triggerCrisis("fleet1");
    sys.update(61.0f); // exceeds max_crisis_duration of 60
    assertTrue(sys.getDepartures("fleet1") == 1, "One departure after timeout");
    assertTrue(!sys.isCrisisActive("fleet1"), "Crisis deactivated after departure");
    assertTrue(sys.getFleetMorale("fleet1") < 50.0f, "Morale dropped after departure");
}

void testFleetMoraleResMultiple() {
    std::cout << "\n=== FleetMoraleRes: Multiple ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeFleet("fleet1");
    sys.triggerCrisis("fleet1");
    sys.resolveWithMethod("fleet1", "Vote"); // +15
    sys.triggerCrisis("fleet1");
    sys.resolveWithMethod("fleet1", "AuthorityOverride"); // +5
    assertTrue(sys.getResolutionCount("fleet1") == 2, "Two resolutions applied");
    assertTrue(sys.getFleetMorale("fleet1") > 50.0f, "Morale increased from resolutions");
}

void testFleetMoraleResMissing() {
    std::cout << "\n=== FleetMoraleRes: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleResolutionSystem sys(&world);
    assertTrue(!sys.initializeFleet("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.triggerCrisis("nonexistent"), "Trigger fails on missing");
    assertTrue(!sys.resolveWithMethod("nonexistent", "Compromise"), "Resolve fails on missing");
    assertTrue(!sys.adjustMorale("nonexistent", 10.0f), "Adjust fails on missing");
    assertTrue(!sys.setIdeologyAlignment("nonexistent", 0.5f), "Set ideology fails on missing");
    assertTrue(approxEqual(sys.getFleetMorale("nonexistent"), 0.0f), "0 morale on missing");
    assertTrue(!sys.isCrisisActive("nonexistent"), "No crisis on missing");
    assertTrue(sys.getDepartures("nonexistent") == 0, "0 departures on missing");
    assertTrue(sys.getResolutionCount("nonexistent") == 0, "0 resolutions on missing");
}

// ==================== FleetSupplyLine System Tests ====================

void testSupplyLineCreate() {
    std::cout << "\n=== SupplyLine: Create ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    assertTrue(sys.initializeSupplyLine("fleet1"), "Init supply line succeeds");
    assertTrue(sys.getDepotCount("fleet1") == 0, "No depots initially");
    assertTrue(sys.getTotalResupplies("fleet1") == 0, "No resupplies initially");
}

void testSupplyLineAddDepot() {
    std::cout << "\n=== SupplyLine: AddDepot ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    assertTrue(sys.addDepot("fleet1", "depot1", "system_alpha", 100.0f), "Add depot succeeds");
    assertTrue(sys.getDepotCount("fleet1") == 1, "1 depot");
    assertTrue(!sys.addDepot("fleet1", "depot1", "system_beta", 50.0f), "Duplicate depot rejected");
}

void testSupplyLineResupply() {
    std::cout << "\n=== SupplyLine: Resupply ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    assertTrue(sys.resupplyDepot("fleet1", "depot1", 50.0f, 60.0f), "Resupply succeeds");
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 50.0f), "Fuel is 50");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 60.0f), "Ammo is 60");
    assertTrue(sys.getTotalResupplies("fleet1") == 1, "1 resupply counted");
}

void testSupplyLineResupplyCap() {
    std::cout << "\n=== SupplyLine: ResupplyCap ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 80.0f, 80.0f);
    sys.resupplyDepot("fleet1", "depot1", 50.0f, 50.0f); // should cap at 100
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 100.0f), "Fuel capped at 100");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 100.0f), "Ammo capped at 100");
}

void testSupplyLineConsume() {
    std::cout << "\n=== SupplyLine: Consume ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 80.0f, 70.0f);
    assertTrue(sys.consumeSupplies("fleet1", "depot1", 30.0f, 20.0f), "Consume succeeds");
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 50.0f), "Fuel is 50 after consume");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 50.0f), "Ammo is 50 after consume");
}

void testSupplyLineConsumeFloor() {
    std::cout << "\n=== SupplyLine: ConsumeFloor ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 10.0f, 5.0f);
    sys.consumeSupplies("fleet1", "depot1", 50.0f, 50.0f); // consume more than available
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 0.0f), "Fuel floored at 0");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 0.0f), "Ammo floored at 0");
}

void testSupplyLineCritical() {
    std::cout << "\n=== SupplyLine: Critical ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 15.0f, 50.0f); // fuel < 20
    assertTrue(sys.isDepotCritical("fleet1", "depot1"), "Depot critical (low fuel)");
    sys.resupplyDepot("fleet1", "depot1", 30.0f, 0.0f); // fuel now 45
    assertTrue(!sys.isDepotCritical("fleet1", "depot1"), "Depot not critical (both above 20)");
}

void testSupplyLineUpdate() {
    std::cout << "\n=== SupplyLine: Update ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 50.0f, 50.0f);
    sys.update(10.0f); // consumption_rate=1.0, delta=10 → drain 10 each
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 40.0f), "Fuel after update is 40");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 40.0f), "Ammo after update is 40");
}

void testSupplyLineRemoveDepot() {
    std::cout << "\n=== SupplyLine: RemoveDepot ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.addDepot("fleet1", "depot2", "sys2", 100.0f);
    assertTrue(sys.removeDepot("fleet1", "depot1"), "Remove depot succeeds");
    assertTrue(sys.getDepotCount("fleet1") == 1, "1 depot remains");
    assertTrue(!sys.removeDepot("fleet1", "depot1"), "Remove nonexistent fails");
}

void testSupplyLineMissing() {
    std::cout << "\n=== SupplyLine: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    assertTrue(!sys.initializeSupplyLine("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.addDepot("nonexistent", "d1", "s1", 100), "Add depot fails on missing");
    assertTrue(!sys.removeDepot("nonexistent", "d1"), "Remove fails on missing");
    assertTrue(!sys.resupplyDepot("nonexistent", "d1", 10, 10), "Resupply fails on missing");
    assertTrue(!sys.consumeSupplies("nonexistent", "d1", 10, 10), "Consume fails on missing");
    assertTrue(sys.getDepotCount("nonexistent") == 0, "0 depots on missing");
    assertTrue(approxEqual(sys.getFuelLevel("nonexistent", "d1"), 0.0f), "0 fuel on missing");
    assertTrue(approxEqual(sys.getAmmoLevel("nonexistent", "d1"), 0.0f), "0 ammo on missing");
    assertTrue(sys.getTotalResupplies("nonexistent") == 0, "0 resupplies on missing");
    assertTrue(!sys.isDepotCritical("nonexistent", "d1"), "Not critical on missing");
}


void run_fleet_tests() {
    testFleetCreateAndDisband();
    testFleetAddRemoveMembers();
    testFleetFCLeavePromotes();
    testFleetDisbandOnEmpty();
    testFleetPromoteMember();
    testFleetSquadAndWingOrganization();
    testFleetBonuses();
    testFleetBroadcastTarget();
    testFleetWarp();
    testFleetDisbandPermission();
    testFleetMembershipComponent();
    testFleetMoraleRecordWin();
    testFleetMoraleRecordLoss();
    testFleetMoraleMultipleEvents();
    testFleetMoraleLossStreak();
    testFleetMoraleSavedByPlayer();
    testFleetMoraleMissionTogether();
    testFleetCargoAddContributor();
    testFleetCargoRemoveContributor();
    testFleetCargoMultipleShips();
    testFleetCargoUsedCapacity();
    testFleetCargoGetCapacity();
    testFleetFormationSetFormation();
    testFleetFormationLeaderAtOrigin();
    testFleetFormationArrowOffsets();
    testFleetFormationLineOffsets();
    testFleetFormationDiamondOffsets();
    testFleetFormationNoneType();
    testPlayerFleetCreate();
    testPlayerFleetAssignCaptains();
    testPlayerFleetMaxCap();
    testPlayerFleetNotPlayerFleet();
    testFleetProgressionEarlyStage();
    testFleetProgressionMidStage();
    testFleetProgressionEndStage();
    testFleetProgressionCanAddShip();
    testFleetProgressionRemoveShip();
    testFleetProgressionWingRoleUnlock();
    testFleetProgressionEndStageAllRoles();
    testFleetCargoShipLoss();
    testFleetCargoScaledCapacity();
    testFleetCargoScaledWithMoralePenalty();
    testFleetWarpFormationBeginEnd();
    testFleetWarpFormationShipClassSelection();
    testFleetWarpFormationBreathing();
    testFleetWarpFormationDistortion();
    testFleetWarpFormationLeaderAtOrigin();
    testFleetWarpFormationSlotOffsets();
    testFleetCivilizationThresholdNotMet();
    testFleetCivilizationThresholdMet();
    testFleetHistoryEmpty();
    testFleetHistoryRecordEvent();
    testFleetHistoryMaxEvents();
    testFleetHistoryByType();
    testFleetDoctrineCreate();
    testFleetDoctrineAddSlot();
    testFleetDoctrineRemoveSlot();
    testFleetDoctrineAssignShip();
    testFleetDoctrineUnassignShip();
    testFleetDoctrineReadiness();
    testFleetDoctrineLock();
    testFleetDoctrineShipCounts();
    testFleetDoctrineMissing();
    testFleetDoctrineComponentDefaults();
    testFleetSquadCreate();
    testFleetSquadDissolve();
    testFleetSquadAddMember();
    testFleetSquadMaxMembers();
    testFleetSquadRemoveMember();
    testFleetSquadRemoveLeader();
    testFleetSquadFormation();
    testFleetSquadRole();
    testFleetSquadEffectiveness();
    testFleetSquadMissing();
    testFleetMoraleResCreate();
    testFleetMoraleResCrisis();
    testFleetMoraleResResolve();
    testFleetMoraleResAdjust();
    testFleetMoraleResIdeology();
    testFleetMoraleResRecovery();
    testFleetMoraleResAutoFracture();
    testFleetMoraleResDeparture();
    testFleetMoraleResMultiple();
    testFleetMoraleResMissing();
    testSupplyLineCreate();
    testSupplyLineAddDepot();
    testSupplyLineResupply();
    testSupplyLineResupplyCap();
    testSupplyLineConsume();
    testSupplyLineConsumeFloor();
    testSupplyLineCritical();
    testSupplyLineUpdate();
    testSupplyLineRemoveDepot();
    testSupplyLineMissing();
}
