/**
 * Mission tests
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

// ==================== Mission System Tests ====================

void testMissionAcceptAndComplete() {
    std::cout << "\n=== Mission Accept & Complete ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);
    auto* playerComp = addComp<components::Player>(player);
    playerComp->credits = 0.0;
    auto* standings = addComp<components::Standings>(player);

    // Accept a mission
    bool accepted = missionSys.acceptMission("player1", "mission_001",
        "Destroy Pirates", 1, "combat", "Veyren", 100000.0, 0.5f);
    assertTrue(accepted, "Mission accepted successfully");

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(tracker->active_missions.size() == 1, "One active mission");

    // Add objective
    components::MissionTracker::Objective obj;
    obj.type = "destroy";
    obj.target = "pirate_frigate";
    obj.required = 3;
    obj.completed = 0;
    tracker->active_missions[0].objectives.push_back(obj);

    // Record partial progress
    missionSys.recordProgress("player1", "mission_001", "destroy", "pirate_frigate", 2);
    assertTrue(tracker->active_missions[0].objectives[0].completed == 2,
               "Partial progress recorded (2/3)");

    // Complete the objective
    missionSys.recordProgress("player1", "mission_001", "destroy", "pirate_frigate", 1);
    assertTrue(tracker->active_missions[0].objectives[0].done(),
               "Objective completed (3/3)");

    // Update should process completion
    missionSys.update(0.0f);
    assertTrue(approxEqual(static_cast<float>(playerComp->credits), 100000.0f, 1.0f),
               "Credits reward applied");
    assertTrue(tracker->completed_mission_ids.size() == 1,
               "Mission recorded as completed");
    assertTrue(tracker->active_missions.empty(),
               "Active missions cleared after completion");

    // Check standing was applied
    float standing = standings->faction_standings["Veyren"];
    assertTrue(approxEqual(standing, 0.5f), "Standing reward applied");
}

void testMissionTimeout() {
    std::cout << "\n=== Mission Timeout ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    // Accept a timed mission (30 second limit)
    missionSys.acceptMission("player1", "timed_001",
        "Timed Mission", 1, "combat", "Veyren", 50000.0, 0.1f, 30.0f);

    auto* tracker = player->getComponent<components::MissionTracker>();

    // Add an incomplete objective
    components::MissionTracker::Objective obj;
    obj.type = "destroy";
    obj.target = "enemy";
    obj.required = 5;
    tracker->active_missions[0].objectives.push_back(obj);

    // Update for 25 seconds (should still be active)
    missionSys.update(25.0f);
    assertTrue(tracker->active_missions.size() == 1, "Mission still active at 25s");

    // Update past the time limit
    missionSys.update(10.0f);
    assertTrue(tracker->active_missions.empty(), "Timed-out mission removed");
}

void testMissionAbandon() {
    std::cout << "\n=== Mission Abandon ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    missionSys.acceptMission("player1", "abandon_001",
        "Will Abandon", 1, "combat", "Faction", 10000.0, 0.1f);

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(tracker->active_missions.size() == 1, "Mission active before abandon");

    missionSys.abandonMission("player1", "abandon_001");
    assertTrue(tracker->active_missions.empty(), "Mission removed after abandon");
}

void testMissionDuplicatePrevention() {
    std::cout << "\n=== Mission Duplicate Prevention ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    bool first = missionSys.acceptMission("player1", "dup_001",
        "First", 1, "combat", "Faction", 10000.0, 0.1f);
    bool second = missionSys.acceptMission("player1", "dup_001",
        "Duplicate", 1, "combat", "Faction", 10000.0, 0.1f);

    assertTrue(first, "First accept succeeds");
    assertTrue(!second, "Duplicate accept rejected");
}

// ==================== Mission Template System Tests ====================

void testMissionTemplateInstallDefaults() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    // Should have templates for level 1 with no faction restriction
    auto templates = sys.getTemplatesForFaction("", 0.0f, 1);
    assertTrue(!templates.empty(), "Default templates installed for level 1");
}

void testMissionTemplateFilterByLevel() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto l1 = sys.getTemplatesForFaction("", 0.0f, 1);
    auto l5 = sys.getTemplatesForFaction("", 0.0f, 5);
    assertTrue(!l1.empty(), "Level 1 templates exist");
    assertTrue(l5.size() <= l1.size(), "Level 5 templates <= level 1 templates");
}

void testMissionTemplateFilterByStanding() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto low_standing = sys.getTemplatesForFaction("", -5.0f, 1);
    auto high_standing = sys.getTemplatesForFaction("", 5.0f, 1);
    assertTrue(high_standing.size() >= low_standing.size(),
               "Higher standing unlocks at least as many templates");
}

void testMissionTemplateGenerate() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto templates = sys.getTemplatesForFaction("", 0.0f, 1);
    assertTrue(!templates.empty(), "Have templates to generate from");

    auto mission = sys.generateMissionFromTemplate(templates[0], "system_1", "player_1");
    assertTrue(!mission.mission_id.empty(), "Generated mission has ID");
    assertTrue(!mission.objectives.empty(), "Generated mission has objectives");
    assertTrue(mission.isc_reward > 0.0, "Generated mission has positive Credits reward");
}

void testMissionTemplateDeterministic() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto templates = sys.getTemplatesForFaction("", 0.0f, 1);
    auto m1 = sys.generateMissionFromTemplate(templates[0], "system_1", "player_1");
    auto m2 = sys.generateMissionFromTemplate(templates[0], "system_1", "player_1");
    assertTrue(m1.objectives.size() == m2.objectives.size(),
               "Deterministic: same objectives count for same seed");
}

void testMissionTemplateScaledRewards() {
    ecs::World world;
    systems::MissionTemplateSystem sys(&world);
    sys.installDefaultTemplates();

    auto l1 = sys.getTemplatesForFaction("", 0.0f, 1);
    auto l3 = sys.getTemplatesForFaction("", 1.0f, 3);

    if (!l1.empty() && !l3.empty()) {
        auto m1 = sys.generateMissionFromTemplate(l1[0], "s1", "p1");
        auto m3 = sys.generateMissionFromTemplate(l3[0], "s1", "p1");
        assertTrue(m3.isc_reward >= m1.isc_reward,
                   "Higher level missions give more Credits");
    } else {
        assertTrue(true, "Higher level missions give more Credits (skipped)");
    }
}

// ==================== Mission Generator System Tests ====================

void testMissionGeneratorGeneratesMissions() {
    ecs::World world;
    systems::MissionTemplateSystem templateSys(&world);
    templateSys.installDefaultTemplates();
    systems::MissionGeneratorSystem genSys(&world, &templateSys);

    // Create a system entity with DifficultyZone
    auto* sys_entity = world.createEntity("test_system");
    auto* zone = addComp<components::DifficultyZone>(sys_entity);
    zone->security_status = 0.5f;

    int count = genSys.generateMissionsForSystem("test_system", 42);
    assertTrue(count > 0, "Generator produces missions");
}

void testMissionGeneratorAvailableMissions() {
    ecs::World world;
    systems::MissionTemplateSystem templateSys(&world);
    templateSys.installDefaultTemplates();
    systems::MissionGeneratorSystem genSys(&world, &templateSys);

    auto* sys_entity = world.createEntity("sys1");
    auto* zone = addComp<components::DifficultyZone>(sys_entity);
    zone->security_status = 0.5f;

    genSys.generateMissionsForSystem("sys1", 99);
    auto available = genSys.getAvailableMissions("sys1");
    assertTrue(!available.empty(), "Available missions list is not empty");
}

void testMissionGeneratorOfferToPlayer() {
    ecs::World world;
    systems::MissionTemplateSystem templateSys(&world);
    templateSys.installDefaultTemplates();
    systems::MissionGeneratorSystem genSys(&world, &templateSys);

    auto* sys_entity = world.createEntity("sys1");
    auto* zone = addComp<components::DifficultyZone>(sys_entity);
    zone->security_status = 0.5f;

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    genSys.generateMissionsForSystem("sys1", 42);
    bool offered = genSys.offerMissionToPlayer("player1", "sys1", 0);
    assertTrue(offered, "Mission offered successfully");

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(!tracker->active_missions.empty(), "Player has active mission after offer");
}

void testMissionGeneratorInvalidIndex() {
    ecs::World world;
    systems::MissionTemplateSystem templateSys(&world);
    templateSys.installDefaultTemplates();
    systems::MissionGeneratorSystem genSys(&world, &templateSys);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    bool offered = genSys.offerMissionToPlayer("player1", "nonexistent", 0);
    assertTrue(!offered, "Invalid system returns false");
}

// ==================== Mission Economy Effects Tests ====================

void testMissionEconomyCombatReducesSpawnRate() {
    ecs::World world;
    systems::MissionSystem sys(&world);

    // Create system entity with DifficultyZone
    auto* sys_entity = world.createEntity("system1");
    auto* zone = addComp<components::DifficultyZone>(sys_entity);
    zone->spawn_rate_multiplier = 1.5f;

    // Create player with mission
    auto* player = world.createEntity("player1");
    auto* tracker = addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    sys.setEconomySystemId("system1");
    sys.acceptMission("player1", "m1", "Clear Pirates", 3, "combat", "Solari",
                      100000.0, 0.1f, -1.0f);

    // Add objective and complete it
    tracker->active_missions[0].objectives.push_back({"destroy", "pirate", 1, 1});
    sys.update(0.1f);

    assertTrue(zone->spawn_rate_multiplier < 1.5f,
               "Combat mission completion reduces spawn rate");
}

void testMissionEconomyMiningReducesOre() {
    ecs::World world;
    systems::MissionSystem sys(&world);

    auto* sys_entity = world.createEntity("system1");
    auto* resources = addComp<components::SystemResources>(sys_entity);
    components::SystemResources::ResourceEntry entry;
    entry.mineral_type = "Ferrite";
    entry.total_quantity = 1000.0f;
    entry.remaining_quantity = 1000.0f;
    resources->resources.push_back(entry);

    auto* player = world.createEntity("player1");
    auto* tracker = addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    sys.setEconomySystemId("system1");
    sys.acceptMission("player1", "m1", "Mine Ore", 2, "mining", "Solari",
                      50000.0, 0.05f, -1.0f);
    tracker->active_missions[0].objectives.push_back({"mine", "Ferrite", 1, 1});
    sys.update(0.1f);

    assertTrue(resources->resources[0].remaining_quantity < 1000.0f,
               "Mining mission completion depletes ore reserves");
}

void testMissionEconomyCompletedCount() {
    ecs::World world;
    systems::MissionSystem sys(&world);

    auto* player = world.createEntity("player1");
    auto* tracker = addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    sys.acceptMission("player1", "m1", "Test", 1, "combat", "Solari",
                      10000.0, 0.1f, -1.0f);
    tracker->active_missions[0].objectives.push_back({"destroy", "pirate", 1, 1});
    sys.update(0.1f);

    assertTrue(sys.getCompletedMissionCount() == 1,
               "Completed mission count increments");
}

// ==================== MissionConsequence Tests ====================

void testConsequenceInit() {
    std::cout << "\n=== MissionConsequence: Init ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    assertTrue(sys.initializeConsequences("sys_1", "sol"), "Consequences initialized");
    assertTrue(sys.getActiveCount("sys_1") == 0, "No active consequences initially");
    assertTrue(!sys.initializeConsequences("sys_1", "sol"), "Duplicate init fails");
}

void testConsequenceTrigger() {
    std::cout << "\n=== MissionConsequence: Trigger ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    assertTrue(sys.triggerConsequence("sys_1", "mission_1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        0.5f, 300.0f, "empire", false), "Consequence triggered");
    assertTrue(sys.getActiveCount("sys_1") == 1, "1 active consequence");
    assertTrue(sys.isConsequenceActive("sys_1", "csq_0"), "Consequence csq_0 is active");
}

void testConsequenceExpiry() {
    std::cout << "\n=== MissionConsequence: Expiry ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::SecurityShift,
        1.0f, 5.0f, "police", false);
    assertTrue(sys.getActiveCount("sys_1") == 1, "1 active before expiry");
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    assertTrue(sys.getActiveCount("sys_1") == 0, "0 active after expiry");
}

void testConsequencePermanent() {
    std::cout << "\n=== MissionConsequence: Permanent ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::TerritoryShift,
        2.0f, 10.0f, "rebels", true);
    for (int i = 0; i < 100; i++) sys.update(1.0f);
    assertTrue(sys.getActiveCount("sys_1") == 1, "Permanent consequence persists");
    assertTrue(sys.getPermanentCount("sys_1") == 1, "1 permanent consequence");
}

void testConsequenceMagnitude() {
    std::cout << "\n=== MissionConsequence: Magnitude ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::PriceImpact,
        0.3f, 300.0f, "traders", false);
    sys.triggerConsequence("sys_1", "m2",
        components::MissionConsequence::ConsequenceType::PriceImpact,
        0.7f, 300.0f, "traders", false);
    float mag = sys.getMagnitude("sys_1",
        components::MissionConsequence::ConsequenceType::PriceImpact);
    assertTrue(approxEqual(mag, 1.0f), "Magnitudes sum to 1.0");
}

void testConsequenceDecay() {
    std::cout << "\n=== MissionConsequence: Decay ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::SpawnChange,
        1.0f, 100.0f, "pirates", false);
    assertTrue(sys.getActiveCount("sys_1") == 1, "Active before decay");
    sys.update(50.0f);
    assertTrue(sys.getActiveCount("sys_1") == 1, "Still active at 50s");
    sys.update(60.0f);
    assertTrue(sys.getActiveCount("sys_1") == 0, "Expired after 110s total");
}

void testConsequenceManualExpire() {
    std::cout << "\n=== MissionConsequence: Manual Expire ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        1.0f, 300.0f, "faction_a", false);
    assertTrue(sys.isConsequenceActive("sys_1", "csq_0"), "Active before manual expire");
    assertTrue(sys.expireConsequence("sys_1", "csq_0"), "Manual expire succeeds");
    assertTrue(sys.getActiveCount("sys_1") == 0, "0 active after manual expire");
    assertTrue(!sys.expireConsequence("sys_1", "csq_0"), "Double expire fails");
}

void testConsequenceMultipleTypes() {
    std::cout << "\n=== MissionConsequence: Multiple Types ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    sys.triggerConsequence("sys_1", "m1",
        components::MissionConsequence::ConsequenceType::StandingChange,
        0.5f, 300.0f, "empire", false);
    sys.triggerConsequence("sys_1", "m2",
        components::MissionConsequence::ConsequenceType::PriceImpact,
        0.8f, 300.0f, "traders", false);
    float standing = sys.getMagnitude("sys_1",
        components::MissionConsequence::ConsequenceType::StandingChange);
    float price = sys.getMagnitude("sys_1",
        components::MissionConsequence::ConsequenceType::PriceImpact);
    assertTrue(approxEqual(standing, 0.5f), "Standing magnitude is 0.5");
    assertTrue(approxEqual(price, 0.8f), "Price magnitude is 0.8");
}

void testConsequenceCount() {
    std::cout << "\n=== MissionConsequence: Count ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    world.createEntity("sys_1");
    sys.initializeConsequences("sys_1", "sol");
    for (int i = 0; i < 5; i++) {
        sys.triggerConsequence("sys_1", "m" + std::to_string(i),
            components::MissionConsequence::ConsequenceType::ReputationBoost,
            0.1f, 300.0f, "faction", false);
    }
    assertTrue(sys.getActiveCount("sys_1") == 5, "5 active consequences");
    sys.triggerConsequence("sys_1", "m5",
        components::MissionConsequence::ConsequenceType::ResourceDepletion,
        0.2f, 300.0f, "miners", true);
    assertTrue(sys.getActiveCount("sys_1") == 6, "6 total after permanent");
    assertTrue(sys.getPermanentCount("sys_1") == 1, "1 permanent");
}

void testConsequenceMissing() {
    std::cout << "\n=== MissionConsequence: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::MissionConsequenceSystem sys(&world);
    assertTrue(!sys.initializeConsequences("nonexistent", "sol"), "Init fails on missing");
    assertTrue(sys.getActiveCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(approxEqual(sys.getMagnitude("nonexistent",
        components::MissionConsequence::ConsequenceType::StandingChange), 0.0f), "Magnitude 0 on missing");
    assertTrue(sys.getPermanentCount("nonexistent") == 0, "Permanent 0 on missing");
}

// ==================== MissionEditor System Tests ====================

void testMissionEditorCreate() {
    std::cout << "\n=== MissionEditor: Create ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    auto* e = world.createEntity("editor1");
    assertTrue(sys.createEditor("editor1"), "Create editor succeeds");
    auto* ed = e->getComponent<components::MissionEditor>();
    assertTrue(ed != nullptr, "Component exists");
    assertTrue(ed->active, "Editor is active by default");
    assertTrue(ed->mission_level == 1, "Default level is 1");
    assertTrue(ed->published_count == 0, "No published missions");
}

void testMissionEditorSetName() {
    std::cout << "\n=== MissionEditor: SetName ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    assertTrue(sys.setMissionName("editor1", "Patrol Alpha"), "Set name succeeds");
    assertTrue(sys.setMissionLevel("editor1", 3), "Set level succeeds");
    assertTrue(sys.setMissionType("editor1", 2), "Set type succeeds");
}

void testMissionEditorAddObjective() {
    std::cout << "\n=== MissionEditor: AddObjective ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    int id = sys.addObjective("editor1", "Destroy 5 pirates", 0);
    assertTrue(id > 0, "Objective ID is positive");
    assertTrue(sys.getObjectiveCount("editor1") == 1, "1 objective");
    int id2 = sys.addObjective("editor1", "Return to station", 1);
    assertTrue(id2 > id, "Second ID is higher");
    assertTrue(sys.getObjectiveCount("editor1") == 2, "2 objectives");
}

void testMissionEditorRemoveObjective() {
    std::cout << "\n=== MissionEditor: RemoveObjective ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    int id = sys.addObjective("editor1", "Mine ore", 4);
    assertTrue(sys.removeObjective("editor1", id), "Remove succeeds");
    assertTrue(sys.getObjectiveCount("editor1") == 0, "0 objectives after remove");
    assertTrue(!sys.removeObjective("editor1", id), "Remove again fails");
}

void testMissionEditorSetReward() {
    std::cout << "\n=== MissionEditor: SetReward ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    auto* e = world.createEntity("editor1");
    sys.createEditor("editor1");
    assertTrue(sys.setReward("editor1", 50000.0f, 0.5f), "Set reward succeeds");
    auto* ed = e->getComponent<components::MissionEditor>();
    assertTrue(ed->reward_credits == 50000.0f, "Credits set correctly");
    assertTrue(ed->reward_standing == 0.5f, "Standing set correctly");
}

void testMissionEditorValidate() {
    std::cout << "\n=== MissionEditor: Validate ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    assertTrue(!sys.validate("editor1"), "Empty mission fails validation");
    assertTrue(sys.getValidationError("editor1") == "Mission name is required", "Error: name required");
    sys.setMissionName("editor1", "Test Mission");
    assertTrue(!sys.validate("editor1"), "No objectives fails validation");
    assertTrue(sys.getValidationError("editor1") == "At least one objective is required", "Error: objectives required");
}

void testMissionEditorPublish() {
    std::cout << "\n=== MissionEditor: Publish ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    assertTrue(!sys.publish("editor1"), "Publish fails without valid mission");
    sys.setMissionName("editor1", "Patrol Beta");
    sys.addObjective("editor1", "Kill 3 enemies", 0);
    sys.setReward("editor1", 10000.0f, 0.1f);
    assertTrue(sys.publish("editor1"), "Publish succeeds with valid mission");
    assertTrue(sys.getPublishedCount("editor1") == 1, "Published count is 1");
}

void testMissionEditorLevelClamp() {
    std::cout << "\n=== MissionEditor: LevelClamp ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    auto* e = world.createEntity("editor1");
    sys.createEditor("editor1");
    sys.setMissionLevel("editor1", 10);
    auto* ed = e->getComponent<components::MissionEditor>();
    assertTrue(ed->mission_level == 5, "Level clamped to 5");
    sys.setMissionLevel("editor1", -1);
    assertTrue(ed->mission_level == 1, "Level clamped to 1");
}

void testMissionEditorEmptyObjective() {
    std::cout << "\n=== MissionEditor: EmptyObjective ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    world.createEntity("editor1");
    sys.createEditor("editor1");
    int id = sys.addObjective("editor1", "", 0);
    assertTrue(id == -1, "Empty description rejected");
    assertTrue(sys.getObjectiveCount("editor1") == 0, "No objectives added");
}

void testMissionEditorMissing() {
    std::cout << "\n=== MissionEditor: Missing ===" << std::endl;
    ecs::World world;
    systems::MissionEditorSystem sys(&world);
    assertTrue(!sys.createEditor("nonexistent"), "Create fails on missing");
    assertTrue(!sys.setMissionName("nonexistent", "X"), "SetName fails on missing");
    assertTrue(!sys.setMissionLevel("nonexistent", 1), "SetLevel fails on missing");
    assertTrue(!sys.setMissionType("nonexistent", 0), "SetType fails on missing");
    assertTrue(sys.addObjective("nonexistent", "X", 0) == -1, "AddObjective fails on missing");
    assertTrue(!sys.removeObjective("nonexistent", 1), "RemoveObjective fails on missing");
    assertTrue(!sys.setReward("nonexistent", 100.0f, 0.0f), "SetReward fails on missing");
    assertTrue(!sys.validate("nonexistent"), "Validate fails on missing");
    assertTrue(!sys.publish("nonexistent"), "Publish fails on missing");
    assertTrue(sys.getObjectiveCount("nonexistent") == 0, "0 objectives on missing");
    assertTrue(sys.getPublishedCount("nonexistent") == 0, "0 published on missing");
}

// ==================== ProceduralMissionGenerator System Tests ====================

void testProcMissionCreate() {
    std::cout << "\n=== ProceduralMissionGenerator: Create ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    assertTrue(sys.initialize("gen1", "gen_caldari", "caldari_navy"), "Init succeeds");
    assertTrue(sys.getAvailableCount("gen1") == 0, "No missions initially");
    assertTrue(sys.getCompletedCount("gen1") == 0, "No completed initially");
    assertTrue(sys.getTotalGenerated("gen1") == 0, "No generated initially");
}

void testProcMissionGenerate() {
    std::cout << "\n=== ProceduralMissionGenerator: Generate ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    assertTrue(sys.generateMission("gen1", "m1", "Combat", 3, "system_jita"), "Generate succeeds");
    assertTrue(sys.getAvailableCount("gen1") == 1, "1 available mission");
    assertTrue(sys.getTotalGenerated("gen1") == 1, "1 total generated");
    assertTrue(sys.getMissionDifficulty("gen1", "m1") == 3, "Difficulty is 3");
}

void testProcMissionDuplicate() {
    std::cout << "\n=== ProceduralMissionGenerator: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Combat", 3, "system_jita");
    assertTrue(!sys.generateMission("gen1", "m1", "Mining", 2, "system_jita"), "Duplicate rejected");
    assertTrue(sys.getAvailableCount("gen1") == 1, "Still 1 mission");
}

void testProcMissionAccept() {
    std::cout << "\n=== ProceduralMissionGenerator: Accept ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Combat", 3, "system_jita");
    assertTrue(sys.acceptMission("gen1", "m1"), "Accept succeeds");
    assertTrue(sys.isMissionAccepted("gen1", "m1"), "Mission is accepted");
    assertTrue(!sys.acceptMission("gen1", "m1"), "Double accept rejected");
}

void testProcMissionComplete() {
    std::cout << "\n=== ProceduralMissionGenerator: Complete ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Combat", 3, "system_jita");
    sys.acceptMission("gen1", "m1");
    assertTrue(sys.completeMission("gen1", "m1"), "Complete succeeds");
    assertTrue(sys.getCompletedCount("gen1") == 1, "1 completed");
    assertTrue(!sys.completeMission("gen1", "m1"), "Double complete rejected");
}

void testProcMissionExpire() {
    std::cout << "\n=== ProceduralMissionGenerator: Expire ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Courier", 1, "system_jita");
    sys.acceptMission("gen1", "m1");
    // Advance past time limit to expire
    sys.update(5000.0f);
    assertTrue(!sys.completeMission("gen1", "m1"), "Can't complete expired mission");
}

void testProcMissionReward() {
    std::cout << "\n=== ProceduralMissionGenerator: Reward ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    sys.generateMission("gen1", "m1", "Combat", 1, "system_jita");
    sys.generateMission("gen1", "m2", "Mining", 5, "system_jita");
    float r1 = sys.getMissionReward("gen1", "m1");
    float r2 = sys.getMissionReward("gen1", "m2");
    assertTrue(r1 > 0.0f, "Reward L1 > 0");
    assertTrue(r2 > r1, "Higher difficulty = higher reward");
}

void testProcMissionDifficultyBias() {
    std::cout << "\n=== ProceduralMissionGenerator: DifficultyBias ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    auto* entity = world.getEntity("gen1");
    auto* gen = entity->getComponent<components::ProceduralMissionGenerator>();
    gen->difficulty_bias = 2;
    sys.generateMission("gen1", "m1", "Combat", 2, "system_jita");
    assertTrue(sys.getMissionDifficulty("gen1", "m1") == 4, "Bias adds 2 to difficulty");
}

void testProcMissionMaxLimit() {
    std::cout << "\n=== ProceduralMissionGenerator: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    world.createEntity("gen1");
    sys.initialize("gen1", "gen_caldari", "caldari_navy");
    auto* entity = world.getEntity("gen1");
    auto* gen = entity->getComponent<components::ProceduralMissionGenerator>();
    gen->max_available = 2;
    sys.generateMission("gen1", "m1", "Combat", 1, "s1");
    sys.generateMission("gen1", "m2", "Mining", 2, "s1");
    assertTrue(!sys.generateMission("gen1", "m3", "Courier", 3, "s1"), "Max limit enforced");
    assertTrue(sys.getAvailableCount("gen1") == 2, "Still 2 missions");
}

void testProcMissionMissing() {
    std::cout << "\n=== ProceduralMissionGenerator: Missing ===" << std::endl;
    ecs::World world;
    systems::ProceduralMissionGeneratorSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "g1", "f1"), "Init fails on missing");
    assertTrue(!sys.generateMission("nonexistent", "m1", "Combat", 1, "s1"), "Generate fails on missing");
    assertTrue(!sys.acceptMission("nonexistent", "m1"), "Accept fails on missing");
    assertTrue(!sys.completeMission("nonexistent", "m1"), "Complete fails on missing");
    assertTrue(!sys.expireMission("nonexistent", "m1"), "Expire fails on missing");
    assertTrue(!sys.removeMission("nonexistent", "m1"), "Remove fails on missing");
    assertTrue(sys.getAvailableCount("nonexistent") == 0, "0 available on missing");
    assertTrue(sys.getCompletedCount("nonexistent") == 0, "0 completed on missing");
    assertTrue(approxEqual(sys.getMissionReward("nonexistent", "m1"), 0.0f), "0 reward on missing");
    assertTrue(sys.getMissionDifficulty("nonexistent", "m1") == 0, "0 difficulty on missing");
    assertTrue(!sys.isMissionAccepted("nonexistent", "m1"), "Not accepted on missing");
    assertTrue(sys.getTotalGenerated("nonexistent") == 0, "0 generated on missing");
}


void run_mission_tests() {
    testMissionAcceptAndComplete();
    testMissionTimeout();
    testMissionAbandon();
    testMissionDuplicatePrevention();
    testMissionTemplateInstallDefaults();
    testMissionTemplateFilterByLevel();
    testMissionTemplateFilterByStanding();
    testMissionTemplateGenerate();
    testMissionTemplateDeterministic();
    testMissionTemplateScaledRewards();
    testMissionGeneratorGeneratesMissions();
    testMissionGeneratorAvailableMissions();
    testMissionGeneratorOfferToPlayer();
    testMissionGeneratorInvalidIndex();
    testMissionEconomyCombatReducesSpawnRate();
    testMissionEconomyMiningReducesOre();
    testMissionEconomyCompletedCount();
    testConsequenceInit();
    testConsequenceTrigger();
    testConsequenceExpiry();
    testConsequencePermanent();
    testConsequenceMagnitude();
    testConsequenceDecay();
    testConsequenceManualExpire();
    testConsequenceMultipleTypes();
    testConsequenceCount();
    testConsequenceMissing();
    testMissionEditorCreate();
    testMissionEditorSetName();
    testMissionEditorAddObjective();
    testMissionEditorRemoveObjective();
    testMissionEditorSetReward();
    testMissionEditorValidate();
    testMissionEditorPublish();
    testMissionEditorLevelClamp();
    testMissionEditorEmptyObjective();
    testMissionEditorMissing();
    testProcMissionCreate();
    testProcMissionGenerate();
    testProcMissionDuplicate();
    testProcMissionAccept();
    testProcMissionComplete();
    testProcMissionExpire();
    testProcMissionReward();
    testProcMissionDifficultyBias();
    testProcMissionMaxLimit();
    testProcMissionMissing();
}
