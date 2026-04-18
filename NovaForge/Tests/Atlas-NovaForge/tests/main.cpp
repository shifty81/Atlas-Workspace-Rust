#include <iostream>

// GraphVM tests
void test_basic_arithmetic();
void test_subtraction();
void test_multiplication();
void test_division();
void test_division_by_zero();
void test_comparison();
void test_conditional_jump();
void test_variables();

// ECS tests
void test_create_entity();
void test_destroy_entity();
void test_tick_callback();

// ECS Component tests
void test_add_and_get_component();
void test_has_component();
void test_remove_component();
void test_multiple_components();
void test_destroy_entity_removes_components();
void test_component_update();

// Asset tests
void test_asset_binary_roundtrip();
void test_asset_registry_scan();

// Marketplace importer tests
void test_marketplace_registry();
void test_itch_io_importer();
void test_marketplace_metadata();
void test_marketplace_import_options();
void test_unreal_marketplace_importer();
void test_unity_assetstore_importer();

// Network tests
void test_net_init();
void test_net_authority();
void test_net_shutdown();

// Network queue tests
void test_net_add_peer();
void test_net_remove_peer();
void test_net_send_receive();
void test_net_broadcast_receive();
void test_net_shutdown_clears_queues();

// World tests
void test_cube_sphere_projection();
void test_cube_sphere_chunk_roundtrip();
void test_cube_sphere_neighbors();
void test_cube_sphere_lod();
void test_voxel_chunk_roundtrip();
void test_voxel_neighbors();

// Terrain mesh tests
void test_terrain_flat_mesh();
void test_terrain_heightmap_mesh();
void test_terrain_normals_computed();
void test_terrain_resolution_one();

// Noise tests
void test_perlin_range();
void test_perlin_deterministic();
void test_perlin_seed_variation();
void test_fbm_octaves();
void test_fbm_deterministic();
void test_perlin_spatial_variation();

// Streaming tests
void test_streamer_request_load();
void test_streamer_set_chunk_data();
void test_streamer_unload_chunk();
void test_streamer_get_loaded_chunks();
void test_streamer_disk_cache();
void test_streamer_duplicate_request();

// Galaxy tests
void test_galaxy_system_count();
void test_galaxy_deterministic();
void test_galaxy_seed_variation();
void test_galaxy_region_filter();
void test_galaxy_unique_ids();
void test_galaxy_star_classes();

// Compiler tests
void test_compile_constants_and_add();
void test_compile_and_execute_full();
void test_compile_multiply();

// Engine tests
void test_engine_init_and_shutdown();
void test_engine_run_loop_ticks();
void test_engine_capabilities();
void test_engine_net_mode_from_config();
void test_engine_frame_pacing_default_enabled();
void test_engine_frame_pacing_prevents_spin();
void test_engine_ui_update_in_loop();

// Logger tests
void test_logger_creates_log_directory();
void test_logger_writes_to_file();

// Console tests
void test_console_spawn_entity();
void test_console_ecs_dump();
void test_console_set_tickrate();
void test_console_net_mode();
void test_console_help();
void test_console_unknown_command();

// Interaction tests
void test_utterance_creation();
void test_intent_creation();
void test_rule_intent_resolver();
void test_interaction_system_uninitialized();
void test_interaction_system_no_match();
void test_interaction_system_full_pipeline();

// Interaction (graph-based) tests
void test_intent_registry_dispatch();
void test_intent_registry_missing();
void test_intent_registry_has_handler();
void test_pattern_resolver();
void test_pattern_resolver_no_match();
void test_pattern_resolver_case_insensitive();

// Diplomacy tests
void test_faction_request_neutral();
void test_faction_request_trusted();
void test_faction_request_hostile();
void test_faction_threaten_increases_hostility();
void test_faction_unknown_intent();
void test_faction_personality_affects_hostility();

// Project tests
void test_project_load_valid();
void test_project_load_minimal();
void test_project_load_missing_file();
void test_project_invalid_schema();
void test_project_missing_name();
void test_project_missing_name_field();
void test_project_invalid_version();
void test_project_schema_validation();

// Command tests
void test_command_execute();
void test_command_undo();
void test_command_redo();
void test_command_redo_cleared_on_execute();
void test_command_clear();
void test_command_last_executed();
void test_command_multiple_undo_redo();

// WorldGraph tests
void test_worldgraph_add_nodes();
void test_worldgraph_remove_node();
void test_worldgraph_compile_empty();
void test_worldgraph_compile_single_node();
void test_worldgraph_compile_chain();
void test_worldgraph_deterministic();
void test_worldgraph_clamp_node();
void test_worldgraph_biome_node();
void test_worldgraph_erosion_node();
void test_worldgraph_erosion_deterministic();

// Voice Command tests
void test_voice_register_command();
void test_voice_match_command();
void test_voice_no_match();
void test_voice_get_by_context();

// Plugin System tests
void test_plugin_validate_valid();
void test_plugin_validate_no_name();
void test_plugin_validate_nondeterministic();
void test_plugin_validate_incompatible_version();
void test_plugin_registry_register();
void test_plugin_registry_find();
void test_plugin_registry_unregister();
void test_plugin_registry_get_by_type();

// Heightfield tests
void test_heightfield_at();
void test_heightfield_mesh_generation();
void test_heightfield_mesh_lod();

// StrategyGraph tests
void test_strategygraph_add_nodes();
void test_strategygraph_remove_node();
void test_strategygraph_compile_empty();
void test_strategygraph_compile_single_node();
void test_strategygraph_compile_chain();
void test_strategygraph_execute();
void test_strategygraph_deterministic();
void test_strategygraph_objective_score();
void test_strategygraph_read_ai_signal();
void test_strategygraph_emit_action();

// Server Rules tests
void test_server_rules_register();
void test_server_rules_get_value();
void test_server_rules_set_multiplier();
void test_server_rules_clamp_multiplier();
void test_server_rules_missing_rule();
void test_server_rules_hot_reload_flag();
void test_server_rules_clear();

// ConversationGraph tests
void test_conversation_add_nodes();
void test_conversation_remove_node();
void test_conversation_compile_empty();
void test_conversation_dialogue_node();
void test_conversation_chain();
void test_conversation_relationship_delta();
void test_conversation_player_choice();

// AI Signal tests
void test_ai_signal_register();
void test_ai_signal_read_write();
void test_ai_signal_missing();
void test_ai_signal_namespace();
void test_ai_signal_clear();

// AI Memory tests
void test_ai_memory_store_recall();
void test_ai_memory_has();
void test_ai_memory_forget();
void test_ai_memory_decay();
void test_ai_memory_permanent();
void test_ai_memory_clear();

// Relationship Model tests
void test_relationship_set_get();
void test_relationship_default_zero();
void test_relationship_modify();
void test_relationship_has();
void test_relationship_remove();
void test_relationship_clear();

// Asset graph tests
void test_asset_context();
void test_asset_graph_executor_empty();
void test_asset_graph_executor_single_node();
void test_asset_graph_executor_multiple_nodes();
void test_damage_state();

// Mod tests
void test_mod_register_graph();
void test_mod_register_archetype();
void test_mod_registry_mixed();

// Editor assistant tests
void test_editor_assistant_explain_performance();
void test_editor_assistant_explain_node();
void test_editor_assistant_unknown();
void test_interaction_debugger_record();
void test_interaction_debugger_clear();

// Input tests
void test_input_bind_action();
void test_input_unbind_action();
void test_input_press_release();
void test_input_axis();
void test_input_callback();
void test_input_unbound_action();

// Camera tests
void test_camera_default_state();
void test_camera_set_mode();
void test_camera_position();
void test_camera_fov_clamp();
void test_camera_orbit();
void test_camera_movement();
void test_camera_pitch_clamp();

// Physics tests
void test_physics_create_body();
void test_physics_destroy_body();
void test_physics_gravity();
void test_physics_static_body();
void test_physics_apply_force();
void test_physics_collision_detection();

// Audio tests
void test_audio_load_sound();
void test_audio_unload_sound();
void test_audio_play_pause_stop();
void test_audio_volume();
void test_audio_master_volume();
void test_audio_looping();

// Gameplay mechanic tests
void test_mechanic_register();
void test_mechanic_unregister();
void test_mechanic_find_by_name();
void test_mechanic_params();
void test_mechanic_get_by_type();

// Skill tree tests
void test_skill_add_node();
void test_skill_unlock();
void test_skill_prerequisites();
void test_skill_effects();
void test_skill_tree_chain();

// Weapon Graph tests
void test_weapongraph_add_nodes();
void test_weapongraph_remove_node();
void test_weapongraph_compile_empty();
void test_weapongraph_compile_single_node();
void test_weapongraph_compile_chain();
void test_weapongraph_execute();
void test_weapongraph_wear_degrades_stats();
void test_weapongraph_deterministic();

// Character Graph tests
void test_charactergraph_add_nodes();
void test_charactergraph_remove_node();
void test_charactergraph_compile_empty();
void test_charactergraph_compile_single_node();
void test_charactergraph_compile_chain();
void test_charactergraph_execute();
void test_charactergraph_deterministic();
void test_charactergraph_facial_features();
void test_charactergraph_hair_style();
void test_charactergraph_facial_deterministic();

// Animation Graph tests
void test_animationgraph_add_nodes();
void test_animationgraph_remove_node();
void test_animationgraph_compile_empty();
void test_animationgraph_compile_single_node();
void test_animationgraph_compile_chain();
void test_animationgraph_execute();
void test_animationgraph_modifier();
void test_animationgraph_deterministic();

// Tile Graph tests
void test_tilegraph_add_nodes();
void test_tilegraph_remove_node();
void test_tilegraph_compile_empty();
void test_tilegraph_compile_single_node();
void test_tilegraph_compile_chain();
void test_tilegraph_execute();
void test_tilegraph_deterministic();

// Procedural Mesh Graph tests
void test_procedural_add_nodes();
void test_procedural_remove_node();
void test_procedural_compile_empty();
void test_procedural_cube();
void test_procedural_plane();
void test_procedural_sphere();
void test_procedural_transform();
void test_procedural_merge();
void test_procedural_subdivide();
void test_procedural_noise_determinism();
void test_procedural_full_pipeline();

// Procedural Material Graph tests
void test_material_solid_color();
void test_material_checkerboard();
void test_material_noise();
void test_material_blend();
void test_material_normal_map();
void test_material_graph_compile();
void test_material_graph_execute_solid();
void test_material_graph_blend_pipeline();
void test_material_graph_node_count();
void test_material_graph_remove_node();
void test_procedural_texture_generation();
void test_procedural_texture_deterministic();
void test_procedural_texture_non_repeating();

// LOD Baking Graph tests
void test_lod_decimate_mesh();
void test_lod_merge_vertices();
void test_lod_recompute_normals();
void test_lod_chain_generation();
void test_lod_graph_add_nodes();
void test_lod_graph_remove_node();
void test_lod_graph_compile_empty();
void test_lod_graph_execute_basic();
void test_lod_graph_decimate_pipeline();
void test_lod_graph_lod_chain_output();

// UI Logic Graph tests
void test_ui_logic_add_nodes();
void test_ui_logic_remove_node();
void test_ui_logic_compile_empty();
void test_ui_logic_compile_cycle_detection();
void test_ui_logic_condition_node();
void test_ui_logic_action_node();
void test_ui_logic_databind_node();
void test_ui_logic_transition_node();
void test_ui_logic_timer_node();
void test_ui_logic_gate_node_pass();
void test_ui_logic_gate_node_block();

// Sound Graph tests
void test_soundgraph_add_nodes();
void test_soundgraph_remove_node();
void test_soundgraph_compile_empty();
void test_soundgraph_compile_single_node();
void test_soundgraph_compile_chain();
void test_soundgraph_execute();
void test_soundgraph_deterministic();

// BehaviorGraph tests
void test_behaviorgraph_add_nodes();
void test_behaviorgraph_remove_node();
void test_behaviorgraph_compile_empty();
void test_behaviorgraph_compile_single_node();
void test_behaviorgraph_compile_chain();
void test_behaviorgraph_execute();
void test_behaviorgraph_deterministic();
void test_behaviorgraph_group_tactics();
void test_behaviorgraph_group_tactics_charge();
void test_behaviorgraph_adaptive_behavior();
void test_behaviorgraph_adaptive_difficulty_scaling();

// UI Screen tests
void test_ui_add_widget();
void test_ui_visibility();
void test_ui_parent_child();
void test_ui_remove_widget();

// Game Flow Graph tests
void test_gameflowgraph_add_nodes();
void test_gameflowgraph_remove_node();
void test_gameflowgraph_compile_empty();
void test_gameflowgraph_compile_single_node();
void test_gameflowgraph_compile_chain();
void test_gameflowgraph_execute();
void test_gameflowgraph_deterministic();

// Flow IR tests
void test_flow_ir_default_values();
void test_flow_ir_add_nodes();
void test_flow_ir_add_edges();
void test_flow_ir_to_json();
void test_flow_ir_roundtrip();
void test_flow_ir_validate_valid();
void test_flow_ir_validate_invalid();
void test_flow_ir_migrate_v1();

// Flow Codegen tests
void test_flow_codegen_nonempty();
void test_flow_codegen_function_signature();
void test_flow_codegen_empty_graph();
void test_flow_codegen_node_properties();
void test_flow_codegen_edge_wiring();
void test_flow_codegen_all_node_types();
void test_flow_codegen_header_comment();

// Flow Debugger tests
void test_debugger_initial_state();
void test_debugger_add_breakpoint();
void test_debugger_remove_breakpoint();
void test_debugger_clear_breakpoints();
void test_debugger_execute_no_breakpoints();
void test_debugger_breakpoint_pause();
void test_debugger_step_node();

// Shader IR tests
void test_shader_module_defaults();
void test_shader_passthrough_vertex();
void test_shader_solid_color_fragment();
void test_shader_validate_valid();
void test_shader_validate_empty();
void test_shader_hash_deterministic();
void test_shader_hash_different();
void test_shader_serialize_roundtrip();

// Build Manifest tests
void test_manifest_defaults();
void test_manifest_add_artifact();
void test_manifest_find_artifact();
void test_manifest_find_missing();
void test_manifest_validate_valid();
void test_manifest_validate_missing_fields();
void test_manifest_to_json();
void test_manifest_roundtrip();
void test_manifest_verify_hash();

// UIGraph tests
void test_uigraph_add_nodes();
void test_uigraph_remove_node();
void test_uigraph_compile_empty();
void test_uigraph_compile_single_node();
void test_uigraph_compile_chain();
void test_uigraph_execute();
void test_uigraph_deterministic();

// UICommandBus tests
void test_command_bus_enqueue();
void test_command_bus_drain();
void test_command_bus_clear();
void test_command_bus_dispatch();
void test_command_bus_dispatch_unhandled();
void test_command_bus_move_enqueue();

// EditorCommandBus tests
void test_editor_command_bus_post_and_process();
void test_editor_command_bus_undo();
void test_editor_command_bus_redo();
void test_editor_command_bus_redo_cleared_on_new_command();
void test_editor_command_bus_multiple_undo_redo();
void test_editor_command_bus_undo_empty();
void test_editor_command_bus_clear();
void test_editor_command_bus_listener();
void test_editor_command_bus_null_command();

// UIManager tests
void test_ui_manager_init();
void test_ui_manager_context_switch();
void test_ui_manager_screen_access();
void test_ui_manager_graph_access();
void test_ui_manager_update();
void test_ui_manager_command_bus();
void test_ui_manager_shutdown();
void test_ui_manager_server_context();

// UILayoutSolver tests
void test_layout_solver_single_entry();
void test_layout_solver_horizontal_split();
void test_layout_solver_vertical_split();
void test_layout_solver_weighted();
void test_layout_solver_min_size_respect();
void test_layout_solver_clear();
void test_layout_solver_deterministic();
void test_layout_solver_offset();

// UI Nodes Extended tests
void test_slotgrid_node_defaults();
void test_inputfield_node_defaults();
void test_inputfield_node_with_text();
void test_slotgrid_node_metadata();
void test_inputfield_node_metadata();

// Schema Validator tests
void test_schema_valid();
void test_schema_empty_id();
void test_schema_zero_version();
void test_schema_duplicate_node_id();
void test_schema_empty_node_id();
void test_schema_no_nodes();

// Graph Cache tests
void test_cache_store_and_get();
void test_cache_has();
void test_cache_invalidate();
void test_cache_invalidate_all();
void test_cache_evict_before();
void test_cache_hash_key();

// Graph Serialization tests
void test_json_builder_object();
void test_json_builder_array();
void test_json_reader_parse();
void test_json_reader_array();
void test_json_roundtrip();

// ECS Serialization tests
void test_ecs_serialize_empty_world();
void test_ecs_serialize_entities_no_components();
void test_ecs_serialize_roundtrip();
void test_ecs_serialize_preserves_next_id();
void test_ecs_deserialize_clears_existing();
void test_ecs_deserialize_invalid_data();
void test_ecs_has_serializer();

// Snapshot / Rollback tests
void test_snapshot_saves_ecs_state();
void test_rollback_restores_ecs_state();
void test_rollback_removes_future_snapshots();
void test_snapshot_without_world();
void test_rollback_with_multiple_entities();
void test_record_and_replay_input();
void test_replay_applies_input_frames();

// ECS Inspector tests
void test_inspector_empty_world();
void test_inspector_entities_with_components();
void test_inspector_select_entity();
void test_inspector_refreshes_on_draw();
void test_inspector_state_blocks_empty();
void test_inspector_state_blocks_with_world_state();

// Replication tests
void test_replication_add_rule();
void test_replication_remove_rule();
void test_replication_get_rule();
void test_replication_replace_rule();
void test_replication_dirty_tracking();
void test_replication_dirty_no_duplicates();
void test_replication_collect_delta();
void test_replication_apply_delta();
void test_replication_multiple_rules();
void test_replication_delta_roundtrip();
void test_replication_delta_every_tick();

// Asset Browser tests
void test_asset_browser_empty();
void test_asset_browser_with_assets();
void test_asset_browser_filter();
void test_asset_browser_sort_modes();
void test_asset_browser_select_asset();

// Asset Cooker tests
void test_cooker_default_settings();
void test_cooker_set_output_dir();
void test_cooker_cook_single_asset();
void test_cooker_source_not_found();
void test_cooker_cook_all();
void test_cooker_strip_editor_data();
void test_cooker_clear_log();
void test_cooker_progress_callback();

// Graph Editor Panel tests
void test_graph_panel_no_graph();
void test_graph_panel_with_nodes();
void test_graph_panel_select_node();
void test_graph_panel_compile_and_preview();
void test_graph_panel_summary();
void test_graph_panel_node_info();

// Asset Graph Assistant tests
void test_assistant_suggest_empty();
void test_assistant_suggest_with_executor();
void test_assistant_explain_no_executor();
void test_assistant_explain_with_nodes();
void test_assistant_mutate_no_executor();
void test_assistant_mutate_intensity();

// Story tests
void test_story_add_node();
void test_story_connections();
void test_story_tags();
void test_story_get_by_type();

// Build Profile tests
void test_profile_default_debug();
void test_profile_default_development();
void test_profile_default_release();
void test_profile_name();
void test_profile_set_active();
void test_profile_set_custom_config();
void test_profile_overrides();
void test_profile_override_missing_key();

// Mod Loader tests
void test_mod_loader_register();
void test_mod_loader_register_invalid();
void test_mod_loader_register_duplicate();
void test_mod_loader_unregister();
void test_mod_loader_enable_disable();
void test_mod_loader_get_mod();
void test_mod_loader_dependencies();
void test_mod_loader_scan_directory();
void test_mod_loader_clear();

// Platform Target tests
void test_platform_default_windows();
void test_platform_default_linux();
void test_platform_default_macos();
void test_platform_default_web();
void test_platform_name();
void test_platform_set_target();
void test_platform_custom_config();
void test_platform_module_exclusion();
void test_platform_host();
void test_platform_supported_list();

// Profiler Panel tests
void test_profiler_empty();
void test_profiler_record_frame();
void test_profiler_average_and_peak();
void test_profiler_max_history();
void test_profiler_system_metrics();
void test_profiler_pause();
void test_profiler_clear();

// Replay Recorder tests
void test_replay_initial_state();
void test_replay_start_recording();
void test_replay_record_frames();
void test_replay_stop_recording();
void test_replay_save_and_load();
void test_replay_load_invalid();
void test_replay_clear();

// State Hasher tests
void test_hasher_initial_state();
void test_hasher_advance_tick();
void test_hasher_deterministic();
void test_hasher_chaining();
void test_hasher_divergence_detection();
void test_hasher_different_seeds();
void test_hasher_empty_data();
void test_hasher_raw_pointer_api();
void test_replay_record_with_hash();
void test_replay_save_load_with_hash();
void test_replay_default_hash_zero();
void test_hash_combine_deterministic();

// Visual Diff tests
void test_diff_identical();
void test_diff_added();
void test_diff_removed();
void test_diff_modified();
void test_diff_empty();
void test_diff_compare_graphs();
void test_diff_summarize();

// Crash Handler tests
void test_crash_breadcrumb();
void test_crash_breadcrumb_limit();
void test_crash_clear_breadcrumbs();
void test_crash_generate_report();
void test_crash_save_report();
void test_crash_callback();
void test_crash_report_to_file();

// Net Inspector Panel tests
void test_net_inspector_standalone();
void test_net_inspector_server_with_peers();
void test_net_inspector_mode_names();

// Game Packager Panel tests
void test_game_packager_defaults();
void test_game_packager_summary();
void test_game_packager_modify_settings();
void test_package_config_strip_tools_default();

// Game Module tests
void test_game_module_describe();
void test_game_module_lifecycle();
void test_module_loader_static();
void test_module_loader_already_loaded();
void test_module_loader_not_found();

// Atlas Gameplay tests
void test_faction_register();
void test_faction_relations();
void test_faction_clear();
void test_combat_register_unit();
void test_combat_resolve();
void test_combat_clear();
void test_economy_register();
void test_economy_transactions();
void test_economy_capacity();
void test_economy_clear();

// EveOffline Module tests
void test_eveoffline_describe();
void test_eveoffline_register_types();
void test_eveoffline_replication_rules();
void test_eveoffline_server_rules();
void test_eveoffline_lifecycle();
void test_eveoffline_factory();

// Arena2D Module tests
void test_arena2d_describe();
void test_arena2d_register_types();
void test_arena2d_replication_rules();
void test_arena2d_server_rules();
void test_arena2d_lifecycle();
void test_arena2d_factory();

// Replay Timeline Panel tests
void test_replay_timeline_empty();
void test_replay_timeline_load();
void test_replay_timeline_scrub();
void test_replay_timeline_markers();
void test_replay_timeline_compare_identical();
void test_replay_timeline_compare_divergent();
void test_replay_timeline_inject_input();
void test_replay_timeline_branch();

// Replay Divergence Inspector tests
void test_divergence_no_mismatch();
void test_divergence_hash_mismatch();
void test_divergence_frame_comparison();
void test_divergence_single_frame_check();
void test_divergence_report_format();
void test_divergence_history();
void test_divergence_severity();

// Deterministic Allocator tests
void test_allocator_create();
void test_allocator_single_alloc();
void test_allocator_multiple_allocs();
void test_allocator_dealloc();
void test_allocator_reuse_space();
void test_allocator_stable_ids();
void test_allocator_reset();
void test_allocator_deterministic();

// Permission Manager tests
void test_permission_defaults();
void test_permission_register_user();
void test_permission_tier_viewonly();
void test_permission_tier_developer();
void test_permission_tier_admin();
void test_permission_grant_custom();
void test_permission_revoke();
void test_permission_user_management();

// HUD Overlay tests
void test_hud_defaults();
void test_hud_tick_display();
void test_hud_warnings();
void test_hud_critical_warning();
void test_hud_time_controls();
void test_hud_step_forward();
void test_hud_determinism_status();
void test_hud_visibility();
void test_hud_world_dilation();
void test_hud_world_paused();

// Time Model tests
void test_time_model_defaults();
void test_time_model_set_tick_rate();
void test_time_model_advance_tick();
void test_time_model_world_dilation();
void test_time_model_world_pause();
void test_time_model_presentation();
void test_time_model_reset();
void test_simulation_time_elapsed();

// World State tests
void test_world_state_register_block();
void test_world_state_find_block();
void test_world_state_snapshot();
void test_world_state_snapshot_deterministic();
void test_world_state_push_and_retrieve();
void test_world_state_max_snapshots();
void test_world_state_prune();
void test_world_state_clear();
void test_world_state_derived_rebuild();

// Save System tests
void test_save_system_save_and_load();
void test_save_system_hash_integrity();
void test_save_system_file_not_found();
void test_save_system_validate();
void test_save_system_invalid_format();
void test_save_system_empty_data();
void test_save_system_clear();

// Engine Integration tests
void test_engine_time_model_initialized();
void test_engine_time_model_tick_rate_matches();
void test_engine_time_model_world_time_advances();
void test_engine_world_state_snapshots_in_server();
void test_engine_world_state_snapshot_at_tick();
void test_engine_save_system_accessible();
void test_console_save_command();
void test_console_load_command();
void test_console_load_file_not_found();
void test_console_save_no_path();
void test_console_time_info();
void test_ai_behavior_deterministic_across_runs();
void test_ai_relationship_determinism_across_save_load();

// Next Tasks tests
void test_engine_autosave_creates_file();
void test_time_model_set_tick();
void test_replay_save_point_mark();
void test_replay_save_point_multiple();
void test_replay_save_point_serialization();
void test_replay_start_from_save();
void test_engine_load_and_replay();
void test_divergence_export_report();
void test_engine_rollback_to_tick();
void test_net_broadcast_save_tick();
void test_state_hash_diff_panel_no_divergence();
void test_state_hash_diff_panel_with_divergence();
void test_hash_ladder_save_load_continuity();
void test_engine_replay_from_save();
void test_engine_replay_from_save_bad_save();
void test_engine_replay_from_save_bad_replay();
void test_engine_replay_from_save_no_frames_after_save_point();
void test_state_hash_diff_panel_component_breakdown();
void test_state_hash_diff_panel_no_component_divergence();

// Next Tasks Phase 2 tests
void test_behaviorgraph_serialize_state();
void test_behaviorgraph_serialize_empty();
void test_partial_save_and_load();
void test_partial_save_hash_integrity();
void test_partial_save_empty_chunks();
void test_save_file_browser_scan();
void test_save_file_browser_empty_dir();
void test_save_file_browser_select();
void test_hash_ladder_comparison();
void test_hash_ladder_comparison_no_divergence();
void test_divergence_detail();
void test_divergence_detail_no_divergence();
void test_replay_auto_divergence_callback();
void test_replay_no_divergence_callback();
void test_detailed_divergence_report();
void test_detailed_divergence_report_no_divergence();

// GUI DSL Parser tests
void test_dsl_parse_simple_layout();
void test_dsl_parse_split();
void test_dsl_parse_nested_splits();
void test_dsl_parse_dock();
void test_dsl_parse_tabs();
void test_dsl_parse_full_example();
void test_dsl_parse_comments();
void test_dsl_parse_error_missing_layout();
void test_dsl_parse_error_unterminated_string();
void test_dsl_parse_error_unknown_statement();
void test_dsl_version();
void test_dsl_parse_dock_targets();

// GUI Input Recorder tests
void test_recorder_initial_state();
void test_recorder_start_stop_recording();
void test_recorder_record_events();
void test_recorder_record_only_when_recording();
void test_recorder_playback();
void test_recorder_save_load();
void test_recorder_load_invalid_file();
void test_recorder_load_nonexistent_file();

// Headless GUI tests
void test_headless_gui_init();
void test_headless_gui_available_commands();
void test_headless_gui_widget_count();
void test_headless_gui_widget_info();
void test_headless_gui_widget_info_not_found();
void test_headless_gui_unknown_command();
void test_headless_gui_empty_command();
void test_headless_gui_help();
void test_headless_gui_status();
void test_headless_gui_enqueue();
void test_headless_gui_custom_command();
void test_headless_gui_quoted_args();
void test_headless_gui_diag_toggle();
void test_headless_gui_diag_show_hide();
void test_headless_gui_diag_status();

// Job Execution Tracer
void test_job_tracer_empty();
void test_job_tracer_single_tick();
void test_job_tracer_multiple_ticks();
void test_job_tracer_deterministic_hash();
void test_job_tracer_different_order_different_hash();
void test_job_tracer_compare_order();
void test_job_tracer_max_history();
void test_job_tracer_clear();
void test_job_trace_panel_no_tracer();
void test_job_trace_panel_consistent();
void test_job_trace_panel_mismatch();
void test_job_trace_panel_entries_at_tick();

// Render and Platform tests
void test_render_api_enum();
void test_null_renderer();
void test_gl_renderer_viewport();
void test_vulkan_renderer_viewport();
void test_ui_manager_set_renderer();
void test_ui_manager_render_empty();
void test_ui_manager_render_widgets();
void test_ui_manager_render_null_renderer();
void test_engine_config_render_fields();
void test_engine_headless_no_window();
void test_engine_server_no_window();
void test_platform_window_event_type();
void test_platform_window_config();
void test_platform_has_window_implementation();
void test_engine_no_window_error_without_platform();

// Viewport Framebuffer
void test_null_viewport_framebuffer_defaults();
void test_null_viewport_framebuffer_sized();
void test_null_viewport_framebuffer_resize();
void test_null_viewport_framebuffer_bind_unbind();
void test_engine_editor_has_viewport_framebuffer();
void test_engine_client_no_viewport_framebuffer();
void test_engine_server_no_viewport_framebuffer();

// Phase 13: GLViewportFramebuffer, ScaleLayout, viewport propagation
void test_gl_viewport_fb_default_state();
void test_gl_viewport_fb_create_requires_nonzero();
void test_gl_viewport_fb_resize_to_zero();
void test_gl_viewport_fb_bind_unbind_safe();
void test_scale_layout_basic();
void test_scale_layout_identity();
void test_scale_layout_zero_old_noop();
void test_scale_layout_zero_new_noop();
void test_scale_layout_shrink();
void test_ui_manager_viewport_propagates_layout();
void test_ui_manager_viewport_first_set_no_scale();
void test_engine_editor_sets_initial_viewport();

// Phase 14: Vulkan Device, HttpLLMBackend, FontBootstrap enhancements
void register_next_tasks_phase14();

// Phase 15: MeshViewerPanel and PrefabEditorPanel Draw implementations
void register_next_tasks_phase15();

// Phase 16: AI assistant LLM wiring + permission enforcement
void register_next_tasks_phase16();

// Phase 17: Component migration, SimMirror, DesyncVisualizerPanel
void register_next_tasks_phase17();

// Networking improvements: latency/jitter, CRC, bandwidth enforcement, manual replication, reliable/unreliable
void register_net_improvements_tests();

// Phase 19: ProceduralGenerator, AIDebuggerPanel, HotReloadConfig
void register_next_tasks_phase19();

// Phase 18: TickStepDebugger, SimulationStateAuditor, FPDriftDetector
void register_next_tasks_phase18();

// Phase 20: DesyncReproducer, QoSScheduler, ServerAssetValidator
void register_next_tasks_phase20();

// Panel Draw() implementations
void register_panel_draw_impl_tests();

// Component Category
void test_component_category_defaults();
void test_component_category_simulated();
void test_component_category_presentation();
void test_component_category_debug();
void test_component_category_derived();
void test_component_category_assert_simulated();
void test_component_category_assert_not_presentation();
void test_component_category_runtime_query();

// Include Firewall tests
void test_include_firewall_sim_no_render();
void test_include_firewall_contract_no_render();
void test_no_imgui_in_engine();
void test_no_imgui_in_editor();
void test_no_imgui_in_client();
void test_no_imgui_in_server();

// FP Drift Detector tests
void test_fp_drift_no_drift();
void test_fp_drift_detected();
void test_fp_drift_system_comparison();
void test_fp_drift_format_report();
void test_fp_drift_export_report();
void test_fp_drift_add_system_hashes();

// Asset Import Pipeline tests
void test_asset_import_registry_empty();
void test_asset_import_registry_register();
void test_asset_import_mesh_can_import();
void test_asset_import_texture_can_import();
void test_asset_import_audio_can_import();
void test_asset_import_font_can_import();
void test_asset_import_sprite_can_import();
void test_asset_import_find_importer();
void test_asset_import_file_roundtrip();
void test_asset_import_file_not_found();
void test_asset_import_registry_import();

// Asset Validator tests
void test_asset_validator_validate_valid_file();
void test_asset_validator_validate_corrupt_file();
void test_asset_validator_validate_header_bad_magic();
void test_asset_validator_migration_path();
void test_asset_validator_dependencies();
void test_asset_validator_circular_dependency();
void test_asset_validator_no_circular_dependency();
void test_asset_validator_compute_file_hash();

// Golden Replay tests
void run_golden_replay_tests();

// Enforcement tests
void test_tick_rate_lock();
void test_tick_rate_set_before_lock();
void test_engine_locks_tick_rate_on_run();
void test_engine_system_order();
void test_engine_system_order_deterministic();
void test_net_packet_schema_version();
void test_asset_lock_and_check();
void test_asset_lock_multiple();
void test_determinism_config_exists();

// Lockdown Phase 2 tests
void test_rollback_and_verify_success();
void test_rollback_and_verify_bad_ticks();
void test_asset_schema_lock();
void test_asset_schema_lock_default();
void test_layout_hash_deterministic();
void test_layout_hash_differs_on_change();
void test_layout_hash_empty();
void test_mutation_ownership_register();
void test_mutation_ownership_exclusive();
void test_mutation_can_mutate();
void test_mutation_owned_components();
void test_mutation_owner_of_unregistered();

// Lockdown Phase 3 tests
void test_auditor_empty_passes();
void test_auditor_system_no_state_fails();
void test_auditor_system_with_state_passes();
void test_auditor_multiple_systems();
void test_auditor_duplicate_register();
void test_auditor_declared_blocks();
void test_auditor_unregistered_declaration_warning();
void test_auditor_clear();
void test_server_validator_empty_manifest();
void test_server_validator_register_and_check();
void test_server_validator_hash_match();
void test_server_validator_hash_mismatch();
void test_server_validator_unknown_asset();
void test_server_validator_file_validation();
void test_server_validator_file_not_found();
void test_server_validator_manifest();
void test_verify_save_load_determinism();
void test_desync_reproducer_output_dir();
void test_desync_reproducer_capture();
void test_desync_reproducer_generate_command();
void test_desync_reproducer_multiple_captures();

// GUI Completion
void test_editor_theme_dark_defaults();
void test_editor_theme_custom_values();
void test_panel_closable_by_default();
void test_panel_set_non_closable();
void test_layout_close_closable_panel();
void test_layout_close_non_closable_panel();
void test_layout_close_unknown_panel();
void test_layout_find_panel();
void test_main_panels_non_closable();
void test_attach_init();
void test_attach_standalone();
void test_attach_live_client();
void test_attach_live_client_missing_host();
void test_attach_headless_server();
void test_attach_replay();
void test_attach_replay_missing_path();
void test_attach_disconnect();
void test_default_editor_dsl_parses();
void test_default_editor_dsl_has_panels();

// Proof Viewer Panel tests
void test_proof_viewer_init();
void test_proof_viewer_load_specs();
void test_proof_viewer_select_spec();
void test_proof_viewer_tokenize();
void test_proof_viewer_invariants();
void test_proof_viewer_verification_status();
void test_proof_viewer_line_count();
void test_proof_viewer_empty_select();

// Replay Proof Exporter tests
void test_proof_exporter_empty();
void test_proof_exporter_add_entries();
void test_proof_exporter_validate_chain_valid();
void test_proof_exporter_validate_chain_invalid();
void test_proof_exporter_export_tla();
void test_proof_exporter_export_json();
void test_proof_exporter_export_csv();
void test_proof_exporter_save_points();
void test_proof_exporter_format_entry();
void test_proof_exporter_clear();

// CI Dashboard Panel tests
void test_ci_dashboard_init();
void test_ci_dashboard_start_run();
void test_ci_dashboard_add_checks();
void test_ci_dashboard_complete_run_passed();
void test_ci_dashboard_complete_run_failed();
void test_ci_dashboard_partial_failure();
void test_ci_dashboard_pass_rate();
void test_ci_dashboard_history();
void test_ci_dashboard_max_history();
void test_ci_dashboard_summary();
void test_ci_dashboard_clear();

// TLC Model Checker
void test_tlc_register_spec();
void test_tlc_register_multiple_specs();
void test_tlc_register_bad_path();
void test_tlc_unregister_spec();
void test_tlc_get_spec();
void test_tlc_extract_invariants_replay();
void test_tlc_extract_invariants_ecs();
void test_tlc_extract_invariants_layout();
void test_tlc_validate_spec();
void test_tlc_check_spec();
void test_tlc_check_spec_not_found();
void test_tlc_check_all();
void test_tlc_ci_check_pass();
void test_tlc_format_report();
void test_tlc_export_report();
void test_tlc_path_management();
void test_tlc_extract_invariants_static();

// Deterministic Animation Graph tests
void test_det_anim_add_nodes();
void test_det_anim_remove_node();
void test_det_anim_compile_empty();
void test_det_anim_rest_pose();
void test_det_anim_fk();
void test_det_anim_ik();
void test_det_anim_blend_tree();
void test_det_anim_bone_mask();
void test_det_anim_additive_blend();
void test_det_anim_deterministic();

// Collaborative Editor tests
void test_collab_add_peer();
void test_collab_remove_peer();
void test_collab_cursor_update();
void test_collab_submit_operation();
void test_collab_receive_remote();
void test_collab_detect_conflicts();
void test_collab_resolve_last_writer_wins();
void test_collab_resolve_first_writer_wins();
void test_collab_no_conflicts();
void test_collab_clear();

// AtlasAI Core
void test_ai_core_default_permissions();
void test_ai_core_set_permissions();
void test_ai_core_has_permission();
void test_ai_core_register_intent();
void test_ai_core_unregister_intent();
void test_ai_core_list_intents();
void test_ai_core_process_request();
void test_ai_core_permission_denied();
void test_ai_core_unknown_intent();
void test_ai_core_history();
void test_ai_core_clear();

// AI Asset Decision Framework
void run_ai_decision_framework_tests();

// Atlas Assistant Panel
void test_assistant_panel_name();
void test_assistant_panel_context();
void test_assistant_panel_submit_prompt();
void test_assistant_panel_suggestions();
void test_assistant_panel_apply_suggestion();
void test_assistant_panel_diff_preview();
void test_assistant_panel_conversation();
void test_assistant_panel_clear();

// AI Diff Viewer Panel
void test_diff_viewer_name();
void test_diff_viewer_no_diff();
void test_diff_viewer_load();
void test_diff_viewer_accept_hunk();
void test_diff_viewer_reject_hunk();
void test_diff_viewer_accept_all();
void test_diff_viewer_reject_all();
void test_diff_viewer_apply();
void test_diff_viewer_history();
void test_diff_viewer_clear();

// Flow Graph Refactorer
void test_flow_refactorer_load();
void test_flow_refactorer_find_dead_nodes();
void test_flow_refactorer_find_chains();
void test_flow_refactorer_find_duplicates();
void test_flow_refactorer_analyze();
void test_flow_refactorer_remove_dead();
void test_flow_refactorer_rename();
void test_flow_refactorer_simplify_chain();
void test_flow_refactorer_history();
void test_flow_refactorer_clear();

// Web Aggregation KB
void test_kb_add_entry();
void test_kb_get_entry();
void test_kb_remove_entry();
void test_kb_search();
void test_kb_search_by_category();
void test_kb_search_by_tag();
void test_kb_categories();
void test_kb_tags();
void test_kb_export_import();
void test_kb_clear();

// Game GUI Asset
void test_gui_asset_create_widget();
void test_gui_asset_add_child();
void test_gui_asset_find_widget();
void test_gui_asset_remove_widget();
void test_gui_asset_list_by_type();
void test_gui_asset_properties();
void test_gui_asset_bindings();
void test_gui_asset_validate();
void test_gui_asset_export_json();
void test_gui_asset_clear();

// Widget DSL
void test_widget_dsl_tokenize();
void test_widget_dsl_parse_simple();
void test_widget_dsl_parse_nested();
void test_widget_dsl_parse_properties();
void test_widget_dsl_validate_valid();
void test_widget_dsl_validate_invalid();
void test_widget_dsl_empty();
void test_widget_dsl_multiple_roots();

// Game GUI Binding
void test_binding_add();
void test_binding_remove();
void test_binding_get();
void test_binding_list();
void test_binding_evaluate_direct();
void test_binding_evaluate_formatted();
void test_binding_evaluate_computed();
void test_binding_custom_formatter();
void test_binding_validate();
void test_binding_clear();

// Game Mechanics UI Panel
void test_mechanics_ui_name();
void test_mechanics_ui_add_element();
void test_mechanics_ui_remove_element();
void test_mechanics_ui_get_element();
void test_mechanics_ui_list_elements();
void test_mechanics_ui_select_element();
void test_mechanics_ui_preview_mode();
void test_mechanics_ui_export();
void test_mechanics_ui_import();
void test_mechanics_ui_clear();

// Determinism Versioning
void test_dv_set_version();
void test_dv_register_fork();
void test_dv_unregister_fork();
void test_dv_get_fork();
void test_dv_list_forks();
void test_dv_compatible();
void test_dv_incompatible();
void test_dv_check_all();
void test_dv_report();
void test_dv_clear();

// Certified Build
void test_cert_default_level();
void test_cert_set_level();
void test_cert_add_artifact();
void test_cert_get_artifact();
void test_cert_list_artifacts();
void test_cert_verify_all_pass();
void test_cert_verify_with_failure();
void test_cert_generate_report();
void test_cert_export_json();
void test_cert_clear();

// Certified Build Verification Steps tests (Phase 3)
void test_cert_register_step();
void test_cert_run_all_steps();
void test_cert_run_all_steps_with_failure();
void test_cert_verify_file_hash();
void test_cert_verify_file_hash_not_found();
void test_cert_verify_test_results();
void test_cert_verify_contract_scan();
void test_cert_clear_clears_steps();

// Font Bootstrap tests
void test_font_bootstrap_initial_state();
void test_font_bootstrap_init_missing_font_generates_fallback();
void test_font_bootstrap_init_unwritable_root();
void test_font_bootstrap_shutdown();
void test_font_bootstrap_rebuild_noop();

// Diagnostics Overlay tests
void test_diagnostics_overlay_default_off();
void test_diagnostics_overlay_toggle();
void test_diagnostics_overlay_set_enabled();
void test_diagnostics_overlay_render_null_renderer();
void test_diagnostics_overlay_render_when_disabled();
void test_diagnostics_overlay_render_when_enabled();

// UI Event Router tests
void test_event_router_empty();
void test_event_router_register_dispatch();
void test_event_router_miss();
void test_event_router_z_order();
void test_event_router_unregister();
void test_event_router_hover_tracking();
void test_event_router_clear();

// UI Draw List tests
void test_draw_list_empty();
void test_draw_list_draw_rect();
void test_draw_list_draw_text();
void test_draw_list_draw_icon();
void test_draw_list_draw_border();
void test_draw_list_draw_image();
void test_draw_list_multiple_commands();
void test_draw_list_clear();
void test_draw_list_flush();
void test_draw_list_flush_null_renderer();

// Engine Phase tests
void test_engine_phase_to_string();
void test_engine_phase_values();

// Launcher Screen tests
void test_launcher_initial_state();
void test_launcher_scan_nonexistent();
void test_launcher_scan_atlas_descriptors();
void test_launcher_select_invalid();
void test_launcher_confirm_no_selection();
void test_launcher_request_new_project();
void test_launcher_request_quit();

// Launcher descriptor parsing tests (Phase 3)
void test_launcher_parses_project_name_from_descriptor();
void test_launcher_parses_last_opened();
void test_launcher_fallback_on_minimal_descriptor();
void test_launcher_project_description_field();

// DSL Layout tests (Phase 3)
void test_dsl_parses_default_editor_layout();
void test_dsl_layout_has_panels();
void test_dsl_layout_has_dock_and_split();

// Tile Editor tests
void test_tile_editor_name();
void test_tile_editor_register();
void test_tile_editor_modes();
void test_tile_editor_paint();
void test_tile_editor_erase();
void test_tile_editor_paint_no_tile_selected();
void test_tile_editor_active_layer();
void test_tile_editor_unregister();
void test_tile_editor_grid_coord_hash();
void test_tile_editor_brush_paint_single();
void test_tile_editor_brush_paint_square_3x3();
void test_tile_editor_brush_paint_circle();
void test_tile_editor_brush_erase();
void test_tile_editor_paint_rect();
void test_tile_editor_paint_rect_reversed_corners();
void test_tile_editor_erase_rect();
void test_tile_editor_flood_fill_empty_region();
void test_tile_editor_flood_fill_replace_tile();
void test_tile_editor_flood_fill_same_tile_noop();
void test_tile_editor_on_tile_painted_callback();
void test_tile_editor_on_tile_erased_callback();
void test_tile_editor_brush_paint_fires_callback();
void test_tile_editor_rect_paint_fires_callback();
void test_tile_editor_get_brush_shape();
void test_tile_editor_get_brush_size();

// Text Renderer tests
void test_null_text_renderer();
void test_glyph_default();
void test_font_atlas_default();

// Game Packager Build tests
void test_game_packager_validate_empty_source();
void test_game_packager_validate_nonexistent_source();
void test_game_packager_validate_valid();
void test_game_packager_package_empty_dir();
void test_game_packager_package_with_assets();
void test_game_packager_progress_callback();
void test_game_packager_last_report();
void test_game_packager_validation_failed();

// Layout Persistence tests
void test_layout_persistence_serialize_empty();
void test_layout_persistence_roundtrip();
void test_layout_persistence_save_load_file();
void test_layout_persistence_load_nonexistent();
void test_layout_persistence_panel_closable();
void test_layout_persistence_vertical_split();

// Net Hardening tests
void test_net_hardening_default_config();
void test_net_hardening_configure();
void test_net_hardening_connect_disconnect();
void test_net_hardening_connection_timeout();
void test_net_hardening_reconnect();
void test_net_hardening_packet_received_connects();
void test_net_hardening_bandwidth_limit();
void test_net_hardening_bandwidth_unlimited();
void test_net_hardening_packet_size_validation();
void test_net_hardening_rtt_tracking();
void test_net_hardening_heartbeat_timeout();
void test_net_hardening_heartbeat_reset();
void test_net_hardening_stats_tracking();
void test_net_hardening_reset_stats();
void test_net_hardening_state_callback();

// Play-In-Editor tests
void test_pie_initial_state();
void test_pie_start_stop();
void test_pie_pause_resume();
void test_pie_step_tick();
void test_pie_state_restore();
void test_pie_possess_entity();
void test_pie_possess_zero_fails();
void test_pie_loopback();
void test_pie_mode_callback();
void test_pie_double_start_fails();
void test_pie_stop_when_stopped();
void test_pie_auto_possess();

// Engine Input Routing
void test_engine_has_event_router();
void test_engine_mouse_tracking_defaults();
void test_engine_event_router_register();
void test_diagnostics_overlay_toggle_state();
void test_f3_key_constant_defined();
void test_window_event_scroll_wheel_type();
void test_window_event_mouse_button_zero_indexed();
void test_scroll_wheel_dispatch_through_ui_manager();

// World State Serializer
void test_serializer_default_version();
void test_serializer_set_version();
void test_serializer_serialize_roundtrip();
void test_serializer_serialize_empty();
void test_serializer_hash_mismatch();
void test_serializer_version_too_old();
void test_serializer_version_too_new();
void test_serializer_migration();
void test_serializer_can_migrate();
void test_serializer_migration_path();
void test_serializer_migration_failure();
void test_serializer_schema_version_comparison();

// Asset Category Registry
void test_category_registry_empty();
void test_category_registry_register();
void test_category_registry_replace();
void test_category_registry_unregister();
void test_category_registry_by_content();
void test_category_registry_by_system();
void test_category_registry_content_for();
void test_category_registry_system_for();
void test_category_registry_defaults();
void test_category_registry_clear();
void test_category_registry_unregistered_defaults();

// Replay Versioning
void test_replay_version_defaults();
void test_replay_version_set_versions();
void test_replay_version_register();
void test_replay_version_replace();
void test_replay_version_compatibility_current();
void test_replay_version_compatibility_too_new();
void test_replay_version_compatibility_too_old();
void test_replay_version_compatibility_upgradeable();
void test_replay_version_compatibility_unknown();
void test_replay_version_migration_path();
void test_replay_version_can_migrate();
void test_replay_version_deprecated();
void test_replay_version_all_versions();
void test_replay_version_migration_execution();
void test_replay_version_clear();

// Mesh Viewer Panel tests
void test_mesh_viewer_name();
void test_mesh_viewer_no_mesh();
void test_mesh_viewer_load_mesh();
void test_mesh_viewer_bounds();
void test_mesh_viewer_view_modes();
void test_mesh_viewer_show_normals();
void test_mesh_viewer_show_grid();
void test_mesh_viewer_select_vertex();
void test_mesh_viewer_summary();
void test_mesh_viewer_clear();

// Material Editor Panel tests
void test_material_editor_name();
void test_material_editor_no_material();
void test_material_editor_load_material();
void test_material_editor_add_parameter();
void test_material_editor_remove_parameter();
void test_material_editor_get_parameter();
void test_material_editor_set_parameter();
void test_material_editor_select_parameter();
void test_material_editor_preview_mode();
void test_material_editor_summary();
void test_material_editor_clear();

// Ship Hull Graph tests
void test_ship_hull_from_control_points();
void test_ship_hull_minimum_points();
void test_ship_hull_two_points();
void test_ship_turret_mount();
void test_ship_turret_sizes();
void test_ship_light_fixture();
void test_ship_light_interior_vs_exterior();
void test_ship_interior_volume();
void test_ship_visual_feature();
void test_ship_default_control_points_frigate();
void test_ship_default_control_points_capital();
void test_ship_default_control_points_deterministic();
void test_ship_default_hardpoints();
void test_ship_default_interiors();
void test_ship_default_interiors_large_ship();
void test_ship_default_lights();
void test_ship_graph_add_nodes();
void test_ship_graph_remove_node();
void test_ship_graph_compile_empty();
void test_ship_graph_control_points();
void test_ship_graph_turret_hardpoints();
void test_ship_graph_lights();
void test_ship_graph_interiors();
void test_ship_graph_visual_features();
void test_ship_graph_generate_from_seed();
void test_ship_graph_full_pipeline();
void test_ship_graph_export();
void test_ship_pcg_record_edits();
void test_ship_pcg_learning_profile();
void test_ship_pcg_apply_learning();

// Ship Editor Panel tests
void test_ship_editor_name();
void test_ship_editor_no_ship();
void test_ship_editor_new_ship();
void test_ship_editor_new_ship_classes();
void test_ship_editor_compile_and_generate();
void test_ship_editor_selection();
void test_ship_editor_tools();
void test_ship_editor_view_modes();
void test_ship_editor_move_control_point();
void test_ship_editor_add_control_point();
void test_ship_editor_remove_control_point();
void test_ship_editor_add_turret();
void test_ship_editor_remove_turret();
void test_ship_editor_add_light();
void test_ship_editor_remove_light();
void test_ship_editor_add_interior();
void test_ship_editor_remove_interior();
void test_ship_editor_add_visual_feature();
void test_ship_editor_remove_visual_feature();
void test_ship_editor_has_cockpit();
void test_ship_editor_export_import();
void test_ship_editor_pcg_learning();
void test_ship_editor_summary();
void test_ship_editor_draw();
void test_ship_editor_draw_no_ship();
void test_ship_editor_set_ship_info();
void test_ship_editor_mark_clean();

// Character Viewer / Editor / Animation tests
void test_human_mesh_generate_default();
void test_human_mesh_generate_from_context();
void test_human_mesh_deterministic();
void test_human_mesh_different_seeds();
void test_human_mesh_params_from_graph();
void test_human_mesh_height_affects_size();
void test_anim_controller_default_state();
void test_anim_controller_set_state();
void test_anim_controller_update();
void test_anim_controller_clip_lengths();
void test_anim_controller_speed_and_damage();
void test_char_select_init_slots();
void test_char_select_select_slot();
void test_char_select_confirm();
void test_char_select_rotation();
void test_char_select_zoom();
void test_char_select_animation();
void test_char_select_draw();
void test_char_select_regenerate();
void test_char_editor_name();
void test_char_editor_default_params();
void test_char_editor_set_params();
void test_char_editor_individual_setters();
void test_char_editor_regenerate_mesh();
void test_char_editor_sections();
void test_char_editor_animation();
void test_char_editor_draw();
void test_char_editor_summary();
void test_char_editor_callback();

// Prefab Editor Panel tests
void test_prefab_editor_name();
void test_prefab_editor_empty();
void test_prefab_editor_add_entity();
void test_prefab_editor_remove_entity();
void test_prefab_editor_parent_child();
void test_prefab_editor_remove_parent_removes_children();
void test_prefab_editor_remove_grandchildren();
void test_prefab_editor_remove_clears_selected_child();
void test_prefab_editor_add_component();
void test_prefab_editor_remove_component();
void test_prefab_editor_select_entity();
void test_prefab_editor_summary();
void test_prefab_editor_clear();

// Truth UI Panel tests
void test_truth_ui_name();
void test_truth_ui_defaults();
void test_truth_ui_wiring();
void test_truth_ui_summary_stopped();
void test_truth_ui_summary_no_pie();
void test_truth_ui_draw();

// Asset Diff Commit Flow tests
void test_asset_diff_commit_empty();
void test_asset_diff_commit_track_change();
void test_asset_diff_commit_untrack();
void test_asset_diff_commit_stage();
void test_asset_diff_commit_commit_success();
void test_asset_diff_commit_commit_no_staged();
void test_asset_diff_commit_validation_failure();
void test_asset_diff_commit_clear();
void test_asset_diff_commit_summary();
void test_asset_diff_commit_update_tracked();

// Cross-Platform Save Compatibility
void test_cross_platform_save_header_size();
void test_cross_platform_partial_header_size();
void test_cross_platform_save_magic_offset();
void test_cross_platform_save_hash_determinism();
void test_cross_platform_save_byte_exact();
void test_cross_platform_save_raw_bytes_roundtrip();
void test_cross_platform_partial_save_roundtrip();
void test_cross_platform_save_large_payload();
void test_cross_platform_hash_combine_deterministic();

// UIScrollState
void test_scroll_state_defaults();
void test_scroll_state_content_smaller_than_viewport();
void test_scroll_state_scrollable();
void test_scroll_state_scroll_down();
void test_scroll_state_scroll_up();
void test_scroll_state_clamp_top();
void test_scroll_state_clamp_bottom();
void test_scroll_state_scroll_to_top();
void test_scroll_state_scroll_to_bottom();
void test_scroll_state_viewport_resize_clamps();

// UIManager Viewport/DPI/Input/Font
void test_ui_manager_viewport_defaults();
void test_ui_manager_set_viewport_size();
void test_ui_manager_dpi_scale();
void test_ui_manager_event_router_access();
void test_ui_manager_dispatch_event();
void test_ui_manager_dispatch_not_initialized();
void test_ui_manager_font_bootstrap_access();
void test_ui_manager_shutdown_resets_viewport();

// Truth UI Panel Draw Commands
void test_truth_ui_draw_produces_commands();
void test_truth_ui_draw_no_pie_shows_message();
void test_truth_ui_draw_with_pie();
void test_truth_ui_draw_no_divergence();
void test_truth_ui_draw_clears_between_frames();
void test_truth_ui_draw_has_header();

// UI Style
void test_ui_style_dark_defaults();
void test_ui_style_light_theme();
void test_ui_style_spacing_defaults();
void test_ui_style_typography_defaults();
void test_ui_style_border_defaults();
void test_ui_style_status_colors();
void test_ui_style_dark_light_differ();

// UI Scene Graph
void test_scene_graph_init();
void test_scene_graph_add_child();
void test_scene_graph_remove_child();
void test_scene_graph_find_child();
void test_scene_graph_find_nested_child();
void test_scene_graph_layout_vertical();
void test_scene_graph_layout_horizontal();
void test_scene_graph_hit_test();
void test_scene_graph_draw_tree();
void test_scene_graph_invisible_child_skipped();
void test_scene_graph_dispatch_event();

// Tile Chunk Builder
void test_chunk_builder_empty_layer();
void test_chunk_builder_single_tile();
void test_chunk_builder_multiple_tiles();
void test_chunk_builder_world_to_chunk();
void test_chunk_builder_world_to_chunk_negative();
void test_chunk_builder_is_inside_chunk();
void test_chunk_builder_mark_dirty();
void test_chunk_builder_tiles_outside_chunk_ignored();
void test_chunk_builder_deterministic();
void test_chunk_builder_flip_flags();

// Tile Palette Panel
void test_tile_palette_name();
void test_tile_palette_add_entries();
void test_tile_palette_selection();
void test_tile_palette_selection_oob();
void test_tile_palette_filter();
void test_tile_palette_favorites();
void test_tile_palette_favorites_only();
void test_tile_palette_columns();
void test_tile_palette_clear();
void test_tile_palette_draw();
void test_tile_palette_draw_with_filter();

// Rule Graph Editor Panel
void test_rule_graph_editor_name();
void test_rule_graph_editor_add_node();
void test_rule_graph_editor_remove_node();
void test_rule_graph_editor_add_wire();
void test_rule_graph_editor_remove_wires_for_node();
void test_rule_graph_editor_remove_node_removes_wires();
void test_rule_graph_editor_selection();
void test_rule_graph_editor_pan_zoom();
void test_rule_graph_editor_debug_mode();
void test_rule_graph_editor_draw();
void test_rule_graph_editor_draw_debug();
void test_rule_graph_editor_draw_with_selection();
void run_next_tasks_phase4_tests();
void run_next_tasks_phase5_tests();
void run_next_tasks_phase6_tests();
void run_next_tasks_phase7_tests();
void run_next_tasks_phase8_tests();
void run_next_tasks_phase9_tests();
void run_next_tasks_phase10_tests();
void run_next_tasks_phase11_tests();
void run_next_tasks_phase12_tests();
void run_panel_draw_tests();

// Menu System tests
void test_menu_creation();
void test_menu_state();
void test_menu_manager();
void test_menu_hover();
void test_ui_manager_integration();

// GUI Full Completion tests
void test_widget_disabled_state();
void test_disabled_item_not_clickable();
void test_disabled_item_widget_direct();
void test_shortcut_label_on_widget();
void test_shortcut_label_empty_by_default();
void test_shortcut_label_multiple_items();
void test_keyboard_nav_down();
void test_keyboard_nav_up();
void test_keyboard_nav_enter_select();
void test_keyboard_nav_escape_close();
void test_keyboard_nav_skip_disabled();
void test_keyboard_nav_skip_separator();
void test_keyboard_nav_left_right_switch_menu();
void test_keyboard_no_consume_when_closed();
void test_context_menu_open_close();
void test_context_menu_item_click();
void test_context_menu_keyboard_nav();
void test_submenu_flag_on_widget();
void test_submenu_open_via_keyboard();
void test_submenu_close_via_left();
void test_submenu_escape_closes_submenu_first();
void test_toolbar_widget_creation();
void test_toolbar_with_button_children();
void test_toolbar_rendering_with_null_renderer();
void test_statusbar_widget_creation();
void test_statusbar_text_update();
void test_statusbar_rendering_with_null_renderer();
void test_full_editor_menu_setup();
void test_menu_manager_is_any_open();
void test_close_all_menus_clears_focus();

// Menu overlay and System tab tests
void test_menu_overlay_renders_on_top();
void test_menu_overlay_not_rendered_when_closed();
void test_logger_sink_callback();
void test_system_tab_creation();

// Menu action wiring tests
void test_engine_request_exit();
void test_menu_toggle_panel_visibility();
void test_menu_action_item_dispatch();
void test_status_bar_update();

// GUI next tasks tests
void test_checkable_default_false();
void test_set_checkable();
void test_set_checked();
void test_checkable_toggle_on_click();
void test_checkable_toggle_via_keyboard();
void test_icon_id_default_zero();
void test_set_icon_id();
void test_icon_rendering_with_null_renderer();
void test_tooltip_widget_creation();
void test_tooltip_rendering_with_null_renderer();
void test_tab_widget_creation();
void test_tab_active_state();
void test_tab_rendering_with_null_renderer();
void test_scrollview_widget_creation();
void test_scrollview_rendering_with_null_renderer();
void test_dockarea_widget_creation();
void test_dockarea_with_panels();
void test_dockarea_rendering_with_null_renderer();
void test_editor_checkable_view_items();

// GUI Managers tests
void test_tab_manager_activate();
void test_tab_manager_no_change_same_tab();
void test_tab_manager_get_active();
void test_tab_manager_content_visibility();
void test_tab_manager_callback();
void test_tab_manager_handle_click();
void test_scroll_manager_register();
void test_scroll_manager_scroll_wheel();
void test_scroll_manager_scroll_outside();
void test_scroll_manager_not_scrollable();
void test_scroll_manager_to_top_bottom();
void test_scroll_manager_set_content_height();
void test_toolbar_manager_click();
void test_toolbar_manager_toggle();
void test_toolbar_manager_miss();
void test_scroll_wheel_event_type();
void test_ui_manager_tab_integration();
void test_ui_manager_scroll_integration();
void test_ui_manager_toolbar_integration();

// GUI Next Managers (FocusManager, TooltipManager, DockManager, InputFieldManager)
void test_focus_manager_init();
void test_focus_manager_click_input_field();
void test_focus_manager_click_outside();
void test_focus_manager_set_focusable();
void test_focus_manager_clear_focus();
void test_focus_manager_tab_cycles();
void test_focus_manager_callback();
void test_tooltip_manager_init();
void test_tooltip_manager_set_tooltip();
void test_tooltip_manager_show_after_delay();
void test_tooltip_manager_hide_on_leave();
void test_tooltip_manager_hide_all();
void test_tooltip_manager_remove_tooltip();
void test_dock_manager_init();
void test_dock_manager_register();
void test_dock_manager_dock_panel_left();
void test_dock_manager_dock_panel_right();
void test_dock_manager_dock_panel_center();
void test_dock_manager_undock_panel();
void test_dock_manager_callback();
void test_dock_manager_split_ratio();
void test_input_field_manager_init();
void test_input_field_manager_register();
void test_input_field_manager_type_text();
void test_input_field_manager_backspace();
void test_input_field_manager_delete_key();
void test_input_field_manager_arrow_keys();
void test_input_field_manager_enter_submits();
void test_input_field_manager_text_changed_callback();
void test_input_field_manager_set_text();
void test_input_field_manager_not_registered();
void test_ui_manager_focus_integration();
void test_ui_manager_dock_integration();
void test_ui_manager_tooltip_integration();

// Crash Reporter tests
void test_crash_reporter_tool_exists();
void test_crash_reporter_help();
void test_crash_reporter_validate_empty_dir();
void test_crash_reporter_validate_manifest();
void test_crash_reporter_bundle();

// Include Firewall Enhancement tests
void test_include_firewall_header_exists();
void test_include_firewall_defines_marker();
void test_include_firewall_guards_simulation();
void test_include_firewall_guards_core();
void test_include_firewall_ecs_no_render();
void test_include_firewall_physics_no_render();
void test_crash_reporter_workflow_exists();

// Editor UI Fix tests
void run_editor_ui_fix_tests();

// GUI Interaction Fix tests
void run_gui_interaction_fix_tests();

// GUI New Widgets tests
void test_checkbox_widget_creation();
void test_checkbox_toggle();
void test_checkbox_rendering_with_null_renderer();
void test_slider_widget_creation();
void test_slider_set_value();
void test_slider_value_range();
void test_slider_rendering_with_null_renderer();
void test_progressbar_widget_creation();
void test_progressbar_set_value();
void test_progressbar_rendering_with_null_renderer();
void test_combobox_widget_creation();
void test_combobox_selected_index();
void test_combobox_open_state();
void test_combobox_rendering_with_null_renderer();
void test_treenode_widget_creation();
void test_treenode_expand_collapse();
void test_treenode_depth();
void test_treenode_rendering_with_null_renderer();
void test_splitter_widget_creation();
void test_splitter_rendering_with_null_renderer();
void test_colorpicker_widget_creation();
void test_colorpicker_set_get_color();
void test_colorpicker_rendering_with_null_renderer();
void test_slider_manager_init();
void test_slider_manager_click_sets_value();
void test_slider_manager_drag_updates_value();
void test_slider_manager_clamps_value();
void test_slider_manager_click_outside_ignored();
void test_combobox_manager_init();
void test_combobox_manager_set_items();
void test_combobox_manager_click_opens();
void test_combobox_manager_select_item();
void test_combobox_manager_click_outside_closes();
void test_combobox_manager_get_selected_text_no_selection();
void test_combobox_manager_empty_items();
void test_ui_manager_slider_integration();
void test_ui_manager_combobox_integration();
void test_ui_manager_new_widget_rendering();

// GUI Interaction Manager tests
void test_checkbox_manager_init();
void test_checkbox_manager_click_toggles();
void test_checkbox_manager_click_outside_ignored();
void test_checkbox_manager_ignores_non_click();
void test_treenode_manager_init();
void test_treenode_manager_click_toggles_expand();
void test_treenode_manager_click_outside_ignored();
void test_splitter_manager_init();
void test_splitter_manager_drag_moves();
void test_splitter_manager_click_outside_ignored();
void test_splitter_manager_horizontal_drag();
void test_colorpicker_manager_init();
void test_colorpicker_manager_click_opens();
void test_colorpicker_manager_click_outside_closes();
void test_colorpicker_manager_select_color();
void test_colorpicker_manager_set_color_fires_callback();
void test_ui_manager_checkbox_integration();
void test_ui_manager_treenode_integration();
void test_ui_manager_splitter_integration();
void test_ui_manager_colorpicker_integration();

// Renderer backend tests
void run_renderer_backend_tests();

// Script VM and ABI capsule tests
void run_script_and_abi_tests();

int main() {
    std::cout << "=== Atlas Engine Tests ===" << std::endl;

    // GraphVM
    std::cout << "\n--- Graph VM ---" << std::endl;
    test_basic_arithmetic();
    test_subtraction();
    test_multiplication();
    test_division();
    test_division_by_zero();
    test_comparison();
    test_conditional_jump();
    test_variables();

    // ECS
    std::cout << "\n--- ECS ---" << std::endl;
    test_create_entity();
    test_destroy_entity();
    test_tick_callback();

    // ECS Components
    std::cout << "\n--- ECS Components ---" << std::endl;
    test_add_and_get_component();
    test_has_component();
    test_remove_component();
    test_multiple_components();
    test_destroy_entity_removes_components();
    test_component_update();

    // Assets
    std::cout << "\n--- Asset System ---" << std::endl;
    test_asset_binary_roundtrip();
    test_asset_registry_scan();

    // Networking
    std::cout << "\n--- Networking ---" << std::endl;
    test_net_init();
    test_net_authority();
    test_net_shutdown();

    // Network Queue
    std::cout << "\n--- Network Queue ---" << std::endl;
    test_net_add_peer();
    test_net_remove_peer();
    test_net_send_receive();
    test_net_broadcast_receive();
    test_net_shutdown_clears_queues();

    // Replication
    std::cout << "\n--- Replication ---" << std::endl;
    test_replication_add_rule();
    test_replication_remove_rule();
    test_replication_get_rule();
    test_replication_replace_rule();
    test_replication_dirty_tracking();
    test_replication_dirty_no_duplicates();
    test_replication_collect_delta();
    test_replication_apply_delta();
    test_replication_multiple_rules();
    test_replication_delta_roundtrip();
    test_replication_delta_every_tick();

    // Asset Browser
    std::cout << "\n--- Asset Browser ---" << std::endl;
    test_asset_browser_empty();
    test_asset_browser_with_assets();
    test_asset_browser_filter();
    test_asset_browser_sort_modes();
    test_asset_browser_select_asset();

    // Asset Cooker
    std::cout << "\n--- Asset Cooker ---" << std::endl;
    test_cooker_default_settings();
    test_cooker_set_output_dir();
    test_cooker_cook_single_asset();
    test_cooker_source_not_found();
    test_cooker_cook_all();
    test_cooker_strip_editor_data();
    test_cooker_clear_log();
    test_cooker_progress_callback();

    // Graph Editor Panel
    std::cout << "\n--- Graph Editor Panel ---" << std::endl;
    test_graph_panel_no_graph();
    test_graph_panel_with_nodes();
    test_graph_panel_select_node();
    test_graph_panel_compile_and_preview();
    test_graph_panel_summary();
    test_graph_panel_node_info();

    // Asset Graph Assistant
    std::cout << "\n--- Asset Graph Assistant ---" << std::endl;
    test_assistant_suggest_empty();
    test_assistant_suggest_with_executor();
    test_assistant_explain_no_executor();
    test_assistant_explain_with_nodes();
    test_assistant_mutate_no_executor();
    test_assistant_mutate_intensity();

    // World
    std::cout << "\n--- World Layout ---" << std::endl;
    test_cube_sphere_projection();
    test_cube_sphere_chunk_roundtrip();
    test_cube_sphere_neighbors();
    test_cube_sphere_lod();
    test_voxel_chunk_roundtrip();
    test_voxel_neighbors();

    // Terrain Mesh
    std::cout << "\n--- Terrain Mesh ---" << std::endl;
    test_terrain_flat_mesh();
    test_terrain_heightmap_mesh();
    test_terrain_normals_computed();
    test_terrain_resolution_one();

    // Render and Platform
    std::cout << "\n--- Render and Platform ---" << std::endl;
    test_render_api_enum();
    test_null_renderer();
    test_gl_renderer_viewport();
    test_vulkan_renderer_viewport();
    test_ui_manager_set_renderer();
    test_ui_manager_render_empty();
    test_ui_manager_render_widgets();
    test_ui_manager_render_null_renderer();
    test_engine_config_render_fields();
    test_engine_headless_no_window();
    test_engine_server_no_window();
    test_platform_window_event_type();
    test_platform_window_config();
    test_platform_has_window_implementation();
    test_engine_no_window_error_without_platform();

    // Viewport Framebuffer
    std::cout << "\n--- Viewport Framebuffer ---" << std::endl;
    test_null_viewport_framebuffer_defaults();
    test_null_viewport_framebuffer_sized();
    test_null_viewport_framebuffer_resize();
    test_null_viewport_framebuffer_bind_unbind();
    test_engine_editor_has_viewport_framebuffer();
    test_engine_client_no_viewport_framebuffer();
    test_engine_server_no_viewport_framebuffer();

    // Phase 13: GLViewportFramebuffer, ScaleLayout, viewport resize
    std::cout << "\n--- Phase 13: GL Viewport, ScaleLayout, Resize ---" << std::endl;
    test_gl_viewport_fb_default_state();
    test_gl_viewport_fb_create_requires_nonzero();
    test_gl_viewport_fb_resize_to_zero();
    test_gl_viewport_fb_bind_unbind_safe();
    test_scale_layout_basic();
    test_scale_layout_identity();
    test_scale_layout_zero_old_noop();
    test_scale_layout_zero_new_noop();
    test_scale_layout_shrink();
    test_ui_manager_viewport_propagates_layout();
    test_ui_manager_viewport_first_set_no_scale();
    test_engine_editor_sets_initial_viewport();

    // Phase 14: Vulkan Device, HttpLLMBackend, FontBootstrap
    register_next_tasks_phase14();

    // Phase 15: MeshViewerPanel and PrefabEditorPanel Draw
    register_next_tasks_phase15();

    // Phase 16: AI assistant LLM wiring + permission enforcement
    register_next_tasks_phase16();

    // Phase 17: Component migration, SimMirror, DesyncVisualizerPanel
    register_next_tasks_phase17();

    // Phase 18: TickStepDebugger, SimulationStateAuditor, FPDriftDetector
    register_next_tasks_phase18();

    // Networking improvements
    register_net_improvements_tests();

    // Phase 19: ProceduralGenerator, AIDebuggerPanel, HotReloadConfig
    register_next_tasks_phase19();

    // Phase 20: DesyncReproducer, QoSScheduler, ServerAssetValidator
    register_next_tasks_phase20();

    // Engine Input Routing (moved before panel draw tests to avoid pre-existing crash)
    std::cout << "\n--- Engine Input Routing ---" << std::endl;
    test_engine_has_event_router();
    test_engine_mouse_tracking_defaults();
    test_engine_event_router_register();
    test_diagnostics_overlay_toggle_state();
    test_f3_key_constant_defined();
    test_window_event_scroll_wheel_type();
    test_window_event_mouse_button_zero_indexed();
    test_scroll_wheel_dispatch_through_ui_manager();

    // Panel Draw() implementations
    register_panel_draw_impl_tests();

    // Noise
    std::cout << "\n--- Noise Generator ---" << std::endl;
    test_perlin_range();
    test_perlin_deterministic();
    test_perlin_seed_variation();
    test_fbm_octaves();
    test_fbm_deterministic();
    test_perlin_spatial_variation();

    // Streaming
    std::cout << "\n--- World Streaming ---" << std::endl;
    test_streamer_request_load();
    test_streamer_set_chunk_data();
    test_streamer_unload_chunk();
    test_streamer_get_loaded_chunks();
    test_streamer_disk_cache();
    test_streamer_duplicate_request();

    // Galaxy
    std::cout << "\n--- Galaxy Generator ---" << std::endl;
    test_galaxy_system_count();
    test_galaxy_deterministic();
    test_galaxy_seed_variation();
    test_galaxy_region_filter();
    test_galaxy_unique_ids();
    test_galaxy_star_classes();

    // Compiler
    std::cout << "\n--- Graph Compiler ---" << std::endl;
    test_compile_constants_and_add();
    test_compile_and_execute_full();
    test_compile_multiply();

    // Engine
    std::cout << "\n--- Engine ---" << std::endl;
    test_engine_init_and_shutdown();
    test_engine_run_loop_ticks();
    test_engine_capabilities();
    test_engine_net_mode_from_config();
    test_engine_frame_pacing_default_enabled();
    test_engine_frame_pacing_prevents_spin();
    test_engine_ui_update_in_loop();

    // Logger
    std::cout << "\n--- Logger ---" << std::endl;
    test_logger_creates_log_directory();
    test_logger_writes_to_file();

    // Console
    std::cout << "\n--- Console ---" << std::endl;
    test_console_spawn_entity();
    test_console_ecs_dump();
    test_console_set_tickrate();
    test_console_net_mode();
    test_console_help();
    test_console_unknown_command();

    // Interaction
    std::cout << "\n--- Interaction System ---" << std::endl;
    test_utterance_creation();
    test_intent_creation();
    test_rule_intent_resolver();
    test_interaction_system_uninitialized();
    test_interaction_system_no_match();
    test_interaction_system_full_pipeline();

    // Interaction (graph-based)
    std::cout << "\n--- Interaction (Graph-Based) ---" << std::endl;
    test_intent_registry_dispatch();
    test_intent_registry_missing();
    test_intent_registry_has_handler();
    test_pattern_resolver();
    test_pattern_resolver_no_match();
    test_pattern_resolver_case_insensitive();

    // Diplomacy
    std::cout << "\n--- AI Diplomacy ---" << std::endl;
    test_faction_request_neutral();
    test_faction_request_trusted();
    test_faction_request_hostile();
    test_faction_threaten_increases_hostility();
    test_faction_unknown_intent();
    test_faction_personality_affects_hostility();

    // Project
    std::cout << "\n--- Project System ---" << std::endl;
    test_project_load_valid();
    test_project_load_minimal();
    test_project_load_missing_file();
    test_project_invalid_schema();
    test_project_missing_name();
    test_project_missing_name_field();
    test_project_invalid_version();
    test_project_schema_validation();

    // Command
    std::cout << "\n--- Command System ---" << std::endl;
    test_command_execute();
    test_command_undo();
    test_command_redo();
    test_command_redo_cleared_on_execute();
    test_command_clear();
    test_command_last_executed();
    test_command_multiple_undo_redo();

    // WorldGraph
    std::cout << "\n--- World Graph ---" << std::endl;
    test_worldgraph_add_nodes();
    test_worldgraph_remove_node();
    test_worldgraph_compile_empty();
    test_worldgraph_compile_single_node();
    test_worldgraph_compile_chain();
    test_worldgraph_deterministic();
    test_worldgraph_clamp_node();
    test_worldgraph_biome_node();
    test_worldgraph_erosion_node();
    test_worldgraph_erosion_deterministic();

    // Voice Commands
    std::cout << "\n--- Voice Commands ---" << std::endl;
    test_voice_register_command();
    test_voice_match_command();
    test_voice_no_match();
    test_voice_get_by_context();

    // Plugin System
    std::cout << "\n--- Plugin System ---" << std::endl;
    test_plugin_validate_valid();
    test_plugin_validate_no_name();
    test_plugin_validate_nondeterministic();
    test_plugin_validate_incompatible_version();
    test_plugin_registry_register();
    test_plugin_registry_find();
    test_plugin_registry_unregister();
    test_plugin_registry_get_by_type();

    // Heightfield
    std::cout << "\n--- Heightfield Mesher ---" << std::endl;
    test_heightfield_at();
    test_heightfield_mesh_generation();
    test_heightfield_mesh_lod();

    // StrategyGraph
    std::cout << "\n--- Strategy Graph ---" << std::endl;
    test_strategygraph_add_nodes();
    test_strategygraph_remove_node();
    test_strategygraph_compile_empty();
    test_strategygraph_compile_single_node();
    test_strategygraph_compile_chain();
    test_strategygraph_execute();
    test_strategygraph_deterministic();
    test_strategygraph_objective_score();
    test_strategygraph_read_ai_signal();
    test_strategygraph_emit_action();

    // Server Rules
    std::cout << "\n--- Server Rules ---" << std::endl;
    test_server_rules_register();
    test_server_rules_get_value();
    test_server_rules_set_multiplier();
    test_server_rules_clamp_multiplier();
    test_server_rules_missing_rule();
    test_server_rules_hot_reload_flag();
    test_server_rules_clear();

    // ConversationGraph
    std::cout << "\n--- Conversation Graph ---" << std::endl;
    test_conversation_add_nodes();
    test_conversation_remove_node();
    test_conversation_compile_empty();
    test_conversation_dialogue_node();
    test_conversation_chain();
    test_conversation_relationship_delta();
    test_conversation_player_choice();

    // AI Signals
    std::cout << "\n--- AI Signals ---" << std::endl;
    test_ai_signal_register();
    test_ai_signal_read_write();
    test_ai_signal_missing();
    test_ai_signal_namespace();
    test_ai_signal_clear();

    // AI Memory
    std::cout << "\n--- AI Memory ---" << std::endl;
    test_ai_memory_store_recall();
    test_ai_memory_has();
    test_ai_memory_forget();
    test_ai_memory_decay();
    test_ai_memory_permanent();
    test_ai_memory_clear();

    // Relationship Model
    std::cout << "\n--- Relationship Model ---" << std::endl;
    test_relationship_set_get();
    test_relationship_default_zero();
    test_relationship_modify();
    test_relationship_has();
    test_relationship_remove();
    test_relationship_clear();

    // Asset Graph
    std::cout << "\n--- Asset Graph ---" << std::endl;
    test_asset_context();
    test_asset_graph_executor_empty();
    test_asset_graph_executor_single_node();
    test_asset_graph_executor_multiple_nodes();
    test_damage_state();

    // Mod System
    std::cout << "\n--- Mod System ---" << std::endl;
    test_mod_register_graph();
    test_mod_register_archetype();
    test_mod_registry_mixed();

    // Editor Assistant
    std::cout << "\n--- Editor Assistant ---" << std::endl;
    test_editor_assistant_explain_performance();
    test_editor_assistant_explain_node();
    test_editor_assistant_unknown();
    test_interaction_debugger_record();
    test_interaction_debugger_clear();

    // Input
    std::cout << "\n--- Input System ---" << std::endl;
    test_input_bind_action();
    test_input_unbind_action();
    test_input_press_release();
    test_input_axis();
    test_input_callback();
    test_input_unbound_action();

    // Camera
    std::cout << "\n--- Camera System ---" << std::endl;
    test_camera_default_state();
    test_camera_set_mode();
    test_camera_position();
    test_camera_fov_clamp();
    test_camera_orbit();
    test_camera_movement();
    test_camera_pitch_clamp();

    // Physics
    std::cout << "\n--- Physics ---" << std::endl;
    test_physics_create_body();
    test_physics_destroy_body();
    test_physics_gravity();
    test_physics_static_body();
    test_physics_apply_force();
    test_physics_collision_detection();

    // Audio
    std::cout << "\n--- Audio System ---" << std::endl;
    test_audio_load_sound();
    test_audio_unload_sound();
    test_audio_play_pause_stop();
    test_audio_volume();
    test_audio_master_volume();
    test_audio_looping();

    // Gameplay Mechanics
    std::cout << "\n--- Gameplay Mechanics ---" << std::endl;
    test_mechanic_register();
    test_mechanic_unregister();
    test_mechanic_find_by_name();
    test_mechanic_params();
    test_mechanic_get_by_type();

    // Skill Tree
    std::cout << "\n--- Skill Tree ---" << std::endl;
    test_skill_add_node();
    test_skill_unlock();
    test_skill_prerequisites();
    test_skill_effects();
    test_skill_tree_chain();

    // Weapon Graph
    std::cout << "\n--- Weapon Graph ---" << std::endl;
    test_weapongraph_add_nodes();
    test_weapongraph_remove_node();
    test_weapongraph_compile_empty();
    test_weapongraph_compile_single_node();
    test_weapongraph_compile_chain();
    test_weapongraph_execute();
    test_weapongraph_wear_degrades_stats();
    test_weapongraph_deterministic();

    // Character Graph
    std::cout << "\n--- Character Graph ---" << std::endl;
    test_charactergraph_add_nodes();
    test_charactergraph_remove_node();
    test_charactergraph_compile_empty();
    test_charactergraph_compile_single_node();
    test_charactergraph_compile_chain();
    test_charactergraph_execute();
    test_charactergraph_deterministic();
    test_charactergraph_facial_features();
    test_charactergraph_hair_style();
    test_charactergraph_facial_deterministic();

    // Animation Graph
    std::cout << "\n--- Animation Graph ---" << std::endl;
    test_animationgraph_add_nodes();
    test_animationgraph_remove_node();
    test_animationgraph_compile_empty();
    test_animationgraph_compile_single_node();
    test_animationgraph_compile_chain();
    test_animationgraph_execute();
    test_animationgraph_modifier();
    test_animationgraph_deterministic();

    // Tile Graph
    std::cout << "\n--- Tile Graph ---" << std::endl;
    test_tilegraph_add_nodes();
    test_tilegraph_remove_node();
    test_tilegraph_compile_empty();
    test_tilegraph_compile_single_node();
    test_tilegraph_compile_chain();
    test_tilegraph_execute();
    test_tilegraph_deterministic();

    // Procedural Mesh Graph
    std::cout << "\n--- Procedural Mesh Graph ---" << std::endl;
    test_procedural_add_nodes();
    test_procedural_remove_node();
    test_procedural_compile_empty();
    test_procedural_cube();
    test_procedural_plane();
    test_procedural_sphere();
    test_procedural_transform();
    test_procedural_merge();
    test_procedural_subdivide();
    test_procedural_noise_determinism();
    test_procedural_full_pipeline();

    // Procedural Material Graph
    std::cout << "\n--- Procedural Material Graph ---" << std::endl;
    test_material_solid_color();
    test_material_checkerboard();
    test_material_noise();
    test_material_blend();
    test_material_normal_map();
    test_material_graph_compile();
    test_material_graph_execute_solid();
    test_material_graph_blend_pipeline();
    test_material_graph_node_count();
    test_material_graph_remove_node();
    test_procedural_texture_generation();
    test_procedural_texture_deterministic();
    test_procedural_texture_non_repeating();

    // LOD Baking Graph
    std::cout << "\n--- LOD Baking Graph ---" << std::endl;
    test_lod_decimate_mesh();
    test_lod_merge_vertices();
    test_lod_recompute_normals();
    test_lod_chain_generation();
    test_lod_graph_add_nodes();
    test_lod_graph_remove_node();
    test_lod_graph_compile_empty();
    test_lod_graph_execute_basic();
    test_lod_graph_decimate_pipeline();
    test_lod_graph_lod_chain_output();

    // UI Logic Graph
    std::cout << "\n--- UI Logic Graph ---" << std::endl;
    test_ui_logic_add_nodes();
    test_ui_logic_remove_node();
    test_ui_logic_compile_empty();
    test_ui_logic_compile_cycle_detection();
    test_ui_logic_condition_node();
    test_ui_logic_action_node();
    test_ui_logic_databind_node();
    test_ui_logic_transition_node();
    test_ui_logic_timer_node();
    test_ui_logic_gate_node_pass();
    test_ui_logic_gate_node_block();

    // Sound Graph
    std::cout << "\n--- Sound Graph ---" << std::endl;
    test_soundgraph_add_nodes();
    test_soundgraph_remove_node();
    test_soundgraph_compile_empty();
    test_soundgraph_compile_single_node();
    test_soundgraph_compile_chain();
    test_soundgraph_execute();
    test_soundgraph_deterministic();

    // Behavior Graph
    std::cout << "\n--- Behavior Graph ---" << std::endl;
    test_behaviorgraph_add_nodes();
    test_behaviorgraph_remove_node();
    test_behaviorgraph_compile_empty();
    test_behaviorgraph_compile_single_node();
    test_behaviorgraph_compile_chain();
    test_behaviorgraph_execute();
    test_behaviorgraph_deterministic();
    test_behaviorgraph_group_tactics();
    test_behaviorgraph_group_tactics_charge();
    test_behaviorgraph_adaptive_behavior();
    test_behaviorgraph_adaptive_difficulty_scaling();

    // UI Screen
    std::cout << "\n--- UI Screen ---" << std::endl;
    test_ui_add_widget();
    test_ui_visibility();
    test_ui_parent_child();
    test_ui_remove_widget();

    // Game Flow Graph
    std::cout << "\n--- Game Flow Graph ---" << std::endl;
    test_gameflowgraph_add_nodes();
    test_gameflowgraph_remove_node();
    test_gameflowgraph_compile_empty();
    test_gameflowgraph_compile_single_node();
    test_gameflowgraph_compile_chain();
    test_gameflowgraph_execute();
    test_gameflowgraph_deterministic();

    // Flow IR
    std::cout << "\n--- Flow IR ---" << std::endl;
    test_flow_ir_default_values();
    test_flow_ir_add_nodes();
    test_flow_ir_add_edges();
    test_flow_ir_to_json();
    test_flow_ir_roundtrip();
    test_flow_ir_validate_valid();
    test_flow_ir_validate_invalid();
    test_flow_ir_migrate_v1();

    // Flow Codegen
    std::cout << "\n--- Flow Codegen ---" << std::endl;
    test_flow_codegen_nonempty();
    test_flow_codegen_function_signature();
    test_flow_codegen_empty_graph();
    test_flow_codegen_node_properties();
    test_flow_codegen_edge_wiring();
    test_flow_codegen_all_node_types();
    test_flow_codegen_header_comment();

    // Flow Debugger
    std::cout << "\n--- Flow Debugger ---" << std::endl;
    test_debugger_initial_state();
    test_debugger_add_breakpoint();
    test_debugger_remove_breakpoint();
    test_debugger_clear_breakpoints();
    test_debugger_execute_no_breakpoints();
    test_debugger_breakpoint_pause();
    test_debugger_step_node();

    // Shader IR
    std::cout << "\n--- Shader IR ---" << std::endl;
    test_shader_module_defaults();
    test_shader_passthrough_vertex();
    test_shader_solid_color_fragment();
    test_shader_validate_valid();
    test_shader_validate_empty();
    test_shader_hash_deterministic();
    test_shader_hash_different();
    test_shader_serialize_roundtrip();

    // Build Manifest
    std::cout << "\n--- Build Manifest ---" << std::endl;
    test_manifest_defaults();
    test_manifest_add_artifact();
    test_manifest_find_artifact();
    test_manifest_find_missing();
    test_manifest_validate_valid();
    test_manifest_validate_missing_fields();
    test_manifest_to_json();
    test_manifest_roundtrip();
    test_manifest_verify_hash();

    // UI Graph
    std::cout << "\n--- UI Graph ---" << std::endl;
    test_uigraph_add_nodes();
    test_uigraph_remove_node();
    test_uigraph_compile_empty();
    test_uigraph_compile_single_node();
    test_uigraph_compile_chain();
    test_uigraph_execute();
    test_uigraph_deterministic();

    // UI Command Bus
    std::cout << "\n--- UI Command Bus ---" << std::endl;
    test_command_bus_enqueue();
    test_command_bus_drain();
    test_command_bus_clear();
    test_command_bus_dispatch();
    test_command_bus_dispatch_unhandled();
    test_command_bus_move_enqueue();

    // Editor Command Bus
    std::cout << "\n--- Editor Command Bus ---" << std::endl;
    test_editor_command_bus_post_and_process();
    test_editor_command_bus_undo();
    test_editor_command_bus_redo();
    test_editor_command_bus_redo_cleared_on_new_command();
    test_editor_command_bus_multiple_undo_redo();
    test_editor_command_bus_undo_empty();
    test_editor_command_bus_clear();
    test_editor_command_bus_listener();
    test_editor_command_bus_null_command();

    // UI Manager
    std::cout << "\n--- UI Manager ---" << std::endl;
    test_ui_manager_init();
    test_ui_manager_context_switch();
    test_ui_manager_screen_access();
    test_ui_manager_graph_access();
    test_ui_manager_update();
    test_ui_manager_command_bus();
    test_ui_manager_shutdown();
    test_ui_manager_server_context();

    // UI Layout Solver
    std::cout << "\n--- UI Layout Solver ---" << std::endl;
    test_layout_solver_single_entry();
    test_layout_solver_horizontal_split();
    test_layout_solver_vertical_split();
    test_layout_solver_weighted();
    test_layout_solver_min_size_respect();
    test_layout_solver_clear();
    test_layout_solver_deterministic();
    test_layout_solver_offset();

    // UI Nodes Extended
    std::cout << "\n--- UI Nodes Extended ---" << std::endl;
    test_slotgrid_node_defaults();
    test_inputfield_node_defaults();
    test_inputfield_node_with_text();
    test_slotgrid_node_metadata();
    test_inputfield_node_metadata();

    // Schema Validator
    std::cout << "\n--- Schema Validator ---" << std::endl;
    test_schema_valid();
    test_schema_empty_id();
    test_schema_zero_version();
    test_schema_duplicate_node_id();
    test_schema_empty_node_id();
    test_schema_no_nodes();

    // Graph Cache
    std::cout << "\n--- Graph Cache ---" << std::endl;
    test_cache_store_and_get();
    test_cache_has();
    test_cache_invalidate();
    test_cache_invalidate_all();
    test_cache_evict_before();
    test_cache_hash_key();

    // Graph Serialization
    std::cout << "\n--- Graph Serialization ---" << std::endl;
    test_json_builder_object();
    test_json_builder_array();
    test_json_reader_parse();
    test_json_reader_array();
    test_json_roundtrip();

    // ECS Serialization
    std::cout << "\n--- ECS Serialization ---" << std::endl;
    test_ecs_serialize_empty_world();
    test_ecs_serialize_entities_no_components();
    test_ecs_serialize_roundtrip();
    test_ecs_serialize_preserves_next_id();
    test_ecs_deserialize_clears_existing();
    test_ecs_deserialize_invalid_data();
    test_ecs_has_serializer();

    // Snapshot / Rollback
    std::cout << "\n--- Snapshot / Rollback ---" << std::endl;
    test_snapshot_saves_ecs_state();
    test_rollback_restores_ecs_state();
    test_rollback_removes_future_snapshots();
    test_snapshot_without_world();
    test_rollback_with_multiple_entities();
    test_record_and_replay_input();
    test_replay_applies_input_frames();

    // ECS Inspector
    std::cout << "\n--- ECS Inspector ---" << std::endl;
    test_inspector_empty_world();
    test_inspector_entities_with_components();
    test_inspector_select_entity();
    test_inspector_refreshes_on_draw();
    test_inspector_state_blocks_empty();
    test_inspector_state_blocks_with_world_state();

    // Story
    std::cout << "\n--- Story System ---" << std::endl;
    test_story_add_node();
    test_story_connections();
    test_story_tags();
    test_story_get_by_type();

    // Build Profiles
    std::cout << "\n--- Build Profiles ---" << std::endl;
    test_profile_default_debug();
    test_profile_default_development();
    test_profile_default_release();
    test_profile_name();
    test_profile_set_active();
    test_profile_set_custom_config();
    test_profile_overrides();
    test_profile_override_missing_key();

    // Mod Loader
    std::cout << "\n--- Mod Loader ---" << std::endl;
    test_mod_loader_register();
    test_mod_loader_register_invalid();
    test_mod_loader_register_duplicate();
    test_mod_loader_unregister();
    test_mod_loader_enable_disable();
    test_mod_loader_get_mod();
    test_mod_loader_dependencies();
    test_mod_loader_scan_directory();
    test_mod_loader_clear();

    // Platform Target
    std::cout << "\n--- Platform Target ---" << std::endl;
    test_platform_default_windows();
    test_platform_default_linux();
    test_platform_default_macos();
    test_platform_default_web();
    test_platform_name();
    test_platform_set_target();
    test_platform_custom_config();
    test_platform_module_exclusion();
    test_platform_host();
    test_platform_supported_list();

    // Profiler Panel
    std::cout << "\n--- Profiler Panel ---" << std::endl;
    test_profiler_empty();
    test_profiler_record_frame();
    test_profiler_average_and_peak();
    test_profiler_max_history();
    test_profiler_system_metrics();
    test_profiler_pause();
    test_profiler_clear();

    // Replay Recorder
    std::cout << "\n--- Replay Recorder ---" << std::endl;
    test_replay_initial_state();
    test_replay_start_recording();
    test_replay_record_frames();
    test_replay_stop_recording();
    test_replay_save_and_load();
    test_replay_load_invalid();
    test_replay_clear();

    // State Hasher
    std::cout << "\n--- State Hasher ---" << std::endl;
    test_hasher_initial_state();
    test_hasher_advance_tick();
    test_hasher_deterministic();
    test_hasher_chaining();
    test_hasher_divergence_detection();
    test_hasher_different_seeds();
    test_hasher_empty_data();
    test_hasher_raw_pointer_api();
    test_replay_record_with_hash();
    test_replay_save_load_with_hash();
    test_replay_default_hash_zero();
    test_hash_combine_deterministic();

    // Visual Diff
    std::cout << "\n--- Visual Diff ---" << std::endl;
    test_diff_identical();
    test_diff_added();
    test_diff_removed();
    test_diff_modified();
    test_diff_empty();
    test_diff_compare_graphs();
    test_diff_summarize();

    // Crash Handler
    std::cout << "\n--- Crash Handler ---" << std::endl;
    test_crash_breadcrumb();
    test_crash_breadcrumb_limit();
    test_crash_clear_breadcrumbs();
    test_crash_generate_report();
    test_crash_save_report();
    test_crash_callback();
    test_crash_report_to_file();

    // Net Inspector Panel
    std::cout << "\n--- Net Inspector Panel ---" << std::endl;
    test_net_inspector_standalone();
    test_net_inspector_server_with_peers();
    test_net_inspector_mode_names();

    // Game Packager Panel
    std::cout << "\n--- Game Packager Panel ---" << std::endl;
    test_game_packager_defaults();
    test_game_packager_summary();
    test_game_packager_modify_settings();
    test_package_config_strip_tools_default();

    // Game Module System
    std::cout << "\n--- Game Module System ---" << std::endl;
    test_game_module_describe();
    test_game_module_lifecycle();
    test_module_loader_static();
    test_module_loader_already_loaded();
    test_module_loader_not_found();

    // Atlas Gameplay
    std::cout << "\n--- Atlas Gameplay ---" << std::endl;
    test_faction_register();
    test_faction_relations();
    test_faction_clear();
    test_combat_register_unit();
    test_combat_resolve();
    test_combat_clear();
    test_economy_register();
    test_economy_transactions();
    test_economy_capacity();
    test_economy_clear();

    // EveOffline Module
    std::cout << "\n--- EveOffline Module ---" << std::endl;
    test_eveoffline_describe();
    test_eveoffline_register_types();
    test_eveoffline_replication_rules();
    test_eveoffline_server_rules();
    test_eveoffline_lifecycle();
    test_eveoffline_factory();

    // Arena2D Module
    std::cout << "\n--- Arena2D Module ---" << std::endl;
    test_arena2d_describe();
    test_arena2d_register_types();
    test_arena2d_replication_rules();
    test_arena2d_server_rules();
    test_arena2d_lifecycle();
    test_arena2d_factory();

    // Replay Timeline Panel
    std::cout << "\n--- Replay Timeline Panel ---" << std::endl;
    test_replay_timeline_empty();
    test_replay_timeline_load();
    test_replay_timeline_scrub();
    test_replay_timeline_markers();
    test_replay_timeline_compare_identical();
    test_replay_timeline_compare_divergent();
    test_replay_timeline_inject_input();
    test_replay_timeline_branch();

    // Replay Divergence Inspector
    std::cout << "\n--- Replay Divergence Inspector ---" << std::endl;
    test_divergence_no_mismatch();
    test_divergence_hash_mismatch();
    test_divergence_frame_comparison();
    test_divergence_single_frame_check();
    test_divergence_report_format();
    test_divergence_history();
    test_divergence_severity();

    // Deterministic Allocator
    std::cout << "\n--- Deterministic Allocator ---" << std::endl;
    test_allocator_create();
    test_allocator_single_alloc();
    test_allocator_multiple_allocs();
    test_allocator_dealloc();
    test_allocator_reuse_space();
    test_allocator_stable_ids();
    test_allocator_reset();
    test_allocator_deterministic();

    // Permission Manager
    std::cout << "\n--- Permission Manager ---" << std::endl;
    test_permission_defaults();
    test_permission_register_user();
    test_permission_tier_viewonly();
    test_permission_tier_developer();
    test_permission_tier_admin();
    test_permission_grant_custom();
    test_permission_revoke();
    test_permission_user_management();

    // HUD Overlay
    std::cout << "\n--- HUD Overlay ---" << std::endl;
    test_hud_defaults();
    test_hud_tick_display();
    test_hud_warnings();
    test_hud_critical_warning();
    test_hud_time_controls();
    test_hud_step_forward();
    test_hud_determinism_status();
    test_hud_visibility();
    test_hud_world_dilation();
    test_hud_world_paused();

    // Time Model
    std::cout << "\n--- Time Model ---" << std::endl;
    test_time_model_defaults();
    test_time_model_set_tick_rate();
    test_time_model_advance_tick();
    test_time_model_world_dilation();
    test_time_model_world_pause();
    test_time_model_presentation();
    test_time_model_reset();
    test_simulation_time_elapsed();

    // World State
    std::cout << "\n--- World State ---" << std::endl;
    test_world_state_register_block();
    test_world_state_find_block();
    test_world_state_snapshot();
    test_world_state_snapshot_deterministic();
    test_world_state_push_and_retrieve();
    test_world_state_max_snapshots();
    test_world_state_prune();
    test_world_state_clear();
    test_world_state_derived_rebuild();

    // Save System
    std::cout << "\n--- Save System ---" << std::endl;
    test_save_system_save_and_load();
    test_save_system_hash_integrity();
    test_save_system_file_not_found();
    test_save_system_validate();
    test_save_system_invalid_format();
    test_save_system_empty_data();
    test_save_system_clear();

    // Engine Integration
    std::cout << "\n--- Engine Integration ---" << std::endl;
    test_engine_time_model_initialized();
    test_engine_time_model_tick_rate_matches();
    test_engine_time_model_world_time_advances();
    test_engine_world_state_snapshots_in_server();
    test_engine_world_state_snapshot_at_tick();
    test_engine_save_system_accessible();
    test_console_save_command();
    test_console_load_command();
    test_console_load_file_not_found();
    test_console_save_no_path();
    test_console_time_info();
    test_ai_behavior_deterministic_across_runs();
    test_ai_relationship_determinism_across_save_load();

    // Next Tasks
    std::cout << "\n--- Next Tasks ---" << std::endl;
    test_engine_autosave_creates_file();
    test_time_model_set_tick();
    test_replay_save_point_mark();
    test_replay_save_point_multiple();
    test_replay_save_point_serialization();
    test_replay_start_from_save();
    test_engine_load_and_replay();
    test_divergence_export_report();
    test_engine_rollback_to_tick();
    test_net_broadcast_save_tick();
    test_state_hash_diff_panel_no_divergence();
    test_state_hash_diff_panel_with_divergence();
    test_hash_ladder_save_load_continuity();
    test_engine_replay_from_save();
    test_engine_replay_from_save_bad_save();
    test_engine_replay_from_save_bad_replay();
    test_engine_replay_from_save_no_frames_after_save_point();
    test_state_hash_diff_panel_component_breakdown();
    test_state_hash_diff_panel_no_component_divergence();

    // Next Tasks Phase 2
    std::cout << "\n--- Next Tasks Phase 2 ---" << std::endl;
    test_behaviorgraph_serialize_state();
    test_behaviorgraph_serialize_empty();
    test_partial_save_and_load();
    test_partial_save_hash_integrity();
    test_partial_save_empty_chunks();
    test_save_file_browser_scan();
    test_save_file_browser_empty_dir();
    test_save_file_browser_select();
    test_hash_ladder_comparison();
    test_hash_ladder_comparison_no_divergence();
    test_divergence_detail();
    test_divergence_detail_no_divergence();
    test_replay_auto_divergence_callback();
    test_replay_no_divergence_callback();
    test_detailed_divergence_report();
    test_detailed_divergence_report_no_divergence();

    // GUI DSL Parser
    std::cout << "\n--- GUI DSL Parser ---" << std::endl;
    test_dsl_parse_simple_layout();
    test_dsl_parse_split();
    test_dsl_parse_nested_splits();
    test_dsl_parse_dock();
    test_dsl_parse_tabs();
    test_dsl_parse_full_example();
    test_dsl_parse_comments();
    test_dsl_parse_error_missing_layout();
    test_dsl_parse_error_unterminated_string();
    test_dsl_parse_error_unknown_statement();
    test_dsl_version();
    test_dsl_parse_dock_targets();

    // GUI Input Recorder
    std::cout << "\n--- GUI Input Recorder ---" << std::endl;
    test_recorder_initial_state();
    test_recorder_start_stop_recording();
    test_recorder_record_events();
    test_recorder_record_only_when_recording();
    test_recorder_playback();
    test_recorder_save_load();
    test_recorder_load_invalid_file();
    test_recorder_load_nonexistent_file();

    // Headless GUI
    std::cout << "\n--- Headless GUI ---" << std::endl;
    test_headless_gui_init();
    test_headless_gui_available_commands();
    test_headless_gui_widget_count();
    test_headless_gui_widget_info();
    test_headless_gui_widget_info_not_found();
    test_headless_gui_unknown_command();
    test_headless_gui_empty_command();
    test_headless_gui_help();
    test_headless_gui_status();
    test_headless_gui_enqueue();
    test_headless_gui_custom_command();
    test_headless_gui_quoted_args();
    test_headless_gui_diag_toggle();
    test_headless_gui_diag_show_hide();
    test_headless_gui_diag_status();

    // Job Execution Tracer
    std::cout << "\n--- Job Execution Tracer ---" << std::endl;
    test_job_tracer_empty();
    test_job_tracer_single_tick();
    test_job_tracer_multiple_ticks();
    test_job_tracer_deterministic_hash();
    test_job_tracer_different_order_different_hash();
    test_job_tracer_compare_order();
    test_job_tracer_max_history();
    test_job_tracer_clear();
    test_job_trace_panel_no_tracer();
    test_job_trace_panel_consistent();
    test_job_trace_panel_mismatch();
    test_job_trace_panel_entries_at_tick();

    // Component Category
    std::cout << "\n--- Component Category ---" << std::endl;
    test_component_category_defaults();
    test_component_category_simulated();
    test_component_category_presentation();
    test_component_category_debug();
    test_component_category_derived();
    test_component_category_assert_simulated();
    test_component_category_assert_not_presentation();
    test_component_category_runtime_query();

    // Include Firewall
    std::cout << "\n--- Include Firewall ---" << std::endl;
    test_include_firewall_sim_no_render();
    test_include_firewall_contract_no_render();

    // ImGui Ban (ATLAS_CORE_CONTRACT.md §6)
    std::cout << "\n--- ImGui Ban Enforcement ---" << std::endl;
    test_no_imgui_in_engine();
    test_no_imgui_in_editor();
    test_no_imgui_in_client();
    test_no_imgui_in_server();

    // FP Drift Detector
    std::cout << "\n--- FP Drift Detector ---" << std::endl;
    test_fp_drift_no_drift();
    test_fp_drift_detected();
    test_fp_drift_system_comparison();
    test_fp_drift_format_report();
    test_fp_drift_export_report();
    test_fp_drift_add_system_hashes();

    // Asset Import Pipeline
    std::cout << "\n--- Asset Import Pipeline ---" << std::endl;
    test_asset_import_registry_empty();
    test_asset_import_registry_register();
    test_asset_import_mesh_can_import();
    test_asset_import_texture_can_import();
    test_asset_import_audio_can_import();
    test_asset_import_font_can_import();
    test_asset_import_sprite_can_import();
    test_asset_import_find_importer();
    test_asset_import_file_roundtrip();
    test_asset_import_file_not_found();
    test_asset_import_registry_import();

    // Marketplace Importer
    std::cout << "\n--- Marketplace Importer ---" << std::endl;
    test_marketplace_registry();
    test_itch_io_importer();
    test_marketplace_metadata();
    test_marketplace_import_options();
    test_unreal_marketplace_importer();
    test_unity_assetstore_importer();

    // Asset Validator
    std::cout << "\n--- Asset Validator ---" << std::endl;
    test_asset_validator_validate_valid_file();
    test_asset_validator_validate_corrupt_file();
    test_asset_validator_validate_header_bad_magic();
    test_asset_validator_migration_path();
    test_asset_validator_dependencies();
    test_asset_validator_circular_dependency();
    test_asset_validator_no_circular_dependency();
    test_asset_validator_compute_file_hash();

    // Golden Replays
    std::cout << "\n--- Golden Replays ---" << std::endl;
    run_golden_replay_tests();

    // Enforcement
    std::cout << "\n--- Enforcement ---" << std::endl;
    test_tick_rate_lock();
    test_tick_rate_set_before_lock();
    test_engine_locks_tick_rate_on_run();
    test_engine_system_order();
    test_engine_system_order_deterministic();
    test_net_packet_schema_version();
    test_asset_lock_and_check();
    test_asset_lock_multiple();
    test_determinism_config_exists();

    // Lockdown Phase 2
    std::cout << "\n--- Lockdown Phase 2 ---" << std::endl;
    test_rollback_and_verify_success();
    test_rollback_and_verify_bad_ticks();
    test_asset_schema_lock();
    test_asset_schema_lock_default();
    test_layout_hash_deterministic();
    test_layout_hash_differs_on_change();
    test_layout_hash_empty();
    test_mutation_ownership_register();
    test_mutation_ownership_exclusive();
    test_mutation_can_mutate();
    test_mutation_owned_components();
    test_mutation_owner_of_unregistered();

    // Lockdown Phase 3
    std::cout << "\n--- Lockdown Phase 3 ---" << std::endl;
    test_auditor_empty_passes();
    test_auditor_system_no_state_fails();
    test_auditor_system_with_state_passes();
    test_auditor_multiple_systems();
    test_auditor_duplicate_register();
    test_auditor_declared_blocks();
    test_auditor_unregistered_declaration_warning();
    test_auditor_clear();
    test_server_validator_empty_manifest();
    test_server_validator_register_and_check();
    test_server_validator_hash_match();
    test_server_validator_hash_mismatch();
    test_server_validator_unknown_asset();
    test_server_validator_file_validation();
    test_server_validator_file_not_found();
    test_server_validator_manifest();
    test_verify_save_load_determinism();
    test_desync_reproducer_output_dir();
    test_desync_reproducer_capture();
    test_desync_reproducer_generate_command();
    test_desync_reproducer_multiple_captures();

    // GUI Completion
    std::cout << "\n--- GUI Completion ---" << std::endl;
    test_editor_theme_dark_defaults();
    test_editor_theme_custom_values();
    test_panel_closable_by_default();
    test_panel_set_non_closable();
    test_layout_close_closable_panel();
    test_layout_close_non_closable_panel();
    test_layout_close_unknown_panel();
    test_layout_find_panel();
    test_main_panels_non_closable();
    test_attach_init();
    test_attach_standalone();
    test_attach_live_client();
    test_attach_live_client_missing_host();
    test_attach_headless_server();
    test_attach_replay();
    test_attach_replay_missing_path();
    test_attach_disconnect();
    test_default_editor_dsl_parses();
    test_default_editor_dsl_has_panels();

    // Proof Viewer Panel
    std::cout << "\n--- Proof Viewer Panel ---" << std::endl;
    test_proof_viewer_init();
    test_proof_viewer_load_specs();
    test_proof_viewer_select_spec();
    test_proof_viewer_tokenize();
    test_proof_viewer_invariants();
    test_proof_viewer_verification_status();
    test_proof_viewer_line_count();
    test_proof_viewer_empty_select();

    // Replay Proof Exporter
    std::cout << "\n--- Replay Proof Exporter ---" << std::endl;
    test_proof_exporter_empty();
    test_proof_exporter_add_entries();
    test_proof_exporter_validate_chain_valid();
    test_proof_exporter_validate_chain_invalid();
    test_proof_exporter_export_tla();
    test_proof_exporter_export_json();
    test_proof_exporter_export_csv();
    test_proof_exporter_save_points();
    test_proof_exporter_format_entry();
    test_proof_exporter_clear();

    // CI Dashboard Panel
    std::cout << "\n--- CI Dashboard Panel ---" << std::endl;
    test_ci_dashboard_init();
    test_ci_dashboard_start_run();
    test_ci_dashboard_add_checks();
    test_ci_dashboard_complete_run_passed();
    test_ci_dashboard_complete_run_failed();
    test_ci_dashboard_partial_failure();
    test_ci_dashboard_pass_rate();
    test_ci_dashboard_history();
    test_ci_dashboard_max_history();
    test_ci_dashboard_summary();
    test_ci_dashboard_clear();

    // TLC Model Checker
    std::cout << "\n--- TLC Model Checker ---" << std::endl;
    test_tlc_register_spec();
    test_tlc_register_multiple_specs();
    test_tlc_register_bad_path();
    test_tlc_unregister_spec();
    test_tlc_get_spec();
    test_tlc_extract_invariants_replay();
    test_tlc_extract_invariants_ecs();
    test_tlc_extract_invariants_layout();
    test_tlc_validate_spec();
    test_tlc_check_spec();
    test_tlc_check_spec_not_found();
    test_tlc_check_all();
    test_tlc_ci_check_pass();
    test_tlc_format_report();
    test_tlc_export_report();
    test_tlc_path_management();
    test_tlc_extract_invariants_static();

    // Deterministic Animation Graph
    std::cout << "\n--- Deterministic Animation Graph ---" << std::endl;
    test_det_anim_add_nodes();
    test_det_anim_remove_node();
    test_det_anim_compile_empty();
    test_det_anim_rest_pose();
    test_det_anim_fk();
    test_det_anim_ik();
    test_det_anim_blend_tree();
    test_det_anim_bone_mask();
    test_det_anim_additive_blend();
    test_det_anim_deterministic();

    // Collaborative Editor
    std::cout << "\n--- Collaborative Editor ---" << std::endl;
    test_collab_add_peer();
    test_collab_remove_peer();
    test_collab_cursor_update();
    test_collab_submit_operation();
    test_collab_receive_remote();
    test_collab_detect_conflicts();
    test_collab_resolve_last_writer_wins();
    test_collab_resolve_first_writer_wins();
    test_collab_no_conflicts();
    test_collab_clear();

    // Next Tasks Phase 12
    run_next_tasks_phase12_tests();

    // AtlasAI Core
    std::cout << "\n--- AtlasAI Core ---" << std::endl;
    test_ai_core_default_permissions();
    test_ai_core_set_permissions();
    test_ai_core_has_permission();
    test_ai_core_register_intent();
    test_ai_core_unregister_intent();
    test_ai_core_list_intents();
    test_ai_core_process_request();
    test_ai_core_permission_denied();
    test_ai_core_unknown_intent();
    test_ai_core_history();
    test_ai_core_clear();

    // AI Asset Decision Framework
    run_ai_decision_framework_tests();

    // Atlas Assistant Panel
    std::cout << "\n--- Atlas Assistant Panel ---" << std::endl;
    test_assistant_panel_name();
    test_assistant_panel_context();
    test_assistant_panel_submit_prompt();
    test_assistant_panel_suggestions();
    test_assistant_panel_apply_suggestion();
    test_assistant_panel_diff_preview();
    test_assistant_panel_conversation();
    test_assistant_panel_clear();

    // AI Diff Viewer Panel
    std::cout << "\n--- AI Diff Viewer Panel ---" << std::endl;
    test_diff_viewer_name();
    test_diff_viewer_no_diff();
    test_diff_viewer_load();
    test_diff_viewer_accept_hunk();
    test_diff_viewer_reject_hunk();
    test_diff_viewer_accept_all();
    test_diff_viewer_reject_all();
    test_diff_viewer_apply();
    test_diff_viewer_history();
    test_diff_viewer_clear();

    // Flow Graph Refactorer
    std::cout << "\n--- Flow Graph Refactorer ---" << std::endl;
    test_flow_refactorer_load();
    test_flow_refactorer_find_dead_nodes();
    test_flow_refactorer_find_chains();
    test_flow_refactorer_find_duplicates();
    test_flow_refactorer_analyze();
    test_flow_refactorer_remove_dead();
    test_flow_refactorer_rename();
    test_flow_refactorer_simplify_chain();
    test_flow_refactorer_history();
    test_flow_refactorer_clear();

    // Web Aggregation KB
    std::cout << "\n--- Web Aggregation KB ---" << std::endl;
    test_kb_add_entry();
    test_kb_get_entry();
    test_kb_remove_entry();
    test_kb_search();
    test_kb_search_by_category();
    test_kb_search_by_tag();
    test_kb_categories();
    test_kb_tags();
    test_kb_export_import();
    test_kb_clear();

    // Game GUI Asset
    std::cout << "\n--- Game GUI Asset ---" << std::endl;
    test_gui_asset_create_widget();
    test_gui_asset_add_child();
    test_gui_asset_find_widget();
    test_gui_asset_remove_widget();
    test_gui_asset_list_by_type();
    test_gui_asset_properties();
    test_gui_asset_bindings();
    test_gui_asset_validate();
    test_gui_asset_export_json();
    test_gui_asset_clear();

    // Widget DSL
    std::cout << "\n--- Widget DSL ---" << std::endl;
    test_widget_dsl_tokenize();
    test_widget_dsl_parse_simple();
    test_widget_dsl_parse_nested();
    test_widget_dsl_parse_properties();
    test_widget_dsl_validate_valid();
    test_widget_dsl_validate_invalid();
    test_widget_dsl_empty();
    test_widget_dsl_multiple_roots();

    // Game GUI Binding
    std::cout << "\n--- Game GUI Binding ---" << std::endl;
    test_binding_add();
    test_binding_remove();
    test_binding_get();
    test_binding_list();
    test_binding_evaluate_direct();
    test_binding_evaluate_formatted();
    test_binding_evaluate_computed();
    test_binding_custom_formatter();
    test_binding_validate();
    test_binding_clear();

    // Game Mechanics UI Panel
    std::cout << "\n--- Game Mechanics UI Panel ---" << std::endl;
    test_mechanics_ui_name();
    test_mechanics_ui_add_element();
    test_mechanics_ui_remove_element();
    test_mechanics_ui_get_element();
    test_mechanics_ui_list_elements();
    test_mechanics_ui_select_element();
    test_mechanics_ui_preview_mode();
    test_mechanics_ui_export();
    test_mechanics_ui_import();
    test_mechanics_ui_clear();

    // Determinism Versioning
    std::cout << "\n--- Determinism Versioning ---" << std::endl;
    test_dv_set_version();
    test_dv_register_fork();
    test_dv_unregister_fork();
    test_dv_get_fork();
    test_dv_list_forks();
    test_dv_compatible();
    test_dv_incompatible();
    test_dv_check_all();
    test_dv_report();
    test_dv_clear();

    // Certified Build
    std::cout << "\n--- Certified Build ---" << std::endl;
    test_cert_default_level();
    test_cert_set_level();
    test_cert_add_artifact();
    test_cert_get_artifact();
    test_cert_list_artifacts();
    test_cert_verify_all_pass();
    test_cert_verify_with_failure();
    test_cert_generate_report();
    test_cert_export_json();
    test_cert_clear();

    // Certified Build Verification Steps (Phase 3)
    std::cout << "\n--- Certified Build Steps ---" << std::endl;
    test_cert_register_step();
    test_cert_run_all_steps();
    test_cert_run_all_steps_with_failure();
    test_cert_verify_file_hash();
    test_cert_verify_file_hash_not_found();
    test_cert_verify_test_results();
    test_cert_verify_contract_scan();
    test_cert_clear_clears_steps();

    // Font Bootstrap
    std::cout << "\n--- Font Bootstrap ---" << std::endl;
    test_font_bootstrap_initial_state();
    test_font_bootstrap_init_missing_font_generates_fallback();
    test_font_bootstrap_init_unwritable_root();
    test_font_bootstrap_shutdown();
    test_font_bootstrap_rebuild_noop();

    // Diagnostics Overlay
    std::cout << "\n--- Diagnostics Overlay ---" << std::endl;
    test_diagnostics_overlay_default_off();
    test_diagnostics_overlay_toggle();
    test_diagnostics_overlay_set_enabled();
    test_diagnostics_overlay_render_null_renderer();
    test_diagnostics_overlay_render_when_disabled();
    test_diagnostics_overlay_render_when_enabled();

    // UI Event Router
    std::cout << "\n--- UI Event Router ---" << std::endl;
    test_event_router_empty();
    test_event_router_register_dispatch();
    test_event_router_miss();
    test_event_router_z_order();
    test_event_router_unregister();
    test_event_router_hover_tracking();
    test_event_router_clear();

    // UI Draw List
    std::cout << "\n--- UI Draw List ---" << std::endl;
    test_draw_list_empty();
    test_draw_list_draw_rect();
    test_draw_list_draw_text();
    test_draw_list_draw_icon();
    test_draw_list_draw_border();
    test_draw_list_draw_image();
    test_draw_list_multiple_commands();
    test_draw_list_clear();
    test_draw_list_flush();
    test_draw_list_flush_null_renderer();

    // Engine Phase
    std::cout << "\n--- Engine Phase ---" << std::endl;
    test_engine_phase_to_string();
    test_engine_phase_values();

    // Launcher Screen
    std::cout << "\n--- Launcher Screen ---" << std::endl;
    test_launcher_initial_state();
    test_launcher_scan_nonexistent();
    test_launcher_scan_atlas_descriptors();
    test_launcher_select_invalid();
    test_launcher_confirm_no_selection();
    test_launcher_request_new_project();
    test_launcher_request_quit();

    // Launcher Descriptor Parsing (Phase 3)
    std::cout << "\n--- Launcher Descriptor Parsing ---" << std::endl;
    test_launcher_parses_project_name_from_descriptor();
    test_launcher_parses_last_opened();
    test_launcher_fallback_on_minimal_descriptor();
    test_launcher_project_description_field();

    // DSL Layout (Phase 3)
    std::cout << "\n--- DSL Layout ---" << std::endl;
    test_dsl_parses_default_editor_layout();
    test_dsl_layout_has_panels();
    test_dsl_layout_has_dock_and_split();

    // Tile Editor
    std::cout << "\n--- Tile Editor ---" << std::endl;
    test_tile_editor_name();
    test_tile_editor_register();
    test_tile_editor_modes();
    test_tile_editor_paint();
    test_tile_editor_erase();
    test_tile_editor_paint_no_tile_selected();
    test_tile_editor_active_layer();
    test_tile_editor_unregister();
    test_tile_editor_grid_coord_hash();
    test_tile_editor_brush_paint_single();
    test_tile_editor_brush_paint_square_3x3();
    test_tile_editor_brush_paint_circle();
    test_tile_editor_brush_erase();
    test_tile_editor_paint_rect();
    test_tile_editor_paint_rect_reversed_corners();
    test_tile_editor_erase_rect();
    test_tile_editor_flood_fill_empty_region();
    test_tile_editor_flood_fill_replace_tile();
    test_tile_editor_flood_fill_same_tile_noop();
    test_tile_editor_on_tile_painted_callback();
    test_tile_editor_on_tile_erased_callback();
    test_tile_editor_brush_paint_fires_callback();
    test_tile_editor_rect_paint_fires_callback();
    test_tile_editor_get_brush_shape();
    test_tile_editor_get_brush_size();

    // Text Renderer
    std::cout << "\n--- Text Renderer ---" << std::endl;
    test_null_text_renderer();
    test_glyph_default();
    test_font_atlas_default();

    // Game Packager Build Pipeline
    std::cout << "\n--- Game Packager Build Pipeline ---" << std::endl;
    test_game_packager_validate_empty_source();
    test_game_packager_validate_nonexistent_source();
    test_game_packager_validate_valid();
    test_game_packager_package_empty_dir();
    test_game_packager_package_with_assets();
    test_game_packager_progress_callback();
    test_game_packager_last_report();
    test_game_packager_validation_failed();

    // Layout Persistence
    std::cout << "\n--- Layout Persistence ---" << std::endl;
    test_layout_persistence_serialize_empty();
    test_layout_persistence_roundtrip();
    test_layout_persistence_save_load_file();
    test_layout_persistence_load_nonexistent();
    test_layout_persistence_panel_closable();
    test_layout_persistence_vertical_split();

    // Net Hardening
    std::cout << "\n--- Net Hardening ---" << std::endl;
    test_net_hardening_default_config();
    test_net_hardening_configure();
    test_net_hardening_connect_disconnect();
    test_net_hardening_connection_timeout();
    test_net_hardening_reconnect();
    test_net_hardening_packet_received_connects();
    test_net_hardening_bandwidth_limit();
    test_net_hardening_bandwidth_unlimited();
    test_net_hardening_packet_size_validation();
    test_net_hardening_rtt_tracking();
    test_net_hardening_heartbeat_timeout();
    test_net_hardening_heartbeat_reset();
    test_net_hardening_stats_tracking();
    test_net_hardening_reset_stats();
    test_net_hardening_state_callback();

    // Play-In-Editor
    std::cout << "\n--- Play-In-Editor ---" << std::endl;
    test_pie_initial_state();
    test_pie_start_stop();
    test_pie_pause_resume();
    test_pie_step_tick();
    test_pie_state_restore();
    test_pie_possess_entity();
    test_pie_possess_zero_fails();
    test_pie_loopback();
    test_pie_mode_callback();
    test_pie_double_start_fails();
    test_pie_stop_when_stopped();
    test_pie_auto_possess();

    // World State Serializer
    std::cout << "\n--- World State Serializer ---" << std::endl;
    test_serializer_default_version();
    test_serializer_set_version();
    test_serializer_serialize_roundtrip();
    test_serializer_serialize_empty();
    test_serializer_hash_mismatch();
    test_serializer_version_too_old();
    test_serializer_version_too_new();
    test_serializer_migration();
    test_serializer_can_migrate();
    test_serializer_migration_path();
    test_serializer_migration_failure();
    test_serializer_schema_version_comparison();

    // Asset Category Registry
    std::cout << "\n--- Asset Category Registry ---" << std::endl;
    test_category_registry_empty();
    test_category_registry_register();
    test_category_registry_replace();
    test_category_registry_unregister();
    test_category_registry_by_content();
    test_category_registry_by_system();
    test_category_registry_content_for();
    test_category_registry_system_for();
    test_category_registry_defaults();
    test_category_registry_clear();
    test_category_registry_unregistered_defaults();

    // Replay Versioning
    std::cout << "\n--- Replay Versioning ---" << std::endl;
    test_replay_version_defaults();
    test_replay_version_set_versions();
    test_replay_version_register();
    test_replay_version_replace();
    test_replay_version_compatibility_current();
    test_replay_version_compatibility_too_new();
    test_replay_version_compatibility_too_old();
    test_replay_version_compatibility_upgradeable();
    test_replay_version_compatibility_unknown();
    test_replay_version_migration_path();
    test_replay_version_can_migrate();
    test_replay_version_deprecated();
    test_replay_version_all_versions();
    test_replay_version_migration_execution();
    test_replay_version_clear();

    // Mesh Viewer Panel
    std::cout << "\n--- Mesh Viewer Panel ---" << std::endl;
    test_mesh_viewer_name();
    test_mesh_viewer_no_mesh();
    test_mesh_viewer_load_mesh();
    test_mesh_viewer_bounds();
    test_mesh_viewer_view_modes();
    test_mesh_viewer_show_normals();
    test_mesh_viewer_show_grid();
    test_mesh_viewer_select_vertex();
    test_mesh_viewer_summary();
    test_mesh_viewer_clear();

    // Material Editor Panel
    std::cout << "\n--- Material Editor Panel ---" << std::endl;
    test_material_editor_name();
    test_material_editor_no_material();
    test_material_editor_load_material();
    test_material_editor_add_parameter();
    test_material_editor_remove_parameter();
    test_material_editor_get_parameter();
    test_material_editor_set_parameter();
    test_material_editor_select_parameter();
    test_material_editor_preview_mode();
    test_material_editor_summary();
    test_material_editor_clear();

    // Ship Hull Graph
    std::cout << "\n--- Ship Hull Graph ---" << std::endl;
    test_ship_hull_from_control_points();
    test_ship_hull_minimum_points();
    test_ship_hull_two_points();
    test_ship_turret_mount();
    test_ship_turret_sizes();
    test_ship_light_fixture();
    test_ship_light_interior_vs_exterior();
    test_ship_interior_volume();
    test_ship_visual_feature();
    test_ship_default_control_points_frigate();
    test_ship_default_control_points_capital();
    test_ship_default_control_points_deterministic();
    test_ship_default_hardpoints();
    test_ship_default_interiors();
    test_ship_default_interiors_large_ship();
    test_ship_default_lights();
    test_ship_graph_add_nodes();
    test_ship_graph_remove_node();
    test_ship_graph_compile_empty();
    test_ship_graph_control_points();
    test_ship_graph_turret_hardpoints();
    test_ship_graph_lights();
    test_ship_graph_interiors();
    test_ship_graph_visual_features();
    test_ship_graph_generate_from_seed();
    test_ship_graph_full_pipeline();
    test_ship_graph_export();
    test_ship_pcg_record_edits();
    test_ship_pcg_learning_profile();
    test_ship_pcg_apply_learning();

    // Ship Editor Panel
    std::cout << "\n--- Ship Editor Panel ---" << std::endl;
    test_ship_editor_name();
    test_ship_editor_no_ship();
    test_ship_editor_new_ship();
    test_ship_editor_new_ship_classes();
    test_ship_editor_compile_and_generate();
    test_ship_editor_selection();
    test_ship_editor_tools();
    test_ship_editor_view_modes();
    test_ship_editor_move_control_point();
    test_ship_editor_add_control_point();
    test_ship_editor_remove_control_point();
    test_ship_editor_add_turret();
    test_ship_editor_remove_turret();
    test_ship_editor_add_light();
    test_ship_editor_remove_light();
    test_ship_editor_add_interior();
    test_ship_editor_remove_interior();
    test_ship_editor_add_visual_feature();
    test_ship_editor_remove_visual_feature();
    test_ship_editor_has_cockpit();
    test_ship_editor_export_import();
    test_ship_editor_pcg_learning();
    test_ship_editor_summary();
    test_ship_editor_draw();
    test_ship_editor_draw_no_ship();
    test_ship_editor_set_ship_info();
    test_ship_editor_mark_clean();

    // Character Viewer / Editor / Animation
    std::cout << "\n--- Human Mesh Generator ---" << std::endl;
    test_human_mesh_generate_default();
    test_human_mesh_generate_from_context();
    test_human_mesh_deterministic();
    test_human_mesh_different_seeds();
    test_human_mesh_params_from_graph();
    test_human_mesh_height_affects_size();

    std::cout << "\n--- Character Animation Controller ---" << std::endl;
    test_anim_controller_default_state();
    test_anim_controller_set_state();
    test_anim_controller_update();
    test_anim_controller_clip_lengths();
    test_anim_controller_speed_and_damage();

    std::cout << "\n--- Character Select Screen ---" << std::endl;
    test_char_select_init_slots();
    test_char_select_select_slot();
    test_char_select_confirm();
    test_char_select_rotation();
    test_char_select_zoom();
    test_char_select_animation();
    test_char_select_draw();
    test_char_select_regenerate();

    std::cout << "\n--- Character Editor Panel ---" << std::endl;
    test_char_editor_name();
    test_char_editor_default_params();
    test_char_editor_set_params();
    test_char_editor_individual_setters();
    test_char_editor_regenerate_mesh();
    test_char_editor_sections();
    test_char_editor_animation();
    test_char_editor_draw();
    test_char_editor_summary();
    test_char_editor_callback();

    // Prefab Editor Panel
    std::cout << "\n--- Prefab Editor Panel ---" << std::endl;
    test_prefab_editor_name();
    test_prefab_editor_empty();
    test_prefab_editor_add_entity();
    test_prefab_editor_remove_entity();
    test_prefab_editor_parent_child();
    test_prefab_editor_remove_parent_removes_children();
    test_prefab_editor_remove_grandchildren();
    test_prefab_editor_remove_clears_selected_child();
    test_prefab_editor_add_component();
    test_prefab_editor_remove_component();
    test_prefab_editor_select_entity();
    test_prefab_editor_summary();
    test_prefab_editor_clear();

    // Truth UI Panel
    std::cout << "\n--- Truth UI Panel ---" << std::endl;
    test_truth_ui_name();
    test_truth_ui_defaults();
    test_truth_ui_wiring();
    test_truth_ui_summary_stopped();
    test_truth_ui_summary_no_pie();
    test_truth_ui_draw();

    // Asset Diff Commit Flow
    std::cout << "\n--- Asset Diff Commit Flow ---" << std::endl;
    test_asset_diff_commit_empty();
    test_asset_diff_commit_track_change();
    test_asset_diff_commit_untrack();
    test_asset_diff_commit_stage();
    test_asset_diff_commit_commit_success();
    test_asset_diff_commit_commit_no_staged();
    test_asset_diff_commit_validation_failure();
    test_asset_diff_commit_clear();
    test_asset_diff_commit_summary();
    test_asset_diff_commit_update_tracked();

    // Cross-Platform Save Compatibility
    std::cout << "\n--- Cross-Platform Save Compatibility ---" << std::endl;
    test_cross_platform_save_header_size();
    test_cross_platform_partial_header_size();
    test_cross_platform_save_magic_offset();
    test_cross_platform_save_hash_determinism();
    test_cross_platform_save_byte_exact();
    test_cross_platform_save_raw_bytes_roundtrip();
    test_cross_platform_partial_save_roundtrip();
    test_cross_platform_save_large_payload();
    test_cross_platform_hash_combine_deterministic();

    // UIScrollState
    std::cout << "\n--- UIScrollState ---" << std::endl;
    test_scroll_state_defaults();
    test_scroll_state_content_smaller_than_viewport();
    test_scroll_state_scrollable();
    test_scroll_state_scroll_down();
    test_scroll_state_scroll_up();
    test_scroll_state_clamp_top();
    test_scroll_state_clamp_bottom();
    test_scroll_state_scroll_to_top();
    test_scroll_state_scroll_to_bottom();
    test_scroll_state_viewport_resize_clamps();

    // UIManager Viewport/DPI/Input/Font
    std::cout << "\n--- UIManager Viewport/DPI/Input/Font ---" << std::endl;
    test_ui_manager_viewport_defaults();
    test_ui_manager_set_viewport_size();
    test_ui_manager_dpi_scale();
    test_ui_manager_event_router_access();
    test_ui_manager_dispatch_event();
    test_ui_manager_dispatch_not_initialized();
    test_ui_manager_font_bootstrap_access();
    test_ui_manager_shutdown_resets_viewport();

    // Truth UI Panel Draw Commands
    std::cout << "\n--- Truth UI Panel Draw Commands ---" << std::endl;
    test_truth_ui_draw_produces_commands();
    test_truth_ui_draw_no_pie_shows_message();
    test_truth_ui_draw_with_pie();
    test_truth_ui_draw_no_divergence();
    test_truth_ui_draw_clears_between_frames();
    test_truth_ui_draw_has_header();

    // UI Style
    std::cout << "\n--- UI Style ---" << std::endl;
    test_ui_style_dark_defaults();
    test_ui_style_light_theme();
    test_ui_style_spacing_defaults();
    test_ui_style_typography_defaults();
    test_ui_style_border_defaults();
    test_ui_style_status_colors();
    test_ui_style_dark_light_differ();

    // UI Scene Graph
    std::cout << "\n--- UI Scene Graph ---" << std::endl;
    test_scene_graph_init();
    test_scene_graph_add_child();
    test_scene_graph_remove_child();
    test_scene_graph_find_child();
    test_scene_graph_find_nested_child();
    test_scene_graph_layout_vertical();
    test_scene_graph_layout_horizontal();
    test_scene_graph_hit_test();
    test_scene_graph_draw_tree();
    test_scene_graph_invisible_child_skipped();
    test_scene_graph_dispatch_event();

    // Tile Chunk Builder
    std::cout << "\n--- Tile Chunk Builder ---" << std::endl;
    test_chunk_builder_empty_layer();
    test_chunk_builder_single_tile();
    test_chunk_builder_multiple_tiles();
    test_chunk_builder_world_to_chunk();
    test_chunk_builder_world_to_chunk_negative();
    test_chunk_builder_is_inside_chunk();
    test_chunk_builder_mark_dirty();
    test_chunk_builder_tiles_outside_chunk_ignored();
    test_chunk_builder_deterministic();
    test_chunk_builder_flip_flags();

    // Tile Palette Panel
    std::cout << "\n--- Tile Palette Panel ---" << std::endl;
    test_tile_palette_name();
    test_tile_palette_add_entries();
    test_tile_palette_selection();
    test_tile_palette_selection_oob();
    test_tile_palette_filter();
    test_tile_palette_favorites();
    test_tile_palette_favorites_only();
    test_tile_palette_columns();
    test_tile_palette_clear();
    test_tile_palette_draw();
    test_tile_palette_draw_with_filter();

    // Rule Graph Editor Panel
    std::cout << "\n--- Rule Graph Editor Panel ---" << std::endl;
    test_rule_graph_editor_name();
    test_rule_graph_editor_add_node();
    test_rule_graph_editor_remove_node();
    test_rule_graph_editor_add_wire();
    test_rule_graph_editor_remove_wires_for_node();
    test_rule_graph_editor_remove_node_removes_wires();
    test_rule_graph_editor_selection();
    test_rule_graph_editor_pan_zoom();
    test_rule_graph_editor_debug_mode();
    test_rule_graph_editor_draw();
    test_rule_graph_editor_draw_debug();
    test_rule_graph_editor_draw_with_selection();

    // Next Tasks Phase 4
    run_next_tasks_phase4_tests();

    // Next Tasks Phase 5
    run_next_tasks_phase5_tests();

    // Next Tasks Phase 6
    run_next_tasks_phase6_tests();

    // Next Tasks Phase 7
    run_next_tasks_phase7_tests();

    // Next Tasks Phase 8
    run_next_tasks_phase8_tests();

    // Next Tasks Phase 9
    run_next_tasks_phase9_tests();

    // Next Tasks Phase 10
    run_next_tasks_phase10_tests();

    // Next Tasks Phase 11
    run_next_tasks_phase11_tests();

    // Panel Draw + TileEditor Input Tests
    run_panel_draw_tests();

    // Menu System
    std::cout << "\n--- Menu System ---" << std::endl;
    test_menu_creation();
    test_menu_state();
    test_menu_manager();
    test_menu_hover();
    test_ui_manager_integration();

    std::cout << "\n--- GUI Full Completion ---" << std::endl;
    test_widget_disabled_state();
    test_disabled_item_not_clickable();
    test_disabled_item_widget_direct();
    test_shortcut_label_on_widget();
    test_shortcut_label_empty_by_default();
    test_shortcut_label_multiple_items();
    test_keyboard_nav_down();
    test_keyboard_nav_up();
    test_keyboard_nav_enter_select();
    test_keyboard_nav_escape_close();
    test_keyboard_nav_skip_disabled();
    test_keyboard_nav_skip_separator();
    test_keyboard_nav_left_right_switch_menu();
    test_keyboard_no_consume_when_closed();
    test_context_menu_open_close();
    test_context_menu_item_click();
    test_context_menu_keyboard_nav();
    test_submenu_flag_on_widget();
    test_submenu_open_via_keyboard();
    test_submenu_close_via_left();
    test_submenu_escape_closes_submenu_first();
    test_toolbar_widget_creation();
    test_toolbar_with_button_children();
    test_toolbar_rendering_with_null_renderer();
    test_statusbar_widget_creation();
    test_statusbar_text_update();
    test_statusbar_rendering_with_null_renderer();
    test_full_editor_menu_setup();
    test_menu_manager_is_any_open();
    test_close_all_menus_clears_focus();

    std::cout << "\n--- Menu Overlay & System Tab ---" << std::endl;
    test_menu_overlay_renders_on_top();
    test_menu_overlay_not_rendered_when_closed();
    test_logger_sink_callback();
    test_system_tab_creation();

    std::cout << "\n--- Menu Action Wiring ---" << std::endl;
    test_engine_request_exit();
    test_menu_toggle_panel_visibility();
    test_menu_action_item_dispatch();
    test_status_bar_update();

    std::cout << "\n--- GUI Next Tasks ---" << std::endl;
    test_checkable_default_false();
    test_set_checkable();
    test_set_checked();
    test_checkable_toggle_on_click();
    test_checkable_toggle_via_keyboard();
    test_icon_id_default_zero();
    test_set_icon_id();
    test_icon_rendering_with_null_renderer();
    test_tooltip_widget_creation();
    test_tooltip_rendering_with_null_renderer();
    test_tab_widget_creation();
    test_tab_active_state();
    test_tab_rendering_with_null_renderer();
    test_scrollview_widget_creation();
    test_scrollview_rendering_with_null_renderer();
    test_dockarea_widget_creation();
    test_dockarea_with_panels();
    test_dockarea_rendering_with_null_renderer();
    test_editor_checkable_view_items();

    std::cout << "\n--- GUI Managers ---" << std::endl;
    test_tab_manager_activate();
    test_tab_manager_no_change_same_tab();
    test_tab_manager_get_active();
    test_tab_manager_content_visibility();
    test_tab_manager_callback();
    test_tab_manager_handle_click();
    test_scroll_manager_register();
    test_scroll_manager_scroll_wheel();
    test_scroll_manager_scroll_outside();
    test_scroll_manager_not_scrollable();
    test_scroll_manager_to_top_bottom();
    test_scroll_manager_set_content_height();
    test_toolbar_manager_click();
    test_toolbar_manager_toggle();
    test_toolbar_manager_miss();
    test_scroll_wheel_event_type();
    test_ui_manager_tab_integration();
    test_ui_manager_scroll_integration();
    test_ui_manager_toolbar_integration();

    // GUI Next Managers
    std::cout << "\n--- FocusManager ---" << std::endl;
    test_focus_manager_init();
    test_focus_manager_click_input_field();
    test_focus_manager_click_outside();
    test_focus_manager_set_focusable();
    test_focus_manager_clear_focus();
    test_focus_manager_tab_cycles();
    test_focus_manager_callback();

    std::cout << "\n--- TooltipManager ---" << std::endl;
    test_tooltip_manager_init();
    test_tooltip_manager_set_tooltip();
    test_tooltip_manager_show_after_delay();
    test_tooltip_manager_hide_on_leave();
    test_tooltip_manager_hide_all();
    test_tooltip_manager_remove_tooltip();

    std::cout << "\n--- DockManager ---" << std::endl;
    test_dock_manager_init();
    test_dock_manager_register();
    test_dock_manager_dock_panel_left();
    test_dock_manager_dock_panel_right();
    test_dock_manager_dock_panel_center();
    test_dock_manager_undock_panel();
    test_dock_manager_callback();
    test_dock_manager_split_ratio();

    std::cout << "\n--- InputFieldManager ---" << std::endl;
    test_input_field_manager_init();
    test_input_field_manager_register();
    test_input_field_manager_type_text();
    test_input_field_manager_backspace();
    test_input_field_manager_delete_key();
    test_input_field_manager_arrow_keys();
    test_input_field_manager_enter_submits();
    test_input_field_manager_text_changed_callback();
    test_input_field_manager_set_text();
    test_input_field_manager_not_registered();

    std::cout << "\n--- UIManager Next Integration ---" << std::endl;
    test_ui_manager_focus_integration();
    test_ui_manager_dock_integration();
    test_ui_manager_tooltip_integration();

    std::cout << "\n--- Crash Reporter ---" << std::endl;
    test_crash_reporter_tool_exists();
    test_crash_reporter_help();
    test_crash_reporter_validate_empty_dir();
    test_crash_reporter_validate_manifest();
    test_crash_reporter_bundle();

    std::cout << "\n--- Include Firewall Enhancements ---" << std::endl;
    test_include_firewall_header_exists();
    test_include_firewall_defines_marker();
    test_include_firewall_guards_simulation();
    test_include_firewall_guards_core();
    test_include_firewall_ecs_no_render();
    test_include_firewall_physics_no_render();
    test_crash_reporter_workflow_exists();

    // Editor UI Fix tests
    run_editor_ui_fix_tests();

    // GUI Interaction Fix tests
    run_gui_interaction_fix_tests();

    // GUI New Widgets
    std::cout << "\n--- Checkbox Widget ---" << std::endl;
    test_checkbox_widget_creation();
    test_checkbox_toggle();
    test_checkbox_rendering_with_null_renderer();

    std::cout << "\n--- Slider Widget ---" << std::endl;
    test_slider_widget_creation();
    test_slider_set_value();
    test_slider_value_range();
    test_slider_rendering_with_null_renderer();

    std::cout << "\n--- ProgressBar Widget ---" << std::endl;
    test_progressbar_widget_creation();
    test_progressbar_set_value();
    test_progressbar_rendering_with_null_renderer();

    std::cout << "\n--- ComboBox Widget ---" << std::endl;
    test_combobox_widget_creation();
    test_combobox_selected_index();
    test_combobox_open_state();
    test_combobox_rendering_with_null_renderer();

    std::cout << "\n--- TreeNode Widget ---" << std::endl;
    test_treenode_widget_creation();
    test_treenode_expand_collapse();
    test_treenode_depth();
    test_treenode_rendering_with_null_renderer();

    std::cout << "\n--- Splitter Widget ---" << std::endl;
    test_splitter_widget_creation();
    test_splitter_rendering_with_null_renderer();

    std::cout << "\n--- ColorPicker Widget ---" << std::endl;
    test_colorpicker_widget_creation();
    test_colorpicker_set_get_color();
    test_colorpicker_rendering_with_null_renderer();

    std::cout << "\n--- SliderManager ---" << std::endl;
    test_slider_manager_init();
    test_slider_manager_click_sets_value();
    test_slider_manager_drag_updates_value();
    test_slider_manager_clamps_value();
    test_slider_manager_click_outside_ignored();

    std::cout << "\n--- ComboBoxManager ---" << std::endl;
    test_combobox_manager_init();
    test_combobox_manager_set_items();
    test_combobox_manager_click_opens();
    test_combobox_manager_select_item();
    test_combobox_manager_click_outside_closes();
    test_combobox_manager_get_selected_text_no_selection();
    test_combobox_manager_empty_items();

    std::cout << "\n--- UIManager New Widget Integration ---" << std::endl;
    test_ui_manager_slider_integration();
    test_ui_manager_combobox_integration();
    test_ui_manager_new_widget_rendering();

    std::cout << "\n--- CheckboxManager ---" << std::endl;
    test_checkbox_manager_init();
    test_checkbox_manager_click_toggles();
    test_checkbox_manager_click_outside_ignored();
    test_checkbox_manager_ignores_non_click();

    std::cout << "\n--- TreeNodeManager ---" << std::endl;
    test_treenode_manager_init();
    test_treenode_manager_click_toggles_expand();
    test_treenode_manager_click_outside_ignored();

    std::cout << "\n--- SplitterManager ---" << std::endl;
    test_splitter_manager_init();
    test_splitter_manager_drag_moves();
    test_splitter_manager_click_outside_ignored();
    test_splitter_manager_horizontal_drag();

    std::cout << "\n--- ColorPickerManager ---" << std::endl;
    test_colorpicker_manager_init();
    test_colorpicker_manager_click_opens();
    test_colorpicker_manager_click_outside_closes();
    test_colorpicker_manager_select_color();
    test_colorpicker_manager_set_color_fires_callback();

    std::cout << "\n--- UIManager Interaction Manager Integration ---" << std::endl;
    test_ui_manager_checkbox_integration();
    test_ui_manager_treenode_integration();
    test_ui_manager_splitter_integration();
    test_ui_manager_colorpicker_integration();

    // Renderer backend tests
    run_renderer_backend_tests();

    // Script VM and ABI capsule tests
    run_script_and_abi_tests();

    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
}
