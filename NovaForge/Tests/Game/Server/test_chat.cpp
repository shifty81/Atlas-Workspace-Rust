/**
 * Chat tests
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

// ==================== Chat System Tests ====================

void testChatJoinChannel() {
    std::cout << "\n=== Chat Join Channel ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);
    channel->channel_name = "local";

    assertTrue(chatSys.joinChannel("chat_channel_1", "player_1", "Alice"),
               "Player 1 joins channel");
    assertTrue(chatSys.joinChannel("chat_channel_1", "player_2", "Bob"),
               "Player 2 joins channel");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 2,
               "Member count is 2");
    // 2 join system messages
    assertTrue(chatSys.getMessageCount("chat_channel_1") >= 2,
               "System join messages sent");
}

void testChatLeaveChannel() {
    std::cout << "\n=== Chat Leave Channel ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 1,
               "Member count is 1 after join");

    assertTrue(chatSys.leaveChannel("chat_channel_1", "player_1"),
               "Player leaves channel");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 0,
               "Member count is 0 after leave");
    // 1 join + 1 leave system message
    bool hasLeaveMsg = false;
    auto* ch = entity->getComponent<components::ChatChannel>();
    for (const auto& m : ch->messages) {
        if (m.content.find("has left the channel") != std::string::npos)
            hasLeaveMsg = true;
    }
    assertTrue(hasLeaveMsg, "Leave system message exists");
}

void testChatSendMessage() {
    std::cout << "\n=== Chat Send Message ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");
    int baseCount = chatSys.getMessageCount("chat_channel_1");

    assertTrue(chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "Hello!"),
               "First message sent");
    assertTrue(chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "World!"),
               "Second message sent");
    assertTrue(chatSys.getMessageCount("chat_channel_1") == baseCount + 2,
               "Message count increased by 2");
}

void testChatMutePlayer() {
    std::cout << "\n=== Chat Mute Player ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_mod", "Moderator");
    chatSys.joinChannel("chat_channel_1", "player_2", "Bob");

    // Set moderator role
    for (auto& m : channel->members) {
        if (m.player_id == "player_mod") m.role = "moderator";
    }

    assertTrue(chatSys.mutePlayer("chat_channel_1", "player_mod", "player_2"),
               "Moderator mutes player");
    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_2", "Bob", "test"),
               "Muted player cannot send message");
}

void testChatUnmutePlayer() {
    std::cout << "\n=== Chat Unmute Player ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_mod", "Moderator");
    chatSys.joinChannel("chat_channel_1", "player_2", "Bob");

    for (auto& m : channel->members) {
        if (m.player_id == "player_mod") m.role = "moderator";
    }

    chatSys.mutePlayer("chat_channel_1", "player_mod", "player_2");
    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_2", "Bob", "blocked"),
               "Muted player cannot send");

    assertTrue(chatSys.unmutePlayer("chat_channel_1", "player_mod", "player_2"),
               "Moderator unmutes player");
    assertTrue(chatSys.sendMessage("chat_channel_1", "player_2", "Bob", "free!"),
               "Unmuted player can send again");
}

void testChatSetMotd() {
    std::cout << "\n=== Chat Set MOTD ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_owner", "Owner");
    chatSys.joinChannel("chat_channel_1", "player_2", "Bob");

    // Set owner role
    for (auto& m : channel->members) {
        if (m.player_id == "player_owner") m.role = "owner";
    }

    assertTrue(chatSys.setMotd("chat_channel_1", "player_owner", "Welcome!"),
               "Owner sets MOTD");
    assertTrue(channel->motd == "Welcome!", "MOTD was set correctly");

    assertTrue(!chatSys.setMotd("chat_channel_1", "player_2", "Hacked!"),
               "Regular member cannot set MOTD");
    assertTrue(channel->motd == "Welcome!", "MOTD unchanged after failed attempt");
}

void testChatMaxMembers() {
    std::cout << "\n=== Chat Max Members ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);
    channel->max_members = 2;

    assertTrue(chatSys.joinChannel("chat_channel_1", "player_1", "Alice"),
               "Player 1 joins (1/2)");
    assertTrue(chatSys.joinChannel("chat_channel_1", "player_2", "Bob"),
               "Player 2 joins (2/2)");
    assertTrue(!chatSys.joinChannel("chat_channel_1", "player_3", "Charlie"),
               "Player 3 cannot join (channel full)");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 2,
               "Member count stays at 2");
}

void testChatMessageHistory() {
    std::cout << "\n=== Chat Message History ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);
    channel->max_history = 5;

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");
    // join message = 1, then send 8 more = 9 total
    for (int i = 0; i < 8; ++i) {
        chatSys.sendMessage("chat_channel_1", "player_1", "Alice",
                            "Message " + std::to_string(i));
    }
    assertTrue(static_cast<int>(channel->messages.size()) > 5,
               "Messages exceed max_history before trim");

    chatSys.update(0.0f);
    assertTrue(static_cast<int>(channel->messages.size()) <= 5,
               "Messages trimmed to max_history after update");
}

void testChatMutedPlayerCannotSend() {
    std::cout << "\n=== Chat Muted Player Cannot Send ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");

    // Directly mute via component
    for (auto& m : channel->members) {
        if (m.player_id == "player_1") m.is_muted = true;
    }

    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "test"),
               "Directly muted player cannot send");
}

void testChatNonMemberCannotSend() {
    std::cout << "\n=== Chat Non-Member Cannot Send ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    addComp<components::ChatChannel>(entity);

    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "test"),
               "Non-member cannot send message");
}


void run_chat_tests() {
    testChatJoinChannel();
    testChatLeaveChannel();
    testChatSendMessage();
    testChatMutePlayer();
    testChatUnmutePlayer();
    testChatSetMotd();
    testChatMaxMembers();
    testChatMessageHistory();
    testChatMutedPlayerCannotSend();
    testChatNonMemberCannotSend();
}
