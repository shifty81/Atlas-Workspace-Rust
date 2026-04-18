/**
 * Data tests
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

// ==================== ShipDatabase Tests ====================

void testShipDatabaseLoadFromDirectory() {
    std::cout << "\n=== ShipDatabase Load From Directory ===" << std::endl;
    
    data::ShipDatabase db;
    int count = db.loadFromDirectory("../data");
    
    // If data/ isn't at ../data (depends on CWD), try other paths
    if (count == 0) {
        count = db.loadFromDirectory("data");
    }
    if (count == 0) {
        count = db.loadFromDirectory("../../data");
    }
    
    assertTrue(count > 0, "Loaded at least 1 ship from data directory");
    assertTrue(db.getShipCount() > 0, "Ship count > 0");
}

void testShipDatabaseGetShip() {
    std::cout << "\n=== ShipDatabase Get Ship ===" << std::endl;
    
    data::ShipDatabase db;
    // Try multiple paths
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    const data::ShipTemplate* fang = db.getShip("fang");
    if (fang) {
        assertTrue(fang->name == "Fang", "Fang name correct");
        assertTrue(fang->ship_class == "Frigate", "Fang class is Frigate");
        assertTrue(fang->race == "Keldari", "Fang race is Keldari");
        assertTrue(fang->shield_hp > 0.0f, "Fang has shield HP");
        assertTrue(fang->armor_hp > 0.0f, "Fang has armor HP");
        assertTrue(fang->hull_hp > 0.0f, "Fang has hull HP");
        assertTrue(fang->cpu > 0.0f, "Fang has CPU");
        assertTrue(fang->powergrid > 0.0f, "Fang has powergrid");
        assertTrue(fang->max_velocity > 0.0f, "Fang has velocity");
        assertTrue(fang->scan_resolution > 0.0f, "Fang has scan resolution");
        assertTrue(fang->max_locked_targets > 0, "Fang has max locked targets");
    } else {
        assertTrue(false, "Fang template found in database");
    }
    
    const data::ShipTemplate* missing = db.getShip("nonexistent_ship");
    assertTrue(missing == nullptr, "Nonexistent ship returns nullptr");
}

void testShipDatabaseResistances() {
    std::cout << "\n=== ShipDatabase Resistances ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    const data::ShipTemplate* fang = db.getShip("fang");
    if (fang) {
        // Fang shield: em=0, thermal=20, kinetic=40, explosive=50 (in JSON)
        // Converted to fractions: 0.0, 0.20, 0.40, 0.50
        assertTrue(approxEqual(fang->shield_resists.em, 0.0f), "Shield EM resist = 0%");
        assertTrue(approxEqual(fang->shield_resists.thermal, 0.20f), "Shield thermal resist = 20%");
        assertTrue(approxEqual(fang->shield_resists.kinetic, 0.40f), "Shield kinetic resist = 40%");
        assertTrue(approxEqual(fang->shield_resists.explosive, 0.50f), "Shield explosive resist = 50%");
        
        // Armor: em=60, thermal=35, kinetic=25, explosive=10
        assertTrue(approxEqual(fang->armor_resists.em, 0.60f), "Armor EM resist = 60%");
        assertTrue(approxEqual(fang->armor_resists.thermal, 0.35f), "Armor thermal resist = 35%");
    } else {
        assertTrue(false, "Fang template found for resistance check");
    }
}

void testShipDatabaseGetShipIds() {
    std::cout << "\n=== ShipDatabase Get Ship IDs ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    auto ids = db.getShipIds();
    assertTrue(ids.size() > 0, "getShipIds returns non-empty list");
    
    // Check that 'fang' is in the list
    bool found = false;
    for (const auto& id : ids) {
        if (id == "fang") found = true;
    }
    assertTrue(found, "fang is in ship ID list");
}

void testShipDatabaseCapitalShips() {
    std::cout << "\n=== ShipDatabase Capital Ships ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify capital ships are loaded
    const data::ShipTemplate* solarius = db.getShip("solarius");
    if (solarius) {
        assertTrue(solarius->name == "Solarius", "Solarius name correct");
        assertTrue(solarius->ship_class == "Carrier", "Solarius class is Carrier");
        assertTrue(solarius->race == "Solari", "Solarius race is Solari");
        assertTrue(solarius->hull_hp > 10000.0f, "Solarius has high hull HP");
        assertTrue(solarius->armor_hp > 50000.0f, "Solarius has high armor HP");
    } else {
        assertTrue(false, "Solarius carrier found in database");
    }
    
    // Verify titan is loaded
    const data::ShipTemplate* empyrean = db.getShip("empyrean");
    if (empyrean) {
        assertTrue(empyrean->name == "Empyrean", "Empyrean name correct");
        assertTrue(empyrean->ship_class == "Titan", "Empyrean class is Titan");
        assertTrue(empyrean->hull_hp > 100000.0f, "Empyrean has very high hull HP");
    } else {
        assertTrue(false, "Empyrean titan found in database");
    }
    
    // Verify multiple ship categories loaded
    auto ids = db.getShipIds();
    bool hasCapital = false, hasBattleship = false, hasFrigate = false;
    bool hasTech2Cruiser = false, hasMiningBarge = false;
    bool hasMarauder = false, hasIndustrial = false;
    bool hasInterdictor = false, hasStealthBomber = false;
    for (const auto& id : ids) {
        if (id == "solarius") hasCapital = true;
        if (id == "gale") hasBattleship = true;
        if (id == "fang") hasFrigate = true;
        if (id == "wanderer") hasTech2Cruiser = true;
        if (id == "ironbore") hasMiningBarge = true;
        if (id == "ironheart") hasMarauder = true;
        if (id == "drifthauler") hasIndustrial = true;
        if (id == "gripshard") hasInterdictor = true;
        if (id == "shadowfang") hasStealthBomber = true;
    }
    assertTrue(hasCapital, "Capital ships loaded");
    assertTrue(hasBattleship, "Battleships loaded");
    assertTrue(hasFrigate, "Frigates loaded");
    assertTrue(hasTech2Cruiser, "Tech II cruisers loaded");
    assertTrue(hasMiningBarge, "Mining barges loaded");
    assertTrue(hasMarauder, "Marauder battleships loaded");
    assertTrue(hasIndustrial, "Industrial ships loaded");
    assertTrue(hasInterdictor, "Interdictor destroyers loaded");
    assertTrue(hasStealthBomber, "Stealth Bomber frigates loaded");
    assertTrue(ids.size() >= 50, "At least 50 ship templates loaded");
}

void testShipDatabaseMarauders() {
    std::cout << "\n=== ShipDatabase Marauder Ships ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify all 4 Marauders are loaded
    const data::ShipTemplate* ironheart = db.getShip("ironheart");
    if (ironheart) {
        assertTrue(ironheart->name == "Ironheart", "Ironheart name correct");
        assertTrue(ironheart->ship_class == "Marauder", "Ironheart class is Marauder");
        assertTrue(ironheart->race == "Keldari", "Ironheart race is Keldari");
        assertTrue(ironheart->hull_hp > 8000.0f, "Ironheart has high hull HP");
        assertTrue(ironheart->shield_hp > 10000.0f, "Ironheart has high shield HP");
        assertTrue(ironheart->max_locked_targets >= 10, "Ironheart has 10 locked targets");
    } else {
        assertTrue(false, "Ironheart marauder found in database");
    }
    
    const data::ShipTemplate* monolith = db.getShip("monolith");
    assertTrue(monolith != nullptr, "Monolith marauder found in database");
    if (monolith) {
        assertTrue(monolith->race == "Veyren", "Monolith race is Veyren");
    }
    
    const data::ShipTemplate* majeste = db.getShip("majeste");
    assertTrue(majeste != nullptr, "Majeste marauder found in database");
    if (majeste) {
        assertTrue(majeste->race == "Aurelian", "Majeste race is Aurelian");
    }
    
    const data::ShipTemplate* solarius_prime = db.getShip("solarius_prime");
    assertTrue(solarius_prime != nullptr, "Solarius Prime marauder found in database");
    if (solarius_prime) {
        assertTrue(solarius_prime->race == "Solari", "Solarius Prime race is Solari");
    }
}

void testShipDatabaseInterdictors() {
    std::cout << "\n=== ShipDatabase Interdictor Ships ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify all 4 Interdictors are loaded
    const data::ShipTemplate* gripshard = db.getShip("gripshard");
    if (gripshard) {
        assertTrue(gripshard->name == "Gripshard", "Gripshard name correct");
        assertTrue(gripshard->ship_class == "Interdictor", "Gripshard class is Interdictor");
        assertTrue(gripshard->race == "Keldari", "Gripshard race is Keldari");
        assertTrue(gripshard->hull_hp > 700.0f, "Gripshard has destroyer-class hull HP");
        assertTrue(gripshard->max_locked_targets >= 7, "Gripshard has 7 locked targets");
    } else {
        assertTrue(false, "Gripshard interdictor found in database");
    }
    
    const data::ShipTemplate* nettvar = db.getShip("nettvar");
    assertTrue(nettvar != nullptr, "Nettvar interdictor found in database");
    if (nettvar) {
        assertTrue(nettvar->race == "Veyren", "Nettvar race is Veyren");
    }
    
    const data::ShipTemplate* barricade = db.getShip("barricade");
    assertTrue(barricade != nullptr, "Barricade interdictor found in database");
    if (barricade) {
        assertTrue(barricade->race == "Aurelian", "Barricade race is Aurelian");
    }
    
    const data::ShipTemplate* denouncer = db.getShip("denouncer");
    assertTrue(denouncer != nullptr, "Denouncer interdictor found in database");
    if (denouncer) {
        assertTrue(denouncer->race == "Solari", "Denouncer race is Solari");
    }
}

void testShipDatabaseStealthBombers() {
    std::cout << "\n=== ShipDatabase Stealth Bomber Ships ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify all 4 Stealth Bombers are loaded
    const data::ShipTemplate* shadowfang = db.getShip("shadowfang");
    if (shadowfang) {
        assertTrue(shadowfang->name == "Shadowfang", "Shadowfang name correct");
        assertTrue(shadowfang->ship_class == "Stealth Bomber", "Shadowfang class is Stealth Bomber");
        assertTrue(shadowfang->race == "Keldari", "Shadowfang race is Keldari");
        assertTrue(shadowfang->max_targeting_range >= 45000.0f, "Shadowfang has long targeting range");
    } else {
        assertTrue(false, "Shadowfang stealth bomber found in database");
    }
    
    const data::ShipTemplate* frostbane = db.getShip("frostbane");
    assertTrue(frostbane != nullptr, "Frostbane stealth bomber found in database");
    if (frostbane) {
        assertTrue(frostbane->race == "Veyren", "Frostbane race is Veyren");
    }
    
    const data::ShipTemplate* vengeresse = db.getShip("vengeresse");
    assertTrue(vengeresse != nullptr, "Vengeresse stealth bomber found in database");
    if (vengeresse) {
        assertTrue(vengeresse->race == "Aurelian", "Vengeresse race is Aurelian");
    }
    
    const data::ShipTemplate* sanctifier = db.getShip("sanctifier");
    assertTrue(sanctifier != nullptr, "Sanctifier stealth bomber found in database");
    if (sanctifier) {
        assertTrue(sanctifier->race == "Solari", "Sanctifier race is Solari");
    }
}

void testShipDatabaseSecondHACs() {
    std::cout << "\n=== ShipDatabase Second HAC Variants ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify all 4 second HAC variants are loaded
    const data::ShipTemplate* gunnolf = db.getShip("gunnolf");
    if (gunnolf) {
        assertTrue(gunnolf->name == "Gunnolf", "Gunnolf name correct");
        assertTrue(gunnolf->ship_class == "Heavy Assault Cruiser", "Gunnolf class is HAC");
        assertTrue(gunnolf->race == "Keldari", "Gunnolf race is Keldari");
        assertTrue(gunnolf->max_targeting_range >= 70000.0f, "Gunnolf has long targeting range");
    } else {
        assertTrue(false, "Gunnolf HAC found in database");
    }
    
    const data::ShipTemplate* valdris = db.getShip("valdris");
    if (valdris) {
        assertTrue(valdris->name == "Valdris", "Valdris name correct");
        assertTrue(valdris->ship_class == "Heavy Assault Cruiser", "Valdris class is HAC");
        assertTrue(valdris->race == "Veyren", "Valdris race is Veyren");
        assertTrue(valdris->shield_hp >= 3000.0f, "Valdris has strong shields");
    } else {
        assertTrue(false, "Valdris HAC found in database");
    }
    
    const data::ShipTemplate* cavalier = db.getShip("cavalier");
    if (cavalier) {
        assertTrue(cavalier->name == "Cavalier", "Cavalier name correct");
        assertTrue(cavalier->ship_class == "Heavy Assault Cruiser", "Cavalier class is HAC");
        assertTrue(cavalier->race == "Aurelian", "Cavalier race is Aurelian");
        assertTrue(cavalier->armor_hp >= 2000.0f, "Cavalier has strong armor");
    } else {
        assertTrue(false, "Cavalier HAC found in database");
    }
    
    const data::ShipTemplate* inquisitor = db.getShip("inquisitor");
    if (inquisitor) {
        assertTrue(inquisitor->name == "Inquisitor", "Inquisitor name correct");
        assertTrue(inquisitor->ship_class == "Heavy Assault Cruiser", "Inquisitor class is HAC");
        assertTrue(inquisitor->race == "Solari", "Inquisitor race is Solari");
        assertTrue(inquisitor->armor_hp >= 2500.0f, "Inquisitor has heavy armor");
        assertTrue(inquisitor->capacitor >= 1400.0f, "Inquisitor has strong capacitor");
    } else {
        assertTrue(false, "Inquisitor HAC found in database");
    }
}

// ==================== WormholeDatabase Tests ====================

void testWormholeDatabaseLoad() {
    std::cout << "\n=== WormholeDatabase Load ===" << std::endl;
    
    data::WormholeDatabase db;
    int count = db.loadFromDirectory("../data");
    if (count == 0) count = db.loadFromDirectory("data");
    if (count == 0) count = db.loadFromDirectory("../../data");
    
    assertTrue(db.getClassCount() == 6, "Loaded all 6 wormhole classes (C1-C6)");
    assertTrue(db.getEffectCount() > 0, "Loaded at least 1 wormhole effect");
}

void testWormholeDatabaseGetClass() {
    std::cout << "\n=== WormholeDatabase Get Class ===" << std::endl;
    
    data::WormholeDatabase db;
    if (db.loadFromDirectory("../data") == 0)
        if (db.loadFromDirectory("data") == 0)
            db.loadFromDirectory("../../data");
    
    const data::WormholeClassTemplate* c1 = db.getWormholeClass("c1");
    if (c1) {
        assertTrue(c1->wormhole_class == 1, "C1 wormhole class is 1");
        assertTrue(c1->difficulty == "easy", "C1 difficulty is easy");
        assertTrue(c1->max_ship_class == "Battlecruiser", "C1 max ship is Battlecruiser");
        assertTrue(!c1->dormant_spawns.empty(), "C1 has dormant spawns");
        assertTrue(c1->salvage_value_multiplier > 0.0f, "C1 has salvage multiplier");
    } else {
        assertTrue(false, "C1 wormhole class found");
    }
    
    const data::WormholeClassTemplate* c6 = db.getWormholeClass("c6");
    if (c6) {
        assertTrue(c6->wormhole_class == 6, "C6 wormhole class is 6");
        assertTrue(c6->difficulty == "extreme", "C6 difficulty is extreme");
        assertTrue(c6->blue_loot_isc > c1->blue_loot_isc, "C6 loot > C1 loot");
    } else {
        assertTrue(false, "C6 wormhole class found");
    }
    
    assertTrue(db.getWormholeClass("nonexistent") == nullptr, "Nonexistent class returns nullptr");
}

void testWormholeDatabaseEffects() {
    std::cout << "\n=== WormholeDatabase Effects ===" << std::endl;
    
    data::WormholeDatabase db;
    if (db.loadFromDirectory("../data") == 0)
        if (db.loadFromDirectory("data") == 0)
            db.loadFromDirectory("../../data");
    
    const data::WormholeEffect* magnetar = db.getEffect("magnetar");
    if (magnetar) {
        assertTrue(magnetar->name == "Magnetar", "Magnetar name correct");
        assertTrue(!magnetar->modifiers.empty(), "Magnetar has modifiers");
        auto it = magnetar->modifiers.find("damage_multiplier");
        assertTrue(it != magnetar->modifiers.end(), "Magnetar has damage_multiplier");
        if (it != magnetar->modifiers.end()) {
            assertTrue(approxEqual(it->second, 1.86f), "Magnetar damage_multiplier is 1.86");
        }
    } else {
        assertTrue(false, "Magnetar effect found");
    }
    
    assertTrue(db.getEffect("nonexistent") == nullptr, "Nonexistent effect returns nullptr");
}

void testWormholeDatabaseClassIds() {
    std::cout << "\n=== WormholeDatabase Class IDs ===" << std::endl;
    
    data::WormholeDatabase db;
    if (db.loadFromDirectory("../data") == 0)
        if (db.loadFromDirectory("data") == 0)
            db.loadFromDirectory("../../data");
    
    auto ids = db.getClassIds();
    assertTrue(ids.size() == 6, "getClassIds returns 6 classes");
    
    auto effect_ids = db.getEffectIds();
    assertTrue(effect_ids.size() == 6, "getEffectIds returns 6 effects");
}

// ==================== NpcDatabase Tests ====================

void testNpcDatabaseLoad() {
    std::cout << "\n=== NpcDatabase Load ===" << std::endl;

    data::NpcDatabase npcDb;

    // Try multiple paths (same strategy as ShipDatabase tests)
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    assertTrue(loaded > 0, "NpcDatabase loaded NPCs from directory");
    assertTrue(npcDb.getNpcCount() >= 32, "At least 32 NPC templates loaded");
}

void testNpcDatabaseGetNpc() {
    std::cout << "\n=== NpcDatabase GetNpc ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "venom_syndicate_scout found");
    assertTrue(scout->name == "Venom Syndicate Scout", "NPC name correct");
    assertTrue(scout->type == "frigate", "NPC type correct");
    assertTrue(scout->faction == "Venom Syndicate", "NPC faction correct");
}

void testNpcDatabaseHpValues() {
    std::cout << "\n=== NpcDatabase HP Values ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "Scout found for HP test");
    assertTrue(approxEqual(scout->hull_hp, 300.0f), "Hull HP is 300");
    assertTrue(approxEqual(scout->armor_hp, 250.0f), "Armor HP is 250");
    assertTrue(approxEqual(scout->shield_hp, 350.0f), "Shield HP is 350");
    assertTrue(approxEqual(static_cast<float>(scout->bounty), 12500.0f), "Bounty is 12500");
}

void testNpcDatabaseWeapons() {
    std::cout << "\n=== NpcDatabase Weapons ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "Scout found for weapons test");
    assertTrue(!scout->weapons.empty(), "Scout has weapons");
    assertTrue(scout->weapons[0].type == "small_hybrid", "Weapon type is small_hybrid");
    assertTrue(approxEqual(scout->weapons[0].damage, 28.0f), "Weapon damage is 28");
    assertTrue(scout->weapons[0].damage_type == "kinetic", "Weapon damage type is kinetic");
    assertTrue(approxEqual(scout->weapons[0].rate_of_fire, 4.5f), "Rate of fire is 4.5");
}

void testNpcDatabaseResistances() {
    std::cout << "\n=== NpcDatabase Resistances ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "Scout found for resistances test");

    // Shield: em=0, thermal=60, kinetic=85, explosive=50 -> /100
    assertTrue(approxEqual(scout->shield_resists.em, 0.0f), "Shield EM resist is 0.0");
    assertTrue(approxEqual(scout->shield_resists.thermal, 0.60f), "Shield thermal resist is 0.60");
    assertTrue(approxEqual(scout->shield_resists.kinetic, 0.85f), "Shield kinetic resist is 0.85");
    assertTrue(approxEqual(scout->shield_resists.explosive, 0.50f), "Shield explosive resist is 0.50");

    // Armor: em=10, thermal=35, kinetic=25, explosive=45 -> /100
    assertTrue(approxEqual(scout->armor_resists.em, 0.10f), "Armor EM resist is 0.10");
    assertTrue(approxEqual(scout->armor_resists.kinetic, 0.25f), "Armor kinetic resist is 0.25");
}

void testNpcDatabaseIds() {
    std::cout << "\n=== NpcDatabase IDs ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    auto ids = npcDb.getNpcIds();
    assertTrue(!ids.empty(), "getNpcIds returns non-empty list");
    assertTrue(ids.size() == npcDb.getNpcCount(), "IDs count matches getNpcCount");
}

void testNpcDatabaseNonexistent() {
    std::cout << "\n=== NpcDatabase Nonexistent ===" << std::endl;

    data::NpcDatabase npcDb;
    npcDb.loadFromDirectory("../data");

    const data::NpcTemplate* result = npcDb.getNpc("totally_fake_npc");
    assertTrue(result == nullptr, "Nonexistent NPC returns nullptr");
}

// ==================== Ship Generation Model Data Tests ====================

void testShipModelDataParsed() {
    std::cout << "\n=== ShipDatabase: Model Data Parsed ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    const data::ShipTemplate* fang = db.getShip("fang");
    if (fang) {
        assertTrue(fang->model_data.has_model_data, "Fang has model_data");
        assertTrue(fang->model_data.turret_hardpoints >= 2, "Fang turret hardpoints >= 2");
        assertTrue(fang->model_data.turret_hardpoints <= 3, "Fang turret hardpoints <= 3");
        assertTrue(fang->model_data.engine_count >= 2, "Fang engine count >= 2");
        assertTrue(fang->model_data.engine_count <= 3, "Fang engine count <= 3");
        assertTrue(fang->model_data.generation_seed > 0, "Fang generation seed > 0");
    } else {
        assertTrue(false, "Fang template found for model_data test");
    }
}

void testShipModelDataCapitalShips() {
    std::cout << "\n=== ShipDatabase: Capital Ship Model Data ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    const data::ShipTemplate* empyrean = db.getShip("empyrean");
    if (empyrean) {
        assertTrue(empyrean->model_data.has_model_data, "Empyrean has model_data");
        assertTrue(empyrean->model_data.turret_hardpoints >= 6, "Titan turrets >= 6");
        assertTrue(empyrean->model_data.turret_hardpoints <= 10, "Titan turrets <= 10");
        assertTrue(empyrean->model_data.engine_count >= 8, "Titan engines >= 8");
        assertTrue(empyrean->model_data.engine_count <= 12, "Titan engines <= 12");
    } else {
        assertTrue(false, "Empyrean template found for capital model_data test");
    }

    const data::ShipTemplate* solarius = db.getShip("solarius");
    if (solarius) {
        assertTrue(solarius->model_data.has_model_data, "Solarius has model_data");
        assertTrue(solarius->model_data.drone_bays >= 5, "Carrier drone_bays >= 5");
        assertTrue(solarius->model_data.drone_bays <= 10, "Carrier drone_bays <= 10");
    } else {
        assertTrue(false, "Solarius template found for carrier model_data test");
    }
}

void testShipModelDataAllShipsHaveModelData() {
    std::cout << "\n=== ShipDatabase: All Ships Have Model Data ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    auto ids = db.getShipIds();
    assertTrue(ids.size() >= 90, "At least 90 ships loaded");

    int withModelData = 0;
    for (const auto& id : ids) {
        const data::ShipTemplate* ship = db.getShip(id);
        if (ship && ship->model_data.has_model_data) {
            withModelData++;
        }
    }

    assertTrue(withModelData == static_cast<int>(ids.size()),
               "All ships have model_data (" + std::to_string(withModelData) + "/" + std::to_string(ids.size()) + ")");
}

void testShipModelDataSeedUniqueness() {
    std::cout << "\n=== ShipDatabase: Model Data Seed Uniqueness ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    auto ids = db.getShipIds();
    std::map<int, std::string> seedMap;
    int uniqueSeeds = 0;
    for (const auto& id : ids) {
        const data::ShipTemplate* ship = db.getShip(id);
        if (ship && ship->model_data.has_model_data && ship->model_data.generation_seed > 0) {
            if (seedMap.find(ship->model_data.generation_seed) == seedMap.end()) {
                seedMap[ship->model_data.generation_seed] = id;
                uniqueSeeds++;
            }
        }
    }

    // Seeds should be mostly unique (allow small number of collisions due to hash)
    assertTrue(uniqueSeeds >= static_cast<int>(ids.size()) * 9 / 10,
               "Most seeds are unique (" + std::to_string(uniqueSeeds) + "/" + std::to_string(ids.size()) + ")");
}

void testShipModelDataEngineCountPositive() {
    std::cout << "\n=== ShipDatabase: All Ships Have Engines ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    auto ids = db.getShipIds();
    int allHaveEngines = 0;
    for (const auto& id : ids) {
        const data::ShipTemplate* ship = db.getShip(id);
        if (ship && ship->model_data.has_model_data && ship->model_data.engine_count >= 2) {
            allHaveEngines++;
        }
    }

    assertTrue(allHaveEngines == static_cast<int>(ids.size()),
               "All ships have >= 2 engines (" + std::to_string(allHaveEngines) + "/" + std::to_string(ids.size()) + ")");
}

void testShipModelDataMissingReturnsDefaults() {
    std::cout << "\n=== ShipDatabase: Missing Model Data Returns Defaults ===" << std::endl;

    // A ShipTemplate without model_data block should have defaults
    data::ShipTemplate empty;
    assertTrue(!empty.model_data.has_model_data, "Default template has no model_data");
    assertTrue(empty.model_data.turret_hardpoints == 0, "Default turret hardpoints is 0");
    assertTrue(empty.model_data.launcher_hardpoints == 0, "Default launcher hardpoints is 0");
    assertTrue(empty.model_data.drone_bays == 0, "Default drone bays is 0");
    assertTrue(empty.model_data.engine_count == 2, "Default engine count is 2");
    assertTrue(empty.model_data.generation_seed == 0, "Default generation seed is 0");
}

// ==================== New Component Default Tests ====================

void testWarpProfileDefaults() {
    std::cout << "\n=== WarpProfile Defaults ===" << std::endl;
    components::WarpProfile wp;
    assertTrue(approxEqual(wp.warp_speed, 3.0f), "WarpProfile default warp_speed is 3.0");
    assertTrue(approxEqual(wp.mass_norm, 0.0f), "WarpProfile default mass_norm is 0.0");
    assertTrue(approxEqual(wp.intensity, 0.0f), "WarpProfile default intensity is 0.0");
    assertTrue(approxEqual(wp.comfort_scale, 1.0f), "WarpProfile default comfort_scale is 1.0");
}

void testWarpVisualDefaults() {
    std::cout << "\n=== WarpVisual Defaults ===" << std::endl;
    components::WarpVisual wv;
    assertTrue(approxEqual(wv.distortion_strength, 0.0f), "WarpVisual default distortion_strength is 0.0");
    assertTrue(approxEqual(wv.tunnel_noise_scale, 1.0f), "WarpVisual default tunnel_noise_scale is 1.0");
    assertTrue(approxEqual(wv.vignette_amount, 0.0f), "WarpVisual default vignette_amount is 0.0");
    assertTrue(approxEqual(wv.bloom_strength, 0.0f), "WarpVisual default bloom_strength is 0.0");
    assertTrue(approxEqual(wv.starfield_speed, 1.0f), "WarpVisual default starfield_speed is 1.0");
}

void testWarpEventDefaults() {
    std::cout << "\n=== WarpEvent Defaults ===" << std::endl;
    components::WarpEvent we;
    assertTrue(we.current_event.empty(), "WarpEvent default current_event is empty");
    assertTrue(approxEqual(we.event_timer, 0.0f), "WarpEvent default event_timer is 0.0");
    assertTrue(we.severity == 0, "WarpEvent default severity is 0");
}

void testTacticalProjectionDefaults() {
    std::cout << "\n=== TacticalProjection Defaults ===" << std::endl;
    components::TacticalProjection tp;
    assertTrue(approxEqual(tp.projected_x, 0.0f), "TacticalProjection default projected_x is 0.0");
    assertTrue(approxEqual(tp.projected_y, 0.0f), "TacticalProjection default projected_y is 0.0");
    assertTrue(approxEqual(tp.vertical_offset, 0.0f), "TacticalProjection default vertical_offset is 0.0");
    assertTrue(tp.visible, "TacticalProjection default visible is true");
}

void testPlayerPresenceDefaults() {
    std::cout << "\n=== PlayerPresence Defaults ===" << std::endl;
    components::PlayerPresence pp;
    assertTrue(approxEqual(pp.time_since_last_command, 0.0f), "PlayerPresence default time_since_last_command is 0.0");
    assertTrue(approxEqual(pp.time_since_last_speech, 0.0f), "PlayerPresence default time_since_last_speech is 0.0");
}

void testFactionCultureDefaults() {
    std::cout << "\n=== FactionCulture Defaults ===" << std::endl;
    components::FactionCulture fc;
    assertTrue(fc.faction.empty(), "FactionCulture default faction is empty");
    assertTrue(approxEqual(fc.chatter_frequency_mod, 1.0f), "FactionCulture default chatter_frequency_mod is 1.0");
    assertTrue(approxEqual(fc.formation_tightness_mod, 1.0f), "FactionCulture default formation_tightness_mod is 1.0");
    assertTrue(approxEqual(fc.morale_sensitivity, 1.0f), "FactionCulture default morale_sensitivity is 1.0");
    assertTrue(approxEqual(fc.risk_tolerance, 0.5f), "FactionCulture default risk_tolerance is 0.5");
}

// ==================== New Component Persistence Tests ====================

void testPersistenceWarpProfile() {
    std::cout << "\n=== Persistence: WarpProfile Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("warp_prof1");
    auto* wp = addComp<components::WarpProfile>(e);
    wp->warp_speed = 5.5f;
    wp->mass_norm = 0.8f;
    wp->intensity = 0.6f;
    wp->comfort_scale = 0.7f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with WarpProfile");

    auto* e2 = world2.getEntity("warp_prof1");
    assertTrue(e2 != nullptr, "WarpProfile entity found");
    auto* wp2 = e2->getComponent<components::WarpProfile>();
    assertTrue(wp2 != nullptr, "WarpProfile component present");
    assertTrue(approxEqual(wp2->warp_speed, 5.5f), "warp_speed preserved");
    assertTrue(approxEqual(wp2->mass_norm, 0.8f), "mass_norm preserved");
    assertTrue(approxEqual(wp2->intensity, 0.6f), "intensity preserved");
    assertTrue(approxEqual(wp2->comfort_scale, 0.7f), "comfort_scale preserved");
}

void testPersistenceWarpVisual() {
    std::cout << "\n=== Persistence: WarpVisual Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("warp_vis1");
    auto* wv = addComp<components::WarpVisual>(e);
    wv->distortion_strength = 0.9f;
    wv->tunnel_noise_scale = 2.0f;
    wv->vignette_amount = 0.4f;
    wv->bloom_strength = 0.5f;
    wv->starfield_speed = 3.0f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with WarpVisual");

    auto* e2 = world2.getEntity("warp_vis1");
    assertTrue(e2 != nullptr, "WarpVisual entity found");
    auto* wv2 = e2->getComponent<components::WarpVisual>();
    assertTrue(wv2 != nullptr, "WarpVisual component present");
    assertTrue(approxEqual(wv2->distortion_strength, 0.9f), "distortion_strength preserved");
    assertTrue(approxEqual(wv2->tunnel_noise_scale, 2.0f), "tunnel_noise_scale preserved");
    assertTrue(approxEqual(wv2->vignette_amount, 0.4f), "vignette_amount preserved");
    assertTrue(approxEqual(wv2->bloom_strength, 0.5f), "bloom_strength preserved");
    assertTrue(approxEqual(wv2->starfield_speed, 3.0f), "starfield_speed preserved");
}

void testPersistenceWarpEvent() {
    std::cout << "\n=== Persistence: WarpEvent Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("warp_evt1");
    auto* we = addComp<components::WarpEvent>(e);
    we->current_event = "tunnel_eddy";
    we->event_timer = 4.5f;
    we->severity = 2;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with WarpEvent");

    auto* e2 = world2.getEntity("warp_evt1");
    assertTrue(e2 != nullptr, "WarpEvent entity found");
    auto* we2 = e2->getComponent<components::WarpEvent>();
    assertTrue(we2 != nullptr, "WarpEvent component present");
    assertTrue(we2->current_event == "tunnel_eddy", "current_event preserved");
    assertTrue(approxEqual(we2->event_timer, 4.5f), "event_timer preserved");
    assertTrue(we2->severity == 2, "severity preserved");
}

void testPersistenceTacticalProjection() {
    std::cout << "\n=== Persistence: TacticalProjection Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("tact_proj1");
    auto* tp = addComp<components::TacticalProjection>(e);
    tp->projected_x = 150.0f;
    tp->projected_y = 250.0f;
    tp->vertical_offset = -10.5f;
    tp->visible = false;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with TacticalProjection");

    auto* e2 = world2.getEntity("tact_proj1");
    assertTrue(e2 != nullptr, "TacticalProjection entity found");
    auto* tp2 = e2->getComponent<components::TacticalProjection>();
    assertTrue(tp2 != nullptr, "TacticalProjection component present");
    assertTrue(approxEqual(tp2->projected_x, 150.0f), "projected_x preserved");
    assertTrue(approxEqual(tp2->projected_y, 250.0f), "projected_y preserved");
    assertTrue(approxEqual(tp2->vertical_offset, -10.5f), "vertical_offset preserved");
    assertTrue(!tp2->visible, "visible preserved as false");
}

void testPersistencePlayerPresence() {
    std::cout << "\n=== Persistence: PlayerPresence Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("player_pres1");
    auto* pp = addComp<components::PlayerPresence>(e);
    pp->time_since_last_command = 45.0f;
    pp->time_since_last_speech = 120.0f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with PlayerPresence");

    auto* e2 = world2.getEntity("player_pres1");
    assertTrue(e2 != nullptr, "PlayerPresence entity found");
    auto* pp2 = e2->getComponent<components::PlayerPresence>();
    assertTrue(pp2 != nullptr, "PlayerPresence component present");
    assertTrue(approxEqual(pp2->time_since_last_command, 45.0f), "time_since_last_command preserved");
    assertTrue(approxEqual(pp2->time_since_last_speech, 120.0f), "time_since_last_speech preserved");
}

void testPersistenceFactionCulture() {
    std::cout << "\n=== Persistence: FactionCulture Round-Trip ===" << std::endl;
    ecs::World world;

    auto* e = world.createEntity("fc_solari1");
    auto* fc = addComp<components::FactionCulture>(e);
    fc->faction = "Solari";
    fc->chatter_frequency_mod = 1.2f;
    fc->formation_tightness_mod = 0.9f;
    fc->morale_sensitivity = 1.3f;
    fc->risk_tolerance = 0.3f;

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialized world with FactionCulture");

    auto* e2 = world2.getEntity("fc_solari1");
    assertTrue(e2 != nullptr, "FactionCulture entity found");
    auto* fc2 = e2->getComponent<components::FactionCulture>();
    assertTrue(fc2 != nullptr, "FactionCulture component present");
    assertTrue(fc2->faction == "Solari", "faction preserved");
    assertTrue(approxEqual(fc2->chatter_frequency_mod, 1.2f), "chatter_frequency_mod preserved");
    assertTrue(approxEqual(fc2->formation_tightness_mod, 0.9f), "formation_tightness_mod preserved");
    assertTrue(approxEqual(fc2->morale_sensitivity, 1.3f), "morale_sensitivity preserved");
    assertTrue(approxEqual(fc2->risk_tolerance, 0.3f), "risk_tolerance preserved");
}

// ==================== Mineral Economy Integration Test ====================

void testMineralEconomyEndToEnd() {
    std::cout << "\n=== Mineral Economy End-to-End ===" << std::endl;
    ecs::World world;
    systems::MiningSystem mining(&world);
    systems::RefiningSystem refining(&world);
    systems::MarketSystem market(&world);

    // Create station with refining and market
    auto* station = world.createEntity("econ_station");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "econ_station";
    addComp<components::RefiningFacility>(station);
    refining.installDefaultRecipes("econ_station");

    // Seed NPC market
    int seeded = market.seedNPCOrders("econ_station");
    assertTrue(seeded == 4, "NPC orders seeded");

    // Create miner with laser and inventory
    auto* miner = world.createEntity("econ_miner");
    auto* minerPos = addComp<components::Position>(miner);
    minerPos->x = 0; minerPos->y = 0; minerPos->z = 0;
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 100.0f;
    laser->cycle_time = 1.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 10000.0f;

    // Create deposit
    std::string depositId = mining.createDeposit("Ferrite", 5000.0f, 10.0f, 0.0f, 0.0f);
    assertTrue(!depositId.empty(), "Deposit created");

    // Mine ore
    bool started = mining.startMining("econ_miner", depositId);
    assertTrue(started, "Mining started");

    // Run several cycles
    for (int i = 0; i < 5; ++i) {
        mining.update(1.0f);
    }

    // Check miner has ore
    bool hasOre = false;
    for (const auto& item : inv->items) {
        if (item.name == "Ferrite" && item.quantity > 0) hasOre = true;
    }
    assertTrue(hasOre, "Miner has Ferrite ore after mining");

    // Refine the ore at the station
    int refined = refining.refineOre("econ_miner", "econ_station", "Ferrite", 1);
    assertTrue(refined > 0, "Ore refined successfully");

    // Check that Stellium was produced (Ferrite → Stellium)
    bool hasStellium = false;
    for (const auto& item : inv->items) {
        if (item.name == "Stellium" && item.quantity > 0) hasStellium = true;
    }
    assertTrue(hasStellium, "Miner has Stellium after refining Ferrite");

    // Verify market still has mineral prices
    double tritPrice = market.getLowestSellPrice("econ_station", "mineral_tritanium");
    assertTrue(tritPrice > 0.0, "Stellium still available on market");
}

// ==================== Expanded Ship Class Tests ====================

void testShipGeneratorAllHullClasses() {
    std::cout << "\n=== PCG: ShipGenerator all 20 hull classes ===" << std::endl;
    using atlas::pcg::HullClass;
    using atlas::pcg::ShipGenerator;
    using atlas::pcg::PCGContext;

    HullClass allClasses[] = {
        HullClass::Frigate, HullClass::Destroyer, HullClass::Cruiser,
        HullClass::Battlecruiser, HullClass::Battleship, HullClass::Capital,
        HullClass::Interceptor, HullClass::CovertOps, HullClass::AssaultFrigate,
        HullClass::StealthBomber, HullClass::Logistics, HullClass::Recon,
        HullClass::CommandShip, HullClass::Marauder, HullClass::Industrial,
        HullClass::MiningBarge, HullClass::Exhumer, HullClass::Carrier,
        HullClass::Dreadnought, HullClass::Titan,
    };

    for (int i = 0; i < 20; ++i) {
        PCGContext ctx{ static_cast<uint64_t>(i * 1337 + 42), 1 };
        auto ship = ShipGenerator::generate(ctx, allClasses[i]);
        assertTrue(ship.valid, ("Valid ship for " + ShipGenerator::hullClassName(allClasses[i])).c_str());
        assertTrue(ship.hullClass == allClasses[i],
                   ("Hull class matches for " + ShipGenerator::hullClassName(allClasses[i])).c_str());
        assertTrue(ship.mass > 0.0f, ("Positive mass for " + ShipGenerator::hullClassName(allClasses[i])).c_str());
        assertTrue(ship.thrust > 0.0f, ("Positive thrust for " + ShipGenerator::hullClassName(allClasses[i])).c_str());
    }
}

void testShipGeneratorTechLevels() {
    std::cout << "\n=== PCG: ShipGenerator tech levels ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 555, 1 };
    auto frigate = ShipGenerator::generate(ctx, HullClass::Frigate);
    assertTrue(frigate.techLevel == 1, "Frigate is Tech I");

    auto interceptor = ShipGenerator::generate(ctx, HullClass::Interceptor);
    assertTrue(interceptor.techLevel == 2, "Interceptor is Tech II");

    auto marauder = ShipGenerator::generate(ctx, HullClass::Marauder);
    assertTrue(marauder.techLevel == 2, "Marauder is Tech II");

    auto industrial = ShipGenerator::generate(ctx, HullClass::Industrial);
    assertTrue(industrial.techLevel == 1, "Industrial is Tech I");

    auto exhumer = ShipGenerator::generate(ctx, HullClass::Exhumer);
    assertTrue(exhumer.techLevel == 2, "Exhumer is Tech II");

    auto titan = ShipGenerator::generate(ctx, HullClass::Titan);
    assertTrue(titan.techLevel == 1, "Titan is Tech I");
}

void testShipGeneratorCargoCapacity() {
    std::cout << "\n=== PCG: ShipGenerator cargo capacity ===" << std::endl;
    using namespace atlas::pcg;

    // Industrials should have much larger cargo than combat ships.
    float industrialCargoSum = 0.0f;
    float frigateCargoSum = 0.0f;
    for (int i = 0; i < 20; ++i) {
        PCGContext ctx{ static_cast<uint64_t>(i * 99 + 7), 1 };
        auto ind = ShipGenerator::generate(ctx, HullClass::Industrial);
        industrialCargoSum += static_cast<float>(ind.cargoCapacity);
        auto frig = ShipGenerator::generate(ctx, HullClass::Frigate);
        frigateCargoSum += static_cast<float>(frig.cargoCapacity);
    }
    assertTrue(industrialCargoSum > frigateCargoSum * 5.0f,
               "Industrials have much larger cargo than frigates");
}

void testShipGeneratorXLargeWeapons() {
    std::cout << "\n=== PCG: ShipGenerator XLarge weapons ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 777, 1 };
    auto dread = ShipGenerator::generate(ctx, HullClass::Dreadnought);
    assertTrue(dread.maxWeaponSize == WeaponSize::XLarge, "Dreadnought has XLarge weapons");

    auto titan = ShipGenerator::generate(ctx, HullClass::Titan);
    assertTrue(titan.maxWeaponSize == WeaponSize::XLarge, "Titan has XLarge weapons");

    auto frigate = ShipGenerator::generate(ctx, HullClass::Frigate);
    assertTrue(frigate.maxWeaponSize == WeaponSize::Small, "Frigate has Small weapons");
}

void testShipGeneratorStealthBomberLaunchers() {
    std::cout << "\n=== PCG: ShipGenerator stealth bomber launchers ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 888, 1 };
    auto bomber = ShipGenerator::generate(ctx, HullClass::StealthBomber);
    assertTrue(bomber.launcherSlots >= 3, "StealthBomber has 3+ launcher slots");
    assertTrue(bomber.maxWeaponSize == WeaponSize::Large, "StealthBomber uses Large launchers");
}

void testShipGeneratorCarrierNoDPS() {
    std::cout << "\n=== PCG: ShipGenerator carrier has no turrets ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 999, 1 };
    auto carrier = ShipGenerator::generate(ctx, HullClass::Carrier);
    assertTrue(carrier.turretSlots == 0, "Carrier has 0 turret slots");
    assertTrue(carrier.launcherSlots == 0, "Carrier has 0 launcher slots");
    assertTrue(carrier.droneBay >= 200, "Carrier has large drone bay (200+)");
}

void testShipGeneratorHullClassNames() {
    std::cout << "\n=== PCG: ShipGenerator all hull class names ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(ShipGenerator::hullClassName(HullClass::Interceptor) == "Interceptor", "Interceptor name");
    assertTrue(ShipGenerator::hullClassName(HullClass::CovertOps) == "CovertOps", "CovertOps name");
    assertTrue(ShipGenerator::hullClassName(HullClass::AssaultFrigate) == "AssaultFrigate", "AssaultFrigate name");
    assertTrue(ShipGenerator::hullClassName(HullClass::StealthBomber) == "StealthBomber", "StealthBomber name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Logistics) == "Logistics", "Logistics name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Recon) == "Recon", "Recon name");
    assertTrue(ShipGenerator::hullClassName(HullClass::CommandShip) == "CommandShip", "CommandShip name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Marauder) == "Marauder", "Marauder name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Industrial) == "Industrial", "Industrial name");
    assertTrue(ShipGenerator::hullClassName(HullClass::MiningBarge) == "MiningBarge", "MiningBarge name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Exhumer) == "Exhumer", "Exhumer name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Carrier) == "Carrier", "Carrier name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Dreadnought) == "Dreadnought", "Dreadnought name");
    assertTrue(ShipGenerator::hullClassName(HullClass::Titan) == "Titan", "Titan name");
}

void testBaseHullClassMapping() {
    std::cout << "\n=== PCG: baseHullClass mapping ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(baseHullClass(HullClass::Interceptor) == HullClass::Frigate, "Interceptor base is Frigate");
    assertTrue(baseHullClass(HullClass::CovertOps) == HullClass::Frigate, "CovertOps base is Frigate");
    assertTrue(baseHullClass(HullClass::AssaultFrigate) == HullClass::Frigate, "AssaultFrigate base is Frigate");
    assertTrue(baseHullClass(HullClass::StealthBomber) == HullClass::Frigate, "StealthBomber base is Frigate");
    assertTrue(baseHullClass(HullClass::Logistics) == HullClass::Cruiser, "Logistics base is Cruiser");
    assertTrue(baseHullClass(HullClass::Recon) == HullClass::Cruiser, "Recon base is Cruiser");
    assertTrue(baseHullClass(HullClass::CommandShip) == HullClass::Battlecruiser, "CommandShip base is Battlecruiser");
    assertTrue(baseHullClass(HullClass::Marauder) == HullClass::Battleship, "Marauder base is Battleship");
    assertTrue(baseHullClass(HullClass::Industrial) == HullClass::Cruiser, "Industrial base is Cruiser");
    assertTrue(baseHullClass(HullClass::Carrier) == HullClass::Capital, "Carrier base is Capital");
    assertTrue(baseHullClass(HullClass::Dreadnought) == HullClass::Capital, "Dreadnought base is Capital");
    assertTrue(baseHullClass(HullClass::Titan) == HullClass::Capital, "Titan base is Capital");
    assertTrue(baseHullClass(HullClass::Frigate) == HullClass::Frigate, "Frigate base is Frigate");
}


void run_data_tests() {
    testShipDatabaseLoadFromDirectory();
    testShipDatabaseGetShip();
    testShipDatabaseResistances();
    testShipDatabaseGetShipIds();
    testShipDatabaseCapitalShips();
    testShipDatabaseMarauders();
    testShipDatabaseInterdictors();
    testShipDatabaseStealthBombers();
    testShipDatabaseSecondHACs();
    testWormholeDatabaseLoad();
    testWormholeDatabaseGetClass();
    testWormholeDatabaseEffects();
    testWormholeDatabaseClassIds();
    testNpcDatabaseLoad();
    testNpcDatabaseGetNpc();
    testNpcDatabaseHpValues();
    testNpcDatabaseWeapons();
    testNpcDatabaseResistances();
    testNpcDatabaseIds();
    testNpcDatabaseNonexistent();
    testShipModelDataParsed();
    testShipModelDataCapitalShips();
    testShipModelDataAllShipsHaveModelData();
    testShipModelDataSeedUniqueness();
    testShipModelDataEngineCountPositive();
    testShipModelDataMissingReturnsDefaults();
    testWarpProfileDefaults();
    testWarpVisualDefaults();
    testWarpEventDefaults();
    testTacticalProjectionDefaults();
    testPlayerPresenceDefaults();
    testFactionCultureDefaults();
    testPersistenceWarpProfile();
    testPersistenceWarpVisual();
    testPersistenceWarpEvent();
    testPersistenceTacticalProjection();
    testPersistencePlayerPresence();
    testPersistenceFactionCulture();
    testMineralEconomyEndToEnd();
    testShipGeneratorAllHullClasses();
    testShipGeneratorTechLevels();
    testShipGeneratorCargoCapacity();
    testShipGeneratorXLargeWeapons();
    testShipGeneratorStealthBomberLaunchers();
    testShipGeneratorCarrierNoDPS();
    testShipGeneratorHullClassNames();
    testBaseHullClassMapping();
}
