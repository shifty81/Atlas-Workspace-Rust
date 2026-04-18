/**
 * Character tests
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

// ==================== CharacterCreationSystem Tests ====================

void testCharacterCreate() {
    std::cout << "\n=== Character Create ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);

    bool result = charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");
    assertTrue(result, "createCharacter returns true for valid race");

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->character_name == "TestPilot", "Character name is set correctly");
    assertTrue(sheet->intelligence == 23 && sheet->memory == 21, "Caldari starting attributes are correct");
}

void testCharacterInvalidRace() {
    std::cout << "\n=== Character Invalid Race ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);

    bool result = charSys.createCharacter("pilot_1", "TestPilot", "Jove", "Unknown", "Unknown", "male");
    assertTrue(!result, "createCharacter returns false for invalid race Jove");
}

void testCharacterInstallImplant() {
    std::cout << "\n=== Character Install Implant ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool result = charSys.installImplant("pilot_1", "imp_1", "Neural Boost", 1, "intelligence", 3);
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->implants.size() == 1, "Implant added to implants vector");
    assertTrue(charSys.getEffectiveAttribute("pilot_1", "intelligence") == 23 + 3, "Effective attribute includes implant bonus");
}

void testCharacterImplantSlotOccupied() {
    std::cout << "\n=== Character Implant Slot Occupied ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool first_install_result = charSys.installImplant("pilot_1", "imp_1", "Neural Boost", 1, "intelligence", 3);
    assertTrue(first_install_result, "First implant in slot 1 succeeds");

    bool second_install_result = charSys.installImplant("pilot_1", "imp_2", "Another Boost", 1, "perception", 2);
    assertTrue(!second_install_result, "Second implant in same slot 1 fails");
}

void testCharacterRemoveImplant() {
    std::cout << "\n=== Character Remove Implant ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");
    charSys.installImplant("pilot_1", "imp_1", "Neural Boost", 1, "intelligence", 3);

    bool result = charSys.removeImplant("pilot_1", 1);
    assertTrue(result, "removeImplant returns true for occupied slot");

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->implants.empty(), "Implants vector is empty after removal");
}

void testCharacterCloneGrade() {
    std::cout << "\n=== Character Clone Grade ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool result = charSys.setCloneGrade("pilot_1", "apex");
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(result && sheet->clone_grade == "apex", "Clone grade set to omega");

    bool gamma_result = charSys.setCloneGrade("pilot_1", "gamma");
    assertTrue(!gamma_result, "Invalid clone grade gamma returns false");
}

void testCharacterRelayClone() {
    std::cout << "\n=== Character Relay Clone ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool result = charSys.jumpClone("pilot_1");
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(result && sheet->clone_jump_cooldown > 0, "Relay clone sets cooldown");

    bool second = charSys.jumpClone("pilot_1");
    assertTrue(!second, "Cannot relay clone while on cooldown");
}

void testCharacterCloneCooldownDecay() {
    std::cout << "\n=== Character Clone Cooldown Decay ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    charSys.jumpClone("pilot_1");
    charSys.update(86400.0f);

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->clone_jump_cooldown == 0, "Cooldown decays to 0 after 86400 seconds");

    bool result = charSys.jumpClone("pilot_1");
    assertTrue(result, "Can relay clone again after cooldown expires");
}

void testCharacterSecurityStatus() {
    std::cout << "\n=== Character Security Status ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    charSys.modifySecurityStatus("pilot_1", 5.0f);
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(approxEqual(sheet->security_status, 5.0f), "Security status increased to 5.0");

    charSys.modifySecurityStatus("pilot_1", 8.0f);
    assertTrue(approxEqual(sheet->security_status, 10.0f), "Security status clamped to 10.0");
}

void testCharacterEmploymentHistory() {
    std::cout << "\n=== Character Employment History ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    charSys.addEmploymentRecord("pilot_1", "corp_1", "Test Corp", 1000.0f);
    charSys.addEmploymentRecord("pilot_1", "corp_2", "Another Corp", 2000.0f);

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->employment_history.size() == 2, "Two employment records added");
}

void testCharacterRaceAttributes() {
    std::cout << "\n=== Character Race Attributes ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* e1 = world.createEntity("amarr_pilot");
    addComp<components::CharacterSheet>(e1);
    charSys.createCharacter("amarr_pilot", "AmarrPilot", "Amarr", "Khanid", "Cyber Knight", "male");

    auto* e2 = world.createEntity("gallente_pilot");
    addComp<components::CharacterSheet>(e2);
    charSys.createCharacter("gallente_pilot", "GallentePilot", "Gallente", "Intaki", "Diplomat", "female");

    auto* e3 = world.createEntity("minmatar_pilot");
    addComp<components::CharacterSheet>(e3);
    charSys.createCharacter("minmatar_pilot", "MinmatarPilot", "Minmatar", "Brutor", "Warrior", "male");

    auto* e4 = world.createEntity("caldari_pilot");
    addComp<components::CharacterSheet>(e4);
    charSys.createCharacter("caldari_pilot", "CaldariPilot", "Caldari", "Deteis", "Scientist", "male");

    auto* s1 = e1->getComponent<components::CharacterSheet>();
    assertTrue(s1->willpower == 22, "Amarr willpower is 22");

    auto* s2 = e2->getComponent<components::CharacterSheet>();
    assertTrue(s2->charisma == 22, "Gallente charisma is 22");

    auto* s3 = e3->getComponent<components::CharacterSheet>();
    assertTrue(s3->perception == 22, "Minmatar perception is 22");

    auto* s4 = e4->getComponent<components::CharacterSheet>();
    assertTrue(s4->intelligence == 23, "Caldari intelligence is 23");
}

// ==================== CaptainPersonalitySystem Tests ====================

void testCaptainPersonalityAssign() {
    std::cout << "\n=== Captain Personality Assign ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Solari");
    float agg = sys.getPersonalityTrait("cap1", "aggression");
    float soc = sys.getPersonalityTrait("cap1", "sociability");
    float opt = sys.getPersonalityTrait("cap1", "optimism");
    float pro = sys.getPersonalityTrait("cap1", "professionalism");
    assertTrue(agg >= 0.0f && agg <= 1.0f, "Aggression in valid range");
    assertTrue(soc >= 0.0f && soc <= 1.0f, "Sociability in valid range");
    assertTrue(opt >= 0.0f && opt <= 1.0f, "Optimism in valid range");
    assertTrue(pro >= 0.0f && pro <= 1.0f, "Professionalism in valid range");
}

void testCaptainPersonalityFactionTraits() {
    std::cout << "\n=== Captain Personality Faction Traits ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "Keldari_Captain", "Keldari");
    float agg = sys.getPersonalityTrait("cap1", "aggression");
    assertTrue(agg > 0.5f, "Keldari captain has high aggression");
}

void testCaptainPersonalitySetTrait() {
    std::cout << "\n=== Captain Personality Set Trait ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Solari");
    sys.setPersonalityTrait("cap1", "aggression", 0.9f);
    assertTrue(approxEqual(sys.getPersonalityTrait("cap1", "aggression"), 0.9f),
               "Set trait reads back correctly");
}

void testCaptainPersonalityGetFaction() {
    std::cout << "\n=== Captain Personality Get Faction ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Veyren");
    assertTrue(sys.getCaptainFaction("cap1") == "Veyren", "Faction returned correctly");
}

void testCaptainPersonalityDeterministic() {
    std::cout << "\n=== Captain Personality Deterministic ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Aurelian");
    float agg1 = sys.getPersonalityTrait("cap1", "aggression");
    float soc1 = sys.getPersonalityTrait("cap1", "sociability");
    // Assign again - should get same result (deterministic)
    sys.assignPersonality("cap1", "TestCaptain", "Aurelian");
    float agg2 = sys.getPersonalityTrait("cap1", "aggression");
    float soc2 = sys.getPersonalityTrait("cap1", "sociability");
    assertTrue(approxEqual(agg1, agg2), "Aggression is deterministic");
    assertTrue(approxEqual(soc1, soc2), "Sociability is deterministic");
}

// ==================== FleetChatterSystem Tests ====================

void testFleetChatterSetActivity() {
    std::cout << "\n=== Fleet Chatter Set Activity ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    sys.setActivity("cap1", "Mining");
    auto* chatter = entity->getComponent<components::FleetChatterState>();
    assertTrue(chatter != nullptr, "FleetChatterState component created");
    assertTrue(chatter->current_activity == "Mining", "Activity set to Mining");
}

void testFleetChatterGetLine() {
    std::cout << "\n=== Fleet Chatter Get Line ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    addComp<components::FleetMorale>(entity);
    sys.setActivity("cap1", "Mining");
    std::string line = sys.getNextChatterLine("cap1");
    assertTrue(!line.empty(), "Chatter line is non-empty");
}

void testFleetChatterCooldown() {
    std::cout << "\n=== Fleet Chatter Cooldown ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Idle");
    sys.getNextChatterLine("cap1");
    std::string line2 = sys.getNextChatterLine("cap1");
    assertTrue(line2.empty(), "Second line empty due to cooldown");
}

void testFleetChatterLinesSpoken() {
    std::cout << "\n=== Fleet Chatter Lines Spoken ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Combat");
    sys.getNextChatterLine("cap1");
    assertTrue(sys.getTotalLinesSpoken("cap1") == 1, "Total lines spoken is 1");
}

void testFleetChatterCooldownExpires() {
    std::cout << "\n=== Fleet Chatter Cooldown Expires ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Warp");
    sys.getNextChatterLine("cap1");
    assertTrue(sys.isOnCooldown("cap1"), "On cooldown after speaking");
    sys.update(60.0f);
    assertTrue(!sys.isOnCooldown("cap1"), "Cooldown expired after 60s");
    std::string line = sys.getNextChatterLine("cap1");
    assertTrue(!line.empty(), "Can speak again after cooldown expires");
}

// ==================== CaptainRelationshipSystem Tests ====================

void testCaptainRelationshipRecordEvent() {
    std::cout << "\n=== Captain Relationship Record Event ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    sys.recordEvent("cap1", "cap2", "saved_in_combat");
    assertTrue(sys.getAffinity("cap1", "cap2") > 0.0f,
               "Affinity positive after saved_in_combat");
}

void testCaptainRelationshipAbandoned() {
    std::cout << "\n=== Captain Relationship Abandoned ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    sys.recordEvent("cap1", "cap2", "abandoned");
    assertTrue(sys.getAffinity("cap1", "cap2") < 0.0f,
               "Affinity negative after abandoned");
}

void testCaptainRelationshipStatus() {
    std::cout << "\n=== Captain Relationship Status Friend ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    // saved_in_combat gives +10 each, need >50
    for (int i = 0; i < 6; i++) {
        sys.recordEvent("cap1", "cap2", "saved_in_combat");
    }
    assertTrue(sys.getRelationshipStatus("cap1", "cap2") == "Friend",
               "Status is Friend with high affinity");
}

void testCaptainRelationshipGrudge() {
    std::cout << "\n=== Captain Relationship Grudge ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    // abandoned gives -20 each, need < -50
    for (int i = 0; i < 3; i++) {
        sys.recordEvent("cap1", "cap2", "abandoned");
    }
    assertTrue(sys.getRelationshipStatus("cap1", "cap2") == "Grudge",
               "Status is Grudge with very negative affinity");
}

void testCaptainRelationshipMultipleEvents() {
    std::cout << "\n=== Captain Relationship Multiple Events ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    sys.recordEvent("cap1", "cap2", "saved_in_combat");  // +10
    sys.recordEvent("cap1", "cap2", "abandoned");         // -20
    sys.recordEvent("cap1", "cap2", "shared_victory");    // +5
    // Net: -5
    float affinity = sys.getAffinity("cap1", "cap2");
    assertTrue(approxEqual(affinity, -5.0f), "Net affinity reflects mixed events");
}

// ==================== EmotionalArcSystem Tests ====================

void testEmotionalArcVictory() {
    std::cout << "\n=== Emotional Arc Victory ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("cap1");
    float baseline = sys.getConfidence("cap1");
    sys.onCombatVictory("cap1");
    assertTrue(sys.getConfidence("cap1") > baseline, "Confidence increased after victory");
}

void testEmotionalArcDefeat() {
    std::cout << "\n=== Emotional Arc Defeat ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::EmotionalState>(entity);
    float baseline_conf = sys.getConfidence("cap1");
    float baseline_fat = sys.getFatigue("cap1");
    sys.onCombatDefeat("cap1");
    assertTrue(sys.getConfidence("cap1") < baseline_conf, "Confidence decreased after defeat");
    assertTrue(sys.getFatigue("cap1") > baseline_fat, "Fatigue increased after defeat");
}

void testEmotionalArcRest() {
    std::cout << "\n=== Emotional Arc Rest ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* state = addComp<components::EmotionalState>(entity);
    state->fatigue = 50.0f;
    sys.onRest("cap1");
    assertTrue(state->fatigue < 50.0f, "Fatigue decreased after rest");
}

void testEmotionalArcTrust() {
    std::cout << "\n=== Emotional Arc Trust ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::EmotionalState>(entity);
    float baseline = sys.getTrust("cap1");
    sys.onPlayerTrust("cap1");
    assertTrue(sys.getTrust("cap1") > baseline, "Trust increased after player trust");
}

void testEmotionalArcBetray() {
    std::cout << "\n=== Emotional Arc Betray ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::EmotionalState>(entity);
    float baseline = sys.getTrust("cap1");
    sys.onPlayerBetray("cap1");
    assertTrue(sys.getTrust("cap1") < baseline, "Trust decreased after betrayal");
}

// ==================== Extended Captain Personality Tests ====================

void testCaptainPersonalityNewTraitsAssigned() {
    std::cout << "\n=== Captain Personality: New Traits Assigned ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Keldari");

    float loyalty = sys.getPersonalityTrait("cap1", "loyalty");
    float paranoia = sys.getPersonalityTrait("cap1", "paranoia");
    float ambition = sys.getPersonalityTrait("cap1", "ambition");
    float adaptability = sys.getPersonalityTrait("cap1", "adaptability");

    assertTrue(loyalty >= 0.0f && loyalty <= 1.0f, "Loyalty in valid range");
    assertTrue(paranoia >= 0.0f && paranoia <= 1.0f, "Paranoia in valid range");
    assertTrue(ambition >= 0.0f && ambition <= 1.0f, "Ambition in valid range");
    assertTrue(adaptability >= 0.0f && adaptability <= 1.0f, "Adaptability in valid range");
}

void testCaptainPersonalityKeldariHighParanoia() {
    std::cout << "\n=== Captain Personality: Keldari High Paranoia ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "KeldariCaptain", "Keldari");
    float paranoia = sys.getPersonalityTrait("cap1", "paranoia");
    assertTrue(paranoia > 0.5f, "Keldari captain has high paranoia");
}

void testCaptainPersonalitySolariHighLoyalty() {
    std::cout << "\n=== Captain Personality: Solari High Loyalty ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "SolariCaptain", "Solari");
    float loyalty = sys.getPersonalityTrait("cap1", "loyalty");
    assertTrue(loyalty > 0.5f, "Solari captain has high loyalty");
}

void testCaptainPersonalitySetNewTrait() {
    std::cout << "\n=== Captain Personality: Set New Trait ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "Test", "Veyren");
    sys.setPersonalityTrait("cap1", "loyalty", 0.95f);
    assertTrue(approxEqual(sys.getPersonalityTrait("cap1", "loyalty"), 0.95f),
               "Loyalty set correctly");
    sys.setPersonalityTrait("cap1", "paranoia", 0.1f);
    assertTrue(approxEqual(sys.getPersonalityTrait("cap1", "paranoia"), 0.1f),
               "Paranoia set correctly");
}

void testCaptainPersonalityNewTraitsDeterministic() {
    std::cout << "\n=== Captain Personality: New Traits Deterministic ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "Test", "Aurelian");
    float loy1 = sys.getPersonalityTrait("cap1", "loyalty");
    float par1 = sys.getPersonalityTrait("cap1", "paranoia");
    sys.assignPersonality("cap1", "Test", "Aurelian");
    float loy2 = sys.getPersonalityTrait("cap1", "loyalty");
    float par2 = sys.getPersonalityTrait("cap1", "paranoia");
    assertTrue(approxEqual(loy1, loy2), "Loyalty is deterministic");
    assertTrue(approxEqual(par1, par2), "Paranoia is deterministic");
}

// ==================== Captain Memory System Tests ====================

void testCaptainMemoryRecordEvent() {
    std::cout << "\n=== Captain Memory: Record Event ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "vs Pirates", 100.0f, 0.5f);
    assertTrue(sys.totalMemories("cap1") == 1, "One memory recorded");
    assertTrue(sys.countMemories("cap1", "combat_win") == 1, "One combat_win memory");
}

void testCaptainMemoryMultipleEvents() {
    std::cout << "\n=== Captain Memory: Multiple Events ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "vs Pirates", 100.0f, 0.5f);
    sys.recordMemory("cap1", "combat_loss", "vs Boss", 200.0f, -0.8f);
    sys.recordMemory("cap1", "saved_by_player", "close call", 300.0f, 0.9f);
    assertTrue(sys.totalMemories("cap1") == 3, "Three memories recorded");
    assertTrue(sys.countMemories("cap1", "combat_win") == 1, "One combat_win");
    assertTrue(sys.countMemories("cap1", "combat_loss") == 1, "One combat_loss");
}

void testCaptainMemoryAverageWeight() {
    std::cout << "\n=== Captain Memory: Average Weight ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "", 100.0f, 1.0f);
    sys.recordMemory("cap1", "combat_loss", "", 200.0f, -1.0f);
    float avg = sys.averageEmotionalWeight("cap1");
    assertTrue(approxEqual(avg, 0.0f), "Average weight is zero for balanced events");
}

void testCaptainMemoryMostRecent() {
    std::cout << "\n=== Captain Memory: Most Recent ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "", 100.0f, 0.5f);
    sys.recordMemory("cap1", "warp_anomaly", "strange lights", 200.0f, 0.2f);
    assertTrue(sys.mostRecentEvent("cap1") == "warp_anomaly", "Most recent is warp_anomaly");
}

void testCaptainMemoryCapacity() {
    std::cout << "\n=== Captain Memory: Capacity Cap ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    // Fill 55 memories — should cap at 50
    for (int i = 0; i < 55; ++i) {
        sys.recordMemory("cap1", "event" + std::to_string(i), "", static_cast<float>(i), 0.1f);
    }
    assertTrue(sys.totalMemories("cap1") == 50, "Memory capped at 50");
}

void testCaptainMemoryNoEntity() {
    std::cout << "\n=== Captain Memory: No Entity ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);

    assertTrue(sys.totalMemories("nonexistent") == 0, "No memories for nonexistent entity");
    assertTrue(sys.mostRecentEvent("nonexistent").empty(), "No recent event for nonexistent entity");
}

// ==================== Contextual Chatter Tests ====================

void testContextualChatterReturnsLine() {
    std::cout << "\n=== Contextual Chatter: Returns Line ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem chatterSys(&world);
    systems::CaptainPersonalitySystem personalitySys(&world);

    auto* entity = world.createEntity("cap1");
    personalitySys.assignPersonality("cap1", "TestCaptain", "Keldari");
    addComp<components::FleetChatterState>(entity);
    chatterSys.setActivity("cap1", "Combat");

    std::string line = chatterSys.getContextualLine("cap1");
    assertTrue(!line.empty(), "Contextual chatter returns a line");
}

void testContextualChatterRespectsCooldown() {
    std::cout << "\n=== Contextual Chatter: Respects Cooldown ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem chatterSys(&world);
    systems::CaptainPersonalitySystem personalitySys(&world);

    auto* entity = world.createEntity("cap1");
    personalitySys.assignPersonality("cap1", "TestCaptain", "Solari");
    addComp<components::FleetChatterState>(entity);
    chatterSys.setActivity("cap1", "Mining");

    chatterSys.getContextualLine("cap1");
    std::string second = chatterSys.getContextualLine("cap1");
    assertTrue(second.empty(), "Contextual chatter on cooldown returns empty");
}

void testContextualChatterFallbackWithoutPersonality() {
    std::cout << "\n=== Contextual Chatter: Fallback Without Personality ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem chatterSys(&world);

    auto* entity = world.createEntity("cap1");
    addComp<components::FleetChatterState>(entity);
    chatterSys.setActivity("cap1", "Idle");

    std::string line = chatterSys.getContextualLine("cap1");
    assertTrue(!line.empty(), "Falls back to generic pool without personality");
}

// ==================== Phase 9: Interruptible Chatter Tests ====================

void testChatterInterruptHighPriority() {
    std::cout << "\n=== Chatter Interrupt High Priority ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Idle");
    std::string line = sys.getNextChatterLine("cap1");
    assertTrue(!line.empty(), "Captain speaks initially");
    auto* chatter = entity->getComponent<components::FleetChatterState>();
    assertTrue(chatter->is_speaking, "Captain is_speaking after getNextChatterLine");
    assertTrue(chatter->speaking_priority == 1.0f, "Speaking priority is 1.0 (normal)");
    bool interrupted = sys.interruptChatter("cap1", 5.0f);
    assertTrue(interrupted, "Interrupt succeeds with higher priority");
    assertTrue(!chatter->is_speaking, "Captain no longer speaking after interrupt");
    assertTrue(chatter->was_interrupted, "was_interrupted flag set");
}

void testChatterInterruptLowPriorityFails() {
    std::cout << "\n=== Chatter Interrupt Low Priority Fails ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Combat");
    sys.getNextChatterLine("cap1");
    auto* chatter = entity->getComponent<components::FleetChatterState>();
    assertTrue(chatter->is_speaking, "Captain is speaking");
    bool interrupted = sys.interruptChatter("cap1", 0.5f);
    assertTrue(!interrupted, "Interrupt fails with lower priority");
    assertTrue(chatter->is_speaking, "Captain still speaking");
}

void testChatterInterruptNotSpeaking() {
    std::cout << "\n=== Chatter Interrupt Not Speaking ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::FleetChatterState>(entity);
    bool interrupted = sys.interruptChatter("cap1", 10.0f);
    assertTrue(!interrupted, "Cannot interrupt non-speaking captain");
}

// ==================== Phase 9: Friendship/Grudge Formation Effects Tests ====================

void testFormationFriendCloser() {
    std::cout << "\n=== Formation Friend Closer ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Arrow, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Arrow, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", 60.0f);  // Friend (>50)

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Friend spacing: 0.7 × 500 = 350m
    assertTrue(approxEqual(std::fabs(ox), 350.0f, 1.0f), "Friend flies closer (x offset ~350)");
    assertTrue(approxEqual(std::fabs(oz), 350.0f, 1.0f), "Friend flies closer (z offset ~350)");
}

void testFormationGrudgeWider() {
    std::cout << "\n=== Formation Grudge Wider ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Arrow, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Arrow, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", -60.0f);  // Grudge (<-50)

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Grudge spacing: 1.5 × 500 = 750m
    assertTrue(approxEqual(std::fabs(ox), 750.0f, 1.0f), "Grudge flies wider (x offset ~750)");
    assertTrue(approxEqual(std::fabs(oz), 750.0f, 1.0f), "Grudge flies wider (z offset ~750)");
}

void testFormationNeutralDefault() {
    std::cout << "\n=== Formation Neutral Default ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Arrow, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Arrow, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", 5.0f);  // Neutral

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Neutral spacing: 1.0 × 500 = 500m
    assertTrue(approxEqual(std::fabs(ox), 500.0f, 1.0f), "Neutral uses default spacing (x)");
    assertTrue(approxEqual(std::fabs(oz), 500.0f, 1.0f), "Neutral uses default spacing (z)");
}

void testFormationAllySpacing() {
    std::cout << "\n=== Formation Ally Spacing ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Line, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Line, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", 30.0f);  // Ally (>20)

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Ally spacing: 0.85 × 500 = 425m
    assertTrue(approxEqual(std::fabs(oz), 425.0f, 1.0f), "Ally spacing ~425m in line formation");
}

void testFormationRivalSpacing() {
    std::cout << "\n=== Formation Rival Spacing ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem formSys(&world);
    auto* leader = world.createEntity("leader");
    addComp<components::FleetFormation>(leader);
    formSys.setFormation("leader", components::FleetFormation::FormationType::Line, 0);

    auto* wingman = world.createEntity("wingman");
    addComp<components::FleetFormation>(wingman);
    formSys.setFormation("wingman", components::FleetFormation::FormationType::Line, 1);

    auto* rel = addComp<components::CaptainRelationship>(wingman);
    rel->modifyAffinity("leader", -30.0f);  // Rival (<-20)

    formSys.applyRelationshipSpacing("wingman", "leader");
    formSys.computeOffsets();

    float ox, oy, oz;
    formSys.getOffset("wingman", ox, oy, oz);
    // Rival spacing: 1.25 × 500 = 625m
    assertTrue(approxEqual(std::fabs(oz), 625.0f, 1.0f), "Rival spacing ~625m in line formation");
}

// ==================== Phase 9: Disagreement Model Tests ====================

void testDisagreementBasicScore() {
    std::cout << "\n=== Disagreement Basic Score ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* pers = addComp<components::CaptainPersonality>(entity);
    pers->aggression = 0.2f;   // cautious
    pers->optimism = 0.3f;     // somewhat grim
    auto* morale = addComp<components::FleetMorale>(entity);
    morale->losses = 5;

    // risk=0.8, no task mismatch
    float score = sys.computeDisagreement("cap1", 0.8f, false);
    // expected: 0.8*(1-0.2) + 5*(1-0.3) = 0.64 + 3.5 = 4.14
    assertTrue(approxEqual(score, 4.14f, 0.1f), "Disagreement score ~4.14");
}

void testDisagreementTaskMismatch() {
    std::cout << "\n=== Disagreement Task Mismatch ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* pers = addComp<components::CaptainPersonality>(entity);
    pers->aggression = 0.5f;
    pers->optimism = 0.5f;

    float noMismatch = sys.computeDisagreement("cap1", 0.5f, false);
    float withMismatch = sys.computeDisagreement("cap1", 0.5f, true);
    assertTrue(withMismatch - noMismatch >= 9.9f, "Task mismatch adds +10 to score");
}

void testDisagreementAggressiveLow() {
    std::cout << "\n=== Disagreement Aggressive Captain Low Score ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* pers = addComp<components::CaptainPersonality>(entity);
    pers->aggression = 0.9f;  // very aggressive → tolerates risk
    pers->optimism = 0.9f;    // very optimistic → shrugs off losses

    float score = sys.computeDisagreement("cap1", 1.0f, false);
    // expected: 1.0*(1-0.9) + 0*(1-0.9) = 0.1
    assertTrue(score < 1.0f, "Aggressive+optimistic captain has low disagreement");
}

// ==================== Phase 9: Silence Interpretation Tests ====================

void testSilenceInterpretationTriggered() {
    std::cout << "\n=== Silence Interpretation Triggered ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* player = world.createEntity("player");
    auto* presence = addComp<components::PlayerPresence>(player);
    presence->time_since_last_command = 150.0f;  // >120s threshold

    auto* cap = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(cap);
    addComp<components::FleetChatterState>(cap);

    std::string line = sys.getSilenceAwareLine("cap1", "player");
    assertTrue(!line.empty(), "Captain speaks about silence");

    // Should be one of the silence lines
    bool isSilenceLine = (line.find("Quiet") != std::string::npos ||
                          line.find("alright") != std::string::npos ||
                          line.find("heard from you") != std::string::npos ||
                          line.find("okay") != std::string::npos ||
                          line.find("checking in") != std::string::npos);
    assertTrue(isSilenceLine, "Line is a silence interpretation");
}

void testSilenceInterpretationNotTriggered() {
    std::cout << "\n=== Silence Interpretation Not Triggered ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* player = world.createEntity("player");
    auto* presence = addComp<components::PlayerPresence>(player);
    presence->time_since_last_command = 30.0f;  // <120s, no silence

    auto* cap = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(cap);
    addComp<components::FleetChatterState>(cap);
    sys.setActivity("cap1", "Idle");

    std::string line = sys.getSilenceAwareLine("cap1", "player");
    // Should fall back to contextual line (not silence-specific)
    assertTrue(!line.empty(), "Captain speaks (contextual fallback)");
}

// ==================== Character Mesh Cybernetics Tests ====================

void testCharacterBodyTypeOrganic() {
    std::cout << "\n=== PCG: Character organic body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    auto ch = cms.generate(42, Race::TerranDescendant, BodyType::Organic, sl);
    assertTrue(ch.cyber_percentage < 0.01f, "Organic has 0% cyber");
    assertTrue(ch.cyberLimbs.empty(), "Organic has no cyber limbs");
    assertTrue(ch.integrated_weapon_count == 0, "Organic has no weapon limbs");
}

void testCharacterBodyTypeAugmented() {
    std::cout << "\n=== PCG: Character augmented body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    auto ch = cms.generate(100, Race::SynthBorn, BodyType::Augmented, sl);
    assertTrue(ch.cyber_percentage > 0.0f, "Augmented has some cyber");
    assertTrue(ch.cyber_percentage < 0.5f, "Augmented is less than 50% cyber");
    assertTrue(!ch.cyberLimbs.empty(), "Augmented has cyber limbs");
    assertTrue(static_cast<int>(ch.cyberLimbs.size()) <= 2, "Augmented has 1-2 limbs");
}

void testCharacterBodyTypeCybernetic() {
    std::cout << "\n=== PCG: Character cybernetic body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    auto ch = cms.generate(200, Race::PureAlien, BodyType::Cybernetic, sl);
    assertTrue(ch.cyber_percentage >= 0.5f, "Cybernetic is 50%+ cyber");
    assertTrue(static_cast<int>(ch.cyberLimbs.size()) >= 3, "Cybernetic has 3+ limbs");
}

void testCharacterBodyTypeFullSynth() {
    std::cout << "\n=== PCG: Character full synth body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    auto ch = cms.generate(300, Race::HybridEvolutionary, BodyType::FullSynth, sl);
    assertTrue(ch.cyber_percentage >= 0.99f, "FullSynth is 100% cyber");
    assertTrue(static_cast<int>(ch.cyberLimbs.size()) == 6, "FullSynth has all 6 limbs replaced");
}

void testCharacterBodyTypeMechFrame() {
    std::cout << "\n=== PCG: Character mech frame body type ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    sl.height = 0.5f;
    auto organic = cms.generate(400, Race::TerranDescendant, BodyType::Organic, sl);
    auto mech    = cms.generate(400, Race::TerranDescendant, BodyType::MechFrame, sl);
    assertTrue(mech.total_height > organic.total_height, "MechFrame is taller than organic");
    assertTrue(mech.strength_multiplier > 1.5f, "MechFrame has high strength");
    assertTrue(mech.speed_multiplier < 1.0f, "MechFrame is slower");
}

void testCharacterBodyTypeNames() {
    std::cout << "\n=== PCG: Character body type names ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::Organic) == "Organic", "Organic name");
    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::Augmented) == "Augmented", "Augmented name");
    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::Cybernetic) == "Cybernetic", "Cybernetic name");
    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::FullSynth) == "FullSynth", "FullSynth name");
    assertTrue(CharacterMeshSystem::bodyTypeName(BodyType::MechFrame) == "MechFrame", "MechFrame name");
}

void testCharacterBackwardCompatibility() {
    std::cout << "\n=== PCG: Character backward compat ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    // Old API (no body type) should still work and produce Organic.
    auto ch = cms.generate(42, Race::TerranDescendant, sl);
    assertTrue(ch.bodyType == BodyType::Organic, "Old API defaults to Organic");
    assertTrue(ch.cyberLimbs.empty(), "Old API has no cyber limbs");
}

void testCharacterReferenceMeshArchive() {
    std::cout << "\n=== PCG: Character reference mesh archive ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    assertTrue(cms.referenceMeshArchive().empty(), "No archive by default");

    cms.setReferenceMeshArchive("human.zip");
    assertTrue(cms.referenceMeshArchive() == "human.zip", "Archive path stored");

    CharacterSliders sl;
    auto ch = cms.generate(42, Race::TerranDescendant, BodyType::Organic, sl);
    assertTrue(ch.referenceMeshArchive == "human.zip", "Generated character carries archive path");
}

void testCharacterUniformScale() {
    std::cout << "\n=== PCG: Character uniform scale ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;

    // Short character
    CharacterSliders shortSl;
    shortSl.height = 0.0f; // minimum
    auto shortCh = cms.generate(10, Race::TerranDescendant, BodyType::Organic, shortSl);

    // Tall character
    CharacterSliders tallSl;
    tallSl.height = 1.0f; // maximum
    auto tallCh = cms.generate(10, Race::TerranDescendant, BodyType::Organic, tallSl);

    assertTrue(shortCh.uniformScale > 0.0f, "Short character has positive scale");
    assertTrue(tallCh.uniformScale > shortCh.uniformScale, "Tall character has larger scale");
}

void testCharacterMorphWeights() {
    std::cout << "\n=== PCG: Character morph weights ===" << std::endl;
    using namespace atlas::pcg;

    CharacterMeshSystem cms;
    CharacterSliders sl;
    sl.height = 0.8f;
    sl.build = 0.3f;
    sl.limb_length = 0.6f;
    sl.torso_proportion = 0.7f;
    sl.head_shape = 0.2f;

    auto ch = cms.generate(55, Race::SynthBorn, BodyType::Organic, sl);
    assertTrue(ch.morphWeights.count("height") > 0, "height morph present");
    assertTrue(ch.morphWeights.count("build") > 0, "build morph present");
    assertTrue(ch.morphWeights.count("limb_length") > 0, "limb_length morph present");
    assertTrue(ch.morphWeights.count("torso_proportion") > 0, "torso_proportion morph present");
    assertTrue(ch.morphWeights.count("head_shape") > 0, "head_shape morph present");
    assertTrue(ch.morphWeights.at("height") >= 0.0f && ch.morphWeights.at("height") <= 1.0f,
               "height morph in [0,1]");
    assertTrue(ch.morphWeights.at("build") >= 0.0f && ch.morphWeights.at("build") <= 1.0f,
               "build morph in [0,1]");
}

void testFleetDoctrineGeneration() {
    std::cout << "\n=== PCG: FleetDoctrine basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 999, 1 };

    auto fleet = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Brawler, 10);

    assertTrue(fleet.doctrine == atlas::pcg::FleetDoctrine::Brawler, "Doctrine stored");
    assertTrue(static_cast<int>(fleet.slots.size()) == 10, "Correct ship count");
    assertTrue(fleet.totalShips == 10, "totalShips matches");

    // At least one commander.
    bool hasCmd = false;
    for (const auto& s : fleet.slots) {
        if (s.role == atlas::pcg::FleetRole::Commander) hasCmd = true;
    }
    assertTrue(hasCmd, "Fleet has a commander");
}

void testFleetDoctrineDeterminism() {
    std::cout << "\n=== PCG: FleetDoctrine determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 888, 1 };

    auto f1 = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Sniper, 8);
    auto f2 = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Sniper, 8);

    assertTrue(f1.slots.size() == f2.slots.size(), "Same fleet size");
    bool allMatch = true;
    for (size_t i = 0; i < f1.slots.size(); ++i) {
        if (f1.slots[i].role != f2.slots[i].role) { allMatch = false; break; }
        if (f1.slots[i].ship.mass != f2.slots[i].ship.mass) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Two fleets from same seed are identical");
}

void testFleetDoctrineRoles() {
    std::cout << "\n=== PCG: FleetDoctrine role distribution ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 1234, 1 };

    auto fleet = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Logistics, 20);

    int logiCount = 0;
    for (const auto& s : fleet.slots) {
        if (s.role == atlas::pcg::FleetRole::Logistics) logiCount++;
    }
    // Logistics doctrine should have many logi ships (50% target).
    assertTrue(logiCount >= 5, "Logistics doctrine has ≥ 5 logi ships in 20-ship fleet");
}

void testFleetDoctrineZeroShips() {
    std::cout << "\n=== PCG: FleetDoctrine zero ships ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 0, 1 };

    auto fleet = atlas::pcg::FleetDoctrineGenerator::generate(
        ctx, atlas::pcg::FleetDoctrine::Brawler, 0);
    assertTrue(fleet.slots.empty(), "Zero-ship fleet has no slots");
}

// ── Procedural Generation Systems tests ─────────────────────────────────

void testRoomGraphGeneration() {
    std::cout << "\n=== PCG: RoomGraph generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 42, 1 };
    auto rooms = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    assertTrue(!rooms.empty(), "Rooms generated for deck");
    assertTrue(rooms.size() >= 2, "At least 2 rooms per deck");
    for (const auto& r : rooms) {
        assertTrue(r.dimX > 0 && r.dimY > 0 && r.dimZ > 0, "Room has positive dimensions");
    }
}

void testRoomGraphDeterminism() {
    std::cout << "\n=== PCG: RoomGraph determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 42, 1 };
    auto rooms1 = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    auto rooms2 = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    assertTrue(rooms1.size() == rooms2.size(), "Same seed → same room count");
    bool allMatch = true;
    for (size_t i = 0; i < rooms1.size(); ++i) {
        if (rooms1[i].roomId != rooms2[i].roomId) { allMatch = false; break; }
        if (rooms1[i].type != rooms2[i].type) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical rooms");
}

void testDeckGraphGeneration() {
    std::cout << "\n=== PCG: DeckGraph generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 100, 1 };
    auto decks = atlas::pcg::generateDeckStack(3, ctx);
    assertTrue(decks.size() >= 2, "At least 2 decks generated");
    for (size_t i = 0; i < decks.size(); ++i) {
        assertTrue(decks[i].index == static_cast<int>(i), "Deck index correct");
    }
}

void testDeckGraphCorridors() {
    std::cout << "\n=== PCG: DeckGraph corridor connections ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 200, 1 };
    auto rooms = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    auto corridors = atlas::pcg::connectRooms(rooms);
    // Linear corridors: N-1.  Hub-and-spoke adds N-2 more if N >= 4.
    size_t expected = rooms.size() - 1;
    if (rooms.size() >= 4) expected += rooms.size() - 2;
    assertTrue(corridors.size() == expected, "Corridors match linear + hub-and-spoke count");
}

void testElevatorGeneration() {
    std::cout << "\n=== PCG: ElevatorSystem generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 300, 1 };
    auto decks = atlas::pcg::generateDeckStack(3, ctx);
    assertTrue(decks.size() >= 2, "Have multiple decks for elevator test");
    auto elevator = atlas::pcg::generateElevator(decks[0], static_cast<int>(decks.size()), ctx);
    assertTrue(elevator.floorCount == static_cast<int>(decks.size()), "Elevator covers all floors");
    assertTrue(elevator.buttons.size() == decks.size(), "Button for each floor");
}

void testHullMesherGeneration() {
    std::cout << "\n=== PCG: HullMesher generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 400, 1 };
    auto decks = atlas::pcg::generateDeckStack(2, ctx);
    for (auto& d : decks) {
        d.rooms = atlas::pcg::generateRoomsForDeck(d.index, ctx, 2);
    }
    auto hull = atlas::pcg::generateHullMesh(decks, 1.0f);
    assertTrue(!hull.vertices.empty(), "Hull has vertices");
    assertTrue(!hull.indices.empty(), "Hull has indices");
    assertTrue(hull.indices.size() % 3 == 0, "Hull indices are triangles");
}

void testCapitalShipGeneratorDeterminism() {
    std::cout << "\n=== PCG: CapitalShipGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 500, 1 };
    auto ship1 = atlas::pcg::CapitalShipGenerator::generate(ctx);
    auto ship2 = atlas::pcg::CapitalShipGenerator::generate(ctx);
    assertTrue(ship1.shipClass == ship2.shipClass, "Same ship class");
    assertTrue(ship1.decks.size() == ship2.decks.size(), "Same deck count");
    assertTrue(ship1.valid == ship2.valid, "Same validity");
}

void testCapitalShipGeneratorClassOverride() {
    std::cout << "\n=== PCG: CapitalShipGenerator class override ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 600, 1 };
    auto ship = atlas::pcg::CapitalShipGenerator::generate(ctx, 5);
    assertTrue(ship.shipClass == 5, "Ship class override applied");
    assertTrue(ship.decks.size() >= 2, "Capital has multiple decks");
    assertTrue(!ship.hull.vertices.empty(), "Capital has hull mesh");
}

void testCapitalShipGeneratorValidity() {
    std::cout << "\n=== PCG: CapitalShipGenerator validity ===" << std::endl;
    bool allValid = true;
    for (uint64_t i = 1; i <= 50; ++i) {
        atlas::pcg::PCGContext ctx{ i * 71, 1 };
        auto ship = atlas::pcg::CapitalShipGenerator::generate(ctx);
        if (!ship.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All 50 capital ships are valid");
}

void testShipDesignerOverride() {
    std::cout << "\n=== PCG: ShipDesigner room override ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 700, 1 };
    auto decks = atlas::pcg::generateDeckStack(2, ctx);
    for (auto& d : decks) {
        d.rooms = atlas::pcg::generateRoomsForDeck(d.index, ctx, 2);
    }
    int targetRoom = decks[0].rooms[0].roomId;
    atlas::pcg::ShipDesignerSave save;
    save.pcgVersion = 1;
    save.shipClass = 2;
    save.seed = 700;
    save.roomOverrides.push_back({ targetRoom, atlas::pcg::RoomType::Reactor });

    atlas::pcg::applyDesignerOverrides(decks, save);
    assertTrue(decks[0].rooms[0].type == atlas::pcg::RoomType::Reactor, "Room type overridden");
}

void testShipDesignerSaveLoad() {
    std::cout << "\n=== PCG: ShipDesigner save/load ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 800, 1 };
    auto decks = atlas::pcg::generateDeckStack(2, ctx);
    for (auto& d : decks) {
        d.rooms = atlas::pcg::generateRoomsForDeck(d.index, ctx, 2);
    }
    auto save = atlas::pcg::saveShipLayout(decks, 2, 800);
    assertTrue(save.pcgVersion == 1, "Save version set");
    assertTrue(save.shipClass == 2, "Save class stored");
    assertTrue(save.seed == 800, "Save seed stored");
}

void testSnappableGridCreation() {
    std::cout << "\n=== PCG: SnappableGrid creation ===" << std::endl;
    atlas::pcg::SnappableGrid grid(10, 10, 10, 5.0f);
    assertTrue(grid.width() == 10, "Grid width correct");
    assertTrue(grid.height() == 10, "Grid height correct");
    assertTrue(grid.depth() == 10, "Grid depth correct");
    assertTrue(std::abs(grid.cellSize() - 5.0f) < 0.001f, "Grid cell size correct");
}

void testSnappableGridPlacement() {
    std::cout << "\n=== PCG: SnappableGrid placement ===" << std::endl;
    atlas::pcg::SnappableGrid grid(10, 10, 10, 5.0f);
    assertTrue(grid.placeContent(0, 0, 0, 1), "Place content at (0,0,0)");
    assertTrue(!grid.placeContent(0, 0, 0, 2), "Cannot overwrite occupied cell");
    auto* cell = grid.getCell(0, 0, 0);
    assertTrue(cell != nullptr, "Cell exists");
    assertTrue(cell->occupied, "Cell is occupied");
    assertTrue(cell->contentType == 1, "Content type correct");
    assertTrue(grid.removeContent(0, 0, 0), "Remove content");
    assertTrue(!cell->occupied, "Cell cleared after removal");
}

void testSnappableGridSectorGeneration() {
    std::cout << "\n=== PCG: SnappableGrid sector generation ===" << std::endl;
    atlas::pcg::SnappableGrid grid(20, 20, 20, 5.0f);
    atlas::pcg::PCGContext ctx{ 1234, 1 };
    grid.generateSector(ctx);
    int occupied = 0;
    for (int x = 0; x < 20; ++x)
        for (int y = 0; y < 20; ++y)
            for (int z = 0; z < 20; ++z) {
                auto* c = grid.getCell(x, y, z);
                if (c && c->occupied) ++occupied;
            }
    assertTrue(occupied > 0, "Sector generation placed content");
    assertTrue(occupied < 20*20*20, "Not all cells occupied");
}

void testStationGeneratorBasic() {
    std::cout << "\n=== PCG: StationGenerator basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 2000, 1 };
    auto station = atlas::pcg::StationGenerator::generate(ctx);
    assertTrue(!station.modules.empty(), "Station has modules");
    assertTrue(station.modules.size() >= 3, "Station has >= 3 modules");
    assertTrue(station.valid, "Station is valid");
}

void testStationGeneratorDeterminism() {
    std::cout << "\n=== PCG: StationGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 2100, 1 };
    auto s1 = atlas::pcg::StationGenerator::generate(ctx);
    auto s2 = atlas::pcg::StationGenerator::generate(ctx);
    assertTrue(s1.modules.size() == s2.modules.size(), "Same module count");
    bool allMatch = true;
    for (size_t i = 0; i < s1.modules.size(); ++i) {
        if (s1.modules[i].type != s2.modules[i].type) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical stations");
}

void testStationGeneratorPower() {
    std::cout << "\n=== PCG: StationGenerator power balance ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 2200, 1 };
    auto station = atlas::pcg::StationGenerator::generate(ctx);
    atlas::pcg::StationGenerator::recalculatePower(station);
    assertTrue(station.totalPowerProduction >= station.totalPowerConsumption,
               "Station power production >= consumption");
}

void testSalvageFieldGeneration() {
    std::cout << "\n=== PCG: SalvageSystem field generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 3000, 1 };
    auto field = atlas::pcg::SalvageSystem::generateSalvageField(ctx, 20);
    assertTrue(field.totalNodes == 20, "Correct node count");
    assertTrue(static_cast<int>(field.nodes.size()) == 20, "Nodes vector matches");
    float totalValue = atlas::pcg::SalvageSystem::calculateTotalValue(field);
    assertTrue(totalValue > 0.0f, "Salvage has positive value");
}

void testSalvageFieldDeterminism() {
    std::cout << "\n=== PCG: SalvageSystem determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 3100, 1 };
    auto f1 = atlas::pcg::SalvageSystem::generateSalvageField(ctx, 15);
    auto f2 = atlas::pcg::SalvageSystem::generateSalvageField(ctx, 15);
    bool allMatch = true;
    for (size_t i = 0; i < f1.nodes.size(); ++i) {
        if (f1.nodes[i].category != f2.nodes[i].category) { allMatch = false; break; }
        if (f1.nodes[i].value != f2.nodes[i].value) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical salvage field");
}

void testSalvageFieldHiddenNodes() {
    std::cout << "\n=== PCG: SalvageSystem hidden nodes ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 3200, 1 };
    auto field = atlas::pcg::SalvageSystem::generateSalvageField(ctx, 100);
    assertTrue(field.hiddenNodes > 0, "Some nodes are hidden (require scan)");
    assertTrue(field.hiddenNodes < 100, "Not all nodes are hidden");
}

void testRoverGeneration() {
    std::cout << "\n=== PCG: RoverSystem generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 4000, 1 };
    auto rover = atlas::pcg::RoverSystem::generate(ctx);
    assertTrue(rover.valid, "Rover is valid");
    assertTrue(!rover.modules.empty(), "Rover has modules");
    assertTrue(rover.maxSpeed > 0.0f, "Rover has positive speed");
    assertTrue(rover.mass > 0.0f, "Rover has positive mass");
}

void testRoverDeployDock() {
    std::cout << "\n=== PCG: RoverSystem deploy/dock ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 4100, 1 };
    auto rover = atlas::pcg::RoverSystem::generate(ctx);
    assertTrue(!rover.deployed, "Rover starts docked");
    assertTrue(atlas::pcg::RoverSystem::deploy(rover, 10.0f, 0.0f, 5.0f), "Deploy succeeded");
    assertTrue(rover.deployed, "Rover is deployed");
    assertTrue(!atlas::pcg::RoverSystem::deploy(rover, 0, 0, 0), "Cannot deploy when already deployed");
    assertTrue(atlas::pcg::RoverSystem::dock(rover), "Dock succeeded");
    assertTrue(!rover.deployed, "Rover is docked");
}

void testRoverCargoCapacity() {
    std::cout << "\n=== PCG: RoverSystem cargo capacity ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 4200, 1 };
    auto rover = atlas::pcg::RoverSystem::generate(ctx);
    int cargo = atlas::pcg::RoverSystem::totalCargoCapacity(rover);
    assertTrue(cargo > 0, "Rover has cargo capacity");
}

void testCollisionManagerBasic() {
    std::cout << "\n=== PCG: CollisionManager basic operations ===" << std::endl;
    atlas::pcg::CollisionManager mgr;
    assertTrue(mgr.volumeCount() == 0, "Starts empty");

    atlas::pcg::AABB box = atlas::pcg::CollisionManager::computeRoomAABB(0, 0, 0, 4, 4, 4);
    atlas::pcg::CollisionVolume vol{ 1, box, true, true };
    mgr.addVolume(vol);
    assertTrue(mgr.volumeCount() == 1, "One volume after add");
    assertTrue(mgr.testPoint(0, 0, 0), "Point inside volume detected");
    assertTrue(!mgr.testPoint(10, 10, 10), "Point outside volume not detected");
}

void testCollisionManagerAABBQuery() {
    std::cout << "\n=== PCG: CollisionManager AABB query ===" << std::endl;
    atlas::pcg::CollisionManager mgr;
    atlas::pcg::AABB box1 = atlas::pcg::CollisionManager::computeRoomAABB(0, 0, 0, 4, 4, 4);
    atlas::pcg::AABB box2 = atlas::pcg::CollisionManager::computeRoomAABB(10, 0, 0, 4, 4, 4);
    mgr.addVolume({ 1, box1, true, true });
    mgr.addVolume({ 2, box2, true, true });

    atlas::pcg::AABB query = { -1, -1, -1, 1, 1, 1 };
    auto results = mgr.queryRegion(query);
    assertTrue(results.size() == 1, "Query finds one overlapping volume");
    assertTrue(results[0] == 1, "Correct volume found");
}

void testCollisionManagerRemove() {
    std::cout << "\n=== PCG: CollisionManager remove ===" << std::endl;
    atlas::pcg::CollisionManager mgr;
    atlas::pcg::AABB box = atlas::pcg::CollisionManager::computeRoomAABB(0, 0, 0, 4, 4, 4);
    mgr.addVolume({ 1, box, true, true });
    mgr.removeVolume(1);
    assertTrue(mgr.volumeCount() == 0, "Volume removed");
    assertTrue(!mgr.testPoint(0, 0, 0), "Point no longer detected after removal");
}

// ==================== Captain Departure System Tests ====================

void testDeparturePhaseNone() {
    std::cout << "\n=== Departure Phase None ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep1");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 2.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f);
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep1") == DP::None, "Low disagreement stays None");
}

void testDeparturePhaseGrumbling() {
    std::cout << "\n=== Departure Phase Grumbling ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep2");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 6.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f);
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep2") == DP::Grumbling, "Score >= grumble_threshold transitions to Grumbling");
}

void testDeparturePhaseRequest() {
    std::cout << "\n=== Departure Phase Request ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep3");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 11.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f);
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep3") == DP::FormalRequest, "Score >= request_threshold transitions to FormalRequest");
}

void testDepartureAcknowledge() {
    std::cout << "\n=== Departure Acknowledge ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep4");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 11.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f);
    sys.acknowledgeRequest("dep4");
    sys.update(0.1f);
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep4") == DP::Departing, "Acknowledge causes transition to Departing");
}

void testDepartureTimerCountdown() {
    std::cout << "\n=== Departure Timer Countdown ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep5");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 11.0f;
    state->departure_delay = 10.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f); // transitions to FormalRequest
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep5") == DP::FormalRequest, "In FormalRequest phase");
    sys.update(5.0f);
    assertTrue(state->departure_timer < 10.0f, "Timer counting down");
    sys.update(6.0f);
    assertTrue(sys.getDeparturePhase("dep5") == DP::Departing, "Timer expired, now Departing");
}

// ==================== Captain Transfer System Tests ====================

void testTransferHighMorale() {
    std::cout << "\n=== Transfer High Morale ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("xfer1");
    auto* req = addComp<components::CaptainTransferRequest>(e);
    auto* morale = addComp<components::FleetMorale>(e);
    morale->morale_score = 85.0f;

    systems::CaptainTransferSystem sys(&world);
    sys.update(1.0f);
    assertTrue(req->request_pending, "High morale triggers transfer request");
    assertTrue(req->request_type == components::CaptainTransferRequest::TransferType::BiggerShip,
               "High morale requests BiggerShip");
}

void testTransferLowMorale() {
    std::cout << "\n=== Transfer Low Morale ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("xfer2");
    auto* req = addComp<components::CaptainTransferRequest>(e);
    auto* morale = addComp<components::FleetMorale>(e);
    morale->morale_score = 20.0f;

    systems::CaptainTransferSystem sys(&world);
    sys.update(1.0f);
    assertTrue(req->request_pending, "Low morale triggers transfer request");
    assertTrue(req->request_type == components::CaptainTransferRequest::TransferType::EscortOnly,
               "Low morale requests EscortOnly");
}

void testTransferApprove() {
    std::cout << "\n=== Transfer Approve ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("xfer3");
    auto* req = addComp<components::CaptainTransferRequest>(e);
    auto* morale = addComp<components::FleetMorale>(e);
    morale->morale_score = 90.0f;

    systems::CaptainTransferSystem sys(&world);
    sys.update(1.0f);
    assertTrue(req->request_pending, "Request is pending");
    sys.approveTransfer("xfer3");
    assertTrue(!req->request_pending, "Approve clears pending");
}

// ==================== Character Mesh System Tests ====================

void testCharacterMeshGeneration() {
    std::cout << "\n=== Character Mesh Generation ===" << std::endl;
    pcg::CharacterMeshSystem meshSys;
    pcg::CharacterSliders sliders;
    sliders.height = 0.5f;
    auto character = meshSys.generate(12345, pcg::Race::TerranDescendant, sliders);
    assertTrue(character.total_height >= 1.5f && character.total_height <= 2.2f, "Height in valid range");
    assertTrue(character.arm_span > 0.0f, "Arm span positive");
    assertTrue(character.head_radius >= 0.09f && character.head_radius <= 0.13f, "Head radius in range");
}

void testCharacterMeshDeterminism() {
    std::cout << "\n=== Character Mesh Determinism ===" << std::endl;
    pcg::CharacterMeshSystem meshSys;
    pcg::CharacterSliders sliders;
    auto c1 = meshSys.generate(99999, pcg::Race::SynthBorn, sliders);
    auto c2 = meshSys.generate(99999, pcg::Race::SynthBorn, sliders);
    assertTrue(approxEqual(c1.total_height, c2.total_height), "Same seed same height");
    assertTrue(approxEqual(c1.arm_span, c2.arm_span), "Same seed same arm span");
}

void testCharacterRacialTraits() {
    std::cout << "\n=== Character Racial Traits ===" << std::endl;
    pcg::CharacterMeshSystem meshSys;
    pcg::CharacterSliders sliders;

    auto terran = meshSys.generate(1, pcg::Race::TerranDescendant, sliders);
    assertTrue(approxEqual(terran.learning_rate, 1.2f), "Terran learning rate 1.2");
    assertTrue(approxEqual(terran.diplomacy_bonus, 0.15f), "Terran diplomacy 0.15");

    auto synth = meshSys.generate(2, pcg::Race::SynthBorn, sliders);
    assertTrue(approxEqual(synth.automation_bonus, 0.25f), "SynthBorn automation 0.25");
    assertTrue(approxEqual(synth.resilience, 0.8f), "SynthBorn resilience 0.8");

    auto alien = meshSys.generate(3, pcg::Race::PureAlien, sliders);
    assertTrue(approxEqual(alien.resilience, 1.3f), "PureAlien resilience 1.3");

    auto hybrid = meshSys.generate(4, pcg::Race::HybridEvolutionary, sliders);
    assertTrue(approxEqual(hybrid.learning_rate, 1.1f), "Hybrid learning rate 1.1");
    assertTrue(approxEqual(hybrid.resilience, 1.1f), "Hybrid resilience 1.1");
}

// ==================== Crew System Tests ====================

void testCrewDefaults() {
    std::cout << "\n=== Crew Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* crew = addComp<components::ShipCrew>(e);
    assertTrue(crew->current_crew == 0, "No crew initially");
    assertTrue(crew->crew_member_ids.empty(), "Empty crew list");
}

void testCrewAssign() {
    std::cout << "\n=== Crew Assign ===" << std::endl;
    ecs::World world;
    auto* ship = world.createEntity("ship2");
    addComp<components::ShipCrew>(ship);
    auto* member = world.createEntity("crew_a");
    addComp<components::CrewMember>(member);

    systems::CrewSystem sys(&world);
    bool result = sys.assignCrew("ship2", "crew_a", "engine_room");
    assertTrue(result, "Assign succeeded");
    assertTrue(sys.getCrewCount("ship2") == 1, "Crew count is 1");
}

void testCrewEfficiency() {
    std::cout << "\n=== Crew Efficiency ===" << std::endl;
    ecs::World world;
    auto* ship = world.createEntity("ship3");
    addComp<components::ShipCrew>(ship);
    auto* m1 = world.createEntity("crew_b");
    auto* cm = addComp<components::CrewMember>(m1);
    cm->skill_level = 3.0f;
    cm->morale = 80.0f;

    systems::CrewSystem sys(&world);
    sys.assignCrew("ship3", "crew_b", "bridge");
    sys.update(0.0f);

    // efficiency_bonus = (3.0 * 0.1) + (80.0 / 200.0) = 0.3 + 0.4 = 0.7
    assertTrue(approxEqual(cm->efficiency_bonus, 0.7f), "Efficiency bonus is 0.7");
    assertTrue(approxEqual(sys.getOverallEfficiency("ship3"), 0.7f), "Overall efficiency is 0.7");
}

void testCrewRemove() {
    std::cout << "\n=== Crew Remove ===" << std::endl;
    ecs::World world;
    auto* ship = world.createEntity("ship4");
    addComp<components::ShipCrew>(ship);
    auto* m1 = world.createEntity("crew_c");
    addComp<components::CrewMember>(m1);

    systems::CrewSystem sys(&world);
    sys.assignCrew("ship4", "crew_c", "medbay");
    assertTrue(sys.getCrewCount("ship4") == 1, "Crew count 1 after assign");
    bool result = sys.removeCrew("ship4", "crew_c");
    assertTrue(result, "Remove succeeded");
    assertTrue(sys.getCrewCount("ship4") == 0, "Crew count 0 after remove");
}

// ==================== Character Creation Screen System Tests ====================

void testCharCreationScreenDefaults() {
    std::cout << "\n=== Char Creation Screen Defaults ===" << std::endl;
    components::CharacterCreationScreen screen;
    assertTrue(!screen.is_open, "Default not open");
    assertTrue(!screen.finalized, "Default not finalized");
    assertTrue(screen.selected_race.empty(), "No race selected");
    assertTrue(screen.selected_faction.empty(), "No faction selected");
}

void testCharCreationScreenOpen() {
    std::cout << "\n=== Char Creation Screen Open ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);

    assertTrue(sys.openScreen("player1"), "Open screen succeeds");
    assertTrue(sys.isScreenOpen("player1"), "Screen is open");
    assertTrue(!sys.openScreen("player1"), "Cannot open twice");
}

void testCharCreationScreenRaceSelect() {
    std::cout << "\n=== Char Creation Screen Race Select ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(sys.selectRace("player1", "Synth-Born"), "Valid race accepted");
    assertTrue(sys.getSelectedRace("player1") == "Synth-Born", "Race saved correctly");
    assertTrue(!sys.selectRace("player1", "InvalidRace"), "Invalid race rejected");
}

void testCharCreationScreenFactionSelect() {
    std::cout << "\n=== Char Creation Screen Faction Select ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(sys.selectFaction("player1", "Solari"), "Valid faction accepted");
    assertTrue(sys.getSelectedFaction("player1") == "Solari", "Faction saved correctly");
    assertTrue(!sys.selectFaction("player1", "InvalidFaction"), "Invalid faction rejected");
}

void testCharCreationScreenSliders() {
    std::cout << "\n=== Char Creation Screen Sliders ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(sys.setAttributeSlider("player1", "strength", 0.8f), "Set strength slider");
    assertTrue(approxEqual(sys.getAttributeSlider("player1", "strength"), 0.8f), "Strength value correct");
    assertTrue(sys.setAttributeSlider("player1", "agility", 1.5f), "Set agility clamped");
    assertTrue(approxEqual(sys.getAttributeSlider("player1", "agility"), 1.0f), "Agility clamped to 1.0");
    assertTrue(!sys.setAttributeSlider("player1", "invalid_attr", 0.5f), "Invalid attribute rejected");
}

void testCharCreationScreenValidation() {
    std::cout << "\n=== Char Creation Screen Validation ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(!sys.validateSelections("player1"), "Invalid without race/faction");
    sys.selectRace("player1", "Terran Descendant");
    assertTrue(!sys.validateSelections("player1"), "Invalid without faction");
    sys.selectFaction("player1", "Veyren");
    assertTrue(sys.validateSelections("player1"), "Valid with race and faction");
}

void testCharCreationScreenFinalize() {
    std::cout << "\n=== Char Creation Screen Finalize ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(!sys.finalizeCharacter("player1", "TestChar"), "Cannot finalize without selections");
    sys.selectRace("player1", "Pure Alien");
    sys.selectFaction("player1", "Aurelian");
    assertTrue(!sys.finalizeCharacter("player1", ""), "Cannot finalize with empty name");
    assertTrue(sys.finalizeCharacter("player1", "TestChar"), "Finalize succeeds");
    assertTrue(!sys.isScreenOpen("player1"), "Screen closed after finalize");
}

void testCharCreationScreenAppearance() {
    std::cout << "\n=== Char Creation Screen Appearance ===" << std::endl;
    ecs::World world;
    systems::CharacterCreationScreenSystem sys(&world);
    sys.openScreen("player1");

    assertTrue(sys.setAppearanceSlider("player1", "height", 0.7f), "Set height slider");
    assertTrue(sys.setAppearanceSlider("player1", "build", 0.3f), "Set build slider");
    assertTrue(!sys.setAppearanceSlider("player_none", "height", 0.5f), "Fails for non-existent player");
}

// ==================== PlayerProgressionSystem Tests ====================

void testPlayerProgressionInit() {
    std::cout << "\n=== Player Progression: Init ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    assertTrue(sys.initProgression("player1"), "Progression initialized");
    assertTrue(!sys.initProgression("player1"), "Duplicate init rejected");
    assertTrue(!sys.initProgression("nonexistent"), "Missing entity fails");
    assertTrue(sys.getLevel("player1") == 1, "Initial level is 1");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 0.0f), "Initial XP is 0");
}

void testPlayerProgressionAwardXP() {
    std::cout << "\n=== Player Progression: Award XP ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    assertTrue(sys.awardXP("player1", "combat", 50.0f), "Combat XP awarded");
    assertTrue(sys.awardXP("player1", "mining", 30.0f), "Mining XP awarded");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCategoryXP("player1", "combat"), 50.0f), "Combat XP is 50");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "mining"), 30.0f), "Mining XP is 30");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 80.0f), "Total XP is 80");
}

void testPlayerProgressionInvalidCategory() {
    std::cout << "\n=== Player Progression: Invalid Category ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    assertTrue(!sys.awardXP("player1", "invalid_category", 50.0f), "Invalid category rejected");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "invalid_category"), 0.0f), "Unknown category returns 0");
}

void testPlayerProgressionLevelUp() {
    std::cout << "\n=== Player Progression: Level Up ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    // Level 1 requires 100*1^1.5 = 100 XP
    sys.awardXP("player1", "combat", 100.0f);
    sys.update(0.0f);
    assertTrue(sys.getLevel("player1") == 2, "Level 2 after 100 XP");

    // Level 2 requires 100*2^1.5 ≈ 282.84 XP additional
    sys.awardXP("player1", "combat", 283.0f);
    sys.update(0.0f);
    assertTrue(sys.getLevel("player1") == 3, "Level 3 after ~383 XP total");
}

void testPlayerProgressionLevelProgress() {
    std::cout << "\n=== Player Progression: Level Progress ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    sys.awardXP("player1", "combat", 50.0f);
    sys.update(0.0f);
    // 50 XP, need 100 for level 1->2, so progress = 0.5
    float progress = sys.getLevelProgress("player1");
    assertTrue(approxEqual(progress, 0.5f), "Level progress is 0.5 at 50/100 XP");
}

void testPlayerProgressionMilestone() {
    std::cout << "\n=== Player Progression: Milestone ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    sys.addMilestone("player1", "First Blood", "combat", 25.0f);
    sys.addMilestone("player1", "Expert Miner", "mining", 100.0f);

    sys.awardXP("player1", "combat", 30.0f);
    sys.update(0.0f);
    assertTrue(sys.getMilestonesAchieved("player1") == 1, "One milestone achieved (First Blood)");

    sys.awardXP("player1", "mining", 110.0f);
    sys.update(0.0f);
    assertTrue(sys.getMilestonesAchieved("player1") == 2, "Two milestones achieved");
}

void testPlayerProgressionPrestige() {
    std::cout << "\n=== Player Progression: Prestige ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");

    // Can't prestige below level 50
    assertTrue(!sys.prestige("player1"), "Cannot prestige at level 1");

    // Award enough XP to reach level 50
    // Cumulative XP for levels 1-49: ~724,870
    sys.awardXP("player1", "combat", 730000.0f);
    sys.update(0.0f);
    assertTrue(sys.getLevel("player1") >= 50, "Reached level 50+");

    assertTrue(sys.prestige("player1"), "Prestige succeeds");
    assertTrue(sys.getPrestigeLevel("player1") == 1, "Prestige level is 1");
    assertTrue(approxEqual(sys.getPrestigeMultiplier("player1"), 1.1f), "Prestige multiplier is 1.1");
    assertTrue(sys.getLevel("player1") == 1, "Level reset to 1");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 0.0f), "XP reset to 0");
}

void testPlayerProgressionPrestigeXPBonus() {
    std::cout << "\n=== Player Progression: Prestige XP Bonus ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");

    // Get to prestige first
    sys.awardXP("player1", "combat", 730000.0f);
    sys.update(0.0f);
    sys.prestige("player1");

    // Now award XP with prestige bonus (1.1x)
    sys.awardXP("player1", "combat", 100.0f);
    sys.update(0.0f);
    // Should be 100 * 1.1 = 110 actual XP
    assertTrue(approxEqual(sys.getCategoryXP("player1", "combat"), 110.0f), "XP scaled by prestige multiplier");
}

void testPlayerProgressionAllCategories() {
    std::cout << "\n=== Player Progression: All Categories ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    sys.awardXP("player1", "combat", 10.0f);
    sys.awardXP("player1", "mining", 20.0f);
    sys.awardXP("player1", "exploration", 30.0f);
    sys.awardXP("player1", "industry", 40.0f);
    sys.awardXP("player1", "social", 50.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getTotalXP("player1"), 150.0f), "Total XP is sum of all categories");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "exploration"), 30.0f), "Exploration XP correct");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "industry"), 40.0f), "Industry XP correct");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "social"), 50.0f), "Social XP correct");
}

void testPlayerProgressionMissing() {
    std::cout << "\n=== Player Progression: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionSystem sys(&world);
    assertTrue(sys.getLevel("nonexistent") == 0, "Default level for missing");
    assertTrue(approxEqual(sys.getTotalXP("nonexistent"), 0.0f), "Default XP for missing");
    assertTrue(sys.getMilestonesAchieved("nonexistent") == 0, "Default milestones for missing");
    assertTrue(sys.getPrestigeLevel("nonexistent") == 0, "Default prestige for missing");
}

void testPlayerProgressionXPCurve() {
    std::cout << "\n=== Player Progression: XP Curve ===" << std::endl;
    float level1 = systems::PlayerProgressionSystem::xpForLevel(1);
    float level2 = systems::PlayerProgressionSystem::xpForLevel(2);
    float level10 = systems::PlayerProgressionSystem::xpForLevel(10);
    assertTrue(approxEqual(level1, 100.0f), "Level 1 requires 100 XP");
    assertTrue(level2 > level1, "Level 2 requires more XP than level 1");
    assertTrue(level10 > level2, "Level 10 requires more XP than level 2");
    // Level 2: 100*2^1.5 ≈ 282.84
    assertTrue(approxEqual(level2, 100.0f * std::pow(2.0f, 1.5f)), "Level 2 XP follows curve");
}

// ==================== CommanderDisagreementSystem Tests ====================

void testDisagreementRaise() {
    std::cout << "\n=== Commander Disagreement: Raise ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    assertTrue(sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy"), "Disagreement raised");
    assertTrue(sys.getActiveCount("fleet1") == 1, "One active disagreement");
    assertTrue(sys.getTotalDisagreements("fleet1") == 1, "Total disagreements is 1");
    assertTrue(sys.getFleetTension("fleet1") > 0.0f, "Fleet tension increased");
}

void testDisagreementResolveCompromise() {
    std::cout << "\n=== Commander Disagreement: Resolve Compromise ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Target");
    float tension_before = sys.getFleetTension("fleet1");
    assertTrue(sys.resolveDisagreement("fleet1", 0, "Compromise"), "Resolved by compromise");
    assertTrue(sys.getActiveCount("fleet1") == 0, "No active disagreements");
    assertTrue(sys.getTotalResolved("fleet1") == 1, "One resolved");
    assertTrue(sys.getResolution("fleet1", 0) == "Compromise", "Resolution is Compromise");
    assertTrue(sys.getFleetTension("fleet1") < tension_before, "Tension reduced after compromise");
}

void testDisagreementResolveVote() {
    std::cout << "\n=== Commander Disagreement: Resolve Vote ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Formation");
    assertTrue(sys.resolveDisagreement("fleet1", 0, "Vote"), "Resolved by vote");
    assertTrue(sys.getResolution("fleet1", 0) == "Vote", "Resolution is Vote");
    assertTrue(sys.getTotalResolved("fleet1") == 1, "One resolved");
}

void testDisagreementResolveAuthority() {
    std::cout << "\n=== Commander Disagreement: Resolve Authority ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Retreat");
    assertTrue(sys.resolveDisagreement("fleet1", 0, "AuthorityOverride"), "Resolved by authority");
    assertTrue(sys.getResolution("fleet1", 0) == "AuthorityOverride", "Resolution is AuthorityOverride");
    float impact = sys.getMoraleImpact("fleet1", 0);
    assertTrue(impact < -2.0f, "Authority override hurts morale more");
}

void testDisagreementEscalation() {
    std::cout << "\n=== Commander Disagreement: Escalation ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    auto* cd = addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "LootSplit");
    assertTrue(sys.getSeverity("fleet1", 0) == "Minor", "Starts at Minor");

    // Set short escalation for testing
    cd->disagreements[0].escalation_threshold = 5.0f;
    sys.update(5.0f);
    assertTrue(sys.getSeverity("fleet1", 0) == "Moderate", "Escalated to Moderate");
    sys.update(5.0f);
    assertTrue(sys.getSeverity("fleet1", 0) == "Serious", "Escalated to Serious");
    sys.update(5.0f);
    assertTrue(sys.getSeverity("fleet1", 0) == "Critical", "Escalated to Critical");
    assertTrue(sys.getActiveCount("fleet1") == 0, "Auto-resolved at Critical (escalated)");
}

void testDisagreementDismiss() {
    std::cout << "\n=== Commander Disagreement: Dismiss ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy");
    assertTrue(sys.dismissDisagreement("fleet1", 0), "Dismissed disagreement");
    assertTrue(sys.getActiveCount("fleet1") == 0, "No active after dismiss");
}

void testDisagreementMultiple() {
    std::cout << "\n=== Commander Disagreement: Multiple ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy");
    sys.raiseDisagreement("fleet1", "cmd_b", "cmd_c", "Target");
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_c", "Retreat");
    assertTrue(sys.getActiveCount("fleet1") == 3, "Three active disagreements");
    assertTrue(sys.getTotalDisagreements("fleet1") == 3, "Three total");
    assertTrue(sys.getFleetTension("fleet1") >= 15.0f, "High fleet tension from three disagreements");
}

void testDisagreementTensionCap() {
    std::cout << "\n=== Commander Disagreement: Tension Cap ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    auto* cd = addComp<components::CommanderDisagreement>(e);
    cd->fleet_tension = 98.0f;

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy");
    assertTrue(sys.getFleetTension("fleet1") <= 100.0f, "Tension capped at 100");
}

void testDisagreementAlreadyResolved() {
    std::cout << "\n=== Commander Disagreement: Already Resolved ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("fleet1");
    addComp<components::CommanderDisagreement>(e);

    systems::CommanderDisagreementSystem sys(&world);
    sys.raiseDisagreement("fleet1", "cmd_a", "cmd_b", "Strategy");
    sys.resolveDisagreement("fleet1", 0, "Vote");
    assertTrue(!sys.resolveDisagreement("fleet1", 0, "Compromise"), "Cannot resolve already resolved");
}

void testDisagreementMissing() {
    std::cout << "\n=== Commander Disagreement: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CommanderDisagreementSystem sys(&world);
    assertTrue(sys.getActiveCount("nonexistent") == 0, "Default active count 0");
    assertTrue(approxEqual(sys.getFleetTension("nonexistent"), 0.0f), "Default tension 0");
    assertTrue(sys.getTotalDisagreements("nonexistent") == 0, "Default total 0");
    assertTrue(sys.getTotalResolved("nonexistent") == 0, "Default resolved 0");
    assertTrue(sys.getSeverity("nonexistent", 0) == "Unknown", "Default severity Unknown");
    assertTrue(sys.getResolution("nonexistent", 0) == "None", "Default resolution None");
    assertTrue(approxEqual(sys.getMoraleImpact("nonexistent", 0), 0.0f), "Default morale impact 0");
}

// ==================== CaptainBackgroundSystem Tests ====================

void testCaptainBgAssign() {
    std::cout << "\n=== Captain Background: Assign ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.assignBackground("captain1", "ExMilitary"), "Background assigned");
    assertTrue(sys.getBackground("captain1") == "ExMilitary", "Background is ExMilitary");
    assertTrue(sys.getPreferredRole("captain1") == "Combat", "ExMilitary prefers Combat");
    assertTrue(sys.getDialogueFlavor("captain1") == "formal", "ExMilitary speaks formally");
}

void testCaptainBgFormerMiner() {
    std::cout << "\n=== Captain Background: Former Miner ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    sys.assignBackground("captain1", "FormerMiner");
    assertTrue(sys.getPreferredRole("captain1") == "Mining", "FormerMiner prefers Mining");
    assertTrue(sys.getDialogueFlavor("captain1") == "gruff", "FormerMiner is gruff");
    assertTrue(sys.getSkillCategory("captain1") == "mining", "Skill category is mining");
    assertTrue(sys.getSkillBonus("captain1") > 0.0f, "Has skill bonus");
    assertTrue(sys.getAggressionModifier("captain1") < 0.0f, "Miners are less aggressive");
}

void testCaptainBgSmuggler() {
    std::cout << "\n=== Captain Background: Smuggler ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    sys.assignBackground("captain1", "Smuggler");
    assertTrue(sys.getPreferredRole("captain1") == "Scout", "Smuggler prefers Scout");
    assertTrue(sys.getDialogueFlavor("captain1") == "sly", "Smuggler is sly");
    assertTrue(sys.getLoyaltyModifier("captain1") < 0.0f, "Smugglers have lower loyalty");
}

void testCaptainBgScientist() {
    std::cout << "\n=== Captain Background: Scientist ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    sys.assignBackground("captain1", "Scientist");
    assertTrue(sys.getPreferredRole("captain1") == "Support", "Scientist prefers Support");
    assertTrue(sys.getDialogueFlavor("captain1") == "analytical", "Scientist is analytical");
    assertTrue(sys.getSkillCategory("captain1") == "exploration", "Skill category is exploration");
    assertTrue(sys.getAggressionModifier("captain1") < 0.0f, "Scientists are less aggressive");
}

void testCaptainBgGenerate() {
    std::cout << "\n=== Captain Background: Generate from Seed ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.generateBackground("captain1", 42), "Generated background");
    std::string bg = sys.getBackground("captain1");
    assertTrue(bg != "Unknown", "Valid background generated");
    assertTrue(!sys.getPreferredRole("captain1").empty(), "Has preferred role");
    assertTrue(!sys.getDialogueFlavor("captain1").empty(), "Has dialogue flavor");
    assertTrue(sys.getExperience("captain1") > 0, "Has experience years");
    assertTrue(!sys.getOriginSystem("captain1").empty(), "Has origin system");
}

void testCaptainBgDeterministic() {
    std::cout << "\n=== Captain Background: Deterministic ===" << std::endl;
    ecs::World world;
    auto* e1 = world.createEntity("captain1");
    auto* e2 = world.createEntity("captain2");
    addComp<components::CaptainBackground>(e1);
    addComp<components::CaptainBackground>(e2);

    systems::CaptainBackgroundSystem sys(&world);
    sys.generateBackground("captain1", 12345);
    sys.generateBackground("captain2", 12345);
    assertTrue(sys.getBackground("captain1") == sys.getBackground("captain2"), "Same seed = same background");
    assertTrue(sys.getExperience("captain1") == sys.getExperience("captain2"), "Same seed = same experience");
    assertTrue(sys.getOriginSystem("captain1") == sys.getOriginSystem("captain2"), "Same seed = same origin");
}

void testCaptainBgSetOrigin() {
    std::cout << "\n=== Captain Background: Set Origin ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.setOriginSystem("captain1", "Solari Prime"), "Origin set");
    assertTrue(sys.getOriginSystem("captain1") == "Solari Prime", "Origin is Solari Prime");
}

void testCaptainBgSetExperience() {
    std::cout << "\n=== Captain Background: Set Experience ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("captain1");
    addComp<components::CaptainBackground>(e);

    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.setExperience("captain1", 15), "Experience set");
    assertTrue(sys.getExperience("captain1") == 15, "Experience is 15 years");
}

void testCaptainBgAllTypes() {
    std::cout << "\n=== Captain Background: All Types ===" << std::endl;
    ecs::World world;
    systems::CaptainBackgroundSystem sys(&world);
    const char* types[] = {"FormerMiner", "ExMilitary", "Smuggler", "Scientist",
                           "Noble", "Colonist", "BountyHunter", "Trader"};
    for (int i = 0; i < 8; ++i) {
        std::string id = "captain_" + std::to_string(i);
        auto* e = world.createEntity(id);
        addComp<components::CaptainBackground>(e);
        sys.assignBackground(id, types[i]);
        assertTrue(!sys.getPreferredRole(id).empty(), std::string(types[i]) + " has preferred role");
        assertTrue(!sys.getDialogueFlavor(id).empty(), std::string(types[i]) + " has dialogue flavor");
        assertTrue(sys.getSkillBonus(id) > 0.0f, std::string(types[i]) + " has skill bonus");
    }
}

void testCaptainBgMissing() {
    std::cout << "\n=== Captain Background: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CaptainBackgroundSystem sys(&world);
    assertTrue(sys.getBackground("nonexistent") == "Unknown", "Default background Unknown");
    assertTrue(sys.getPreferredRole("nonexistent").empty(), "Default role empty");
    assertTrue(sys.getDialogueFlavor("nonexistent").empty(), "Default flavor empty");
    assertTrue(approxEqual(sys.getSkillBonus("nonexistent"), 0.0f), "Default skill bonus 0");
    assertTrue(sys.getSkillCategory("nonexistent").empty(), "Default skill category empty");
    assertTrue(approxEqual(sys.getAggressionModifier("nonexistent"), 0.0f), "Default aggression mod 0");
    assertTrue(approxEqual(sys.getLoyaltyModifier("nonexistent"), 0.0f), "Default loyalty mod 0");
    assertTrue(sys.getExperience("nonexistent") == 0, "Default experience 0");
    assertTrue(sys.getOriginSystem("nonexistent").empty(), "Default origin empty");
}

// ==================== CrewTraining System Tests ====================

void testCrewTrainingCreate() {
    std::cout << "\n=== CrewTraining: Create ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeTraining("ship1"), "Init training succeeds");
    assertTrue(sys.getTraineeCount("ship1") == 0, "No trainees initially");
    assertTrue(sys.getTotalCompleted("ship1") == 0, "No completions initially");
    assertTrue(approxEqual(sys.getXpBonus("ship1"), 1.0f), "Default XP bonus is 1.0");
}

void testCrewTrainingEnroll() {
    std::cout << "\n=== CrewTraining: Enroll ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    assertTrue(sys.enrollTrainee("ship1", "crew1", "Navigation"), "Enroll succeeds");
    assertTrue(sys.getTraineeCount("ship1") == 1, "1 trainee");
    assertTrue(sys.getSkillName("ship1", "crew1") == "Navigation", "Skill is Navigation");
    assertTrue(approxEqual(sys.getProgress("ship1", "crew1"), 0.0f), "Initial progress is 0");
}

void testCrewTrainingDuplicate() {
    std::cout << "\n=== CrewTraining: Duplicate ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Navigation");
    assertTrue(!sys.enrollTrainee("ship1", "crew1", "Weapons"), "Duplicate trainee rejected");
    assertTrue(sys.getTraineeCount("ship1") == 1, "Still 1 trainee");
}

void testCrewTrainingProgress() {
    std::cout << "\n=== CrewTraining: Progress ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Gunnery");
    // training_rate=0.01, xp_bonus=1.0, delta=10 → progress = 0.1
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getProgress("ship1", "crew1"), 0.1f), "Progress is 0.1 after 10s");
    assertTrue(!sys.isComplete("ship1", "crew1"), "Not complete yet");
}

void testCrewTrainingCompletion() {
    std::cout << "\n=== CrewTraining: Completion ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Engineering");
    // training_rate=0.01, xp_bonus=1.0, delta=100 → progress = 1.0
    sys.update(100.0f);
    assertTrue(approxEqual(sys.getProgress("ship1", "crew1"), 1.0f), "Progress capped at 1.0");
    assertTrue(sys.isComplete("ship1", "crew1"), "Training complete");
    assertTrue(sys.getTotalCompleted("ship1") == 1, "1 completion counted");
}

void testCrewTrainingXpBonus() {
    std::cout << "\n=== CrewTraining: XpBonus ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Shields");
    assertTrue(sys.setXpBonus("ship1", 2.0f), "Set XP bonus succeeds");
    assertTrue(approxEqual(sys.getXpBonus("ship1"), 2.0f), "XP bonus is 2.0");
    // training_rate=0.01, xp_bonus=2.0, delta=10 → progress = 0.2
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getProgress("ship1", "crew1"), 0.2f), "Progress 0.2 with 2x bonus");
}

void testCrewTrainingRemove() {
    std::cout << "\n=== CrewTraining: Remove ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Navigation");
    sys.enrollTrainee("ship1", "crew2", "Weapons");
    assertTrue(sys.removeTrainee("ship1", "crew1"), "Remove succeeds");
    assertTrue(sys.getTraineeCount("ship1") == 1, "1 trainee remains");
    assertTrue(!sys.removeTrainee("ship1", "crew1"), "Remove nonexistent fails");
}

void testCrewTrainingMaxLimit() {
    std::cout << "\n=== CrewTraining: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    // default max_trainees = 5
    sys.enrollTrainee("ship1", "c1", "Skill1");
    sys.enrollTrainee("ship1", "c2", "Skill2");
    sys.enrollTrainee("ship1", "c3", "Skill3");
    sys.enrollTrainee("ship1", "c4", "Skill4");
    sys.enrollTrainee("ship1", "c5", "Skill5");
    assertTrue(!sys.enrollTrainee("ship1", "c6", "Skill6"), "Max limit enforced at 5");
    assertTrue(sys.getTraineeCount("ship1") == 5, "Still 5 trainees");
}

void testCrewTrainingMultiComplete() {
    std::cout << "\n=== CrewTraining: MultiComplete ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Gunnery");
    sys.enrollTrainee("ship1", "crew2", "Shields");
    sys.update(100.0f); // both complete
    assertTrue(sys.getTotalCompleted("ship1") == 2, "2 completions");
    assertTrue(sys.isComplete("ship1", "crew1"), "crew1 complete");
    assertTrue(sys.isComplete("ship1", "crew2"), "crew2 complete");
    // completed training doesn't increment again
    sys.update(10.0f);
    assertTrue(sys.getTotalCompleted("ship1") == 2, "Still 2 completions");
}

void testCrewTrainingMissing() {
    std::cout << "\n=== CrewTraining: Missing ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    assertTrue(!sys.initializeTraining("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.enrollTrainee("nonexistent", "c1", "Skill"), "Enroll fails on missing");
    assertTrue(!sys.removeTrainee("nonexistent", "c1"), "Remove fails on missing");
    assertTrue(sys.getTraineeCount("nonexistent") == 0, "0 trainees on missing");
    assertTrue(approxEqual(sys.getProgress("nonexistent", "c1"), 0.0f), "0 progress on missing");
    assertTrue(!sys.isComplete("nonexistent", "c1"), "Not complete on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completions on missing");
    assertTrue(!sys.setXpBonus("nonexistent", 2.0f), "Set XP fails on missing");
    assertTrue(approxEqual(sys.getXpBonus("nonexistent"), 0.0f), "0 XP bonus on missing");
    assertTrue(sys.getSkillName("nonexistent", "c1") == "", "Empty skill on missing");
}


void run_character_tests() {
    testCharacterCreate();
    testCharacterInvalidRace();
    testCharacterInstallImplant();
    testCharacterImplantSlotOccupied();
    testCharacterRemoveImplant();
    testCharacterCloneGrade();
    testCharacterRelayClone();
    testCharacterCloneCooldownDecay();
    testCharacterSecurityStatus();
    testCharacterEmploymentHistory();
    testCharacterRaceAttributes();
    testCaptainPersonalityAssign();
    testCaptainPersonalityFactionTraits();
    testCaptainPersonalitySetTrait();
    testCaptainPersonalityGetFaction();
    testCaptainPersonalityDeterministic();
    testFleetChatterSetActivity();
    testFleetChatterGetLine();
    testFleetChatterCooldown();
    testFleetChatterLinesSpoken();
    testFleetChatterCooldownExpires();
    testCaptainRelationshipRecordEvent();
    testCaptainRelationshipAbandoned();
    testCaptainRelationshipStatus();
    testCaptainRelationshipGrudge();
    testCaptainRelationshipMultipleEvents();
    testEmotionalArcVictory();
    testEmotionalArcDefeat();
    testEmotionalArcRest();
    testEmotionalArcTrust();
    testEmotionalArcBetray();
    testCaptainPersonalityNewTraitsAssigned();
    testCaptainPersonalityKeldariHighParanoia();
    testCaptainPersonalitySolariHighLoyalty();
    testCaptainPersonalitySetNewTrait();
    testCaptainPersonalityNewTraitsDeterministic();
    testCaptainMemoryRecordEvent();
    testCaptainMemoryMultipleEvents();
    testCaptainMemoryAverageWeight();
    testCaptainMemoryMostRecent();
    testCaptainMemoryCapacity();
    testCaptainMemoryNoEntity();
    testContextualChatterReturnsLine();
    testContextualChatterRespectsCooldown();
    testContextualChatterFallbackWithoutPersonality();
    testChatterInterruptHighPriority();
    testChatterInterruptLowPriorityFails();
    testChatterInterruptNotSpeaking();
    testFormationFriendCloser();
    testFormationGrudgeWider();
    testFormationNeutralDefault();
    testFormationAllySpacing();
    testFormationRivalSpacing();
    testDisagreementBasicScore();
    testDisagreementTaskMismatch();
    testDisagreementAggressiveLow();
    testSilenceInterpretationTriggered();
    testSilenceInterpretationNotTriggered();
    testCharacterBodyTypeOrganic();
    testCharacterBodyTypeAugmented();
    testCharacterBodyTypeCybernetic();
    testCharacterBodyTypeFullSynth();
    testCharacterBodyTypeMechFrame();
    testCharacterBodyTypeNames();
    testCharacterBackwardCompatibility();
    testCharacterReferenceMeshArchive();
    testCharacterUniformScale();
    testCharacterMorphWeights();
    testFleetDoctrineGeneration();
    testFleetDoctrineDeterminism();
    testFleetDoctrineRoles();
    testFleetDoctrineZeroShips();
    testRoomGraphGeneration();
    testRoomGraphDeterminism();
    testDeckGraphGeneration();
    testDeckGraphCorridors();
    testElevatorGeneration();
    testHullMesherGeneration();
    testCapitalShipGeneratorDeterminism();
    testCapitalShipGeneratorClassOverride();
    testCapitalShipGeneratorValidity();
    testShipDesignerOverride();
    testShipDesignerSaveLoad();
    testSnappableGridCreation();
    testSnappableGridPlacement();
    testSnappableGridSectorGeneration();
    testStationGeneratorBasic();
    testStationGeneratorDeterminism();
    testStationGeneratorPower();
    testSalvageFieldGeneration();
    testSalvageFieldDeterminism();
    testSalvageFieldHiddenNodes();
    testRoverGeneration();
    testRoverDeployDock();
    testRoverCargoCapacity();
    testCollisionManagerBasic();
    testCollisionManagerAABBQuery();
    testCollisionManagerRemove();
    testDeparturePhaseNone();
    testDeparturePhaseGrumbling();
    testDeparturePhaseRequest();
    testDepartureAcknowledge();
    testDepartureTimerCountdown();
    testTransferHighMorale();
    testTransferLowMorale();
    testTransferApprove();
    testCharacterMeshGeneration();
    testCharacterMeshDeterminism();
    testCharacterRacialTraits();
    testCrewDefaults();
    testCrewAssign();
    testCrewEfficiency();
    testCrewRemove();
    testCharCreationScreenDefaults();
    testCharCreationScreenOpen();
    testCharCreationScreenRaceSelect();
    testCharCreationScreenFactionSelect();
    testCharCreationScreenSliders();
    testCharCreationScreenValidation();
    testCharCreationScreenFinalize();
    testCharCreationScreenAppearance();
    testPlayerProgressionInit();
    testPlayerProgressionAwardXP();
    testPlayerProgressionInvalidCategory();
    testPlayerProgressionLevelUp();
    testPlayerProgressionLevelProgress();
    testPlayerProgressionMilestone();
    testPlayerProgressionPrestige();
    testPlayerProgressionPrestigeXPBonus();
    testPlayerProgressionAllCategories();
    testPlayerProgressionMissing();
    testPlayerProgressionXPCurve();
    testDisagreementRaise();
    testDisagreementResolveCompromise();
    testDisagreementResolveVote();
    testDisagreementResolveAuthority();
    testDisagreementEscalation();
    testDisagreementDismiss();
    testDisagreementMultiple();
    testDisagreementTensionCap();
    testDisagreementAlreadyResolved();
    testDisagreementMissing();
    testCaptainBgAssign();
    testCaptainBgFormerMiner();
    testCaptainBgSmuggler();
    testCaptainBgScientist();
    testCaptainBgGenerate();
    testCaptainBgDeterministic();
    testCaptainBgSetOrigin();
    testCaptainBgSetExperience();
    testCaptainBgAllTypes();
    testCaptainBgMissing();
    testCrewTrainingCreate();
    testCrewTrainingEnroll();
    testCrewTrainingDuplicate();
    testCrewTrainingProgress();
    testCrewTrainingCompletion();
    testCrewTrainingXpBonus();
    testCrewTrainingRemove();
    testCrewTrainingMaxLimit();
    testCrewTrainingMultiComplete();
    testCrewTrainingMissing();
}
