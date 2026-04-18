/**
 * Combat tests
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

// ==================== Damage Event Tests ====================

void testDamageEventOnShieldHit() {
    std::cout << "\n=== Damage Event On Shield Hit ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;
    health->armor_hp = 300.0f;
    health->armor_max = 300.0f;
    health->hull_hp = 200.0f;
    health->hull_max = 200.0f;

    combatSys.applyDamage("target1", 50.0f, "kinetic");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent created on damage");
    assertTrue(dmgEvent->recent_hits.size() == 1, "One hit recorded");
    assertTrue(dmgEvent->recent_hits[0].layer_hit == "shield", "Hit registered on shield layer");
    assertTrue(approxEqual(dmgEvent->recent_hits[0].damage_amount, 50.0f), "Damage amount recorded");
    assertTrue(dmgEvent->recent_hits[0].damage_type == "kinetic", "Damage type recorded");
    assertTrue(!dmgEvent->recent_hits[0].shield_depleted, "Shield not depleted");
}

void testDamageEventShieldDepleted() {
    std::cout << "\n=== Damage Event Shield Depleted ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 20.0f;
    health->shield_max = 500.0f;
    health->armor_hp = 300.0f;
    health->armor_max = 300.0f;
    health->hull_hp = 200.0f;
    health->hull_max = 200.0f;

    // Apply 50 damage; 20 to shield (depletes) + 30 overflows to armor
    combatSys.applyDamage("target1", 50.0f, "kinetic");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent created");
    assertTrue(dmgEvent->recent_hits.size() == 1, "One hit recorded");
    assertTrue(dmgEvent->recent_hits[0].shield_depleted, "Shield depleted flag set");
    assertTrue(approxEqual(health->shield_hp, 0.0f), "Shield HP is 0");
}

void testDamageEventHullCritical() {
    std::cout << "\n=== Damage Event Hull Critical ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 0.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 0.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 100.0f;
    health->hull_max = 100.0f;

    // Hit hull for 80 damage, leaving 20 HP (20% < 25% threshold)
    combatSys.applyDamage("target1", 80.0f, "explosive");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent created");
    assertTrue(dmgEvent->recent_hits[0].hull_critical, "Hull critical flag set (below 25%)");
    assertTrue(dmgEvent->recent_hits[0].layer_hit == "hull", "Hit on hull layer");
}

void testDamageEventMultipleHits() {
    std::cout << "\n=== Damage Event Multiple Hits ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;
    health->armor_hp = 300.0f;
    health->armor_max = 300.0f;

    combatSys.applyDamage("target1", 10.0f, "em");
    combatSys.applyDamage("target1", 20.0f, "thermal");
    combatSys.applyDamage("target1", 30.0f, "kinetic");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent exists");
    assertTrue(dmgEvent->recent_hits.size() == 3, "Three hits recorded");
    assertTrue(approxEqual(dmgEvent->total_damage_taken, 60.0f), "Total damage tracked");
}

void testDamageEventClearOldHits() {
    std::cout << "\n=== Damage Event Clear Old Hits ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;

    combatSys.applyDamage("target1", 10.0f, "em");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent exists");
    assertTrue(dmgEvent->recent_hits.size() == 1, "One hit before clear");

    // Clear with a future timestamp beyond max_age
    dmgEvent->clearOldHits(100.0f, 5.0f);
    assertTrue(dmgEvent->recent_hits.size() == 0, "Old hits cleared");
}

// ==================== AI Retreat Tests ====================

void testAIFleeOnLowHealth() {
    std::cout << "\n=== AI Flee On Low Health ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player1";
    ai->flee_threshold = 0.25f;

    auto* health = addComp<components::Health>(npc);
    health->shield_hp = 0.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 0.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 50.0f;   // 50 out of 300 total = 16.7% < 25%
    health->hull_max = 100.0f;

    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;

    auto* player = world.createEntity("player1");
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 1000.0f; playerPos->y = 0.0f; playerPos->z = 0.0f;
    addComp<components::Player>(player);

    aiSys.update(0.1f);

    assertTrue(ai->state == components::AI::State::Fleeing, "NPC flees when health below threshold");
}

void testAINoFleeAboveThreshold() {
    std::cout << "\n=== AI No Flee Above Threshold ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player1";
    ai->flee_threshold = 0.25f;

    auto* health = addComp<components::Health>(npc);
    health->shield_hp = 50.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 50.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 100.0f;   // 200 out of 300 total = 66.7% > 25%
    health->hull_max = 100.0f;

    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;

    auto* player = world.createEntity("player1");
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 1000.0f; playerPos->y = 0.0f; playerPos->z = 0.0f;
    addComp<components::Player>(player);

    // NPC needs a weapon to stay in attacking state (orbitBehavior checks for weapon)
    addComp<components::Weapon>(npc);

    aiSys.update(0.1f);

    assertTrue(ai->state != components::AI::State::Fleeing, "NPC does not flee when health above threshold");
}

void testAIFleeThresholdCustom() {
    std::cout << "\n=== AI Flee Threshold Custom ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player1";
    ai->flee_threshold = 0.50f;  // Custom high threshold

    auto* health = addComp<components::Health>(npc);
    health->shield_hp = 30.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 30.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 80.0f;   // 140 out of 300 = 46.7% < 50%
    health->hull_max = 100.0f;

    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;

    auto* player = world.createEntity("player1");
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 1000.0f; playerPos->y = 0.0f; playerPos->z = 0.0f;
    addComp<components::Player>(player);

    aiSys.update(0.1f);

    assertTrue(ai->state == components::AI::State::Fleeing, "NPC flees with custom high threshold");
}

// ==================== Combat Death → Wreck Integration Tests ====================

void testCombatDeathCallbackFires() {
    std::cout << "\n=== Combat: Death Callback Fires ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_created;
    combatSys.setDeathCallback([&](const std::string& entity_id, float x, float y, float z) {
        wreck_created = wreckSys.createWreck(entity_id, x, y, z);
    });

    auto* npc = world.createEntity("npc_1");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 0.0f;
    hp->armor_hp = 0.0f;
    hp->hull_hp = 10.0f;
    hp->hull_max = 100.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 500.0f;
    pos->y = 200.0f;
    pos->z = 100.0f;

    // Apply lethal damage
    combatSys.applyDamage("npc_1", 50.0f, "kinetic");

    assertTrue(!wreck_created.empty(), "Wreck created on death");
    assertTrue(wreckSys.getActiveWreckCount() == 1, "One active wreck");

    auto* wreck_entity = world.getEntity(wreck_created);
    assertTrue(wreck_entity != nullptr, "Wreck entity exists");
    auto* wreck_pos = wreck_entity->getComponent<components::Position>();
    assertTrue(wreck_pos != nullptr, "Wreck has position");
    assertTrue(approxEqual(wreck_pos->x, 500.0f), "Wreck at correct x");
    assertTrue(approxEqual(wreck_pos->y, 200.0f), "Wreck at correct y");
}

void testCombatDeathCallbackWithLoot() {
    std::cout << "\n=== Combat: Death Callback with Loot ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(999);

    std::string wreck_id;
    combatSys.setDeathCallback([&](const std::string& entity_id, float, float, float) {
        wreck_id = lootSys.generateLoot(entity_id);
    });

    auto* npc = world.createEntity("npc_loot");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 0.0f;
    hp->armor_hp = 0.0f;
    hp->hull_hp = 5.0f;
    hp->hull_max = 100.0f;
    addComp<components::Position>(npc);

    auto* lt = addComp<components::LootTable>(npc);
    components::LootTable::LootEntry entry;
    entry.item_id = "module_afterburner";
    entry.name = "Afterburner I";
    entry.type = "module";
    entry.drop_chance = 1.0f; // always drops
    entry.min_quantity = 1;
    entry.max_quantity = 1;
    entry.volume = 5.0f;
    lt->entries.push_back(entry);
    lt->isc_drop = 5000.0;

    combatSys.applyDamage("npc_loot", 100.0f, "em");

    assertTrue(!wreck_id.empty(), "Loot wreck created on death");
    auto* wreck = world.getEntity(wreck_id);
    assertTrue(wreck != nullptr, "Wreck entity exists");
    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    assertTrue(wreck_inv != nullptr, "Wreck has inventory");
    assertTrue(wreck_inv->items.size() >= 1, "Wreck contains loot");
}

void testCombatNoCallbackOnSurvival() {
    std::cout << "\n=== Combat: No Callback On Survival ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    bool callback_fired = false;
    combatSys.setDeathCallback([&](const std::string&, float, float, float) {
        callback_fired = true;
    });

    auto* npc = world.createEntity("npc_alive");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 100.0f;
    hp->armor_hp = 100.0f;
    hp->hull_hp = 100.0f;
    hp->hull_max = 100.0f;

    combatSys.applyDamage("npc_alive", 10.0f, "kinetic");
    assertTrue(!callback_fired, "Death callback does NOT fire when entity survives");
}

// ==================== Combat Loop Integration Tests ====================

void testCombatFireWeaponAppliesDamage() {
    std::cout << "\n=== Combat Loop: Fire Weapon Applies Damage ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    // Create shooter with weapon
    auto* shooter = world.createEntity("player_1");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 50.0f;
    wep->damage_type = "kinetic";
    wep->optimal_range = 10000.0f;
    wep->falloff_range = 5000.0f;
    wep->rate_of_fire = 3.0f;
    wep->cooldown = 0.0f;
    wep->ammo_count = 100;
    wep->capacitor_cost = 10.0f;
    auto* spos = addComp<components::Position>(shooter);
    spos->x = 0.0f; spos->y = 0.0f; spos->z = 0.0f;

    // Create target with health
    auto* target = world.createEntity("npc_target");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 5000.0f; tpos->y = 0.0f; tpos->z = 0.0f;

    float shield_before = hp->shield_hp;
    bool fired = combatSys.fireWeapon("player_1", "npc_target");

    assertTrue(fired, "Weapon fired successfully");
    assertTrue(hp->shield_hp < shield_before, "Target shield took damage");
    assertTrue(wep->cooldown > 0.0f, "Weapon now on cooldown");
    assertTrue(wep->ammo_count == 99, "Ammo consumed");
}

void testCombatFireWeaponCooldownPrevents() {
    std::cout << "\n=== Combat Loop: Cooldown Prevents Firing ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* shooter = world.createEntity("player_cd");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 50.0f;
    wep->damage_type = "em";
    wep->optimal_range = 10000.0f;
    wep->falloff_range = 5000.0f;
    wep->rate_of_fire = 3.0f;
    wep->cooldown = 2.0f;  // On cooldown
    wep->ammo_count = 100;
    addComp<components::Position>(shooter);

    auto* target = world.createEntity("npc_cd_target");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    addComp<components::Position>(target);

    bool fired = combatSys.fireWeapon("player_cd", "npc_cd_target");

    assertTrue(!fired, "Weapon blocked by cooldown");
    assertTrue(hp->shield_hp == 100.0f, "Target took no damage");
}

void testCombatFireWeaponOutOfRange() {
    std::cout << "\n=== Combat Loop: Out of Range Prevents Firing ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* shooter = world.createEntity("player_range");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 50.0f;
    wep->damage_type = "thermal";
    wep->optimal_range = 5000.0f;
    wep->falloff_range = 2000.0f;
    wep->rate_of_fire = 3.0f;
    wep->cooldown = 0.0f;
    wep->ammo_count = 100;
    auto* spos = addComp<components::Position>(shooter);
    spos->x = 0.0f; spos->y = 0.0f; spos->z = 0.0f;

    auto* target = world.createEntity("npc_far");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 50000.0f; tpos->y = 0.0f; tpos->z = 0.0f;  // Way beyond range

    bool fired = combatSys.fireWeapon("player_range", "npc_far");

    assertTrue(!fired, "Weapon blocked by range");
    assertTrue(hp->shield_hp == 100.0f, "Target took no damage when out of range");
}

void testCombatFireWeaponDamageLayerCascade() {
    std::cout << "\n=== Combat Loop: Damage Cascades Shield → Armor → Hull ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* shooter = world.createEntity("player_cascade");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 200.0f;
    wep->damage_type = "em";
    wep->optimal_range = 20000.0f;
    wep->falloff_range = 5000.0f;
    wep->rate_of_fire = 5.0f;
    wep->cooldown = 0.0f;
    wep->ammo_count = 100;
    auto* spos = addComp<components::Position>(shooter);
    spos->x = 0.0f; spos->y = 0.0f; spos->z = 0.0f;

    auto* target = world.createEntity("npc_cascade");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 50.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 50.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 1000.0f; tpos->y = 0.0f; tpos->z = 0.0f;

    combatSys.fireWeapon("player_cascade", "npc_cascade");

    assertTrue(hp->shield_hp == 0.0f, "Shield depleted");
    assertTrue(hp->armor_hp == 0.0f, "Armor depleted");
    assertTrue(hp->hull_hp < 100.0f, "Hull took overflow damage");
}

// ==================== System Resources Component Tests ====================

void testSystemResourcesTracking() {
    std::cout << "\n=== System Resources: Tracking ===" << std::endl;

    ecs::World world;
    auto* system = world.createEntity("system_jita");
    auto* res = addComp<components::SystemResources>(system);

    components::SystemResources::ResourceEntry ferrite;
    ferrite.mineral_type = "Ferrite";
    ferrite.total_quantity = 100000.0f;
    ferrite.remaining_quantity = 100000.0f;
    res->resources.push_back(ferrite);

    components::SystemResources::ResourceEntry galvite;
    galvite.mineral_type = "Galvite";
    galvite.total_quantity = 50000.0f;
    galvite.remaining_quantity = 30000.0f;
    res->resources.push_back(galvite);

    assertTrue(res->resources.size() == 2, "Two resource types tracked");
    assertTrue(approxEqual(res->totalRemaining(), 130000.0f), "Total remaining correct");

    // Simulate depletion
    res->resources[0].remaining_quantity -= 10000.0f;
    assertTrue(approxEqual(res->totalRemaining(), 120000.0f), "Total after depletion");
}

// ==================== AI Reputation Targeting Tests ====================

void testAISkipsFriendlyTargets() {
    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create an NPC with faction
    auto* npc = world.createEntity("npc1");
    addComp<components::AI>(npc)->behavior = components::AI::Behavior::Aggressive;
    addComp<components::Position>(npc);
    addComp<components::Velocity>(npc);
    auto* npcFaction = addComp<components::Faction>(npc);
    npcFaction->faction_name = "Solari";

    // Create a player with positive standing toward Solari
    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 100.0f;
    auto* standings = addComp<components::Standings>(player);
    standings->faction_standings["Solari"] = 5.0f;

    ecs::Entity* target = aiSys.selectTarget(npc);
    assertTrue(target == nullptr, "AI does not target player with positive faction standing");
}

void testAITargetsHostileEntities() {
    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->awareness_range = 100000.0f;
    addComp<components::Position>(npc);
    addComp<components::Velocity>(npc);
    auto* npcFaction = addComp<components::Faction>(npc);
    npcFaction->faction_name = "Serpentis";

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 100.0f;
    auto* standings = addComp<components::Standings>(player);
    standings->faction_standings["Serpentis"] = -5.0f;

    ecs::Entity* target = aiSys.selectTarget(npc);
    assertTrue(target != nullptr, "AI targets player with negative faction standing");
}

void testAITargetsHostileNPCFaction() {
    ecs::World world;
    systems::AISystem aiSys(&world);
    constexpr float kTestAwarenessRange = 100000.0f;
    constexpr float kTestTargetDistance = 100.0f;

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->awareness_range = kTestAwarenessRange;
    addComp<components::Position>(npc);
    addComp<components::Velocity>(npc);
    auto* npcFaction = addComp<components::Faction>(npc);
    npcFaction->faction_name = "Solari";
    npcFaction->standings["Veyren"] = -5.0f;

    auto* hostileNpc = world.createEntity("npc2");
    addComp<components::AI>(hostileNpc);
    auto* hostilePos = addComp<components::Position>(hostileNpc);
    hostilePos->x = kTestTargetDistance;
    auto* hostileFaction = addComp<components::Faction>(hostileNpc);
    hostileFaction->faction_name = "Veyren";

    ecs::Entity* target = aiSys.selectTarget(npc);
    assertTrue(target == hostileNpc, "AI targets hostile NPC faction when no player target exists");
}

// ==================== Combat Threat System Tests ====================

void testCombatThreatDamage() {
    std::cout << "\n=== Combat Threat: Damage Increases Threat ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    auto* sys = world.createEntity("system_1");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.0f;

    ctSys.recordCombatDamage("system_1", 500.0f);
    assertTrue(ctSys.getPendingDamage("system_1") == 500.0f, "Pending damage recorded");

    ctSys.update(1.0f);
    assertTrue(state->threat_level > 0.0f, "Threat increased after damage");
    float expected = 500.0f * ctSys.damage_threat_factor;
    assertTrue(approxEqual(state->threat_level, expected), "Threat matches expected value");
}

void testCombatThreatDestruction() {
    std::cout << "\n=== Combat Threat: Destruction Spikes Threat ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    auto* sys = world.createEntity("system_2");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.1f;

    ctSys.recordShipDestruction("system_2");
    ctSys.recordShipDestruction("system_2");
    assertTrue(ctSys.getPendingDestructions("system_2") == 2, "Two destructions pending");

    ctSys.update(1.0f);
    float expected = 0.1f + 2 * ctSys.destruction_threat_spike;
    assertTrue(approxEqual(state->threat_level, expected), "Threat spiked by 2 destructions");
}

void testCombatThreatClamped() {
    std::cout << "\n=== Combat Threat: Threat Clamped at Max ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    auto* sys = world.createEntity("system_3");
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->threat_level = 0.95f;

    ctSys.recordShipDestruction("system_3");
    ctSys.recordShipDestruction("system_3");
    ctSys.recordShipDestruction("system_3");
    ctSys.update(1.0f);
    assertTrue(state->threat_level <= ctSys.max_threat, "Threat clamped at max");
}

void testCombatThreatClearedAfterUpdate() {
    std::cout << "\n=== Combat Threat: Pending Cleared After Update ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    auto* sys = world.createEntity("system_4");
    addComp<components::SimStarSystemState>(sys);

    ctSys.recordCombatDamage("system_4", 100.0f);
    ctSys.update(1.0f);
    assertTrue(ctSys.getPendingDamage("system_4") == 0.0f, "Pending damage cleared");
    assertTrue(ctSys.getPendingDestructions("system_4") == 0, "Pending destructions cleared");
}

void testCombatThreatNoPendingForUnknown() {
    std::cout << "\n=== Combat Threat: No Pending for Unknown System ===" << std::endl;
    ecs::World world;
    systems::CombatThreatSystem ctSys(&world);

    assertTrue(ctSys.getPendingDamage("unknown") == 0.0f, "No pending damage for unknown");
    assertTrue(ctSys.getPendingDestructions("unknown") == 0, "No pending destructions for unknown");
}

// ==================== Security Response System Tests ====================

void testSecurityResponseDefaults() {
    std::cout << "\n=== Security Response: Defaults ===" << std::endl;
    ecs::World world;

    auto* sys = world.createEntity("sec_sys");
    auto* resp = addComp<components::SecurityResponseState>(sys);

    assertTrue(!resp->responding, "Not responding by default");
    assertTrue(resp->response_timer == 0.0f, "Timer starts at 0");
    assertTrue(resp->response_strength == 0.0f, "Strength starts at 0");
}

void testSecurityResponseTriggered() {
    std::cout << "\n=== Security Response: Triggered in High-Sec ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    auto* sys = world.createEntity("sec_highsec");
    auto* resp  = addComp<components::SecurityResponseState>(sys);
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.9f;
    state->threat_level = 0.5f;  // above threshold

    // Run enough ticks to pass the delay
    for (int i = 0; i < 100; i++) {
        secSys.update(1.0f);
    }

    assertTrue(resp->responding, "Security response active in high-sec with threat");
    assertTrue(secSys.isResponding("sec_highsec"), "Query confirms responding");
}

void testSecurityResponseNoTriggerLowSec() {
    std::cout << "\n=== Security Response: No Response in Low-Sec ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    auto* sys = world.createEntity("sec_lowsec");
    auto* resp  = addComp<components::SecurityResponseState>(sys);
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.2f;  // below min level
    state->threat_level = 0.9f;    // high threat

    for (int i = 0; i < 100; i++) {
        secSys.update(1.0f);
    }

    assertTrue(!resp->responding, "No response in low-sec");
}

void testSecurityResponseNoTriggerLowThreat() {
    std::cout << "\n=== Security Response: No Response on Low Threat ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    auto* sys = world.createEntity("sec_calm");
    auto* resp  = addComp<components::SecurityResponseState>(sys);
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.8f;
    state->threat_level = 0.1f;  // below threshold

    secSys.update(10.0f);
    assertTrue(!resp->responding, "No response on low threat");
}

void testSecurityResponseDelayScaling() {
    std::cout << "\n=== Security Response: Faster in Higher Sec ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    // High-sec system — should respond faster
    auto* sys1 = world.createEntity("sec_fast");
    auto* resp1 = addComp<components::SecurityResponseState>(sys1);
    auto* st1   = addComp<components::SimStarSystemState>(sys1);
    st1->security_level = 1.0f;
    st1->threat_level = 0.5f;

    secSys.update(0.1f);  // start timer
    float timer_high = resp1->response_timer;

    ecs::World world2;
    systems::SecurityResponseSystem secSys2(&world2);
    auto* sys2 = world2.createEntity("sec_slow");
    auto* resp2 = addComp<components::SecurityResponseState>(sys2);
    auto* st2   = addComp<components::SimStarSystemState>(sys2);
    st2->security_level = 0.5f;
    st2->threat_level = 0.5f;

    secSys2.update(0.1f);
    float timer_mid = resp2->response_timer;

    assertTrue(timer_high < timer_mid, "High-sec response delay shorter than mid-sec");
}

void testSecurityResponseDuration() {
    std::cout << "\n=== Security Response: Response Expires ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);
    secSys.response_duration = 10.0f;

    auto* sys = world.createEntity("sec_expire");
    auto* resp  = addComp<components::SecurityResponseState>(sys);
    auto* state = addComp<components::SimStarSystemState>(sys);
    state->security_level = 0.9f;
    state->threat_level = 0.0f;  // no threat initially

    // Manually activate the response to test duration expiry
    resp->responding = true;
    resp->response_timer = 10.0f;
    assertTrue(resp->responding, "Response is active");

    // Tick past the response duration
    for (int i = 0; i < 15; i++) secSys.update(1.0f);
    assertTrue(!resp->responding, "Response expired after duration");
}

void testSecurityResponseRespondingSystems() {
    std::cout << "\n=== Security Response: List Responding Systems ===" << std::endl;
    ecs::World world;
    systems::SecurityResponseSystem secSys(&world);

    auto* sys1 = world.createEntity("sec_a");
    auto* resp1 = addComp<components::SecurityResponseState>(sys1);
    addComp<components::SimStarSystemState>(sys1);
    resp1->responding = true;  // manually set for query test

    auto* sys2 = world.createEntity("sec_b");
    auto* resp2 = addComp<components::SecurityResponseState>(sys2);
    addComp<components::SimStarSystemState>(sys2);
    resp2->responding = false;

    auto list = secSys.getRespondingSystems();
    assertTrue(list.size() == 1, "One responding system");
    assertTrue(list[0] == "sec_a", "Correct responding system");
}

// ==================== Turret Placement System Tests ====================

void testTurretPlacementBasic() {
    std::cout << "\n=== Turret Placement Basic ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto placement = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Cruiser, 4);
    assertTrue(static_cast<int>(placement.mounts.size()) == 4, "4 mounts placed");
    assertTrue(placement.coverage_score > 0.0f, "Has some coverage");
    assertTrue(placement.coverage_score <= 1.0f, "Coverage ≤ 1.0");
}

void testTurretPlacementDeterminism() {
    std::cout << "\n=== Turret Placement Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto p1 = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Battleship, 6);
    auto p2 = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Battleship, 6);
    assertTrue(p1.mounts.size() == p2.mounts.size(), "Same mount count");
    assertTrue(approxEqual(p1.coverage_score, p2.coverage_score), "Same coverage");
    assertTrue(approxEqual(p1.max_overlap, p2.max_overlap), "Same overlap");
}

void testTurretPlacementOverlapThreshold() {
    std::cout << "\n=== Turret Placement Overlap Threshold ===" << std::endl;
    // Moderate turret counts should pass the 30% overlap threshold.
    bool allValid = true;
    for (uint64_t i = 1; i <= 50; ++i) {
        pcg::PCGContext ctx{ i * 13, 1 };
        auto p = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Cruiser, 3);
        if (!p.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "3-turret cruisers all pass overlap threshold");
}

void testTurretPlacementFaction() {
    std::cout << "\n=== Turret Placement Faction ===" << std::endl;
    pcg::PCGContext ctx{ 999, 1 };
    auto solari = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Cruiser, 4, "Solari");
    auto veyren = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Cruiser, 4, "Veyren");
    assertTrue(static_cast<int>(solari.mounts.size()) == 4, "Solari has 4 mounts");
    assertTrue(static_cast<int>(veyren.mounts.size()) == 4, "Veyren has 4 mounts");
    // Veyren should have wider arcs (utilitarian spread).
    float solariArcSum = 0.0f, veyrenArcSum = 0.0f;
    for (const auto& m : solari.mounts) solariArcSum += m.arc_deg;
    for (const auto& m : veyren.mounts) veyrenArcSum += m.arc_deg;
    assertTrue(veyrenArcSum > solariArcSum, "Veyren has wider total arcs than Solari");
}

void testTurretPlacementZeroTurrets() {
    std::cout << "\n=== Turret Placement Zero Turrets ===" << std::endl;
    pcg::PCGContext ctx{ 0, 1 };
    auto p = pcg::TurretPlacementSystem::place(ctx, pcg::HullClass::Frigate, 0);
    assertTrue(p.mounts.empty(), "Zero turrets produces no mounts");
    assertTrue(approxEqual(p.coverage_score, 0.0f), "No coverage with zero turrets");
}

void testTurretPlacementCoverageComputation() {
    std::cout << "\n=== Turret Placement Coverage Computation ===" << std::endl;
    // A single turret with 360° arc should give 100% coverage.
    std::vector<pcg::TurretMount> mounts;
    pcg::TurretMount m{};
    m.direction_deg = 0.0f;
    m.arc_deg = 360.0f;
    mounts.push_back(m);
    float cov = pcg::TurretPlacementSystem::computeCoverage(mounts);
    assertTrue(approxEqual(cov, 1.0f), "360° arc gives 100% coverage");
}

// ==================== FPS Combat System Tests ====================

void testFPSCombatCreateWeapon() {
    std::cout << "\n=== FPS Combat Create Weapon ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    bool ok = sys.createWeapon("wpn_pistol", "p1",
                                components::FPSWeapon::WeaponCategory::Sidearm,
                                "kinetic", 15.0f, 50.0f, 0.5f, 12);
    assertTrue(ok, "Create weapon succeeds");
    assertTrue(!sys.createWeapon("wpn_pistol", "p1",
                                  components::FPSWeapon::WeaponCategory::Sidearm,
                                  "kinetic", 15.0f, 50.0f, 0.5f, 12),
               "Duplicate weapon fails");
    assertTrue(sys.getAmmo("wpn_pistol") == 12, "Ammo set correctly");
}

void testFPSCombatEquipUnequip() {
    std::cout << "\n=== FPS Combat Equip/Unequip ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    sys.createWeapon("wpn_1", "p1", components::FPSWeapon::WeaponCategory::Rifle,
                      "kinetic", 25.0f, 80.0f, 0.3f, 30);

    assertTrue(sys.equipWeapon("wpn_1"), "Equip succeeds");
    auto* e = world.getEntity("wpn_1");
    auto* w = e->getComponent<components::FPSWeapon>();
    assertTrue(w->is_equipped, "Weapon is equipped");

    assertTrue(sys.unequipWeapon("wpn_1"), "Unequip succeeds");
    assertTrue(!w->is_equipped, "Weapon is unequipped");
}

void testFPSCombatReload() {
    std::cout << "\n=== FPS Combat Reload ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    sys.createWeapon("wpn_1", "p1", components::FPSWeapon::WeaponCategory::Rifle,
                      "kinetic", 25.0f, 80.0f, 0.3f, 30, 1.0f, 2.0f);

    auto* e = world.getEntity("wpn_1");
    auto* w = e->getComponent<components::FPSWeapon>();
    w->ammo = 5;  // Partially depleted

    assertTrue(sys.startReload("wpn_1"), "Start reload succeeds");
    assertTrue(sys.isReloading("wpn_1"), "Weapon is reloading");
    assertTrue(!sys.startReload("wpn_1"), "Can't double-reload");

    // Tick through reload
    sys.update(2.5f);
    assertTrue(!sys.isReloading("wpn_1"), "Reload complete");
    assertTrue(sys.getAmmo("wpn_1") == 30, "Ammo refilled");
}

void testFPSCombatReloadFullMag() {
    std::cout << "\n=== FPS Combat Reload Full Magazine ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    sys.createWeapon("wpn_1", "p1", components::FPSWeapon::WeaponCategory::Rifle,
                      "kinetic", 25.0f, 80.0f, 0.3f, 30);

    assertTrue(!sys.startReload("wpn_1"), "Can't reload full magazine");
}

void testFPSCombatCreateHealth() {
    std::cout << "\n=== FPS Combat Create Health ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target_npc");
    assertTrue(sys.createHealth("target_npc", 100.0f, 50.0f, 5.0f), "Create health succeeds");
    assertTrue(!sys.createHealth("target_npc", 100.0f, 50.0f, 5.0f), "Duplicate health fails");
    assertTrue(approxEqual(sys.getHealth("target_npc"), 100.0f), "Health set");
    assertTrue(approxEqual(sys.getShield("target_npc"), 50.0f), "Shield set");
    assertTrue(sys.isAlive("target_npc"), "Alive");
}

void testFPSCombatDamageShieldFirst() {
    std::cout << "\n=== FPS Combat Damage Shield First ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target");
    sys.createHealth("target", 100.0f, 50.0f, 5.0f);

    sys.applyDamage("target", 30.0f, "kinetic");
    assertTrue(approxEqual(sys.getShield("target"), 20.0f), "Shield absorbed 30 damage");
    assertTrue(approxEqual(sys.getHealth("target"), 100.0f), "Health untouched");
}

void testFPSCombatDamageOverflow() {
    std::cout << "\n=== FPS Combat Damage Shield Overflow ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target");
    sys.createHealth("target", 100.0f, 30.0f, 5.0f);

    sys.applyDamage("target", 50.0f, "kinetic");
    assertTrue(approxEqual(sys.getShield("target"), 0.0f), "Shield depleted");
    assertTrue(approxEqual(sys.getHealth("target"), 80.0f), "Health took overflow (20)");
}

void testFPSCombatDeath() {
    std::cout << "\n=== FPS Combat Death ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    std::string dead_id;
    sys.setDeathCallback([&](const std::string& id, float, float, float) {
        dead_id = id;
    });

    auto* entity = world.createEntity("victim");
    sys.createHealth("victim", 50.0f, 0.0f, 0.0f);

    sys.applyDamage("victim", 999.0f, "kinetic");
    assertTrue(!sys.isAlive("victim"), "Victim is dead");
    assertTrue(dead_id == "victim", "Death callback fired");
}

void testFPSCombatShieldRecharge() {
    std::cout << "\n=== FPS Combat Shield Recharge ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target");
    sys.createHealth("target", 100.0f, 50.0f, 10.0f);

    // Damage shield
    sys.applyDamage("target", 30.0f, "kinetic");
    assertTrue(approxEqual(sys.getShield("target"), 20.0f), "Shield after damage");

    // Wait for recharge delay (default 3s)
    sys.update(4.0f);
    float shield_after = sys.getShield("target");
    assertTrue(shield_after > 20.0f, "Shield began recharging after delay");
}

void testFPSCombatShieldRechargeDelay() {
    std::cout << "\n=== FPS Combat Shield Recharge Delay ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);

    auto* entity = world.createEntity("target");
    sys.createHealth("target", 100.0f, 50.0f, 10.0f);

    sys.applyDamage("target", 30.0f, "kinetic");

    // Update with less than recharge delay — shield should not recharge
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getShield("target"), 20.0f),
               "Shield not recharging during delay");
}

void testFPSCombatFireWeapon() {
    std::cout << "\n=== FPS Combat Fire Weapon ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    // Shooter
    charSys.spawnCharacter("shooter", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    sys.createWeapon("wpn_s", "shooter", components::FPSWeapon::WeaponCategory::Sidearm,
                      "kinetic", 15.0f, 50.0f, 0.5f, 12);
    sys.equipWeapon("wpn_s");

    // Target (FPS character with health)
    charSys.spawnCharacter("target", "interior_1", 10.0f, 0.0f, 0.0f, 0.0f);
    auto* targetEnt = world.getEntity("fpschar_target");
    sys.createHealth("fpschar_target", 100.0f, 0.0f, 0.0f);

    bool ok = sys.fireWeapon("shooter", "fpschar_target");
    assertTrue(ok, "Fire weapon succeeds");
    assertTrue(sys.getAmmo("wpn_s") == 11, "Ammo decremented");
    assertTrue(approxEqual(sys.getHealth("fpschar_target"), 85.0f), "Target took 15 damage");
}

void testFPSCombatFireOutOfRange() {
    std::cout << "\n=== FPS Combat Fire Out Of Range ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("shooter", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    sys.createWeapon("wpn_s", "shooter", components::FPSWeapon::WeaponCategory::Sidearm,
                      "kinetic", 15.0f, 50.0f, 0.5f, 12);
    sys.equipWeapon("wpn_s");

    charSys.spawnCharacter("target", "interior_1", 200.0f, 0.0f, 0.0f, 0.0f);
    sys.createHealth("fpschar_target", 100.0f, 0.0f, 0.0f);

    bool ok = sys.fireWeapon("shooter", "fpschar_target");
    assertTrue(!ok, "Fire out of range fails");
    assertTrue(sys.getAmmo("wpn_s") == 12, "Ammo not consumed");
}

void testFPSCombatFireOnCooldown() {
    std::cout << "\n=== FPS Combat Fire On Cooldown ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("shooter", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    sys.createWeapon("wpn_s", "shooter", components::FPSWeapon::WeaponCategory::Sidearm,
                      "kinetic", 15.0f, 50.0f, 0.5f, 12);
    sys.equipWeapon("wpn_s");

    charSys.spawnCharacter("target", "interior_1", 10.0f, 0.0f, 0.0f, 0.0f);
    sys.createHealth("fpschar_target", 100.0f, 0.0f, 0.0f);

    assertTrue(sys.fireWeapon("shooter", "fpschar_target"), "First shot succeeds");
    assertTrue(!sys.fireWeapon("shooter", "fpschar_target"), "Second shot on cooldown fails");

    sys.update(1.0f);  // Clear cooldown
    assertTrue(sys.fireWeapon("shooter", "fpschar_target"), "Shot after cooldown succeeds");
}

void testFPSCombatFireNoAmmo() {
    std::cout << "\n=== FPS Combat Fire No Ammo ===" << std::endl;
    ecs::World world;
    systems::FPSCombatSystem sys(&world);
    systems::FPSCharacterControllerSystem charSys(&world);

    charSys.spawnCharacter("shooter", "interior_1", 0.0f, 0.0f, 0.0f, 0.0f);
    sys.createWeapon("wpn_s", "shooter", components::FPSWeapon::WeaponCategory::Sidearm,
                      "kinetic", 15.0f, 50.0f, 0.5f, 1);
    sys.equipWeapon("wpn_s");

    charSys.spawnCharacter("target", "interior_1", 10.0f, 0.0f, 0.0f, 0.0f);
    sys.createHealth("fpschar_target", 100.0f, 0.0f, 0.0f);

    assertTrue(sys.fireWeapon("shooter", "fpschar_target"), "First shot uses last ammo");
    sys.update(1.0f);
    assertTrue(!sys.fireWeapon("shooter", "fpschar_target"), "No ammo to fire");
}

void testFPSCombatCategoryNames() {
    std::cout << "\n=== FPS Combat Category Names ===" << std::endl;
    assertTrue(systems::FPSCombatSystem::categoryName(0) == "Sidearm", "Sidearm name");
    assertTrue(systems::FPSCombatSystem::categoryName(1) == "Rifle", "Rifle name");
    assertTrue(systems::FPSCombatSystem::categoryName(2) == "Shotgun", "Shotgun name");
    assertTrue(systems::FPSCombatSystem::categoryName(3) == "Tool", "Tool name");
    assertTrue(systems::FPSCombatSystem::categoryName(99) == "Unknown", "Unknown category");
}

void testFPSCombatWeaponDefaults() {
    std::cout << "\n=== FPS Combat Weapon Defaults ===" << std::endl;
    components::FPSWeapon w;
    assertTrue(w.category == 0, "Default category Sidearm");
    assertTrue(w.damage_type == "kinetic", "Default damage type kinetic");
    assertTrue(approxEqual(w.damage, 15.0f), "Default damage 15");
    assertTrue(approxEqual(w.range, 50.0f), "Default range 50");
    assertTrue(w.ammo == 30, "Default ammo 30");
    assertTrue(!w.is_reloading, "Default not reloading");
    assertTrue(!w.is_equipped, "Default not equipped");
}

void testFPSCombatHealthDefaults() {
    std::cout << "\n=== FPS Combat Health Defaults ===" << std::endl;
    components::FPSHealth h;
    assertTrue(approxEqual(h.health, 100.0f), "Default health 100");
    assertTrue(approxEqual(h.shield, 50.0f), "Default shield 50");
    assertTrue(h.is_alive, "Default alive");
    assertTrue(approxEqual(h.getHealthFraction(), 1.0f), "Health fraction 1.0");
    assertTrue(approxEqual(h.getShieldFraction(), 1.0f), "Shield fraction 1.0");
}


void run_combat_tests() {
    testDamageEventOnShieldHit();
    testDamageEventShieldDepleted();
    testDamageEventHullCritical();
    testDamageEventMultipleHits();
    testDamageEventClearOldHits();
    testAIFleeOnLowHealth();
    testAINoFleeAboveThreshold();
    testAIFleeThresholdCustom();
    testCombatDeathCallbackFires();
    testCombatDeathCallbackWithLoot();
    testCombatNoCallbackOnSurvival();
    testCombatFireWeaponAppliesDamage();
    testCombatFireWeaponCooldownPrevents();
    testCombatFireWeaponOutOfRange();
    testCombatFireWeaponDamageLayerCascade();
    testSystemResourcesTracking();
    testAISkipsFriendlyTargets();
    testAITargetsHostileEntities();
    testAITargetsHostileNPCFaction();
    testCombatThreatDamage();
    testCombatThreatDestruction();
    testCombatThreatClamped();
    testCombatThreatClearedAfterUpdate();
    testCombatThreatNoPendingForUnknown();
    testSecurityResponseDefaults();
    testSecurityResponseTriggered();
    testSecurityResponseNoTriggerLowSec();
    testSecurityResponseNoTriggerLowThreat();
    testSecurityResponseDelayScaling();
    testSecurityResponseDuration();
    testSecurityResponseRespondingSystems();
    testTurretPlacementBasic();
    testTurretPlacementDeterminism();
    testTurretPlacementOverlapThreshold();
    testTurretPlacementFaction();
    testTurretPlacementZeroTurrets();
    testTurretPlacementCoverageComputation();
    testFPSCombatCreateWeapon();
    testFPSCombatEquipUnequip();
    testFPSCombatReload();
    testFPSCombatReloadFullMag();
    testFPSCombatCreateHealth();
    testFPSCombatDamageShieldFirst();
    testFPSCombatDamageOverflow();
    testFPSCombatDeath();
    testFPSCombatShieldRecharge();
    testFPSCombatShieldRechargeDelay();
    testFPSCombatFireWeapon();
    testFPSCombatFireOutOfRange();
    testFPSCombatFireOnCooldown();
    testFPSCombatFireNoAmmo();
    testFPSCombatCategoryNames();
    testFPSCombatWeaponDefaults();
    testFPSCombatHealthDefaults();
}
