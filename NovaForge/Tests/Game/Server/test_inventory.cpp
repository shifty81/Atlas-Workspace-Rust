/**
 * Inventory tests
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

// ==================== Skill System Tests ====================

void testSkillTraining() {
    std::cout << "\n=== Skill Training ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    // Queue skill training
    bool queued = skillSys.queueSkillTraining("player1", "gunnery_001",
        "Small Projectile Turret", 1, 60.0f);
    assertTrue(queued, "Skill training queued");

    auto* skillset = player->getComponent<components::SkillSet>();
    assertTrue(skillset->training_queue.size() == 1, "One skill in queue");

    // Partially train
    skillSys.update(30.0f);
    assertTrue(skillSys.getSkillLevel("player1", "gunnery_001") == 0,
               "Skill not yet complete after 30s");

    // Complete training
    skillSys.update(35.0f);
    assertTrue(skillSys.getSkillLevel("player1", "gunnery_001") == 1,
               "Skill trained to level 1 after 65s");
    assertTrue(skillset->training_queue.empty(), "Queue empty after completion");
    assertTrue(skillset->total_sp > 0.0, "SP awarded");
}

void testSkillInstantTrain() {
    std::cout << "\n=== Skill Instant Train ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    bool trained = skillSys.trainSkillInstant("player1", "nav_001",
        "Navigation", 3);
    assertTrue(trained, "Instant train succeeds");
    assertTrue(skillSys.getSkillLevel("player1", "nav_001") == 3,
               "Skill is level 3");
}

void testSkillQueueMultiple() {
    std::cout << "\n=== Skill Queue Multiple ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    skillSys.queueSkillTraining("player1", "skill_a", "Skill A", 1, 10.0f);
    skillSys.queueSkillTraining("player1", "skill_b", "Skill B", 1, 20.0f);

    auto* skillset = player->getComponent<components::SkillSet>();
    assertTrue(skillset->training_queue.size() == 2, "Two skills in queue");

    // Complete first
    skillSys.update(12.0f);
    assertTrue(skillSys.getSkillLevel("player1", "skill_a") == 1, "First skill complete");
    assertTrue(skillset->training_queue.size() == 1, "One skill remaining");

    // Complete second
    skillSys.update(20.0f);
    assertTrue(skillSys.getSkillLevel("player1", "skill_b") == 1, "Second skill complete");
    assertTrue(skillset->training_queue.empty(), "Queue empty");
}

void testSkillInvalidLevel() {
    std::cout << "\n=== Skill Invalid Level ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    bool result = skillSys.queueSkillTraining("player1", "test", "Test", 6, 10.0f);
    assertTrue(!result, "Level 6 rejected (max is 5)");

    result = skillSys.queueSkillTraining("player1", "test", "Test", 0, 10.0f);
    assertTrue(!result, "Level 0 rejected (min is 1)");
}

// ==================== Module System Tests ====================

void testModuleActivation() {
    std::cout << "\n=== Module Activation ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 100.0f;
    cap->capacitor_max = 100.0f;

    // Add a module to high slot
    components::ModuleRack::FittedModule gun;
    gun.module_id = "gun_001";
    gun.name = "125mm Autocannon";
    gun.slot_type = "high";
    gun.slot_index = 0;
    gun.cycle_time = 5.0f;
    gun.capacitor_cost = 10.0f;
    rack->high_slots.push_back(gun);

    // Activate
    bool activated = modSys.activateModule("ship1", "high", 0);
    assertTrue(activated, "Module activated");
    assertTrue(rack->high_slots[0].active, "Module is active");

    // Can't activate again
    bool double_activate = modSys.activateModule("ship1", "high", 0);
    assertTrue(!double_activate, "Can't activate already active module");
}

void testModuleCycling() {
    std::cout << "\n=== Module Cycling ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 100.0f;
    cap->capacitor_max = 100.0f;

    components::ModuleRack::FittedModule repper;
    repper.module_id = "rep_001";
    repper.name = "Small Armor Repairer";
    repper.slot_type = "low";
    repper.slot_index = 0;
    repper.cycle_time = 4.0f;
    repper.capacitor_cost = 20.0f;
    rack->low_slots.push_back(repper);

    modSys.activateModule("ship1", "low", 0);

    // Partially cycle
    modSys.update(2.0f);
    assertTrue(approxEqual(rack->low_slots[0].cycle_progress, 0.5f),
               "Half cycle after 2s (4s cycle time)");

    // Complete cycle — should consume cap
    modSys.update(3.0f);
    assertTrue(approxEqual(cap->capacitor, 80.0f, 1.0f),
               "Capacitor consumed after cycle completion");
}

void testModuleCapDrain() {
    std::cout << "\n=== Module Capacitor Drain ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 15.0f;  // Just enough for one cycle
    cap->capacitor_max = 100.0f;

    components::ModuleRack::FittedModule mod;
    mod.cycle_time = 1.0f;
    mod.capacitor_cost = 10.0f;
    rack->high_slots.push_back(mod);

    modSys.activateModule("ship1", "high", 0);

    // First cycle completes
    modSys.update(1.5f);
    assertTrue(rack->high_slots[0].active, "Module still active after first cycle");

    // Second cycle — not enough cap
    modSys.update(1.5f);
    assertTrue(!rack->high_slots[0].active,
               "Module deactivated when capacitor exhausted");
}

void testModuleFittingValidation() {
    std::cout << "\n=== Module Fitting Validation ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->cpu_max = 100.0f;
    shipComp->powergrid_max = 50.0f;
    auto* rack = addComp<components::ModuleRack>(ship);

    // Fit a module within limits
    components::ModuleRack::FittedModule mod1;
    mod1.cpu_usage = 30.0f;
    mod1.powergrid_usage = 20.0f;
    rack->high_slots.push_back(mod1);

    assertTrue(modSys.validateFitting("ship1"), "Fitting within limits");

    // Exceed CPU
    components::ModuleRack::FittedModule mod2;
    mod2.cpu_usage = 80.0f;
    mod2.powergrid_usage = 10.0f;
    rack->mid_slots.push_back(mod2);

    assertTrue(!modSys.validateFitting("ship1"), "Fitting exceeds CPU");
}

void testModuleToggle() {
    std::cout << "\n=== Module Toggle ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 100.0f;

    components::ModuleRack::FittedModule mod;
    mod.capacitor_cost = 5.0f;
    rack->mid_slots.push_back(mod);

    // Toggle on
    modSys.toggleModule("ship1", "mid", 0);
    assertTrue(rack->mid_slots[0].active, "Module toggled on");

    // Toggle off
    modSys.toggleModule("ship1", "mid", 0);
    assertTrue(!rack->mid_slots[0].active, "Module toggled off");
}

// ==================== Inventory System Tests ====================

void testInventoryAddItem() {
    std::cout << "\n=== Inventory Add Item ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 100.0f;

    bool added = invSys.addItem("ship1", "stellium", "Stellium", "ore", 10, 1.0f);
    assertTrue(added, "Item added successfully");
    assertTrue(inv->items.size() == 1, "One item stack in inventory");
    assertTrue(inv->items[0].quantity == 10, "Quantity is 10");
    assertTrue(approxEqual(inv->usedCapacity(), 10.0f), "Used capacity is 10 m3");

    // Stack with existing
    added = invSys.addItem("ship1", "stellium", "Stellium", "ore", 5, 1.0f);
    assertTrue(added, "Stacked item added");
    assertTrue(inv->items.size() == 1, "Still one stack after stacking");
    assertTrue(inv->items[0].quantity == 15, "Quantity is 15 after stacking");
}

void testInventoryCapacityLimit() {
    std::cout << "\n=== Inventory Capacity Limit ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 50.0f;

    bool added = invSys.addItem("ship1", "ore", "Ferrite", "ore", 40, 1.0f);
    assertTrue(added, "40 m3 fits in 50 m3 hold");

    added = invSys.addItem("ship1", "big_item", "Big Module", "module", 1, 20.0f);
    assertTrue(!added, "20 m3 item rejected (only 10 m3 free)");
    assertTrue(approxEqual(inv->freeCapacity(), 10.0f), "Free capacity is 10 m3");
}

void testInventoryRemoveItem() {
    std::cout << "\n=== Inventory Remove Item ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;

    invSys.addItem("ship1", "ammo", "Hybrid Charges", "ammo", 100, 0.01f);

    int removed = invSys.removeItem("ship1", "ammo", 30);
    assertTrue(removed == 30, "Removed 30 units");
    assertTrue(invSys.getItemCount("ship1", "ammo") == 70, "70 remaining");

    removed = invSys.removeItem("ship1", "ammo", 200);
    assertTrue(removed == 70, "Removed only 70 (all available)");
    assertTrue(inv->items.empty(), "Item stack removed when depleted");
}

void testInventoryTransfer() {
    std::cout << "\n=== Inventory Transfer ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship1 = world.createEntity("ship1");
    auto* inv1 = addComp<components::Inventory>(ship1);
    inv1->max_capacity = 400.0f;

    auto* ship2 = world.createEntity("ship2");
    auto* inv2 = addComp<components::Inventory>(ship2);
    inv2->max_capacity = 400.0f;

    invSys.addItem("ship1", "salvage", "Armor Plates", "salvage", 20, 2.0f);

    bool transferred = invSys.transferItem("ship1", "ship2", "salvage", 10);
    assertTrue(transferred, "Transfer succeeded");
    assertTrue(invSys.getItemCount("ship1", "salvage") == 10, "Source has 10 left");
    assertTrue(invSys.getItemCount("ship2", "salvage") == 10, "Destination has 10");

    // Transfer fails if source lacks quantity
    transferred = invSys.transferItem("ship1", "ship2", "nonexistent", 5);
    assertTrue(!transferred, "Transfer fails for missing item");
}

void testInventoryHasItem() {
    std::cout << "\n=== Inventory HasItem ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    addComp<components::Inventory>(ship);

    invSys.addItem("ship1", "dogtag", "Pirate Dogtag", "commodity", 5, 0.1f);

    assertTrue(invSys.hasItem("ship1", "dogtag", 3), "Has 3 dogtags (has 5)");
    assertTrue(invSys.hasItem("ship1", "dogtag", 5), "Has exactly 5 dogtags");
    assertTrue(!invSys.hasItem("ship1", "dogtag", 6), "Does not have 6 dogtags");
    assertTrue(!invSys.hasItem("ship1", "nope"), "Does not have nonexistent item");
}

// ==================== Loot System Tests ====================

void testLootGenerate() {
    std::cout << "\n=== Loot Generate ===" << std::endl;

    ecs::World world;
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(42);

    auto* npc = world.createEntity("pirate1");
    auto* lt = addComp<components::LootTable>(npc);
    lt->isc_drop = 15000.0;

    components::LootTable::LootEntry entry1;
    entry1.item_id     = "scrap_metal";
    entry1.name        = "Scrap Metal";
    entry1.type        = "salvage";
    entry1.drop_chance = 1.0f;  // always drops
    entry1.min_quantity = 1;
    entry1.max_quantity = 5;
    entry1.volume      = 1.0f;
    lt->entries.push_back(entry1);

    components::LootTable::LootEntry entry2;
    entry2.item_id     = "rare_module";
    entry2.name        = "Rare Module";
    entry2.type        = "module";
    entry2.drop_chance = 1.0f;  // always drops for testing
    entry2.min_quantity = 1;
    entry2.max_quantity = 1;
    entry2.volume      = 5.0f;
    lt->entries.push_back(entry2);

    std::string wreck_id = lootSys.generateLoot("pirate1");
    assertTrue(!wreck_id.empty(), "Wreck entity created");

    auto* wreck = world.getEntity(wreck_id);
    assertTrue(wreck != nullptr, "Wreck entity exists in world");

    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    assertTrue(wreck_inv != nullptr, "Wreck has Inventory component");
    assertTrue(wreck_inv->items.size() >= 1, "Wreck has at least one item");

    auto* wreck_lt = wreck->getComponent<components::LootTable>();
    assertTrue(wreck_lt != nullptr, "Wreck has LootTable for Credits");
    assertTrue(approxEqual(static_cast<float>(wreck_lt->isc_drop), 15000.0f),
               "Credits bounty preserved on wreck");
}

void testLootCollect() {
    std::cout << "\n=== Loot Collect ===" << std::endl;

    ecs::World world;
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(42);

    // Create NPC with loot
    auto* npc = world.createEntity("pirate2");
    auto* lt = addComp<components::LootTable>(npc);
    lt->isc_drop = 25000.0;

    components::LootTable::LootEntry entry;
    entry.item_id     = "hybrid_charges";
    entry.name        = "Hybrid Charges";
    entry.type        = "ammo";
    entry.drop_chance = 1.0f;
    entry.min_quantity = 10;
    entry.max_quantity = 10;
    entry.volume      = 0.01f;
    lt->entries.push_back(entry);

    std::string wreck_id = lootSys.generateLoot("pirate2");

    // Create player
    auto* player = world.createEntity("player1");
    auto* player_inv = addComp<components::Inventory>(player);
    player_inv->max_capacity = 400.0f;
    auto* player_comp = addComp<components::Player>(player);
    player_comp->credits = 100000.0;

    bool collected = lootSys.collectLoot(wreck_id, "player1");
    assertTrue(collected, "Loot collected successfully");
    assertTrue(player_inv->items.size() >= 1, "Player received items");
    assertTrue(approxEqual(static_cast<float>(player_comp->credits), 125000.0f),
               "Player Credits increased by bounty");
}

void testLootEmptyTable() {
    std::cout << "\n=== Loot Empty Table ===" << std::endl;

    ecs::World world;
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(99);

    auto* npc = world.createEntity("pirate3");
    auto* lt = addComp<components::LootTable>(npc);
    lt->isc_drop = 0.0;
    // No entries

    std::string wreck_id = lootSys.generateLoot("pirate3");
    assertTrue(!wreck_id.empty(), "Wreck created even with empty loot table");

    auto* wreck = world.getEntity(wreck_id);
    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    assertTrue(wreck_inv->items.empty(), "Wreck has no items from empty table");
}

// ==================== DroneSystem Tests ====================

void testDroneLaunch() {
    std::cout << "\n=== Drone Launch ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    bay->bay_capacity = 25.0f;
    bay->max_bandwidth = 25;

    components::DroneBay::DroneInfo d;
    d.drone_id = "hobgoblin"; d.name = "Hobgoblin I";
    d.type = "light_combat_drone"; d.damage_type = "thermal";
    d.damage = 25.0f; d.rate_of_fire = 3.0f; d.optimal_range = 5000.0f;
    d.hitpoints = 45.0f; d.current_hp = 45.0f; d.bandwidth_use = 5; d.volume = 5.0f;
    bay->stored_drones.push_back(d);

    assertTrue(droneSys.launchDrone("player_ship", "hobgoblin"),
               "Drone launched successfully");
    assertTrue(bay->deployed_drones.size() == 1, "One drone deployed");
    assertTrue(bay->stored_drones.empty(), "Bay empty after launch");
    assertTrue(droneSys.getDeployedCount("player_ship") == 1, "getDeployedCount returns 1");
}

void testDroneRecall() {
    std::cout << "\n=== Drone Recall ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);

    components::DroneBay::DroneInfo d;
    d.drone_id = "warrior"; d.name = "Warrior I";
    d.type = "light_combat_drone"; d.damage_type = "explosive";
    d.damage = 22.0f; d.bandwidth_use = 5; d.volume = 5.0f;
    d.hitpoints = 38.0f; d.current_hp = 38.0f;
    bay->stored_drones.push_back(d);

    droneSys.launchDrone("player_ship", "warrior");
    assertTrue(bay->deployed_drones.size() == 1, "Drone deployed before recall");

    assertTrue(droneSys.recallDrone("player_ship", "warrior"),
               "Drone recalled successfully");
    assertTrue(bay->deployed_drones.empty(), "No deployed drones after recall");
    assertTrue(bay->stored_drones.size() == 1, "Drone back in bay");
}

void testDroneRecallAll() {
    std::cout << "\n=== Drone Recall All ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    bay->max_bandwidth = 25;

    // Add 3 drones
    for (int i = 0; i < 3; ++i) {
        components::DroneBay::DroneInfo d;
        d.drone_id = "drone_" + std::to_string(i);
        d.name = "Test Drone " + std::to_string(i);
        d.type = "light_combat_drone"; d.damage_type = "thermal";
        d.damage = 10.0f; d.bandwidth_use = 5; d.volume = 5.0f;
        d.hitpoints = 40.0f; d.current_hp = 40.0f;
        bay->stored_drones.push_back(d);
    }

    // Launch all 3
    droneSys.launchDrone("player_ship", "drone_0");
    droneSys.launchDrone("player_ship", "drone_1");
    droneSys.launchDrone("player_ship", "drone_2");
    assertTrue(bay->deployed_drones.size() == 3, "3 drones deployed");

    int recalled = droneSys.recallAll("player_ship");
    assertTrue(recalled == 3, "recallAll returns 3");
    assertTrue(bay->deployed_drones.empty(), "No deployed drones after recallAll");
    assertTrue(bay->stored_drones.size() == 3, "All drones back in bay");
}

void testDroneBandwidthLimit() {
    std::cout << "\n=== Drone Bandwidth Limit ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    bay->max_bandwidth = 10;  // Only 10 Mbit/s

    // Add two drones each using 5 bandwidth (exactly max), then a third
    for (int i = 0; i < 3; ++i) {
        components::DroneBay::DroneInfo d;
        d.drone_id = "drone_" + std::to_string(i);
        d.name = "Test Drone " + std::to_string(i);
        d.type = "light_combat_drone"; d.damage_type = "kinetic";
        d.damage = 10.0f; d.bandwidth_use = 5; d.volume = 5.0f;
        d.hitpoints = 40.0f; d.current_hp = 40.0f;
        bay->stored_drones.push_back(d);
    }

    assertTrue(droneSys.launchDrone("player_ship", "drone_0"), "First drone fits bandwidth");
    assertTrue(droneSys.launchDrone("player_ship", "drone_1"), "Second drone fits bandwidth");
    assertTrue(!droneSys.launchDrone("player_ship", "drone_2"),
               "Third drone exceeds bandwidth limit");
    assertTrue(bay->deployed_drones.size() == 2, "Only 2 drones deployed");
    assertTrue(bay->stored_drones.size() == 1, "One drone remains in bay");
}

void testDroneCombatUpdate() {
    std::cout << "\n=== Drone Combat Update ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    // Create player ship with drone
    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    auto* target_comp = addComp<components::Target>(ship);

    components::DroneBay::DroneInfo d;
    d.drone_id = "hobgoblin"; d.name = "Hobgoblin I";
    d.type = "light_combat_drone"; d.damage_type = "thermal";
    d.damage = 25.0f; d.rate_of_fire = 3.0f; d.optimal_range = 5000.0f;
    d.hitpoints = 45.0f; d.current_hp = 45.0f; d.bandwidth_use = 5;
    bay->stored_drones.push_back(d);
    droneSys.launchDrone("player_ship", "hobgoblin");

    // Create target NPC
    auto* npc = world.createEntity("npc_target");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;

    // Lock the target
    target_comp->locked_targets.push_back("npc_target");

    // First tick: drone fires (cooldown == 0 initially)
    droneSys.update(0.1f);
    assertTrue(hp->shield_hp < 100.0f, "Drone dealt damage to shields");
    float shield_after = hp->shield_hp;

    // Second tick: drone is on cooldown, no additional damage
    droneSys.update(0.1f);
    assertTrue(approxEqual(hp->shield_hp, shield_after),
               "Drone on cooldown, no additional damage");

    // Wait out the cooldown (3.0 seconds)
    droneSys.update(3.0f);
    // Cooldown just expired this tick; drone fires on next update
    droneSys.update(0.01f);
    assertTrue(hp->shield_hp < shield_after, "Drone fires again after cooldown");
}

void testDroneDestroyedRemoval() {
    std::cout << "\n=== Drone Destroyed Removal ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);

    components::DroneBay::DroneInfo d;
    d.drone_id = "hobgoblin"; d.name = "Hobgoblin I";
    d.type = "light_combat_drone"; d.damage_type = "thermal";
    d.damage = 25.0f; d.bandwidth_use = 5; d.volume = 5.0f;
    d.hitpoints = 45.0f; d.current_hp = 45.0f;
    bay->stored_drones.push_back(d);
    droneSys.launchDrone("player_ship", "hobgoblin");
    assertTrue(bay->deployed_drones.size() == 1, "Drone deployed");

    // Simulate drone being destroyed
    bay->deployed_drones[0].current_hp = 0.0f;

    droneSys.update(1.0f);
    assertTrue(bay->deployed_drones.empty(), "Destroyed drone removed from deployed list");
}

void testSerializeDeserializeDroneBay() {
    std::cout << "\n=== Serialize/Deserialize DroneBay ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("drone_ship");
    auto* bay = addComp<components::DroneBay>(entity);
    bay->bay_capacity = 50.0f;
    bay->max_bandwidth = 50;

    // Add stored drone
    components::DroneBay::DroneInfo stored;
    stored.drone_id = "ogre"; stored.name = "Ogre I";
    stored.type = "heavy_combat_drone"; stored.damage_type = "thermal";
    stored.damage = 55.0f; stored.rate_of_fire = 6.0f;
    stored.optimal_range = 3000.0f; stored.hitpoints = 120.0f;
    stored.current_hp = 120.0f; stored.bandwidth_use = 25; stored.volume = 25.0f;
    bay->stored_drones.push_back(stored);

    // Add deployed drone
    components::DroneBay::DroneInfo deployed;
    deployed.drone_id = "hobgoblin"; deployed.name = "Hobgoblin I";
    deployed.type = "light_combat_drone"; deployed.damage_type = "thermal";
    deployed.damage = 25.0f; deployed.rate_of_fire = 3.0f;
    deployed.optimal_range = 5000.0f; deployed.hitpoints = 45.0f;
    deployed.current_hp = 30.0f; deployed.bandwidth_use = 5; deployed.volume = 5.0f;
    bay->deployed_drones.push_back(deployed);

    // Serialize
    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    // Deserialize into new world
    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "DroneBay deserialization succeeds");

    auto* e2 = world2.getEntity("drone_ship");
    assertTrue(e2 != nullptr, "Entity recreated");

    auto* bay2 = e2->getComponent<components::DroneBay>();
    assertTrue(bay2 != nullptr, "DroneBay component recreated");
    assertTrue(approxEqual(bay2->bay_capacity, 50.0f), "bay_capacity preserved");
    assertTrue(bay2->max_bandwidth == 50, "max_bandwidth preserved");
    assertTrue(bay2->stored_drones.size() == 1, "One stored drone");
    assertTrue(bay2->stored_drones[0].drone_id == "ogre", "Stored drone id preserved");
    assertTrue(approxEqual(bay2->stored_drones[0].damage, 55.0f), "Stored drone damage preserved");
    assertTrue(bay2->deployed_drones.size() == 1, "One deployed drone");
    assertTrue(bay2->deployed_drones[0].drone_id == "hobgoblin", "Deployed drone id preserved");
    assertTrue(approxEqual(bay2->deployed_drones[0].current_hp, 30.0f), "Deployed drone current_hp preserved");
}

// ==================== Insurance System Tests ====================

void testInsurancePurchase() {
    std::cout << "\n=== Insurance Purchase ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 1000000.0;

    assertTrue(insSys.purchaseInsurance("player_ship", "basic", 500000.0),
               "Basic insurance purchased");
    auto* policy = ship->getComponent<components::InsurancePolicy>();
    assertTrue(policy != nullptr, "InsurancePolicy component created");
    assertTrue(policy->tier == "basic", "Policy tier is basic");
    assertTrue(approxEqual(static_cast<float>(policy->coverage_fraction), 0.5f), "Basic coverage is 50%");
    assertTrue(approxEqual(static_cast<float>(policy->payout_value), 250000.0f), "Payout is 50% of ship value");
    assertTrue(player->credits < 1000000.0, "Premium deducted from Credits");
    assertTrue(policy->active, "Policy is active");
}

void testInsuranceClaim() {
    std::cout << "\n=== Insurance Claim ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 1000000.0;

    insSys.purchaseInsurance("player_ship", "standard", 500000.0);
    double isk_after_purchase = player->credits;

    double payout = insSys.claimInsurance("player_ship");
    assertTrue(payout > 0.0, "Claim returns positive payout");
    assertTrue(approxEqual(static_cast<float>(payout), 350000.0f), "Standard pays 70% of ship value");
    assertTrue(approxEqual(static_cast<float>(player->credits), static_cast<float>(isk_after_purchase + payout)),
               "Credits increased by payout");

    auto* policy = ship->getComponent<components::InsurancePolicy>();
    assertTrue(policy->claimed, "Policy marked as claimed");

    double second_claim = insSys.claimInsurance("player_ship");
    assertTrue(approxEqual(static_cast<float>(second_claim), 0.0f), "Double claim returns 0");
}

void testInsurancePlatinum() {
    std::cout << "\n=== Insurance Platinum ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 1000000.0;

    assertTrue(insSys.purchaseInsurance("player_ship", "platinum", 500000.0),
               "Platinum insurance purchased");
    auto* policy = ship->getComponent<components::InsurancePolicy>();
    assertTrue(approxEqual(static_cast<float>(policy->coverage_fraction), 1.0f), "Platinum coverage is 100%");
    assertTrue(approxEqual(static_cast<float>(policy->payout_value), 500000.0f), "Platinum payout is full value");
}

void testInsuranceExpiry() {
    std::cout << "\n=== Insurance Expiry ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 1000000.0;

    insSys.purchaseInsurance("player_ship", "basic", 500000.0);
    auto* policy = ship->getComponent<components::InsurancePolicy>();
    policy->duration_remaining = 10.0f; // 10 seconds

    insSys.update(5.0f);
    assertTrue(policy->active, "Policy still active at 5s");
    assertTrue(insSys.hasActivePolicy("player_ship"), "hasActivePolicy returns true");

    insSys.update(6.0f);
    assertTrue(!policy->active, "Policy expired after 11s");
    assertTrue(!insSys.hasActivePolicy("player_ship"), "hasActivePolicy returns false after expiry");
}

void testInsuranceInsufficientFunds() {
    std::cout << "\n=== Insurance Insufficient Funds ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->credits = 100.0; // Not enough

    assertTrue(!insSys.purchaseInsurance("player_ship", "basic", 500000.0),
               "Insurance rejected with insufficient funds");
    assertTrue(ship->getComponent<components::InsurancePolicy>() == nullptr,
               "No policy created on failure");
}

// ==================== BountySystem Tests ====================

void testBountyProcessKill() {
    std::cout << "\n=== Bounty Process Kill ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    auto* player = world.createEntity("player_1");
    auto* pc = addComp<components::Player>(player);
    pc->credits = 100000.0;
    
    double bounty = bountySys.processKill("player_1", "npc_pirate_1", "Venom Scout", 12500.0, "Venom Syndicate");
    assertTrue(approxEqual(static_cast<float>(bounty), 12500.0f), "Bounty returned correctly");
    assertTrue(approxEqual(static_cast<float>(pc->credits), 112500.0f), "Credits increased by bounty");
    assertTrue(bountySys.getTotalKills("player_1") == 1, "Kill count is 1");
    assertTrue(approxEqual(static_cast<float>(bountySys.getTotalBounty("player_1")), 12500.0f), "Total bounty correct");
}

void testBountyMultipleKills() {
    std::cout << "\n=== Bounty Multiple Kills ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    auto* player = world.createEntity("player_1");
    auto* pc = addComp<components::Player>(player);
    pc->credits = 0.0;
    
    bountySys.processKill("player_1", "npc_1", "Scout", 10000.0);
    bountySys.processKill("player_1", "npc_2", "Cruiser", 50000.0);
    bountySys.processKill("player_1", "npc_3", "Battleship", 150000.0);
    
    assertTrue(bountySys.getTotalKills("player_1") == 3, "3 kills recorded");
    assertTrue(approxEqual(static_cast<float>(bountySys.getTotalBounty("player_1")), 210000.0f), "Total bounty is 210K");
    assertTrue(approxEqual(static_cast<float>(pc->credits), 210000.0f), "Credits matches total bounty");
}

void testBountyLedgerRecordLimit() {
    std::cout << "\n=== Bounty Ledger Record Limit ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    auto* player = world.createEntity("player_1");
    addComp<components::Player>(player);
    
    for (int i = 0; i < 60; ++i) {
        bountySys.processKill("player_1", "npc_" + std::to_string(i), "NPC " + std::to_string(i), 1000.0);
    }
    
    auto* ledger = player->getComponent<components::BountyLedger>();
    assertTrue(ledger != nullptr, "Ledger exists");
    assertTrue(static_cast<int>(ledger->recent_kills.size()) <= components::BountyLedger::MAX_RECENT,
               "Recent kills capped at MAX_RECENT");
    assertTrue(ledger->total_kills == 60, "Total kills tracks all 60");
}

void testBountyNonexistentPlayer() {
    std::cout << "\n=== Bounty Nonexistent Player ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    double bounty = bountySys.processKill("fake_player", "npc_1", "Scout", 10000.0);
    assertTrue(approxEqual(static_cast<float>(bounty), 0.0f), "No bounty for nonexistent player");
    assertTrue(bountySys.getTotalKills("fake_player") == 0, "Zero kills for nonexistent");
    assertTrue(approxEqual(static_cast<float>(bountySys.getTotalBounty("fake_player")), 0.0f), "Zero bounty for nonexistent");
}

// ==================== Ship Fitting System Tests ====================

void testShipFittingFitModule() {
    std::cout << "\n=== Ship Fitting: Fit Module ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 100.0f;
    ship->powergrid_max = 50.0f;
    addComp<components::ModuleRack>(ship_entity);

    bool ok = fittingSys.fitModule("ship1", "mod1", "Light Autocannon", "high", 10.0f, 5.0f);
    assertTrue(ok, "Fitting a high-slot module succeeds");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 1, "One high-slot module fitted");
}

void testShipFittingSlotLimit() {
    std::cout << "\n=== Ship Fitting: Slot Limit ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 500.0f;
    ship->powergrid_max = 500.0f;
    addComp<components::ModuleRack>(ship_entity);

    // Frigate has 3 high slots
    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 5.0f, 5.0f);
    fittingSys.fitModule("ship1", "h2", "Gun 2", "high", 5.0f, 5.0f);
    fittingSys.fitModule("ship1", "h3", "Gun 3", "high", 5.0f, 5.0f);
    bool ok = fittingSys.fitModule("ship1", "h4", "Gun 4", "high", 5.0f, 5.0f);
    assertTrue(!ok, "Cannot fit more than 3 high-slot modules on Frigate");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 3, "Still 3 high-slot modules");
}

void testShipFittingCPUOverflow() {
    std::cout << "\n=== Ship Fitting: CPU Overflow ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Cruiser";
    ship->cpu_max = 30.0f;
    ship->powergrid_max = 500.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 15.0f, 5.0f);
    fittingSys.fitModule("ship1", "h2", "Gun 2", "high", 15.0f, 5.0f);
    bool ok = fittingSys.fitModule("ship1", "h3", "Gun 3", "high", 15.0f, 5.0f);
    assertTrue(!ok, "Cannot exceed CPU budget");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 2, "Still 2 modules");
}

void testShipFittingRemoveModule() {
    std::cout << "\n=== Ship Fitting: Remove Module ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 100.0f;
    ship->powergrid_max = 50.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 10.0f, 5.0f);
    fittingSys.fitModule("ship1", "h2", "Gun 2", "high", 10.0f, 5.0f);
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 2, "2 modules before remove");

    bool ok = fittingSys.removeModule("ship1", "high", 0);
    assertTrue(ok, "Remove succeeds");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 1, "1 module after remove");
}

void testShipFittingValidate() {
    std::cout << "\n=== Ship Fitting: Validate ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 100.0f;
    ship->powergrid_max = 50.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 10.0f, 5.0f);
    assertTrue(fittingSys.validateFitting("ship1"), "Valid fitting within budget");
}

void testShipFittingSlotCapacity() {
    std::cout << "\n=== Ship Fitting: Slot Capacity Lookup ===" << std::endl;
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Frigate", "high") == 3,
               "Frigate has 3 high slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Frigate", "mid") == 3,
               "Frigate has 3 mid slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Frigate", "low") == 2,
               "Frigate has 2 low slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Battleship", "high") == 7,
               "Battleship has 7 high slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Cruiser", "mid") == 4,
               "Cruiser has 4 mid slots");
}

void testShipFittingMidLowSlots() {
    std::cout << "\n=== Ship Fitting: Mid and Low Slots ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Cruiser";
    ship->cpu_max = 500.0f;
    ship->powergrid_max = 500.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "m1", "Shield Booster", "mid", 10.0f, 10.0f);
    fittingSys.fitModule("ship1", "l1", "Armor Plate", "low", 10.0f, 10.0f);
    assertTrue(fittingSys.getFittedCount("ship1", "mid") == 1, "1 mid-slot module fitted");
    assertTrue(fittingSys.getFittedCount("ship1", "low") == 1, "1 low-slot module fitted");
}

// ==================== Rig System Tests ====================

void testRigLoadoutDefaults() {
    std::cout << "\n=== Rig Loadout Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig1");
    auto* loadout = addComp<components::RigLoadout>(e);
    assertTrue(loadout->rack_width == 2, "Default rack width 2");
    assertTrue(loadout->rack_height == 2, "Default rack height 2");
    assertTrue(loadout->max_slots() == 4, "Default max slots 4");
}

void testRigInstallModule() {
    std::cout << "\n=== Rig Install Module ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig2");
    addComp<components::RigLoadout>(e);
    auto* mod = world.createEntity("mod1");
    addComp<components::RigModule>(mod);

    systems::RigSystem sys(&world);
    assertTrue(sys.installModule("rig2", "mod1"), "Install module succeeds");
    assertTrue(sys.getInstalledCount("rig2") == 1, "One module installed");
}

void testRigModuleFull() {
    std::cout << "\n=== Rig Module Full ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig3");
    auto* loadout = addComp<components::RigLoadout>(e);
    loadout->rack_width = 1;
    loadout->rack_height = 1;

    systems::RigSystem sys(&world);
    auto* m1 = world.createEntity("m1");
    addComp<components::RigModule>(m1);
    assertTrue(sys.installModule("rig3", "m1"), "First install succeeds");
    assertTrue(!sys.installModule("rig3", "m2"), "Second install fails (full)");
}

void testRigDerivedStats() {
    std::cout << "\n=== Rig Derived Stats ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig4");
    addComp<components::RigLoadout>(e);
    auto* mod = world.createEntity("mod_ls");
    auto* rm = addComp<components::RigModule>(mod);
    rm->type = components::RigModule::ModuleType::LifeSupport;
    rm->tier = 2;
    rm->efficiency = 1.0f;

    systems::RigSystem sys(&world);
    sys.installModule("rig4", "mod_ls");
    sys.update(0.0f);

    auto* loadout = e->getComponent<components::RigLoadout>();
    assertTrue(approxEqual(loadout->total_oxygen, 200.0f), "LifeSupport tier 2 = 200 oxygen");
}

void testRigRemoveModule() {
    std::cout << "\n=== Rig Remove Module ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig5");
    addComp<components::RigLoadout>(e);

    systems::RigSystem sys(&world);
    auto* m1 = world.createEntity("rm1");
    addComp<components::RigModule>(m1);
    sys.installModule("rig5", "rm1");
    assertTrue(sys.getInstalledCount("rig5") == 1, "One module before remove");
    assertTrue(sys.removeModule("rig5", "rm1"), "Remove succeeds");
    assertTrue(sys.getInstalledCount("rig5") == 0, "Zero modules after remove");
}

// ==================== Salvage Exploration Tests ====================

void testSalvageSiteDefaults() {
    std::cout << "\n=== Salvage Site Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site1");
    auto* site = addComp<components::SalvageSite>(e);
    assertTrue(site->discovered_nodes == 0, "No discovered nodes");
    assertTrue(site->looted_nodes == 0, "No looted nodes");
    assertTrue(site->total_loot_nodes == 0, "No total nodes");
}

void testSalvageDiscover() {
    std::cout << "\n=== Salvage Discover ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site2");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 5;

    systems::SalvageExplorationSystem sys(&world);
    bool result = sys.discoverNode("site2");
    assertTrue(result, "Discover succeeded");
    assertTrue(sys.getDiscoveredNodes("site2") == 1, "Discovered 1 node");
}

void testSalvageLoot() {
    std::cout << "\n=== Salvage Loot ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site3");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 5;

    systems::SalvageExplorationSystem sys(&world);
    sys.discoverNode("site3");
    bool result = sys.lootNode("site3");
    assertTrue(result, "Loot succeeded");
    assertTrue(site->looted_nodes == 1, "Looted 1 node");

    // Can't loot more than discovered
    bool result2 = sys.lootNode("site3");
    assertTrue(!result2, "Can't loot undiscovered");
}

void testSalvageFullyLooted() {
    std::cout << "\n=== Salvage Fully Looted ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site4");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 2;

    systems::SalvageExplorationSystem sys(&world);
    sys.discoverNode("site4");
    sys.discoverNode("site4");
    sys.lootNode("site4");
    sys.lootNode("site4");
    assertTrue(sys.isFullyLooted("site4"), "Fully looted");
    assertTrue(sys.getRemainingNodes("site4") == 0, "0 remaining");
}

void testSalvageTrinkets() {
    std::cout << "\n=== Salvage Trinkets ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site5");
    addComp<components::SalvageSite>(e);

    systems::SalvageExplorationSystem sys(&world);
    int count = sys.generateTrinkets("site5", 42);
    assertTrue(count >= 0 && count <= 5, "Trinket count 0-5");
}

// ==================== FPS Inventory System Tests ====================

void testFPSInventoryCreate() {
    std::cout << "\n=== FPS Inventory Create ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    assertTrue(sys.createInventory("p1", 8), "Create inventory succeeds");
    assertTrue(!sys.createInventory("p1", 8), "Duplicate create fails");
    assertTrue(sys.getMaxSlots("p1") == 8, "Max slots correct");
    assertTrue(sys.getItemCount("p1") == 0, "Starts empty");
}

void testFPSInventoryAddRemove() {
    std::cout << "\n=== FPS Inventory Add/Remove ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 4);

    assertTrue(sys.addItem("p1", "item_pistol", "Pistol"), "Add item succeeds");
    assertTrue(sys.hasItem("p1", "item_pistol"), "Has item");
    assertTrue(sys.getItemCount("p1") == 1, "1 item");

    assertTrue(sys.removeItem("p1", "item_pistol"), "Remove succeeds");
    assertTrue(!sys.hasItem("p1", "item_pistol"), "Item removed");
    assertTrue(sys.getItemCount("p1") == 0, "0 items");
}

void testFPSInventoryStacking() {
    std::cout << "\n=== FPS Inventory Stacking ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 4);

    sys.addItem("p1", "ammo_9mm", "9mm Ammo", 10);
    sys.addItem("p1", "ammo_9mm", "9mm Ammo", 5);

    assertTrue(sys.getItemCount("p1") == 1, "Stacked into one slot");
    assertTrue(sys.hasItem("p1", "ammo_9mm"), "Has stacked item");
}

void testFPSInventoryFull() {
    std::cout << "\n=== FPS Inventory Full ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 2);

    sys.addItem("p1", "item_a", "Item A");
    sys.addItem("p1", "item_b", "Item B");
    assertTrue(sys.isInventoryFull("p1"), "Inventory full");
    assertTrue(!sys.addItem("p1", "item_c", "Item C"), "Can't add when full");
}

void testFPSInventoryEquipWeapon() {
    std::cout << "\n=== FPS Inventory Equip Weapon ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 8);
    sys.addItem("p1", "wpn_pistol", "Pistol");

    assertTrue(sys.equipWeapon("p1", "wpn_pistol"), "Equip weapon succeeds");
    assertTrue(sys.getEquippedWeapon("p1") == "wpn_pistol", "Weapon equipped");

    assertTrue(sys.unequipWeapon("p1"), "Unequip succeeds");
    assertTrue(sys.getEquippedWeapon("p1").empty(), "No weapon equipped");
}

void testFPSInventoryEquipWeaponNotInInventory() {
    std::cout << "\n=== FPS Inventory Equip Not In Inventory ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 8);

    assertTrue(!sys.equipWeapon("p1", "wpn_ghost"), "Can't equip missing item");
}

void testFPSInventoryEquipTool() {
    std::cout << "\n=== FPS Inventory Equip Tool ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);

    sys.createInventory("p1", 8);
    sys.addItem("p1", "tool_repair", "Repair Tool");

    assertTrue(sys.equipTool("p1", "tool_repair"), "Equip tool succeeds");
    assertTrue(sys.getEquippedTool("p1") == "tool_repair", "Tool equipped");

    assertTrue(sys.unequipTool("p1"), "Unequip tool succeeds");
    assertTrue(sys.getEquippedTool("p1").empty(), "No tool equipped");
}

void testFPSInventoryUseConsumableOxygen() {
    std::cout << "\n=== FPS Inventory Use Consumable Oxygen ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    auto* charEnt = world.getEntity("fpschar_p1");
    auto needs = std::make_unique<components::SurvivalNeeds>();
    needs->oxygen = 30.0f;
    charEnt->addComponent(std::move(needs));

    sys.createInventory("p1", 8);
    sys.addItem("p1", "oxygen_canister", "Oxygen Canister");

    assertTrue(sys.useConsumable("p1", "oxygen_canister"), "Oxygen consumed");
    auto* n = charEnt->getComponent<components::SurvivalNeeds>();
    assertTrue(n->oxygen > 30.0f, "Oxygen increased");
    assertTrue(!sys.hasItem("p1", "oxygen_canister"), "Item consumed");
}

void testFPSInventoryUseConsumableFood() {
    std::cout << "\n=== FPS Inventory Use Consumable Food ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    auto* charEnt = world.getEntity("fpschar_p1");
    auto needs = std::make_unique<components::SurvivalNeeds>();
    needs->hunger = 60.0f;
    charEnt->addComponent(std::move(needs));

    sys.createInventory("p1", 8);
    sys.addItem("p1", "food_ration", "Ration");

    assertTrue(sys.useConsumable("p1", "food_ration"), "Food consumed");
    auto* n = charEnt->getComponent<components::SurvivalNeeds>();
    assertTrue(n->hunger < 60.0f, "Hunger reduced");
}

void testFPSInventoryUseConsumableMedkit() {
    std::cout << "\n=== FPS Inventory Use Consumable Medkit ===" << std::endl;
    ecs::World world;
    systems::FPSInventorySystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);
    systems::FPSCombatSystem combatSys(&world);

    charSys.spawnCharacter("p1", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    auto* charEnt = world.getEntity("fpschar_p1");
    combatSys.createHealth("fpschar_p1", 100.0f, 0.0f, 0.0f);
    combatSys.applyDamage("fpschar_p1", 60.0f);

    sys.createInventory("p1", 8);
    sys.addItem("p1", "medkit_basic", "Medkit");

    assertTrue(sys.useConsumable("p1", "medkit_basic"), "Medkit consumed");
    assertTrue(combatSys.getHealth("fpschar_p1") > 40.0f, "Health restored");
}

void testFPSInventoryComponentDefaults() {
    std::cout << "\n=== FPS Inventory Component Defaults ===" << std::endl;
    components::FPSInventoryComponent inv;
    assertTrue(inv.max_slots == 8, "Default max slots 8");
    assertTrue(inv.slots.empty(), "Default empty slots");
    assertTrue(inv.equipped_weapon_id.empty(), "Default no weapon equipped");
    assertTrue(inv.equipped_tool_id.empty(), "Default no tool equipped");
    assertTrue(!inv.isFull(), "Default not full");
    assertTrue(inv.itemCount() == 0, "Default 0 items");
}

// ==================== CloakingSystem Tests ====================

void testCloakingActivate() {
    std::cout << "\n=== Cloaking: Activate ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    addComp<components::CloakingState>(e);

    systems::CloakingSystem sys(&world);
    assertTrue(sys.getPhase("ship1") == "Inactive", "Initial phase is Inactive");
    assertTrue(sys.activateCloak("ship1"), "Cloak activated");
    assertTrue(sys.getPhase("ship1") == "Activating", "Phase is Activating");
    assertTrue(!sys.activateCloak("ship1"), "Cannot activate while activating");
}

void testCloakingFullCycle() {
    std::cout << "\n=== Cloaking: Full Cycle ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 2.0f;
    cloak->deactivation_time = 1.0f;
    auto* cap = addComp<components::Capacitor>(e);
    cap->capacitor = 100.0f;
    cap->capacitor_max = 100.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(2.0f);  // activation_time reached
    assertTrue(sys.isCloaked("ship1"), "Ship is cloaked after activation_time");
    assertTrue(sys.deactivateCloak("ship1"), "Decloak initiated");
    assertTrue(sys.getPhase("ship1") == "Deactivating", "Phase is Deactivating");
    sys.update(1.0f);  // deactivation_time reached
    assertTrue(sys.getPhase("ship1") == "Inactive", "Phase returns to Inactive");
}

void testCloakingCapacitorDrain() {
    std::cout << "\n=== Cloaking: Capacitor Drain ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 1.0f;
    cloak->fuel_per_second = 10.0f;
    auto* cap = addComp<components::Capacitor>(e);
    cap->capacitor = 25.0f;
    cap->capacitor_max = 100.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(1.0f);  // become cloaked
    assertTrue(sys.isCloaked("ship1"), "Ship cloaked");
    sys.update(2.0f);  // drain 20 cap (25 - 20 = 5)
    assertTrue(sys.isCloaked("ship1"), "Still cloaked with remaining cap");
    assertTrue(cap->capacitor < 10.0f, "Capacitor drained");
}

void testCloakingCapacitorEmpty() {
    std::cout << "\n=== Cloaking: Capacitor Empty Force Decloak ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 1.0f;
    cloak->fuel_per_second = 50.0f;
    auto* cap = addComp<components::Capacitor>(e);
    cap->capacitor = 10.0f;
    cap->capacitor_max = 100.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(1.0f);  // become cloaked
    assertTrue(sys.isCloaked("ship1"), "Ship cloaked");
    sys.update(1.0f);  // drain 50 cap but only 10 available → force decloak
    assertTrue(!sys.isCloaked("ship1"), "Force decloaked by cap empty");
    assertTrue(sys.getPhase("ship1") == "Deactivating", "Phase is Deactivating");
    assertTrue(approxEqual(cap->capacitor, 0.0f), "Capacitor at zero");
}

void testCloakingProximityDecloak() {
    std::cout << "\n=== Cloaking: Proximity Decloak ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 1.0f;
    cloak->proximity_decloak_range = 2000.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(1.0f);
    assertTrue(sys.isCloaked("ship1"), "Ship cloaked");
    assertTrue(!sys.proximityDecloak("ship1", 3000.0f), "No decloak at 3000m");
    assertTrue(sys.proximityDecloak("ship1", 1500.0f), "Decloak at 1500m");
    assertTrue(sys.getPhase("ship1") == "Deactivating", "Phase is Deactivating");
}

void testCloakingTargetingLockout() {
    std::cout << "\n=== Cloaking: Targeting Lockout ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 1.0f;
    cloak->targeting_delay = 5.0f;

    systems::CloakingSystem sys(&world);
    sys.activateCloak("ship1");
    sys.update(1.0f);  // cloaked
    sys.deactivateCloak("ship1");
    assertTrue(sys.isTargetingLocked("ship1"), "Targeting locked after decloak");
    assertTrue(approxEqual(sys.getTargetingLockoutRemaining("ship1"), 5.0f), "Lockout is 5s");
    sys.update(3.0f);  // 3s pass
    assertTrue(sys.isTargetingLocked("ship1"), "Still locked after 3s");
    sys.update(3.0f);  // 3 more seconds
    assertTrue(!sys.isTargetingLocked("ship1"), "Lockout expired");
}

void testCloakingCovertOps() {
    std::cout << "\n=== Cloaking: Covert Ops ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->cloak_type = components::CloakingState::CloakType::CovertOps;
    cloak->can_warp_while_cloaked = true;

    systems::CloakingSystem sys(&world);
    assertTrue(sys.canWarpCloaked("ship1"), "CovertOps can warp cloaked");
}

void testCloakingDecloakCount() {
    std::cout << "\n=== Cloaking: Decloak Count ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    auto* cloak = addComp<components::CloakingState>(e);
    cloak->activation_time = 0.1f;
    cloak->deactivation_time = 0.1f;

    systems::CloakingSystem sys(&world);
    assertTrue(sys.getDecloakCount("ship1") == 0, "Initial decloak count 0");
    sys.activateCloak("ship1");
    sys.update(0.1f);  // cloaked
    sys.deactivateCloak("ship1");
    assertTrue(sys.getDecloakCount("ship1") == 1, "Decloak count 1");
    sys.update(0.1f);  // inactive
    sys.activateCloak("ship1");
    sys.update(0.1f);  // cloaked
    sys.deactivateCloak("ship1");
    assertTrue(sys.getDecloakCount("ship1") == 2, "Decloak count 2");
}

void testCloakingSetProximityRange() {
    std::cout << "\n=== Cloaking: Set Proximity Range ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("ship1");
    addComp<components::CloakingState>(e);

    systems::CloakingSystem sys(&world);
    assertTrue(sys.setProximityRange("ship1", 5000.0f), "Set range success");
    assertTrue(!sys.setProximityRange("nonexistent", 5000.0f), "Set range on missing fails");
}

void testCloakingMissing() {
    std::cout << "\n=== Cloaking: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CloakingSystem sys(&world);
    assertTrue(sys.getPhase("nonexistent") == "unknown", "Default phase for missing");
    assertTrue(!sys.isCloaked("nonexistent"), "Default not cloaked for missing");
    assertTrue(!sys.isTargetingLocked("nonexistent"), "Default no lockout for missing");
    assertTrue(approxEqual(sys.getTargetingLockoutRemaining("nonexistent"), 0.0f), "Default lockout 0");
    assertTrue(approxEqual(sys.getFuelRate("nonexistent"), 0.0f), "Default fuel rate 0");
    assertTrue(!sys.canWarpCloaked("nonexistent"), "Default no warp for missing");
    assertTrue(sys.getDecloakCount("nonexistent") == 0, "Default decloak count 0");
}

// ==================== CargoScanSystem Tests ====================

void testCargoScanInitiate() {
    std::cout << "\n=== Cargo Scan: Initiate ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    addComp<components::CargoScanState>(scanner);
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    assertTrue(sys.getPhase("scanner1") == "idle", "Initial phase is idle");
    assertTrue(sys.initiateScan("scanner1", "target1"), "Scan initiated");
    assertTrue(sys.getPhase("scanner1") == "scanning", "Phase is scanning");
    assertTrue(!sys.initiateScan("scanner1", "target1"), "Cannot initiate while scanning");
}

void testCargoScanCancel() {
    std::cout << "\n=== Cargo Scan: Cancel ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    addComp<components::CargoScanState>(scanner);
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    sys.initiateScan("scanner1", "target1");
    assertTrue(sys.cancelScan("scanner1"), "Cancel scan success");
    assertTrue(sys.getPhase("scanner1") == "idle", "Phase back to idle");
    assertTrue(!sys.cancelScan("scanner1"), "Cannot cancel when idle");
}

void testCargoScanComplete() {
    std::cout << "\n=== Cargo Scan: Complete Clean ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    auto* scan = addComp<components::CargoScanState>(scanner);
    scan->scan_time = 2.0f;
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    sys.initiateScan("scanner1", "target1");
    sys.update(2.0f);
    assertTrue(sys.getPhase("scanner1") == "complete", "Phase is complete");
    assertTrue(sys.getContrabandFound("scanner1") == 0, "No contraband on clean target");
    assertTrue(sys.getTotalScans("scanner1") == 1, "1 total scan");
}

void testCargoScanContraband() {
    std::cout << "\n=== Cargo Scan: Contraband Found ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    auto* scan = addComp<components::CargoScanState>(scanner);
    scan->scan_time = 1.0f;
    scan->fine_per_contraband = 1000.0f;
    auto* target = world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Narcotics);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Weapons);
    sys.initiateScan("scanner1", "target1");
    sys.update(1.0f);
    assertTrue(sys.getPhase("scanner1") == "complete", "Scan complete");
    assertTrue(sys.getContrabandFound("scanner1") == 2, "Found 2 contraband items");
    assertTrue(sys.getTotalContrabandDetected("scanner1") == 2, "Lifetime 2 detected");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalFinesIssued("scanner1")), 2000.0f), "Fines: 2 * 1000");
}

void testCargoScanProgress() {
    std::cout << "\n=== Cargo Scan: Progress ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    auto* scan = addComp<components::CargoScanState>(scanner);
    scan->scan_time = 4.0f;
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    assertTrue(approxEqual(sys.getScanProgress("scanner1"), 0.0f), "Progress 0 before scan");
    sys.initiateScan("scanner1", "target1");
    sys.update(2.0f);
    assertTrue(approxEqual(sys.getScanProgress("scanner1"), 0.5f), "Progress 0.5 at halfway");
}

void testCargoScanCustoms() {
    std::cout << "\n=== Cargo Scan: Customs Scanner ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    addComp<components::CargoScanState>(scanner);

    systems::CargoScanSystem sys(&world);
    assertTrue(!sys.isCustomsScanner("scanner1"), "Not customs by default");
    assertTrue(sys.markAsCustomsScanner("scanner1", true), "Mark as customs");
    assertTrue(sys.isCustomsScanner("scanner1"), "Is now customs");
}

void testCargoScanRemoveContraband() {
    std::cout << "\n=== Cargo Scan: Remove Contraband ===" << std::endl;
    ecs::World world;
    world.createEntity("target1");

    systems::CargoScanSystem sys(&world);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Narcotics);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Stolen);
    assertTrue(sys.removeContraband("target1", components::CargoScanState::ContrabandType::Narcotics), "Remove success");
    auto types = sys.getDetectedTypes("target1");
    assertTrue(types.size() == 1, "1 type remaining");
    assertTrue(types[0] == "stolen", "Remaining type is stolen");
}

void testCargoScanDetectionChance() {
    std::cout << "\n=== Cargo Scan: Detection Chance ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    addComp<components::CargoScanState>(scanner);

    systems::CargoScanSystem sys(&world);
    assertTrue(sys.setDetectionChance("scanner1", 0.5f), "Set detection chance");
    assertTrue(!sys.setDetectionChance("nonexistent", 0.5f), "Set on missing fails");
}

void testCargoScanMultipleScans() {
    std::cout << "\n=== Cargo Scan: Multiple Scans ===" << std::endl;
    ecs::World world;
    auto* scanner = world.createEntity("scanner1");
    auto* scan = addComp<components::CargoScanState>(scanner);
    scan->scan_time = 1.0f;
    scan->fine_per_contraband = 500.0f;
    world.createEntity("target1");
    world.createEntity("target2");

    systems::CargoScanSystem sys(&world);
    sys.plantContraband("target1", components::CargoScanState::ContrabandType::Exotic);

    // Scan 1
    sys.initiateScan("scanner1", "target1");
    sys.update(1.0f);
    assertTrue(sys.getTotalScans("scanner1") == 1, "1 scan complete");
    assertTrue(sys.getContrabandFound("scanner1") == 1, "Found 1 contraband");

    // Reset to idle for next scan
    scan->phase = components::CargoScanState::ScanPhase::Idle;

    // Scan 2: clean target
    sys.initiateScan("scanner1", "target2");
    sys.update(1.0f);
    assertTrue(sys.getTotalScans("scanner1") == 2, "2 scans total");
    assertTrue(sys.getContrabandFound("scanner1") == 0, "No contraband on clean target");
    assertTrue(sys.getTotalContrabandDetected("scanner1") == 1, "Lifetime 1 detected");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalFinesIssued("scanner1")), 500.0f), "Total fines 500");
}

void testCargoScanMissing() {
    std::cout << "\n=== Cargo Scan: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CargoScanSystem sys(&world);
    assertTrue(sys.getPhase("nonexistent") == "unknown", "Default phase for missing");
    assertTrue(approxEqual(sys.getScanProgress("nonexistent"), 0.0f), "Default progress for missing");
    assertTrue(sys.getContrabandFound("nonexistent") == 0, "Default contraband for missing");
    assertTrue(sys.getTotalScans("nonexistent") == 0, "Default scans for missing");
    assertTrue(sys.getTotalContrabandDetected("nonexistent") == 0, "Default lifetime for missing");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalFinesIssued("nonexistent")), 0.0f), "Default fines for missing");
    assertTrue(!sys.isCustomsScanner("nonexistent"), "Default not customs for missing");
}

// ==================== VisualRigSystem Tests ====================

void testVisualRigInit() {
    std::cout << "\n=== Visual Rig: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("rig1");

    systems::VisualRigSystem sys(&world);
    assertTrue(sys.initializeVisualState("rig1", 12345), "Visual state initialized");
    assertTrue(sys.getThrusterConfig("rig1") == "none", "No thrusters initially");
    assertTrue(sys.getCargoSize("rig1") == "none", "No cargo initially");
    assertTrue(!sys.initializeVisualState("rig1", 67890), "Duplicate init rejected");
}

void testVisualRigUpdateFromLoadout() {
    std::cout << "\n=== Visual Rig: Update From Loadout ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod_jet");
    auto* mod2 = world.createEntity("mod_cargo");

    // Add rig loadout
    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids.push_back("mod_jet");
    loadout->installed_module_ids.push_back("mod_cargo");
    rig->addComponent(std::move(loadout));

    // Add modules
    auto jet = std::make_unique<components::RigModule>();
    jet->type = components::RigModule::ModuleType::JetpackTank;
    mod1->addComponent(std::move(jet));

    auto cargo = std::make_unique<components::RigModule>();
    cargo->type = components::RigModule::ModuleType::CargoPod;
    mod2->addComponent(std::move(cargo));

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    assertTrue(sys.updateFromLoadout("rig1"), "Updated from loadout");
    assertTrue(sys.getThrusterConfig("rig1") == "single", "Single thruster");
    assertTrue(sys.getCargoSize("rig1") == "small", "Small cargo");
}

void testVisualRigThrusterConfig() {
    std::cout << "\n=== Visual Rig: Thruster Config ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod1");
    auto* mod2 = world.createEntity("mod2");
    auto* mod3 = world.createEntity("mod3");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids = {"mod1", "mod2", "mod3"};
    rig->addComponent(std::move(loadout));

    // Add 3 jetpack tanks
    for (const auto& id : {"mod1", "mod2", "mod3"}) {
        auto* e = world.getEntity(id);
        auto jet = std::make_unique<components::RigModule>();
        jet->type = components::RigModule::ModuleType::JetpackTank;
        e->addComponent(std::move(jet));
    }

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.updateFromLoadout("rig1");
    assertTrue(sys.getThrusterConfig("rig1") == "quad", "Quad thrusters from 3 tanks");
}

void testVisualRigCargoSize() {
    std::cout << "\n=== Visual Rig: Cargo Size ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod1");
    auto* mod2 = world.createEntity("mod2");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids = {"mod1", "mod2"};
    rig->addComponent(std::move(loadout));

    for (const auto& id : {"mod1", "mod2"}) {
        auto* e = world.getEntity(id);
        auto cargo = std::make_unique<components::RigModule>();
        cargo->type = components::RigModule::ModuleType::CargoPod;
        e->addComponent(std::move(cargo));
    }

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.updateFromLoadout("rig1");
    assertTrue(sys.getCargoSize("rig1") == "medium", "Medium cargo from 2 pods");
}

void testVisualRigFeatures() {
    std::cout << "\n=== Visual Rig: Features ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod1");
    auto* mod2 = world.createEntity("mod2");
    auto* mod3 = world.createEntity("mod3");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids = {"mod1", "mod2", "mod3"};
    rig->addComponent(std::move(loadout));

    auto shield = std::make_unique<components::RigModule>();
    shield->type = components::RigModule::ModuleType::Shield;
    world.getEntity("mod1")->addComponent(std::move(shield));

    auto sensor = std::make_unique<components::RigModule>();
    sensor->type = components::RigModule::ModuleType::Sensor;
    world.getEntity("mod2")->addComponent(std::move(sensor));

    auto solar = std::make_unique<components::RigModule>();
    solar->type = components::RigModule::ModuleType::SolarPanel;
    world.getEntity("mod3")->addComponent(std::move(solar));

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.updateFromLoadout("rig1");
    assertTrue(sys.hasShieldEmitter("rig1"), "Has shield emitter");
    assertTrue(sys.hasAntenna("rig1"), "Has antenna from sensor");
    assertTrue(sys.hasSolarPanels("rig1"), "Has solar panels");
}

void testVisualRigMounts() {
    std::cout << "\n=== Visual Rig: Weapon/Tool Mounts ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");
    auto* mod1 = world.createEntity("mod1");
    auto* mod2 = world.createEntity("mod2");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->installed_module_ids = {"mod1", "mod2"};
    rig->addComponent(std::move(loadout));

    auto weapon = std::make_unique<components::RigModule>();
    weapon->type = components::RigModule::ModuleType::WeaponMount;
    world.getEntity("mod1")->addComponent(std::move(weapon));

    auto tool = std::make_unique<components::RigModule>();
    tool->type = components::RigModule::ModuleType::ToolMount;
    world.getEntity("mod2")->addComponent(std::move(tool));

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.updateFromLoadout("rig1");
    assertTrue(sys.getWeaponMountCount("rig1") == 1, "1 weapon mount");
    assertTrue(sys.getToolMountCount("rig1") == 1, "1 tool mount");
}

void testVisualRigColors() {
    std::cout << "\n=== Visual Rig: Colors ===" << std::endl;
    ecs::World world;
    world.createEntity("rig1");

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);

    assertTrue(sys.setColors("rig1", "red", "white"), "Colors set");
    assertTrue(sys.getPrimaryColor("rig1") == "red", "Primary is red");
    assertTrue(sys.getSecondaryColor("rig1") == "white", "Secondary is white");
}

void testVisualRigTrinkets() {
    std::cout << "\n=== Visual Rig: Trinkets ===" << std::endl;
    ecs::World world;
    world.createEntity("rig1");

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);

    assertTrue(sys.canAddTrinket("rig1"), "Can add trinket");
    assertTrue(sys.addTrinket("rig1", "bobblehead1"), "Trinket added");
    assertTrue(sys.getTrinketCount("rig1") == 1, "1 trinket");
    assertTrue(!sys.addTrinket("rig1", "bobblehead1"), "Duplicate rejected");

    sys.addTrinket("rig1", "sticker1");
    sys.addTrinket("rig1", "mug1");
    sys.addTrinket("rig1", "figure1");
    assertTrue(sys.getTrinketCount("rig1") == 4, "4 trinkets");
    assertTrue(!sys.canAddTrinket("rig1"), "Max trinkets reached");

    assertTrue(sys.removeTrinket("rig1", "mug1"), "Trinket removed");
    assertTrue(sys.getTrinketCount("rig1") == 3, "3 trinkets");
}

void testVisualRigBulkGlow() {
    std::cout << "\n=== Visual Rig: Bulk and Glow ===" << std::endl;
    ecs::World world;
    auto* rig = world.createEntity("rig1");

    auto loadout = std::make_unique<components::RigLoadout>();
    loadout->total_cargo = 500.0f;
    loadout->total_shield = 50.0f;
    loadout->total_power = 100.0f;
    rig->addComponent(std::move(loadout));

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);
    sys.update(0.1f);

    assertTrue(sys.getTotalBulk("rig1") > 1.0f, "Bulk > 1 with cargo/shield");
    assertTrue(approxEqual(sys.getGlowIntensity("rig1"), 1.0f), "Glow at max from power");

    assertTrue(sys.setGlowIntensity("rig1", 0.5f), "Glow set");
    assertTrue(approxEqual(sys.getGlowIntensity("rig1"), 0.5f), "Glow at 0.5");
}

void testVisualRigScale() {
    std::cout << "\n=== Visual Rig: Scale ===" << std::endl;
    ecs::World world;
    world.createEntity("rig1");

    systems::VisualRigSystem sys(&world);
    sys.initializeVisualState("rig1", 12345);

    assertTrue(sys.setThrusterScale("rig1", 2.0f), "Thruster scale set");
    assertTrue(approxEqual(sys.getThrusterScale("rig1"), 2.0f), "Thruster scale is 2.0");
    assertTrue(sys.setCargoScale("rig1", 1.5f), "Cargo scale set");
    assertTrue(approxEqual(sys.getCargoScale("rig1"), 1.5f), "Cargo scale is 1.5");
}

void testVisualRigMissing() {
    std::cout << "\n=== Visual Rig: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::VisualRigSystem sys(&world);
    assertTrue(sys.getThrusterConfig("nonexistent") == "unknown", "Unknown config on missing");
    assertTrue(sys.getCargoSize("nonexistent") == "unknown", "Unknown cargo on missing");
    assertTrue(!sys.hasShieldEmitter("nonexistent"), "No shield on missing");
    assertTrue(sys.getPrimaryColor("nonexistent").empty(), "Empty color on missing");
    assertTrue(sys.getTrinketCount("nonexistent") == 0, "No trinkets on missing");
}

// ==================== RigLocker Tests ====================

void testRigLockerInit() {
    std::cout << "\n=== RigLocker: Init ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    assertTrue(sys.initializeLocker("locker_1", "player_1"), "Locker initialized");
    assertTrue(sys.getPresetCount("locker_1") == 0, "No presets initially");
    assertTrue(!sys.initializeLocker("locker_1", "player_1"), "Duplicate init fails");
}

void testRigLockerSave() {
    std::cout << "\n=== RigLocker: Save Preset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    std::vector<std::string> modules = {"LifeSupport", "PowerCore", "JetpackTank"};
    assertTrue(sys.savePreset("locker_1", "Combat Suit", modules), "Preset saved");
    assertTrue(sys.getPresetCount("locker_1") == 1, "1 preset after save");
}

void testRigLockerDelete() {
    std::cout << "\n=== RigLocker: Delete Preset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "Mining Suit", {"Sensor", "CargoPod"});
    assertTrue(sys.getPresetCount("locker_1") == 1, "1 preset before delete");
    assertTrue(sys.deletePreset("locker_1", "preset_0"), "Preset deleted");
    assertTrue(sys.getPresetCount("locker_1") == 0, "0 presets after delete");
    assertTrue(!sys.deletePreset("locker_1", "preset_0"), "Double delete fails");
}

void testRigLockerRename() {
    std::cout << "\n=== RigLocker: Rename Preset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "Old Name", {"PowerCore"});
    assertTrue(sys.renamePreset("locker_1", "preset_0", "New Name"), "Renamed");
    assertTrue(!sys.renamePreset("locker_1", "nonexistent", "X"), "Rename nonexistent fails");
}

void testRigLockerEquip() {
    std::cout << "\n=== RigLocker: Equip Preset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "EVA Suit", {"LifeSupport", "JetpackTank"});
    assertTrue(sys.equipPreset("locker_1", "preset_0"), "Preset equipped");
    assertTrue(sys.getActivePreset("locker_1") == "preset_0", "Active preset matches");
    assertTrue(!sys.equipPreset("locker_1", "nonexistent"), "Equip nonexistent fails");
}

void testRigLockerFavorite() {
    std::cout << "\n=== RigLocker: Favorite ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "Suit A", {"PowerCore"});
    sys.savePreset("locker_1", "Suit B", {"Shield"});
    assertTrue(sys.toggleFavorite("locker_1", "preset_0"), "Toggled favorite on");
    assertTrue(sys.getFavoriteCount("locker_1") == 1, "1 favorite");
    assertTrue(sys.toggleFavorite("locker_1", "preset_0"), "Toggled favorite off");
    assertTrue(sys.getFavoriteCount("locker_1") == 0, "0 favorites after toggle off");
}

void testRigLockerMass() {
    std::cout << "\n=== RigLocker: Mass Calculation ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "Heavy Suit", {"A", "B", "C", "D"});
    float mass = sys.getPresetMass("locker_1", "preset_0");
    assertTrue(mass > 0.0f, "Mass is positive");
    assertTrue(approxEqual(mass, 6.0f), "4 modules × 1.5kg = 6kg");
}

void testRigLockerMaxPresets() {
    std::cout << "\n=== RigLocker: Max Presets ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    for (int i = 0; i < 10; i++) {
        assertTrue(sys.savePreset("locker_1", "Preset " + std::to_string(i), {"mod"}),
                   "Save preset " + std::to_string(i));
    }
    assertTrue(sys.getPresetCount("locker_1") == 10, "10 presets at max");
    assertTrue(!sys.savePreset("locker_1", "Overflow", {"mod"}), "11th preset rejected");
}

void testRigLockerEquipTracking() {
    std::cout << "\n=== RigLocker: Equip Tracking ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "A", {"mod_a"});
    sys.savePreset("locker_1", "B", {"mod_b"});
    sys.equipPreset("locker_1", "preset_0");
    sys.equipPreset("locker_1", "preset_1");
    assertTrue(sys.getActivePreset("locker_1") == "preset_1", "Active is last equipped");
    sys.deletePreset("locker_1", "preset_1");
    assertTrue(sys.getActivePreset("locker_1").empty(), "Active cleared after delete");
}

void testRigLockerMissing() {
    std::cout << "\n=== RigLocker: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    assertTrue(!sys.initializeLocker("nonexistent", "p"), "Init fails on missing");
    assertTrue(!sys.savePreset("nonexistent", "x", {}), "Save fails on missing");
    assertTrue(sys.getPresetCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getActivePreset("nonexistent").empty(), "Empty active on missing");
    assertTrue(approxEqual(sys.getPresetMass("nonexistent", "x"), 0.0f), "0 mass on missing");
}

// ==================== VisualCoupling Tests ====================

void testVisualCouplingInit() {
    std::cout << "\n=== VisualCoupling: Init ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    assertTrue(sys.initializeCoupling("ship_1", "frigate_01"), "Coupling initialized");
    assertTrue(sys.getCouplingCount("ship_1") == 0, "No couplings initially");
    assertTrue(!sys.initializeCoupling("ship_1", "frigate_01"), "Duplicate init fails");
}

void testVisualCouplingAdd() {
    std::cout << "\n=== VisualCoupling: Add ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "frigate_01");
    assertTrue(sys.addCoupling("ship_1", "solar_1",
        components::VisualCoupling::ExteriorFeature::SolarPanel, 1.0f), "Solar panel added");
    assertTrue(sys.getCouplingCount("ship_1") == 1, "1 coupling");
    assertTrue(!sys.addCoupling("ship_1", "solar_1",
        components::VisualCoupling::ExteriorFeature::SolarPanel, 1.0f), "Duplicate rejected");
}

void testVisualCouplingRemove() {
    std::cout << "\n=== VisualCoupling: Remove ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "frigate_01");
    sys.addCoupling("ship_1", "vent_1",
        components::VisualCoupling::ExteriorFeature::Vent, 0.5f);
    assertTrue(sys.getCouplingCount("ship_1") == 1, "1 before remove");
    assertTrue(sys.removeCoupling("ship_1", "vent_1"), "Removed");
    assertTrue(sys.getCouplingCount("ship_1") == 0, "0 after remove");
    assertTrue(!sys.removeCoupling("ship_1", "vent_1"), "Double remove fails");
}

void testVisualCouplingVisibility() {
    std::cout << "\n=== VisualCoupling: Visibility ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "frigate_01");
    sys.addCoupling("ship_1", "ore_1",
        components::VisualCoupling::ExteriorFeature::OreContainer, 1.5f);
    assertTrue(sys.getVisibleCount("ship_1") == 1, "1 visible initially");
    assertTrue(sys.setVisibility("ship_1", "ore_1", false), "Set invisible");
    assertTrue(sys.getVisibleCount("ship_1") == 0, "0 visible after hide");
    assertTrue(sys.setVisibility("ship_1", "ore_1", true), "Set visible again");
    assertTrue(sys.getVisibleCount("ship_1") == 1, "1 visible after show");
}

void testVisualCouplingOffset() {
    std::cout << "\n=== VisualCoupling: Offset ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "cruiser_01");
    sys.addCoupling("ship_1", "antenna_1",
        components::VisualCoupling::ExteriorFeature::Antenna, 0.8f);
    assertTrue(sys.setOffset("ship_1", "antenna_1", 1.0f, 2.0f, 3.0f), "Offset set");
    assertTrue(!sys.setOffset("ship_1", "nonexistent", 0, 0, 0), "Offset fails for missing");
}

void testVisualCouplingFeatureCount() {
    std::cout << "\n=== VisualCoupling: Feature Count ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "battleship_01");
    sys.addCoupling("ship_1", "solar_1",
        components::VisualCoupling::ExteriorFeature::SolarPanel, 1.0f);
    sys.addCoupling("ship_1", "solar_2",
        components::VisualCoupling::ExteriorFeature::SolarPanel, 1.0f);
    sys.addCoupling("ship_1", "weapon_1",
        components::VisualCoupling::ExteriorFeature::WeaponMount, 1.0f);
    assertTrue(sys.getFeatureCount("ship_1",
        components::VisualCoupling::ExteriorFeature::SolarPanel) == 2, "2 solar panels");
    assertTrue(sys.getFeatureCount("ship_1",
        components::VisualCoupling::ExteriorFeature::WeaponMount) == 1, "1 weapon mount");
    assertTrue(sys.getFeatureCount("ship_1",
        components::VisualCoupling::ExteriorFeature::Vent) == 0, "0 vents");
}

void testVisualCouplingFeatureNames() {
    std::cout << "\n=== VisualCoupling: Feature Names ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    assertTrue(sys.getFeatureName(components::VisualCoupling::ExteriorFeature::SolarPanel) == "SolarPanel", "SolarPanel name");
    assertTrue(sys.getFeatureName(components::VisualCoupling::ExteriorFeature::OreContainer) == "OreContainer", "OreContainer name");
    assertTrue(sys.getFeatureName(components::VisualCoupling::ExteriorFeature::CargoRack) == "CargoRack", "CargoRack name");
}

void testVisualCouplingUpdate() {
    std::cout << "\n=== VisualCoupling: Update ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "frigate_01");
    sys.addCoupling("ship_1", "mod_1",
        components::VisualCoupling::ExteriorFeature::EngineBooster, 1.0f);
    sys.update(0.016f);
    sys.update(0.016f);
    auto* entity = world.getEntity("ship_1");
    auto* coupling = entity->getComponent<components::VisualCoupling>();
    assertTrue(coupling->total_updates == 2, "Update counter incremented");
}

void testVisualCouplingMaxEntries() {
    std::cout << "\n=== VisualCoupling: Max Entries ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    world.createEntity("ship_1");
    sys.initializeCoupling("ship_1", "titan_01");
    auto* entity = world.getEntity("ship_1");
    auto* coupling = entity->getComponent<components::VisualCoupling>();
    coupling->max_entries = 3;
    sys.addCoupling("ship_1", "a", components::VisualCoupling::ExteriorFeature::Vent, 1.0f);
    sys.addCoupling("ship_1", "b", components::VisualCoupling::ExteriorFeature::Vent, 1.0f);
    sys.addCoupling("ship_1", "c", components::VisualCoupling::ExteriorFeature::Vent, 1.0f);
    assertTrue(sys.getCouplingCount("ship_1") == 3, "3 at max");
    assertTrue(!sys.addCoupling("ship_1", "d",
        components::VisualCoupling::ExteriorFeature::Vent, 1.0f), "4th rejected");
}

void testVisualCouplingMissing() {
    std::cout << "\n=== VisualCoupling: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::VisualCouplingSystem sys(&world);
    assertTrue(!sys.initializeCoupling("nonexistent", "s"), "Init fails on missing");
    assertTrue(!sys.addCoupling("nonexistent", "m",
        components::VisualCoupling::ExteriorFeature::Vent, 1.0f), "Add fails on missing");
    assertTrue(sys.getCouplingCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getVisibleCount("nonexistent") == 0, "Visible 0 on missing");
}

// ==================== ShipTemplateModSystem Tests ====================

void testShipTemplateRegister() {
    std::cout << "\n=== ShipTemplateMod: Register ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    auto* e = world.createEntity("tmpl1");
    assertTrue(sys.registerTemplate("tmpl1", "rifter_v2", "Rifter Mk II", "frigate", "minmatar"), "Register succeeds");
    auto* mod = e->getComponent<components::ShipTemplateMod>();
    assertTrue(mod != nullptr, "Component exists");
    assertTrue(mod->template_id == "rifter_v2", "Template ID set");
    assertTrue(mod->ship_class == "frigate", "Ship class set");
}

void testShipTemplateValidate() {
    std::cout << "\n=== ShipTemplateMod: Validate ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    world.createEntity("tmpl1");
    sys.registerTemplate("tmpl1", "t1", "Ship", "cruiser", "amarr");
    assertTrue(sys.validateTemplate("tmpl1"), "Validation passes");
    assertTrue(sys.isValid("tmpl1"), "Is valid after validation");
}

void testShipTemplateValidateFail() {
    std::cout << "\n=== ShipTemplateMod: Validate Fail ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    auto* e = world.createEntity("tmpl1");
    sys.registerTemplate("tmpl1", "t1", "", "cruiser", "amarr");
    assertTrue(!sys.validateTemplate("tmpl1"), "Validation fails with empty name");
    assertTrue(!sys.isValid("tmpl1"), "Not valid");
}

void testShipTemplateModSource() {
    std::cout << "\n=== ShipTemplateMod: Mod Source ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    auto* e = world.createEntity("tmpl1");
    sys.registerTemplate("tmpl1", "t1", "Ship", "frigate", "gallente");
    assertTrue(sys.setModSource("tmpl1", "my_mod", 10), "Set mod source");
    auto* mod = e->getComponent<components::ShipTemplateMod>();
    assertTrue(mod->mod_source == "my_mod", "Mod source set");
    assertTrue(mod->priority == 10, "Priority set");
}

void testShipTemplateBaseInherit() {
    std::cout << "\n=== ShipTemplateMod: Base Template Inheritance ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    auto* base = world.createEntity("base1");
    sys.registerTemplate("base1", "base_frigate", "Base Frigate", "frigate", "minmatar");
    auto* baseMod = base->getComponent<components::ShipTemplateMod>();
    baseMod->hull_hp = 500.0f;
    baseMod->shield_hp = 300.0f;
    baseMod->high_slots = 4;

    auto* child = world.createEntity("child1");
    sys.registerTemplate("child1", "child_frigate", "Child Frigate", "frigate", "minmatar");
    sys.setBaseTemplate("child1", "base_frigate");
    auto* childMod = child->getComponent<components::ShipTemplateMod>();
    sys.update(0.0f);
    assertTrue(approxEqual(childMod->hull_hp, 500.0f), "Hull inherited from base");
    assertTrue(childMod->high_slots == 4, "High slots inherited");
}

void testShipTemplateAutoValidate() {
    std::cout << "\n=== ShipTemplateMod: Auto-Validate on Update ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    world.createEntity("tmpl1");
    sys.registerTemplate("tmpl1", "t1", "Ship", "cruiser", "caldari");
    sys.update(0.0f);
    assertTrue(sys.isValid("tmpl1"), "Auto-validated after update");
}

void testShipTemplateHighestPriority() {
    std::cout << "\n=== ShipTemplateMod: Highest Priority ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    world.createEntity("t1");
    world.createEntity("t2");
    sys.registerTemplate("t1", "id1", "S1", "frigate", "f1");
    sys.registerTemplate("t2", "id2", "S2", "cruiser", "f2");
    sys.setModSource("t1", "base", 5);
    sys.setModSource("t2", "addon", 15);
    assertTrue(sys.getHighestPriority() == 15, "Highest priority is 15");
}

void testShipTemplateCount() {
    std::cout << "\n=== ShipTemplateMod: Template Count ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    assertTrue(sys.getTemplateCount() == 0, "0 templates initially");
    world.createEntity("t1");
    sys.registerTemplate("t1", "id1", "S1", "frigate", "f1");
    assertTrue(sys.getTemplateCount() == 1, "1 template after register");
    world.createEntity("t2");
    sys.registerTemplate("t2", "id2", "S2", "cruiser", "f2");
    assertTrue(sys.getTemplateCount() == 2, "2 templates after second register");
}

void testShipTemplateGetters() {
    std::cout << "\n=== ShipTemplateMod: Getters ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    world.createEntity("t1");
    sys.registerTemplate("t1", "my_template", "Battleship", "battleship", "amarr");
    assertTrue(sys.getTemplateId("t1") == "my_template", "Template ID getter");
    assertTrue(sys.getShipClass("t1") == "battleship", "Ship class getter");
}

void testShipTemplateMissing() {
    std::cout << "\n=== ShipTemplateMod: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::ShipTemplateModSystem sys(&world);
    assertTrue(!sys.registerTemplate("nonexistent", "t", "n", "c", "f"), "Register fails on missing");
    assertTrue(!sys.setBaseTemplate("nonexistent", "b"), "SetBase fails on missing");
    assertTrue(!sys.setModSource("nonexistent", "m", 0), "SetModSource fails on missing");
    assertTrue(!sys.validateTemplate("nonexistent"), "Validate fails on missing");
    assertTrue(!sys.isValid("nonexistent"), "Not valid on missing");
    assertTrue(sys.getTemplateId("nonexistent") == "", "Empty template ID on missing");
    assertTrue(sys.getShipClass("nonexistent") == "", "Empty ship class on missing");
}


void run_inventory_tests() {
    testSkillTraining();
    testSkillInstantTrain();
    testSkillQueueMultiple();
    testSkillInvalidLevel();
    testModuleActivation();
    testModuleCycling();
    testModuleCapDrain();
    testModuleFittingValidation();
    testModuleToggle();
    testInventoryAddItem();
    testInventoryCapacityLimit();
    testInventoryRemoveItem();
    testInventoryTransfer();
    testInventoryHasItem();
    testLootGenerate();
    testLootCollect();
    testLootEmptyTable();
    testDroneLaunch();
    testDroneRecall();
    testDroneRecallAll();
    testDroneBandwidthLimit();
    testDroneCombatUpdate();
    testDroneDestroyedRemoval();
    testSerializeDeserializeDroneBay();
    testInsurancePurchase();
    testInsuranceClaim();
    testInsurancePlatinum();
    testInsuranceExpiry();
    testInsuranceInsufficientFunds();
    testBountyProcessKill();
    testBountyMultipleKills();
    testBountyLedgerRecordLimit();
    testBountyNonexistentPlayer();
    testShipFittingFitModule();
    testShipFittingSlotLimit();
    testShipFittingCPUOverflow();
    testShipFittingRemoveModule();
    testShipFittingValidate();
    testShipFittingSlotCapacity();
    testShipFittingMidLowSlots();
    testRigLoadoutDefaults();
    testRigInstallModule();
    testRigModuleFull();
    testRigDerivedStats();
    testRigRemoveModule();
    testSalvageSiteDefaults();
    testSalvageDiscover();
    testSalvageLoot();
    testSalvageFullyLooted();
    testSalvageTrinkets();
    testFPSInventoryCreate();
    testFPSInventoryAddRemove();
    testFPSInventoryStacking();
    testFPSInventoryFull();
    testFPSInventoryEquipWeapon();
    testFPSInventoryEquipWeaponNotInInventory();
    testFPSInventoryEquipTool();
    testFPSInventoryUseConsumableOxygen();
    testFPSInventoryUseConsumableFood();
    testFPSInventoryUseConsumableMedkit();
    testFPSInventoryComponentDefaults();
    testCloakingActivate();
    testCloakingFullCycle();
    testCloakingCapacitorDrain();
    testCloakingCapacitorEmpty();
    testCloakingProximityDecloak();
    testCloakingTargetingLockout();
    testCloakingCovertOps();
    testCloakingDecloakCount();
    testCloakingSetProximityRange();
    testCloakingMissing();
    testCargoScanInitiate();
    testCargoScanCancel();
    testCargoScanComplete();
    testCargoScanContraband();
    testCargoScanProgress();
    testCargoScanCustoms();
    testCargoScanRemoveContraband();
    testCargoScanDetectionChance();
    testCargoScanMultipleScans();
    testCargoScanMissing();
    testVisualRigInit();
    testVisualRigUpdateFromLoadout();
    testVisualRigThrusterConfig();
    testVisualRigCargoSize();
    testVisualRigFeatures();
    testVisualRigMounts();
    testVisualRigColors();
    testVisualRigTrinkets();
    testVisualRigBulkGlow();
    testVisualRigScale();
    testVisualRigMissing();
    testRigLockerInit();
    testRigLockerSave();
    testRigLockerDelete();
    testRigLockerRename();
    testRigLockerEquip();
    testRigLockerFavorite();
    testRigLockerMass();
    testRigLockerMaxPresets();
    testRigLockerEquipTracking();
    testRigLockerMissing();
    testVisualCouplingInit();
    testVisualCouplingAdd();
    testVisualCouplingRemove();
    testVisualCouplingVisibility();
    testVisualCouplingOffset();
    testVisualCouplingFeatureCount();
    testVisualCouplingFeatureNames();
    testVisualCouplingUpdate();
    testVisualCouplingMaxEntries();
    testVisualCouplingMissing();
    testShipTemplateRegister();
    testShipTemplateValidate();
    testShipTemplateValidateFail();
    testShipTemplateModSource();
    testShipTemplateBaseInherit();
    testShipTemplateAutoValidate();
    testShipTemplateHighestPriority();
    testShipTemplateCount();
    testShipTemplateGetters();
    testShipTemplateMissing();
}
