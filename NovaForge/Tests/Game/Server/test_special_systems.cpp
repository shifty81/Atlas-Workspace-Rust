/**
 * Special Systems tests
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

// ==================== Anomaly Visual Cue Tests ====================

void testAnomalyVisualCueDefaults() {
    std::cout << "\n=== Anomaly Visual Cue Defaults ===" << std::endl;
    components::AnomalyVisualCue cue;
    assertTrue(cue.cue_type == components::AnomalyVisualCue::CueType::None, "Default cue type is None");
    assertTrue(approxEqual(cue.intensity, 1.0f), "Default intensity is 1.0");
    assertTrue(approxEqual(cue.radius, 500.0f), "Default radius is 500.0");
    assertTrue(cue.active, "Default active is true");
    assertTrue(approxEqual(cue.distortion_strength, 0.0f), "Default distortion is 0.0");
}

void testAnomalyVisualCueTypeMapping() {
    std::cout << "\n=== Anomaly Visual Cue Type Mapping ===" << std::endl;
    using AT = components::Anomaly::Type;
    using CT = components::AnomalyVisualCue::CueType;

    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Wormhole) == CT::GravityLens,
               "Wormhole maps to GravityLens");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Gas) == CT::ParticleCloud,
               "Gas maps to ParticleCloud");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Combat) == CT::EnergyPulse,
               "Combat maps to EnergyPulse");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Mining) == CT::Shimmer,
               "Mining maps to Shimmer");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Data) == CT::ElectricArc,
               "Data maps to ElectricArc");
    assertTrue(systems::AnomalySystem::cueTypeFromAnomalyType(AT::Relic) == CT::Shimmer,
               "Relic maps to Shimmer");
}

// ==================== Legend System Tests ====================

void testLegendEmpty() {
    std::cout << "\n=== Legend Empty ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("player1");
    addComp<components::PlayerLegend>(e);

    systems::LegendSystem sys(&world);
    assertTrue(sys.getLegendScore("player1") == 0, "No score initially");
    assertTrue(sys.getTitle("player1") == "Unknown", "Title is Unknown");
}

void testLegendRecord() {
    std::cout << "\n=== Legend Record ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("player2");
    addComp<components::PlayerLegend>(e);

    systems::LegendSystem sys(&world);
    sys.recordLegend("player2", "destroyed_capital", "Destroyed the Warlord", 100.0f, "sys_1", 25);
    assertTrue(sys.getLegendScore("player2") == 25, "Score is 25 after event");
    auto entries = sys.getLegendEntries("player2");
    assertTrue(static_cast<int>(entries.size()) == 1, "One entry recorded");
}

void testLegendTitle() {
    std::cout << "\n=== Legend Title ===" << std::endl;
    assertTrue(systems::LegendSystem::computeTitle(0) == "Unknown", "Score 0 = Unknown");
    assertTrue(systems::LegendSystem::computeTitle(10) == "Notable", "Score 10 = Notable");
    assertTrue(systems::LegendSystem::computeTitle(50) == "Famous", "Score 50 = Famous");
    assertTrue(systems::LegendSystem::computeTitle(100) == "Legendary", "Score 100 = Legendary");
    assertTrue(systems::LegendSystem::computeTitle(500) == "Mythic", "Score 500 = Mythic");
}

// ==================== Ancient Tech Tests ====================

void testAncientTechDefaults() {
    std::cout << "\n=== Ancient Tech Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("tech1");
    auto* tech = addComp<components::AncientTechModule>(e);
    assertTrue(tech->state == components::AncientTechModule::TechState::Broken, "Starts Broken");
    assertTrue(!tech->isUsable(), "Not usable when broken");
}

void testAncientTechRepair() {
    std::cout << "\n=== Ancient Tech Repair ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("tech2");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->repair_cost = 10.0f;

    systems::AncientTechSystem sys(&world);
    assertTrue(sys.startRepair("tech2"), "Start repair succeeds");
    assertTrue(sys.getState("tech2") == components::AncientTechModule::TechState::Repairing, "State is Repairing");

    // Simulate enough time to complete (repair_cost * 0.5 = 5.0 seconds needed)
    sys.update(6.0f);
    assertTrue(sys.getState("tech2") == components::AncientTechModule::TechState::Repaired, "State is Repaired");
    assertTrue(sys.isUsable("tech2"), "Usable after repair");
}

void testAncientTechReverseEngineer() {
    std::cout << "\n=== Ancient Tech Reverse Engineer ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("tech3");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;
    tech->blueprint_id = "bp_ancient_weapon";

    systems::AncientTechSystem sys(&world);
    std::string bp = sys.reverseEngineer("tech3");
    assertTrue(bp == "bp_ancient_weapon", "Returns correct blueprint");
    assertTrue(tech->reverse_engineered, "Marked as reverse engineered");
}

// ==================== Survival System Tests ====================

void testSurvivalDefaults() {
    std::cout << "\n=== Survival Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("surv1");
    auto* needs = addComp<components::SurvivalNeeds>(e);
    assertTrue(approxEqual(needs->oxygen, 100.0f), "Full oxygen");
    assertTrue(approxEqual(needs->hunger, 0.0f), "No hunger");
    assertTrue(approxEqual(needs->fatigue, 0.0f), "No fatigue");
}

void testSurvivalDrain() {
    std::cout << "\n=== Survival Drain ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("surv2");
    addComp<components::SurvivalNeeds>(e);

    systems::SurvivalSystem sys(&world);
    sys.update(10.0f);

    auto [oxy, hun, fat] = sys.getNeeds("surv2");
    assertTrue(oxy < 100.0f, "Oxygen drained");
    assertTrue(approxEqual(oxy, 95.0f), "Oxygen at 95 after 10s");
}

void testSurvivalRefill() {
    std::cout << "\n=== Survival Refill ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("surv3");
    auto* needs = addComp<components::SurvivalNeeds>(e);
    needs->oxygen = 50.0f;

    systems::SurvivalSystem sys(&world);
    float newOxy = sys.refillOxygen("surv3", 30.0f);
    assertTrue(approxEqual(newOxy, 80.0f), "Oxygen refilled to 80");
}

void testSurvivalHunger() {
    std::cout << "\n=== Survival Hunger ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("surv4");
    addComp<components::SurvivalNeeds>(e);

    systems::SurvivalSystem sys(&world);
    sys.update(20.0f);

    auto [oxy, hun, fat] = sys.getNeeds("surv4");
    assertTrue(hun > 0.0f, "Hunger increased");
    assertTrue(approxEqual(hun, 2.0f), "Hunger at 2.0 after 20s");
}

// ==================== Menu System Tests ====================

void testMenuStateDefaults() {
    std::cout << "\n=== Menu State Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("menu1");
    auto* menu = addComp<components::MenuState>(e);
    assertTrue(menu->current_screen == components::MenuState::Screen::TitleScreen, "Starts at TitleScreen");
    assertTrue(menu->previous_screen == components::MenuState::Screen::TitleScreen, "Previous is TitleScreen");
    assertTrue(!menu->transition_active, "No transition active");
}

void testMenuNavigateTo() {
    std::cout << "\n=== Menu Navigate To ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("menu2");
    addComp<components::MenuState>(e);

    systems::MenuSystem sys(&world);
    bool result = sys.navigateTo("menu2", components::MenuState::Screen::NewGame);
    assertTrue(result, "Navigate succeeded");
    assertTrue(sys.getCurrentScreen("menu2") == components::MenuState::Screen::NewGame, "Screen is NewGame");
}

void testMenuGoBack() {
    std::cout << "\n=== Menu Go Back ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("menu3");
    addComp<components::MenuState>(e);

    systems::MenuSystem sys(&world);
    sys.navigateTo("menu3", components::MenuState::Screen::NewGame);
    bool result = sys.goBack("menu3");
    assertTrue(result, "GoBack succeeded");
    assertTrue(sys.getCurrentScreen("menu3") == components::MenuState::Screen::TitleScreen, "Back to TitleScreen");
}

void testMenuIsInGame() {
    std::cout << "\n=== Menu IsInGame ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("menu4");
    addComp<components::MenuState>(e);

    systems::MenuSystem sys(&world);
    assertTrue(!sys.isInGame("menu4"), "Not in game at title");
    sys.navigateTo("menu4", components::MenuState::Screen::InGame);
    assertTrue(sys.isInGame("menu4"), "In game after navigate");
}

// ==================== Race & Lore Tests ====================

void testRaceDefaults() {
    std::cout << "\n=== Race Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("race1");
    auto* info = addComp<components::RaceInfo>(e);
    assertTrue(info->race == components::RaceInfo::RaceName::TerranDescendant, "Default TerranDescendant");
    assertTrue(approxEqual(info->learning_rate, 1.0f), "Default learning rate 1.0");
}

void testRaceApplyDefaults() {
    std::cout << "\n=== Race Apply Defaults ===" << std::endl;
    ecs::World world;

    auto* e1 = world.createEntity("race_td");
    auto* td = addComp<components::RaceInfo>(e1);
    td->race = components::RaceInfo::RaceName::TerranDescendant;
    components::RaceInfo::applyRaceDefaults(*td);
    assertTrue(approxEqual(td->learning_rate, 1.2f), "TD learning rate 1.2");
    assertTrue(approxEqual(td->diplomacy_modifier, 0.15f), "TD diplomacy 0.15");

    auto* e2 = world.createEntity("race_sb");
    auto* sb = addComp<components::RaceInfo>(e2);
    sb->race = components::RaceInfo::RaceName::SynthBorn;
    components::RaceInfo::applyRaceDefaults(*sb);
    assertTrue(approxEqual(sb->automation_bonus, 0.25f), "SB automation 0.25");

    auto* e3 = world.createEntity("race_pa");
    auto* pa = addComp<components::RaceInfo>(e3);
    pa->race = components::RaceInfo::RaceName::PureAlien;
    components::RaceInfo::applyRaceDefaults(*pa);
    assertTrue(approxEqual(pa->environmental_resilience, 1.3f), "PA resilience 1.3");

    auto* e4 = world.createEntity("race_he");
    auto* he = addComp<components::RaceInfo>(e4);
    he->race = components::RaceInfo::RaceName::HybridEvolutionary;
    components::RaceInfo::applyRaceDefaults(*he);
    assertTrue(approxEqual(he->mutation_rate, 0.05f), "HE mutation 0.05");
}

void testLoreAddEntry() {
    std::cout << "\n=== Lore Add Entry ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("lore1");
    auto* lore = addComp<components::LoreEntry>(e);
    lore->addLore("Great Calamity", "Year 3355 event", 100.0f, "ship_log");
    assertTrue(lore->getLoreCount() == 1, "One lore entry");
    assertTrue(lore->discovered_lore[0].title == "Great Calamity", "Correct title");
}

void testLoreMaxEntries() {
    std::cout << "\n=== Lore Max Entries ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("lore2");
    auto* lore = addComp<components::LoreEntry>(e);
    lore->max_entries = 3;
    lore->addLore("A", "a", 1.0f, "ruin");
    lore->addLore("B", "b", 2.0f, "ruin");
    lore->addLore("C", "c", 3.0f, "ruin");
    lore->addLore("D", "d", 4.0f, "ruin");
    assertTrue(lore->getLoreCount() == 3, "Trimmed to max 3");
    assertTrue(lore->discovered_lore[0].title == "B", "Oldest removed");
}

// ==================== Visual Cue System Tests ====================

void testVisualCueDefaults() {
    std::cout << "\n=== Visual Cue Defaults ===" << std::endl;
    components::VisualCue cue;
    assertTrue(cue.lockdown_active == false, "Default lockdown_active is false");
    assertTrue(cue.lockdown_intensity == 0.0f, "Default lockdown_intensity is 0");
    assertTrue(cue.traffic_density == 0.0f, "Default traffic_density is 0");
    assertTrue(cue.traffic_ship_count == 0, "Default traffic_ship_count is 0");
    assertTrue(cue.threat_glow == 0.0f, "Default threat_glow is 0");
    assertTrue(cue.prosperity_indicator == 0.5f, "Default prosperity_indicator is 0.5");
    assertTrue(cue.pirate_warning == 0.0f, "Default pirate_warning is 0");
    assertTrue(cue.resource_highlight == 0.5f, "Default resource_highlight is 0.5");
    assertTrue(cue.dominant_faction.empty(), "Default dominant_faction is empty");
    assertTrue(cue.faction_influence_strength == 0.0f, "Default faction_influence_strength is 0");
}

void testVisualCueLockdown() {
    std::cout << "\n=== Visual Cue Lockdown ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_alpha");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->threat_level = 0.9f;
    state->security_level = 0.5f;

    vcSys.update(1.0f);
    assertTrue(vcSys.isLockdownActive("system_alpha") == true, "High threat triggers lockdown");

    state->threat_level = 0.1f;
    state->security_level = 0.1f;
    vcSys.update(1.0f);
    assertTrue(vcSys.isLockdownActive("system_alpha") == true, "Low security triggers lockdown");

    state->threat_level = 0.3f;
    state->security_level = 0.5f;
    vcSys.update(1.0f);
    assertTrue(vcSys.isLockdownActive("system_alpha") == false, "Moderate values no lockdown");
}

void testVisualCueTrafficDensity() {
    std::cout << "\n=== Visual Cue Traffic Density ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_beta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->traffic_level = 0.75f;

    vcSys.update(1.0f);
    float td = vcSys.getTrafficDensity("system_beta");
    assertTrue(td > 0.74f && td < 0.76f, "Traffic density mapped from sim state");
    auto* cue = sys->getComponent<components::VisualCue>();
    assertTrue(cue->traffic_ship_count == 75, "Traffic ship count is traffic * 100");
}

void testVisualCueThreatGlow() {
    std::cout << "\n=== Visual Cue Threat Glow ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_gamma");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->threat_level = 0.6f;

    vcSys.update(1.0f);
    float glow = vcSys.getThreatGlow("system_gamma");
    assertTrue(glow > 0.59f && glow < 0.61f, "Threat glow maps from threat_level");
}

void testVisualCueProsperity() {
    std::cout << "\n=== Visual Cue Prosperity ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_delta");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->economic_index = 0.85f;

    vcSys.update(1.0f);
    float p = vcSys.getProsperityIndicator("system_delta");
    assertTrue(p > 0.84f && p < 0.86f, "Prosperity maps from economic_index");
}

void testVisualCuePirateWarning() {
    std::cout << "\n=== Visual Cue Pirate Warning ===" << std::endl;
    ecs::World world;
    systems::VisualCueSystem vcSys(&world);

    auto* sys = world.createEntity("system_epsilon");
    auto* state = addComp<components::SimStarSystemState>(sys);
    sys->addComponent(std::make_unique<components::VisualCue>());
    state->pirate_activity = 0.7f;

    vcSys.update(1.0f);
    float pw = vcSys.getPirateWarning("system_epsilon");
    assertTrue(pw > 0.69f && pw < 0.71f, "Pirate warning maps from pirate_activity");
}

void testSupplyDemandDefaults() {
    std::cout << "\n=== Supply/Demand Defaults ===" << std::endl;
    components::SupplyDemand sd;
    assertTrue(sd.commodities.empty(), "Default commodities is empty");
    assertTrue(approxEqual(sd.price_elasticity, 0.5f), "Default price_elasticity is 0.5");
    assertTrue(approxEqual(sd.npc_activity_modifier, 1.0f), "Default npc_activity_modifier is 1.0");
    assertTrue(approxEqual(sd.price_floor_multiplier, 0.2f), "Default price_floor_multiplier is 0.2");
    assertTrue(approxEqual(sd.price_ceiling_multiplier, 5.0f), "Default price_ceiling_multiplier is 5.0");
    assertTrue(approxEqual(sd.supply_decay_rate, 0.01f), "Default supply_decay_rate is 0.01");
    assertTrue(approxEqual(sd.demand_drift_rate, 0.005f), "Default demand_drift_rate is 0.005");
}

void testSupplyDemandAddCommodity() {
    std::cout << "\n=== Supply/Demand Add Commodity ===" << std::endl;
    components::SupplyDemand sd;
    sd.addCommodity("ore", 50.0f, 200.0f, 150.0f);
    assertTrue(sd.getCommodityCount() == 1, "One commodity added");
    auto* c = sd.getCommodity("ore");
    assertTrue(c != nullptr, "Commodity found by id");
    assertTrue(approxEqual(c->base_price, 50.0f), "Base price is 50");
    assertTrue(approxEqual(c->supply, 200.0f), "Initial supply is 200");
    assertTrue(approxEqual(c->demand, 150.0f), "Initial demand is 150");
    sd.addCommodity("ore", 999.0f, 999.0f, 999.0f);
    assertTrue(sd.getCommodityCount() == 1, "Duplicate commodity not added");
    assertTrue(sd.getCommodity("nonexistent") == nullptr, "Unknown commodity returns null");
}

void testSupplyDemandPriceCalculation() {
    std::cout << "\n=== Supply/Demand Price Calculation ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sdSys(&world);
    auto* entity = world.createEntity("system_alpha");
    auto* sd = addComp<components::SupplyDemand>(entity);
    sd->addCommodity("ore", 100.0f, 50.0f, 150.0f);
    sd->supply_decay_rate = 0.0f;
    sd->demand_drift_rate = 0.0f;
    sd->commodities[0].supply_rate = 0.0f;

    sdSys.update(1.0f);
    auto* c = sd->getCommodity("ore");
    assertTrue(c->current_price > 100.0f, "Price increases when demand > supply");

    // Reset: supply > demand
    c->supply = 200.0f;
    c->demand = 50.0f;
    sdSys.update(1.0f);
    assertTrue(c->current_price < 100.0f, "Price decreases when supply > demand");
}

void testSupplyDemandPriceFloor() {
    std::cout << "\n=== Supply/Demand Price Floor ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sdSys(&world);
    auto* entity = world.createEntity("system_floor");
    auto* sd = addComp<components::SupplyDemand>(entity);
    sd->addCommodity("ore", 100.0f, 10000.0f, 1.0f);
    sd->supply_decay_rate = 0.0f;
    sd->demand_drift_rate = 0.0f;
    sd->commodities[0].supply_rate = 0.0f;
    sd->price_elasticity = 10.0f;

    sdSys.update(1.0f);
    auto* c = sd->getCommodity("ore");
    float floor = 100.0f * sd->price_floor_multiplier;
    assertTrue(c->current_price >= floor, "Price does not go below floor");
    assertTrue(approxEqual(c->current_price, floor, 0.01f), "Price clamped to floor");
}

void testSupplyDemandPriceCeiling() {
    std::cout << "\n=== Supply/Demand Price Ceiling ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sdSys(&world);
    auto* entity = world.createEntity("system_ceiling");
    auto* sd = addComp<components::SupplyDemand>(entity);
    sd->addCommodity("ore", 100.0f, 0.1f, 10000.0f);
    sd->supply_decay_rate = 0.0f;
    sd->demand_drift_rate = 0.0f;
    sd->commodities[0].supply_rate = 0.0f;
    sd->price_elasticity = 10.0f;

    sdSys.update(1.0f);
    auto* c = sd->getCommodity("ore");
    float ceiling = 100.0f * sd->price_ceiling_multiplier;
    assertTrue(c->current_price <= ceiling, "Price does not exceed ceiling");
    assertTrue(approxEqual(c->current_price, ceiling, 0.01f), "Price clamped to ceiling");
}

void testSupplyDemandNPCModifier() {
    std::cout << "\n=== Supply/Demand NPC Modifier ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sdSys(&world);
    auto* entity = world.createEntity("system_npc");
    auto* sd = addComp<components::SupplyDemand>(entity);
    sd->addCommodity("ore", 100.0f, 100.0f, 100.0f);
    sd->supply_decay_rate = 0.0f;
    sd->demand_drift_rate = 0.0f;
    sd->commodities[0].supply_rate = 10.0f;

    float supply_before = sd->commodities[0].supply;
    sd->npc_activity_modifier = 2.0f;
    sdSys.update(1.0f);
    float supply_after_2x = sd->commodities[0].supply;
    float gained_2x = supply_after_2x - supply_before;

    // Reset
    sd->commodities[0].supply = 100.0f;
    sd->npc_activity_modifier = 1.0f;
    sdSys.update(1.0f);
    float supply_after_1x = sd->commodities[0].supply;
    float gained_1x = supply_after_1x - 100.0f;

    assertTrue(approxEqual(gained_2x, gained_1x * 2.0f, 0.01f), "2x NPC modifier doubles supply gain");
}

// ==================== Myth Boss System Tests ====================

void testMythBossDefaults() {
    std::cout << "\n=== Myth Boss Defaults ===" << std::endl;
    ecs::World world;
    systems::MythBossSystem bossSys(&world);
    assertTrue(bossSys.getActiveBossCount() == 0, "No active bosses initially");
}

void testMythBossGenerateEncounter() {
    std::cout << "\n=== Myth Boss Generate Encounter ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("ancient_pilot", "Solari", "heroic", "Battle of the Ancients");
    std::string encId = bossSys.generateEncounter(mythId, "system_alpha");
    assertTrue(!encId.empty(), "Encounter ID returned");
    assertTrue(bossSys.isEncounterActive(encId), "Encounter is active");
    assertTrue(bossSys.getActiveBossCount() == 1, "One active boss");
}

void testMythBossDifficulty() {
    std::cout << "\n=== Myth Boss Difficulty ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("legend", "Veyren", "villainous");
    // Spread the myth to increase difficulty
    propSys.spreadMyth(mythId, "sys1");
    propSys.spreadMyth(mythId, "sys2");
    propSys.spreadMyth(mythId, "sys3");
    std::string encId = bossSys.generateEncounter(mythId, "system_beta");
    float diff = bossSys.getBossDifficulty(encId);
    assertTrue(diff > 1.0f, "Difficulty scales with myth spread");
}

void testMythBossComplete() {
    std::cout << "\n=== Myth Boss Complete ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("hero", "Aurelian", "mysterious");
    std::string encId = bossSys.generateEncounter(mythId, "system_gamma");
    assertTrue(bossSys.completeEncounter(encId, true), "Complete succeeds");
    assertTrue(!bossSys.isEncounterActive(encId), "No longer active after completion");
    assertTrue(bossSys.getActiveBossCount() == 0, "Zero active bosses after completion");
}

void testMythBossExpiry() {
    std::cout << "\n=== Myth Boss Expiry ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("fading_one", "Keldari", "exaggerated");
    std::string encId = bossSys.generateEncounter(mythId, "system_delta");
    assertTrue(bossSys.isEncounterActive(encId), "Active before expiry");

    // Simulate time past max duration
    bossSys.update(3601.0f);
    assertTrue(!bossSys.isEncounterActive(encId), "Expired after max duration");
}

void testMythBossTypeName() {
    std::cout << "\n=== Myth Boss Type Name ===" << std::endl;
    assertTrue(systems::MythBossSystem::getBossTypeName(0) == "Guardian", "Type 0 is Guardian");
    assertTrue(systems::MythBossSystem::getBossTypeName(1) == "Destroyer", "Type 1 is Destroyer");
    assertTrue(systems::MythBossSystem::getBossTypeName(2) == "Phantom", "Type 2 is Phantom");
    assertTrue(systems::MythBossSystem::getBossTypeName(3) == "Colossus", "Type 3 is Colossus");
    assertTrue(systems::MythBossSystem::getBossTypeName(4) == "Mirage", "Type 4 is Mirage");
    assertTrue(systems::MythBossSystem::getBossTypeName(99) == "Unknown", "Unknown type returns Unknown");
}

void testMythBossSourceMyth() {
    std::cout << "\n=== Myth Boss Source Myth ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    systems::MythBossSystem bossSys(&world);

    std::string mythId = propSys.generateMyth("tracker", "Solari", "fabricated");
    std::string encId = bossSys.generateEncounter(mythId, "system_epsilon");
    assertTrue(bossSys.getEncounterMythId(encId) == mythId, "Source myth ID matches");
}

// ==================== Ancient Tech Upgrade System Tests ====================

void testAncientTechUpgradeDefaults() {
    std::cout << "\n=== Ancient Tech Upgrade Defaults ===" << std::endl;
    ecs::World world;
    systems::AncientTechUpgradeSystem upgSys(&world);
    assertTrue(upgSys.getUpgradingCount() == 0, "No modules upgrading initially");
    assertTrue(upgSys.getUpgradedCount() == 0, "No modules upgraded initially");
}

void testAncientTechUpgradeStart() {
    std::cout << "\n=== Ancient Tech Upgrade Start ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient1");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;
    tech->tech_type = "shield";
    tech->power_multiplier = 1.8f;

    systems::AncientTechUpgradeSystem upgSys(&world);
    assertTrue(upgSys.startUpgrade("ancient1"), "Start upgrade succeeds on Repaired module");
    assertTrue(upgSys.getUpgradingCount() == 1, "One module upgrading");
    assertTrue(!upgSys.hasRuleBreakingBonuses("ancient1"), "Not yet upgraded");
}

void testAncientTechUpgradeNotRepaired() {
    std::cout << "\n=== Ancient Tech Upgrade Not Repaired ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient2");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Broken;

    systems::AncientTechUpgradeSystem upgSys(&world);
    assertTrue(!upgSys.startUpgrade("ancient2"), "Cannot upgrade Broken module");
}

void testAncientTechUpgradeComplete() {
    std::cout << "\n=== Ancient Tech Upgrade Complete ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient3");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;
    tech->tech_type = "weapon";
    tech->power_multiplier = 1.5f;
    auto* upg = addComp<components::AncientTechUpgradeState>(e);
    upg->upgrade_cost = 10.0f;

    systems::AncientTechUpgradeSystem upgSys(&world);
    assertTrue(upgSys.startUpgrade("ancient3"), "Start upgrade");

    upgSys.update(11.0f); // More than upgrade_cost
    assertTrue(tech->state == components::AncientTechModule::TechState::Upgraded, "Module is now Upgraded");
    assertTrue(upgSys.hasRuleBreakingBonuses("ancient3"), "Has rule-breaking bonuses");
    assertTrue(upgSys.getUpgradedCount() == 1, "One upgraded module");
    assertTrue(approxEqual(upgSys.getStatMultiplier("ancient3"), 1.5f), "Stat multiplier matches power_multiplier");
}

void testAncientTechUpgradeCancel() {
    std::cout << "\n=== Ancient Tech Upgrade Cancel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient4");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;

    systems::AncientTechUpgradeSystem upgSys(&world);
    upgSys.startUpgrade("ancient4");
    assertTrue(upgSys.cancelUpgrade("ancient4"), "Cancel succeeds");
    assertTrue(upgSys.getUpgradingCount() == 0, "No modules upgrading after cancel");
    assertTrue(tech->state == components::AncientTechModule::TechState::Repaired, "State back to Repaired");
}

void testAncientTechUpgradeProgress() {
    std::cout << "\n=== Ancient Tech Upgrade Progress ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient5");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Repaired;
    auto* upg = addComp<components::AncientTechUpgradeState>(e);
    upg->upgrade_cost = 100.0f;

    systems::AncientTechUpgradeSystem upgSys(&world);
    upgSys.startUpgrade("ancient5");
    upgSys.update(50.0f);
    assertTrue(approxEqual(upgSys.getUpgradeProgress("ancient5"), 0.5f), "50% progress after half time");
}

void testAncientTechUpgradeBonusDescription() {
    std::cout << "\n=== Ancient Tech Upgrade Bonus Description ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ancient6");
    auto* tech = addComp<components::AncientTechModule>(e);
    tech->state = components::AncientTechModule::TechState::Upgraded;
    tech->tech_type = "shield";
    tech->power_multiplier = 1.8f;

    systems::AncientTechUpgradeSystem upgSys(&world);
    std::string desc = upgSys.getBonusDescription("ancient6");
    assertTrue(!desc.empty(), "Bonus description not empty for upgraded module");
    assertTrue(desc.find("1.8") != std::string::npos, "Description contains multiplier value");
}

// ==================== Mod Manifest System Tests ====================

void testModManifestDefaults() {
    std::cout << "\n=== Mod Manifest Defaults ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);
    assertTrue(modSys.getModCount() == 0, "No mods registered initially");
    assertTrue(modSys.getEnabledModCount() == 0, "No mods enabled initially");
    assertTrue(modSys.validateAll(), "Empty registry is valid");
}

void testModManifestRegister() {
    std::cout << "\n=== Mod Manifest Register ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    assertTrue(modSys.registerMod("core_expansion", "Core Expansion", "1.0.0", "NovaForge Team"), "Register succeeds");
    assertTrue(modSys.isModRegistered("core_expansion"), "Mod is registered");
    assertTrue(modSys.getModCount() == 1, "One mod registered");
    assertTrue(modSys.isModEnabled("core_expansion"), "Mod enabled by default");
    assertTrue(modSys.getModVersion("core_expansion") == "1.0.0", "Version matches");
}

void testModManifestDuplicate() {
    std::cout << "\n=== Mod Manifest Duplicate ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("mod_a", "Mod A", "1.0.0", "Author A");
    assertTrue(!modSys.registerMod("mod_a", "Mod A Dupe", "2.0.0", "Author B"), "Duplicate mod ID rejected");
    assertTrue(modSys.getModCount() == 1, "Still one mod");
}

void testModManifestUnregister() {
    std::cout << "\n=== Mod Manifest Unregister ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("temp_mod", "Temp Mod", "0.1.0", "Tester");
    assertTrue(modSys.unregisterMod("temp_mod"), "Unregister succeeds");
    assertTrue(!modSys.isModRegistered("temp_mod"), "Mod no longer registered");
    assertTrue(modSys.getModCount() == 0, "Zero mods after unregister");
}

void testModManifestDependencies() {
    std::cout << "\n=== Mod Manifest Dependencies ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("base_lib", "Base Library", "1.0.0", "Core Team");
    modSys.registerMod("addon_pack", "Addon Pack", "1.0.0", "Addon Team", {"base_lib"});

    assertTrue(modSys.areDependenciesMet("addon_pack"), "Dependencies met when base_lib present");
    assertTrue(modSys.validateAll(), "All valid with dependencies satisfied");

    // Register mod with unmet dependency
    modSys.registerMod("broken_mod", "Broken Mod", "1.0.0", "Nobody", {"missing_lib"});
    assertTrue(!modSys.areDependenciesMet("broken_mod"), "Unmet dependency detected");
    assertTrue(!modSys.validateAll(), "Validation fails with unmet dependency");
}

void testModManifestLoadOrder() {
    std::cout << "\n=== Mod Manifest Load Order ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("foundation", "Foundation", "1.0.0", "Core");
    modSys.registerMod("graphics", "Graphics Pack", "1.0.0", "Art", {"foundation"});
    modSys.registerMod("gameplay", "Gameplay Mod", "1.0.0", "Design", {"foundation"});

    auto order = modSys.getLoadOrder();
    assertTrue(order.size() == 3, "All 3 mods in load order");
    // foundation must come before graphics and gameplay
    int foundIdx = -1, gfxIdx = -1, gameIdx = -1;
    for (size_t i = 0; i < order.size(); i++) {
        if (order[i] == "foundation") foundIdx = static_cast<int>(i);
        if (order[i] == "graphics") gfxIdx = static_cast<int>(i);
        if (order[i] == "gameplay") gameIdx = static_cast<int>(i);
    }
    assertTrue(foundIdx < gfxIdx, "Foundation loads before Graphics");
    assertTrue(foundIdx < gameIdx, "Foundation loads before Gameplay");
}

void testModManifestEnableDisable() {
    std::cout << "\n=== Mod Manifest Enable/Disable ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    modSys.registerMod("toggle_mod", "Toggle Mod", "1.0.0", "Dev");
    assertTrue(modSys.isModEnabled("toggle_mod"), "Enabled by default");

    assertTrue(modSys.setModEnabled("toggle_mod", false), "Disable succeeds");
    assertTrue(!modSys.isModEnabled("toggle_mod"), "Mod is disabled");
    assertTrue(modSys.getEnabledModCount() == 0, "Zero enabled mods");

    assertTrue(modSys.setModEnabled("toggle_mod", true), "Re-enable succeeds");
    assertTrue(modSys.isModEnabled("toggle_mod"), "Mod is enabled again");
}

void testModManifestEmptyId() {
    std::cout << "\n=== Mod Manifest Empty ID ===" << std::endl;
    ecs::World world;
    systems::ModManifestSystem modSys(&world);

    assertTrue(!modSys.registerMod("", "No ID", "1.0.0", "Nobody"), "Empty mod ID rejected");
    assertTrue(modSys.getModCount() == 0, "No mods registered with empty ID");
}

// ==================== Ancient AI Remnant System Tests ====================

void testAncientAIRemnantDefaults() {
    std::cout << "\n=== Ancient AI Remnant Defaults ===" << std::endl;
    components::AncientAIRemnant remnant;
    assertTrue(remnant.active == true, "Default active");
    assertTrue(remnant.defeated == false, "Default not defeated");
    assertTrue(remnant.tier == 1, "Default tier 1");
    assertTrue(remnant.remnant_type == components::AncientAIRemnant::RemnantType::Sentinel, "Default Sentinel type");
    assertTrue(remnant.isActive(), "Active by default");
}

void testAncientAIRemnantSpawn() {
    std::cout << "\n=== Ancient AI Remnant Spawn ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id = sys.spawnRemnant("ancient_site_1", 3);
    assertTrue(!id.empty(), "Remnant ID generated");
    assertTrue(sys.isRemnantActive(id), "Remnant is active after spawn");
    assertTrue(sys.getActiveRemnantCount() == 1, "One active remnant");
}

void testAncientAIRemnantTierScaling() {
    std::cout << "\n=== Ancient AI Remnant Tier Scaling ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id1 = sys.spawnRemnant("site_1", 1);
    std::string id5 = sys.spawnRemnant("site_5", 5);

    float diff1 = sys.getRemnantDifficulty(id1);
    float diff5 = sys.getRemnantDifficulty(id5);
    assertTrue(diff5 > diff1, "Higher tier = higher difficulty");
    assertTrue(approxEqual(diff1, 1.0f), "Tier 1 difficulty is 1.0");
    assertTrue(approxEqual(diff5, 5.0f), "Tier 5 difficulty is 5.0");
}

void testAncientAIRemnantDefeat() {
    std::cout << "\n=== Ancient AI Remnant Defeat ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id = sys.spawnRemnant("ancient_site_1", 2);
    assertTrue(sys.isRemnantActive(id), "Active before defeat");
    assertTrue(sys.defeatRemnant(id), "Defeat succeeds");
    assertTrue(!sys.isRemnantActive(id), "Inactive after defeat");
    assertTrue(sys.getActiveRemnantCount() == 0, "Zero active after defeat");
}

void testAncientAIRemnantExpiry() {
    std::cout << "\n=== Ancient AI Remnant Expiry ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id = sys.spawnRemnant("ancient_site_1", 1);
    // Default max_duration is 7200s. Advance time past that.
    sys.update(7201.0f);
    assertTrue(!sys.isRemnantActive(id), "Inactive after expiry");
}

void testAncientAIRemnantSiteId() {
    std::cout << "\n=== Ancient AI Remnant Site ID ===" << std::endl;
    ecs::World world;
    systems::AncientAIRemnantSystem sys(&world);

    std::string id = sys.spawnRemnant("ancient_ruins_alpha", 4);
    assertTrue(sys.getRemnantSiteId(id) == "ancient_ruins_alpha", "Site ID matches");
}

void testAncientAIRemnantTypeName() {
    std::cout << "\n=== Ancient AI Remnant Type Name ===" << std::endl;
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(0) == "Sentinel", "Type 0 is Sentinel");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(1) == "Swarm", "Type 1 is Swarm");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(2) == "Construct", "Type 2 is Construct");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(3) == "Warden", "Type 3 is Warden");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(4) == "Leviathan", "Type 4 is Leviathan");
    assertTrue(systems::AncientAIRemnantSystem::getRemnantTypeName(99) == "Unknown", "Invalid type is Unknown");
}

// ==================== View Mode Transition System Tests ====================

void testViewModeDefaults() {
    std::cout << "\n=== View Mode Defaults ===" << std::endl;
    components::ViewModeState state;
    assertTrue(state.current_mode == 1, "Default mode is Interior (1)");
    assertTrue(!state.transitioning, "Default not transitioning");
    assertTrue(approxEqual(state.transition_progress, 0.0f), "Default zero progress");
}

void testViewModeInitialize() {
    std::cout << "\n=== View Mode Initialize ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);

    assertTrue(sys.initializePlayer("player1"), "Initialize succeeds");
    assertTrue(sys.getCurrentMode("player1") == 1, "Default Interior mode");
    assertTrue(!sys.isTransitioning("player1"), "Not transitioning initially");
}

void testViewModeTransition() {
    std::cout << "\n=== View Mode Transition ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);
    sys.initializePlayer("player1");

    int cockpit = static_cast<int>(components::ViewModeState::Mode::Cockpit);
    assertTrue(sys.requestTransition("player1", cockpit), "Interior to Cockpit valid");
    assertTrue(sys.isTransitioning("player1"), "Transitioning after request");

    // Complete transition
    sys.update(2.0f);
    assertTrue(!sys.isTransitioning("player1"), "Not transitioning after completion");
    assertTrue(sys.getCurrentMode("player1") == cockpit, "Now in Cockpit mode");
}

void testViewModeInvalidTransition() {
    std::cout << "\n=== View Mode Invalid Transition ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);
    sys.initializePlayer("player1");

    int eva = static_cast<int>(components::ViewModeState::Mode::EVA);
    // Interior -> EVA is a valid adjacent transition, so test a truly invalid one:
    // First transition to RTS Overlay, then try EVA which is not adjacent to RTS
    int rts = static_cast<int>(components::ViewModeState::Mode::RTSOverlay);
    sys.requestTransition("player1", rts);
    sys.update(2.0f); // complete transition to RTS
    assertTrue(!sys.requestTransition("player1", eva), "RTS to EVA invalid (not adjacent)");
}

void testViewModeCancel() {
    std::cout << "\n=== View Mode Cancel ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);
    sys.initializePlayer("player1");

    int cockpit = static_cast<int>(components::ViewModeState::Mode::Cockpit);
    sys.requestTransition("player1", cockpit);
    assertTrue(sys.isTransitioning("player1"), "Transitioning");
    assertTrue(sys.cancelTransition("player1"), "Cancel succeeds");
    assertTrue(!sys.isTransitioning("player1"), "No longer transitioning");
    assertTrue(sys.getCurrentMode("player1") == 1, "Still in Interior after cancel");
}

void testViewModeProgress() {
    std::cout << "\n=== View Mode Progress ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);
    sys.initializePlayer("player1");

    int cockpit = static_cast<int>(components::ViewModeState::Mode::Cockpit);
    sys.requestTransition("player1", cockpit);
    sys.update(0.75f); // Half of 1.5s default duration
    float progress = sys.getTransitionProgress("player1");
    assertTrue(progress > 0.0f && progress < 1.0f, "Progress is mid-transition");
    assertTrue(sys.isTransitioning("player1"), "Still transitioning");
}

void testViewModeNames() {
    std::cout << "\n=== View Mode Names ===" << std::endl;
    assertTrue(systems::ViewModeTransitionSystem::getModeName(0) == "Cockpit", "Mode 0 is Cockpit");
    assertTrue(systems::ViewModeTransitionSystem::getModeName(1) == "Interior", "Mode 1 is Interior");
    assertTrue(systems::ViewModeTransitionSystem::getModeName(2) == "EVA", "Mode 2 is EVA");
    assertTrue(systems::ViewModeTransitionSystem::getModeName(3) == "RTS Overlay", "Mode 3 is RTS Overlay");
    assertTrue(systems::ViewModeTransitionSystem::getModeName(99) == "Unknown", "Invalid mode is Unknown");
}

// ==================== WingManagementSystem Tests ====================

void testWingManagementCreateWing() {
    std::cout << "\n=== Wing Management: Create Wing ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    assertTrue(sys.createWing("fleet1", "wing_alpha", "Combat"), "Create combat wing succeeds");
    assertTrue(sys.getWingCount("fleet1") == 1, "Fleet has 1 wing");
    assertTrue(sys.getWingRole("fleet1", "wing_alpha") == "Combat", "Wing role is Combat");
}

void testWingManagementDuplicateWing() {
    std::cout << "\n=== Wing Management: Duplicate Wing ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    assertTrue(!sys.createWing("fleet1", "wing_alpha", "Mining"), "Duplicate wing creation fails");
    assertTrue(sys.getWingCount("fleet1") == 1, "Still 1 wing");
}

void testWingManagementDissolveWing() {
    std::cout << "\n=== Wing Management: Dissolve Wing ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    assertTrue(sys.dissolveWing("fleet1", "wing_alpha"), "Dissolve wing succeeds");
    assertTrue(sys.getWingCount("fleet1") == 0, "Fleet has 0 wings");
}

void testWingManagementAssignMember() {
    std::cout << "\n=== Wing Management: Assign Member ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Mining");
    assertTrue(sys.assignToWing("fleet1", "wing_alpha", "pilot1"), "Assign pilot1 succeeds");
    assertTrue(sys.assignToWing("fleet1", "wing_alpha", "pilot2"), "Assign pilot2 succeeds");

    auto members = sys.getWingMembers("fleet1", "wing_alpha");
    assertTrue(static_cast<int>(members.size()) == 2, "Wing has 2 members");
}

void testWingManagementMaxMembers() {
    std::cout << "\n=== Wing Management: Max Members ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    for (int i = 0; i < 5; i++) {
        sys.assignToWing("fleet1", "wing_alpha", "pilot" + std::to_string(i));
    }
    assertTrue(!sys.assignToWing("fleet1", "wing_alpha", "pilot5"), "6th member rejected");
    assertTrue(static_cast<int>(sys.getWingMembers("fleet1", "wing_alpha").size()) == 5, "Wing capped at 5");
}

void testWingManagementRemoveMember() {
    std::cout << "\n=== Wing Management: Remove Member ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    sys.assignToWing("fleet1", "wing_alpha", "pilot1");
    sys.assignToWing("fleet1", "wing_alpha", "pilot2");
    assertTrue(sys.removeFromWing("fleet1", "wing_alpha", "pilot1"), "Remove pilot1 succeeds");
    assertTrue(static_cast<int>(sys.getWingMembers("fleet1", "wing_alpha").size()) == 1, "Wing has 1 member");
}

void testWingManagementSetCommander() {
    std::cout << "\n=== Wing Management: Set Commander ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Logistics");
    sys.assignToWing("fleet1", "wing_alpha", "pilot1");
    assertTrue(sys.setWingCommander("fleet1", "wing_alpha", "pilot1"), "Set commander succeeds");
    assertTrue(sys.getWingCommander("fleet1", "wing_alpha") == "pilot1", "Commander is pilot1");
}

void testWingManagementMorale() {
    std::cout << "\n=== Wing Management: Wing Morale ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    auto* p1 = world.createEntity("pilot1");
    auto* m1 = addComp<components::FleetMorale>(p1);
    m1->morale_score = 80.0f;

    auto* p2 = world.createEntity("pilot2");
    auto* m2 = addComp<components::FleetMorale>(p2);
    m2->morale_score = 40.0f;

    systems::WingManagementSystem sys(&world);
    sys.createWing("fleet1", "wing_alpha", "Combat");
    sys.assignToWing("fleet1", "wing_alpha", "pilot1");
    sys.assignToWing("fleet1", "wing_alpha", "pilot2");
    sys.update(1.0f);

    // morale_score is -100..+100, mapped to 0..100: (score+100)/2
    // pilot1: (80+100)/2=90, pilot2: (40+100)/2=70, avg=80
    float morale = sys.getWingMorale("fleet1", "wing_alpha");
    assertTrue(approxEqual(morale, 80.0f), "Wing morale is average of mapped members (80)");
}

void testWingManagementInvalidFleet() {
    std::cout << "\n=== Wing Management: Invalid Fleet ===" << std::endl;
    ecs::World world;
    systems::WingManagementSystem sys(&world);
    assertTrue(!sys.createWing("nonexistent", "w1", "Combat"), "Create wing on missing fleet fails");
    assertTrue(sys.getWingCount("nonexistent") == 0, "Missing fleet has 0 wings");
    assertTrue(sys.getWingMorale("nonexistent", "w1") == 50.0f, "Default morale for missing");
}

void testWingManagementInvalidRole() {
    std::cout << "\n=== Wing Management: Invalid Role ===" << std::endl;
    ecs::World world;
    auto* fleet = world.createEntity("fleet1");
    addComp<components::FleetProgression>(fleet);

    systems::WingManagementSystem sys(&world);
    assertTrue(!sys.createWing("fleet1", "w1", "InvalidRole"), "Invalid role rejected");
    assertTrue(sys.getWingCount("fleet1") == 0, "No wings created");
}

// ==================== TerraformingSystem Tests ====================

void testTerraformingStart() {
    std::cout << "\n=== Terraforming: Start ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    assertTrue(sys.startTerraforming("planet1", "kepler-442b"), "Terraforming started");
    assertTrue(sys.isActive("planet1"), "Terraforming is active");
    assertTrue(sys.getStage("planet1") == "planning", "Stage is planning");
    assertTrue(!sys.startTerraforming("planet1", "kepler-442b"), "Duplicate start rejected");
}

void testTerraformingPause() {
    std::cout << "\n=== Terraforming: Pause ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    assertTrue(sys.pauseTerraforming("planet1"), "Terraforming paused");
    assertTrue(!sys.isActive("planet1"), "Terraforming is inactive");
}

void testTerraformingResume() {
    std::cout << "\n=== Terraforming: Resume ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    sys.pauseTerraforming("planet1");
    assertTrue(sys.resumeTerraforming("planet1"), "Terraforming resumed");
    assertTrue(sys.isActive("planet1"), "Terraforming is active again");
}

void testTerraformingCancel() {
    std::cout << "\n=== Terraforming: Cancel ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    assertTrue(sys.cancelTerraforming("planet1"), "Terraforming cancelled");
    assertTrue(sys.getStage("planet1") == "unknown", "No stage after cancel");
    assertTrue(!sys.cancelTerraforming("planet1"), "Cancel on missing fails");
}

void testTerraformingStageAdvance() {
    std::cout << "\n=== Terraforming: Stage Advance ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    assertTrue(sys.getStage("planet1") == "planning", "Starts at planning");
    sys.advanceStage("planet1");
    assertTrue(sys.getStage("planet1") == "infrastructure", "Advanced to infrastructure");
    sys.advanceStage("planet1");
    assertTrue(sys.getStage("planet1") == "atmosphere_processing", "Advanced to atmosphere_processing");
}

void testTerraformingAutoAdvance() {
    std::cout << "\n=== Terraforming: Auto Advance ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    auto* entity = world.getEntity("planet1");
    auto* tf = entity->getComponent<components::Terraforming>();
    tf->time_per_stage = 10.0f; // speed up for test

    sys.update(11.0f); // past one stage
    assertTrue(sys.getStage("planet1") == "infrastructure", "Auto-advanced to infrastructure");
}

void testTerraformingSetTargets() {
    std::cout << "\n=== Terraforming: Set Targets ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    assertTrue(sys.setTargets("planet1", 0.8f, 300.0f, 0.5f), "Targets set");
    assertTrue(!sys.setTargets("nonexistent", 0.8f, 300.0f, 0.5f), "Missing entity fails");
}

void testTerraformingProgress() {
    std::cout << "\n=== Terraforming: Progress ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    auto* entity = world.getEntity("planet1");
    auto* tf = entity->getComponent<components::Terraforming>();
    tf->time_per_stage = 100.0f;

    sys.update(50.0f); // 50% through first stage
    assertTrue(approxEqual(sys.getProgress("planet1"), 0.5f), "Progress is 0.5 in stage");
    assertTrue(sys.getTotalProgress("planet1") > 0.0f, "Total progress > 0");
    assertTrue(sys.getTotalCreditsSpent("planet1") > 0.0, "Credits spent > 0");
}

void testTerraformingComplete() {
    std::cout << "\n=== Terraforming: Complete ===" << std::endl;
    ecs::World world;
    world.createEntity("planet1");

    systems::TerraformingSystem sys(&world);
    sys.startTerraforming("planet1", "kepler-442b");
    // Advance through all stages to completion
    sys.advanceStage("planet1"); // -> infrastructure
    sys.advanceStage("planet1"); // -> atmosphere_processing
    sys.advanceStage("planet1"); // -> temperature_regulation
    sys.advanceStage("planet1"); // -> biome_seeding
    sys.advanceStage("planet1"); // -> complete
    assertTrue(sys.getStage("planet1") == "complete", "Stage is complete");
    assertTrue(approxEqual(sys.getTotalProgress("planet1"), 1.0f), "Total progress is 1.0");
    assertTrue(!sys.advanceStage("planet1"), "Cannot advance past complete");
}

void testTerraformingMissing() {
    std::cout << "\n=== Terraforming: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::TerraformingSystem sys(&world);
    assertTrue(sys.getStage("nonexistent") == "unknown", "Default stage for missing");
    assertTrue(approxEqual(sys.getProgress("nonexistent"), 0.0f), "Default progress for missing");
    assertTrue(approxEqual(sys.getTotalProgress("nonexistent"), 0.0f), "Default total progress for missing");
    assertTrue(!sys.isActive("nonexistent"), "Default inactive for missing");
    assertTrue(sys.getTotalCreditsSpent("nonexistent") == 0.0, "Default credits for missing");
}

// ==================== FoodProcessorSystem Tests ====================

void testFoodProcessorAddRecipe() {
    std::cout << "\n=== Food Processor: Add Recipe ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}, {"water", 1}};
    assertTrue(sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f), "Recipe added");
    assertTrue(sys.getRecipeCount("processor1") == 1, "Recipe count is 1");
}

void testFoodProcessorDuplicateRecipe() {
    std::cout << "\n=== Food Processor: Duplicate Recipe ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    assertTrue(!sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f), "Duplicate recipe rejected");
    assertTrue(sys.getRecipeCount("processor1") == 1, "Still 1 recipe");
}

void testFoodProcessorStartCrafting() {
    std::cout << "\n=== Food Processor: Start Crafting ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    assertTrue(sys.startCrafting("processor1", "bread", "player1"), "Crafting started");
    assertTrue(sys.getActiveJobCount("processor1") == 1, "1 active job");
    assertTrue(!sys.startCrafting("processor1", "unknown_recipe", "player1"), "Unknown recipe fails");
}

void testFoodProcessorMaxJobs() {
    std::cout << "\n=== Food Processor: Max Jobs ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    auto* fp = addComp<components::FoodProcessor>(e);
    fp->max_concurrent_jobs = 1;

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    sys.startCrafting("processor1", "bread", "player1");
    assertTrue(!sys.startCrafting("processor1", "bread", "player2"), "Max jobs reached");
    assertTrue(sys.getActiveJobCount("processor1") == 1, "Still 1 active job");
}

void testFoodProcessorCraftCompletion() {
    std::cout << "\n=== Food Processor: Craft Completion ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    sys.startCrafting("processor1", "bread", "player1");
    assertTrue(!sys.isJobComplete("processor1", "player1"), "Job not complete yet");
    sys.update(11.0f); // past craft_time
    assertTrue(sys.isJobComplete("processor1", "player1"), "Job complete after update");
    assertTrue(sys.getActiveJobCount("processor1") == 0, "No active jobs after completion");
}

void testFoodProcessorCancel() {
    std::cout << "\n=== Food Processor: Cancel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    sys.startCrafting("processor1", "bread", "player1");
    assertTrue(sys.cancelCrafting("processor1", "player1"), "Crafting cancelled");
    assertTrue(sys.getActiveJobCount("processor1") == 0, "No active jobs after cancel");
    assertTrue(!sys.cancelCrafting("processor1", "player1"), "Cancel on empty fails");
}

void testFoodProcessorPowered() {
    std::cout << "\n=== Food Processor: Powered ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    assertTrue(sys.isPowered("processor1"), "Initially powered");
    sys.setPowered("processor1", false);
    assertTrue(!sys.isPowered("processor1"), "Unpowered after toggle");

    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    assertTrue(!sys.startCrafting("processor1", "bread", "player1"), "Cannot craft when unpowered");
}

void testFoodProcessorEfficiency() {
    std::cout << "\n=== Food Processor: Efficiency ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    addComp<components::FoodProcessor>(e);

    systems::FoodProcessorSystem sys(&world);
    sys.setEfficiency("processor1", 2.0f);
    assertTrue(approxEqual(sys.getEfficiency("processor1"), 2.0f), "Efficiency set to 2.0");

    std::vector<std::pair<std::string, int>> ingredients = {{"flour", 2}};
    sys.addRecipe("processor1", "bread", "bread_loaf", 1, ingredients, 10.0f, 30.0f);
    sys.startCrafting("processor1", "bread", "player1");
    sys.update(6.0f); // 6.0 * 2.0 = 12.0 effective, past 10.0
    assertTrue(sys.isJobComplete("processor1", "player1"), "Job completes faster with efficiency 2.0");
}

void testFoodProcessorMissingEntity() {
    std::cout << "\n=== Food Processor: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FoodProcessorSystem sys(&world);
    assertTrue(sys.getRecipeCount("nonexistent") == 0, "Default recipe count for missing");
    assertTrue(sys.getActiveJobCount("nonexistent") == 0, "Default job count for missing");
    assertTrue(!sys.isPowered("nonexistent"), "Default powered for missing");
    assertTrue(approxEqual(sys.getEfficiency("nonexistent"), 0.0f), "Default efficiency for missing");
}

void testFoodProcessorComponentDefaults() {
    std::cout << "\n=== Food Processor: Component Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("processor1");
    auto* fp = addComp<components::FoodProcessor>(e);
    assertTrue(fp->max_concurrent_jobs == 1, "Default max_concurrent_jobs is 1");
    assertTrue(approxEqual(fp->efficiency, 1.0f), "Default efficiency is 1.0");
    assertTrue(fp->powered == true, "Default powered is true");
    assertTrue(fp->available_recipes.empty(), "Default recipes empty");
    assertTrue(fp->active_jobs.empty(), "Default jobs empty");
}

// ==================== ImperfectInformationSystem Tests ====================

void testIntelRecord() {
    std::cout << "\n=== Imperfect Information: Record Intel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    assertTrue(sys.recordIntel("observer1", "target1", 0.8f, 100.0f), "Intel recorded");
    assertTrue(sys.hasIntel("observer1", "target1"), "Has intel on target1");
    assertTrue(sys.getConfidence("observer1", "target1") > 0.0f, "Confidence > 0");
    assertTrue(sys.getIntelCount("observer1") == 1, "One intel entry");
    assertTrue(sys.getTotalScans("observer1") == 1, "One scan recorded");
}

void testIntelConfidenceDistance() {
    std::cout << "\n=== Imperfect Information: Confidence by Distance ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    sys.recordIntel("observer1", "close_target", 0.8f, 10.0f);
    sys.recordIntel("observer1", "far_target", 0.8f, 5000.0f);
    float close_conf = sys.getConfidence("observer1", "close_target");
    float far_conf = sys.getConfidence("observer1", "far_target");
    assertTrue(close_conf > far_conf, "Close target has higher confidence");
}

void testIntelDecay() {
    std::cout << "\n=== Imperfect Information: Intel Decay ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    auto* intel = addComp<components::EntityIntel>(e);
    intel->entries.push_back({"target1", 0.5f, 0.8f, 0.0f, 0.1f, 100.0f, false});

    systems::ImperfectInformationSystem sys(&world);
    float initial = sys.getConfidence("observer1", "target1");
    sys.update(2.0f);  // 2 seconds of decay at 0.1/s = 0.2 lost
    float after = sys.getConfidence("observer1", "target1");
    assertTrue(after < initial, "Confidence decayed over time");
    assertTrue(sys.getIntelAge("observer1", "target1") > 0.0f, "Intel age increased");
}

void testIntelGhost() {
    std::cout << "\n=== Imperfect Information: Sensor Ghost ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    auto* intel = addComp<components::EntityIntel>(e);
    intel->ghost_threshold = 0.1f;
    intel->entries.push_back({"target1", 0.15f, 0.8f, 0.0f, 0.1f, 100.0f, false});

    systems::ImperfectInformationSystem sys(&world);
    assertTrue(!sys.isGhost("observer1", "target1"), "Not ghost initially");
    sys.update(1.0f);  // 0.15 - 0.1 = 0.05, below 0.1 threshold
    assertTrue(sys.isGhost("observer1", "target1"), "Became ghost after decay");
}

void testIntelRefresh() {
    std::cout << "\n=== Imperfect Information: Refresh Intel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    auto* intel = addComp<components::EntityIntel>(e);
    intel->entries.push_back({"target1", 0.3f, 0.5f, 10.0f, 0.01f, 200.0f, true});

    systems::ImperfectInformationSystem sys(&world);
    assertTrue(sys.isGhost("observer1", "target1"), "Was ghost");
    sys.recordIntel("observer1", "target1", 0.9f, 50.0f);
    assertTrue(!sys.isGhost("observer1", "target1"), "No longer ghost after refresh");
    assertTrue(sys.getConfidence("observer1", "target1") > 0.3f, "Confidence improved");
}

void testIntelClear() {
    std::cout << "\n=== Imperfect Information: Clear Intel ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    sys.recordIntel("observer1", "target1", 0.8f, 100.0f);
    sys.recordIntel("observer1", "target2", 0.6f, 200.0f);
    assertTrue(sys.getIntelCount("observer1") == 2, "Two entries");
    assertTrue(sys.clearIntel("observer1", "target1"), "Cleared target1");
    assertTrue(!sys.hasIntel("observer1", "target1"), "target1 removed");
    assertTrue(sys.hasIntel("observer1", "target2"), "target2 still present");
}

void testIntelClearAll() {
    std::cout << "\n=== Imperfect Information: Clear All ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    sys.recordIntel("observer1", "target1", 0.8f, 100.0f);
    sys.recordIntel("observer1", "target2", 0.6f, 200.0f);
    assertTrue(sys.clearAllIntel("observer1"), "Cleared all");
    assertTrue(sys.getIntelCount("observer1") == 0, "No entries after clear all");
}

void testIntelSensorStrength() {
    std::cout << "\n=== Imperfect Information: Sensor Strength ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    addComp<components::EntityIntel>(e);

    systems::ImperfectInformationSystem sys(&world);
    assertTrue(approxEqual(sys.getSensorStrength("observer1"), 1.0f), "Default sensor strength 1.0");
    sys.setSensorStrength("observer1", 2.0f);
    assertTrue(approxEqual(sys.getSensorStrength("observer1"), 2.0f), "Sensor strength set to 2.0");

    sys.recordIntel("observer1", "target1", 0.5f, 10.0f);
    float strong_conf = sys.getConfidence("observer1", "target1");
    assertTrue(strong_conf > 0.5f, "Higher sensor strength gives better confidence");
}

void testIntelZeroDecayRemoval() {
    std::cout << "\n=== Imperfect Information: Zero Decay Removal ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("observer1");
    auto* intel = addComp<components::EntityIntel>(e);
    intel->entries.push_back({"target1", 0.05f, 0.5f, 0.0f, 0.1f, 100.0f, false});

    systems::ImperfectInformationSystem sys(&world);
    sys.update(1.0f);  // 0.05 - 0.1 = 0, removed
    assertTrue(!sys.hasIntel("observer1", "target1"), "Removed entry at zero confidence");
}

void testIntelMissing() {
    std::cout << "\n=== Imperfect Information: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::ImperfectInformationSystem sys(&world);
    assertTrue(approxEqual(sys.getConfidence("nonexistent", "t"), 0.0f), "Default confidence 0");
    assertTrue(!sys.hasIntel("nonexistent", "t"), "No intel on missing");
    assertTrue(approxEqual(sys.getIntelAge("nonexistent", "t"), 0.0f), "Default age 0");
    assertTrue(!sys.isGhost("nonexistent", "t"), "Not ghost for missing");
    assertTrue(sys.getIntelCount("nonexistent") == 0, "Default count 0");
    assertTrue(sys.getTotalScans("nonexistent") == 0, "Default scans 0");
    assertTrue(approxEqual(sys.getSensorStrength("nonexistent"), 0.0f), "Default sensor 0");
}

// ==================== Solar Panel System Tests ====================

void testSolarPanelInit() {
    std::cout << "\n=== Solar Panel: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    assertTrue(sys.initializePanels("base1", "owner_001", 4), "Panels initialized");
    assertTrue(sys.getPanelCount("base1") == 4, "4 panels");
    assertTrue(!sys.isDeployed("base1"), "Not deployed initially");
    assertTrue(!sys.initializePanels("base1", "owner_002", 2), "Duplicate init rejected");
}

void testSolarPanelDeploy() {
    std::cout << "\n=== Solar Panel: Deploy ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);

    assertTrue(sys.deployPanels("base1"), "Deployed");
    assertTrue(sys.isDeployed("base1"), "Is deployed");
}

void testSolarPanelRetract() {
    std::cout << "\n=== Solar Panel: Retract ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");

    assertTrue(sys.retractPanels("base1"), "Retracted");
    assertTrue(!sys.isDeployed("base1"), "Not deployed after retract");
}

void testSolarPanelAddRemove() {
    std::cout << "\n=== Solar Panel: Add/Remove ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);

    assertTrue(sys.addPanel("base1"), "Panel added");
    assertTrue(sys.getPanelCount("base1") == 5, "5 panels");
    assertTrue(sys.removePanel("base1"), "Panel removed");
    assertTrue(sys.getPanelCount("base1") == 4, "Back to 4 panels");

    // Fill to max
    for (int i = 0; i < 6; i++) sys.addPanel("base1");
    assertTrue(sys.getPanelCount("base1") == 10, "At max panels");
    assertTrue(!sys.addPanel("base1"), "Cannot exceed max");
}

void testSolarPanelEnergyOutput() {
    std::cout << "\n=== Solar Panel: Energy Output ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");
    sys.setDayCyclePosition("base1", 0.5f);  // noon

    sys.update(0.1f);
    assertTrue(sys.getEnergyOutput("base1") > 0.0f, "Energy output at noon > 0");
    assertTrue(sys.isDaytime("base1"), "Is daytime at noon");
}

void testSolarPanelNighttime() {
    std::cout << "\n=== Solar Panel: Nighttime ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");
    sys.setDayCyclePosition("base1", 0.0f);  // midnight

    sys.update(0.001f);  // tiny dt to avoid advancing cycle much
    assertTrue(approxEqual(sys.getEnergyOutput("base1"), 0.0f), "No energy at midnight");
}

void testSolarPanelDegradation() {
    std::cout << "\n=== Solar Panel: Degradation ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");
    sys.setDayCyclePosition("base1", 0.5f);

    float initial_eff = sys.getEfficiency("base1");
    for (int i = 0; i < 100; i++) {
        sys.setDayCyclePosition("base1", 0.5f);  // keep at noon
        sys.update(1.0f);
    }
    assertTrue(sys.getEfficiency("base1") < initial_eff, "Efficiency decreased over time");
}

void testSolarPanelMaintenance() {
    std::cout << "\n=== Solar Panel: Maintenance ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");

    // Degrade
    for (int i = 0; i < 50; i++) {
        sys.setDayCyclePosition("base1", 0.5f);
        sys.update(1.0f);
    }
    float degraded = sys.getEfficiency("base1");
    assertTrue(degraded < 1.0f, "Efficiency degraded");

    assertTrue(sys.performMaintenance("base1", 0.5f), "Maintenance performed");
    assertTrue(sys.getEfficiency("base1") > degraded, "Efficiency restored after maintenance");
    assertTrue(sys.getEfficiency("base1") <= 1.0f, "Efficiency clamped to 1.0");
}

void testSolarPanelEnergyStorage() {
    std::cout << "\n=== Solar Panel: Energy Storage ===" << std::endl;
    ecs::World world;
    world.createEntity("base1");

    systems::SolarPanelSystem sys(&world);
    sys.initializePanels("base1", "owner_001", 4);
    sys.deployPanels("base1");
    sys.setDayCyclePosition("base1", 0.5f);

    assertTrue(approxEqual(sys.getEnergyStored("base1"), 0.0f), "No stored energy initially");
    sys.update(1.0f);
    assertTrue(sys.getEnergyStored("base1") > 0.0f, "Energy stored after update");
}

void testSolarPanelMissing() {
    std::cout << "\n=== Solar Panel: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SolarPanelSystem sys(&world);
    assertTrue(!sys.initializePanels("nonexistent", "owner", 4), "Init fails on missing");
    assertTrue(!sys.deployPanels("nonexistent"), "Deploy fails on missing");
    assertTrue(sys.getPanelCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(approxEqual(sys.getEnergyOutput("nonexistent"), 0.0f), "Output 0 on missing");
    assertTrue(!sys.isDeployed("nonexistent"), "Not deployed on missing");
}

// ==================== Farming Deck System Tests ====================

void testFarmingDeckInit() {
    std::cout << "\n=== Farming Deck: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    assertTrue(sys.initializeDeck("ship1", "owner_001", 6), "Deck initialized");
    assertTrue(sys.getPlotCount("ship1") == 0, "No plots initially");
    assertTrue(!sys.initializeDeck("ship1", "owner_002", 4), "Duplicate init rejected");
}

void testFarmingDeckPlant() {
    std::cout << "\n=== Farming Deck: Plant ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);

    assertTrue(sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain), "Crop planted");
    assertTrue(sys.getPlotCount("ship1") == 1, "1 plot");
    assertTrue(sys.getGrowthStage("ship1", "plot_a") == "planted", "Stage is planted");
    assertTrue(approxEqual(sys.getGrowthProgress("ship1", "plot_a"), 0.0f), "Progress at 0");
}

void testFarmingDeckGrowth() {
    std::cout << "\n=== Farming Deck: Growth ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    // Update enough to advance growth
    for (int i = 0; i < 50; i++) {
        sys.waterPlot("ship1", "plot_a", 0.5f);
        sys.fertilizePlot("ship1", "plot_a", 0.5f);
        sys.update(0.5f);
    }
    assertTrue(sys.getGrowthProgress("ship1", "plot_a") > 0.0f, "Growth progressed");
    std::string stage = sys.getGrowthStage("ship1", "plot_a");
    assertTrue(stage != "planted", "Stage advanced beyond planted");
}

void testFarmingDeckHarvest() {
    std::cout << "\n=== Farming Deck: Harvest ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    // Grow to harvestable
    for (int i = 0; i < 200; i++) {
        sys.waterPlot("ship1", "plot_a", 1.0f);
        sys.fertilizePlot("ship1", "plot_a", 1.0f);
        sys.update(0.1f);
    }
    assertTrue(sys.getGrowthStage("ship1", "plot_a") == "harvestable", "Crop is harvestable");

    float yield = sys.harvestCrop("ship1", "plot_a");
    assertTrue(yield > 0.0f, "Harvest yields food");
    assertTrue(sys.getGrowthStage("ship1", "plot_a") == "empty", "Plot reset to empty");
    assertTrue(sys.getTotalFoodProduced("ship1") > 0.0f, "Total food increased");
}

void testFarmingDeckHarvestNotReady() {
    std::cout << "\n=== Farming Deck: Harvest Not Ready ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    sys.update(0.1f);  // barely any growth
    float yield = sys.harvestCrop("ship1", "plot_a");
    assertTrue(approxEqual(yield, 0.0f), "Cannot harvest unready crop");
}

void testFarmingDeckWater() {
    std::cout << "\n=== Farming Deck: Water ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    // Water should consume first, then add
    sys.update(1.0f);  // consume some water
    assertTrue(sys.waterPlot("ship1", "plot_a", 0.5f), "Watered plot");
}

void testFarmingDeckFertilize() {
    std::cout << "\n=== Farming Deck: Fertilize ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    sys.update(1.0f);  // consume some nutrients
    assertTrue(sys.fertilizePlot("ship1", "plot_a", 0.5f), "Fertilized plot");
}

void testFarmingDeckWithering() {
    std::cout << "\n=== Farming Deck: Withering ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    // Drain water low so it withers on next update cycle
    // Water starts at 1.0, consumption is 0.02/s. Use many small updates without rewatering.
    // Set growth rate slow by using low light
    sys.setLightLevel("ship1", 0.01f);  // minimal light = very slow growth
    for (int i = 0; i < 200; i++) {
        sys.update(0.5f);
    }
    assertTrue(sys.getGrowthStage("ship1", "plot_a") == "withered", "Crop withered without water");
}

void testFarmingDeckPower() {
    std::cout << "\n=== Farming Deck: Power ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::FarmingDeckSystem sys(&world);
    sys.initializeDeck("ship1", "owner_001", 6);
    sys.plantCrop("ship1", "plot_a", components::FarmingDeck::CropType::Grain);

    assertTrue(sys.setPowerEnabled("ship1", false), "Power disabled");
    float progress_before = sys.getGrowthProgress("ship1", "plot_a");
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getGrowthProgress("ship1", "plot_a"), progress_before), "No growth without power");
}

void testFarmingDeckMissing() {
    std::cout << "\n=== Farming Deck: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FarmingDeckSystem sys(&world);
    assertTrue(!sys.initializeDeck("nonexistent", "owner", 6), "Init fails on missing");
    assertTrue(!sys.plantCrop("nonexistent", "plot", components::FarmingDeck::CropType::Grain), "Plant fails on missing");
    assertTrue(sys.getPlotCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(approxEqual(sys.getTotalFoodProduced("nonexistent"), 0.0f), "Food 0 on missing");
}

// ==================== Grid Construction System Tests ====================

void testGridInit() {
    std::cout << "\n=== Grid Construction: Init ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    assertTrue(sys.initializeGrid("test_grid", "player1", 10, 10), "Grid initialized");
    assertTrue(sys.getGridWidth("test_grid") == 10, "Width is 10");
    assertTrue(sys.getGridHeight("test_grid") == 10, "Height is 10");
    assertTrue(!sys.initializeGrid("test_grid", "player1", 8, 8), "Duplicate init fails");
}

void testGridPlace() {
    std::cout << "\n=== Grid Construction: Place ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    assertTrue(sys.placeModule("test_grid", 0, 0, components::GridConstruction::ModuleType::Foundation), "Place foundation");
    assertTrue(sys.getModuleAt("test_grid", 0, 0) == "foundation", "Module is foundation");
    assertTrue(sys.getModuleCount("test_grid") == 1, "Module count is 1");
    assertTrue(!sys.placeModule("test_grid", 0, 0, components::GridConstruction::ModuleType::Wall), "Cannot place on occupied");
}

void testGridRemove() {
    std::cout << "\n=== Grid Construction: Remove ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 2, 2, components::GridConstruction::ModuleType::Wall);
    assertTrue(sys.removeModule("test_grid", 2, 2), "Remove module");
    assertTrue(sys.getModuleAt("test_grid", 2, 2) == "empty", "Cell empty after remove");
    assertTrue(!sys.removeModule("test_grid", 2, 2), "Cannot remove empty cell");
}

void testGridAdjacency() {
    std::cout << "\n=== Grid Construction: Adjacency ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 1, 1, components::GridConstruction::ModuleType::Foundation);
    sys.placeModule("test_grid", 0, 1, components::GridConstruction::ModuleType::Wall);
    sys.placeModule("test_grid", 2, 1, components::GridConstruction::ModuleType::Wall);
    sys.placeModule("test_grid", 1, 0, components::GridConstruction::ModuleType::Floor);
    sys.placeModule("test_grid", 1, 2, components::GridConstruction::ModuleType::Floor);
    float integrity = sys.calculateIntegrity("test_grid");
    assertTrue(integrity > 0.0f, "Integrity > 0 with modules");
}

void testGridPower() {
    std::cout << "\n=== Grid Construction: Power ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 1, 1, components::GridConstruction::ModuleType::PowerNode);
    sys.placeModule("test_grid", 1, 0, components::GridConstruction::ModuleType::HabitatModule);
    float balance = sys.calculatePower("test_grid");
    assertTrue(approxEqual(balance, 8.0f), "Power balance 10 - 2 = 8");
    sys.update(1.0f);
    assertTrue(sys.getPoweredCount("test_grid") == 2, "2 cells powered");
}

void testGridIntegrity() {
    std::cout << "\n=== Grid Construction: Integrity ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    // Place a center module with 3+ neighbors for bonus
    sys.placeModule("test_grid", 3, 3, components::GridConstruction::ModuleType::Foundation);
    sys.placeModule("test_grid", 2, 3, components::GridConstruction::ModuleType::Wall);
    sys.placeModule("test_grid", 4, 3, components::GridConstruction::ModuleType::Wall);
    sys.placeModule("test_grid", 3, 2, components::GridConstruction::ModuleType::Floor);
    float integrity = sys.calculateIntegrity("test_grid");
    assertTrue(integrity > 0.0f, "Integrity calculated");
    assertTrue(integrity <= 1.0f, "Integrity within bounds");
}

void testGridDamage() {
    std::cout << "\n=== Grid Construction: Damage ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 0, 0, components::GridConstruction::ModuleType::Wall);
    assertTrue(approxEqual(sys.getModuleHealth("test_grid", 0, 0), 1.0f), "Initial health 1.0");
    assertTrue(sys.damageModule("test_grid", 0, 0, 0.3f), "Damage applied");
    assertTrue(approxEqual(sys.getModuleHealth("test_grid", 0, 0), 0.7f), "Health reduced to 0.7");
}

void testGridRepair() {
    std::cout << "\n=== Grid Construction: Repair ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 8, 8);
    sys.placeModule("test_grid", 0, 0, components::GridConstruction::ModuleType::Wall);
    sys.damageModule("test_grid", 0, 0, 0.5f);
    assertTrue(sys.repairModule("test_grid", 0, 0, 0.3f), "Repair applied");
    assertTrue(approxEqual(sys.getModuleHealth("test_grid", 0, 0), 0.8f), "Health restored to 0.8");
    sys.repairModule("test_grid", 0, 0, 1.0f);
    assertTrue(approxEqual(sys.getModuleHealth("test_grid", 0, 0), 1.0f), "Health capped at 1.0");
}

void testGridBounds() {
    std::cout << "\n=== Grid Construction: Bounds ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    world.createEntity("test_grid");
    sys.initializeGrid("test_grid", "player1", 4, 4);
    assertTrue(!sys.placeModule("test_grid", -1, 0, components::GridConstruction::ModuleType::Wall), "Negative x rejected");
    assertTrue(!sys.placeModule("test_grid", 0, -1, components::GridConstruction::ModuleType::Wall), "Negative y rejected");
    assertTrue(!sys.placeModule("test_grid", 4, 0, components::GridConstruction::ModuleType::Wall), "X out of bounds rejected");
    assertTrue(!sys.placeModule("test_grid", 0, 4, components::GridConstruction::ModuleType::Wall), "Y out of bounds rejected");
    assertTrue(sys.getModuleAt("test_grid", 10, 10) == "unknown", "Out of bounds query returns unknown");
}

void testGridMissing() {
    std::cout << "\n=== Grid Construction: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::GridConstructionSystem sys(&world);
    assertTrue(!sys.initializeGrid("nonexistent", "player1", 8, 8), "Init fails on missing");
    assertTrue(!sys.placeModule("nonexistent", 0, 0, components::GridConstruction::ModuleType::Wall), "Place fails on missing");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getGridWidth("nonexistent") == 0, "Width 0 on missing");
}

// =====================================================
// Space-Planet Transition System Tests (Phase 14)
// =====================================================

void testTransitionInit() {
    std::cout << "\n=== Space-Planet Transition: Init ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    assertTrue(sys.initializeTransition("ship_1", "planet_1", true), "Transition initialized");
    assertTrue(sys.getTransitionState("ship_1") == "in_space", "Starts in space");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 1000.0f), "Altitude starts at 1000");
    assertTrue(!sys.initializeTransition("ship_1", "planet_1", true), "Duplicate init fails");
}

void testTransitionBeginDescent() {
    std::cout << "\n=== Space-Planet Transition: Begin Descent ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", true);
    assertTrue(sys.beginDescent("ship_1"), "Descent started");
    assertTrue(sys.getTransitionState("ship_1") == "orbit_entry", "State is orbit_entry");
    assertTrue(!sys.beginDescent("ship_1"), "Cannot start descent again while in transition");
}

void testTransitionDescentSequence() {
    std::cout << "\n=== Space-Planet Transition: Descent Sequence ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", false);
    sys.beginDescent("ship_1");
    // Advance through OrbitEntry (10s), AtmosphereEntry (10s), DescentPhase (10s), LandingApproach (10s)
    for (int i = 0; i < 40; i++) sys.update(1.0f);
    assertTrue(sys.getTransitionState("ship_1") == "landed", "Reached landed state");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 0.0f), "Altitude is 0 when landed");
}

void testTransitionBeginLaunch() {
    std::cout << "\n=== Space-Planet Transition: Begin Launch ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", false);
    sys.beginDescent("ship_1");
    for (int i = 0; i < 40; i++) sys.update(1.0f);
    assertTrue(sys.getTransitionState("ship_1") == "landed", "Is landed");
    assertTrue(sys.beginLaunch("ship_1"), "Launch started");
    assertTrue(sys.getTransitionState("ship_1") == "launch_sequence", "State is launch_sequence");
}

void testTransitionLaunchSequence() {
    std::cout << "\n=== Space-Planet Transition: Launch Sequence ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", false);
    sys.beginDescent("ship_1");
    for (int i = 0; i < 40; i++) sys.update(1.0f);
    sys.beginLaunch("ship_1");
    // Advance through LaunchSequence (10s), AtmosphereExit (10s), OrbitExit (10s)
    for (int i = 0; i < 30; i++) sys.update(1.0f);
    assertTrue(sys.getTransitionState("ship_1") == "in_space", "Returned to space");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 1000.0f), "Altitude reset to 1000");
}

void testTransitionAbort() {
    std::cout << "\n=== Space-Planet Transition: Abort ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", true);
    sys.beginDescent("ship_1");
    sys.update(3.0f);  // Partially into orbit entry
    assertTrue(sys.abortTransition("ship_1"), "Abort succeeded");
    assertTrue(sys.getTransitionState("ship_1") == "in_space", "Returned to space");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 1000.0f), "Altitude reset");
}

void testTransitionAbortInvalid() {
    std::cout << "\n=== Space-Planet Transition: Abort Invalid ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", false);
    assertTrue(!sys.abortTransition("ship_1"), "Cannot abort when InSpace");
    sys.beginDescent("ship_1");
    for (int i = 0; i < 40; i++) sys.update(1.0f);
    assertTrue(!sys.abortTransition("ship_1"), "Cannot abort when Landed");
}

void testTransitionAtmosphereHeat() {
    std::cout << "\n=== Space-Planet Transition: Atmosphere Heat ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", true);
    assertTrue(approxEqual(sys.getHeatLevel("ship_1"), 0.0f), "Initial heat is 0");
    sys.beginDescent("ship_1");
    // Advance through OrbitEntry (10s) into AtmosphereEntry
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    // Now in AtmosphereEntry, heat should build
    sys.update(5.0f);
    assertTrue(sys.getHeatLevel("ship_1") > 0.0f, "Heat built up during atmosphere entry");
}

void testTransitionAutopilot() {
    std::cout << "\n=== Space-Planet Transition: Autopilot ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_1", true);
    assertTrue(sys.setAutopilot("ship_1", true), "Autopilot enabled");
    assertTrue(sys.setLandingTarget("ship_1", 100.0f, 200.0f), "Landing target set");
}

void testTransitionMissing() {
    std::cout << "\n=== Space-Planet Transition: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);
    assertTrue(!sys.initializeTransition("nonexistent", "planet_1", true), "Init fails on missing");
    assertTrue(!sys.beginDescent("nonexistent"), "Descent fails on missing");
    assertTrue(approxEqual(sys.getAltitude("nonexistent"), 0.0f), "Altitude 0 on missing");
    assertTrue(sys.getTransitionState("nonexistent") == "unknown", "State unknown on missing");
}

// =====================================================
// Fleet Hangar System Tests (Phase 14)
// =====================================================

void testFleetHangarInit() {
    std::cout << "\n=== Fleet Hangar: Init ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    assertTrue(sys.initializeHangar("station_1", "player1", "Main Hangar", 1), "Hangar initialized");
    assertTrue(sys.getTier("station_1") == 1, "Tier is 1");
    assertTrue(sys.getMaxSlots("station_1") == 5, "Max slots is 5 for tier 1");
    assertTrue(sys.getShipCount("station_1") == 0, "No ships docked");
    assertTrue(!sys.initializeHangar("station_1", "player1", "Dup", 1), "Duplicate init fails");
}

void testFleetHangarDock() {
    std::cout << "\n=== Fleet Hangar: Dock ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    assertTrue(sys.dockShip("station_1", "ship_1", "frigate", 100.0f), "Ship docked");
    assertTrue(sys.getShipCount("station_1") == 1, "Ship count is 1");
    assertTrue(!sys.dockShip("station_1", "ship_1", "frigate", 100.0f), "Duplicate dock fails");
}

void testFleetHangarUndock() {
    std::cout << "\n=== Fleet Hangar: Undock ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    sys.dockShip("station_1", "ship_1", "frigate", 100.0f);
    assertTrue(sys.undockShip("station_1", "ship_1"), "Ship undocked");
    assertTrue(sys.getShipCount("station_1") == 0, "Ship count is 0");
    assertTrue(!sys.undockShip("station_1", "ship_1"), "Undock of absent ship fails");
}

void testFleetHangarFull() {
    std::cout << "\n=== Fleet Hangar: Full ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    for (int i = 0; i < 5; i++) {
        sys.dockShip("station_1", "ship_" + std::to_string(i), "frigate", 100.0f);
    }
    assertTrue(sys.getShipCount("station_1") == 5, "Hangar full with 5 ships");
    assertTrue(!sys.dockShip("station_1", "ship_extra", "cruiser", 100.0f), "Cannot dock when full");
}

void testFleetHangarLock() {
    std::cout << "\n=== Fleet Hangar: Lock ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    sys.dockShip("station_1", "ship_1", "frigate", 100.0f);
    assertTrue(sys.lockShip("station_1", "ship_1"), "Ship locked");
    assertTrue(!sys.undockShip("station_1", "ship_1"), "Cannot undock locked ship");
    assertTrue(sys.unlockShip("station_1", "ship_1"), "Ship unlocked");
    assertTrue(sys.undockShip("station_1", "ship_1"), "Undock after unlock");
}

void testFleetHangarUpgrade() {
    std::cout << "\n=== Fleet Hangar: Upgrade ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    assertTrue(sys.upgradeHangar("station_1"), "Upgrade to tier 2");
    assertTrue(sys.getTier("station_1") == 2, "Tier is 2");
    assertTrue(sys.getMaxSlots("station_1") == 10, "Max slots is 10 for tier 2");
}

void testFleetHangarMaxTier() {
    std::cout << "\n=== Fleet Hangar: Max Tier ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 5);
    assertTrue(sys.getTier("station_1") == 5, "Tier is 5");
    assertTrue(sys.getMaxSlots("station_1") == 50, "Max slots is 50 for tier 5");
    assertTrue(!sys.upgradeHangar("station_1"), "Cannot upgrade past tier 5");
}

void testFleetHangarRepair() {
    std::cout << "\n=== Fleet Hangar: Repair ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    sys.dockShip("station_1", "ship_1", "frigate", 50.0f);
    assertTrue(sys.repairShip("station_1", "ship_1", 30.0f), "Repair applied");
    // Verify ship count still correct
    assertTrue(sys.getShipCount("station_1") == 1, "Ship still docked after repair");
}

void testFleetHangarPower() {
    std::cout << "\n=== Fleet Hangar: Power ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    world.createEntity("station_1");
    sys.initializeHangar("station_1", "player1", "Hangar", 1);
    sys.setPowerEnabled("station_1", false);
    assertTrue(!sys.dockShip("station_1", "ship_1", "frigate", 100.0f), "Cannot dock without power");
    sys.setPowerEnabled("station_1", true);
    assertTrue(sys.dockShip("station_1", "ship_1", "frigate", 100.0f), "Dock with power");
    sys.setPowerEnabled("station_1", false);
    assertTrue(!sys.undockShip("station_1", "ship_1"), "Cannot undock without power");
}

void testFleetHangarMissing() {
    std::cout << "\n=== Fleet Hangar: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetHangarSystem sys(&world);
    assertTrue(!sys.initializeHangar("nonexistent", "p", "H", 1), "Init fails on missing");
    assertTrue(!sys.dockShip("nonexistent", "s", "c", 100.0f), "Dock fails on missing");
    assertTrue(sys.getShipCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getTier("nonexistent") == 0, "Tier 0 on missing");
}

// =====================================================
// Hangar Environment System Tests (Phase 14)
// =====================================================

void testHangarEnvInit() {
    std::cout << "\n=== Hangar Environment: Init ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    assertTrue(sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f), "Env initialized");
    assertTrue(approxEqual(sys.getToxicity("hangar_1"), 0.0f), "Initial toxicity is 0");
    assertTrue(!sys.isAlarmActive("hangar_1"), "Alarm not active initially");
    assertTrue(!sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f), "Duplicate init fails");
}

void testHangarEnvOpenClose() {
    std::cout << "\n=== Hangar Environment: Open/Close ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Breathable, 22.0f, 1.0f);
    assertTrue(sys.openHangar("hangar_1"), "Hangar opened");
    assertTrue(sys.closeHangar("hangar_1"), "Hangar closed");
}

void testHangarEnvToxicExposure() {
    std::cout << "\n=== Hangar Environment: Toxic Exposure ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -30.0f, 0.3f);
    sys.openHangar("hangar_1");
    // Update several ticks - toxicity should increase
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    assertTrue(sys.getToxicity("hangar_1") > 0.0f, "Toxicity increased when open in toxic");
}

void testHangarEnvCorrosiveExposure() {
    std::cout << "\n=== Hangar Environment: Corrosive Exposure ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Corrosive, 100.0f, 2.0f);
    sys.openHangar("hangar_1");
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    // Check corrosion by getting occupant damage (need to add occupant)
    sys.addOccupant("hangar_1", "player_1", false, 0.0f);
    float damage = sys.getOccupantDamage("hangar_1", "player_1");
    assertTrue(damage > 0.0f, "Damage > 0 in corrosive atmosphere");
}

void testHangarEnvUnsuitedDamage() {
    std::cout << "\n=== Hangar Environment: Unsuited Damage ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f);
    sys.openHangar("hangar_1");
    sys.addOccupant("hangar_1", "player_1", false, 0.0f);
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    float damage = sys.getOccupantDamage("hangar_1", "player_1");
    assertTrue(damage > 0.0f, "Unsuited occupant takes damage");
}

void testHangarEnvSuitedProtection() {
    std::cout << "\n=== Hangar Environment: Suited Protection ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f);
    sys.openHangar("hangar_1");
    sys.addOccupant("hangar_1", "unsuited", false, 0.0f);
    sys.addOccupant("hangar_1", "suited", true, 0.8f);
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    float unsuited_dmg = sys.getOccupantDamage("hangar_1", "unsuited");
    float suited_dmg = sys.getOccupantDamage("hangar_1", "suited");
    assertTrue(suited_dmg < unsuited_dmg, "Suited occupant takes less damage");
}

void testHangarEnvRecovery() {
    std::cout << "\n=== Hangar Environment: Recovery ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f);
    sys.openHangar("hangar_1");
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    float tox_open = sys.getToxicity("hangar_1");
    sys.closeHangar("hangar_1");
    for (int i = 0; i < 20; i++) sys.update(1.0f);
    float tox_closed = sys.getToxicity("hangar_1");
    assertTrue(tox_closed < tox_open, "Toxicity decreased after closing");
}

void testHangarEnvAlarm() {
    std::cout << "\n=== Hangar Environment: Alarm ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::Toxic, -50.0f, 0.5f);
    assertTrue(!sys.isAlarmActive("hangar_1"), "No alarm initially");
    sys.openHangar("hangar_1");
    // Run enough ticks for toxicity > 0.3 (mix rate 0.1/s, need > 3 seconds)
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    assertTrue(sys.isAlarmActive("hangar_1"), "Alarm active when toxicity > 0.3");
}

void testHangarEnvVacuum() {
    std::cout << "\n=== Hangar Environment: Vacuum ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    world.createEntity("hangar_1");
    sys.initializeEnvironment("hangar_1",
        components::HangarEnvironment::AtmosphereType::None, -270.0f, 0.0f);
    sys.openHangar("hangar_1");
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    // In vacuum, internal pressure should drop
    auto* entity = world.getEntity("hangar_1");
    auto* env = entity->getComponent<components::HangarEnvironment>();
    assertTrue(env->internal_pressure < 1.0f, "Pressure dropped in vacuum");
}

void testHangarEnvMissing() {
    std::cout << "\n=== Hangar Environment: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::HangarEnvironmentSystem sys(&world);
    assertTrue(!sys.initializeEnvironment("nonexistent",
        components::HangarEnvironment::AtmosphereType::Toxic, 0.0f, 0.0f), "Init fails on missing");
    assertTrue(!sys.openHangar("nonexistent"), "Open fails on missing");
    assertTrue(approxEqual(sys.getToxicity("nonexistent"), 0.0f), "Toxicity 0 on missing");
    assertTrue(sys.getOccupantCount("nonexistent") == 0, "Count 0 on missing");
}

// ==================== PvPToggleSystem Tests ====================

void testPvPCreate() {
    std::cout << "\n=== PvPToggle: Create ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.createPvPState("player1"), "Create PvP state succeeds");
    assertTrue(!sys.isPvPEnabled("player1"), "PvP disabled by default");
    assertTrue(sys.getSafetyLevel("player1") == "HighSec", "Default safety is HighSec");
    assertTrue(approxEqual(sys.getSecurityStatus("player1"), 5.0f), "Default security status 5.0");
}

void testPvPEnable() {
    std::cout << "\n=== PvPToggle: Enable ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");
    assertTrue(!sys.enablePvP("player1"), "Cannot enable PvP in HighSec");
    sys.setSafetyLevel("player1", "LowSec");
    assertTrue(sys.enablePvP("player1"), "Enable PvP in LowSec succeeds");
    assertTrue(sys.isPvPEnabled("player1"), "PvP is now enabled");
}

void testPvPDisable() {
    std::cout << "\n=== PvPToggle: Disable ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");
    sys.setSafetyLevel("player1", "LowSec");
    sys.enablePvP("player1");
    assertTrue(sys.disablePvP("player1"), "Disable PvP in LowSec succeeds");
    assertTrue(!sys.isPvPEnabled("player1"), "PvP is now disabled");
}

void testPvPSafetyLevel() {
    std::cout << "\n=== PvPToggle: SafetyLevel ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");
    assertTrue(sys.setSafetyLevel("player1", "LowSec"), "Set LowSec succeeds");
    assertTrue(sys.getSafetyLevel("player1") == "LowSec", "Safety is LowSec");
    assertTrue(sys.setSafetyLevel("player1", "NullSec"), "Set NullSec succeeds");
    assertTrue(sys.isPvPEnabled("player1"), "NullSec forces PvP enabled");
    assertTrue(!sys.disablePvP("player1"), "Cannot disable PvP in NullSec");
    assertTrue(sys.setSafetyLevel("player1", "Wormhole"), "Set Wormhole succeeds");
    assertTrue(sys.isPvPEnabled("player1"), "Wormhole forces PvP enabled");
    assertTrue(!sys.setSafetyLevel("player1", "InvalidZone"), "Invalid zone rejected");
}

void testPvPCanEngage() {
    std::cout << "\n=== PvPToggle: CanEngage ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("attacker");
    world.createEntity("defender");
    sys.createPvPState("attacker");
    sys.createPvPState("defender");
    assertTrue(!sys.canEngage("attacker", "defender"), "Cannot engage in HighSec");
    sys.setSafetyLevel("attacker", "LowSec");
    sys.setSafetyLevel("defender", "LowSec");
    sys.enablePvP("attacker");
    assertTrue(!sys.canEngage("attacker", "defender"), "Cannot engage if defender unflagged");
    sys.enablePvP("defender");
    assertTrue(sys.canEngage("attacker", "defender"), "Both flagged in LowSec can engage");
}

void testPvPNullSecEngage() {
    std::cout << "\n=== PvPToggle: NullSecEngage ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("attacker");
    world.createEntity("defender");
    sys.createPvPState("attacker");
    sys.createPvPState("defender");
    sys.setSafetyLevel("attacker", "NullSec");
    sys.setSafetyLevel("defender", "NullSec");
    assertTrue(sys.canEngage("attacker", "defender"), "NullSec: always engageable");
}

void testPvPRecordEngagement() {
    std::cout << "\n=== PvPToggle: RecordEngagement ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("a");
    world.createEntity("d");
    sys.createPvPState("a");
    sys.createPvPState("d");
    assertTrue(sys.recordEngagement("a", "d"), "Record engagement succeeds");
    assertTrue(approxEqual(sys.getAggressionTimer("a"), 300.0f), "Attacker aggression timer set");
    assertTrue(approxEqual(sys.getAggressionTimer("d"), 300.0f), "Defender aggression timer set");
}

void testPvPAggressionCooldown() {
    std::cout << "\n=== PvPToggle: AggressionCooldown ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("player1");
    sys.createPvPState("player1");
    sys.setSafetyLevel("player1", "LowSec");
    sys.enablePvP("player1");
    world.createEntity("target");
    sys.createPvPState("target");
    sys.recordEngagement("player1", "target");
    assertTrue(!sys.disablePvP("player1"), "Cannot disable PvP with active aggression");
    sys.update(200.0f);
    assertTrue(approxEqual(sys.getAggressionTimer("player1"), 100.0f), "Timer decreased by 200s");
    sys.update(100.0f);
    assertTrue(approxEqual(sys.getAggressionTimer("player1"), 0.0f), "Timer reached 0");
    assertTrue(sys.disablePvP("player1"), "Can disable PvP after timer expires");
}

void testPvPRecordKill() {
    std::cout << "\n=== PvPToggle: RecordKill ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    world.createEntity("killer");
    sys.createPvPState("killer");
    assertTrue(sys.recordKill("killer"), "Record kill succeeds");
    assertTrue(sys.getKillCount("killer") == 1, "Kill count is 1");
    assertTrue(approxEqual(sys.getBounty("killer"), 1000.0f), "Bounty is 1000");
    assertTrue(approxEqual(sys.getSecurityStatus("killer"), 4.5f), "Security status decreased to 4.5");
    sys.recordKill("killer");
    sys.recordKill("killer");
    assertTrue(sys.getKillCount("killer") == 3, "Kill count is 3");
    assertTrue(approxEqual(sys.getBounty("killer"), 3000.0f), "Bounty is 3000");
    assertTrue(approxEqual(sys.getSecurityStatus("killer"), 3.5f), "Security status decreased to 3.5");
}

void testPvPMissing() {
    std::cout << "\n=== PvPToggle: Missing ===" << std::endl;
    ecs::World world;
    systems::PvPToggleSystem sys(&world);
    assertTrue(!sys.createPvPState("nonexistent"), "Create fails on missing entity");
    assertTrue(!sys.enablePvP("nonexistent"), "Enable fails on missing");
    assertTrue(!sys.disablePvP("nonexistent"), "Disable fails on missing");
    assertTrue(!sys.isPvPEnabled("nonexistent"), "isPvPEnabled false on missing");
    assertTrue(sys.getSafetyLevel("nonexistent") == "", "Empty safety on missing");
    assertTrue(sys.getKillCount("nonexistent") == 0, "0 kills on missing");
    assertTrue(approxEqual(sys.getSecurityStatus("nonexistent"), 0.0f), "0 security on missing");
    assertTrue(approxEqual(sys.getBounty("nonexistent"), 0.0f), "0 bounty on missing");
    assertTrue(approxEqual(sys.getAggressionTimer("nonexistent"), 0.0f), "0 timer on missing");
    assertTrue(!sys.recordKill("nonexistent"), "Record kill fails on missing");
}

// ==================== DynamicEventSystem Tests ====================

void testDynamicEventCreate() {
    std::cout << "\n=== DynamicEvent: Create ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    assertTrue(sys.createEventManager("world1"), "Create event manager succeeds");
    assertTrue(sys.getActiveEventCount("world1") == 0, "Initial active events is 0");
    assertTrue(sys.getTotalCompleted("world1") == 0, "Initial total completed is 0");
}

void testDynamicEventSchedule() {
    std::cout << "\n=== DynamicEvent: Schedule ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    assertTrue(sys.scheduleEvent("world1", "ev1", "PirateInvasion", 100.0f, 0.8f), "Schedule succeeds");
    assertTrue(sys.getEventState("world1", "ev1") == "Pending", "New event is Pending");
    assertTrue(sys.getEventType("world1", "ev1") == "PirateInvasion", "Event type is PirateInvasion");
    assertTrue(approxEqual(sys.getIntensity("world1", "ev1"), 0.8f), "Intensity is 0.8");
}

void testDynamicEventStart() {
    std::cout << "\n=== DynamicEvent: Start ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "ResourceSurge", 60.0f, 0.5f);
    assertTrue(sys.startEvent("world1", "ev1"), "Manual start succeeds");
    assertTrue(sys.getEventState("world1", "ev1") == "Active", "Event is Active after start");
    assertTrue(sys.getActiveEventCount("world1") == 1, "Active event count is 1");
}

void testDynamicEventJoinLeave() {
    std::cout << "\n=== DynamicEvent: JoinLeave ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "AnomalyStorm", 120.0f, 0.6f);
    sys.startEvent("world1", "ev1");
    assertTrue(sys.joinEvent("world1", "ev1", "player1"), "Join event succeeds");
    assertTrue(sys.getParticipantCount("world1", "ev1") == 1, "1 participant");
    assertTrue(sys.joinEvent("world1", "ev1", "player2"), "Second join succeeds");
    assertTrue(sys.getParticipantCount("world1", "ev1") == 2, "2 participants");
    assertTrue(!sys.joinEvent("world1", "ev1", "player1"), "Duplicate join rejected");
    assertTrue(sys.leaveEvent("world1", "ev1", "player1"), "Leave succeeds");
    assertTrue(sys.getParticipantCount("world1", "ev1") == 1, "1 participant after leave");
}

void testDynamicEventLifecycle() {
    std::cout << "\n=== DynamicEvent: Lifecycle ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "TradeBonus", 100.0f, 0.5f);
    sys.startEvent("world1", "ev1");
    sys.update(79.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Active", "Still Active at 79%");
    sys.update(1.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Concluding", "Concluding at 80%");
    sys.update(20.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Completed", "Completed at 100%");
    assertTrue(sys.getTotalCompleted("world1") == 1, "Total completed is 1");
}

void testDynamicEventRewardPool() {
    std::cout << "\n=== DynamicEvent: RewardPool ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "SecurityAlert", 100.0f, 1.0f);
    sys.startEvent("world1", "ev1");
    assertTrue(approxEqual(sys.getRewardPool("world1", "ev1"), 0.0f), "Initial reward pool is 0");
    sys.update(10.0f);
    assertTrue(sys.getRewardPool("world1", "ev1") > 0.0f, "Reward pool accumulates");
    float pool = sys.getRewardPool("world1", "ev1");
    assertTrue(approxEqual(pool, 1000.0f), "10s at intensity 1.0 yields 1000 reward");
}

void testDynamicEventAutoStart() {
    std::cout << "\n=== DynamicEvent: AutoStart ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "FactionConflict", 50.0f, 0.7f);
    assertTrue(sys.getEventState("world1", "ev1") == "Pending", "Starts as Pending");
    sys.update(4.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Pending", "Still Pending at 4s");
    sys.update(2.0f);
    assertTrue(sys.getEventState("world1", "ev1") == "Active", "Auto-starts after delay");
}

void testDynamicEventCancel() {
    std::cout << "\n=== DynamicEvent: Cancel ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    sys.scheduleEvent("world1", "ev1", "PirateInvasion", 100.0f, 0.5f);
    sys.startEvent("world1", "ev1");
    assertTrue(sys.cancelEvent("world1", "ev1"), "Cancel active event succeeds");
    assertTrue(sys.getEventState("world1", "ev1") == "Completed", "Cancelled event is Completed");
    assertTrue(!sys.cancelEvent("world1", "ev1"), "Cannot cancel already Completed event");
}

void testDynamicEventMaxConcurrent() {
    std::cout << "\n=== DynamicEvent: MaxConcurrent ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    world.createEntity("world1");
    sys.createEventManager("world1");
    assertTrue(sys.scheduleEvent("world1", "e1", "PirateInvasion", 100.0f, 0.5f), "Schedule 1 succeeds");
    assertTrue(sys.scheduleEvent("world1", "e2", "ResourceSurge", 100.0f, 0.5f), "Schedule 2 succeeds");
    assertTrue(sys.scheduleEvent("world1", "e3", "AnomalyStorm", 100.0f, 0.5f), "Schedule 3 succeeds");
    assertTrue(sys.scheduleEvent("world1", "e4", "TradeBonus", 100.0f, 0.5f), "Schedule 4 succeeds");
    assertTrue(sys.scheduleEvent("world1", "e5", "SecurityAlert", 100.0f, 0.5f), "Schedule 5 succeeds");
    assertTrue(!sys.scheduleEvent("world1", "e6", "FactionConflict", 100.0f, 0.5f), "Schedule 6 rejected (max 5)");
}

void testDynamicEventMissing() {
    std::cout << "\n=== DynamicEvent: Missing ===" << std::endl;
    ecs::World world;
    systems::DynamicEventSystem sys(&world);
    assertTrue(!sys.createEventManager("nonexistent"), "Create fails on missing entity");
    assertTrue(!sys.scheduleEvent("nonexistent", "x", "PirateInvasion", 100.0f, 0.5f), "Schedule fails on missing");
    assertTrue(!sys.startEvent("nonexistent", "x"), "Start fails on missing");
    assertTrue(!sys.joinEvent("nonexistent", "x", "p1"), "Join fails on missing");
    assertTrue(!sys.leaveEvent("nonexistent", "x", "p1"), "Leave fails on missing");
    assertTrue(sys.getEventState("nonexistent", "x") == "", "Empty state on missing");
    assertTrue(sys.getActiveEventCount("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(approxEqual(sys.getElapsedTime("nonexistent", "x"), 0.0f), "0 elapsed on missing");
    assertTrue(!sys.cancelEvent("nonexistent", "x"), "Cancel fails on missing");
}

// ===== OuterRimLogisticsDistortion System Tests =====

void testOuterRimCreate() {
    std::cout << "\n=== OuterRim: Create ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    assertTrue(sys.initializeRegion("region1", "outer_rim_alpha"), "Init region succeeds");
    assertTrue(approxEqual(sys.getGlobalThreat("region1"), 0.0f), "Initial threat is 0");
    assertTrue(sys.getRouteCount("region1") == 0, "No routes initially");
    assertTrue(sys.getDisruptedRouteCount("region1") == 0, "No disrupted routes");
    assertTrue(approxEqual(sys.getTotalPriceImpact("region1"), 0.0f), "No price impact");
}

void testOuterRimAddRoute() {
    std::cout << "\n=== OuterRim: AddRoute ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    assertTrue(sys.addRoute("region1", "route_a"), "Add route succeeds");
    assertTrue(sys.getRouteCount("region1") == 1, "Route count is 1");
    assertTrue(approxEqual(sys.getRouteEfficiency("region1", "route_a"), 1.0f), "Default efficiency is 1.0");
    assertTrue(!sys.addRoute("region1", "route_a"), "Duplicate route fails");
}

void testOuterRimGlobalThreat() {
    std::cout << "\n=== OuterRim: GlobalThreat ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    assertTrue(sys.setGlobalThreat("region1", 0.6f), "Set global threat");
    assertTrue(approxEqual(sys.getGlobalThreat("region1"), 0.6f), "Threat is 0.6");
    assertTrue(sys.setGlobalThreat("region1", 2.0f), "Set threat past max");
    assertTrue(approxEqual(sys.getGlobalThreat("region1"), 1.0f), "Threat clamped at 1.0");
    assertTrue(sys.setGlobalThreat("region1", -1.0f), "Set threat below min");
    assertTrue(approxEqual(sys.getGlobalThreat("region1"), 0.0f), "Threat clamped at 0.0");
}

void testOuterRimRouteThreat() {
    std::cout << "\n=== OuterRim: RouteThreat ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    assertTrue(sys.setRouteThreat("region1", "route_a", 0.4f), "Set route threat");
    assertTrue(!sys.setRouteThreat("region1", "route_missing", 0.5f), "Set threat on missing route fails");
}

void testOuterRimUpdateDistortion() {
    std::cout << "\n=== OuterRim: UpdateDistortion ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    sys.setGlobalThreat("region1", 0.8f);
    sys.update(1.0f);
    // target_efficiency = 1.0 - (0.8 * 0.5) = 0.6, distortion is instant downward
    assertTrue(approxEqual(sys.getRouteEfficiency("region1", "route_a"), 0.6f), "Efficiency dropped to 0.6");
}

void testOuterRimPriceImpact() {
    std::cout << "\n=== OuterRim: PriceImpact ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    sys.setGlobalThreat("region1", 0.8f);
    sys.update(1.0f);
    // price_impact = (1.0 - 0.6) * 2.0 = 0.8
    assertTrue(approxEqual(sys.getRoutePriceImpact("region1", "route_a"), 0.8f), "Price impact is 0.8");
    assertTrue(approxEqual(sys.getTotalPriceImpact("region1"), 0.8f), "Total price impact is 0.8");
}

void testOuterRimRecovery() {
    std::cout << "\n=== OuterRim: Recovery ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    sys.setGlobalThreat("region1", 0.8f);
    sys.update(1.0f); // efficiency drops to 0.6
    sys.setGlobalThreat("region1", 0.0f);
    sys.update(1.0f); // recovery: 0.6 + 0.05*1.0 = 0.65
    float eff = sys.getRouteEfficiency("region1", "route_a");
    assertTrue(approxEqual(eff, 0.65f), "Efficiency recovering toward 1.0");
}

void testOuterRimDisruptedCount() {
    std::cout << "\n=== OuterRim: DisruptedCount ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    sys.addRoute("region1", "route_a");
    sys.addRoute("region1", "route_b");
    sys.setGlobalThreat("region1", 0.8f);
    sys.update(1.0f);
    // Both routes at 0.6 efficiency (< 0.8 threshold)
    assertTrue(sys.getDisruptedRouteCount("region1") == 2, "2 disrupted routes");
}

void testOuterRimMaxRoutes() {
    std::cout << "\n=== OuterRim: MaxRoutes ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    world.createEntity("region1");
    sys.initializeRegion("region1", "outer_rim_alpha");
    for (int i = 0; i < 55; i++) {
        sys.addRoute("region1", "route_" + std::to_string(i));
    }
    assertTrue(sys.getRouteCount("region1") == 50, "Routes capped at max_routes");
}

void testOuterRimMissing() {
    std::cout << "\n=== OuterRim: Missing ===" << std::endl;
    ecs::World world;
    systems::OuterRimLogisticsDistortionSystem sys(&world);
    assertTrue(!sys.initializeRegion("nonexistent", "r1"), "Init fails on missing entity");
    assertTrue(!sys.addRoute("nonexistent", "route_a"), "Add route fails on missing");
    assertTrue(!sys.setGlobalThreat("nonexistent", 0.5f), "Set threat fails on missing");
    assertTrue(!sys.setRouteThreat("nonexistent", "route_a", 0.5f), "Set route threat fails on missing");
    assertTrue(approxEqual(sys.getRouteEfficiency("nonexistent", "route_a"), 0.0f), "0 efficiency on missing");
    assertTrue(approxEqual(sys.getGlobalThreat("nonexistent"), 0.0f), "0 threat on missing");
    assertTrue(sys.getRouteCount("nonexistent") == 0, "0 routes on missing");
    assertTrue(sys.getDisruptedRouteCount("nonexistent") == 0, "0 disrupted on missing");
    assertTrue(approxEqual(sys.getTotalPriceImpact("nonexistent"), 0.0f), "0 price impact on missing");
}

// ===== GalacticResponseCurve System Tests =====

void testGalacticResponseCreate() {
    std::cout << "\n=== GalacticResponse: Create ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    assertTrue(sys.initializeFaction("faction1", "Solari"), "Init faction succeeds");
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 0.0f), "Initial threat is 0");
    assertTrue(sys.getResponseTier("faction1") == 0, "Initial tier is 0");
    assertTrue(sys.getReinforcementCount("faction1") == 0, "No reinforcements initially");
    assertTrue(sys.getReroutedSystemCount("faction1") == 0, "No rerouted systems");
    assertTrue(approxEqual(sys.getEscalationRate("faction1"), 1.0f), "Default escalation rate is 1.0");
    assertTrue(!sys.isFullMobilization("faction1"), "No full mobilization initially");
}

void testGalacticResponseThreat() {
    std::cout << "\n=== GalacticResponse: Threat ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Solari");
    assertTrue(sys.reportThreat("faction1", 15.0f), "Report threat succeeds");
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 15.0f), "Threat is 15");
    sys.update(0.0f); // update tiers
    assertTrue(sys.getResponseTier("faction1") == 1, "Tier 1 (Alert) at threat 15");
}

void testGalacticResponseTierEscalation() {
    std::cout << "\n=== GalacticResponse: TierEscalation ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Veyren");
    sys.reportThreat("faction1", 30.0f);
    sys.update(0.0f);
    assertTrue(sys.getResponseTier("faction1") == 2, "Tier 2 (Mobilize) at threat 30");
    sys.reportThreat("faction1", 25.0f); // total: 55
    sys.update(0.0f);
    assertTrue(sys.getResponseTier("faction1") == 3, "Tier 3 (Reinforce) at threat 55");
    sys.reportThreat("faction1", 30.0f); // total: 85
    sys.update(0.0f);
    assertTrue(sys.getResponseTier("faction1") == 4, "Tier 4 (FullMobilization) at threat 85");
    assertTrue(sys.isFullMobilization("faction1"), "Full mobilization active");
}

void testGalacticResponseReinforcement() {
    std::cout << "\n=== GalacticResponse: Reinforcement ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Keldari");
    sys.reportThreat("faction1", 55.0f);
    sys.update(0.0f); // tier 3
    assertTrue(sys.dispatchReinforcement("faction1"), "Dispatch at tier 3 succeeds");
    assertTrue(sys.getReinforcementCount("faction1") == 1, "1 reinforcement dispatched");
    assertTrue(sys.dispatchReinforcement("faction1"), "Second dispatch succeeds");
    assertTrue(sys.getReinforcementCount("faction1") == 2, "2 reinforcements dispatched");
}

void testGalacticResponseReinforcementBlocked() {
    std::cout << "\n=== GalacticResponse: ReinforcementBlocked ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Aurelian");
    sys.reportThreat("faction1", 20.0f);
    sys.update(0.0f); // tier 2
    assertTrue(!sys.dispatchReinforcement("faction1"), "Dispatch blocked at tier 2");
    assertTrue(sys.getReinforcementCount("faction1") == 0, "No reinforcements dispatched");
}

void testGalacticResponseReroute() {
    std::cout << "\n=== GalacticResponse: Reroute ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Solari");
    assertTrue(sys.rerouteTradeFor("faction1", "system_alpha"), "Reroute succeeds");
    assertTrue(sys.getReroutedSystemCount("faction1") == 1, "1 system rerouted");
    assertTrue(!sys.rerouteTradeFor("faction1", "system_alpha"), "Duplicate reroute fails");
    assertTrue(sys.rerouteTradeFor("faction1", "system_beta"), "Second reroute succeeds");
    assertTrue(sys.getReroutedSystemCount("faction1") == 2, "2 systems rerouted");
}

void testGalacticResponseDecay() {
    std::cout << "\n=== GalacticResponse: Decay ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Veyren");
    sys.reportThreat("faction1", 30.0f);
    sys.update(10.0f); // decay: 30 - 0.1*10 = 29
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 29.0f), "Threat decayed to 29");
    assertTrue(sys.getResponseTier("faction1") == 2, "Still tier 2 at 29");
}

void testGalacticResponseDecayToZero() {
    std::cout << "\n=== GalacticResponse: DecayToZero ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Keldari");
    sys.reportThreat("faction1", 5.0f);
    sys.update(100.0f); // decay: 5 - 0.1*100 = -5, clamped to 0
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 0.0f), "Threat decayed to 0");
    assertTrue(sys.getResponseTier("faction1") == 0, "Back to tier 0");
}

void testGalacticResponseMultiThreat() {
    std::cout << "\n=== GalacticResponse: MultiThreat ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    world.createEntity("faction1");
    sys.initializeFaction("faction1", "Solari");
    sys.reportThreat("faction1", 5.0f);
    sys.reportThreat("faction1", 8.0f);
    assertTrue(approxEqual(sys.getThreatLevel("faction1"), 13.0f), "Threat accumulated to 13");
    sys.update(0.0f);
    assertTrue(sys.getResponseTier("faction1") == 1, "Tier 1 at 13");
}

void testGalacticResponseMissing() {
    std::cout << "\n=== GalacticResponse: Missing ===" << std::endl;
    ecs::World world;
    systems::GalacticResponseCurveSystem sys(&world);
    assertTrue(!sys.initializeFaction("nonexistent", "Test"), "Init fails on missing entity");
    assertTrue(!sys.reportThreat("nonexistent", 10.0f), "Report fails on missing");
    assertTrue(!sys.dispatchReinforcement("nonexistent"), "Dispatch fails on missing");
    assertTrue(!sys.rerouteTradeFor("nonexistent", "sys1"), "Reroute fails on missing");
    assertTrue(approxEqual(sys.getThreatLevel("nonexistent"), 0.0f), "0 threat on missing");
    assertTrue(sys.getResponseTier("nonexistent") == 0, "0 tier on missing");
    assertTrue(sys.getReinforcementCount("nonexistent") == 0, "0 reinforcements on missing");
    assertTrue(sys.getReroutedSystemCount("nonexistent") == 0, "0 rerouted on missing");
    assertTrue(approxEqual(sys.getEscalationRate("nonexistent"), 0.0f), "0 escalation on missing");
    assertTrue(!sys.isFullMobilization("nonexistent"), "No mobilization on missing");
}

// ==================== ShipCapabilityRating System Tests ====================

void testShipRatingCreate() {
    std::cout << "\n=== ShipRating: Create ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeRating("ship1"), "Init rating succeeds");
    assertTrue(approxEqual(sys.getOverallRating("ship1"), 0.0f), "Overall rating starts at 0");
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 0.0f), "Combat rating starts at 0");
    assertTrue(approxEqual(sys.getMiningRating("ship1"), 0.0f), "Mining rating starts at 0");
}

void testShipRatingCombat() {
    std::cout << "\n=== ShipRating: Combat ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setWeaponCount("ship1", 8);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 5.0f), "8 weapons = 5 star combat");
    sys.setWeaponCount("ship1", 4);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 2.5f), "4 weapons = 2.5 star combat");
}

void testShipRatingMining() {
    std::cout << "\n=== ShipRating: Mining ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setMiningModuleCount("ship1", 5);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getMiningRating("ship1"), 5.0f), "5 mining modules = 5 stars");
    sys.setMiningModuleCount("ship1", 0);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getMiningRating("ship1"), 0.0f), "0 mining modules = 0 stars");
}

void testShipRatingExploration() {
    std::cout << "\n=== ShipRating: Exploration ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setScannerCount("ship1", 4);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getExplorationRating("ship1"), 5.0f), "4 scanners = 5 star exploration");
    sys.setScannerCount("ship1", 2);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getExplorationRating("ship1"), 2.5f), "2 scanners = 2.5 stars");
}

void testShipRatingCargo() {
    std::cout << "\n=== ShipRating: Cargo ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setCargoCapacity("ship1", 50000.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCargoRating("ship1"), 5.0f), "50000 m3 = 5 star cargo");
    sys.setCargoCapacity("ship1", 25000.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCargoRating("ship1"), 2.5f), "25000 m3 = 2.5 star cargo");
}

void testShipRatingDefense() {
    std::cout << "\n=== ShipRating: Defense ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setTotalEHP("ship1", 100000.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getDefenseRating("ship1"), 5.0f), "100k EHP = 5 star defense");
    sys.setTotalEHP("ship1", 50000.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getDefenseRating("ship1"), 2.5f), "50k EHP = 2.5 star defense");
}

void testShipRatingFabrication() {
    std::cout << "\n=== ShipRating: Fabrication ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setIndustryModuleCount("ship1", 5);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getFabricationRating("ship1"), 5.0f), "5 industry = 5 star fabrication");
}

void testShipRatingOverall() {
    std::cout << "\n=== ShipRating: Overall ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setWeaponCount("ship1", 8);
    sys.setMiningModuleCount("ship1", 5);
    sys.setScannerCount("ship1", 4);
    sys.setCargoCapacity("ship1", 50000.0f);
    sys.setTotalEHP("ship1", 100000.0f);
    sys.setIndustryModuleCount("ship1", 5);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getOverallRating("ship1"), 5.0f), "All maxed = 5 star overall");
}

void testShipRatingRecalculate() {
    std::cout << "\n=== ShipRating: Recalculate ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeRating("ship1");
    sys.setWeaponCount("ship1", 4);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 2.5f), "Initial combat 2.5");
    // After update, needs_recalculation is false; modify and force recalc
    assertTrue(sys.recalculate("ship1"), "Recalculate succeeds");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCombatRating("ship1"), 2.5f), "Recalculated same value");
}

void testShipRatingMissing() {
    std::cout << "\n=== ShipRating: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipCapabilityRatingSystem sys(&world);
    assertTrue(!sys.initializeRating("nonexistent"), "Init on missing entity fails");
    assertTrue(approxEqual(sys.getCombatRating("nonexistent"), 0.0f), "Combat rating 0 on missing");
    assertTrue(approxEqual(sys.getOverallRating("nonexistent"), 0.0f), "Overall rating 0 on missing");
    assertTrue(!sys.setWeaponCount("nonexistent", 5), "Set weapons on missing fails");
}

// ==================== ModuleCascadingFailure System Tests ====================

void testCascadeCreate() {
    std::cout << "\n=== Cascade: Create ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeShip("ship1"), "Init ship succeeds");
    assertTrue(sys.getModuleCount("ship1") == 0, "No modules initially");
    assertTrue(sys.getTotalFailures("ship1") == 0, "No failures initially");
    assertTrue(sys.getCascadeEvents("ship1") == 0, "No cascades initially");
}

void testCascadeAddModule() {
    std::cout << "\n=== Cascade: AddModule ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    assertTrue(sys.addModule("ship1", "power_core", "Power", 200.0f), "Add power module");
    assertTrue(sys.addModule("ship1", "weapon_1", "Weapon", 100.0f), "Add weapon module");
    assertTrue(sys.getModuleCount("ship1") == 2, "2 modules added");
    assertTrue(!sys.addModule("ship1", "power_core", "Power", 200.0f), "Duplicate module fails");
    assertTrue(sys.isModuleOnline("ship1", "power_core"), "Power module online");
}

void testCascadeDamage() {
    std::cout << "\n=== Cascade: Damage ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    assertTrue(sys.damageModule("ship1", "weapon_1", 30.0f), "Damage weapon");
    assertTrue(approxEqual(sys.getModuleHP("ship1", "weapon_1"), 70.0f), "HP reduced to 70");
    assertTrue(sys.isModuleOnline("ship1", "weapon_1"), "Still online at 70 HP");
    assertTrue(!sys.isModuleDestroyed("ship1", "weapon_1"), "Not destroyed at 70 HP");
}

void testCascadeDestroy() {
    std::cout << "\n=== Cascade: Destroy ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    sys.damageModule("ship1", "weapon_1", 100.0f);
    assertTrue(sys.isModuleDestroyed("ship1", "weapon_1"), "Weapon destroyed at 0 HP");
    assertTrue(!sys.isModuleOnline("ship1", "weapon_1"), "Destroyed module offline");
    assertTrue(sys.getTotalFailures("ship1") == 1, "1 failure recorded");
    assertTrue(!sys.damageModule("ship1", "weapon_1", 10.0f), "Can't damage destroyed module");
}

void testCascadeRepair() {
    std::cout << "\n=== Cascade: Repair ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    sys.damageModule("ship1", "weapon_1", 100.0f);
    assertTrue(sys.isModuleDestroyed("ship1", "weapon_1"), "Module destroyed");
    sys.repairModule("ship1", "weapon_1", 50.0f);
    assertTrue(!sys.isModuleDestroyed("ship1", "weapon_1"), "Module repaired (not destroyed)");
    assertTrue(sys.isModuleOnline("ship1", "weapon_1"), "Repaired module comes online");
    assertTrue(approxEqual(sys.getModuleHP("ship1", "weapon_1"), 50.0f), "HP is 50 after repair");
}

void testCascadeDependency() {
    std::cout << "\n=== Cascade: Dependency ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "power_core", "Power", 200.0f);
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    assertTrue(sys.addDependency("ship1", "weapon_1", "power_core"), "Add dependency succeeds");
    assertTrue(!sys.addDependency("ship1", "weapon_1", "power_core"), "Duplicate dependency fails");
    assertTrue(!sys.addDependency("ship1", "weapon_1", "nonexistent"), "Dependency on missing fails");
}

void testCascadePropagation() {
    std::cout << "\n=== Cascade: Propagation ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "power_core", "Power", 200.0f);
    sys.addModule("ship1", "weapon_1", "Weapon", 100.0f);
    sys.addModule("ship1", "shield_1", "Shield", 150.0f);
    sys.addDependency("ship1", "weapon_1", "power_core");
    sys.addDependency("ship1", "shield_1", "power_core");
    
    // Destroy power core -> weapons and shields should go offline
    sys.damageModule("ship1", "power_core", 200.0f);
    sys.update(0.0f);
    assertTrue(!sys.isModuleOnline("ship1", "weapon_1"), "Weapon offline after power loss");
    assertTrue(!sys.isModuleOnline("ship1", "shield_1"), "Shield offline after power loss");
    assertTrue(sys.getCascadeEvents("ship1") == 2, "2 cascade events");
}

void testCascadeChain() {
    std::cout << "\n=== Cascade: Chain ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "reactor", "Power", 200.0f);
    sys.addModule("ship1", "distributor", "Power", 150.0f);
    sys.addModule("ship1", "turret", "Weapon", 80.0f);
    sys.addDependency("ship1", "distributor", "reactor");
    sys.addDependency("ship1", "turret", "distributor");
    
    // Destroy reactor -> distributor offline -> turret offline (chain cascade)
    sys.damageModule("ship1", "reactor", 200.0f);
    sys.update(0.0f);
    assertTrue(!sys.isModuleOnline("ship1", "distributor"), "Distributor offline (direct cascade)");
    assertTrue(!sys.isModuleOnline("ship1", "turret"), "Turret offline (chain cascade)");
    assertTrue(sys.getCascadeEvents("ship1") == 2, "2 cascade events in chain");
}

void testCascadeOnlineCount() {
    std::cout << "\n=== Cascade: OnlineCount ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeShip("ship1");
    sys.addModule("ship1", "mod1", "Weapon", 100.0f);
    sys.addModule("ship1", "mod2", "Shield", 100.0f);
    sys.addModule("ship1", "mod3", "Engine", 100.0f);
    assertTrue(sys.getOnlineModuleCount("ship1") == 3, "3 modules online");
    sys.damageModule("ship1", "mod1", 100.0f);
    assertTrue(sys.getOnlineModuleCount("ship1") == 2, "2 modules online after destroy");
}

void testCascadeMissing() {
    std::cout << "\n=== Cascade: Missing ===" << std::endl;
    ecs::World world;
    systems::ModuleCascadingFailureSystem sys(&world);
    assertTrue(!sys.initializeShip("nonexistent"), "Init missing entity fails");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "Module count 0 on missing");
    assertTrue(sys.getTotalFailures("nonexistent") == 0, "Failures 0 on missing");
    assertTrue(!sys.addModule("nonexistent", "mod1", "Weapon", 100.0f), "Add module to missing fails");
    assertTrue(approxEqual(sys.getModuleHP("nonexistent", "mod1"), 0.0f), "HP 0 on missing");
}

// ==================== AsteroidBelt System Tests ====================

void testAsteroidBeltCreate() {
    std::cout << "\n=== AsteroidBelt: Create ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    assertTrue(sys.initializeBelt("belt1", "belt_alpha", "sys1"), "Init belt succeeds");
    assertTrue(sys.getAsteroidCount("belt1") == 0, "No asteroids initially");
    assertTrue(sys.getTotalMined("belt1") == 0, "No mined initially");
    assertTrue(sys.getTotalRespawned("belt1") == 0, "No respawned initially");
}

void testAsteroidBeltAdd() {
    std::cout << "\n=== AsteroidBelt: Add ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    assertTrue(sys.addAsteroid("belt1", "ast1", "Veldspar", 5000.0f, 1.0f), "Add asteroid succeeds");
    assertTrue(sys.getAsteroidCount("belt1") == 1, "1 asteroid");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 5000.0f), "5000 ore remaining");
}

void testAsteroidBeltDuplicate() {
    std::cout << "\n=== AsteroidBelt: Duplicate ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 5000.0f, 1.0f);
    assertTrue(!sys.addAsteroid("belt1", "ast1", "Scordite", 3000.0f, 1.5f), "Duplicate rejected");
    assertTrue(sys.getAsteroidCount("belt1") == 1, "Still 1 asteroid");
}

void testAsteroidBeltMine() {
    std::cout << "\n=== AsteroidBelt: Mine ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 5000.0f, 1.0f);
    float mined = sys.mineAsteroid("belt1", "ast1", 1000.0f);
    assertTrue(approxEqual(mined, 1000.0f), "Mined 1000 ore");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 4000.0f), "4000 remaining");
    assertTrue(!sys.isAsteroidDepleted("belt1", "ast1"), "Not depleted");
}

void testAsteroidBeltDeplete() {
    std::cout << "\n=== AsteroidBelt: Deplete ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Scordite", 1000.0f, 1.0f);
    sys.mineAsteroid("belt1", "ast1", 1000.0f);
    assertTrue(sys.isAsteroidDepleted("belt1", "ast1"), "Asteroid depleted");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 0.0f), "0 ore remaining");
    assertTrue(sys.getTotalMined("belt1") == 1, "1 total mined");
    assertTrue(sys.getDepletedCount("belt1") == 1, "1 depleted");
}

void testAsteroidBeltRichness() {
    std::cout << "\n=== AsteroidBelt: Richness ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Kernite", 5000.0f, 2.0f);
    // Mining 500 with richness 2.0 → actual = 500 * 2.0 = 1000
    float mined = sys.mineAsteroid("belt1", "ast1", 500.0f);
    assertTrue(approxEqual(mined, 1000.0f), "Richness doubles yield");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 4000.0f), "4000 remaining");
}

void testAsteroidBeltRespawn() {
    std::cout << "\n=== AsteroidBelt: Respawn ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 1000.0f, 1.0f);
    sys.mineAsteroid("belt1", "ast1", 1000.0f); // deplete
    assertTrue(sys.isAsteroidDepleted("belt1", "ast1"), "Depleted after mining");
    // Advance past respawn interval (default 3600s)
    sys.update(3600.0f);
    assertTrue(!sys.isAsteroidDepleted("belt1", "ast1"), "Respawned after interval");
    assertTrue(approxEqual(sys.getRemainingOre("belt1", "ast1"), 1000.0f), "Full ore restored");
    assertTrue(sys.getTotalRespawned("belt1") == 1, "1 respawn counted");
}

void testAsteroidBeltRemove() {
    std::cout << "\n=== AsteroidBelt: Remove ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    sys.addAsteroid("belt1", "ast1", "Veldspar", 5000.0f, 1.0f);
    sys.addAsteroid("belt1", "ast2", "Scordite", 3000.0f, 1.0f);
    assertTrue(sys.removeAsteroid("belt1", "ast1"), "Remove succeeds");
    assertTrue(sys.getAsteroidCount("belt1") == 1, "1 asteroid remaining");
    assertTrue(!sys.removeAsteroid("belt1", "ast1"), "Remove nonexistent fails");
}

void testAsteroidBeltMaxLimit() {
    std::cout << "\n=== AsteroidBelt: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    world.createEntity("belt1");
    sys.initializeBelt("belt1", "belt_alpha", "sys1");
    auto* entity = world.getEntity("belt1");
    auto* belt = entity->getComponent<components::AsteroidBelt>();
    belt->max_asteroids = 3;
    sys.addAsteroid("belt1", "a1", "Veldspar", 1000.0f, 1.0f);
    sys.addAsteroid("belt1", "a2", "Scordite", 1000.0f, 1.0f);
    sys.addAsteroid("belt1", "a3", "Pyroxeres", 1000.0f, 1.0f);
    assertTrue(!sys.addAsteroid("belt1", "a4", "Kernite", 1000.0f, 1.0f), "Max limit enforced");
    assertTrue(sys.getAsteroidCount("belt1") == 3, "Still 3 asteroids");
}

void testAsteroidBeltMissing() {
    std::cout << "\n=== AsteroidBelt: Missing ===" << std::endl;
    ecs::World world;
    systems::AsteroidBeltSystem sys(&world);
    assertTrue(!sys.initializeBelt("nonexistent", "b1", "s1"), "Init fails on missing");
    assertTrue(!sys.addAsteroid("nonexistent", "a1", "Ore", 100.0f, 1.0f), "Add fails on missing");
    assertTrue(!sys.removeAsteroid("nonexistent", "a1"), "Remove fails on missing");
    assertTrue(approxEqual(sys.mineAsteroid("nonexistent", "a1", 100.0f), 0.0f), "Mine returns 0 on missing");
    assertTrue(sys.getAsteroidCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getDepletedCount("nonexistent") == 0, "0 depleted on missing");
    assertTrue(approxEqual(sys.getRemainingOre("nonexistent", "a1"), 0.0f), "0 ore on missing");
    assertTrue(!sys.isAsteroidDepleted("nonexistent", "a1"), "Not depleted on missing");
    assertTrue(sys.getTotalMined("nonexistent") == 0, "0 mined on missing");
    assertTrue(sys.getTotalRespawned("nonexistent") == 0, "0 respawned on missing");
}

// ==================== Incursion System Tests ====================

void testIncursionCreate() {
    std::cout << "\n=== Incursion: Create ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    assertTrue(sys.initialize("inc1", "incursion_alpha", "system_jita", 2), "Init succeeds");
    assertTrue(sys.getInfluence("inc1") > 0.9f, "Influence starts at 1.0");
    assertTrue(sys.getParticipantCount("inc1") == 0, "No participants initially");
    assertTrue(sys.getState("inc1") == 0, "State is Pending");
    assertTrue(approxEqual(sys.getRewardPool("inc1"), 0.0f), "Reward pool is 0");
    assertTrue(sys.getTotalWavesDefeated("inc1") == 0, "No waves defeated");
}

void testIncursionAddWave() {
    std::cout << "\n=== Incursion: AddWave ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    assertTrue(sys.addWave("inc1", 1, "Frigate", 5), "Add wave 1 succeeds");
    assertTrue(sys.addWave("inc1", 2, "Cruiser", 3), "Add wave 2 succeeds");
    assertTrue(sys.addWave("inc1", 3, "Battleship", 1), "Add wave 3 succeeds");
}

void testIncursionDuplicate() {
    std::cout << "\n=== Incursion: Duplicate ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    sys.addWave("inc1", 1, "Frigate", 5);
    assertTrue(!sys.addWave("inc1", 1, "Cruiser", 3), "Duplicate wave rejected");
    sys.joinIncursion("inc1", "player1");
    assertTrue(!sys.joinIncursion("inc1", "player1"), "Duplicate participant rejected");
}

void testIncursionSpawn() {
    std::cout << "\n=== Incursion: Spawn ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    sys.addWave("inc1", 1, "Frigate", 5);
    assertTrue(sys.spawnWave("inc1", 1), "Spawn wave succeeds");
    assertTrue(!sys.spawnWave("inc1", 1), "Double spawn rejected");
    assertTrue(!sys.spawnWave("inc1", 99), "Spawn nonexistent wave fails");
}

void testIncursionDefeat() {
    std::cout << "\n=== Incursion: Defeat ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    sys.addWave("inc1", 1, "Frigate", 5);
    sys.spawnWave("inc1", 1);
    float before = sys.getInfluence("inc1");
    assertTrue(sys.defeatWave("inc1", 1), "Defeat wave succeeds");
    assertTrue(sys.getInfluence("inc1") < before, "Influence decreased");
    assertTrue(sys.getTotalWavesDefeated("inc1") == 1, "1 wave defeated");
    assertTrue(!sys.defeatWave("inc1", 1), "Double defeat rejected");
}

void testIncursionJoin() {
    std::cout << "\n=== Incursion: Join ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    assertTrue(sys.joinIncursion("inc1", "player1"), "Join succeeds");
    assertTrue(sys.joinIncursion("inc1", "player2"), "Join second succeeds");
    assertTrue(sys.getParticipantCount("inc1") == 2, "2 participants");
    assertTrue(sys.leaveIncursion("inc1", "player1"), "Leave succeeds");
    assertTrue(sys.getParticipantCount("inc1") == 1, "1 participant after leave");
    assertTrue(!sys.leaveIncursion("inc1", "player1"), "Double leave fails");
}

void testIncursionRewards() {
    std::cout << "\n=== Incursion: Rewards ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);
    assertTrue(sys.contributeRewards("inc1", 1000.0f), "Contribute succeeds");
    assertTrue(sys.contributeRewards("inc1", 500.0f), "Contribute again succeeds");
    assertTrue(approxEqual(sys.getRewardPool("inc1"), 1500.0f), "Pool is 1500");
}

void testIncursionStateTransition() {
    std::cout << "\n=== Incursion: StateTransition ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);

    auto* entity = world.getEntity("inc1");
    auto* inc = entity->getComponent<components::Incursion>();
    inc->max_waves = 2;
    inc->state = components::Incursion::IncursionState::Active;

    sys.addWave("inc1", 1, "Frigate", 5);
    sys.addWave("inc1", 2, "Cruiser", 3);

    sys.defeatWave("inc1", 1);
    sys.update(0.1f);
    assertTrue(sys.getState("inc1") == 1, "Still Active after 1 wave defeated");

    sys.defeatWave("inc1", 2);
    sys.update(0.1f);
    assertTrue(sys.getState("inc1") == 2, "Withdrawing after all waves defeated");

    inc->influence = 0.0f;
    sys.update(0.1f);
    assertTrue(sys.getState("inc1") == 3, "Defeated when influence is 0");
}

void testIncursionMaxLimit() {
    std::cout << "\n=== Incursion: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    world.createEntity("inc1");
    sys.initialize("inc1", "incursion_alpha", "system_jita", 1);

    auto* entity = world.getEntity("inc1");
    auto* inc = entity->getComponent<components::Incursion>();
    inc->max_waves = 2;
    inc->max_participants = 2;

    sys.addWave("inc1", 1, "Frigate", 5);
    sys.addWave("inc1", 2, "Cruiser", 3);
    assertTrue(!sys.addWave("inc1", 3, "Battleship", 1), "Max waves enforced");

    sys.joinIncursion("inc1", "p1");
    sys.joinIncursion("inc1", "p2");
    assertTrue(!sys.joinIncursion("inc1", "p3"), "Max participants enforced");
}

void testIncursionMissing() {
    std::cout << "\n=== Incursion: Missing ===" << std::endl;
    ecs::World world;
    systems::IncursionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "i1", "s1", 1), "Init fails on missing");
    assertTrue(!sys.addWave("nonexistent", 1, "Frigate", 5), "AddWave fails on missing");
    assertTrue(!sys.spawnWave("nonexistent", 1), "SpawnWave fails on missing");
    assertTrue(!sys.defeatWave("nonexistent", 1), "DefeatWave fails on missing");
    assertTrue(!sys.joinIncursion("nonexistent", "p1"), "Join fails on missing");
    assertTrue(!sys.leaveIncursion("nonexistent", "p1"), "Leave fails on missing");
    assertTrue(!sys.contributeRewards("nonexistent", 100.0f), "Contribute fails on missing");
    assertTrue(approxEqual(sys.getInfluence("nonexistent"), 0.0f), "0 influence on missing");
    assertTrue(sys.getParticipantCount("nonexistent") == 0, "0 participants on missing");
    assertTrue(sys.getState("nonexistent") == 0, "0 state on missing");
    assertTrue(approxEqual(sys.getRewardPool("nonexistent"), 0.0f), "0 pool on missing");
    assertTrue(sys.getTotalWavesDefeated("nonexistent") == 0, "0 defeated on missing");
}

// ==================== CloneBay System Tests ====================

void testCloneBayCreate() {
    std::cout << "\n=== CloneBay: Create ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    assertTrue(sys.initialize("bay1", "cb_01", "station_jita"), "Init succeeds");
    assertTrue(sys.getCloneCount("bay1") == 0, "No clones initially");
    assertTrue(sys.getImplantCount("bay1") == 0, "No implants initially");
    assertTrue(sys.getActiveClone("bay1") == 0, "No active clone");
    assertTrue(sys.getTotalDeaths("bay1") == 0, "No deaths initially");
}

void testCloneBayAddClone() {
    std::cout << "\n=== CloneBay: AddClone ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    assertTrue(sys.addClone("bay1", "clone_a", 1), "Add Alpha clone");
    assertTrue(sys.addClone("bay1", "clone_b", 3), "Add Gamma clone");
    assertTrue(sys.getCloneCount("bay1") == 2, "2 clones");
}

void testCloneBayDuplicate() {
    std::cout << "\n=== CloneBay: Duplicate ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 1);
    assertTrue(!sys.addClone("bay1", "clone_a", 2), "Duplicate clone rejected");
    sys.addClone("bay1", "clone_b", 2);
    sys.installImplant("bay1", "imp1", 1, "perception", 3.0f, "clone_b");
    assertTrue(!sys.installImplant("bay1", "imp1", 2, "memory", 3.0f, "clone_b"), "Duplicate implant rejected");
}

void testCloneBayActivate() {
    std::cout << "\n=== CloneBay: Activate ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 1);
    sys.addClone("bay1", "clone_b", 3);
    assertTrue(sys.activateClone("bay1", "clone_a"), "Activate clone_a");
    assertTrue(sys.getActiveClone("bay1") == 1, "Active clone is grade 1");
    assertTrue(sys.activateClone("bay1", "clone_b"), "Activate clone_b");
    assertTrue(sys.getActiveClone("bay1") == 3, "Active clone is grade 3");
}

void testCloneBayImplant() {
    std::cout << "\n=== CloneBay: Implant ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 3);
    assertTrue(sys.installImplant("bay1", "imp1", 1, "perception", 3.0f, "clone_a"), "Install implant");
    assertTrue(sys.getImplantCount("bay1") == 1, "1 implant");
    assertTrue(sys.removeImplant("bay1", "imp1"), "Remove implant");
    assertTrue(sys.getImplantCount("bay1") == 0, "0 implants after remove");
}

void testCloneBayImplantSlot() {
    std::cout << "\n=== CloneBay: ImplantSlot ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 2);  // grade 2 = 2 implant slots
    assertTrue(sys.installImplant("bay1", "imp1", 1, "perception", 3.0f, "clone_a"), "Slot 1");
    assertTrue(sys.installImplant("bay1", "imp2", 2, "memory", 3.0f, "clone_a"), "Slot 2");
    assertTrue(!sys.installImplant("bay1", "imp3", 3, "willpower", 3.0f, "clone_a"), "Slot limit reached");
    assertTrue(sys.getImplantCount("bay1") == 2, "2 implants");
}

void testCloneBayDeath() {
    std::cout << "\n=== CloneBay: Death ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 1);  // sp_limit = 900000
    sys.activateClone("bay1", "clone_a");
    float loss = sys.processDeath("bay1", 1500000.0f);
    assertTrue(loss > 500000.0f, "SP loss is above 500k");
    assertTrue(approxEqual(loss, 600000.0f), "SP loss is 600000");
    assertTrue(sys.getTotalDeaths("bay1") == 1, "1 death recorded");
}

void testCloneBayGrades() {
    std::cout << "\n=== CloneBay: Grades ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    sys.addClone("bay1", "clone_a", 1);
    sys.addClone("bay1", "clone_b", 5);
    auto* entity = world.getEntity("bay1");
    auto* bay = entity->getComponent<components::CloneBay>();
    assertTrue(approxEqual(bay->clones[0].sp_limit, 900000.0f), "Grade 1 sp_limit 900k");
    assertTrue(approxEqual(bay->clones[1].sp_limit, 4500000.0f), "Grade 5 sp_limit 4500k");
    assertTrue(approxEqual(bay->clones[0].cost, 5000000.0f), "Grade 1 cost 5M");
    assertTrue(approxEqual(bay->clones[1].cost, 25000000.0f), "Grade 5 cost 25M");
}

void testCloneBayMaxLimit() {
    std::cout << "\n=== CloneBay: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    world.createEntity("bay1");
    sys.initialize("bay1", "cb_01", "station_jita");
    auto* entity = world.getEntity("bay1");
    auto* bay = entity->getComponent<components::CloneBay>();
    bay->max_clones = 2;
    bay->max_implants = 1;
    sys.addClone("bay1", "c1", 3);
    sys.addClone("bay1", "c2", 3);
    assertTrue(!sys.addClone("bay1", "c3", 3), "Max clones enforced");
    sys.installImplant("bay1", "i1", 1, "perception", 3.0f, "c1");
    assertTrue(!sys.installImplant("bay1", "i2", 2, "memory", 3.0f, "c1"), "Max implants enforced");
}

void testCloneBayMissing() {
    std::cout << "\n=== CloneBay: Missing ===" << std::endl;
    ecs::World world;
    systems::CloneBaySystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "cb1", "s1"), "Init fails on missing");
    assertTrue(!sys.addClone("nonexistent", "c1", 1), "AddClone fails on missing");
    assertTrue(!sys.removeClone("nonexistent", "c1"), "RemoveClone fails on missing");
    assertTrue(!sys.activateClone("nonexistent", "c1"), "Activate fails on missing");
    assertTrue(!sys.installImplant("nonexistent", "i1", 1, "perception", 3.0f, "c1"), "InstallImplant fails on missing");
    assertTrue(!sys.removeImplant("nonexistent", "i1"), "RemoveImplant fails on missing");
    assertTrue(approxEqual(sys.processDeath("nonexistent", 1000.0f), 0.0f), "0 SP loss on missing");
    assertTrue(sys.getActiveClone("nonexistent") == 0, "0 active on missing");
    assertTrue(sys.getCloneCount("nonexistent") == 0, "0 clones on missing");
    assertTrue(sys.getImplantCount("nonexistent") == 0, "0 implants on missing");
    assertTrue(sys.getTotalDeaths("nonexistent") == 0, "0 deaths on missing");
    assertTrue(approxEqual(sys.getSkillPointsAtRisk("nonexistent", 1000.0f), 0.0f), "0 SP at risk on missing");
}


void run_special_systems_tests() {
    testAnomalyVisualCueDefaults();
    testAnomalyVisualCueTypeMapping();
    testLegendEmpty();
    testLegendRecord();
    testLegendTitle();
    testAncientTechDefaults();
    testAncientTechRepair();
    testAncientTechReverseEngineer();
    testSurvivalDefaults();
    testSurvivalDrain();
    testSurvivalRefill();
    testSurvivalHunger();
    testMenuStateDefaults();
    testMenuNavigateTo();
    testMenuGoBack();
    testMenuIsInGame();
    testRaceDefaults();
    testRaceApplyDefaults();
    testLoreAddEntry();
    testLoreMaxEntries();
    testVisualCueDefaults();
    testVisualCueLockdown();
    testVisualCueTrafficDensity();
    testVisualCueThreatGlow();
    testVisualCueProsperity();
    testVisualCuePirateWarning();
    testSupplyDemandDefaults();
    testSupplyDemandAddCommodity();
    testSupplyDemandPriceCalculation();
    testSupplyDemandPriceFloor();
    testSupplyDemandPriceCeiling();
    testSupplyDemandNPCModifier();
    testMythBossDefaults();
    testMythBossGenerateEncounter();
    testMythBossDifficulty();
    testMythBossComplete();
    testMythBossExpiry();
    testMythBossTypeName();
    testMythBossSourceMyth();
    testAncientTechUpgradeDefaults();
    testAncientTechUpgradeStart();
    testAncientTechUpgradeNotRepaired();
    testAncientTechUpgradeComplete();
    testAncientTechUpgradeCancel();
    testAncientTechUpgradeProgress();
    testAncientTechUpgradeBonusDescription();
    testModManifestDefaults();
    testModManifestRegister();
    testModManifestDuplicate();
    testModManifestUnregister();
    testModManifestDependencies();
    testModManifestLoadOrder();
    testModManifestEnableDisable();
    testModManifestEmptyId();
    testAncientAIRemnantDefaults();
    testAncientAIRemnantSpawn();
    testAncientAIRemnantTierScaling();
    testAncientAIRemnantDefeat();
    testAncientAIRemnantExpiry();
    testAncientAIRemnantSiteId();
    testAncientAIRemnantTypeName();
    testViewModeDefaults();
    testViewModeInitialize();
    testViewModeTransition();
    testViewModeInvalidTransition();
    testViewModeCancel();
    testViewModeProgress();
    testViewModeNames();
    testWingManagementCreateWing();
    testWingManagementDuplicateWing();
    testWingManagementDissolveWing();
    testWingManagementAssignMember();
    testWingManagementMaxMembers();
    testWingManagementRemoveMember();
    testWingManagementSetCommander();
    testWingManagementMorale();
    testWingManagementInvalidFleet();
    testWingManagementInvalidRole();
    testTerraformingStart();
    testTerraformingPause();
    testTerraformingResume();
    testTerraformingCancel();
    testTerraformingStageAdvance();
    testTerraformingAutoAdvance();
    testTerraformingSetTargets();
    testTerraformingProgress();
    testTerraformingComplete();
    testTerraformingMissing();
    testFoodProcessorAddRecipe();
    testFoodProcessorDuplicateRecipe();
    testFoodProcessorStartCrafting();
    testFoodProcessorMaxJobs();
    testFoodProcessorCraftCompletion();
    testFoodProcessorCancel();
    testFoodProcessorPowered();
    testFoodProcessorEfficiency();
    testFoodProcessorMissingEntity();
    testFoodProcessorComponentDefaults();
    testIntelRecord();
    testIntelConfidenceDistance();
    testIntelDecay();
    testIntelGhost();
    testIntelRefresh();
    testIntelClear();
    testIntelClearAll();
    testIntelSensorStrength();
    testIntelZeroDecayRemoval();
    testIntelMissing();
    testSolarPanelInit();
    testSolarPanelDeploy();
    testSolarPanelRetract();
    testSolarPanelAddRemove();
    testSolarPanelEnergyOutput();
    testSolarPanelNighttime();
    testSolarPanelDegradation();
    testSolarPanelMaintenance();
    testSolarPanelEnergyStorage();
    testSolarPanelMissing();
    testFarmingDeckInit();
    testFarmingDeckPlant();
    testFarmingDeckGrowth();
    testFarmingDeckHarvest();
    testFarmingDeckHarvestNotReady();
    testFarmingDeckWater();
    testFarmingDeckFertilize();
    testFarmingDeckWithering();
    testFarmingDeckPower();
    testFarmingDeckMissing();
    testGridInit();
    testGridPlace();
    testGridRemove();
    testGridAdjacency();
    testGridPower();
    testGridIntegrity();
    testGridDamage();
    testGridRepair();
    testGridBounds();
    testGridMissing();
    testTransitionInit();
    testTransitionBeginDescent();
    testTransitionDescentSequence();
    testTransitionBeginLaunch();
    testTransitionLaunchSequence();
    testTransitionAbort();
    testTransitionAbortInvalid();
    testTransitionAtmosphereHeat();
    testTransitionAutopilot();
    testTransitionMissing();
    testFleetHangarInit();
    testFleetHangarDock();
    testFleetHangarUndock();
    testFleetHangarFull();
    testFleetHangarLock();
    testFleetHangarUpgrade();
    testFleetHangarMaxTier();
    testFleetHangarRepair();
    testFleetHangarPower();
    testFleetHangarMissing();
    testHangarEnvInit();
    testHangarEnvOpenClose();
    testHangarEnvToxicExposure();
    testHangarEnvCorrosiveExposure();
    testHangarEnvUnsuitedDamage();
    testHangarEnvSuitedProtection();
    testHangarEnvRecovery();
    testHangarEnvAlarm();
    testHangarEnvVacuum();
    testHangarEnvMissing();
    testPvPCreate();
    testPvPEnable();
    testPvPDisable();
    testPvPSafetyLevel();
    testPvPCanEngage();
    testPvPNullSecEngage();
    testPvPRecordEngagement();
    testPvPAggressionCooldown();
    testPvPRecordKill();
    testPvPMissing();
    testDynamicEventCreate();
    testDynamicEventSchedule();
    testDynamicEventStart();
    testDynamicEventJoinLeave();
    testDynamicEventLifecycle();
    testDynamicEventRewardPool();
    testDynamicEventAutoStart();
    testDynamicEventCancel();
    testDynamicEventMaxConcurrent();
    testDynamicEventMissing();
    testOuterRimCreate();
    testOuterRimAddRoute();
    testOuterRimGlobalThreat();
    testOuterRimRouteThreat();
    testOuterRimUpdateDistortion();
    testOuterRimPriceImpact();
    testOuterRimRecovery();
    testOuterRimDisruptedCount();
    testOuterRimMaxRoutes();
    testOuterRimMissing();
    testGalacticResponseCreate();
    testGalacticResponseThreat();
    testGalacticResponseTierEscalation();
    testGalacticResponseReinforcement();
    testGalacticResponseReinforcementBlocked();
    testGalacticResponseReroute();
    testGalacticResponseDecay();
    testGalacticResponseDecayToZero();
    testGalacticResponseMultiThreat();
    testGalacticResponseMissing();
    testShipRatingCreate();
    testShipRatingCombat();
    testShipRatingMining();
    testShipRatingExploration();
    testShipRatingCargo();
    testShipRatingDefense();
    testShipRatingFabrication();
    testShipRatingOverall();
    testShipRatingRecalculate();
    testShipRatingMissing();
    testCascadeCreate();
    testCascadeAddModule();
    testCascadeDamage();
    testCascadeDestroy();
    testCascadeRepair();
    testCascadeDependency();
    testCascadePropagation();
    testCascadeChain();
    testCascadeOnlineCount();
    testCascadeMissing();
    testAsteroidBeltCreate();
    testAsteroidBeltAdd();
    testAsteroidBeltDuplicate();
    testAsteroidBeltMine();
    testAsteroidBeltDeplete();
    testAsteroidBeltRichness();
    testAsteroidBeltRespawn();
    testAsteroidBeltRemove();
    testAsteroidBeltMaxLimit();
    testAsteroidBeltMissing();
    testIncursionCreate();
    testIncursionAddWave();
    testIncursionDuplicate();
    testIncursionSpawn();
    testIncursionDefeat();
    testIncursionJoin();
    testIncursionRewards();
    testIncursionStateTransition();
    testIncursionMaxLimit();
    testIncursionMissing();
    testCloneBayCreate();
    testCloneBayAddClone();
    testCloneBayDuplicate();
    testCloneBayActivate();
    testCloneBayImplant();
    testCloneBayImplantSlot();
    testCloneBayDeath();
    testCloneBayGrades();
    testCloneBayMaxLimit();
    testCloneBayMissing();
}
