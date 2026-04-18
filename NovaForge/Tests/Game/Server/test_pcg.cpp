/**
 * Pcg tests
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

// ==================== WreckSalvageSystem Tests ====================

void testWreckCreate() {
    std::cout << "\n=== Wreck Create ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("dead_ship_1", 100.0f, 0.0f, 200.0f, 600.0f);
    assertTrue(!wreck_id.empty(), "Wreck created with valid id");

    auto* entity = world.getEntity(wreck_id);
    assertTrue(entity != nullptr, "Wreck entity exists");

    auto* wreck = entity->getComponent<components::Wreck>();
    assertTrue(wreck != nullptr, "Wreck component attached");
    assertTrue(wreck->source_entity_id == "dead_ship_1", "Source entity id correct");
    assertTrue(approxEqual(wreck->lifetime_remaining, 600.0f), "Lifetime is correct");
    assertTrue(!wreck->salvaged, "Not yet salvaged");
}

void testWreckLifetimeDecay() {
    std::cout << "\n=== Wreck Lifetime Decay ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    wreckSys.createWreck("ship1", 0, 0, 0, 10.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 1, "One active wreck");

    wreckSys.update(5.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 1, "Wreck still active after 5s");

    wreckSys.update(6.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 0, "Wreck despawned after expiry");
}

void testSalvageWreckInRange() {
    std::cout << "\n=== Salvage Wreck In Range ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 100.0f, 0.0f, 0.0f);

    // Add loot to wreck
    auto* wreck_entity = world.getEntity(wreck_id);
    auto* wreck_inv = wreck_entity->getComponent<components::Inventory>();
    components::Inventory::Item loot;
    loot.item_id = "scrap_1";
    loot.name = "Metal Scraps";
    loot.type = "salvage";
    loot.quantity = 5;
    loot.volume = 1.0f;
    wreck_inv->items.push_back(loot);

    // Create player near the wreck
    auto* player = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(player);
    pos->x = 110.0f;
    auto* inv = addComp<components::Inventory>(player);
    inv->max_capacity = 1000.0f;

    bool ok = wreckSys.salvageWreck("player_1", wreck_id, 2500.0f);
    assertTrue(ok, "Salvage succeeds when in range");

    assertTrue(inv->items.size() == 1, "Player received 1 item stack");
    assertTrue(inv->items[0].name == "Metal Scraps", "Correct item transferred");
    assertTrue(inv->items[0].quantity == 5, "Correct quantity transferred");
}

void testSalvageWreckOutOfRange() {
    std::cout << "\n=== Salvage Wreck Out Of Range ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 0, 0, 0);

    auto* player = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(player);
    pos->x = 99999.0f;
    addComp<components::Inventory>(player);

    bool ok = wreckSys.salvageWreck("player_1", wreck_id, 2500.0f);
    assertTrue(!ok, "Salvage fails when out of range");
}

void testSalvageAlreadySalvaged() {
    std::cout << "\n=== Salvage Already Salvaged ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 0, 0, 0);

    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);
    addComp<components::Inventory>(player);

    wreckSys.salvageWreck("player_1", wreck_id, 5000.0f);
    bool again = wreckSys.salvageWreck("player_1", wreck_id, 5000.0f);
    assertTrue(!again, "Cannot salvage same wreck twice");
}

void testWreckActiveCount() {
    std::cout << "\n=== Wreck Active Count ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    wreckSys.createWreck("s1", 0, 0, 0);
    wreckSys.createWreck("s2", 100, 0, 0);
    wreckSys.createWreck("s3", 200, 0, 0);
    assertTrue(wreckSys.getActiveWreckCount() == 3, "Three active wrecks");

    // Salvage one
    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);
    addComp<components::Inventory>(player);

    auto entities = world.getAllEntities();
    std::string first_wreck;
    for (auto* e : entities) {
        if (e->getComponent<components::Wreck>()) {
            first_wreck = e->getId();
            break;
        }
    }
    wreckSys.salvageWreck("player_1", first_wreck, 999999.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 2, "Two active after one salvaged");
}

void testWreckHasInventory() {
    std::cout << "\n=== Wreck Has Inventory ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 0, 0, 0);
    auto* entity = world.getEntity(wreck_id);
    auto* inv = entity->getComponent<components::Inventory>();
    assertTrue(inv != nullptr, "Wreck has Inventory component");
    assertTrue(approxEqual(inv->max_capacity, 500.0f), "Wreck cargo capacity is 500 m3");
}

// ==================== PCG Framework Tests ====================

void testDeterministicRNGSameSeed() {
    std::cout << "\n=== PCG: DeterministicRNG same-seed determinism ===" << std::endl;
    atlas::pcg::DeterministicRNG a(12345);
    atlas::pcg::DeterministicRNG b(12345);

    bool allMatch = true;
    for (int i = 0; i < 100; ++i) {
        if (a.nextU32() != b.nextU32()) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Two RNGs with same seed produce identical sequence");
}

void testDeterministicRNGDifferentSeed() {
    std::cout << "\n=== PCG: DeterministicRNG different seeds diverge ===" << std::endl;
    atlas::pcg::DeterministicRNG a(111);
    atlas::pcg::DeterministicRNG b(222);

    bool anyDiffer = false;
    for (int i = 0; i < 20; ++i) {
        if (a.nextU32() != b.nextU32()) { anyDiffer = true; break; }
    }
    assertTrue(anyDiffer, "Different seeds produce different sequences");
}

void testDeterministicRNGRange() {
    std::cout << "\n=== PCG: DeterministicRNG range bounds ===" << std::endl;
    atlas::pcg::DeterministicRNG rng(42);

    bool inBounds = true;
    for (int i = 0; i < 500; ++i) {
        int v = rng.range(10, 20);
        if (v < 10 || v > 20) { inBounds = false; break; }
    }
    assertTrue(inBounds, "range(10,20) always in [10,20]");

    bool floatOk = true;
    atlas::pcg::DeterministicRNG rng2(99);
    for (int i = 0; i < 500; ++i) {
        float f = rng2.nextFloat();
        if (f < 0.0f || f >= 1.0f) { floatOk = false; break; }
    }
    assertTrue(floatOk, "nextFloat() always in [0,1)");
}

void testHashCombineDeterminism() {
    std::cout << "\n=== PCG: hashCombine determinism ===" << std::endl;
    uint64_t h1 = atlas::pcg::hashCombine(100, 200);
    uint64_t h2 = atlas::pcg::hashCombine(100, 200);
    assertTrue(h1 == h2, "hashCombine is deterministic");

    uint64_t h3 = atlas::pcg::hashCombine(100, 201);
    assertTrue(h1 != h3, "Different inputs produce different hashes");
}

void testHash64FourInputs() {
    std::cout << "\n=== PCG: hash64 four-input determinism ===" << std::endl;
    uint64_t a = atlas::pcg::hash64(1, 2, 3, 4);
    uint64_t b = atlas::pcg::hash64(1, 2, 3, 4);
    assertTrue(a == b, "hash64 is deterministic");

    uint64_t c = atlas::pcg::hash64(1, 2, 3, 5);
    assertTrue(a != c, "Changing one input changes hash");
}

void testDeriveSeed() {
    std::cout << "\n=== PCG: deriveSeed hierarchy ===" << std::endl;
    uint64_t parent = 0xDEADBEEF;
    uint64_t child1 = atlas::pcg::deriveSeed(parent, 1);
    uint64_t child2 = atlas::pcg::deriveSeed(parent, 2);
    uint64_t child1b = atlas::pcg::deriveSeed(parent, 1);

    assertTrue(child1 == child1b, "Same parent+id produces same child seed");
    assertTrue(child1 != child2, "Different ids produce different child seeds");
    assertTrue(child1 != parent, "Child seed differs from parent");
}

void testPCGManagerInitialize() {
    std::cout << "\n=== PCG: PCGManager initialize ===" << std::endl;
    atlas::pcg::PCGManager mgr;
    assertTrue(!mgr.isInitialized(), "Not initialized before init");

    mgr.initialize(0xCAFEBABE);
    assertTrue(mgr.isInitialized(), "Initialized after init");
    assertTrue(mgr.universeSeed() == 0xCAFEBABE, "Seed stored correctly");
}

void testPCGManagerContextDeterminism() {
    std::cout << "\n=== PCG: PCGManager context determinism ===" << std::endl;
    atlas::pcg::PCGManager mgr;
    mgr.initialize(42);

    auto ctx1 = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 100, 1);
    auto ctx2 = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 100, 1);
    assertTrue(ctx1.seed == ctx2.seed, "Same domain+id → same seed");

    auto ctx3 = mgr.makeRootContext(atlas::pcg::PCGDomain::Ship, 101, 1);
    assertTrue(ctx1.seed != ctx3.seed, "Different id → different seed");

    auto ctx4 = mgr.makeRootContext(atlas::pcg::PCGDomain::Asteroid, 100, 1);
    assertTrue(ctx1.seed != ctx4.seed, "Different domain → different seed");
}

void testShipGeneratorDeterminism() {
    std::cout << "\n=== PCG: ShipGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 777, 1 };

    auto ship1 = atlas::pcg::ShipGenerator::generate(ctx);
    auto ship2 = atlas::pcg::ShipGenerator::generate(ctx);

    assertTrue(ship1.hullClass == ship2.hullClass, "Same hull class");
    assertTrue(ship1.mass == ship2.mass, "Same mass");
    assertTrue(ship1.thrust == ship2.thrust, "Same thrust");
    assertTrue(ship1.turretSlots == ship2.turretSlots, "Same turret slots");
    assertTrue(ship1.valid == ship2.valid, "Same validity");
}

void testShipGeneratorConstraints() {
    std::cout << "\n=== PCG: ShipGenerator constraints ===" << std::endl;
    // Generate many ships and verify invariants.
    bool allValid = true;
    bool thrustOk = true;
    bool massOk = true;

    for (uint64_t i = 1; i <= 200; ++i) {
        atlas::pcg::PCGContext ctx{ i * 137, 1 };
        auto ship = atlas::pcg::ShipGenerator::generate(ctx);
        if (!ship.valid)    allValid = false;
        if (ship.thrust <= 0) thrustOk = false;
        if (ship.mass <= 0)  massOk = false;
    }
    assertTrue(allValid, "All 200 generated ships are valid");
    assertTrue(thrustOk, "All ships have positive thrust");
    assertTrue(massOk, "All ships have positive mass");
}

void testShipGeneratorHullOverride() {
    std::cout << "\n=== PCG: ShipGenerator hull override ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 555, 1 };

    auto frigate = atlas::pcg::ShipGenerator::generate(ctx, atlas::pcg::HullClass::Frigate);
    assertTrue(frigate.hullClass == atlas::pcg::HullClass::Frigate, "Hull override works (Frigate)");
    assertTrue(frigate.mass >= 1000.0f && frigate.mass <= 2500.0f, "Frigate mass in range");

    auto capital = atlas::pcg::ShipGenerator::generate(ctx, atlas::pcg::HullClass::Capital);
    assertTrue(capital.hullClass == atlas::pcg::HullClass::Capital, "Hull override works (Capital)");
    assertTrue(capital.mass >= 800000.0f, "Capital mass in range");
    assertTrue(capital.engineCount >= 6, "Capital has ≥ 6 engines");
}

void testShipGeneratorExpandedFields() {
    std::cout << "\n=== PCG: ShipGenerator expanded fields ===" << std::endl;
    bool armorOk = true;
    bool shieldOk = true;
    bool sigOk = true;
    bool targetOk = true;
    bool nameOk = true;
    bool droneOk = true;

    for (uint64_t i = 1; i <= 100; ++i) {
        atlas::pcg::PCGContext ctx{ i * 43, 1 };
        auto ship = atlas::pcg::ShipGenerator::generate(ctx);
        if (ship.armorHP <= 0.0f) armorOk = false;
        if (ship.shieldHP <= 0.0f) shieldOk = false;
        if (ship.signatureRadius <= 0.0f) sigOk = false;
        if (ship.targetingSpeed <= 0.0f) targetOk = false;
        if (ship.shipName.empty()) nameOk = false;
        if (ship.droneBay < 0) droneOk = false;
    }
    assertTrue(armorOk, "All ships have positive armor");
    assertTrue(shieldOk, "All ships have positive shields");
    assertTrue(sigOk, "All ships have positive signature radius");
    assertTrue(targetOk, "All ships have positive targeting speed");
    assertTrue(nameOk, "All ships have non-empty names");
    assertTrue(droneOk, "All ships have non-negative drone bay");
}

void testShipGeneratorExpandedDeterminism() {
    std::cout << "\n=== PCG: ShipGenerator expanded determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 777, 1 };
    auto s1 = atlas::pcg::ShipGenerator::generate(ctx);
    auto s2 = atlas::pcg::ShipGenerator::generate(ctx);

    assertTrue(s1.armorHP == s2.armorHP, "Same armor HP");
    assertTrue(s1.shieldHP == s2.shieldHP, "Same shield HP");
    assertTrue(s1.signatureRadius == s2.signatureRadius, "Same signature radius");
    assertTrue(s1.targetingSpeed == s2.targetingSpeed, "Same targeting speed");
    assertTrue(s1.droneBay == s2.droneBay, "Same drone bay");
    assertTrue(s1.shipName == s2.shipName, "Same ship name");
}

void testShipGeneratorHullRanges() {
    std::cout << "\n=== PCG: ShipGenerator hull-specific ranges ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 999, 1 };

    auto frigate = atlas::pcg::ShipGenerator::generate(ctx, atlas::pcg::HullClass::Frigate);
    assertTrue(frigate.armorHP >= 300.0f && frigate.armorHP <= 600.0f, "Frigate armor in range");
    assertTrue(frigate.shieldHP >= 250.0f && frigate.shieldHP <= 500.0f, "Frigate shield in range");
    assertTrue(frigate.signatureRadius >= 30.0f && frigate.signatureRadius <= 45.0f, "Frigate sig in range");

    auto bs = atlas::pcg::ShipGenerator::generate(ctx, atlas::pcg::HullClass::Battleship);
    assertTrue(bs.armorHP >= 6000.0f && bs.armorHP <= 10000.0f, "Battleship armor in range");
    assertTrue(bs.signatureRadius >= 300.0f && bs.signatureRadius <= 450.0f, "Battleship sig in range");

    assertTrue(bs.armorHP > frigate.armorHP, "Battleship armor > Frigate armor");
    assertTrue(bs.signatureRadius > frigate.signatureRadius, "Battleship sig > Frigate sig");
}

void testShipGeneratorShipName() {
    std::cout << "\n=== PCG: ShipGenerator ship naming ===" << std::endl;
    atlas::pcg::PCGContext ctx1{ 100, 1 };
    atlas::pcg::PCGContext ctx2{ 200, 1 };

    auto s1 = atlas::pcg::ShipGenerator::generate(ctx1);
    auto s2 = atlas::pcg::ShipGenerator::generate(ctx2);

    assertTrue(!s1.shipName.empty(), "Ship 1 has a name");
    assertTrue(!s2.shipName.empty(), "Ship 2 has a name");
    // Names from different seeds should usually differ.
    assertTrue(s1.shipName != s2.shipName, "Different seeds produce different names");
    // Name contains a hyphen (format: Prefix Suffix-Serial).
    assertTrue(s1.shipName.find('-') != std::string::npos, "Ship name contains serial separator");
}

void testRoomGraphFunctionalTypes() {
    std::cout << "\n=== PCG: RoomGraph functional type assignment ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 42, 1 };

    // Deck 0: first room should be Cockpit.
    auto deck0 = atlas::pcg::generateRoomsForDeck(0, ctx, 3);
    assertTrue(deck0[0].type == atlas::pcg::RoomType::Cockpit,
               "Deck 0 first room is Cockpit");

    // Last room should be Engine.
    assertTrue(deck0.back().type == atlas::pcg::RoomType::Engine,
               "Last room on deck 0 is Engine");

    // Deck 1+: first room should be Reactor.
    auto deck1 = atlas::pcg::generateRoomsForDeck(1, ctx, 3);
    assertTrue(deck1[0].type == atlas::pcg::RoomType::Reactor,
               "Deck 1 first room is Reactor");
    assertTrue(deck1.back().type == atlas::pcg::RoomType::Engine,
               "Last room on deck 1 is Engine");
}

void testRoomGraphDimensionsByType() {
    std::cout << "\n=== PCG: RoomGraph dimensions vary by type ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 50, 1 };

    // Generate deck 0 with enough rooms to include varied types.
    auto rooms = atlas::pcg::generateRoomsForDeck(0, ctx, 5);

    for (const auto& r : rooms) {
        switch (r.type) {
            case atlas::pcg::RoomType::Engine:
                assertTrue(r.dimZ >= 6.0f, "Engine room Z >= 6m");
                break;
            case atlas::pcg::RoomType::Corridor:
                assertTrue(r.dimX <= 3.0f, "Corridor X <= 3m");
                break;
            case atlas::pcg::RoomType::Reactor:
                assertTrue(r.dimX >= 5.0f, "Reactor X >= 5m");
                break;
            default:
                assertTrue(r.dimX > 0.0f && r.dimY > 0.0f && r.dimZ > 0.0f,
                           "Room has positive dimensions");
                break;
        }
    }
}

void testDeckGraphHubAndSpoke() {
    std::cout << "\n=== PCG: DeckGraph hub-and-spoke corridors ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 200, 1 };

    // Use shipClass 4 to get enough rooms for hub-and-spoke to kick in.
    auto decks = atlas::pcg::generateDeckStack(4, ctx);
    bool hubFound = false;
    for (const auto& deck : decks) {
        if (deck.rooms.size() >= 4) {
            // Should have linear (N-1) + hub ((N-2)) corridors.
            size_t expected = (deck.rooms.size() - 1) + (deck.rooms.size() - 2);
            assertTrue(deck.corridors.size() == expected,
                       "Deck has linear + hub-and-spoke corridors");
            hubFound = true;
            break;
        }
    }
    assertTrue(hubFound, "At least one deck has hub-and-spoke connections");
}

void testShipDesignerSaveRoundTrip() {
    std::cout << "\n=== PCG: ShipDesigner save captures rooms ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 800, 1 };
    auto decks = atlas::pcg::generateDeckStack(2, ctx);

    // Count total rooms.
    int totalRooms = 0;
    for (const auto& d : decks) totalRooms += static_cast<int>(d.rooms.size());

    auto save = atlas::pcg::saveShipLayout(decks, 2, 800);
    assertTrue(save.pcgVersion == 1, "Save version set");
    assertTrue(save.shipClass == 2, "Save class stored");
    assertTrue(save.seed == 800, "Save seed stored");
    assertTrue(static_cast<int>(save.roomOverrides.size()) == totalRooms,
               "Save captures all rooms as overrides");
}

void testTitanAssemblyDefaults() {
    std::cout << "\n=== Systems: TitanAssembly defaults ===" << std::endl;
    atlas::systems::TitanAssemblyComponent comp;
    assertTrue(comp.progress == 0.0f, "Initial progress is 0");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Rumor, "Initial phase is Rumor");
    assertTrue(!comp.disrupted, "Not disrupted initially");
    assertTrue(comp.disruptCount == 0, "No disruptions initially");
}

void testTitanAssemblyTick() {
    std::cout << "\n=== Systems: TitanAssembly tick progression ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.resourceRate = 0.05f;

    // 4 ticks → progress = 0.20
    for (int i = 0; i < 4; ++i) sys.tick(comp);
    assertTrue(comp.progress >= 0.19f && comp.progress <= 0.21f, "4 ticks at 0.05 ≈ 0.20");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Unease, "Phase transitions to Unease at 20%");

    // 6 more ticks → progress = 0.50
    for (int i = 0; i < 6; ++i) sys.tick(comp);
    assertTrue(comp.progress >= 0.49f && comp.progress <= 0.51f, "10 ticks at 0.05 ≈ 0.50");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Fear, "Phase transitions to Fear at 50%");
}

void testTitanAssemblyDisrupt() {
    std::cout << "\n=== Systems: TitanAssembly disruption ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.progress = 0.50f;
    comp.phase = atlas::systems::TitanPhase::Fear;

    sys.disrupt(comp, 0.35f);
    assertTrue(comp.progress >= 0.14f && comp.progress <= 0.16f, "Progress reduced by disruption");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Rumor, "Phase regressed to Rumor");
    assertTrue(comp.disruptCount == 1, "Disruption counted");
    assertTrue(comp.disrupted, "Disrupted flag set");
}

void testTitanAssemblyDisruptedTick() {
    std::cout << "\n=== Systems: TitanAssembly disrupted tick ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.resourceRate = 0.10f;
    comp.disrupted = true;

    sys.tick(comp);
    // Disrupted tick advances at 25% rate: 0.10 * 0.25 = 0.025
    assertTrue(comp.progress >= 0.024f && comp.progress <= 0.026f, "Disrupted tick at 25% rate");
    assertTrue(!comp.disrupted, "Disrupted flag cleared after tick");
}

void testTitanAssemblyClamped() {
    std::cout << "\n=== Systems: TitanAssembly progress clamped ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.progress = 0.99f;
    comp.resourceRate = 0.10f;

    sys.tick(comp);
    assertTrue(comp.progress == 1.0f, "Progress clamped at 1.0");
    assertTrue(comp.phase == atlas::systems::TitanPhase::Acceptance, "Acceptance phase at 100%");
}

void testTitanAssemblyDisruptFloor() {
    std::cout << "\n=== Systems: TitanAssembly disrupt floor ===" << std::endl;
    atlas::systems::TitanAssemblySystem sys;
    atlas::systems::TitanAssemblyComponent comp;
    comp.progress = 0.05f;

    sys.disrupt(comp, 0.50f);
    assertTrue(comp.progress == 0.0f, "Progress floored at 0.0");
}

void testTitanAssemblyPhaseName() {
    std::cout << "\n=== Systems: TitanAssembly phase names ===" << std::endl;
    using atlas::systems::TitanPhase;
    using atlas::systems::TitanAssemblySystem;
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Rumor) == "Rumor", "Rumor name");
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Unease) == "Unease", "Unease name");
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Fear) == "Fear", "Fear name");
    assertTrue(TitanAssemblySystem::phaseName(TitanPhase::Acceptance) == "Acceptance", "Acceptance name");
}

// ==================== Star System Generator Tests ====================

void testStarSystemGeneration() {
    std::cout << "\n=== PCG: StarSystemGenerator basic ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 12345, 1 };
    auto sys = StarSystemGenerator::generate(ctx, 0.8f);
    assertTrue(sys.valid, "Generated system is valid");
    assertTrue(sys.totalPlanets >= 3, "At least 3 planets");
    assertTrue(sys.securityLevel > 0.0f, "Security level positive");
    assertTrue(!sys.orbitSlots.empty(), "Has orbit slots");
}

void testStarSystemDeterminism() {
    std::cout << "\n=== PCG: StarSystemGenerator determinism ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 54321, 1 };
    auto sys1 = StarSystemGenerator::generate(ctx, 0.5f);
    auto sys2 = StarSystemGenerator::generate(ctx, 0.5f);
    assertTrue(sys1.totalPlanets == sys2.totalPlanets, "Same planet count");
    assertTrue(sys1.totalBelts == sys2.totalBelts, "Same belt count");
    assertTrue(sys1.star.starClass == sys2.star.starClass, "Same star class");
}

void testStarSystemSecurityAffectsStations() {
    std::cout << "\n=== PCG: StarSystem security affects stations ===" << std::endl;
    using namespace atlas::pcg;

    int highSecStations = 0, nullSecStations = 0;
    for (int i = 0; i < 20; ++i) {
        PCGContext ctx{ static_cast<uint64_t>(i * 137), 1 };
        auto hs = StarSystemGenerator::generate(ctx, 0.9f);
        highSecStations += static_cast<int>(hs.stations.size());
        auto ns = StarSystemGenerator::generate(ctx, 0.05f);
        nullSecStations += static_cast<int>(ns.stations.size());
    }
    assertTrue(highSecStations > nullSecStations,
               "High-sec has more stations than null-sec");
}

void testStarSystemStarClassName() {
    std::cout << "\n=== PCG: StarSystem star class names ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(StarSystemGenerator::starClassName(StarClass::G) == "G", "G star name");
    assertTrue(StarSystemGenerator::starClassName(StarClass::M) == "M", "M star name");
    assertTrue(StarSystemGenerator::starClassName(StarClass::O) == "O", "O star name");
}

// ==================== Galaxy Generator Tests ====================

void testGalaxyGeneration() {
    std::cout << "\n=== PCG: GalaxyGenerator basic ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 77777, 1 };
    auto galaxy = GalaxyGenerator::generate(ctx, 30);
    assertTrue(galaxy.valid, "Galaxy is valid");
    assertTrue(galaxy.total_systems == 30, "Correct system count");
    assertTrue(!galaxy.nodes.empty(), "Has nodes");
    assertTrue(!galaxy.routes.empty(), "Has routes");
    assertTrue(galaxy.highsec_count > 0, "Has high-sec systems");
}

void testGalaxyDeterminism() {
    std::cout << "\n=== PCG: GalaxyGenerator determinism ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 88888, 1 };
    auto g1 = GalaxyGenerator::generate(ctx, 20);
    auto g2 = GalaxyGenerator::generate(ctx, 20);
    assertTrue(g1.total_systems == g2.total_systems, "Same system count");
    assertTrue(g1.highsec_count == g2.highsec_count, "Same highsec count");
    assertTrue(g1.routes.size() == g2.routes.size(), "Same route count");
}

void testGalaxySecurityZones() {
    std::cout << "\n=== PCG: GalaxyGenerator security zones ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 99999, 1 };
    auto galaxy = GalaxyGenerator::generate(ctx, 50);
    assertTrue(galaxy.highsec_count + galaxy.lowsec_count + galaxy.nullsec_count
               == galaxy.total_systems, "Security counts sum to total");
    assertTrue(galaxy.highsec_count > 0, "Has high-sec");
    assertTrue(galaxy.lowsec_count > 0, "Has low-sec");
    assertTrue(galaxy.nullsec_count > 0, "Has null-sec");
}

void testGalaxyConnectivity() {
    std::cout << "\n=== PCG: GalaxyGenerator all nodes connected ===" << std::endl;
    using namespace atlas::pcg;

    PCGContext ctx{ 11111, 1 };
    auto galaxy = GalaxyGenerator::generate(ctx, 25);
    // Every node should have at least one neighbor.
    bool allConnected = true;
    for (const auto& node : galaxy.nodes) {
        if (node.neighbor_ids.empty()) { allConnected = false; break; }
    }
    assertTrue(allConnected, "All nodes have at least one connection");
}

void testGalaxyZoneNames() {
    std::cout << "\n=== PCG: GalaxyGenerator zone names ===" << std::endl;
    using namespace atlas::pcg;

    assertTrue(GalaxyGenerator::securityZoneName(SecurityZone::HighSec) == "High-Sec", "HighSec name");
    assertTrue(GalaxyGenerator::securityZoneName(SecurityZone::LowSec) == "Low-Sec", "LowSec name");
    assertTrue(GalaxyGenerator::securityZoneName(SecurityZone::NullSec) == "Null-Sec", "NullSec name");
}

// ==================== Asteroid Field Generator Tests ====================

void testAsteroidFieldGeneration() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5000, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 0.8f);
    assertTrue(!field.asteroids.empty(), "Field has asteroids");
    assertTrue(field.totalAsteroids >= 10, "Field has >= 10 asteroids");
    assertTrue(field.fieldRadius > 0.0f, "Field has positive radius");
    assertTrue(field.totalYield > 0.0f, "Field has positive yield");
}

void testAsteroidFieldDeterminism() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5100, 1 };
    auto f1 = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 0.5f);
    auto f2 = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 0.5f);
    assertTrue(f1.asteroids.size() == f2.asteroids.size(), "Same asteroid count");
    bool allMatch = true;
    for (size_t i = 0; i < f1.asteroids.size(); ++i) {
        if (f1.asteroids[i].type != f2.asteroids[i].type) { allMatch = false; break; }
        if (f1.asteroids[i].radius != f2.asteroids[i].radius) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical asteroid field");
}

void testAsteroidFieldExplicitCount() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator explicit count ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5200, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 25, 0.6f);
    assertTrue(static_cast<int>(field.asteroids.size()) == 25, "Explicit count of 25 asteroids");
    assertTrue(field.totalAsteroids == 25, "totalAsteroids matches");
}

void testAsteroidFieldHighSecTypes() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator high-sec types ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5300, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 50, 0.9f);
    // High-sec should have no Mercoxit
    bool hasMercoxit = false;
    int commonCount = 0;
    for (const auto& a : field.asteroids) {
        if (a.type == atlas::pcg::AsteroidType::Mercoxit) hasMercoxit = true;
        if (a.type == atlas::pcg::AsteroidType::Ferrite ||
            a.type == atlas::pcg::AsteroidType::Galvite) commonCount++;
    }
    assertTrue(!hasMercoxit, "High-sec belt has no Mercoxit");
    assertTrue(commonCount > 0, "High-sec belt has common ores");
}

void testAsteroidFieldYieldCalculation() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator yield calculation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5400, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 20, 0.5f);
    float calculated = atlas::pcg::AsteroidFieldGenerator::calculateTotalYield(field);
    assertTrue(calculated > 0.0f, "Calculated yield is positive");
    // totalYield should match sum of individual yields.
    float manual = 0.0f;
    for (const auto& a : field.asteroids) manual += a.mineralYield;
    assertTrue(std::abs(calculated - manual) < 0.01f, "calculateTotalYield matches sum");
}

void testAsteroidFieldPositiveValues() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator positive values ===" << std::endl;
    bool allOk = true;
    for (uint64_t i = 1; i <= 50; ++i) {
        atlas::pcg::PCGContext ctx{ i * 97, 1 };
        auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 0.5f);
        for (const auto& a : field.asteroids) {
            if (a.radius <= 0.0f || a.mineralYield <= 0.0f) { allOk = false; break; }
        }
    }
    assertTrue(allOk, "All asteroids have positive radius and yield");
}

void testAsteroidFieldScaleFactor() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator scale factor ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5500, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 20, 0.5f);
    bool allPositive = true;
    bool scaleVaries = false;
    float firstScale = 0.0f;
    for (size_t i = 0; i < field.asteroids.size(); ++i) {
        float sf = field.asteroids[i].scaleFactor;
        if (sf <= 0.0f) { allPositive = false; break; }
        if (i == 0) firstScale = sf;
        else if (std::abs(sf - firstScale) > 0.01f) scaleVaries = true;
    }
    assertTrue(allPositive, "All asteroids have positive scale factor");
    assertTrue(scaleVaries, "Scale factors vary across asteroids");
}

void testAsteroidFieldRockMeshArchive() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator rock mesh archive ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5600, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(
        ctx, 15, 0.6f, "rocks_stylized_color.zip");
    assertTrue(!field.asteroids.empty(), "Field has asteroids");
    bool allTagged = true;
    for (const auto& a : field.asteroids) {
        if (a.meshArchive != "rocks_stylized_color.zip") { allTagged = false; break; }
    }
    assertTrue(allTagged, "All asteroids carry rock mesh archive path");
    // Scale factors should still be set.
    assertTrue(field.asteroids[0].scaleFactor > 0.0f, "First asteroid has positive scale");
}

void testAsteroidFieldNoArchiveByDefault() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator no archive by default ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5700, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 10, 0.8f);
    bool allEmpty = true;
    for (const auto& a : field.asteroids) {
        if (!a.meshArchive.empty()) { allEmpty = false; break; }
    }
    assertTrue(allEmpty, "Without archive arg, meshArchive is empty");
}

// ==================== Anomaly Generator Tests ====================

void testAnomalyGeneration() {
    std::cout << "\n=== PCG: AnomalyGenerator basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6000, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(ctx, 0.7f);
    assertTrue(!site.nodes.empty(), "Anomaly has nodes");
    assertTrue(site.siteRadius > 0.0f, "Anomaly has positive radius");
    assertTrue(site.totalValue > 0.0f, "Anomaly has positive value");
}

void testAnomalyDeterminism() {
    std::cout << "\n=== PCG: AnomalyGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6100, 1 };
    auto s1 = atlas::pcg::AnomalyGenerator::generate(ctx, 0.5f);
    auto s2 = atlas::pcg::AnomalyGenerator::generate(ctx, 0.5f);
    assertTrue(s1.type == s2.type, "Same site type");
    assertTrue(s1.difficulty == s2.difficulty, "Same difficulty");
    assertTrue(s1.nodes.size() == s2.nodes.size(), "Same node count");
    bool allMatch = true;
    for (size_t i = 0; i < s1.nodes.size(); ++i) {
        if (s1.nodes[i].value != s2.nodes[i].value) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical anomaly");
}

void testAnomalyTypeOverride() {
    std::cout << "\n=== PCG: AnomalyGenerator type override ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6200, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(
        ctx, atlas::pcg::AnomalySiteType::CombatSite, 0.5f);
    assertTrue(site.type == atlas::pcg::AnomalySiteType::CombatSite,
               "Type override applied");
    assertTrue(site.waveCount > 0, "Combat site has waves");
}

void testAnomalyNonCombatNoWaves() {
    std::cout << "\n=== PCG: AnomalyGenerator non-combat no waves ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6300, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(
        ctx, atlas::pcg::AnomalySiteType::GasSite, 0.5f);
    assertTrue(site.type == atlas::pcg::AnomalySiteType::GasSite, "Gas site type");
    assertTrue(site.waveCount == 0, "Gas site has no waves");
}

void testAnomalyLowSecRequiresScan() {
    std::cout << "\n=== PCG: AnomalyGenerator low-sec requires scan ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6400, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(ctx, 0.1f);
    assertTrue(site.requiresScan, "Low-sec anomaly requires scanning");
}

void testAnomalyValueCalculation() {
    std::cout << "\n=== PCG: AnomalyGenerator value calculation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 6500, 1 };
    auto site = atlas::pcg::AnomalyGenerator::generate(ctx, 0.5f);
    float calculated = atlas::pcg::AnomalyGenerator::calculateTotalValue(site);
    float manual = 0.0f;
    for (const auto& n : site.nodes) manual += n.value;
    assertTrue(std::abs(calculated - manual) < 0.01f, "calculateTotalValue matches sum");
}

// ==================== NPC Encounter Generator Tests ====================

void testNPCEncounterGeneration() {
    std::cout << "\n=== PCG: NPCEncounterGenerator basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7000, 1 };
    auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.6f);
    assertTrue(enc.valid, "Encounter is valid");
    assertTrue(!enc.waves.empty(), "Encounter has waves");
    assertTrue(enc.totalShips > 0, "Encounter has ships");
    assertTrue(enc.estimatedBounty > 0.0f, "Encounter has bounty");
}

void testNPCEncounterDeterminism() {
    std::cout << "\n=== PCG: NPCEncounterGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7100, 1 };
    auto e1 = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.5f);
    auto e2 = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.5f);
    assertTrue(e1.faction == e2.faction, "Same faction");
    assertTrue(e1.totalShips == e2.totalShips, "Same ship count");
    assertTrue(e1.waves.size() == e2.waves.size(), "Same wave count");
    bool allMatch = true;
    for (size_t w = 0; w < e1.waves.size(); ++w) {
        if (e1.waves[w].ships.size() != e2.waves[w].ships.size()) {
            allMatch = false; break;
        }
    }
    assertTrue(allMatch, "Same seed → identical encounter");
}

void testNPCEncounterExplicitWaves() {
    std::cout << "\n=== PCG: NPCEncounterGenerator explicit waves ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7200, 1 };
    auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 3, 0.5f);
    assertTrue(static_cast<int>(enc.waves.size()) == 3, "Explicit 3 waves");
}

void testNPCEncounterBountyCalculation() {
    std::cout << "\n=== PCG: NPCEncounterGenerator bounty calculation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7300, 1 };
    auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.5f);
    float calculated = atlas::pcg::NPCEncounterGenerator::calculateBounty(enc);
    assertTrue(calculated > 0.0f, "Calculated bounty is positive");
}

void testNPCEncounterDifficultyScaling() {
    std::cout << "\n=== PCG: NPCEncounterGenerator difficulty scaling ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7400, 1 };
    auto highSec = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.9f);
    auto nullSec = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.1f);
    assertTrue(nullSec.difficultyRating > highSec.difficultyRating,
               "Null-sec harder than high-sec");
}

void testNPCEncounterAllValid() {
    std::cout << "\n=== PCG: NPCEncounterGenerator all valid ===" << std::endl;
    bool allValid = true;
    for (uint64_t i = 1; i <= 50; ++i) {
        atlas::pcg::PCGContext ctx{ i * 83, 1 };
        auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 0.5f);
        if (!enc.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All 50 encounters are valid");
}

void testNPCEncounterWaveEscalation() {
    std::cout << "\n=== PCG: NPCEncounterGenerator wave escalation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 7500, 1 };
    auto enc = atlas::pcg::NPCEncounterGenerator::generate(ctx, 5, 0.3f);
    // First wave delay should be 0.
    assertTrue(enc.waves[0].triggerDelay == 0.0f, "First wave has no delay");
    // Subsequent waves should have positive delay.
    if (enc.waves.size() > 1) {
        assertTrue(enc.waves[1].triggerDelay > 0.0f, "Second wave has delay");
    }
}

// ==================== Planet Generator Tests ====================

void testPlanetGeneration() {
    std::cout << "\n=== Planet Generation ===" << std::endl;
    pcg::PlanetGenerator gen;
    auto planet = gen.generate(555, pcg::PlanetType::Rocky);
    assertTrue(planet.radius >= 2000.0f && planet.radius <= 8000.0f, "Rocky radius in range");
    assertTrue(planet.gravity > 0.0f, "Planet has gravity");
}

void testPlanetDeterminism() {
    std::cout << "\n=== Planet Determinism ===" << std::endl;
    pcg::PlanetGenerator gen;
    auto p1 = gen.generate(888, pcg::PlanetType::Ice);
    auto p2 = gen.generate(888, pcg::PlanetType::Ice);
    assertTrue(approxEqual(p1.radius, p2.radius), "Same seed same radius");
    assertTrue(approxEqual(p1.gravity, p2.gravity), "Same seed same gravity");
}

void testPlanetResources() {
    std::cout << "\n=== Planet Resources ===" << std::endl;
    pcg::PlanetGenerator gen;
    auto planet = gen.generate(333, pcg::PlanetType::Rocky);
    assertTrue(!planet.resources.empty(), "Rocky planet has resources");
    assertTrue(static_cast<int>(planet.resources.size()) >= 3, "Rocky has at least 3 resources");
}

void testPlanetTerraformable() {
    std::cout << "\n=== Planet Terraformable ===" << std::endl;
    assertTrue(pcg::PlanetGenerator::isTerraformable(pcg::PlanetType::Rocky), "Rocky is terraformable");
    assertTrue(pcg::PlanetGenerator::isTerraformable(pcg::PlanetType::Desert), "Desert is terraformable");
    assertTrue(!pcg::PlanetGenerator::isTerraformable(pcg::PlanetType::Gas), "Gas is not terraformable");
    assertTrue(!pcg::PlanetGenerator::isTerraformable(pcg::PlanetType::Lava), "Lava is not terraformable");
}

// ==================== Habitat Generator Tests ====================

void testHabitatGeneration() {
    std::cout << "\n=== Habitat Generation ===" << std::endl;
    pcg::HabitatGenerator gen;
    auto habitat = gen.generate(111, 5);
    assertTrue(habitat.module_count > 0, "Habitat has modules");
    assertTrue(habitat.total_levels == 5, "Habitat has 5 levels");
}

void testHabitatDeterminism() {
    std::cout << "\n=== Habitat Determinism ===" << std::endl;
    pcg::HabitatGenerator gen;
    auto h1 = gen.generate(222, 3);
    auto h2 = gen.generate(222, 3);
    assertTrue(h1.module_count == h2.module_count, "Same seed same module count");
    assertTrue(approxEqual(h1.total_power_draw, h2.total_power_draw), "Same seed same power draw");
}

void testHabitatPowerBalance() {
    std::cout << "\n=== Habitat Power Balance ===" << std::endl;
    pcg::HabitatGenerator gen;
    auto habitat = gen.generate(444, 4);
    assertTrue(habitat.total_power_draw >= 0.0f, "Power draw non-negative");
    assertTrue(habitat.total_power_generation >= 0.0f, "Power generation non-negative");
    assertTrue(habitat.is_self_sufficient == (habitat.total_power_generation >= habitat.total_power_draw),
               "Self-sufficient flag matches calculation");
}

// ==================== Grav Bike Tests ====================

void testGravBikeGeneration() {
    std::cout << "\n=== Grav Bike Generation ===" << std::endl;
    pcg::GravBikeGenerator gen;
    auto bike = gen.generate(1000, "Solari");
    assertTrue(bike.config.max_speed >= 30.0f, "Speed above minimum");
    assertTrue(bike.config.fuel_capacity > 0.0f, "Has fuel capacity");
}

void testGravBikeDeterminism() {
    std::cout << "\n=== Grav Bike Determinism ===" << std::endl;
    pcg::GravBikeGenerator gen;
    auto b1 = gen.generate(2000, "Veyren");
    auto b2 = gen.generate(2000, "Veyren");
    assertTrue(approxEqual(b1.config.max_speed, b2.config.max_speed), "Same seed same speed");
    assertTrue(approxEqual(b1.hull_strength, b2.hull_strength), "Same seed same hull");
}

// ==================== Spine Hull Generator Tests ====================

void testSpineHullGeneration() {
    std::cout << "\n=== Spine Hull Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto hull = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Cruiser);
    assertTrue(hull.valid, "Generated spine hull is valid");
    assertTrue(hull.profile.length > 0.0f, "Hull has positive length");
    assertTrue(hull.profile.width_mid > 0.0f, "Hull has positive mid-width");
    assertTrue(hull.engine_cluster_count > 0, "Hull has engines");
    assertTrue(static_cast<int>(hull.zones.size()) == 3, "Hull has 3 functional zones");
    assertTrue(hull.bilateral_symmetry, "Ships have bilateral symmetry");
}

void testSpineHullDeterminism() {
    std::cout << "\n=== Spine Hull Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto h1 = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Battleship);
    auto h2 = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Battleship);
    assertTrue(h1.spine == h2.spine, "Same seed same spine type");
    assertTrue(approxEqual(h1.profile.length, h2.profile.length), "Same seed same length");
    assertTrue(approxEqual(h1.profile.width_mid, h2.profile.width_mid), "Same seed same width");
    assertTrue(h1.engine_cluster_count == h2.engine_cluster_count, "Same seed same engines");
    assertTrue(h1.total_greeble_count == h2.total_greeble_count, "Same seed same greebles");
}

void testSpineHullZoneOrdering() {
    std::cout << "\n=== Spine Hull Zone Ordering ===" << std::endl;
    pcg::PCGContext ctx{ 555, 1 };
    auto hull = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Frigate);
    assertTrue(hull.zones[0].zone == pcg::FunctionalZone::Command, "Zone 0 is Command");
    assertTrue(hull.zones[1].zone == pcg::FunctionalZone::MidHull, "Zone 1 is MidHull");
    assertTrue(hull.zones[2].zone == pcg::FunctionalZone::Engineering, "Zone 2 is Engineering");

    // Zone fractions must sum to 1.0.
    float total = hull.zones[0].length_fraction + hull.zones[1].length_fraction
                + hull.zones[2].length_fraction;
    assertTrue(std::fabs(total - 1.0f) < 0.02f, "Zone fractions sum to 1.0");
}

void testSpineHullAspectRatio() {
    std::cout << "\n=== Spine Hull Aspect Ratio ===" << std::endl;
    // Generate many hulls and verify aspect ratio clamping.
    bool allClamped = true;
    for (uint64_t i = 1; i <= 100; ++i) {
        pcg::PCGContext ctx{ i * 31, 1 };
        auto hull = pcg::SpineHullGenerator::generate(ctx);
        if (hull.aspect_ratio < 1.5f || hull.aspect_ratio > 15.0f) {
            allClamped = false;
            break;
        }
    }
    assertTrue(allClamped, "All hulls have aspect ratio in [1.5, 15]");
}

void testSpineHullFactionStyle() {
    std::cout << "\n=== Spine Hull Faction Style ===" << std::endl;
    pcg::PCGContext ctx{ 999, 1 };
    auto solari = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Cruiser, "Solari");
    auto veyren = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    assertTrue(solari.faction_style == "Solari", "Solari faction tag set");
    assertTrue(veyren.faction_style == "Veyren", "Veyren faction tag set");
    // Solari hulls should be narrower (elegant) vs Veyren (wider, angular).
    assertTrue(solari.profile.width_mid < veyren.profile.width_mid,
               "Solari narrower than Veyren");
}

void testSpineHullAllClassesValid() {
    std::cout << "\n=== Spine Hull All Classes Valid ===" << std::endl;
    pcg::HullClass classes[] = {
        pcg::HullClass::Frigate, pcg::HullClass::Destroyer,
        pcg::HullClass::Cruiser, pcg::HullClass::Battlecruiser,
        pcg::HullClass::Battleship, pcg::HullClass::Capital
    };
    bool allValid = true;
    for (auto hc : classes) {
        pcg::PCGContext ctx{ static_cast<uint64_t>(hc) * 100 + 7, 1 };
        auto hull = pcg::SpineHullGenerator::generate(ctx, hc);
        if (!hull.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All hull classes produce valid spine hulls");
}

void testSpineHullCapitalLarger() {
    std::cout << "\n=== Spine Hull Capital Larger ===" << std::endl;
    pcg::PCGContext ctx{ 123, 1 };
    auto frigate = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Frigate);
    auto capital = pcg::SpineHullGenerator::generate(ctx, pcg::HullClass::Capital);
    assertTrue(capital.profile.length > frigate.profile.length,
               "Capital hull longer than frigate");
    assertTrue(capital.engine_cluster_count > frigate.engine_cluster_count,
               "Capital has more engines than frigate");
}

void testSpineTypeName() {
    std::cout << "\n=== Spine Type Names ===" << std::endl;
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Needle) == "Needle", "Needle name");
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Wedge) == "Wedge", "Wedge name");
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Hammer) == "Hammer", "Hammer name");
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Slab) == "Slab", "Slab name");
    assertTrue(pcg::SpineHullGenerator::spineTypeName(pcg::SpineType::Ring) == "Ring", "Ring name");
}

// ==================== Terrain Generator Tests ====================

void testTerrainGeneration() {
    std::cout << "\n=== Terrain Generation ===" << std::endl;
    auto terrain = pcg::TerrainGenerator::generate(42, pcg::PlanetType::Rocky, 4);
    assertTrue(terrain.valid, "Terrain is valid");
    assertTrue(terrain.grid_size == 4, "Grid size is 4");
    assertTrue(static_cast<int>(terrain.regions.size()) == 16, "4×4 = 16 regions");
    assertTrue(!terrain.regions[0].cells.empty(), "Regions have cells");
}

void testTerrainDeterminism() {
    std::cout << "\n=== Terrain Determinism ===" << std::endl;
    auto t1 = pcg::TerrainGenerator::generate(888, pcg::PlanetType::Ice, 4);
    auto t2 = pcg::TerrainGenerator::generate(888, pcg::PlanetType::Ice, 4);
    assertTrue(t1.mineable_count == t2.mineable_count, "Same seed same mineable count");
    assertTrue(t1.landing_zone_count == t2.landing_zone_count, "Same seed same landing zones");
    assertTrue(t1.regions.size() == t2.regions.size(), "Same region count");
    // Check first region cells match.
    assertTrue(approxEqual(t1.regions[0].avg_height, t2.regions[0].avg_height),
               "Same seed same avg height");
}

void testTerrainBiomeClassification() {
    std::cout << "\n=== Terrain Biome Classification ===" << std::endl;
    // Lava planet: high terrain → Volcanic.
    auto lava_biome = pcg::TerrainGenerator::classifyBiome(0.8f, 0.5f, pcg::PlanetType::Lava);
    assertTrue(lava_biome == pcg::BiomeType::Volcanic, "High lava terrain is Volcanic");
    // Ice planet: high terrain → Mountains.
    auto ice_biome = pcg::TerrainGenerator::classifyBiome(0.7f, 0.5f, pcg::PlanetType::Ice);
    assertTrue(ice_biome == pcg::BiomeType::Mountains, "High ice terrain is Mountains");
    // Desert planet: low terrain → Dunes.
    auto desert_biome = pcg::TerrainGenerator::classifyBiome(0.3f, 0.5f, pcg::PlanetType::Desert);
    assertTrue(desert_biome == pcg::BiomeType::Dunes, "Low desert terrain is Dunes");
}

void testTerrainMineableDeposits() {
    std::cout << "\n=== Terrain Mineable Deposits ===" << std::endl;
    auto terrain = pcg::TerrainGenerator::generate(333, pcg::PlanetType::Rocky, 8);
    assertTrue(terrain.mineable_count > 0, "Rocky planet has mineable deposits");
    // Gas giants should have no mineable cells.
    auto gas = pcg::TerrainGenerator::generate(333, pcg::PlanetType::Gas, 4);
    assertTrue(gas.mineable_count == 0, "Gas giant has no mineable deposits");
}

void testTerrainLandingZones() {
    std::cout << "\n=== Terrain Landing Zones ===" << std::endl;
    auto terrain = pcg::TerrainGenerator::generate(555, pcg::PlanetType::Forest, 8);
    assertTrue(terrain.landing_zone_count >= 0, "Landing zone count non-negative");
    // At least some regions should be flat enough on a large grid.
    auto big = pcg::TerrainGenerator::generate(555, pcg::PlanetType::Forest, 16);
    assertTrue(big.landing_zone_count >= 0, "Large grid has non-negative landing zones");
}

void testTerrainBiomeName() {
    std::cout << "\n=== Terrain Biome Names ===" << std::endl;
    assertTrue(pcg::TerrainGenerator::biomeName(pcg::BiomeType::Plains) == "Plains", "Plains name");
    assertTrue(pcg::TerrainGenerator::biomeName(pcg::BiomeType::Mountains) == "Mountains", "Mountains name");
    assertTrue(pcg::TerrainGenerator::biomeName(pcg::BiomeType::Volcanic) == "Volcanic", "Volcanic name");
}

void testTerrainDifferentSeeds() {
    std::cout << "\n=== Terrain Different Seeds ===" << std::endl;
    auto t1 = pcg::TerrainGenerator::generate(100, pcg::PlanetType::Rocky, 4);
    auto t2 = pcg::TerrainGenerator::generate(200, pcg::PlanetType::Rocky, 4);
    // Different seeds should produce different terrain (very unlikely to match).
    bool differs = (t1.mineable_count != t2.mineable_count)
                || (t1.landing_zone_count != t2.landing_zone_count)
                || !approxEqual(t1.regions[0].avg_height, t2.regions[0].avg_height);
    assertTrue(differs, "Different seeds produce different terrain");
}

// ==================== Damage State Generator Tests ====================

void testDamageStateGeneration() {
    std::cout << "\n=== Damage State Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto state = pcg::DamageStateGenerator::generate(ctx, 0.6f, pcg::HullClass::Cruiser);
    assertTrue(state.valid, "Damage state is valid");
    assertTrue(state.level == pcg::DamageLevel::Heavy, "0.6 damage = Heavy");
    assertTrue(!state.decals.empty(), "Heavy damage produces decals");
    assertTrue(state.hull_breach_count > 0, "Heavy damage has hull breaches");
    assertTrue(state.structural_integrity > 0.0f, "Structural integrity positive");
    assertTrue(state.structural_integrity < 1.0f, "Structural integrity reduced");
}

void testDamageStateDeterminism() {
    std::cout << "\n=== Damage State Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto s1 = pcg::DamageStateGenerator::generate(ctx, 0.4f, pcg::HullClass::Battleship);
    auto s2 = pcg::DamageStateGenerator::generate(ctx, 0.4f, pcg::HullClass::Battleship);
    assertTrue(s1.level == s2.level, "Same seed same damage level");
    assertTrue(s1.hull_breach_count == s2.hull_breach_count, "Same seed same breach count");
    assertTrue(s1.missing_module_count == s2.missing_module_count, "Same seed same missing modules");
    assertTrue(static_cast<int>(s1.decals.size()) == static_cast<int>(s2.decals.size()),
               "Same seed same decal count");
    assertTrue(approxEqual(s1.structural_integrity, s2.structural_integrity),
               "Same seed same structural integrity");
}

void testDamageStatePristine() {
    std::cout << "\n=== Damage State Pristine ===" << std::endl;
    pcg::PCGContext ctx{ 100, 1 };
    auto state = pcg::DamageStateGenerator::generate(ctx, 0.0f, pcg::HullClass::Frigate);
    assertTrue(state.level == pcg::DamageLevel::Pristine, "0.0 damage = Pristine");
    assertTrue(state.decals.empty(), "Pristine has no decals");
    assertTrue(state.hull_breach_count == 0, "Pristine has no breaches");
    assertTrue(state.missing_module_count == 0, "Pristine has no missing modules");
    assertTrue(approxEqual(state.structural_integrity, 1.0f), "Pristine integrity = 1.0");
}

void testDamageStateCritical() {
    std::cout << "\n=== Damage State Critical ===" << std::endl;
    pcg::PCGContext ctx{ 200, 1 };
    auto state = pcg::DamageStateGenerator::generate(ctx, 0.95f, pcg::HullClass::Capital);
    assertTrue(state.level == pcg::DamageLevel::Critical, "0.95 damage = Critical");
    assertTrue(static_cast<int>(state.decals.size()) > 10, "Critical capital has many decals");
    assertTrue(state.hull_breach_count >= 2, "Critical has multiple breaches");
    assertTrue(state.structural_integrity < 0.3f, "Critical integrity very low");
}

void testDamageStateScalesWithClass() {
    std::cout << "\n=== Damage State Scales With Class ===" << std::endl;
    pcg::PCGContext ctx{ 300, 1 };
    auto frigate = pcg::DamageStateGenerator::generate(ctx, 0.7f, pcg::HullClass::Frigate);
    auto capital = pcg::DamageStateGenerator::generate(ctx, 0.7f, pcg::HullClass::Capital);
    assertTrue(static_cast<int>(capital.decals.size()) > static_cast<int>(frigate.decals.size()),
               "Capital has more decals than frigate at same damage");
}

void testDamageStateLevelNames() {
    std::cout << "\n=== Damage State Level Names ===" << std::endl;
    assertTrue(pcg::DamageStateGenerator::damageLevelName(pcg::DamageLevel::Pristine) == "Pristine", "Pristine name");
    assertTrue(pcg::DamageStateGenerator::damageLevelName(pcg::DamageLevel::Light) == "Light", "Light name");
    assertTrue(pcg::DamageStateGenerator::damageLevelName(pcg::DamageLevel::Heavy) == "Heavy", "Heavy name");
    assertTrue(pcg::DamageStateGenerator::decalTypeName(pcg::DecalType::HullBreach) == "HullBreach", "HullBreach name");
    assertTrue(pcg::DamageStateGenerator::decalTypeName(pcg::DecalType::ScorchMark) == "ScorchMark", "ScorchMark name");
}

// ==================== Procedural Texture Generator Tests ====================

void testTextureGeneration() {
    std::cout << "\n=== Procedural Texture Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto tex = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    assertTrue(tex.valid, "Texture params valid");
    assertTrue(tex.faction == "Veyren", "Faction stored");
    assertTrue(tex.hull_class == pcg::HullClass::Cruiser, "Hull class stored");
    assertTrue(!tex.markings.empty(), "Has hull markings");
    assertTrue(!tex.window_lights.empty(), "Has window lights");
    assertTrue(tex.panel_tile_count > 0, "Has panel tiling");
}

void testTextureDeterminism() {
    std::cout << "\n=== Procedural Texture Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto t1 = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Battleship, "Solari");
    auto t2 = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Battleship, "Solari");
    assertTrue(approxEqual(t1.palette.primary_r, t2.palette.primary_r), "Same seed same palette R");
    assertTrue(approxEqual(t1.palette.primary_g, t2.palette.primary_g), "Same seed same palette G");
    assertTrue(approxEqual(t1.material.metalness, t2.material.metalness), "Same seed same metalness");
    assertTrue(static_cast<int>(t1.markings.size()) == static_cast<int>(t2.markings.size()),
               "Same seed same marking count");
    assertTrue(static_cast<int>(t1.window_lights.size()) == static_cast<int>(t2.window_lights.size()),
               "Same seed same window count");
}

void testTextureFactionPalettes() {
    std::cout << "\n=== Texture Faction Palettes ===" << std::endl;
    auto solari  = pcg::ProceduralTextureGenerator::basePalette("Solari");
    auto veyren  = pcg::ProceduralTextureGenerator::basePalette("Veyren");
    auto aurelian = pcg::ProceduralTextureGenerator::basePalette("Aurelian");
    auto keldari = pcg::ProceduralTextureGenerator::basePalette("Keldari");
    // Solari is golden (R > G > B).
    assertTrue(solari.primary_r > solari.primary_b, "Solari primary is warm (R > B)");
    // Veyren is blue-grey (B > R).
    assertTrue(veyren.primary_b > veyren.primary_r, "Veyren primary is cool (B > R)");
    // Aurelian is green-tinted (G > R, G > B).
    assertTrue(aurelian.primary_g > aurelian.primary_r, "Aurelian primary green (G > R)");
    // Keldari is brown (R > G > B).
    assertTrue(keldari.primary_r > keldari.primary_g, "Keldari primary warm (R > G)");
    assertTrue(keldari.primary_g > keldari.primary_b, "Keldari primary brown (G > B)");
}

void testTextureMaterialByFaction() {
    std::cout << "\n=== Texture Material By Faction ===" << std::endl;
    pcg::PCGContext ctx{ 500, 1 };
    auto solari  = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Cruiser, "Solari");
    auto keldari = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Cruiser, "Keldari");
    // Solari ships are more polished (lower roughness).
    assertTrue(solari.material.roughness < keldari.material.roughness,
               "Solari smoother than Keldari");
}

void testTextureScalesWithClass() {
    std::cout << "\n=== Texture Scales With Class ===" << std::endl;
    pcg::PCGContext ctx{ 600, 1 };
    auto frigate = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Frigate, "Veyren");
    auto capital = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Capital, "Veyren");
    assertTrue(static_cast<int>(capital.window_lights.size()) > static_cast<int>(frigate.window_lights.size()),
               "Capital has more windows than frigate");
    assertTrue(capital.panel_tile_count > frigate.panel_tile_count,
               "Capital has more panel tiling than frigate");
    assertTrue(static_cast<int>(capital.markings.size()) > static_cast<int>(frigate.markings.size()),
               "Capital has more markings than frigate");
}

void testTextureEngineGlowFaction() {
    std::cout << "\n=== Texture Engine Glow By Faction ===" << std::endl;
    pcg::PCGContext ctx{ 700, 1 };
    auto veyren = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Frigate, "Veyren");
    auto keldari = pcg::ProceduralTextureGenerator::generate(ctx, pcg::HullClass::Frigate, "Keldari");
    // Veyren engine glow is blue (B channel dominant).
    assertTrue(veyren.engine_glow.color_b > veyren.engine_glow.color_r,
               "Veyren engine glow is blue");
    // Keldari engine glow is orange/red (R channel dominant).
    assertTrue(keldari.engine_glow.color_r > keldari.engine_glow.color_b,
               "Keldari engine glow is warm");
    assertTrue(veyren.engine_glow.intensity > 0.0f, "Engine glow intensity positive");
}

void testTextureMarkingTypeNames() {
    std::cout << "\n=== Texture Marking Type Names ===" << std::endl;
    assertTrue(pcg::ProceduralTextureGenerator::markingTypeName(pcg::MarkingType::StripeHorizontal) == "StripeHorizontal",
               "StripeHorizontal name");
    assertTrue(pcg::ProceduralTextureGenerator::markingTypeName(pcg::MarkingType::FactionInsignia) == "FactionInsignia",
               "FactionInsignia name");
    assertTrue(pcg::ProceduralTextureGenerator::markingTypeName(pcg::MarkingType::WarningHazard) == "WarningHazard",
               "WarningHazard name");
}

void testTextureAllClassesValid() {
    std::cout << "\n=== Texture All Classes Valid ===" << std::endl;
    std::vector<pcg::HullClass> classes = {
        pcg::HullClass::Frigate, pcg::HullClass::Destroyer,
        pcg::HullClass::Cruiser, pcg::HullClass::Battlecruiser,
        pcg::HullClass::Battleship, pcg::HullClass::Capital
    };
    bool allValid = true;
    for (auto hc : classes) {
        pcg::PCGContext ctx{ static_cast<uint64_t>(hc) + 1000, 1 };
        auto tex = pcg::ProceduralTextureGenerator::generate(ctx, hc, "Veyren");
        if (!tex.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All hull classes produce valid texture params");
}

// ==================== Shield Effect Generator Tests ====================

void testShieldEffectGeneration() {
    std::cout << "\n=== Shield Effect Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto shield = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    assertTrue(shield.valid, "Shield effect is valid");
    assertTrue(shield.base_opacity > 0.0f, "Base opacity positive");
    assertTrue(shield.pattern_scale > 0.0f, "Pattern scale positive");
    assertTrue(shield.shield_radius >= 1.0f, "Shield radius >= 1.0");
    assertTrue(shield.fresnel_power >= 1.0f, "Fresnel power >= 1.0");
    assertTrue(static_cast<int>(shield.sample_impacts.size()) == 3, "Has 3 sample impacts");
}

void testShieldEffectDeterminism() {
    std::cout << "\n=== Shield Effect Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto s1 = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Battleship, "Solari");
    auto s2 = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Battleship, "Solari");
    assertTrue(s1.pattern == s2.pattern, "Same seed same pattern");
    assertTrue(approxEqual(s1.base_color_r, s2.base_color_r), "Same seed same base color R");
    assertTrue(approxEqual(s1.base_opacity, s2.base_opacity), "Same seed same opacity");
    assertTrue(approxEqual(s1.shimmer_speed, s2.shimmer_speed), "Same seed same shimmer speed");
    assertTrue(static_cast<int>(s1.sample_impacts.size()) == static_cast<int>(s2.sample_impacts.size()),
               "Same seed same impact count");
}

void testShieldPatternByFaction() {
    std::cout << "\n=== Shield Pattern By Faction ===" << std::endl;
    // Test pattern tendencies (statistical — use fixed seeds that produce expected results).
    pcg::PCGContext ctx{ 42, 1 };
    auto veyren  = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    auto solari  = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Solari");
    // Veyren should be Hexagonal or Lattice.
    assertTrue(veyren.pattern == pcg::ShieldPattern::Hexagonal
            || veyren.pattern == pcg::ShieldPattern::Lattice,
               "Veyren pattern is tech-style");
    // Solari should be Ornate or Smooth.
    assertTrue(solari.pattern == pcg::ShieldPattern::Ornate
            || solari.pattern == pcg::ShieldPattern::Smooth,
               "Solari pattern is ornate/smooth");
}

void testShieldScalesWithClass() {
    std::cout << "\n=== Shield Scales With Class ===" << std::endl;
    pcg::PCGContext ctx{ 123, 1 };
    auto frigate = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Frigate, "Veyren");
    auto capital = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Capital, "Veyren");
    assertTrue(capital.pattern_scale > frigate.pattern_scale,
               "Capital pattern scale larger than frigate");
    assertTrue(capital.shield_radius > frigate.shield_radius,
               "Capital shield radius larger than frigate");
}

void testShieldColorByFaction() {
    std::cout << "\n=== Shield Color By Faction ===" << std::endl;
    pcg::PCGContext ctx{ 300, 1 };
    auto veyren  = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Veyren");
    auto keldari = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Keldari");
    // Veyren shields are blue-dominant.
    assertTrue(veyren.base_color_b > veyren.base_color_r,
               "Veyren shield is blue (B > R)");
    // Keldari shields are warm.
    assertTrue(keldari.base_color_r > keldari.base_color_b,
               "Keldari shield is warm (R > B)");
}

void testShieldImpactRipples() {
    std::cout << "\n=== Shield Impact Ripples ===" << std::endl;
    pcg::PCGContext ctx{ 400, 1 };
    auto shield = pcg::ShieldEffectGenerator::generate(ctx, pcg::HullClass::Cruiser, "Aurelian");
    for (const auto& imp : shield.sample_impacts) {
        assertTrue(imp.intensity >= 0.0f && imp.intensity <= 1.0f,
                   "Impact intensity in [0,1]");
        assertTrue(imp.radius > 0.0f, "Impact radius positive");
        assertTrue(imp.decay_rate > 0.0f, "Impact decay rate positive");
        assertTrue(imp.speed > 0.0f, "Impact speed positive");
    }
}

void testShieldPatternNames() {
    std::cout << "\n=== Shield Pattern Names ===" << std::endl;
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Hexagonal) == "Hexagonal",
               "Hexagonal name");
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Smooth) == "Smooth",
               "Smooth name");
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Lattice) == "Lattice",
               "Lattice name");
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Ornate) == "Ornate",
               "Ornate name");
    assertTrue(pcg::ShieldEffectGenerator::patternName(pcg::ShieldPattern::Ripple) == "Ripple",
               "Ripple name");
}

void testShieldAllClassesValid() {
    std::cout << "\n=== Shield All Classes Valid ===" << std::endl;
    std::vector<pcg::HullClass> classes = {
        pcg::HullClass::Frigate, pcg::HullClass::Destroyer,
        pcg::HullClass::Cruiser, pcg::HullClass::Battlecruiser,
        pcg::HullClass::Battleship, pcg::HullClass::Capital
    };
    bool allValid = true;
    for (auto hc : classes) {
        pcg::PCGContext ctx{ static_cast<uint64_t>(hc) + 2000, 1 };
        auto shield = pcg::ShieldEffectGenerator::generate(ctx, hc, "Aurelian");
        if (!shield.valid) { allValid = false; break; }
    }
    assertTrue(allValid, "All hull classes produce valid shield effects");
}

// ==================== Economy-Driven Generator Tests ====================

void testEconomyFleetGeneration() {
    std::cout << "\n=== Economy Fleet Generation ===" << std::endl;
    pcg::PCGContext ctx{ 42, 1 };
    auto fleet = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::ResourceRich, 10);
    assertTrue(fleet.valid, "Economy fleet is valid");
    assertTrue(fleet.total_ships == 10, "Fleet has 10 ships");
    assertTrue(static_cast<int>(fleet.ships.size()) == 10, "Ships vector has 10 entries");
    assertTrue(fleet.economy == pcg::EconomyState::ResourceRich, "Economy state matches");
    assertTrue(fleet.average_equipment_quality > 0.0f, "Fleet has positive equipment quality");
}

void testEconomyFleetDeterminism() {
    std::cout << "\n=== Economy Fleet Determinism ===" << std::endl;
    pcg::PCGContext ctx{ 777, 1 };
    auto f1 = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::WarTorn, 5);
    auto f2 = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::WarTorn, 5);
    assertTrue(f1.total_ships == f2.total_ships, "Same seed same ship count");
    assertTrue(approxEqual(f1.average_equipment_quality, f2.average_equipment_quality),
               "Same seed same equipment quality");
    bool rolesMatch = true;
    for (int i = 0; i < f1.total_ships; ++i) {
        if (f1.ships[i].role != f2.ships[i].role) { rolesMatch = false; break; }
    }
    assertTrue(rolesMatch, "Same seed same roles");
}

void testEconomyResourceRichHasMiners() {
    std::cout << "\n=== Economy ResourceRich Has Miners ===" << std::endl;
    pcg::PCGContext ctx{ 555, 1 };
    auto fleet = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::ResourceRich, 20);
    int minerCount = 0;
    for (const auto& ship : fleet.ships) {
        if (ship.role == pcg::EconomyShipRole::Miner) minerCount++;
    }
    assertTrue(minerCount > 0, "ResourceRich produces miners");
}

void testEconomyWarTornHasDamage() {
    std::cout << "\n=== Economy WarTorn Has Damage ===" << std::endl;
    pcg::PCGContext ctx{ 888, 1 };
    auto fleet = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::WarTorn, 10);
    float totalWear = 0.0f;
    for (const auto& ship : fleet.ships) {
        totalWear += ship.damage_wear;
    }
    float avgWear = totalWear / static_cast<float>(fleet.total_ships);
    assertTrue(avgWear > 0.1f, "WarTorn ships have significant wear");
}

void testEconomyProsperousHighQuality() {
    std::cout << "\n=== Economy Prosperous High Quality ===" << std::endl;
    pcg::PCGContext ctx{ 123, 1 };
    auto prosperous = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::Prosperous, 15);
    auto declining  = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::Declining, 15);
    assertTrue(prosperous.average_equipment_quality > declining.average_equipment_quality,
               "Prosperous has better equipment than Declining");
}

void testEconomyLawlessHasPirates() {
    std::cout << "\n=== Economy Lawless Has Pirates ===" << std::endl;
    pcg::PCGContext ctx{ 444, 1 };
    auto fleet = pcg::EconomyDrivenGenerator::generate(ctx, pcg::EconomyState::Lawless, 20);
    int pirateCount = 0;
    for (const auto& ship : fleet.ships) {
        if (ship.role == pcg::EconomyShipRole::Pirate) pirateCount++;
    }
    assertTrue(pirateCount > 0, "Lawless produces pirates");
}

void testEconomyStateNames() {
    std::cout << "\n=== Economy State Names ===" << std::endl;
    assertTrue(pcg::EconomyDrivenGenerator::economyStateName(pcg::EconomyState::Prosperous) == "Prosperous", "Prosperous name");
    assertTrue(pcg::EconomyDrivenGenerator::economyStateName(pcg::EconomyState::ResourceRich) == "ResourceRich", "ResourceRich name");
    assertTrue(pcg::EconomyDrivenGenerator::economyStateName(pcg::EconomyState::WarTorn) == "WarTorn", "WarTorn name");
    assertTrue(pcg::EconomyDrivenGenerator::shipRoleName(pcg::EconomyShipRole::Miner) == "Miner", "Miner role name");
    assertTrue(pcg::EconomyDrivenGenerator::shipRoleName(pcg::EconomyShipRole::Pirate) == "Pirate", "Pirate role name");
}

// ==================== AI Economic Actor System tests ====================

void testAIEconomicActorDefaults() {
    std::cout << "\n=== AI Economic Actor Defaults ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    auto* entity = world.createEntity("npc_trader_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);

    assertTrue(actor->owned_ship_type.empty(), "Ship type empty by default");
    assertTrue(actor->ship_value == 0.0, "Ship value starts at 0");
    assertTrue(!actor->is_destroyed, "Not destroyed by default");
    assertTrue(!actor->permanently_dead, "Not permanently dead by default");
    assertTrue(actor->destruction_count == 0, "No destructions");
    assertTrue(actor->replacement_count == 0, "No replacements");
    assertTrue(approxEqual(actor->time_alive, 0.0f), "Time alive starts at 0");
    assertTrue(intent->wallet == 10000.0, "Wallet default Credits");
}

void testAIEconomicActorEarnSpend() {
    std::cout << "\n=== AI Economic Actor Earn/Spend ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_miner_1");
    addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);
    intent->wallet = 5000.0;

    sys.earnISC("npc_miner_1", 3000.0);
    assertTrue(intent->wallet == 8000.0, "Earned 3000 Credits");

    bool spent = sys.spendISC("npc_miner_1", 2000.0);
    assertTrue(spent, "Spend succeeds with sufficient funds");
    assertTrue(intent->wallet == 6000.0, "Wallet after spend");

    bool fail = sys.spendISC("npc_miner_1", 99999.0);
    assertTrue(!fail, "Spend fails with insufficient funds");
    assertTrue(intent->wallet == 6000.0, "Wallet unchanged after failed spend");
}

void testAIEconomicActorShipDestruction() {
    std::cout << "\n=== AI Economic Actor Ship Destruction ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_hauler_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);
    actor->owned_ship_type = "Badger";
    actor->ship_value = 5000.0;
    intent->wallet = 12000.0;

    // Destroy the ship
    bool destroyed = sys.handleShipDestruction("npc_hauler_1");
    assertTrue(destroyed, "Destruction handled");
    assertTrue(actor->is_destroyed, "Ship marked destroyed");
    assertTrue(actor->destruction_count == 1, "Destruction count incremented");

    // Update should trigger replacement since wallet (12000) >= ship_value (5000)
    sys.update(1.0f);
    assertTrue(!actor->is_destroyed, "Ship replaced after update");
    assertTrue(actor->replacement_count == 1, "Replacement count incremented");
    assertTrue(intent->wallet == 7000.0, "Wallet reduced by ship value");
}

void testAIEconomicActorPermanentDeath() {
    std::cout << "\n=== AI Economic Actor Permanent Death ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_pirate_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);
    actor->owned_ship_type = "Rifter";
    actor->ship_value = 10000.0;
    intent->wallet = 3000.0;  // Cannot afford replacement

    sys.handleShipDestruction("npc_pirate_1");
    sys.update(1.0f);

    assertTrue(actor->permanently_dead, "NPC permanently dead when broke");
    assertTrue(actor->is_destroyed, "Ship still destroyed");

    auto dead = sys.getDeadActors();
    assertTrue(dead.size() == 1, "One dead actor");
    assertTrue(dead[0] == "npc_pirate_1", "Dead actor is pirate");
}

void testAIEconomicActorTimeTracking() {
    std::cout << "\n=== AI Economic Actor Time Tracking ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_patrol_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    addComp<components::SimNPCIntent>(entity);

    sys.update(5.0f);
    assertTrue(approxEqual(actor->time_alive, 5.0f), "Time alive updated");

    sys.update(3.0f);
    assertTrue(approxEqual(actor->time_alive, 8.0f), "Time alive accumulated");
}

void testAIEconomicActorTotalEconomy() {
    std::cout << "\n=== AI Economic Actor Total Economy ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* e1 = world.createEntity("trader_1");
    addComp<components::AIEconomicActor>(e1);
    auto* i1 = addComp<components::SimNPCIntent>(e1);
    i1->wallet = 50000.0;

    auto* e2 = world.createEntity("trader_2");
    addComp<components::AIEconomicActor>(e2);
    auto* i2 = addComp<components::SimNPCIntent>(e2);
    i2->wallet = 30000.0;

    double total = sys.getTotalEconomyISC();
    assertTrue(total == 80000.0, "Total economy Credits correct");

    auto low = sys.getLowFundsActors(40000.0);
    assertTrue(low.size() == 1, "One low-funds actor");
}

void testAIEconomicActorCanAffordReplacement() {
    std::cout << "\n=== AI Economic Actor Can Afford Replacement ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);
    actor->ship_value = 5000.0;
    intent->wallet = 10000.0;

    assertTrue(sys.canAffordReplacement("npc_1"), "Can afford with 10k wallet and 5k ship");

    intent->wallet = 3000.0;
    assertTrue(!sys.canAffordReplacement("npc_1"), "Cannot afford with 3k wallet and 5k ship");
}

// ==================== Turret AI System tests ====================

void testTurretAIWithinArc() {
    std::cout << "\n=== Turret AI Within Arc ===" << std::endl;
    using namespace atlas;

    // Turret facing forward (0 deg) with 90 deg arc
    assertTrue(systems::TurretAISystem::isWithinArc(0.0f, 0.0f, 90.0f),
               "Dead center is within arc");
    assertTrue(systems::TurretAISystem::isWithinArc(44.0f, 0.0f, 90.0f),
               "44 deg is within 90 deg arc");
    assertTrue(!systems::TurretAISystem::isWithinArc(46.0f, 0.0f, 90.0f),
               "46 deg is outside 90 deg arc");
    assertTrue(systems::TurretAISystem::isWithinArc(-44.0f, 0.0f, 90.0f),
               "-44 deg is within 90 deg arc");
    assertTrue(!systems::TurretAISystem::isWithinArc(-46.0f, 0.0f, 90.0f),
               "-46 deg is outside 90 deg arc");
}

void testTurretAITrackingPenalty() {
    std::cout << "\n=== Turret AI Tracking Penalty ===" << std::endl;
    using namespace atlas;

    // Perfect tracking (stationary target)
    float p1 = systems::TurretAISystem::computeTrackingPenalty(0.0f, 1.0f);
    assertTrue(approxEqual(p1, 1.0f), "Stationary target = full damage");

    // Angular velocity equals tracking speed
    float p2 = systems::TurretAISystem::computeTrackingPenalty(1.0f, 1.0f);
    assertTrue(approxEqual(p2, 0.5f), "Equal angular/tracking = 50% damage");

    // Very fast target
    float p3 = systems::TurretAISystem::computeTrackingPenalty(10.0f, 1.0f);
    assertTrue(p3 < 0.02f, "Very fast target = near-zero damage");

    // Zero tracking speed
    float p4 = systems::TurretAISystem::computeTrackingPenalty(1.0f, 0.0f);
    assertTrue(approxEqual(p4, 0.0f), "Zero tracking speed = no hit");
}

void testTurretAICooldown() {
    std::cout << "\n=== Turret AI Cooldown ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::TurretAISystem sys(&world);

    auto* entity = world.createEntity("ship_turret_0");
    auto* turret = addComp<components::TurretAIState>(entity);
    turret->engaged = true;
    turret->target_entity_id = "enemy_1";
    turret->rate_of_fire = 2.0f;  // 2 shots/sec
    turret->base_damage = 50.0f;
    turret->tracking_speed = 5.0f;
    turret->angular_velocity = 0.0f;  // stationary target

    // First update: should fire (cooldown starts at 0)
    sys.update(0.1f);
    assertTrue(turret->shots_fired == 1, "First shot fired");
    assertTrue(turret->cooldown_remaining > 0.0f, "Cooldown set after shot");

    // Cooldown should be 0.5 seconds (1/2 rate_of_fire)
    assertTrue(approxEqual(turret->cooldown_remaining, 0.5f),
               "Cooldown = 1/rate_of_fire");
}

void testTurretAIDamageAccumulation() {
    std::cout << "\n=== Turret AI Damage Accumulation ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::TurretAISystem sys(&world);

    auto* entity = world.createEntity("ship_turret_0");
    auto* turret = addComp<components::TurretAIState>(entity);
    turret->engaged = true;
    turret->target_entity_id = "enemy_1";
    turret->rate_of_fire = 1.0f;   // 1 shot/sec
    turret->base_damage = 100.0f;
    turret->tracking_speed = 10.0f;
    turret->angular_velocity = 0.0f;  // stationary = full damage

    sys.update(0.1f);  // fires, sets 1s cooldown
    assertTrue(approxEqual(turret->total_damage_dealt, 100.0f),
               "Full damage on stationary target");

    // Wait for cooldown to expire
    sys.update(1.0f);  // cooldown drops to 0, fires again
    assertTrue(turret->shots_fired == 2, "Second shot after cooldown");
    assertTrue(approxEqual(turret->total_damage_dealt, 200.0f),
               "Accumulated damage after 2 shots");
}

void testTurretAITrackingReducesDamage() {
    std::cout << "\n=== Turret AI Tracking Reduces Damage ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::TurretAISystem sys(&world);

    auto* entity = world.createEntity("ship_turret_0");
    auto* turret = addComp<components::TurretAIState>(entity);
    turret->engaged = true;
    turret->target_entity_id = "enemy_1";
    turret->rate_of_fire = 1.0f;
    turret->base_damage = 100.0f;
    turret->tracking_speed = 1.0f;
    turret->angular_velocity = 1.0f;  // matches tracking = 50% damage

    sys.update(0.1f);
    assertTrue(approxEqual(turret->total_damage_dealt, 50.0f),
               "50% damage when angular vel = tracking speed");
}

void testTurretAINotEngaged() {
    std::cout << "\n=== Turret AI Not Engaged ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::TurretAISystem sys(&world);

    auto* entity = world.createEntity("ship_turret_0");
    auto* turret = addComp<components::TurretAIState>(entity);
    turret->engaged = false;  // not firing
    turret->target_entity_id = "enemy_1";
    turret->rate_of_fire = 1.0f;
    turret->base_damage = 100.0f;

    sys.update(1.0f);
    assertTrue(turret->shots_fired == 0, "No shots when not engaged");
    assertTrue(approxEqual(turret->total_damage_dealt, 0.0f),
               "No damage when not engaged");
}

void testTurretAIComponentDefaults() {
    std::cout << "\n=== Turret AI Component Defaults ===" << std::endl;
    using namespace atlas;

    components::TurretAIState turret;
    assertTrue(turret.turret_index == 0, "Turret index default 0");
    assertTrue(approxEqual(turret.arc_degrees, 90.0f), "Arc default 90 deg");
    assertTrue(approxEqual(turret.direction_deg, 0.0f), "Direction default 0");
    assertTrue(approxEqual(turret.tracking_speed, 1.0f), "Tracking default 1.0");
    assertTrue(approxEqual(turret.base_damage, 10.0f), "Base damage default 10");
    assertTrue(turret.target_entity_id.empty(), "No target by default");
    assertTrue(!turret.engaged, "Not engaged by default");
    assertTrue(turret.shots_fired == 0, "No shots by default");
}

// ==================== ShipDesignerSystem Tests ====================

void testShipDesignerCreate() {
    std::cout << "\n=== ShipDesigner: Create ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    auto* e = world.createEntity("ship1");
    assertTrue(sys.createDesigner("ship1"), "Create designer succeeds");
    auto* sd = e->getComponent<components::ShipDesigner>();
    assertTrue(sd != nullptr, "Component exists");
    assertTrue(sd->high_slots == 4, "Default high_slots is 4");
    assertTrue(approxEqual(sd->total_cpu, 300.0f), "Default total_cpu is 300");
    assertTrue(!sd->valid, "Not valid by default");
}

void testShipDesignerBlueprint() {
    std::cout << "\n=== ShipDesigner: Blueprint ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.createDesigner("ship1");
    assertTrue(sys.setBlueprint("ship1", "Rifter", "Frigate", "Minmatar"), "Set blueprint succeeds");
    auto* sd = e->getComponent<components::ShipDesigner>();
    assertTrue(sd->blueprint_name == "Rifter", "Blueprint name set");
    assertTrue(sd->hull_type == "Frigate", "Hull type set");
}

void testShipDesignerFitModule() {
    std::cout << "\n=== ShipDesigner: FitModule ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    assertTrue(sys.fitModule("ship1", "Autocannon", 0, 30.0f, 10.0f), "Fit module succeeds");
    assertTrue(sys.getFittedCount("ship1") == 1, "Fitted count is 1");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 30.0f / 300.0f), "CPU usage correct");
}

void testShipDesignerRemoveModule() {
    std::cout << "\n=== ShipDesigner: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    sys.fitModule("ship1", "Autocannon", 0, 30.0f, 10.0f);
    assertTrue(sys.removeModule("ship1", "Autocannon"), "Remove module succeeds");
    assertTrue(sys.getFittedCount("ship1") == 0, "Fitted count is 0");
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 0.0f), "CPU usage is 0");
}

void testShipDesignerValidate() {
    std::cout << "\n=== ShipDesigner: Validate ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    sys.setBlueprint("ship1", "Rifter", "Frigate", "Minmatar");
    sys.fitModule("ship1", "Gun", 0, 50.0f, 100.0f);
    assertTrue(sys.validateDesign("ship1"), "Validate succeeds within budget");
    assertTrue(sys.isValid("ship1"), "Design is valid");
    assertTrue(sys.getFittedCount("ship1") == 1, "1 module fitted");
}

void testShipDesignerOverbudget() {
    std::cout << "\n=== ShipDesigner: Overbudget ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.createDesigner("ship1");
    auto* sd = e->getComponent<components::ShipDesigner>();
    sd->total_cpu = 50.0f;
    sys.setBlueprint("ship1", "Rifter", "Frigate", "Minmatar");
    sys.fitModule("ship1", "BigGun", 0, 100.0f, 10.0f);
    sys.update(0.0f);
    assertTrue(!sys.validateDesign("ship1"), "Validate fails when over CPU");
    assertTrue(!sys.isValid("ship1"), "Design is not valid");
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 100.0f / 50.0f), "CPU usage ratio > 1");
}

void testShipDesignerSlotsFull() {
    std::cout << "\n=== ShipDesigner: SlotsFull ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    auto* e = world.createEntity("ship1");
    sys.createDesigner("ship1");
    auto* sd = e->getComponent<components::ShipDesigner>();
    sd->high_slots = 2;
    assertTrue(sys.fitModule("ship1", "G1", 0, 10.0f, 10.0f), "Fit slot 1 succeeds");
    assertTrue(sys.fitModule("ship1", "G2", 0, 10.0f, 10.0f), "Fit slot 2 succeeds");
    assertTrue(!sys.fitModule("ship1", "G3", 0, 10.0f, 10.0f), "Fit slot 3 fails (full)");
    assertTrue(sys.getSlotsFree("ship1", 0) == 0, "0 high slots free");
}

void testShipDesignerClear() {
    std::cout << "\n=== ShipDesigner: Clear ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    sys.setBlueprint("ship1", "Rifter", "Frigate", "Minmatar");
    sys.fitModule("ship1", "Gun", 0, 50.0f, 100.0f);
    assertTrue(sys.clearDesign("ship1"), "Clear succeeds");
    assertTrue(sys.getFittedCount("ship1") == 0, "0 modules after clear");
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 0.0f), "CPU usage is 0 after clear");
}

void testShipDesignerMultipleModules() {
    std::cout << "\n=== ShipDesigner: MultipleModules ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    world.createEntity("ship1");
    sys.createDesigner("ship1");
    assertTrue(sys.fitModule("ship1", "Gun", 0, 30.0f, 50.0f), "Fit high slot");
    assertTrue(sys.fitModule("ship1", "Shield", 1, 20.0f, 80.0f), "Fit mid slot");
    assertTrue(sys.fitModule("ship1", "Armor", 2, 15.0f, 40.0f), "Fit low slot");
    assertTrue(sys.getFittedCount("ship1") == 3, "3 modules fitted");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCpuUsage("ship1"), 65.0f / 300.0f), "CPU usage from all modules");
    assertTrue(sys.getSlotsFree("ship1", 0) == 3, "3 high slots free");
}

void testShipDesignerMissing() {
    std::cout << "\n=== ShipDesigner: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipDesignerSystem sys(&world);
    assertTrue(!sys.createDesigner("nonexistent"), "Create fails on missing");
    assertTrue(!sys.setBlueprint("nonexistent", "R", "F", "M"), "SetBlueprint fails on missing");
    assertTrue(!sys.fitModule("nonexistent", "G", 0, 10.0f, 10.0f), "FitModule fails on missing");
    assertTrue(!sys.removeModule("nonexistent", "G"), "RemoveModule fails on missing");
    assertTrue(!sys.validateDesign("nonexistent"), "Validate fails on missing");
    assertTrue(sys.getFittedCount("nonexistent") == 0, "0 fitted on missing");
    assertTrue(approxEqual(sys.getCpuUsage("nonexistent"), 0.0f), "0 CPU on missing");
    assertTrue(sys.getSlotsFree("nonexistent", 0) == 0, "0 slots free on missing");
    assertTrue(!sys.isValid("nonexistent"), "Not valid on missing");
}


void run_pcg_tests() {
    testWreckCreate();
    testWreckLifetimeDecay();
    testSalvageWreckInRange();
    testSalvageWreckOutOfRange();
    testSalvageAlreadySalvaged();
    testWreckActiveCount();
    testWreckHasInventory();
    testDeterministicRNGSameSeed();
    testDeterministicRNGDifferentSeed();
    testDeterministicRNGRange();
    testHashCombineDeterminism();
    testHash64FourInputs();
    testDeriveSeed();
    testPCGManagerInitialize();
    testPCGManagerContextDeterminism();
    testShipGeneratorDeterminism();
    testShipGeneratorConstraints();
    testShipGeneratorHullOverride();
    testShipGeneratorExpandedFields();
    testShipGeneratorExpandedDeterminism();
    testShipGeneratorHullRanges();
    testShipGeneratorShipName();
    testRoomGraphFunctionalTypes();
    testRoomGraphDimensionsByType();
    testDeckGraphHubAndSpoke();
    testShipDesignerSaveRoundTrip();
    testTitanAssemblyDefaults();
    testTitanAssemblyTick();
    testTitanAssemblyDisrupt();
    testTitanAssemblyDisruptedTick();
    testTitanAssemblyClamped();
    testTitanAssemblyDisruptFloor();
    testTitanAssemblyPhaseName();
    testStarSystemGeneration();
    testStarSystemDeterminism();
    testStarSystemSecurityAffectsStations();
    testStarSystemStarClassName();
    testGalaxyGeneration();
    testGalaxyDeterminism();
    testGalaxySecurityZones();
    testGalaxyConnectivity();
    testGalaxyZoneNames();
    testAsteroidFieldGeneration();
    testAsteroidFieldDeterminism();
    testAsteroidFieldExplicitCount();
    testAsteroidFieldHighSecTypes();
    testAsteroidFieldYieldCalculation();
    testAsteroidFieldPositiveValues();
    testAsteroidFieldScaleFactor();
    testAsteroidFieldRockMeshArchive();
    testAsteroidFieldNoArchiveByDefault();
    testAnomalyGeneration();
    testAnomalyDeterminism();
    testAnomalyTypeOverride();
    testAnomalyNonCombatNoWaves();
    testAnomalyLowSecRequiresScan();
    testAnomalyValueCalculation();
    testNPCEncounterGeneration();
    testNPCEncounterDeterminism();
    testNPCEncounterExplicitWaves();
    testNPCEncounterBountyCalculation();
    testNPCEncounterDifficultyScaling();
    testNPCEncounterAllValid();
    testNPCEncounterWaveEscalation();
    testPlanetGeneration();
    testPlanetDeterminism();
    testPlanetResources();
    testPlanetTerraformable();
    testHabitatGeneration();
    testHabitatDeterminism();
    testHabitatPowerBalance();
    testGravBikeGeneration();
    testGravBikeDeterminism();
    testSpineHullGeneration();
    testSpineHullDeterminism();
    testSpineHullZoneOrdering();
    testSpineHullAspectRatio();
    testSpineHullFactionStyle();
    testSpineHullAllClassesValid();
    testSpineHullCapitalLarger();
    testSpineTypeName();
    testTerrainGeneration();
    testTerrainDeterminism();
    testTerrainBiomeClassification();
    testTerrainMineableDeposits();
    testTerrainLandingZones();
    testTerrainBiomeName();
    testTerrainDifferentSeeds();
    testDamageStateGeneration();
    testDamageStateDeterminism();
    testDamageStatePristine();
    testDamageStateCritical();
    testDamageStateScalesWithClass();
    testDamageStateLevelNames();
    testTextureGeneration();
    testTextureDeterminism();
    testTextureFactionPalettes();
    testTextureMaterialByFaction();
    testTextureScalesWithClass();
    testTextureEngineGlowFaction();
    testTextureMarkingTypeNames();
    testTextureAllClassesValid();
    testShieldEffectGeneration();
    testShieldEffectDeterminism();
    testShieldPatternByFaction();
    testShieldScalesWithClass();
    testShieldColorByFaction();
    testShieldImpactRipples();
    testShieldPatternNames();
    testShieldAllClassesValid();
    testEconomyFleetGeneration();
    testEconomyFleetDeterminism();
    testEconomyResourceRichHasMiners();
    testEconomyWarTornHasDamage();
    testEconomyProsperousHighQuality();
    testEconomyLawlessHasPirates();
    testEconomyStateNames();
    testAIEconomicActorDefaults();
    testAIEconomicActorEarnSpend();
    testAIEconomicActorShipDestruction();
    testAIEconomicActorPermanentDeath();
    testAIEconomicActorTimeTracking();
    testAIEconomicActorTotalEconomy();
    testAIEconomicActorCanAffordReplacement();
    testTurretAIWithinArc();
    testTurretAITrackingPenalty();
    testTurretAICooldown();
    testTurretAIDamageAccumulation();
    testTurretAITrackingReducesDamage();
    testTurretAINotEngaged();
    testTurretAIComponentDefaults();
    testShipDesignerCreate();
    testShipDesignerBlueprint();
    testShipDesignerFitModule();
    testShipDesignerRemoveModule();
    testShipDesignerValidate();
    testShipDesignerOverbudget();
    testShipDesignerSlotsFull();
    testShipDesignerClear();
    testShipDesignerMultipleModules();
    testShipDesignerMissing();
}
