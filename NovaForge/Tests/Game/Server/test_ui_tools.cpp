/**
 * Ui Tools tests
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

// ==================== AtlasUIPanel Tests ====================

void testUIPanelInit() {
    std::cout << "\n=== AtlasUIPanel: Init ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    assertTrue(sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory), "Panel initialized");
    assertTrue(sys.getPanelType("panel_1") == "Inventory", "Panel type is Inventory");
    assertTrue(!sys.isOpen("panel_1"), "Panel closed by default");
    assertTrue(!sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory), "Duplicate init fails");
}

void testUIPanelOpenClose() {
    std::cout << "\n=== AtlasUIPanel: Open/Close ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Market);
    assertTrue(sys.openPanel("panel_1"), "Panel opened");
    assertTrue(sys.isOpen("panel_1"), "Panel is open");
    assertTrue(sys.closePanel("panel_1"), "Panel closed");
    assertTrue(!sys.isOpen("panel_1"), "Panel is closed");
}

void testUIPanelToggle() {
    std::cout << "\n=== AtlasUIPanel: Toggle ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Fitting);
    assertTrue(sys.togglePanel("panel_1"), "Toggle succeeds");
    assertTrue(sys.isOpen("panel_1"), "Panel is open after toggle");
    assertTrue(sys.togglePanel("panel_1"), "Toggle again succeeds");
    assertTrue(!sys.isOpen("panel_1"), "Panel is closed after toggle");
}

void testUIPanelAddItem() {
    std::cout << "\n=== AtlasUIPanel: Add Item ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory);
    assertTrue(sys.addItem("panel_1", "item_1", "Tritanium", 100, 5.0f), "Item added");
    assertTrue(sys.getItemCount("panel_1") == 1, "Item count is 1");
    assertTrue(!sys.addItem("panel_1", "item_1", "Tritanium", 100, 5.0f), "Duplicate item fails");
}

void testUIPanelRemoveItem() {
    std::cout << "\n=== AtlasUIPanel: Remove Item ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory);
    sys.addItem("panel_1", "item_1", "Pyerite", 50, 10.0f);
    assertTrue(sys.removeItem("panel_1", "item_1"), "Item removed");
    assertTrue(sys.getItemCount("panel_1") == 0, "Item count is 0");
    assertTrue(!sys.removeItem("panel_1", "item_1"), "Double remove fails");
}

void testUIPanelFilter() {
    std::cout << "\n=== AtlasUIPanel: Filter ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Market);
    assertTrue(sys.setFilter("panel_1", "ore"), "Filter set");
    assertTrue(sys.setFilter("panel_1", ""), "Filter cleared");
}

void testUIPanelSort() {
    std::cout << "\n=== AtlasUIPanel: Sort ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory);
    assertTrue(sys.setSort("panel_1", "name", true), "Sort set ascending");
    assertTrue(sys.setSort("panel_1", "value", false), "Sort set descending");
}

void testUIPanelSelect() {
    std::cout << "\n=== AtlasUIPanel: Select ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    world.createEntity("panel_1");
    sys.initializePanel("panel_1", "player_1", components::AtlasUIPanel::PanelType::Inventory);
    sys.addItem("panel_1", "item_1", "Ore", 10, 1.0f);
    sys.addItem("panel_1", "item_2", "Mineral", 20, 2.0f);
    assertTrue(sys.selectItem("panel_1", 0), "Selected index 0");
    assertTrue(sys.selectItem("panel_1", 1), "Selected index 1");
    assertTrue(!sys.selectItem("panel_1", 5), "Out of range fails");
}

void testUIPanelTypes() {
    std::cout << "\n=== AtlasUIPanel: All Types ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    using PT = components::AtlasUIPanel::PanelType;
    world.createEntity("p1"); sys.initializePanel("p1", "o", PT::Inventory);
    world.createEntity("p2"); sys.initializePanel("p2", "o", PT::Fitting);
    world.createEntity("p3"); sys.initializePanel("p3", "o", PT::Market);
    world.createEntity("p4"); sys.initializePanel("p4", "o", PT::Overview);
    world.createEntity("p5"); sys.initializePanel("p5", "o", PT::Chat);
    world.createEntity("p6"); sys.initializePanel("p6", "o", PT::Drone);
    assertTrue(sys.getPanelType("p1") == "Inventory", "Inventory type");
    assertTrue(sys.getPanelType("p2") == "Fitting", "Fitting type");
    assertTrue(sys.getPanelType("p3") == "Market", "Market type");
    assertTrue(sys.getPanelType("p4") == "Overview", "Overview type");
    assertTrue(sys.getPanelType("p5") == "Chat", "Chat type");
    assertTrue(sys.getPanelType("p6") == "Drone", "Drone type");
}

void testUIPanelMissing() {
    std::cout << "\n=== AtlasUIPanel: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::AtlasUIPanelSystem sys(&world);
    assertTrue(!sys.initializePanel("nonexistent", "o", components::AtlasUIPanel::PanelType::Inventory), "Init fails on missing");
    assertTrue(!sys.isOpen("nonexistent"), "Not open on missing");
    assertTrue(sys.getItemCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getPanelType("nonexistent").empty(), "Empty type on missing");
}

// ==================== KeyboardNavigation Tests ====================

void testKbNavInit() {
    std::cout << "\n=== KeyboardNavigation: Init ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    assertTrue(sys.initializeNavigation("nav_1", "player_1"), "Navigation initialized");
    assertTrue(sys.getActivePanel("nav_1").empty(), "No active panel initially");
    assertTrue(sys.getFocusIndex("nav_1") == 0, "Focus index is 0");
    assertTrue(!sys.initializeNavigation("nav_1", "player_1"), "Duplicate init fails");
}

void testKbNavFocus() {
    std::cout << "\n=== KeyboardNavigation: Focus ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    assertTrue(sys.setFocusPanel("nav_1", "inventory"), "Focus set to inventory");
    assertTrue(sys.getActivePanel("nav_1") == "inventory", "Active panel is inventory");
}

void testKbNavMoveFocus() {
    std::cout << "\n=== KeyboardNavigation: Move Focus ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    auto* entity = world.getEntity("nav_1");
    auto* nav = entity->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"inv", "fit", "market"};
    assertTrue(sys.moveFocus("nav_1", "Down"), "Move down");
    assertTrue(sys.getFocusIndex("nav_1") == 1, "Focus is 1 after down");
    assertTrue(sys.moveFocus("nav_1", "Up"), "Move up");
    assertTrue(sys.getFocusIndex("nav_1") == 0, "Focus is 0 after up");
}

void testKbNavFocusStack() {
    std::cout << "\n=== KeyboardNavigation: Focus Stack ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    sys.setFocusPanel("nav_1", "main_menu");
    assertTrue(sys.pushFocusStack("nav_1", "submenu"), "Push focus stack");
    assertTrue(sys.getActivePanel("nav_1") == "submenu", "Active is submenu");
    assertTrue(sys.popFocusStack("nav_1"), "Pop focus stack");
    assertTrue(sys.getActivePanel("nav_1") == "main_menu", "Active is main_menu again");
    assertTrue(!sys.popFocusStack("nav_1"), "Pop empty stack fails");
}

void testKbNavBindKey() {
    std::cout << "\n=== KeyboardNavigation: Bind Key ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    assertTrue(sys.bindKey("nav_1", "Tab", "next_panel"), "Key bound");
    assertTrue(sys.bindKey("nav_1", "Escape", "close"), "Escape bound");
}

void testKbNavModal() {
    std::cout << "\n=== KeyboardNavigation: Modal ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    assertTrue(!sys.isModal("nav_1"), "Not modal initially");
    assertTrue(sys.setModal("nav_1", true, "dialog"), "Set modal");
    assertTrue(sys.isModal("nav_1"), "Is modal after set");
    assertTrue(sys.setModal("nav_1", false, ""), "Unset modal");
    assertTrue(!sys.isModal("nav_1"), "Not modal after unset");
}

void testKbNavKeyInput() {
    std::cout << "\n=== KeyboardNavigation: Key Input ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    auto* entity = world.getEntity("nav_1");
    auto* nav = entity->getComponent<components::KeyboardNavigation>();
    nav->tab_order = {"a", "b", "c"};
    sys.bindKey("nav_1", "j", "move_down");
    assertTrue(sys.handleKeyInput("nav_1", "j"), "Bound key handled");
    assertTrue(sys.getFocusIndex("nav_1") == 1, "Focus moved by bound key");
}

void testKbNavCursorBlink() {
    std::cout << "\n=== KeyboardNavigation: Cursor Blink ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    auto* entity = world.getEntity("nav_1");
    auto* nav = entity->getComponent<components::KeyboardNavigation>();
    bool initial = nav->cursor_visible;
    sys.update(1.0f);
    assertTrue(nav->cursor_visible != initial, "Cursor toggled after 1s");
}

void testKbNavInputBuffer() {
    std::cout << "\n=== KeyboardNavigation: Input Buffer ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    world.createEntity("nav_1");
    sys.initializeNavigation("nav_1", "player_1");
    sys.handleKeyInput("nav_1", "a");
    sys.handleKeyInput("nav_1", "b");
    auto* entity = world.getEntity("nav_1");
    auto* nav = entity->getComponent<components::KeyboardNavigation>();
    assertTrue(nav->input_buffer == "ab", "Input buffer has 'ab'");
}

void testKbNavMissing() {
    std::cout << "\n=== KeyboardNavigation: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::KeyboardNavigationSystem sys(&world);
    assertTrue(!sys.initializeNavigation("nonexistent", "o"), "Init fails on missing");
    assertTrue(sys.getActivePanel("nonexistent").empty(), "Empty panel on missing");
    assertTrue(sys.getFocusIndex("nonexistent") == 0, "Focus 0 on missing");
    assertTrue(!sys.isModal("nonexistent"), "Not modal on missing");
}

// ==================== DataBinding Tests ====================

void testDataBindingInit() {
    std::cout << "\n=== DataBinding: Init ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    assertTrue(sys.initializeBindings("db_1", "player_1"), "Bindings initialized");
    assertTrue(sys.getBindingCount("db_1") == 0, "No bindings initially");
    assertTrue(sys.getObserverCount("db_1") == 0, "No observers initially");
    assertTrue(!sys.initializeBindings("db_1", "player_1"), "Duplicate init fails");
}

void testDataBindingAdd() {
    std::cout << "\n=== DataBinding: Add Binding ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    assertTrue(sys.addBinding("db_1", "b1", "ship.shield", "shield_bar", "percent"), "Binding added");
    assertTrue(sys.getBindingCount("db_1") == 1, "Binding count is 1");
    assertTrue(!sys.addBinding("db_1", "b1", "ship.shield", "shield_bar", "percent"), "Duplicate fails");
}

void testDataBindingRemove() {
    std::cout << "\n=== DataBinding: Remove Binding ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addBinding("db_1", "b1", "ship.shield", "shield_bar", "percent");
    assertTrue(sys.removeBinding("db_1", "b1"), "Binding removed");
    assertTrue(sys.getBindingCount("db_1") == 0, "Binding count is 0");
    assertTrue(!sys.removeBinding("db_1", "b1"), "Double remove fails");
}

void testDataBindingUpdate() {
    std::cout << "\n=== DataBinding: Update ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addBinding("db_1", "b1", "ship.hull", "hull_bar", "percent");
    assertTrue(sys.updateBinding("db_1", "b1", "85"), "Binding updated");
    assertTrue(sys.getDirtyCount("db_1") == 1, "1 dirty binding");
}

void testDataBindingObserver() {
    std::cout << "\n=== DataBinding: Observer ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    assertTrue(sys.addObserver("db_1", "obs_1", "ship.*", "cb_shield"), "Observer added");
    assertTrue(sys.getObserverCount("db_1") == 1, "Observer count is 1");
    assertTrue(!sys.addObserver("db_1", "obs_1", "ship.*", "cb_shield"), "Duplicate observer fails");
}

void testDataBindingRemoveObserver() {
    std::cout << "\n=== DataBinding: Remove Observer ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addObserver("db_1", "obs_1", "ship.*", "cb_shield");
    assertTrue(sys.removeObserver("db_1", "obs_1"), "Observer removed");
    assertTrue(sys.getObserverCount("db_1") == 0, "Observer count is 0");
    assertTrue(!sys.removeObserver("db_1", "obs_1"), "Double remove fails");
}

void testDataBindingNotify() {
    std::cout << "\n=== DataBinding: Notify ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addObserver("db_1", "obs_1", "ship.shield", "cb_1");
    assertTrue(sys.notifyObservers("db_1", "ship.shield"), "Notification queued");
    assertTrue(sys.processNotifications("db_1"), "Notifications processed");
}

void testDataBindingDirty() {
    std::cout << "\n=== DataBinding: Dirty Tracking ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    sys.addBinding("db_1", "b1", "ship.cap", "cap_bar", "percent");
    assertTrue(sys.getDirtyCount("db_1") == 1, "New binding is dirty");
    assertTrue(sys.setDirty("db_1", "b1"), "Set dirty succeeds");
}

void testDataBindingMaxBindings() {
    std::cout << "\n=== DataBinding: Max Bindings ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    world.createEntity("db_1");
    sys.initializeBindings("db_1", "player_1");
    for (int i = 0; i < 50; i++) {
        sys.addBinding("db_1", "b_" + std::to_string(i), "path", "widget", "func");
    }
    assertTrue(sys.getBindingCount("db_1") == 50, "50 bindings added");
    assertTrue(!sys.addBinding("db_1", "b_overflow", "path", "widget", "func"), "Cannot exceed max bindings");
}

void testDataBindingMissing() {
    std::cout << "\n=== DataBinding: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::DataBindingSystem sys(&world);
    assertTrue(!sys.initializeBindings("nonexistent", "o"), "Init fails on missing");
    assertTrue(sys.getBindingCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getObserverCount("nonexistent") == 0, "Observer count 0 on missing");
    assertTrue(sys.getDirtyCount("nonexistent") == 0, "Dirty 0 on missing");
}

// ==================== EntityStressTest Tests ====================

void testStressTestInit() {
    std::cout << "\n=== EntityStressTest: Init ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    assertTrue(sys.initializeStressTest("stress_1", "server_1", 500), "Stress test initialized");
    assertTrue(sys.getPhase("stress_1") == "Idle", "Phase is Idle");
    assertTrue(approxEqual(sys.getAverageTickMs("stress_1"), 0.0f), "Avg tick is 0");
    assertTrue(!sys.initializeStressTest("stress_1", "server_1", 500), "Duplicate init fails");
}

void testStressTestStart() {
    std::cout << "\n=== EntityStressTest: Start ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    assertTrue(sys.startTest("stress_1"), "Test started");
    assertTrue(sys.getPhase("stress_1") == "Creating", "Phase is Creating");
    assertTrue(!sys.startTest("stress_1"), "Double start fails");
}

void testStressTestRecordTick() {
    std::cout << "\n=== EntityStressTest: Record Tick ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    assertTrue(sys.recordTick("stress_1", 10.0f), "Tick recorded");
    assertTrue(sys.recordTick("stress_1", 20.0f), "Second tick recorded");
    assertTrue(sys.getAverageTickMs("stress_1") > 0.0f, "Avg tick > 0");
}

void testStressTestRecordQuery() {
    std::cout << "\n=== EntityStressTest: Record Query ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    assertTrue(sys.recordQuery("stress_1", 50.0f), "Query recorded");
    assertTrue(sys.getAverageQueryUs("stress_1") > 0.0f, "Avg query > 0");
}

void testStressTestEntityCount() {
    std::cout << "\n=== EntityStressTest: Entity Count ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    assertTrue(sys.setEntityCount("stress_1", 250), "Entity count set to 250");
    assertTrue(sys.getPhase("stress_1") == "Creating", "Still creating at 250");
    assertTrue(sys.setEntityCount("stress_1", 500), "Entity count set to 500");
    assertTrue(sys.getPhase("stress_1") == "Running", "Running at 500");
}

void testStressTestBudget() {
    std::cout << "\n=== EntityStressTest: Budget ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    sys.recordTick("stress_1", 10.0f);
    sys.recordTick("stress_1", 15.0f);
    sys.recordTick("stress_1", 12.0f);
    sys.completeTest("stress_1");
    assertTrue(sys.isWithinBudget("stress_1"), "Within budget at ~12ms avg");
}

void testStressTestOverBudget() {
    std::cout << "\n=== EntityStressTest: Over Budget ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    sys.recordTick("stress_1", 60.0f);
    sys.recordTick("stress_1", 70.0f);
    sys.completeTest("stress_1");
    assertTrue(!sys.isWithinBudget("stress_1"), "Over budget at 60-70ms avg");
}

void testStressTestMaxTick() {
    std::cout << "\n=== EntityStressTest: Max Tick ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    sys.recordTick("stress_1", 5.0f);
    sys.recordTick("stress_1", 45.0f);
    sys.recordTick("stress_1", 10.0f);
    assertTrue(approxEqual(sys.getMaxTickMs("stress_1"), 45.0f), "Max tick is 45ms");
}

void testStressTestComplete() {
    std::cout << "\n=== EntityStressTest: Complete ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    world.createEntity("stress_1");
    sys.initializeStressTest("stress_1", "server_1", 500);
    sys.startTest("stress_1");
    sys.recordTick("stress_1", 20.0f);
    assertTrue(sys.completeTest("stress_1"), "Test completed");
    assertTrue(sys.getPhase("stress_1") == "Complete", "Phase is Complete");
}

void testStressTestMissing() {
    std::cout << "\n=== EntityStressTest: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::EntityStressTestSystem sys(&world);
    assertTrue(!sys.initializeStressTest("nonexistent", "s", 500), "Init fails on missing");
    assertTrue(approxEqual(sys.getAverageTickMs("nonexistent"), 0.0f), "Avg 0 on missing");
    assertTrue(approxEqual(sys.getMaxTickMs("nonexistent"), 0.0f), "Max 0 on missing");
    assertTrue(sys.getPhase("nonexistent").empty(), "Empty phase on missing");
    assertTrue(!sys.isWithinBudget("nonexistent"), "Not within budget on missing");
}

// ==================== FPSSalvagePath Tests ====================

void testSalvagePathInit() {
    std::cout << "\n=== FPSSalvagePath: Init ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    assertTrue(sys.initializePath("site_1", "wreck_01", "player_1", 5), "Path initialized");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 0.0f), "0% explored initially");
    assertTrue(!sys.initializePath("site_1", "wreck_01", "player_1", 5), "Duplicate init fails");
}

void testSalvagePathEntry() {
    std::cout << "\n=== FPSSalvagePath: Entry Points ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 5);
    assertTrue(sys.addEntryPoint("site_1", "hatch_1", 10.0f, "cutter"), "Entry added");
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "sealed", "Entry is sealed");
    assertTrue(!sys.addEntryPoint("site_1", "hatch_1", 5.0f, "cutter"), "Duplicate entry rejected");
}

void testSalvagePathCutting() {
    std::cout << "\n=== FPSSalvagePath: Cutting ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addEntryPoint("site_1", "hatch_1", 5.0f, "cutter");
    assertTrue(sys.startCutting("site_1", "hatch_1"), "Cutting started");
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "cutting", "State is cutting");
    assertTrue(!sys.startCutting("site_1", "hatch_1"), "Can't restart cutting");
    sys.setActive("site_1", true);
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "open", "Entry is open after cutting");
}

void testSalvagePathExploration() {
    std::cout << "\n=== FPSSalvagePath: Exploration ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 4);
    assertTrue(sys.exploreRoom("site_1"), "Room 1 explored");
    assertTrue(sys.exploreRoom("site_1"), "Room 2 explored");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 0.5f), "50% explored");
    sys.exploreRoom("site_1");
    sys.exploreRoom("site_1");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 1.0f), "100% explored");
    assertTrue(!sys.exploreRoom("site_1"), "Can't explore beyond total");
}

void testSalvagePathLoot() {
    std::cout << "\n=== FPSSalvagePath: Loot Nodes ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    assertTrue(sys.addLootNode("site_1", "loot_1", "Tritanium Plate",
        components::FPSSalvagePath::LootRarity::Common, 100.0f), "Loot added");
    assertTrue(!sys.addLootNode("site_1", "loot_1", "Dup",
        components::FPSSalvagePath::LootRarity::Common, 50.0f), "Duplicate loot rejected");
    assertTrue(sys.getDiscoveredLootCount("site_1") == 0, "0 discovered initially");
}

void testSalvagePathDiscover() {
    std::cout << "\n=== FPSSalvagePath: Discover Loot ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addLootNode("site_1", "loot_1", "Armor Plate",
        components::FPSSalvagePath::LootRarity::Rare, 500.0f);
    assertTrue(sys.discoverLoot("site_1", "loot_1"), "Loot discovered");
    assertTrue(sys.getDiscoveredLootCount("site_1") == 1, "1 discovered");
    assertTrue(!sys.discoverLoot("site_1", "loot_1"), "Can't re-discover");
}

void testSalvagePathCollect() {
    std::cout << "\n=== FPSSalvagePath: Collect Loot ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addLootNode("site_1", "loot_1", "Power Core",
        components::FPSSalvagePath::LootRarity::Epic, 2000.0f);
    assertTrue(!sys.collectLoot("site_1", "loot_1"), "Can't collect undiscovered");
    sys.discoverLoot("site_1", "loot_1");
    assertTrue(sys.collectLoot("site_1", "loot_1"), "Collected discovered loot");
    assertTrue(sys.getCollectedLootCount("site_1") == 1, "1 collected");
    assertTrue(!sys.collectLoot("site_1", "loot_1"), "Can't re-collect");
}

void testSalvagePathMultipleLoot() {
    std::cout << "\n=== FPSSalvagePath: Multiple Loot ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 5);
    sys.addLootNode("site_1", "loot_1", "Scrap Metal",
        components::FPSSalvagePath::LootRarity::Common, 50.0f);
    sys.addLootNode("site_1", "loot_2", "Ancient Relic",
        components::FPSSalvagePath::LootRarity::Legendary, 10000.0f);
    sys.addLootNode("site_1", "loot_3", "Circuit Board",
        components::FPSSalvagePath::LootRarity::Uncommon, 200.0f);
    sys.discoverLoot("site_1", "loot_1");
    sys.discoverLoot("site_1", "loot_2");
    assertTrue(sys.getDiscoveredLootCount("site_1") == 2, "2 discovered");
    sys.collectLoot("site_1", "loot_1");
    assertTrue(sys.getCollectedLootCount("site_1") == 1, "1 collected");
}

void testSalvagePathActive() {
    std::cout << "\n=== FPSSalvagePath: Active State ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addEntryPoint("site_1", "hatch_1", 3.0f, "cutter");
    sys.startCutting("site_1", "hatch_1");
    sys.update(1.0f);
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "cutting", "Still cutting when inactive");
    sys.setActive("site_1", true);
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "open", "Opens when active");
}

void testSalvagePathMissing() {
    std::cout << "\n=== FPSSalvagePath: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    assertTrue(!sys.initializePath("nonexistent", "s", "p", 5), "Init fails on missing");
    assertTrue(!sys.addEntryPoint("nonexistent", "e", 5.0f, "c"), "Entry fails on missing");
    assertTrue(approxEqual(sys.getExplorationProgress("nonexistent"), 0.0f), "0% on missing");
    assertTrue(sys.getDiscoveredLootCount("nonexistent") == 0, "0 discovered on missing");
    assertTrue(sys.getEntryState("nonexistent", "e") == "unknown", "Unknown state on missing");
}

// ==================== TaskSchedulerSystem Tests ====================

void testTaskSchedulerCreate() {
    std::cout << "\n=== TaskScheduler: Create ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    auto* e = world.createEntity("sched1");
    assertTrue(sys.createScheduler("sched1"), "Create scheduler succeeds");
    auto* s = e->getComponent<components::TaskScheduler>();
    assertTrue(s != nullptr, "Component exists");
    assertTrue(s->max_concurrent == 4, "Default max_concurrent is 4");
    assertTrue(s->total_completed == 0, "Default total_completed is 0");
    assertTrue(s->active, "Scheduler is active by default");
}

void testTaskSchedulerAddTask() {
    std::cout << "\n=== TaskScheduler: AddTask ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int id = sys.addTask("sched1", "build", 1);
    assertTrue(id > 0, "Task ID is positive");
    assertTrue(sys.getTaskState("sched1", id) == 0, "Task state is Queued (0)");
    assertTrue(sys.getQueuedCount("sched1") == 1, "Queued count is 1");
}

void testTaskSchedulerRunning() {
    std::cout << "\n=== TaskScheduler: Running ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int id = sys.addTask("sched1", "compile", 1);
    sys.update(0.1f);
    assertTrue(sys.getTaskState("sched1", id) == 1, "Task state is Running (1)");
    assertTrue(sys.getRunningCount("sched1") == 1, "Running count is 1");
    assertTrue(sys.getTaskProgress("sched1", id) > 0.0f, "Progress advanced");
}

void testTaskSchedulerComplete() {
    std::cout << "\n=== TaskScheduler: Complete ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int id = sys.addTask("sched1", "link", 1);
    sys.update(0.5f);
    sys.update(0.6f);
    assertTrue(sys.getTaskState("sched1", id) == 2, "Task state is Complete (2)");
    assertTrue(sys.getTotalCompleted("sched1") == 1, "Total completed is 1");
    assertTrue(approxEqual(sys.getTaskProgress("sched1", id), 1.0f), "Progress is 1.0");
}

void testTaskSchedulerPriority() {
    std::cout << "\n=== TaskScheduler: Priority ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    auto* e = world.createEntity("sched1");
    sys.createScheduler("sched1");
    auto* s = e->getComponent<components::TaskScheduler>();
    s->max_concurrent = 1;
    int low_id = sys.addTask("sched1", "low_task", 0);
    int high_id = sys.addTask("sched1", "high_task", 3);
    sys.update(0.1f);
    assertTrue(sys.getTaskState("sched1", high_id) == 1, "High priority task is Running");
    assertTrue(sys.getTaskState("sched1", low_id) == 0, "Low priority task still Queued");
    assertTrue(sys.getRunningCount("sched1") == 1, "Only 1 running");
}

void testTaskSchedulerMaxConcurrent() {
    std::cout << "\n=== TaskScheduler: MaxConcurrent ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    auto* e = world.createEntity("sched1");
    sys.createScheduler("sched1");
    auto* s = e->getComponent<components::TaskScheduler>();
    s->max_concurrent = 2;
    sys.addTask("sched1", "t1", 1);
    sys.addTask("sched1", "t2", 1);
    sys.addTask("sched1", "t3", 1);
    sys.update(0.1f);
    assertTrue(sys.getRunningCount("sched1") == 2, "Only 2 tasks running");
    assertTrue(sys.getQueuedCount("sched1") == 1, "1 task still queued");
    assertTrue(sys.getTaskProgress("sched1", 3) == 0.0f, "Third task has no progress");
}

void testTaskSchedulerDependency() {
    std::cout << "\n=== TaskScheduler: Dependency ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int t1 = sys.addTask("sched1", "first", 1);
    int t2 = sys.addTask("sched1", "second", 1);
    sys.addDependency("sched1", t2, t1);
    sys.update(0.1f);
    assertTrue(sys.getTaskState("sched1", t1) == 1, "First task is Running");
    assertTrue(sys.getTaskState("sched1", t2) == 0, "Second task still Queued (dependency)");
    // Complete first task
    sys.update(1.0f);
    // Next tick: t2's dependency is met, it can start
    sys.update(0.01f);
    assertTrue(sys.getTaskState("sched1", t2) == 1 || sys.getTaskState("sched1", t2) == 2,
               "Second task started after dependency complete");
}

void testTaskSchedulerCancel() {
    std::cout << "\n=== TaskScheduler: Cancel ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    auto* e = world.createEntity("sched1");
    sys.createScheduler("sched1");
    auto* s = e->getComponent<components::TaskScheduler>();
    s->max_concurrent = 1;
    sys.addTask("sched1", "blocker", 2);
    int t2 = sys.addTask("sched1", "cancel_me", 0);
    assertTrue(sys.cancelTask("sched1", t2), "Cancel queued task succeeds");
    assertTrue(sys.getTaskState("sched1", t2) == 4, "Cancelled task state is 4");
    assertTrue(!sys.cancelTask("sched1", t2), "Cannot cancel already cancelled task");
}

void testTaskSchedulerFailedDep() {
    std::cout << "\n=== TaskScheduler: FailedDep ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    world.createEntity("sched1");
    sys.createScheduler("sched1");
    int t1 = sys.addTask("sched1", "will_cancel", 1);
    int t2 = sys.addTask("sched1", "depends_on_cancelled", 1);
    sys.addDependency("sched1", t2, t1);
    sys.cancelTask("sched1", t1);
    sys.update(0.1f);
    assertTrue(sys.getTaskState("sched1", t2) == 3, "Dependent task Failed");
    assertTrue(sys.getTaskState("sched1", t1) == 4, "Original task still Cancelled");
    assertTrue(sys.getRunningCount("sched1") == 0, "No running tasks");
}

void testTaskSchedulerMissing() {
    std::cout << "\n=== TaskScheduler: Missing ===" << std::endl;
    ecs::World world;
    systems::TaskSchedulerSystem sys(&world);
    assertTrue(!sys.createScheduler("nonexistent"), "Create fails on missing entity");
    assertTrue(sys.addTask("nonexistent", "t", 1) == -1, "addTask returns -1 on missing");
    assertTrue(sys.getTaskState("nonexistent", 1) == -1, "getTaskState returns -1 on missing");
    assertTrue(sys.getRunningCount("nonexistent") == 0, "0 running on missing");
    assertTrue(sys.getQueuedCount("nonexistent") == 0, "0 queued on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(approxEqual(sys.getThroughput("nonexistent"), 0.0f), "0 throughput on missing");
    assertTrue(approxEqual(sys.getTaskProgress("nonexistent", 1), 0.0f), "0 progress on missing");
    assertTrue(!sys.cancelTask("nonexistent", 1), "Cancel fails on missing");
}

// ==================== ModManagerSystem Tests ====================

void testModManagerCreate() {
    std::cout << "\n=== ModManager: Create ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    auto* e = world.createEntity("mgr1");
    assertTrue(sys.createManager("mgr1"), "Create manager succeeds");
    auto* m = e->getComponent<components::ModManager>();
    assertTrue(m != nullptr, "Component exists");
    assertTrue(m->max_mods == 50, "Default max_mods is 50");
    assertTrue(m->total_installed == 0, "Default total_installed is 0");
    assertTrue(m->active, "Manager is active by default");
}

void testModManagerInstall() {
    std::cout << "\n=== ModManager: Install ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    assertTrue(sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev"), "Install succeeds");
    assertTrue(sys.isInstalled("mgr1", "mod_a"), "Mod is installed");
    assertTrue(sys.getModCount("mgr1") == 1, "Mod count is 1");
}

void testModManagerUninstall() {
    std::cout << "\n=== ModManager: Uninstall ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev");
    assertTrue(sys.uninstallMod("mgr1", "mod_a"), "Uninstall succeeds");
    assertTrue(!sys.isInstalled("mgr1", "mod_a"), "Mod no longer installed");
    assertTrue(sys.getModCount("mgr1") == 0, "Mod count is 0");
}

void testModManagerEnable() {
    std::cout << "\n=== ModManager: Enable/Disable ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev");
    assertTrue(sys.enableMod("mgr1", "mod_a"), "Enable succeeds");
    assertTrue(sys.getEnabledCount("mgr1") == 1, "Enabled count is 1");
    assertTrue(sys.disableMod("mgr1", "mod_a"), "Disable succeeds");
    assertTrue(sys.getEnabledCount("mgr1") == 0, "Enabled count is 0");
}

void testModManagerDependency() {
    std::cout << "\n=== ModManager: Dependency ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "base", "Base", "1.0", "Dev");
    sys.installMod("mgr1", "ext", "Extension", "1.0", "Dev");
    assertTrue(sys.addDependency("mgr1", "ext", "base"), "Add dependency succeeds");
    sys.update(0.0f);
    auto order = sys.getLoadOrder("mgr1");
    assertTrue(order.size() == 2, "Load order has 2 entries");
    assertTrue(order[0] == "base", "Base comes first in load order");
}

void testModManagerConflict() {
    std::cout << "\n=== ModManager: Conflict ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev");
    sys.installMod("mgr1", "mod_b", "Beta", "1.0", "Dev");
    sys.addConflict("mgr1", "mod_a", "mod_b");
    sys.enableMod("mgr1", "mod_b");
    assertTrue(sys.hasConflict("mgr1", "mod_a"), "mod_a conflicts with enabled mod_b");
    sys.disableMod("mgr1", "mod_b");
    assertTrue(!sys.hasConflict("mgr1", "mod_a"), "No conflict when mod_b disabled");
}

void testModManagerMaxMods() {
    std::cout << "\n=== ModManager: MaxMods ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    auto* e = world.createEntity("mgr1");
    sys.createManager("mgr1");
    auto* m = e->getComponent<components::ModManager>();
    m->max_mods = 2;
    assertTrue(sys.installMod("mgr1", "m1", "M1", "1.0", "D"), "Install 1 succeeds");
    assertTrue(sys.installMod("mgr1", "m2", "M2", "1.0", "D"), "Install 2 succeeds");
    assertTrue(!sys.installMod("mgr1", "m3", "M3", "1.0", "D"), "Install 3 fails (max reached)");
}

void testModManagerDuplicate() {
    std::cout << "\n=== ModManager: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    assertTrue(sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev"), "First install succeeds");
    assertTrue(!sys.installMod("mgr1", "mod_a", "Alpha2", "2.0", "Dev2"), "Duplicate install fails");
    assertTrue(sys.getModCount("mgr1") == 1, "Still only 1 mod");
}

void testModManagerLoadOrder() {
    std::cout << "\n=== ModManager: LoadOrder ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "c", "C", "1.0", "D");
    sys.installMod("mgr1", "b", "B", "1.0", "D");
    sys.installMod("mgr1", "a", "A", "1.0", "D");
    sys.addDependency("mgr1", "c", "b");
    sys.addDependency("mgr1", "b", "a");
    sys.update(0.0f);
    auto order = sys.getLoadOrder("mgr1");
    assertTrue(order.size() == 3, "Load order has 3 entries");
    // a has no deps (order 0), b depends on a (order 1), c depends on b (order 2)
    assertTrue(order[0] == "a", "a is first");
    assertTrue(order[2] == "c", "c is last");
}

void testModManagerMissing() {
    std::cout << "\n=== ModManager: Missing ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    assertTrue(!sys.createManager("nonexistent"), "Create fails on missing");
    assertTrue(!sys.installMod("nonexistent", "m", "M", "1.0", "D"), "Install fails on missing");
    assertTrue(!sys.uninstallMod("nonexistent", "m"), "Uninstall fails on missing");
    assertTrue(!sys.enableMod("nonexistent", "m"), "Enable fails on missing");
    assertTrue(!sys.disableMod("nonexistent", "m"), "Disable fails on missing");
    assertTrue(sys.getModCount("nonexistent") == 0, "0 mods on missing");
    assertTrue(sys.getEnabledCount("nonexistent") == 0, "0 enabled on missing");
    assertTrue(!sys.isInstalled("nonexistent", "m"), "Not installed on missing");
    assertTrue(sys.getLoadOrder("nonexistent").empty(), "Empty load order on missing");
}

// ==================== ContentValidation System Tests ====================

void testContentValidationCreate() {
    std::cout << "\n=== ContentValidation: Create ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    auto* e = world.createEntity("val1");
    assertTrue(sys.createValidator("val1"), "Create validator succeeds");
    auto* cv = e->getComponent<components::ContentValidation>();
    assertTrue(cv != nullptr, "Component exists");
    assertTrue(cv->active, "Validator is active by default");
    assertTrue(cv->total_validations == 0, "No validations yet");
}

void testContentValidationSubmit() {
    std::cout << "\n=== ContentValidation: Submit ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    assertTrue(sys.submitContent("val1", "ship_001", 0, "Custom Frigate"), "Submit succeeds");
    assertTrue(sys.getContentState("val1", "ship_001") == 0, "State is Pending (0)");
    assertTrue(sys.getPendingCount("val1") == 1, "1 pending");
}

void testContentValidationDuplicate() {
    std::cout << "\n=== ContentValidation: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "ship_001", 0, "Custom Frigate");
    assertTrue(!sys.submitContent("val1", "ship_001", 0, "Duplicate"), "Duplicate rejected");
    assertTrue(sys.getPendingCount("val1") == 1, "Still 1 pending");
}

void testContentValidationRun() {
    std::cout << "\n=== ContentValidation: RunValidation ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "ship_001", 0, "Custom Frigate");
    assertTrue(sys.runValidation("val1", "ship_001"), "Validation starts");
    assertTrue(sys.getContentState("val1", "ship_001") == 1, "State is Validating (1)");
    assertTrue(sys.getTotalValidations("val1") == 1, "1 total validation");
}

void testContentValidationApprove() {
    std::cout << "\n=== ContentValidation: Approve ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "ship_001", 0, "Custom Frigate");
    sys.runValidation("val1", "ship_001");
    assertTrue(sys.approveContent("val1", "ship_001"), "Approve succeeds");
    assertTrue(sys.getContentState("val1", "ship_001") == 2, "State is Approved (2)");
    assertTrue(sys.getApprovedCount("val1") == 1, "1 approved");
}

void testContentValidationReject() {
    std::cout << "\n=== ContentValidation: Reject ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "mod_001", 1, "OP Module");
    sys.runValidation("val1", "mod_001");
    assertTrue(sys.rejectContent("val1", "mod_001", "Overpowered stats"), "Reject succeeds");
    assertTrue(sys.getContentState("val1", "mod_001") == 3, "State is Rejected (3)");
    assertTrue(sys.getRejectedCount("val1") == 1, "1 rejected");
    assertTrue(sys.getRejectionReason("val1", "mod_001") == "Overpowered stats", "Reason stored");
}

void testContentValidationWorkflow() {
    std::cout << "\n=== ContentValidation: Workflow ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "s1", 0, "Ship A");
    sys.submitContent("val1", "s2", 0, "Ship B");
    sys.submitContent("val1", "m1", 2, "Mission X");
    assertTrue(sys.getPendingCount("val1") == 3, "3 pending");
    sys.runValidation("val1", "s1");
    sys.runValidation("val1", "s2");
    sys.approveContent("val1", "s1");
    sys.rejectContent("val1", "s2", "Bad stats");
    assertTrue(sys.getApprovedCount("val1") == 1, "1 approved");
    assertTrue(sys.getRejectedCount("val1") == 1, "1 rejected");
    assertTrue(sys.getPendingCount("val1") == 1, "1 still pending");
    assertTrue(sys.getTotalValidations("val1") == 2, "2 total validations");
}

void testContentValidationStateTransition() {
    std::cout << "\n=== ContentValidation: StateTransition ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    world.createEntity("val1");
    sys.createValidator("val1");
    sys.submitContent("val1", "s1", 0, "Ship");
    assertTrue(!sys.approveContent("val1", "s1"), "Cannot approve from Pending");
    assertTrue(!sys.rejectContent("val1", "s1", "reason"), "Cannot reject from Pending");
    sys.runValidation("val1", "s1");
    assertTrue(!sys.runValidation("val1", "s1"), "Cannot re-validate from Validating");
}

void testContentValidationMissing() {
    std::cout << "\n=== ContentValidation: Missing ===" << std::endl;
    ecs::World world;
    systems::ContentValidationSystem sys(&world);
    assertTrue(!sys.createValidator("nonexistent"), "Create fails on missing");
    assertTrue(!sys.submitContent("nonexistent", "x", 0, "X"), "Submit fails on missing");
    assertTrue(!sys.runValidation("nonexistent", "x"), "Validate fails on missing");
    assertTrue(!sys.approveContent("nonexistent", "x"), "Approve fails on missing");
    assertTrue(!sys.rejectContent("nonexistent", "x", "r"), "Reject fails on missing");
    assertTrue(sys.getContentState("nonexistent", "x") == -1, "-1 state on missing");
    assertTrue(sys.getPendingCount("nonexistent") == 0, "0 pending on missing");
    assertTrue(sys.getApprovedCount("nonexistent") == 0, "0 approved on missing");
    assertTrue(sys.getRejectedCount("nonexistent") == 0, "0 rejected on missing");
    assertTrue(sys.getTotalValidations("nonexistent") == 0, "0 validations on missing");
}

// ==================== VisualFeedbackQueue Tests ====================

void testVisualFeedbackCreate() {
    std::cout << "\n=== VisualFeedbackQueue: Create ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    auto* e = world.createEntity("vfq1");
    assertTrue(sys.createQueue("vfq1"), "Create queue succeeds");
    auto* vfq = e->getComponent<components::VisualFeedbackQueue>();
    assertTrue(vfq != nullptr, "Component exists");
    assertTrue(vfq->active, "Active by default");
    assertTrue(vfq->max_effects == 20, "Default max effects is 20");
    assertTrue(vfq->total_effects_queued == 0, "No effects queued");
    assertTrue(vfq->effects.empty(), "Effects list empty");
}

void testVisualFeedbackQueue() {
    std::cout << "\n=== VisualFeedbackQueue: Queue ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    int id = sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Hit");
    assertTrue(id > 0, "Effect id is positive");
    assertTrue(sys.getActiveEffectCount("vfq1") == 1, "One active effect");
    assertTrue(sys.getTotalQueued("vfq1") == 1, "Total queued is 1");
}

void testVisualFeedbackMultiple() {
    std::cout << "\n=== VisualFeedbackQueue: Multiple ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Hit1");
    sys.queueEffect("vfq1", 1, 0.5f, 3.0f, 2, "Shield");
    sys.queueEffect("vfq1", 2, 0.8f, 1.5f, 0, "Heal");
    assertTrue(sys.getActiveEffectCount("vfq1") == 3, "Three active effects");
    assertTrue(sys.getTotalQueued("vfq1") == 3, "Total queued is 3");
}

void testVisualFeedbackExpiry() {
    std::cout << "\n=== VisualFeedbackQueue: Expiry ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 1.0f, 1, "Hit");
    sys.update(1.5f);
    assertTrue(sys.getActiveEffectCount("vfq1") == 0, "Effect expired");
    assertTrue(sys.getTotalExpired("vfq1") == 1, "One expired");
}

void testVisualFeedbackFading() {
    std::cout << "\n=== VisualFeedbackQueue: Fading ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    auto* e = world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 1.0f, 1, "Hit");
    sys.update(0.8f); // lifetime = 0.2, which is < 0.3
    auto* vfq = e->getComponent<components::VisualFeedbackQueue>();
    assertTrue(vfq->effects.size() == 1, "Effect still alive");
    assertTrue(vfq->effects[0].fading, "Effect is fading");
}

void testVisualFeedbackPriority() {
    std::cout << "\n=== VisualFeedbackQueue: Priority ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Normal");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 3, "Critical");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 0, "Low");
    assertTrue(sys.getHighestPriority("vfq1") == 3, "Highest priority is Critical (3)");
}

void testVisualFeedbackOverflow() {
    std::cout << "\n=== VisualFeedbackQueue: Overflow ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    auto* e = world.createEntity("vfq1");
    sys.createQueue("vfq1");
    auto* vfq = e->getComponent<components::VisualFeedbackQueue>();
    vfq->max_effects = 3;
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "A");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 0, "B");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 2, "C");
    assertTrue(sys.getActiveEffectCount("vfq1") == 3, "At max capacity");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 3, "D");
    assertTrue(sys.getActiveEffectCount("vfq1") == 3, "Still at max after overflow");
    assertTrue(sys.getHighestPriority("vfq1") == 3, "Highest is now 3");
}

void testVisualFeedbackRemove() {
    std::cout << "\n=== VisualFeedbackQueue: Remove ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    int id = sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Hit");
    assertTrue(sys.getActiveEffectCount("vfq1") == 1, "One effect before remove");
    assertTrue(sys.removeEffect("vfq1", id), "Remove succeeds");
    assertTrue(sys.getActiveEffectCount("vfq1") == 0, "Zero effects after remove");
}

void testVisualFeedbackCategory() {
    std::cout << "\n=== VisualFeedbackQueue: Category ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    world.createEntity("vfq1");
    sys.createQueue("vfq1");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Dmg1");
    sys.queueEffect("vfq1", 0, 1.0f, 2.0f, 1, "Dmg2");
    sys.queueEffect("vfq1", 1, 1.0f, 2.0f, 1, "Shield");
    assertTrue(sys.getEffectsByCategory("vfq1", 0) == 2, "Two damage effects");
    assertTrue(sys.getEffectsByCategory("vfq1", 1) == 1, "One shield effect");
    assertTrue(sys.getEffectsByCategory("vfq1", 2) == 0, "Zero heal effects");
}

void testVisualFeedbackMissing() {
    std::cout << "\n=== VisualFeedbackQueue: Missing ===" << std::endl;
    ecs::World world;
    systems::VisualFeedbackQueueSystem sys(&world);
    assertTrue(!sys.createQueue("nonexistent"), "Create fails on missing");
    assertTrue(sys.queueEffect("nonexistent", 0, 1.0f, 1.0f, 1, "X") == -1, "Queue fails on missing");
    assertTrue(!sys.removeEffect("nonexistent", 1), "Remove fails on missing");
    assertTrue(sys.getActiveEffectCount("nonexistent") == 0, "0 effects on missing");
    assertTrue(sys.getHighestPriority("nonexistent") == -1, "-1 priority on missing");
    assertTrue(sys.getTotalQueued("nonexistent") == 0, "0 queued on missing");
    assertTrue(sys.getTotalExpired("nonexistent") == 0, "0 expired on missing");
}

// ==================== ModDocGenerator Tests ====================

void testModDocCreate() {
    std::cout << "\n=== ModDocGenerator: Create ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    auto* e = world.createEntity("mdg1");
    assertTrue(sys.createGenerator("mdg1"), "Create generator succeeds");
    auto* mdg = e->getComponent<components::ModDocGenerator>();
    assertTrue(mdg != nullptr, "Component exists");
    assertTrue(mdg->active, "Active by default");
    assertTrue(mdg->title == "Nova Forge Modding Reference", "Default title");
    assertTrue(mdg->version == "1.0", "Default version");
    assertTrue(!mdg->generated, "Not generated by default");
}

void testModDocRegister() {
    std::cout << "\n=== ModDocGenerator: Register ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("mdg1");
    sys.createGenerator("mdg1");
    assertTrue(sys.registerType("mdg1", "BasicShip", "Ship", "A basic ship", 5), "Register succeeds");
    assertTrue(sys.getEntryCount("mdg1") == 1, "Entry count is 1");
}

void testModDocDuplicate() {
    std::cout << "\n=== ModDocGenerator: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("mdg1");
    sys.createGenerator("mdg1");
    assertTrue(sys.registerType("mdg1", "BasicShip", "Ship", "A basic ship", 5), "First register");
    assertTrue(!sys.registerType("mdg1", "BasicShip", "Ship", "Duplicate", 3), "Duplicate rejected");
    assertTrue(sys.getEntryCount("mdg1") == 1, "Still one entry");
}

void testModDocAddExample() {
    std::cout << "\n=== ModDocGenerator: AddExample ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    auto* e = world.createEntity("mdg1");
    sys.createGenerator("mdg1");
    sys.registerType("mdg1", "BasicShip", "Ship", "A basic ship", 5);
    assertTrue(sys.addExample("mdg1", "BasicShip"), "Add example succeeds");
    auto* mdg = e->getComponent<components::ModDocGenerator>();
    assertTrue(mdg->entries[0].has_example, "Example flag set");
    assertTrue(!sys.addExample("mdg1", "NonExistent"), "Add example to missing type fails");
}

void testModDocValidate() {
    std::cout << "\n=== ModDocGenerator: Validate ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("mdg1");
    sys.createGenerator("mdg1");
    sys.registerType("mdg1", "BasicShip", "Ship", "A basic ship", 5);
    sys.addExample("mdg1", "BasicShip");
    assertTrue(sys.validateEntry("mdg1", "BasicShip"), "Validation succeeds");
    assertTrue(sys.getValidatedCount("mdg1") == 1, "Validated count is 1");
}

void testModDocValidateFail() {
    std::cout << "\n=== ModDocGenerator: ValidateFail ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("mdg1");
    sys.createGenerator("mdg1");
    sys.registerType("mdg1", "BasicShip", "Ship", "A basic ship", 5);
    assertTrue(!sys.validateEntry("mdg1", "BasicShip"), "Validate without example fails");
    assertTrue(sys.getValidatedCount("mdg1") == 0, "Validated count still 0");
}

void testModDocGenerate() {
    std::cout << "\n=== ModDocGenerator: Generate ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("mdg1");
    sys.createGenerator("mdg1");
    sys.registerType("mdg1", "BasicShip", "Ship", "A basic ship", 5);
    sys.addExample("mdg1", "BasicShip");
    sys.validateEntry("mdg1", "BasicShip");
    assertTrue(sys.generate("mdg1"), "Generate succeeds");
    assertTrue(sys.isGenerated("mdg1"), "Is generated");
    assertTrue(sys.getGenerationCount("mdg1") == 1, "Generation count is 1");
}

void testModDocGenerateFail() {
    std::cout << "\n=== ModDocGenerator: GenerateFail ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("mdg1");
    sys.createGenerator("mdg1");
    sys.registerType("mdg1", "BasicShip", "Ship", "A basic ship", 5);
    assertTrue(!sys.generate("mdg1"), "Generate fails with unvalidated entries");
    assertTrue(!sys.isGenerated("mdg1"), "Not generated");
}

void testModDocCategory() {
    std::cout << "\n=== ModDocGenerator: Category ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    world.createEntity("mdg1");
    sys.createGenerator("mdg1");
    sys.registerType("mdg1", "BasicShip", "Ship", "A ship", 5);
    sys.registerType("mdg1", "LaserModule", "Module", "A module", 3);
    sys.registerType("mdg1", "HeavyShip", "Ship", "Heavy ship", 8);
    assertTrue(sys.getEntriesByCategory("mdg1", "Ship") == 2, "Two ship entries");
    assertTrue(sys.getEntriesByCategory("mdg1", "Module") == 1, "One module entry");
    assertTrue(sys.getEntriesByCategory("mdg1", "Mission") == 0, "Zero mission entries");
}

void testModDocMissing() {
    std::cout << "\n=== ModDocGenerator: Missing ===" << std::endl;
    ecs::World world;
    systems::ModDocGeneratorSystem sys(&world);
    assertTrue(!sys.createGenerator("nonexistent"), "Create fails on missing");
    assertTrue(!sys.registerType("nonexistent", "X", "Ship", "desc", 1), "Register fails on missing");
    assertTrue(!sys.addExample("nonexistent", "X"), "AddExample fails on missing");
    assertTrue(!sys.validateEntry("nonexistent", "X"), "Validate fails on missing");
    assertTrue(!sys.generate("nonexistent"), "Generate fails on missing");
    assertTrue(sys.getEntryCount("nonexistent") == 0, "0 entries on missing");
    assertTrue(!sys.isGenerated("nonexistent"), "Not generated on missing");
    assertTrue(sys.getValidatedCount("nonexistent") == 0, "0 validated on missing");
    assertTrue(sys.getGenerationCount("nonexistent") == 0, "0 generations on missing");
}

// ==================== CommunityContentRepositorySystem Tests ====================

void testCommunityContentCreate() {
    std::cout << "\n=== CommunityContentRepository: Create ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    assertTrue(sys.createRepo("repo1"), "Create repo succeeds");
    assertTrue(sys.getContentCount("repo1") == 0, "Initial content count is 0");
    assertTrue(sys.getTotalSubmissions("repo1") == 0, "Initial total submissions is 0");
    assertTrue(sys.getTotalDownloads("repo1") == 0, "Initial total downloads is 0");
}

void testCommunityContentSubmit() {
    std::cout << "\n=== CommunityContentRepository: Submit ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    assertTrue(sys.submitContent("repo1", "ship1", "Ship", "alice", "Cool Ship", "A neat ship"), "Submit ship succeeds");
    assertTrue(sys.getContentCount("repo1") == 1, "Content count is 1 after submit");
    assertTrue(sys.getTotalSubmissions("repo1") == 1, "Total submissions is 1");
    assertTrue(sys.getContentState("repo1", "ship1") == "Draft", "New content starts as Draft");
}

void testCommunityContentPublish() {
    std::cout << "\n=== CommunityContentRepository: Publish ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "ship1", "Ship", "alice", "Cool Ship", "A neat ship");
    assertTrue(sys.publishContent("repo1", "ship1"), "Publish Draft content succeeds");
    assertTrue(sys.getContentState("repo1", "ship1") == "Published", "State is Published");
    assertTrue(!sys.publishContent("repo1", "ship1"), "Cannot publish already Published content");
}

void testCommunityContentFeature() {
    std::cout << "\n=== CommunityContentRepository: Feature ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "mod1", "Module", "bob", "Mod A", "desc");
    assertTrue(!sys.featureContent("repo1", "mod1"), "Cannot feature Draft content");
    sys.publishContent("repo1", "mod1");
    assertTrue(sys.featureContent("repo1", "mod1"), "Feature Published content succeeds");
    assertTrue(sys.getContentState("repo1", "mod1") == "Featured", "State is Featured");
}

void testCommunityContentArchive() {
    std::cout << "\n=== CommunityContentRepository: Archive ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "skin1", "Skin", "carol", "Skin A", "desc");
    assertTrue(sys.archiveContent("repo1", "skin1"), "Archive Draft content succeeds");
    assertTrue(sys.getContentState("repo1", "skin1") == "Archived", "State is Archived");
}

void testCommunityContentRate() {
    std::cout << "\n=== CommunityContentRepository: Rate ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "m1", "Mission", "dave", "Mission A", "desc");
    assertTrue(sys.rateContent("repo1", "m1", 5), "Rate 5 stars succeeds");
    assertTrue(approxEqual(sys.getAverageRating("repo1", "m1"), 5.0f), "Average rating is 5.0");
    assertTrue(sys.rateContent("repo1", "m1", 3), "Rate 3 stars succeeds");
    assertTrue(approxEqual(sys.getAverageRating("repo1", "m1"), 4.0f), "Average rating is 4.0 after two ratings");
    assertTrue(sys.rateContent("repo1", "m1", 1), "Rate 1 star succeeds");
    assertTrue(approxEqual(sys.getAverageRating("repo1", "m1"), 3.0f), "Average rating is 3.0 after three ratings");
}

void testCommunityContentDownload() {
    std::cout << "\n=== CommunityContentRepository: Download ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "ship1", "Ship", "alice", "Cool Ship", "desc");
    assertTrue(sys.downloadContent("repo1", "ship1"), "Download succeeds");
    assertTrue(sys.getDownloadCount("repo1", "ship1") == 1, "Download count is 1");
    sys.downloadContent("repo1", "ship1");
    sys.downloadContent("repo1", "ship1");
    assertTrue(sys.getDownloadCount("repo1", "ship1") == 3, "Download count is 3 after 3 downloads");
    assertTrue(sys.getTotalDownloads("repo1") == 3, "Total downloads is 3");
}

void testCommunityContentByType() {
    std::cout << "\n=== CommunityContentRepository: ByType ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "s1", "Ship", "a", "S1", "d");
    sys.submitContent("repo1", "s2", "Ship", "b", "S2", "d");
    sys.submitContent("repo1", "m1", "Module", "c", "M1", "d");
    assertTrue(sys.getContentByType("repo1", "Ship") == 2, "2 Ship entries");
    assertTrue(sys.getContentByType("repo1", "Module") == 1, "1 Module entry");
    assertTrue(sys.getContentByType("repo1", "Mission") == 0, "0 Mission entries");
}

void testCommunityContentDuplicate() {
    std::cout << "\n=== CommunityContentRepository: Duplicate ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    assertTrue(sys.submitContent("repo1", "dup1", "Ship", "a", "T", "D"), "First submit succeeds");
    assertTrue(!sys.submitContent("repo1", "dup1", "Ship", "b", "T2", "D2"), "Duplicate id rejected");
    assertTrue(sys.getContentCount("repo1") == 1, "Still 1 content after dup attempt");
}

void testCommunityContentWorkflow() {
    std::cout << "\n=== CommunityContentRepository: Workflow ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    world.createEntity("repo1");
    sys.createRepo("repo1");
    sys.submitContent("repo1", "w1", "Skin", "a", "Skin W", "desc");
    assertTrue(sys.getContentState("repo1", "w1") == "Draft", "Starts Draft");
    sys.publishContent("repo1", "w1");
    assertTrue(sys.getContentState("repo1", "w1") == "Published", "Transitions to Published");
    sys.featureContent("repo1", "w1");
    assertTrue(sys.getContentState("repo1", "w1") == "Featured", "Transitions to Featured");
    sys.archiveContent("repo1", "w1");
    assertTrue(sys.getContentState("repo1", "w1") == "Archived", "Transitions to Archived");
    assertTrue(sys.getTotalSubmissions("repo1") == 1, "Total submissions remains 1");
}

void testCommunityContentMissing() {
    std::cout << "\n=== CommunityContentRepository: Missing ===" << std::endl;
    ecs::World world;
    systems::CommunityContentRepositorySystem sys(&world);
    assertTrue(!sys.createRepo("nonexistent"), "Create fails on missing entity");
    assertTrue(!sys.submitContent("nonexistent", "x", "Ship", "a", "T", "D"), "Submit fails on missing");
    assertTrue(!sys.publishContent("nonexistent", "x"), "Publish fails on missing");
    assertTrue(!sys.featureContent("nonexistent", "x"), "Feature fails on missing");
    assertTrue(!sys.archiveContent("nonexistent", "x"), "Archive fails on missing");
    assertTrue(!sys.rateContent("nonexistent", "x", 5), "Rate fails on missing");
    assertTrue(!sys.downloadContent("nonexistent", "x"), "Download fails on missing");
    assertTrue(sys.getContentCount("nonexistent") == 0, "0 count on missing");
    assertTrue(sys.getTotalDownloads("nonexistent") == 0, "0 downloads on missing");
    assertTrue(sys.getContentState("nonexistent", "x") == "", "Empty state on missing");
}


void run_ui_tools_tests() {
    testUIPanelInit();
    testUIPanelOpenClose();
    testUIPanelToggle();
    testUIPanelAddItem();
    testUIPanelRemoveItem();
    testUIPanelFilter();
    testUIPanelSort();
    testUIPanelSelect();
    testUIPanelTypes();
    testUIPanelMissing();
    testKbNavInit();
    testKbNavFocus();
    testKbNavMoveFocus();
    testKbNavFocusStack();
    testKbNavBindKey();
    testKbNavModal();
    testKbNavKeyInput();
    testKbNavCursorBlink();
    testKbNavInputBuffer();
    testKbNavMissing();
    testDataBindingInit();
    testDataBindingAdd();
    testDataBindingRemove();
    testDataBindingUpdate();
    testDataBindingObserver();
    testDataBindingRemoveObserver();
    testDataBindingNotify();
    testDataBindingDirty();
    testDataBindingMaxBindings();
    testDataBindingMissing();
    testStressTestInit();
    testStressTestStart();
    testStressTestRecordTick();
    testStressTestRecordQuery();
    testStressTestEntityCount();
    testStressTestBudget();
    testStressTestOverBudget();
    testStressTestMaxTick();
    testStressTestComplete();
    testStressTestMissing();
    testSalvagePathInit();
    testSalvagePathEntry();
    testSalvagePathCutting();
    testSalvagePathExploration();
    testSalvagePathLoot();
    testSalvagePathDiscover();
    testSalvagePathCollect();
    testSalvagePathMultipleLoot();
    testSalvagePathActive();
    testSalvagePathMissing();
    testTaskSchedulerCreate();
    testTaskSchedulerAddTask();
    testTaskSchedulerRunning();
    testTaskSchedulerComplete();
    testTaskSchedulerPriority();
    testTaskSchedulerMaxConcurrent();
    testTaskSchedulerDependency();
    testTaskSchedulerCancel();
    testTaskSchedulerFailedDep();
    testTaskSchedulerMissing();
    testModManagerCreate();
    testModManagerInstall();
    testModManagerUninstall();
    testModManagerEnable();
    testModManagerDependency();
    testModManagerConflict();
    testModManagerMaxMods();
    testModManagerDuplicate();
    testModManagerLoadOrder();
    testModManagerMissing();
    testContentValidationCreate();
    testContentValidationSubmit();
    testContentValidationDuplicate();
    testContentValidationRun();
    testContentValidationApprove();
    testContentValidationReject();
    testContentValidationWorkflow();
    testContentValidationStateTransition();
    testContentValidationMissing();
    testVisualFeedbackCreate();
    testVisualFeedbackQueue();
    testVisualFeedbackMultiple();
    testVisualFeedbackExpiry();
    testVisualFeedbackFading();
    testVisualFeedbackPriority();
    testVisualFeedbackOverflow();
    testVisualFeedbackRemove();
    testVisualFeedbackCategory();
    testVisualFeedbackMissing();
    testModDocCreate();
    testModDocRegister();
    testModDocDuplicate();
    testModDocAddExample();
    testModDocValidate();
    testModDocValidateFail();
    testModDocGenerate();
    testModDocGenerateFail();
    testModDocCategory();
    testModDocMissing();
    testCommunityContentCreate();
    testCommunityContentSubmit();
    testCommunityContentPublish();
    testCommunityContentFeature();
    testCommunityContentArchive();
    testCommunityContentRate();
    testCommunityContentDownload();
    testCommunityContentByType();
    testCommunityContentDuplicate();
    testCommunityContentWorkflow();
    testCommunityContentMissing();
}
