/**
 * Social tests
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

// ==================== Corporation System Tests ====================

void testCorpCreate() {
    std::cout << "\n=== Corporation Create ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* player = world.createEntity("player1");
    auto* pc = addComp<components::Player>(player);
    pc->player_id = "player1";
    pc->character_name = "TestPilot";

    assertTrue(corpSys.createCorporation("player1", "Test Corp", "TSTC"),
               "Corporation created");

    auto* corp_entity = world.getEntity("corp_test_corp");
    assertTrue(corp_entity != nullptr, "Corp entity exists");

    auto* corp = corp_entity->getComponent<components::Corporation>();
    assertTrue(corp != nullptr, "Corporation component exists");
    assertTrue(corp->ceo_id == "player1", "CEO is the creator");
    assertTrue(corp->corp_name == "Test Corp", "Corp name set");
    assertTrue(corp->ticker == "TSTC", "Ticker set");
    assertTrue(corpSys.getMemberCount("corp_test_corp") == 1, "One member after creation");
    assertTrue(pc->corporation == "Test Corp", "Player corporation updated");
}

void testCorpJoin() {
    std::cout << "\n=== Corporation Join ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    auto* p2 = world.createEntity("player2");
    auto* pc2 = addComp<components::Player>(p2);
    pc2->player_id = "player2";

    corpSys.createCorporation("player1", "Join Corp", "JNCO");

    assertTrue(corpSys.joinCorporation("player2", "corp_join_corp"),
               "Player2 joins corp");
    assertTrue(corpSys.getMemberCount("corp_join_corp") == 2, "Two members after join");
    assertTrue(pc2->corporation == "Join Corp", "Player2 corporation updated");
    assertTrue(!corpSys.joinCorporation("player2", "corp_join_corp"),
               "Duplicate join rejected");
}

void testCorpLeave() {
    std::cout << "\n=== Corporation Leave ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    auto* p2 = world.createEntity("player2");
    auto* pc2 = addComp<components::Player>(p2);
    pc2->player_id = "player2";

    corpSys.createCorporation("player1", "Leave Corp", "LVCO");
    corpSys.joinCorporation("player2", "corp_leave_corp");

    assertTrue(corpSys.leaveCorporation("player2", "corp_leave_corp"),
               "Player2 leaves corp");
    assertTrue(corpSys.getMemberCount("corp_leave_corp") == 1, "One member after leave");
    assertTrue(pc2->corporation == "NPC Corp", "Player2 corporation reset");
}

void testCorpCeoCannotLeave() {
    std::cout << "\n=== Corporation CEO Cannot Leave ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    corpSys.createCorporation("player1", "CEO Corp", "CEOC");

    assertTrue(!corpSys.leaveCorporation("player1", "corp_ceo_corp"),
               "CEO cannot leave corporation");
    assertTrue(corpSys.getMemberCount("corp_ceo_corp") == 1, "Member count unchanged");
}

void testCorpTaxRate() {
    std::cout << "\n=== Corporation Tax Rate ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    auto* p2 = world.createEntity("player2");
    auto* pc2 = addComp<components::Player>(p2);
    pc2->player_id = "player2";

    corpSys.createCorporation("player1", "Tax Corp", "TAXC");
    corpSys.joinCorporation("player2", "corp_tax_corp");

    assertTrue(corpSys.setTaxRate("corp_tax_corp", "player1", 0.10f),
               "CEO can set tax rate");
    auto* corp = world.getEntity("corp_tax_corp")->getComponent<components::Corporation>();
    assertTrue(approxEqual(corp->tax_rate, 0.10f), "Tax rate updated to 10%");

    assertTrue(!corpSys.setTaxRate("corp_tax_corp", "player2", 0.20f),
               "Non-CEO cannot set tax rate");
    assertTrue(approxEqual(corp->tax_rate, 0.10f), "Tax rate unchanged");
}

void testCorpApplyTax() {
    std::cout << "\n=== Corporation Apply Tax ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    corpSys.createCorporation("player1", "Wallet Corp", "WLTC");
    corpSys.setTaxRate("corp_wallet_corp", "player1", 0.10f);

    double remaining = corpSys.applyTax("corp_wallet_corp", 1000.0);
    assertTrue(approxEqual(static_cast<float>(remaining), 900.0f), "Remaining Credits after 10% tax");

    auto* corp = world.getEntity("corp_wallet_corp")->getComponent<components::Corporation>();
    assertTrue(approxEqual(static_cast<float>(corp->corp_wallet), 100.0f), "Corp wallet received tax");
}

void testSerializeDeserializeCorporation() {
    std::cout << "\n=== Serialize/Deserialize Corporation ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("corp_test");
    auto* corp = addComp<components::Corporation>(entity);
    corp->corp_id = "corp_test";
    corp->corp_name = "Serialize Corp";
    corp->ticker = "SRLZ";
    corp->ceo_id = "player1";
    corp->tax_rate = 0.15f;
    corp->corp_wallet = 50000.0;
    corp->member_ids.push_back("player1");
    corp->member_ids.push_back("player2");

    components::Corporation::CorpHangarItem item;
    item.item_id = "stellium"; item.name = "Stellium";
    item.type = "ore"; item.quantity = 1000; item.volume = 0.01f;
    corp->hangar_items.push_back(item);

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "Corporation deserialization succeeds");

    auto* e2 = world2.getEntity("corp_test");
    assertTrue(e2 != nullptr, "Corp entity recreated");

    auto* corp2 = e2->getComponent<components::Corporation>();
    assertTrue(corp2 != nullptr, "Corporation component recreated");
    assertTrue(corp2->corp_name == "Serialize Corp", "corp_name preserved");
    assertTrue(corp2->ticker == "SRLZ", "ticker preserved");
    assertTrue(corp2->ceo_id == "player1", "ceo_id preserved");
    assertTrue(approxEqual(corp2->tax_rate, 0.15f), "tax_rate preserved");
    assertTrue(approxEqual(static_cast<float>(corp2->corp_wallet), 50000.0f), "corp_wallet preserved");
    assertTrue(corp2->member_ids.size() == 2, "member_ids count preserved");
    assertTrue(corp2->member_ids[0] == "player1", "member_ids[0] preserved");
    assertTrue(corp2->member_ids[1] == "player2", "member_ids[1] preserved");
    assertTrue(corp2->hangar_items.size() == 1, "hangar_items count preserved");
    assertTrue(corp2->hangar_items[0].item_id == "stellium", "hangar item_id preserved");
    assertTrue(corp2->hangar_items[0].quantity == 1000, "hangar item quantity preserved");
}

// ==================== Reputation System Tests ====================

void testReputationInstallRelationships() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    float disp = sys.getFactionDisposition("Solari", "Veyren");
    assertTrue(disp < 0.0f, "Solari-Veyren are rivals (negative disposition)");

    float ally = sys.getFactionDisposition("Solari", "Aurelian");
    assertTrue(ally > 0.0f, "Solari-Aurelian are friendly (positive disposition)");
}

void testReputationPirateHostile() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    float disp = sys.getFactionDisposition("Solari", "Serpentis");
    assertTrue(approxEqual(disp, -1.0f, 0.01f), "Player factions hostile to pirates");
}

void testReputationModifyStanding() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    auto* entity = world.createEntity("player1");
    addComp<components::Standings>(entity);

    sys.modifyFactionStanding("player1", "Solari", 2.0f);
    float standing = sys.getEffectiveStanding("player1", "Solari");
    assertTrue(approxEqual(standing, 2.0f, 0.01f), "Direct standing applied");
}

void testReputationDerivedEffects() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    auto* entity = world.createEntity("player1");
    addComp<components::Standings>(entity);

    // Gaining standing with Solari should affect allies/enemies
    sys.modifyFactionStanding("player1", "Solari", 4.0f);

    // Aurelian is friendly (0.3) → derived = 4.0 * 0.3 * 0.5 = 0.6
    float aurelian = sys.getEffectiveStanding("player1", "Aurelian");
    assertTrue(aurelian > 0.0f, "Derived positive standing with ally");

    // Veyren is rival (-0.5) → derived = 4.0 * -0.5 * 0.5 = -1.0
    float veyren = sys.getEffectiveStanding("player1", "Veyren");
    assertTrue(veyren < 0.0f, "Derived negative standing with rival");
}

void testReputationAgentAccess() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    auto* entity = world.createEntity("player1");
    addComp<components::Standings>(entity);

    assertTrue(!sys.hasAgentAccess("player1", "Solari", 1.0f),
               "No access with 0 standing");

    sys.modifyFactionStanding("player1", "Solari", 3.0f);
    assertTrue(sys.hasAgentAccess("player1", "Solari", 1.0f),
               "Access with sufficient standing");
    assertTrue(!sys.hasAgentAccess("player1", "Solari", 5.0f),
               "No access when standing insufficient");
}

void testReputationStandingClamped() {
    ecs::World world;
    systems::ReputationSystem sys(&world);
    sys.installFactionRelationships();

    auto* entity = world.createEntity("player1");
    addComp<components::Standings>(entity);

    sys.modifyFactionStanding("player1", "Solari", 15.0f);
    float standing = sys.getEffectiveStanding("player1", "Solari");
    assertTrue(standing <= 10.0f, "Standing clamped to max 10");

    sys.modifyFactionStanding("player1", "Veyren", -15.0f);
    float neg = sys.getEffectiveStanding("player1", "Veyren");
    assertTrue(neg >= -10.0f, "Standing clamped to min -10");
}

// ==================== Phase 9: Rumor Propagation Tests ====================

void testRumorPropagationNewRumor() {
    std::cout << "\n=== Rumor Propagation New Rumor ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* speaker = world.createEntity("speaker");
    auto* speakerLog = addComp<components::RumorLog>(speaker);
    speakerLog->addRumor("ancient_gate", "There's an old gate near Sigma-7", true);

    auto* listener = world.createEntity("listener");
    // No RumorLog yet

    sys.propagateRumor("speaker", "listener");

    auto* listenerLog = listener->getComponent<components::RumorLog>();
    assertTrue(listenerLog != nullptr, "Listener gained RumorLog");
    assertTrue(listenerLog->hasRumor("ancient_gate"), "Rumor propagated to listener");
    // Second-hand: belief should be halved
    float belief = 0.0f;
    for (const auto& r : listenerLog->rumors) {
        if (r.rumor_id == "ancient_gate") belief = r.belief_strength;
    }
    assertTrue(approxEqual(belief, 0.25f, 0.01f), "Second-hand belief is halved (0.25)");
}

void testRumorPropagationReinforces() {
    std::cout << "\n=== Rumor Propagation Reinforces ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* speaker = world.createEntity("speaker");
    auto* speakerLog = addComp<components::RumorLog>(speaker);
    speakerLog->addRumor("derelict_ship", "Derelict spotted in belt", true);

    auto* listener = world.createEntity("listener");
    auto* listenerLog = addComp<components::RumorLog>(listener);
    listenerLog->addRumor("derelict_ship", "Derelict spotted in belt", false);

    float initialBelief = 0.0f;
    for (const auto& r : listenerLog->rumors) {
        if (r.rumor_id == "derelict_ship") initialBelief = r.belief_strength;
    }

    sys.propagateRumor("speaker", "listener");

    float newBelief = 0.0f;
    int timesHeard = 0;
    for (const auto& r : listenerLog->rumors) {
        if (r.rumor_id == "derelict_ship") {
            newBelief = r.belief_strength;
            timesHeard = r.times_heard;
        }
    }
    assertTrue(newBelief > initialBelief, "Belief reinforced after hearing again");
    assertTrue(timesHeard == 2, "Times heard incremented");
}

void testRumorPropagationNoRumors() {
    std::cout << "\n=== Rumor Propagation No Rumors ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* speaker = world.createEntity("speaker");
    addComp<components::RumorLog>(speaker);  // empty rumor log

    auto* listener = world.createEntity("listener");

    sys.propagateRumor("speaker", "listener");

    auto* listenerLog = listener->getComponent<components::RumorLog>();
    assertTrue(listenerLog == nullptr, "No propagation when speaker has no rumors");
}

// ==================== Rumor Questline System Tests ====================

void testRumorQuestlineNoGraduation() {
    std::cout << "\n=== Rumor Questline No Graduation ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rumor1");
    auto* log = addComp<components::RumorLog>(e);
    log->addRumor("r1", "Some rumor", false);
    log->rumors[0].times_heard = 2;
    log->rumors[0].belief_strength = 0.8f;

    systems::RumorQuestlineSystem sys(&world);
    sys.update(1.0f);
    assertTrue(!sys.hasGraduatedRumor("rumor1", "r1"), "Rumor not graduated with < 3 hearings");
}

void testRumorQuestlineGraduates() {
    std::cout << "\n=== Rumor Questline Graduates ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rumor2");
    auto* log = addComp<components::RumorLog>(e);
    log->addRumor("r1", "A strong rumor", false);
    log->rumors[0].times_heard = 3;
    log->rumors[0].belief_strength = 0.8f;

    systems::RumorQuestlineSystem sys(&world);
    sys.update(1.0f);
    assertTrue(sys.hasGraduatedRumor("rumor2", "r1"), "Rumor graduated with >= 3 hearings and belief >= 0.7");
    auto quests = sys.getGraduatedQuestlines("rumor2");
    assertTrue(quests.size() == 1, "One questline generated");
}

void testRumorQuestlineLowBeliefNoGraduation() {
    std::cout << "\n=== Rumor Questline Low Belief ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rumor3");
    auto* log = addComp<components::RumorLog>(e);
    log->addRumor("r1", "Weak rumor", false);
    log->rumors[0].times_heard = 5;
    log->rumors[0].belief_strength = 0.3f;

    systems::RumorQuestlineSystem sys(&world);
    sys.update(1.0f);
    assertTrue(!sys.hasGraduatedRumor("rumor3", "r1"), "Low belief prevents graduation");
}

// ==================== Local Reputation System Tests ====================

void testLocalReputationDefaults() {
    std::cout << "\n=== Local Reputation Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("sys1");
    addComp<components::LocalReputation>(e);

    systems::LocalReputationSystem sys(&world);
    assertTrue(approxEqual(sys.getReputation("sys1", "player1"), 0.0f), "Default reputation is 0");
    assertTrue(sys.getStanding("sys1", "player1") == "Neutral", "Default standing is Neutral");
}

void testLocalReputationModify() {
    std::cout << "\n=== Local Reputation Modify ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("sys2");
    addComp<components::LocalReputation>(e);

    systems::LocalReputationSystem sys(&world);
    sys.modifyReputation("sys2", "player1", 30.0f);
    assertTrue(approxEqual(sys.getReputation("sys2", "player1"), 30.0f), "Reputation modified");
    sys.modifyReputation("sys2", "player1", 200.0f);
    assertTrue(approxEqual(sys.getReputation("sys2", "player1"), 100.0f), "Reputation clamped to 100");
}

void testLocalReputationDecay() {
    std::cout << "\n=== Local Reputation Decay ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("sys3");
    auto* rep = addComp<components::LocalReputation>(e);
    rep->reputation_decay_rate = 1.0f;

    systems::LocalReputationSystem sys(&world);
    sys.modifyReputation("sys3", "player1", 10.0f);
    sys.update(5.0f);
    assertTrue(sys.getReputation("sys3", "player1") < 10.0f, "Reputation decayed toward 0");
}

void testLocalReputationStanding() {
    std::cout << "\n=== Local Reputation Standing ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("sys4");
    addComp<components::LocalReputation>(e);

    systems::LocalReputationSystem sys(&world);
    sys.modifyReputation("sys4", "p1", -60.0f);
    assertTrue(sys.getStanding("sys4", "p1") == "Hostile", "Hostile at -60");
    sys.modifyReputation("sys4", "p1", 30.0f); // now -30
    assertTrue(sys.getStanding("sys4", "p1") == "Unfriendly", "Unfriendly at -30");

    auto* e2 = world.createEntity("sys5");
    addComp<components::LocalReputation>(e2);
    sys.modifyReputation("sys5", "p2", 60.0f);
    assertTrue(sys.getStanding("sys5", "p2") == "Allied", "Allied at 60");
    sys.modifyReputation("sys5", "p3", 20.0f);
    assertTrue(sys.getStanding("sys5", "p3") == "Friendly", "Friendly at 20");
}

// ========== Alliance System Tests ==========

void testAllianceCreate() {
    std::cout << "\n=== Alliance Create ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* corpEnt = world.createEntity("corp_alpha");
    auto* corp = addComp<components::Corporation>(corpEnt);
    corp->corp_id = "corp_alpha";
    corp->corp_name = "Alpha Corp";

    assertTrue(allianceSys.createAlliance("corp_alpha", "Test Alliance", "TSTA"),
               "Alliance created");

    auto* aEnt = world.getEntity("alliance_test_alliance");
    assertTrue(aEnt != nullptr, "Alliance entity exists");

    auto* a = aEnt->getComponent<components::Alliance>();
    assertTrue(a != nullptr, "Alliance component exists");
    assertTrue(a->executor_corp_id == "corp_alpha", "Executor is creator corp");
    assertTrue(a->alliance_name == "Test Alliance", "Alliance name set");
    assertTrue(a->ticker == "TSTA", "Ticker set");
    assertTrue(a->member_corp_ids.size() == 1, "One member corp");
}

void testAllianceJoinCorp() {
    std::cout << "\n=== Alliance Join Corp ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    auto* c2 = world.createEntity("corp_b");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_b";

    allianceSys.createAlliance("corp_a", "Join Alliance", "JNAL");

    assertTrue(allianceSys.joinAlliance("corp_b", "alliance_join_alliance"),
               "Corp B joins alliance");
    assertTrue(allianceSys.getMemberCorpCount("alliance_join_alliance") == 2,
               "Two member corps");
    assertTrue(allianceSys.isCorpInAlliance("corp_b", "alliance_join_alliance"),
               "Corp B is in alliance");
    assertTrue(!allianceSys.joinAlliance("corp_b", "alliance_join_alliance"),
               "Duplicate join rejected");
}

void testAllianceLeaveCorp() {
    std::cout << "\n=== Alliance Leave Corp ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    auto* c2 = world.createEntity("corp_b");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_b";

    allianceSys.createAlliance("corp_a", "Leave Alliance", "LVAL");
    allianceSys.joinAlliance("corp_b", "alliance_leave_alliance");

    assertTrue(allianceSys.leaveAlliance("corp_b", "alliance_leave_alliance"),
               "Corp B leaves alliance");
    assertTrue(allianceSys.getMemberCorpCount("alliance_leave_alliance") == 1,
               "One member corp after leave");
    assertTrue(!allianceSys.isCorpInAlliance("corp_b", "alliance_leave_alliance"),
               "Corp B no longer in alliance");
}

void testAllianceExecutorCannotLeave() {
    std::cout << "\n=== Alliance Executor Cannot Leave ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    allianceSys.createAlliance("corp_a", "Exec Alliance", "EXAL");

    assertTrue(!allianceSys.leaveAlliance("corp_a", "alliance_exec_alliance"),
               "Executor corp cannot leave");
}

void testAllianceSetExecutor() {
    std::cout << "\n=== Alliance Set Executor ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    auto* c2 = world.createEntity("corp_b");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_b";

    allianceSys.createAlliance("corp_a", "Exec Change", "EXCH");
    allianceSys.joinAlliance("corp_b", "alliance_exec_change");

    // Non-executor cannot change executor
    assertTrue(!allianceSys.setExecutor("alliance_exec_change", "corp_b", "corp_b"),
               "Non-executor cannot change executor");

    // Executor changes executor
    assertTrue(allianceSys.setExecutor("alliance_exec_change", "corp_b", "corp_a"),
               "Executor changed to corp_b");

    auto* a = world.getEntity("alliance_exec_change")->getComponent<components::Alliance>();
    assertTrue(a->executor_corp_id == "corp_b", "New executor is corp_b");
}

void testAllianceDisbandAlliance() {
    std::cout << "\n=== Alliance Disband ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_a");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_a";

    auto* c2 = world.createEntity("corp_b");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_b";

    allianceSys.createAlliance("corp_a", "Disband Alliance", "DSBA");
    allianceSys.joinAlliance("corp_b", "alliance_disband_alliance");

    // Non-executor cannot disband
    assertTrue(!allianceSys.disbandAlliance("alliance_disband_alliance", "corp_b"),
               "Non-executor cannot disband");

    assertTrue(allianceSys.disbandAlliance("alliance_disband_alliance", "corp_a"),
               "Executor disbands alliance");
    assertTrue(world.getEntity("alliance_disband_alliance") == nullptr,
               "Alliance entity destroyed");
}

void testAllianceMaxCorps() {
    std::cout << "\n=== Alliance Max Corps ===" << std::endl;
    ecs::World world;
    systems::AllianceSystem allianceSys(&world);

    auto* c1 = world.createEntity("corp_founder");
    auto* corp1 = addComp<components::Corporation>(c1);
    corp1->corp_id = "corp_founder";

    allianceSys.createAlliance("corp_founder", "Max Alliance", "MXAL");

    // Set max_corps to 2 for testing
    auto* a = world.getEntity("alliance_max_alliance")->getComponent<components::Alliance>();
    a->max_corps = 2;

    auto* c2 = world.createEntity("corp_two");
    auto* corp2 = addComp<components::Corporation>(c2);
    corp2->corp_id = "corp_two";

    assertTrue(allianceSys.joinAlliance("corp_two", "alliance_max_alliance"),
               "Second corp joins");

    auto* c3 = world.createEntity("corp_three");
    auto* corp3 = addComp<components::Corporation>(c3);
    corp3->corp_id = "corp_three";

    assertTrue(!allianceSys.joinAlliance("corp_three", "alliance_max_alliance"),
               "Third corp rejected (max 2)");
}

// ========== Sovereignty System Tests ==========

void testSovereigntyClaim() {
    std::cout << "\n=== Sovereignty Claim ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    assertTrue(sovSys.claimSovereignty("system_alpha", "alliance_1", "Alpha System"),
               "Sovereignty claimed");
    assertTrue(sovSys.getOwner("system_alpha") == "alliance_1",
               "Owner is alliance_1");
    assertTrue(sovSys.getControlLevel("system_alpha") > 0.0f,
               "Control level > 0");
}

void testSovereigntyRelinquish() {
    std::cout << "\n=== Sovereignty Relinquish ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_beta", "alliance_2", "Beta System");

    assertTrue(!sovSys.relinquishSovereignty("system_beta", "wrong_owner"),
               "Non-owner cannot relinquish");
    assertTrue(sovSys.relinquishSovereignty("system_beta", "alliance_2"),
               "Owner relinquishes");
    assertTrue(sovSys.getOwner("system_beta").empty(),
               "No owner after relinquish");
}

void testSovereigntyContest() {
    std::cout << "\n=== Sovereignty Contest ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_gamma", "alliance_3", "Gamma System");

    assertTrue(sovSys.contestSovereignty("system_gamma"),
               "System contested");

    auto* sov = world.getEntity("system_gamma")->getComponent<components::Sovereignty>();
    assertTrue(sov->is_contested, "System is contested");
}

void testSovereigntyUpdateIndices() {
    std::cout << "\n=== Sovereignty Update Indices ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_delta", "alliance_4", "Delta System");

    assertTrue(sovSys.updateIndices("system_delta", 2.0f, 3.0f),
               "Indices updated");

    auto* sov = world.getEntity("system_delta")->getComponent<components::Sovereignty>();
    assertTrue(approxEqual(sov->military_index, 2.0f), "Military index is 2.0");
    assertTrue(approxEqual(sov->industrial_index, 3.0f), "Industrial index is 3.0");

    // Clamp to max
    sovSys.updateIndices("system_delta", 10.0f, 10.0f);
    assertTrue(approxEqual(sov->military_index, 5.0f), "Military index clamped to 5.0");
    assertTrue(approxEqual(sov->industrial_index, 5.0f), "Industrial index clamped to 5.0");
}

void testSovereigntyUpgrade() {
    std::cout << "\n=== Sovereignty Upgrade ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_eps", "alliance_5", "Epsilon System");

    assertTrue(!sovSys.upgradeInfrastructure("system_eps", "wrong_owner"),
               "Non-owner cannot upgrade");
    assertTrue(sovSys.upgradeInfrastructure("system_eps", "alliance_5"),
               "Owner upgrades infrastructure");

    auto* sov = world.getEntity("system_eps")->getComponent<components::Sovereignty>();
    assertTrue(sov->upgrade_level == 1, "Upgrade level is 1");
}

void testSovereigntyMaxUpgrade() {
    std::cout << "\n=== Sovereignty Max Upgrade ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_zeta", "alliance_6", "Zeta System");

    for (int i = 0; i < 5; i++) {
        assertTrue(sovSys.upgradeInfrastructure("system_zeta", "alliance_6"),
                   "Upgrade succeeds");
    }

    assertTrue(!sovSys.upgradeInfrastructure("system_zeta", "alliance_6"),
               "Cannot upgrade beyond 5");

    auto* sov = world.getEntity("system_zeta")->getComponent<components::Sovereignty>();
    assertTrue(sov->upgrade_level == 5, "Upgrade level capped at 5");
}

void testSovereigntyUpdateDecay() {
    std::cout << "\n=== Sovereignty Update Decay ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_eta", "alliance_7", "Eta System");

    auto* sov = world.getEntity("system_eta")->getComponent<components::Sovereignty>();
    sov->control_level = 0.5f;
    sov->is_contested = true;
    sov->military_index = 3.0f;
    sov->industrial_index = 2.0f;

    // Simulate 1 hour (3600 seconds)
    sovSys.update(3600.0f);

    assertTrue(sov->control_level < 0.5f, "Contested control level decayed");
    assertTrue(sov->military_index < 3.0f, "Military index decayed");
    assertTrue(sov->industrial_index < 2.0f, "Industrial index decayed");
}

void testSovereigntyCannotClaimOwned() {
    std::cout << "\n=== Sovereignty Cannot Claim Owned ===" << std::endl;
    ecs::World world;
    systems::SovereigntySystem sovSys(&world);

    sovSys.claimSovereignty("system_theta", "alliance_8", "Theta System");

    assertTrue(!sovSys.claimSovereignty("system_theta", "alliance_9", "Theta System"),
               "Cannot claim already-owned system");
}

// ========== War Declaration System Tests ==========

void testWarDeclare() {
    std::cout << "\n=== War Declare ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    assertTrue(!warId.empty(), "War declared successfully");
    assertTrue(warSys.getWarStatus(warId) == "pending", "War status is pending");
    assertTrue(approxEqual(aggPlayer->credits, 400000000.0), "Credits deducted");
}

void testWarActivate() {
    std::cout << "\n=== War Activate ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    assertTrue(warSys.activateWar(warId), "War activated");
    assertTrue(warSys.getWarStatus(warId) == "active", "War status is active");
    assertTrue(!warSys.activateWar(warId), "Cannot activate already-active war");
}

void testWarMakeMutual() {
    std::cout << "\n=== War Make Mutual ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    warSys.activateWar(warId);

    assertTrue(!warSys.makeMutual(warId, "aggressor1"),
               "Aggressor cannot make mutual");
    assertTrue(warSys.makeMutual(warId, "defender1"),
               "Defender makes war mutual");
    assertTrue(warSys.getWarStatus(warId) == "mutual", "War status is mutual");
}

void testWarSurrender() {
    std::cout << "\n=== War Surrender ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    warSys.activateWar(warId);

    assertTrue(!warSys.surrender(warId, "aggressor1"),
               "Aggressor cannot surrender");
    assertTrue(warSys.surrender(warId, "defender1"),
               "Defender surrenders");
    assertTrue(warSys.getWarStatus(warId) == "surrendered", "War status is surrendered");
}

void testWarRetract() {
    std::cout << "\n=== War Retract ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);

    assertTrue(!warSys.retractWar(warId, "defender1"),
               "Defender cannot retract");
    assertTrue(warSys.retractWar(warId, "aggressor1"),
               "Aggressor retracts war");
    assertTrue(warSys.getWarStatus(warId) == "retracted", "War status is retracted");
}

void testWarRecordKill() {
    std::cout << "\n=== War Record Kill ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    warSys.activateWar(warId);

    assertTrue(warSys.recordKill(warId, "aggressor", 50000000.0),
               "Aggressor kill recorded");
    assertTrue(warSys.recordKill(warId, "defender", 30000000.0),
               "Defender kill recorded");

    auto* war = world.getEntity(warId)->getComponent<components::WarDeclaration>();
    assertTrue(war->aggressor_kills == 1, "Aggressor has 1 kill");
    assertTrue(war->defender_kills == 1, "Defender has 1 kill");
    assertTrue(approxEqual(war->aggressor_isc_destroyed, 50000000.0),
               "Aggressor Credits destroyed correct");
    assertTrue(approxEqual(war->defender_isc_destroyed, 30000000.0),
               "Defender Credits destroyed correct");
}

void testWarAutoFinish() {
    std::cout << "\n=== War Auto Finish ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 500000000.0;

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    warSys.activateWar(warId);

    auto* war = world.getEntity(warId)->getComponent<components::WarDeclaration>();
    war->duration_hours = 1.0f; // 1 hour for testing

    // Simulate 2 hours (7200 seconds)
    warSys.update(7200.0f);

    assertTrue(warSys.getWarStatus(warId) == "finished", "War auto-finished after duration");
}

void testWarInsufficientFunds() {
    std::cout << "\n=== War Insufficient Funds ===" << std::endl;
    ecs::World world;
    systems::WarDeclarationSystem warSys(&world);

    auto* aggEnt = world.createEntity("aggressor1");
    auto* aggPlayer = addComp<components::Player>(aggEnt);
    aggPlayer->player_id = "aggressor1";
    aggPlayer->credits = 50000000.0; // Only 50M, need 100M

    std::string warId = warSys.declareWar("aggressor1", "defender1", 100000000.0);
    assertTrue(warId.empty(), "War declaration failed - insufficient funds");
    assertTrue(approxEqual(aggPlayer->credits, 50000000.0), "Credits not deducted on failure");
}

// ==================== Convoy Ambush System tests ====================

void testConvoyRegisterRoute() {
    std::cout << "\n=== Convoy Register Route ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    std::string routeId = convoysSys.registerRoute("Jita", "Amarr", "Tritanium",
                                                    500000000.0, 0.9f);
    assertTrue(!routeId.empty(), "Route registered successfully");
    assertTrue(convoysSys.getRouteRisk(routeId) > 0.0f, "Route has pirate interest");
}

void testConvoyPlanAmbush() {
    std::cout << "\n=== Convoy Plan Ambush ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    std::string routeId = convoysSys.registerRoute("Jita", "Amarr", "Tritanium",
                                                    500000000.0, 0.5f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    assertTrue(!ambushId.empty(), "Ambush planned");
    assertTrue(convoysSys.getAmbushState(ambushId) == "planned", "Ambush state is planned");
    assertTrue(convoysSys.getPlannedAmbushes().size() == 1, "One planned ambush");
}

void testConvoyExecuteAmbushLowSec() {
    std::cout << "\n=== Convoy Execute Ambush Low-Sec ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    // Low-sec route (security < 0.7) — ambush should succeed
    std::string routeId = convoysSys.registerRoute("Amamake", "Rancer", "Plex",
                                                    1000000000.0, 0.3f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    bool success = convoysSys.executeAmbush(ambushId);
    assertTrue(success, "Ambush succeeded on low-sec route");
    assertTrue(convoysSys.getAmbushState(ambushId) == "active", "Ambush state is active");
}

void testConvoyExecuteAmbushHighSec() {
    std::cout << "\n=== Convoy Execute Ambush High-Sec ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    // High-sec route (security >= 0.7) — ambush should fail
    std::string routeId = convoysSys.registerRoute("Jita", "Amarr", "Tritanium",
                                                    500000000.0, 0.9f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    bool success = convoysSys.executeAmbush(ambushId);
    assertTrue(!success, "Ambush failed on high-sec route");
    assertTrue(convoysSys.getAmbushState(ambushId) == "failed", "Ambush state is failed");
}

void testConvoyDisperseAmbush() {
    std::cout << "\n=== Convoy Disperse Ambush ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    std::string routeId = convoysSys.registerRoute("Amamake", "Rancer", "Plex",
                                                    1000000000.0, 0.2f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    convoysSys.executeAmbush(ambushId);
    assertTrue(convoysSys.disperseAmbush(ambushId), "Ambush dispersed");
    assertTrue(convoysSys.getAmbushState(ambushId) == "dispersed", "State is dispersed");
}

void testConvoyAmbushLootValue() {
    std::cout << "\n=== Convoy Ambush Loot Value ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    // Security 0.0 (fully lawless) means 50% of cargo captured
    std::string routeId = convoysSys.registerRoute("Null-A", "Null-B", "Platinum",
                                                    1000000000.0, 0.0f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    convoysSys.executeAmbush(ambushId);
    auto* ambushEnt = world.getEntity(ambushId);
    auto* ambush = ambushEnt->getComponent<components::ConvoyAmbush>();
    assertTrue(ambush->loot_value > 0.0, "Loot value captured");
    assertTrue(ambush->ships_attacked == 1, "One ship attacked");
}

void testConvoyAmbushCannotExecuteTwice() {
    std::cout << "\n=== Convoy Ambush Cannot Execute Twice ===" << std::endl;
    ecs::World world;
    systems::ConvoyAmbushSystem convoysSys(&world);

    world.createEntity("pirate1");
    std::string routeId = convoysSys.registerRoute("Amamake", "Rancer", "Plex",
                                                    1000000000.0, 0.2f);
    std::string ambushId = convoysSys.planAmbush("pirate1", routeId);
    convoysSys.executeAmbush(ambushId);
    assertTrue(!convoysSys.executeAmbush(ambushId), "Cannot execute active ambush again");
}

// ==================== NPC Dialogue System tests ====================

void testNPCDialogueUnknownPlayer() {
    std::cout << "\n=== NPC Dialogue Unknown Player ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 0;

    std::string line = dialogueSys.generateDialogue("npc1", "player1");
    assertTrue(!line.empty(), "Dialogue generated for unknown player");
    assertTrue(line.find("new") != std::string::npos, "Line mentions being new");
    assertTrue(dialogueSys.getDialogueCount("npc1") == 1, "One line generated");
}

void testNPCDialogueFamousPlayer() {
    std::cout << "\n=== NPC Dialogue Famous Player ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 60; // Famous

    std::string line = dialogueSys.generateDialogue("npc1", "player1");
    assertTrue(!line.empty(), "Dialogue generated for famous player");
    assertTrue(line.find("Famous") != std::string::npos, "Line mentions Famous title");
}

void testNPCDialogueLegendaryPlayer() {
    std::cout << "\n=== NPC Dialogue Legendary Player ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 150; // Legendary

    std::string line = dialogueSys.generateDialogue("npc1", "player1");
    assertTrue(!line.empty(), "Dialogue generated for legendary player");
    assertTrue(line.find("Legendary") != std::string::npos, "Line mentions Legendary title");
}

void testNPCDialogueMythicPlayer() {
    std::cout << "\n=== NPC Dialogue Mythic Player ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 600; // Mythic

    std::string line = dialogueSys.generateDialogue("npc1", "player1");
    assertTrue(!line.empty(), "Dialogue generated for mythic player");
    assertTrue(line.find("Mythic") != std::string::npos, "Line mentions Mythic title");
}

void testNPCDialogueObserveLegend() {
    std::cout << "\n=== NPC Dialogue Observe Legend ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    auto* npcEnt = world.createEntity("npc1");
    addComp<components::NPCDialogue>(npcEnt);

    dialogueSys.observeLegend("npc1", "player1", "titan_kill", 100.0f);
    auto* npc = npcEnt->getComponent<components::NPCDialogue>();
    assertTrue(npc->getObservedCount() == 1, "NPC observed one legend event");
}

void testNPCDialogueMissingComponents() {
    std::cout << "\n=== NPC Dialogue Missing Components ===" << std::endl;
    ecs::World world;
    systems::NPCDialogueSystem dialogueSys(&world);

    // No entity created — should return empty string safely
    std::string line = dialogueSys.generateDialogue("no_npc", "no_player");
    assertTrue(line.empty(), "No crash with missing entities");
    assertTrue(dialogueSys.getDialogueCount("no_npc") == 0, "Count is 0 for missing entity");
}

// ==================== Station Monument System tests ====================

void testMonumentBelowMinScore() {
    std::cout << "\n=== Monument Below Min Score ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 10; // Below kMonumentMinScore (25)

    std::string monumentId = monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    assertTrue(monumentId.empty(), "No monument for score below threshold");
    assertTrue(monumentSys.getMonumentCount("station1") == 0, "Zero monuments in station");
}

void testMonumentCreatedForNotable() {
    std::cout << "\n=== Monument Created for Notable ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 30; // Notable, above threshold

    std::string monumentId = monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    assertTrue(!monumentId.empty(), "Monument created for notable player");
    assertTrue(monumentSys.getMonumentCount("station1") == 1, "One monument in station");
    assertTrue(monumentSys.getMonumentType("station1", "player1") == "Plaque",
               "Monument type is Plaque");
}

void testMonumentTypeScaling() {
    std::cout << "\n=== Monument Type Scaling ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 110; // Statue tier

    std::string monumentId = monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    assertTrue(!monumentId.empty(), "Monument created for statue-tier score");
    assertTrue(monumentSys.getMonumentType("station1", "player1") == "Statue",
               "Monument type is Statue for score 110");
}

void testMonumentUpgrade() {
    std::cout << "\n=== Monument Upgrade ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 30; // Plaque

    monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    assertTrue(monumentSys.getMonumentType("station1", "player1") == "Plaque",
               "Initially a Plaque");

    // Player becomes more famous
    legend->legend_score = 60; // Bust
    std::string upgradeId = monumentSys.checkAndCreateMonument("station1", "player1", 1.0f);
    assertTrue(!upgradeId.empty(), "Monument upgraded");
    assertTrue(monumentSys.getMonumentType("station1", "player1") == "Bust",
               "Upgraded to Bust");
    assertTrue(monumentSys.getMonumentCount("station1") == 1, "Still only one monument");
}

void testMonumentNoUpgradeIfSameType() {
    std::cout << "\n=== Monument No Upgrade If Same Type ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");
    auto* playerEnt = world.createEntity("player1");
    auto* legend = addComp<components::PlayerLegend>(playerEnt);
    legend->legend_score = 30; // Plaque

    monumentSys.checkAndCreateMonument("station1", "player1", 0.0f);
    // Same score — should not change
    std::string result = monumentSys.checkAndCreateMonument("station1", "player1", 1.0f);
    assertTrue(result.empty(), "No monument id returned when no change");
    assertTrue(monumentSys.getMonumentCount("station1") == 1, "Count unchanged");
}

void testMonumentMultiplePlayers() {
    std::cout << "\n=== Monument Multiple Players ===" << std::endl;
    ecs::World world;
    systems::StationMonumentSystem monumentSys(&world);

    world.createEntity("station1");

    for (int i = 1; i <= 3; ++i) {
        std::string pid = "player" + std::to_string(i);
        auto* ent = world.createEntity(pid);
        auto* legend = addComp<components::PlayerLegend>(ent);
        legend->legend_score = 50 * i;
        monumentSys.checkAndCreateMonument("station1", pid, 0.0f);
    }
    assertTrue(monumentSys.getMonumentCount("station1") == 3, "Three monuments for three players");
    assertTrue(monumentSys.getMonuments("station1").size() == 3, "getMonuments returns 3");
}

// ==================== Information Propagation System tests ====================

void testInfoPropReportAction() {
    std::cout << "\n=== Info Prop Report Action ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    sys1->addComponent(std::make_unique<components::InformationPropagation>());

    infoSys.reportPlayerAction("system1", "player1", "combat");
    assertTrue(infoSys.getRumorCount("system1") == 1, "One rumor reported");
    auto rumors = infoSys.getRumors("system1");
    assertTrue(rumors[0].player_id == "player1", "Rumor tracks correct player");
    assertTrue(rumors[0].action_type == "combat", "Rumor tracks correct action");
    assertTrue(rumors[0].personally_witnessed == true, "Rumor is witnessed");
}

void testInfoPropDecay() {
    std::cout << "\n=== Info Prop Decay ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    auto infoProp = std::make_unique<components::InformationPropagation>();
    infoProp->decay_rate = 0.1f;
    infoProp->max_rumor_age = 300.0f;
    sys1->addComponent(std::move(infoProp));

    // Add a non-witnessed rumor
    auto* info = sys1->getComponent<components::InformationPropagation>();
    info->addRumor("rumor1", "player1", "piracy", "system1", false);
    float initial = info->rumors[0].belief_strength;

    infoSys.update(1.0f);  // 1 second
    assertTrue(info->rumors[0].belief_strength < initial, "Belief decayed for non-witnessed");
}

void testInfoPropPropagation() {
    std::cout << "\n=== Info Prop Propagation ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    auto info1 = std::make_unique<components::InformationPropagation>();
    info1->neighbor_system_ids.push_back("system2");
    info1->propagation_interval = 1.0f;  // propagate every 1s for testing
    sys1->addComponent(std::move(info1));

    auto* sys2 = world.createEntity("system2");
    sys2->addComponent(std::make_unique<components::InformationPropagation>());

    infoSys.reportPlayerAction("system1", "player1", "combat");
    assertTrue(infoSys.getRumorCount("system1") == 1, "System1 has rumor");
    assertTrue(infoSys.getRumorCount("system2") == 0, "System2 has no rumor yet");

    infoSys.update(1.0f);  // trigger propagation
    assertTrue(infoSys.getRumorCount("system2") == 1, "Rumor propagated to system2");

    auto rumors2 = infoSys.getRumors("system2");
    assertTrue(rumors2[0].hops == 1, "Propagated rumor has hop count 1");
    assertTrue(rumors2[0].belief_strength < 1.0f, "Propagated rumor has reduced belief");
    assertTrue(rumors2[0].personally_witnessed == false, "Propagated rumor not witnessed");
}

void testInfoPropPlayerNotoriety() {
    std::cout << "\n=== Info Prop Player Notoriety ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    sys1->addComponent(std::make_unique<components::InformationPropagation>());

    infoSys.reportPlayerAction("system1", "player1", "combat");
    infoSys.reportPlayerAction("system1", "player1", "piracy");
    float notoriety = infoSys.getPlayerNotoriety("system1", "player1");
    assertTrue(notoriety > 0.0f, "Player has notoriety");
    assertTrue(infoSys.getRumorCount("system1") == 2, "Two rumors about player");
}

void testInfoPropMaxHops() {
    std::cout << "\n=== Info Prop Max Hops ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    auto info1 = std::make_unique<components::InformationPropagation>();
    info1->neighbor_system_ids.push_back("system2");
    info1->propagation_interval = 1.0f;
    info1->max_hops = 1;
    sys1->addComponent(std::move(info1));

    auto* sys2 = world.createEntity("system2");
    auto info2 = std::make_unique<components::InformationPropagation>();
    info2->neighbor_system_ids.push_back("system3");
    info2->propagation_interval = 1.0f;
    info2->max_hops = 1;
    sys2->addComponent(std::move(info2));

    auto* sys3 = world.createEntity("system3");
    sys3->addComponent(std::make_unique<components::InformationPropagation>());

    infoSys.reportPlayerAction("system1", "player1", "combat");
    infoSys.update(1.0f);  // propagate to system2
    assertTrue(infoSys.getRumorCount("system2") == 1, "Propagated to system2");

    infoSys.update(1.0f);  // try to propagate from system2 to system3
    assertTrue(infoSys.getRumorCount("system3") == 0, "Stopped at max hops");
}

void testInfoPropExpiry() {
    std::cout << "\n=== Info Prop Expiry ===" << std::endl;
    ecs::World world;
    systems::InformationPropagationSystem infoSys(&world);

    auto* sys1 = world.createEntity("system1");
    auto infoProp = std::make_unique<components::InformationPropagation>();
    infoProp->max_rumor_age = 5.0f;
    sys1->addComponent(std::move(infoProp));

    auto* info = sys1->getComponent<components::InformationPropagation>();
    info->addRumor("rumor1", "player1", "combat", "system1", true);
    assertTrue(info->getRumorCount() == 1, "Rumor exists");

    infoSys.update(6.0f);  // age past expiry
    assertTrue(info->getRumorCount() == 0, "Rumor expired and removed");
}

// ==================== Crew Activity System tests ====================

void testCrewActivityAssignRoom() {
    std::cout << "\n=== Crew Activity Assign Room ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    crew1->addComponent(std::make_unique<components::CrewActivity>());

    crewActSys.assignRoom("crew1", "engine_room");
    assertTrue(crewActSys.getAssignedRoom("crew1") == "engine_room", "Room assigned");
    assertTrue(crewActSys.getActivity("crew1") == "Idle", "Initially idle");
}

void testCrewActivityDamageRepair() {
    std::cout << "\n=== Crew Activity Damage Repair ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    crew1->addComponent(std::make_unique<components::CrewActivity>());

    crewActSys.setShipDamaged("crew1", true);
    crewActSys.update(0.1f);
    assertTrue(crewActSys.getActivity("crew1") == "Repairing", "Crew repairs when ship damaged");
}

void testCrewActivityHunger() {
    std::cout << "\n=== Crew Activity Hunger ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    auto ca = std::make_unique<components::CrewActivity>();
    ca->hunger = 0.79f;
    ca->current_activity = components::CrewActivity::Activity::Working;
    crew1->addComponent(std::move(ca));

    // hunger at 0.79, accumulation of 0.003 * delta pushes over 0.8
    crewActSys.update(10.0f);  // hunger += 0.003 * 10 = 0.03 → 0.82
    assertTrue(crewActSys.getActivity("crew1") == "Eating", "Crew eats when hungry");
}

void testCrewActivityFatigue() {
    std::cout << "\n=== Crew Activity Fatigue ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    auto ca = std::make_unique<components::CrewActivity>();
    ca->fatigue = 0.79f;
    ca->current_activity = components::CrewActivity::Activity::Working;
    crew1->addComponent(std::move(ca));

    // fatigue at 0.79, accumulation of 0.002 * delta pushes over 0.8
    crewActSys.update(10.0f);  // fatigue += 0.002 * 10 = 0.02 → 0.81
    assertTrue(crewActSys.getActivity("crew1") == "Resting", "Crew rests when fatigued");
}

void testCrewActivityGetCrewInActivity() {
    std::cout << "\n=== Crew Activity Get Crew In Activity ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    auto ca1 = std::make_unique<components::CrewActivity>();
    ca1->current_activity = components::CrewActivity::Activity::Working;
    crew1->addComponent(std::move(ca1));

    auto* crew2 = world.createEntity("crew2");
    auto ca2 = std::make_unique<components::CrewActivity>();
    ca2->current_activity = components::CrewActivity::Activity::Working;
    crew2->addComponent(std::move(ca2));

    auto* crew3 = world.createEntity("crew3");
    auto ca3 = std::make_unique<components::CrewActivity>();
    ca3->current_activity = components::CrewActivity::Activity::Resting;
    crew3->addComponent(std::move(ca3));

    auto working = crewActSys.getCrewInActivity(components::CrewActivity::Activity::Working);
    assertTrue(working.size() == 2, "Two crew working");
    auto resting = crewActSys.getCrewInActivity(components::CrewActivity::Activity::Resting);
    assertTrue(resting.size() == 1, "One crew resting");
}

void testCrewActivityTransition() {
    std::cout << "\n=== Crew Activity Transition ===" << std::endl;
    ecs::World world;
    systems::CrewActivitySystem crewActSys(&world);

    auto* crew1 = world.createEntity("crew1");
    auto ca = std::make_unique<components::CrewActivity>();
    ca->current_activity = components::CrewActivity::Activity::Idle;
    ca->assigned_room_id = "bridge";
    ca->activity_duration = 5.0f;
    crew1->addComponent(std::move(ca));

    // Idle → Walking after activity_duration
    crewActSys.update(6.0f);
    assertTrue(crewActSys.getActivity("crew1") == "Walking", "Crew walks to room after idle");

    // Walking → Manning after another activity_duration
    crewActSys.update(6.0f);
    assertTrue(crewActSys.getActivity("crew1") == "Manning", "Crew mans station after walking");
}

// ==================== Propaganda System Tests ====================

void testPropagandaDefaults() {
    std::cout << "\n=== Propaganda Defaults ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    auto myths = propSys.getMythsAbout("player1");
    assertTrue(myths.empty(), "No myths about unknown entity");
    
    int count = propSys.getActiveMythCount("player1");
    assertTrue(count == 0, "Active myth count is 0");
}

void testPropagandaGenerateMyth() {
    std::cout << "\n=== Propaganda Generate Myth ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("hero_pilot", "Solari", "heroic", "Battle of Jita");
    assertTrue(!mythId.empty(), "Myth ID returned");
    
    float cred = propSys.getMythCredibility(mythId);
    assertTrue(approxEqual(cred, 1.0f), "New myth has full credibility");
    
    auto myths = propSys.getMythsAbout("hero_pilot");
    assertTrue(myths.size() == 1, "One myth about hero_pilot");
    assertTrue(myths[0].type == components::PropagandaNetwork::MythType::Heroic, "Myth type is heroic");
}

void testPropagandaDebunk() {
    std::cout << "\n=== Propaganda Debunk ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("villain_npc", "Veyren", "villainous");
    float cred = propSys.debunkMyth(mythId, 0.7f);
    assertTrue(approxEqual(cred, 0.3f), "Credibility reduced by evidence strength");
    
    cred = propSys.debunkMyth(mythId, 0.5f);
    assertTrue(approxEqual(cred, 0.0f), "Credibility cannot go below 0");
    
    auto myths = propSys.getMythsAbout("villain_npc", true);
    assertTrue(myths[0].debunked, "Myth marked as debunked");
}

void testPropagandaSpread() {
    std::cout << "\n=== Propaganda Spread ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("mystery_trader", "Keldari", "mysterious");
    
    auto myths = propSys.getMythsAbout("mystery_trader");
    int initialSpread = myths[0].spread_count;
    
    propSys.spreadMyth(mythId, "system_alpha", 0.1f);
    
    myths = propSys.getMythsAbout("mystery_trader");
    assertTrue(myths[0].spread_count == initialSpread + 1, "Spread count increased");
}

void testPropagandaNPCBelief() {
    std::cout << "\n=== Propaganda NPC Belief ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("legend_pilot", "Aurelian", "exaggerated");
    assertTrue(propSys.npcBelievesMyth("npc_1", mythId), "NPCs believe high credibility myths");
    
    propSys.debunkMyth(mythId, 0.8f);
    assertTrue(!propSys.npcBelievesMyth("npc_1", mythId), "NPCs don't believe low credibility myths");
}

void testPropagandaMythTypeName() {
    std::cout << "\n=== Propaganda Myth Type Name ===" << std::endl;
    assertTrue(systems::PropagandaSystem::getMythTypeName(0) == "Heroic", "Type 0 is Heroic");
    assertTrue(systems::PropagandaSystem::getMythTypeName(1) == "Villainous", "Type 1 is Villainous");
    assertTrue(systems::PropagandaSystem::getMythTypeName(2) == "Mysterious", "Type 2 is Mysterious");
    assertTrue(systems::PropagandaSystem::getMythTypeName(3) == "Exaggerated", "Type 3 is Exaggerated");
    assertTrue(systems::PropagandaSystem::getMythTypeName(4) == "Fabricated", "Type 4 is Fabricated");
}

void testPropagandaDecay() {
    std::cout << "\n=== Propaganda Decay ===" << std::endl;
    ecs::World world;
    systems::PropagandaSystem propSys(&world);
    
    std::string mythId = propSys.generateMyth("fading_legend", "Solari", "heroic");
    float initialCred = propSys.getMythCredibility(mythId);
    
    // Update should decay credibility slightly
    propSys.update(100.0f);
    float newCred = propSys.getMythCredibility(mythId);
    assertTrue(newCred < initialCred, "Credibility decays over time");
}

// ===== RumorPropagation System Tests =====

void testRumorPropCreate() {
    std::cout << "\n=== RumorProp: Create ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    assertTrue(sys.initializeNetwork("network1"), "Init network succeeds");
    assertTrue(sys.getRumorCount("network1") == 0, "No rumors initially");
    assertTrue(sys.getConfirmedCount("network1") == 0, "No confirmed initially");
    assertTrue(sys.getExpiredCount("network1") == 0, "No expired initially");
}

void testRumorPropCreateRumor() {
    std::cout << "\n=== RumorProp: CreateRumor ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    assertTrue(sys.createRumor("network1", "rumor_1", "TitanAssembly", 0.8f), "Create rumor succeeds");
    assertTrue(sys.getRumorCount("network1") == 1, "Rumor count is 1");
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 0.8f), "Accuracy is 0.8");
    assertTrue(sys.isRumorActive("network1", "rumor_1"), "Rumor is active");
    assertTrue(!sys.createRumor("network1", "rumor_1", "TitanAssembly", 0.5f), "Duplicate rumor fails");
}

void testRumorPropSpread() {
    std::cout << "\n=== RumorProp: Spread ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "PirateActivity", 0.9f);
    assertTrue(sys.spreadRumor("network1", "rumor_1", "system_alpha"), "Spread succeeds");
    assertTrue(sys.getSpreadCount("network1", "rumor_1") == 1, "Spread count is 1");
    // Accuracy reduced by spread (0.9 * 0.9 = 0.81)
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 0.81f), "Accuracy decayed after spread");
    assertTrue(!sys.spreadRumor("network1", "rumor_1", "system_alpha"), "Duplicate spread fails");
}

void testRumorPropConfirm() {
    std::cout << "\n=== RumorProp: Confirm ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "TradeShift", 0.6f);
    assertTrue(sys.confirmRumor("network1", "rumor_1"), "Confirm succeeds");
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 1.0f), "Confirmed accuracy is 1.0");
    assertTrue(sys.getConfirmedCount("network1") == 1, "Confirmed count is 1");
    assertTrue(!sys.confirmRumor("network1", "rumor_1"), "Re-confirm fails");
}

void testRumorPropDecay() {
    std::cout << "\n=== RumorProp: Decay ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "FactionConflict", 0.5f);
    sys.update(10.0f); // decay: 0.5 - 0.02*10 = 0.3
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 0.3f), "Accuracy decayed to 0.3");
    assertTrue(sys.isRumorActive("network1", "rumor_1"), "Rumor still active");
}

void testRumorPropExpiry() {
    std::cout << "\n=== RumorProp: Expiry ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "TitanAssembly", 0.1f);
    sys.update(5.0f); // decay: 0.1 - 0.02*5 = 0.0, below 0.05 threshold
    assertTrue(!sys.isRumorActive("network1", "rumor_1"), "Rumor expired");
    assertTrue(sys.getExpiredCount("network1") == 1, "Expired count is 1");
}

void testRumorPropConfirmedNoDecay() {
    std::cout << "\n=== RumorProp: ConfirmedNoDecay ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "PirateActivity", 0.8f);
    sys.confirmRumor("network1", "rumor_1");
    sys.update(100.0f); // confirmed rumors don't decay
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 1.0f), "Confirmed rumor stays at 1.0");
}

void testRumorPropMultipleRumors() {
    std::cout << "\n=== RumorProp: Multiple ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "TitanAssembly", 0.9f);
    sys.createRumor("network1", "rumor_2", "PirateActivity", 0.7f);
    sys.createRumor("network1", "rumor_3", "TradeShift", 0.5f);
    assertTrue(sys.getRumorCount("network1") == 3, "3 rumors created");
    sys.confirmRumor("network1", "rumor_2");
    assertTrue(sys.getConfirmedCount("network1") == 1, "1 confirmed");
}

void testRumorPropMultiSpread() {
    std::cout << "\n=== RumorProp: MultiSpread ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    world.createEntity("network1");
    sys.initializeNetwork("network1");
    sys.createRumor("network1", "rumor_1", "TitanAssembly", 1.0f);
    sys.spreadRumor("network1", "rumor_1", "system_a");
    sys.spreadRumor("network1", "rumor_1", "system_b");
    sys.spreadRumor("network1", "rumor_1", "system_c");
    assertTrue(sys.getSpreadCount("network1", "rumor_1") == 3, "Spread to 3 systems");
    // Accuracy: 1.0 * 0.9 * 0.9 * 0.9 = 0.729
    assertTrue(approxEqual(sys.getRumorAccuracy("network1", "rumor_1"), 0.729f), "Accuracy decayed through 3 spreads");
}

void testRumorPropMissing() {
    std::cout << "\n=== RumorProp: Missing ===" << std::endl;
    ecs::World world;
    systems::RumorPropagationSystem sys(&world);
    assertTrue(!sys.initializeNetwork("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.createRumor("nonexistent", "r1", "Test", 0.5f), "Create fails on missing");
    assertTrue(!sys.spreadRumor("nonexistent", "r1", "sys1"), "Spread fails on missing");
    assertTrue(!sys.confirmRumor("nonexistent", "r1"), "Confirm fails on missing");
    assertTrue(approxEqual(sys.getRumorAccuracy("nonexistent", "r1"), 0.0f), "0 accuracy on missing");
    assertTrue(sys.getRumorCount("nonexistent") == 0, "0 rumors on missing");
    assertTrue(sys.getConfirmedCount("nonexistent") == 0, "0 confirmed on missing");
    assertTrue(sys.getExpiredCount("nonexistent") == 0, "0 expired on missing");
    assertTrue(sys.getSpreadCount("nonexistent", "r1") == 0, "0 spread on missing");
    assertTrue(!sys.isRumorActive("nonexistent", "r1"), "Not active on missing");
}


void run_social_tests() {
    testCorpCreate();
    testCorpJoin();
    testCorpLeave();
    testCorpCeoCannotLeave();
    testCorpTaxRate();
    testCorpApplyTax();
    testSerializeDeserializeCorporation();
    testReputationInstallRelationships();
    testReputationPirateHostile();
    testReputationModifyStanding();
    testReputationDerivedEffects();
    testReputationAgentAccess();
    testReputationStandingClamped();
    testRumorPropagationNewRumor();
    testRumorPropagationReinforces();
    testRumorPropagationNoRumors();
    testRumorQuestlineNoGraduation();
    testRumorQuestlineGraduates();
    testRumorQuestlineLowBeliefNoGraduation();
    testLocalReputationDefaults();
    testLocalReputationModify();
    testLocalReputationDecay();
    testLocalReputationStanding();
    testAllianceCreate();
    testAllianceJoinCorp();
    testAllianceLeaveCorp();
    testAllianceExecutorCannotLeave();
    testAllianceSetExecutor();
    testAllianceDisbandAlliance();
    testAllianceMaxCorps();
    testSovereigntyClaim();
    testSovereigntyRelinquish();
    testSovereigntyContest();
    testSovereigntyUpdateIndices();
    testSovereigntyUpgrade();
    testSovereigntyMaxUpgrade();
    testSovereigntyUpdateDecay();
    testSovereigntyCannotClaimOwned();
    testWarDeclare();
    testWarActivate();
    testWarMakeMutual();
    testWarSurrender();
    testWarRetract();
    testWarRecordKill();
    testWarAutoFinish();
    testWarInsufficientFunds();
    testConvoyRegisterRoute();
    testConvoyPlanAmbush();
    testConvoyExecuteAmbushLowSec();
    testConvoyExecuteAmbushHighSec();
    testConvoyDisperseAmbush();
    testConvoyAmbushLootValue();
    testConvoyAmbushCannotExecuteTwice();
    testNPCDialogueUnknownPlayer();
    testNPCDialogueFamousPlayer();
    testNPCDialogueLegendaryPlayer();
    testNPCDialogueMythicPlayer();
    testNPCDialogueObserveLegend();
    testNPCDialogueMissingComponents();
    testMonumentBelowMinScore();
    testMonumentCreatedForNotable();
    testMonumentTypeScaling();
    testMonumentUpgrade();
    testMonumentNoUpgradeIfSameType();
    testMonumentMultiplePlayers();
    testInfoPropReportAction();
    testInfoPropDecay();
    testInfoPropPropagation();
    testInfoPropPlayerNotoriety();
    testInfoPropMaxHops();
    testInfoPropExpiry();
    testCrewActivityAssignRoom();
    testCrewActivityDamageRepair();
    testCrewActivityHunger();
    testCrewActivityFatigue();
    testCrewActivityGetCrewInActivity();
    testCrewActivityTransition();
    testPropagandaDefaults();
    testPropagandaGenerateMyth();
    testPropagandaDebunk();
    testPropagandaSpread();
    testPropagandaNPCBelief();
    testPropagandaMythTypeName();
    testPropagandaDecay();
    testRumorPropCreate();
    testRumorPropCreateRumor();
    testRumorPropSpread();
    testRumorPropConfirm();
    testRumorPropDecay();
    testRumorPropExpiry();
    testRumorPropConfirmedNoDecay();
    testRumorPropMultipleRumors();
    testRumorPropMultiSpread();
    testRumorPropMissing();
}
