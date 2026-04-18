# ── Atlas Engine + Workspace test suite ──────────────────────────

# Guard: if tests are somehow added but online deps are disabled and
# Catch2 is not already available, fail early with a clear message.
find_package(Catch2 3 QUIET)
if(NOT Catch2_FOUND)
    if(NOT ATLAS_ENABLE_ONLINE_DEPS)
        message(FATAL_ERROR
            "Tests requested but Catch2 was not found locally and "
            "ATLAS_ENABLE_ONLINE_DEPS is OFF. Either:\n"
            "  - Set ATLAS_ENABLE_ONLINE_DEPS=ON to fetch from GitHub, or\n"
            "  - Vendor Catch2 locally under ThirdParty/Catch2, or\n"
            "  - Set ATLAS_BUILD_TESTS=OFF to skip tests.")
    endif()

    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)
endif()

# On MSVC, force serialised PDB writes so parallel builds don't corrupt Catch2d.pdb
if(MSVC)
    target_compile_options(Catch2 PRIVATE /FS)
endif()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

# ── Route all test executables into bin/Tests/ ────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Tests)
foreach(CFG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CFG} CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CFG}/Tests)
endforeach()

# Core tests
add_executable(NF_CoreTests
    Core/test_core.cpp
    catch_main.cpp
)
target_link_libraries(NF_CoreTests PRIVATE NF::Core Catch2::Catch2)
catch_discover_tests(NF_CoreTests)

# Engine tests
add_executable(NF_EngineTests
    Engine/test_engine.cpp
    catch_main.cpp
)
target_link_libraries(NF_EngineTests PRIVATE NF::Engine NF::Core Catch2::Catch2)
catch_discover_tests(NF_EngineTests)

# Physics tests
add_executable(NF_PhysicsTests
    Physics/test_physics.cpp
    catch_main.cpp
)
target_link_libraries(NF_PhysicsTests PRIVATE NF::Physics NF::Core Catch2::Catch2)
catch_discover_tests(NF_PhysicsTests)

# Audio tests
add_executable(NF_AudioTests
    Audio/test_audio.cpp
    catch_main.cpp
)
target_link_libraries(NF_AudioTests PRIVATE NF::Audio NF::Core Catch2::Catch2)
catch_discover_tests(NF_AudioTests)

# Animation tests
add_executable(NF_AnimationTests
    Animation/test_animation.cpp
    Animation/test_s5_animation.cpp
    catch_main.cpp
)
target_link_libraries(NF_AnimationTests PRIVATE NF::Animation NF::Core Catch2::Catch2)
catch_discover_tests(NF_AnimationTests)

# Input tests
add_executable(NF_InputTests
    Input/test_input.cpp
    catch_main.cpp
)
target_link_libraries(NF_InputTests PRIVATE NF::Input NF::Core Catch2::Catch2)
catch_discover_tests(NF_InputTests)

# Networking tests
add_executable(NF_NetworkingTests
    Networking/test_networking.cpp
    catch_main.cpp
)
target_link_libraries(NF_NetworkingTests PRIVATE NF::Networking NF::Engine NF::Core Catch2::Catch2)
catch_discover_tests(NF_NetworkingTests)

# GraphVM tests
add_executable(NF_GraphVMTests
    GraphVM/test_graphvm.cpp
    catch_main.cpp
)
target_link_libraries(NF_GraphVMTests PRIVATE NF::GraphVM NF::Core Catch2::Catch2)
catch_discover_tests(NF_GraphVMTests)

# Renderer tests
add_executable(NF_RendererTests
    Renderer/test_renderer.cpp
    catch_main.cpp
)
target_link_libraries(NF_RendererTests PRIVATE NF::Renderer NF::Core Catch2::Catch2)
catch_discover_tests(NF_RendererTests)

# AI tests
add_executable(NF_AITests
    AI/test_ai.cpp
    catch_main.cpp
)
target_link_libraries(NF_AITests PRIVATE NF::AI NF::Engine NF::Core NF::GraphVM Catch2::Catch2)
catch_discover_tests(NF_AITests)

# Editor tests
add_executable(NF_EditorTests
    Editor/test_editor.cpp
    Editor/test_m2_editor.cpp
    Editor/test_m3_editor.cpp
    Editor/test_m4_editor.cpp
    Editor/test_m1c_editor.cpp
    Editor/test_tool_modules.cpp
    Editor/test_workspace_shell.cpp
    Editor/test_asset_editor_tool.cpp
    Editor/test_material_editor_tool.cpp
    Editor/test_animation_editor_tool.cpp
    Editor/test_data_editor_tool.cpp
    Editor/test_visual_logic_editor_tool.cpp
    Editor/test_build_tool.cpp
    Editor/test_atlas_ai_tool.cpp
    Editor/test_workspace_bootstrap.cpp
    catch_main.cpp
)
target_include_directories(NF_EditorTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Source/Editor/src
)
target_link_libraries(NF_EditorTests PRIVATE
    NF::Editor NF::Engine NF::Renderer NF::UI NF::Core NF::GraphVM NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_EditorTests)

# Pipeline tests
add_executable(NF_PipelineTests
    Pipeline/test_pipeline.cpp
    Pipeline/test_tool_wiring.cpp
    Pipeline/test_blender_bridge.cpp
    Pipeline/test_workspace_broker.cpp
    Pipeline/test_arbiter_reasoner.cpp
    Pipeline/test_full_suite.cpp
    catch_main.cpp
)
target_link_libraries(NF_PipelineTests PRIVATE NF::Pipeline NF::Core Catch2::Catch2)
catch_discover_tests(NF_PipelineTests)

# UI tests
add_executable(NF_UITests
    UI/test_ui.cpp
    UI/test_atlasui.cpp
    UI/test_m1_editor.cpp
    UI/test_widgets_v2.cpp
    catch_main.cpp
)
target_link_libraries(NF_UITests PRIVATE NF::UI NF::Core NF::Input Catch2::Catch2)
catch_discover_tests(NF_UITests)

# Phase 2 — UI backend interface contract tests
add_executable(NF_UIBackendTests
    UI/test_ui_backends.cpp
    catch_main.cpp
)
target_link_libraries(NF_UIBackendTests PRIVATE NF::UI NF::Core Catch2::Catch2)
catch_discover_tests(NF_UIBackendTests)

# Phase D.5 — D3D11 Backend Activation
add_executable(NF_PhaseD5Tests
    UI/test_phase_d5_d3d11_activation.cpp
    catch_main.cpp
)
target_link_libraries(NF_PhaseD5Tests PRIVATE NF::UI NF::Core Catch2::Catch2)
catch_discover_tests(NF_PhaseD5Tests)

# Workspace wiring tests (DrawListDispatcher, InputBridge, new widgets)
add_executable(NF_WorkspaceWiringTests
    UI/test_workspace_wiring.cpp
    catch_main.cpp
)
target_link_libraries(NF_WorkspaceWiringTests PRIVATE NF::Editor NF::UI NF::Core NF::Input Catch2::Catch2)
catch_discover_tests(NF_WorkspaceWiringTests)

# Phase 4 — AtlasAI and Codex Integration tests
add_executable(NF_Phase4Tests
    Workspace/test_phase4_atlasai.cpp
    catch_main.cpp
)
target_include_directories(NF_Phase4Tests PRIVATE
    ${CMAKE_SOURCE_DIR}/Source/Editor/src
)
target_link_libraries(NF_Phase4Tests PRIVATE
    NF::Workspace NF::Pipeline NF::Editor NF::Engine NF::Renderer NF::UI NF::Core NF::GraphVM NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase4Tests)

# Phase 2 — Workspace shell subsystem tests
add_executable(NF_WorkspaceSubsystemTests
    Workspace/test_workspace_shell_subsystems.cpp
    catch_main.cpp
)
target_include_directories(NF_WorkspaceSubsystemTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Source/Editor/src
)
target_link_libraries(NF_WorkspaceSubsystemTests PRIVATE
    NF::Workspace NF::Editor NF::Engine NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_WorkspaceSubsystemTests)

# Phase 5 — Hosted Project Support tests
add_executable(NF_Phase5Tests
    Workspace/test_phase5_hosted_project.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase5Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase5Tests)

# Phase 6 — Build, Patch, and Release Pipeline tests
add_executable(NF_Phase6Tests
    Workspace/test_phase6_build_release.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase6Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase6Tests)

# Phase 7 — Workspace Integration Surfaces tests
add_executable(NF_Phase7Tests
    Workspace/test_phase7_workspace_integration.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase7Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase7Tests)

# Phase 8 — Runtime Wiring tests
add_executable(NF_Phase8Tests
    Workspace/test_phase8_runtime_wiring.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase8Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase8Tests)

# Phase 9 — Asset Pipeline and Content Routing tests
add_executable(NF_Phase9Tests
    Workspace/test_phase9_asset_pipeline.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase9Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase9Tests)

# Phase 10 — Project Persistence and Serialization tests
add_executable(NF_Phase10Tests
    Workspace/test_phase10_persistence.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase10Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase10Tests)

# Phase 11 — Command Bus and Action System tests
add_executable(NF_Phase11Tests
    Workspace/test_phase11_command_bus.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase11Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase11Tests)

# Phase 12 — Event Bus and Workspace Notifications tests
add_executable(NF_Phase12Tests
    Workspace/test_phase12_event_bus.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase12Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase12Tests)

# Phase 13 — Workspace Preferences and Configuration tests
add_executable(NF_Phase13Tests
    Workspace/test_phase13_preferences.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase13Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase13Tests)

# Phase 14 — Workspace Plugin System tests
add_executable(NF_Phase14Tests
    Workspace/test_phase14_plugin_system.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase14Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase14Tests)

# Phase 15 — Workspace Diagnostics and Telemetry tests
add_executable(NF_Phase15Tests
    Workspace/test_phase15_diagnostics.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase15Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase15Tests)

# Phase 16 — Workspace Scripting and Automation tests
add_executable(NF_Phase16Tests
    Workspace/test_phase16_scripting.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase16Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase16Tests)

# Phase 17 — Workspace Search and Indexing tests
add_executable(NF_Phase17Tests
    Workspace/test_phase17_search.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase17Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase17Tests)

# Phase 18 — Workspace Undo/Redo Stack tests
add_executable(NF_Phase18Tests
    Workspace/test_phase18_undo_redo.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase18Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase18Tests)

# Phase 19 — Workspace Session Management tests
add_executable(NF_Phase19Tests
    Workspace/test_phase19_session.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase19Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase19Tests)

# Phase 20 — Workspace Clipboard System tests
add_executable(NF_Phase20Tests
    Workspace/test_phase20_clipboard.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase20Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase20Tests)

# Phase 21 — Workspace Focus and Context Tracking tests
add_executable(NF_Phase21Tests
    Workspace/test_phase21_focus.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase21Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase21Tests)

# Phase 22 — Workspace Drag and Drop System tests
add_executable(NF_Phase22Tests
    Workspace/test_phase22_drag_drop.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase22Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase22Tests)

# Phase 23 — Workspace Hotkey Manager tests
add_executable(NF_Phase23Tests
    Workspace/test_phase23_hotkeys.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase23Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase23Tests)

# Phase 24 — Workspace Tooltip and Help System tests
add_executable(NF_Phase24Tests
    Workspace/test_phase24_tooltip.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase24Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase24Tests)

# Phase 25 — Workspace Status Bar System tests
add_executable(NF_Phase25Tests
    Workspace/test_phase25_status_bar.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase25Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase25Tests)

# Phase 26 — Workspace Context Menu System tests
add_executable(NF_Phase26Tests
    Workspace/test_phase26_context_menu.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase26Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase26Tests)

# Phase 27 — Workspace Badge and Icon Registry tests
add_executable(NF_Phase27Tests
    Workspace/test_phase27_badge.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase27Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase27Tests)

# Phase 28 — Workspace Minimap / Overview tests
add_executable(NF_Phase28Tests
    Workspace/test_phase28_minimap.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase28Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase28Tests)

# Phase 29 — Workspace Annotation System tests
add_executable(NF_Phase29Tests
    Workspace/test_phase29_annotation.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase29Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase29Tests)

# Phase 30 — Workspace Filter and Search Index tests
add_executable(NF_Phase30Tests
    Workspace/test_phase30_filter_index.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase30Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase30Tests)

# Phase 31 — Workspace Theme System tests
add_executable(NF_Phase31Tests
    Workspace/test_phase31_theme.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase31Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase31Tests)

# Phase 32 — Workspace Tour / Onboarding System tests
add_executable(NF_Phase32Tests
    Workspace/test_phase32_tour.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase32Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase32Tests)

# Phase 33 — Workspace Split View / Tab Groups tests
add_executable(NF_Phase33Tests
    Workspace/test_phase33_split_view.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase33Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase33Tests)

# Phase 34 — Workspace Frame Controller tests
add_executable(NF_Phase34Tests
    Workspace/test_phase34_frame_controller.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase34Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase34Tests)

# Phase 35 — Workspace Project File tests
add_executable(NF_Phase35Tests
    Workspace/test_phase35_project_file.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase35Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase35Tests)

# Phase 36 — AI Assistant Panel and AI Panel Session tests
add_executable(NF_Phase36Tests
    Workspace/test_phase36_ai_panel.cpp
    catch_main.cpp
)
target_include_directories(NF_Phase36Tests PRIVATE
    ${CMAKE_SOURCE_DIR}/Source/Editor/src
)
target_link_libraries(NF_Phase36Tests PRIVATE
    NF::Workspace NF::Pipeline NF::Editor NF::Engine NF::Renderer NF::UI NF::Core NF::GraphVM NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase36Tests)

# Phase 37 — Console Command Bus tests
add_executable(NF_Phase37Tests
    Workspace/test_phase37_console_command_bus.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase37Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase37Tests)

# Phase 38 — Dock Tree Serializer tests
add_executable(NF_Phase38Tests
    Workspace/test_phase38_dock_tree_serializer.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase38Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase38Tests)

# Phase 39 — Content Router and Drop Target Handler tests
add_executable(NF_Phase39Tests
    Workspace/test_phase39_content_router.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase39Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase39Tests)

# Phase 40 — Asset Import Queue tests
add_executable(NF_Phase40Tests
    Workspace/test_phase40_asset_import_queue.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase40Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase40Tests)

# Phase 41 — Workspace Layout Manager tests
add_executable(NF_Phase41Tests
    Workspace/test_phase41_workspace_layout.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase41Tests PRIVATE
    NF::Workspace NF::Pipeline NF::Editor NF::Engine NF::Renderer NF::UI NF::Core NF::GraphVM NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase41Tests)

# Phase 42 — Logging Route V1 tests
add_executable(NF_Phase42Tests
    Workspace/test_phase42_logging_route.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase42Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase42Tests)

# Phase 43 — Notifications (NotificationQueue) tests
add_executable(NF_Phase43Tests
    Workspace/test_phase43_notifications.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase43Tests PRIVATE
    NF::Workspace NF::Pipeline NF::Editor NF::Engine NF::Renderer NF::UI NF::Core NF::GraphVM NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase43Tests)

# Phase 44 — NotificationSystem tests
add_executable(NF_Phase44Tests
    Workspace/test_phase44_notification_system.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase44Tests PRIVATE
    NF::Workspace NF::Pipeline NF::Editor NF::Engine NF::Renderer NF::UI NF::Core NF::GraphVM NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase44Tests)

# Phase 45 — UndoRedoSystem tests
add_executable(NF_Phase45Tests
    Workspace/test_phase45_undo_redo.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase45Tests PRIVATE
    NF::Workspace NF::Pipeline NF::Editor NF::Engine NF::Renderer NF::UI NF::Core NF::GraphVM NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase45Tests)

# Phase 46 — SelectionSystem tests
add_executable(NF_Phase46Tests
    Workspace/test_phase46_selection_system.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase46Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase46Tests)

# Phase 47 — ProjectManager tests
add_executable(NF_Phase47Tests
    Workspace/test_phase47_project_manager.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase47Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase47Tests)

# Phase 48 — WorkspaceActivityBar
add_executable(NF_Phase48Tests
    Workspace/test_phase48_activity_bar.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase48Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase48Tests)

# Phase 49 — WorkspaceRecentFiles
add_executable(NF_Phase49Tests
    Workspace/test_phase49_recent_files.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase49Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase49Tests)

# Phase 50 — WorkspaceQuickOpen
add_executable(NF_Phase50Tests
    Workspace/test_phase50_quick_open.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase50Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase50Tests)

# Phase 51 — WorkspaceKeymap
add_executable(NF_Phase51Tests
    Workspace/test_phase51_keymap.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase51Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase51Tests)

# Phase 52 — WorkspaceWindowState
add_executable(NF_Phase52Tests
    Workspace/test_phase52_window_state.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase52Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase52Tests)

# Phase 53 — WorkspaceProjectTemplate
add_executable(NF_Phase53Tests
    Workspace/test_phase53_project_template.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase53Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase53Tests)

# Phase 54 — WorkspaceBreadcrumb
add_executable(NF_Phase54Tests
    Workspace/test_phase54_breadcrumb.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase54Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase54Tests)

# Phase 55 — WorkspaceFavorites
add_executable(NF_Phase55Tests
    Workspace/test_phase55_favorites.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase55Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase55Tests)

# Phase 56 — WorkspaceTaskQueue
add_executable(NF_Phase56Tests
    Workspace/test_phase56_task_queue.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase56Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase56Tests)

# Phase 57 — WorkspaceSnippetLibrary
add_executable(NF_Phase57Tests
    Workspace/test_phase57_snippet_library.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase57Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase57Tests)

# Phase 58 — WorkspaceOutputPanel
add_executable(NF_Phase58Tests
    Workspace/test_phase58_output_panel.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase58Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase58Tests)

# Phase 59 — WorkspacePropertyInspector
add_executable(NF_Phase59Tests
    Workspace/test_phase59_property_inspector.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase59Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase59Tests)

# Phase 60 — WorkspaceFileWatcher
add_executable(NF_Phase60Tests
    Workspace/test_phase60_file_watcher.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase60Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase60Tests)

# Phase 61 — WorkspaceChangelog
add_executable(NF_Phase61Tests
    Workspace/test_phase61_changelog.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase61Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase61Tests)

# Phase 62 — WorkspaceVariableStore
add_executable(NF_Phase62Tests
    Workspace/test_phase62_variable_store.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase62Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase62Tests)

# Phase 63 — WorkspaceCommandPalette
add_executable(NF_Phase63Tests
    Workspace/test_phase63_command_palette.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase63Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase63Tests)

# Phase 64 — Viewport Rendering Implementation
add_executable(NF_Phase64Tests
    Workspace/test_phase64_viewport_rendering.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase64Tests PRIVATE
    NF::Workspace NF::UI NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase64Tests)

# Phase 65 — WorkspaceViewportManager
add_executable(NF_Phase65Tests
    Workspace/test_phase65_viewport_manager.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase65Tests PRIVATE
    NF::Workspace NF::UI NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase65Tests)

# Phase 66 — Viewport Wiring (end-to-end: shell → tool → surface → panel)
add_executable(NF_Phase66Tests
    Workspace/test_phase66_viewport_wiring.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase66Tests PRIVATE
    NF::Editor NF::Workspace NF::UI NF::Core NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase66Tests)

# Phase 67 — WorkspaceAssetBrowser
add_executable(NF_Phase67Tests
    Workspace/test_phase67_asset_browser.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase67Tests PRIVATE
    NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_Phase67Tests)

# Phase 68 — TextInput typed-text wiring (IInputContext::typedText, BasicInputContext,
#            WorkspaceInputBridge, TextInput::handleInput)
add_executable(NF_Phase68Tests
    UI/test_phase68_text_input_wiring.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase68Tests PRIVATE
    NF::Workspace NF::UI NF::Core NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase68Tests)

# Phase 69 — Tool Render Contract (Audit Patch 5)
#   ToolViewRenderContext, IHostedTool::renderToolView virtual,
#   per-tool panel layout backed by live runtime state.
add_executable(NF_Phase69Tests
    Editor/test_phase69_tool_render_contract.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase69Tests PRIVATE
    NF::Editor NF::Workspace NF::UI NF::Core NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase69Tests)

# Phase 70 — NovaForge Panel Factories (Audit Patch 6)
#   IEditorPanel interface, 6 NovaForge gameplay panel classes,
#   createPanel factories in NovaForgeAdapter, ProjectSystemsTool live caching.
if(TARGET NF::NovaForgeAdapter)
    add_executable(NF_Phase70Tests
        Workspace/test_phase70_novaforge_panel_factories.cpp
        catch_main.cpp
    )
    target_link_libraries(NF_Phase70Tests PRIVATE
        NF::NovaForgeAdapter NF::Workspace NF::Core
        Catch2::Catch2
    )
    catch_discover_tests(NF_Phase70Tests)
endif()

# Phase 71 — Audit Patches 7/9/11/12
#   Patch 7:  EditorApp deprecation markers
#   Patch 9:  AssetCatalogPopulator — extension classification + catalog population
#   Patch 11: SettingsStore/LayoutPersistence wired into WorkspaceShell
#   Patch 12: Validator gaps (verified by validate_project.sh)
add_executable(NF_Phase71Tests
    Workspace/test_phase71_audit_patches.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase71Tests PRIVATE
    NF::Editor NF::Workspace NF::UI NF::Core NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase71Tests)

# Phase 72 — Viewport entity projection: SoftwareViewportRenderer draws scene entities
#   72.1: NovaForgePreviewRuntime::provideScene() populates ViewportSceneState::entities
#   72.2: SoftwareViewportRenderer::projectPoint() correctly projects world→screen
#   72.3: renderGrid() writes entity-coloured pixels when entities are in front of camera
#   72.4: Behind-camera entities are culled
#   72.5: Selected entities use selectColor
#   72.6: End-to-end provideScene → renderGrid → pixels
if(TARGET NF::NovaForgeAdapter)
    add_executable(NF_Phase72Tests
        Workspace/test_phase72_viewport_entity_render.cpp
        catch_main.cpp
    )
    target_include_directories(NF_Phase72Tests PRIVATE
        ${CMAKE_SOURCE_DIR}/NovaForge/Source/EditorAdapter/include
    )
    target_link_libraries(NF_Phase72Tests PRIVATE
        NF::NovaForgeAdapter NF::Editor NF::Workspace NF::UI NF::Core
        Catch2::Catch2
    )
    catch_discover_tests(NF_Phase72Tests)
endif()


#   B.1: ProjectBootstrapResult + AtlasProjectFileLoader manifest validation
#   B.2: NovaForgeProjectBootstrap service + adapter wiring
#   B.3: NovaForgeDocumentType / NovaForgeDocument / NovaForgeDocumentRegistry
#   B.4: AssetCatalogPopulator filesystem scan + ContentBrowserPanel/InspectorPanel wiring
if(TARGET NF::NovaForgeAdapter)
    add_executable(NF_PhaseBTests
        Workspace/test_phase_b.cpp
        catch_main.cpp
    )
    target_link_libraries(NF_PhaseBTests PRIVATE
        NF::NovaForgeAdapter NF::Editor NF::Workspace NF::Core
        Catch2::Catch2
    )
    catch_discover_tests(NF_PhaseBTests)
endif()

# Phase C — Panels Edit Real Data
#   C.1: IDocumentPanel interface + DocumentPanelBase lifecycle
#   C.2: Six NovaForge gameplay panels with real data schemas
#   C.3: DocumentPropertyGrid schema-driven editing + validation
#   C.4: DocumentSavePipeline — save/revert/validate with notification sink
if(TARGET NF::NovaForgeAdapter)
    add_executable(NF_PhaseCTests
        Workspace/test_phase_c.cpp
        catch_main.cpp
    )
    target_link_libraries(NF_PhaseCTests PRIVATE
        NF::NovaForgeAdapter NF::Editor NF::Workspace NF::Core
        Catch2::Catch2
    )
    catch_discover_tests(NF_PhaseCTests)
endif()

# Phase D — Runtime-Backed Viewport
#   D.1: NovaForgePreviewWorld entity lifecycle + transforms + selection
#   D.1: NovaForgePreviewRuntime IViewportSceneProvider + fly-camera + gizmo + hierarchy
#   D.2: SceneEditorTool provideScene() delegation to attached runtime
#   D.3: NovaForgeAssetPreview IViewportSceneProvider + bind/edit/apply/revert
#   D.3: AssetEditorTool provideScene() delegation to attached asset preview
if(TARGET NF::NovaForgeAdapter)
    add_executable(NF_PhaseDTests
        Workspace/test_phase_d.cpp
        catch_main.cpp
    )
    target_include_directories(NF_PhaseDTests PRIVATE
        ${CMAKE_SOURCE_DIR}/NovaForge/Source/EditorAdapter/include
    )
    target_link_libraries(NF_PhaseDTests PRIVATE
        NF::NovaForgeAdapter NF::Editor NF::Workspace NF::Core
        Catch2::Catch2
    )
    catch_discover_tests(NF_PhaseDTests)
endif()

# Phase D Completion — D.3 extended (collider/socket/anchor/PCG tag metadata) + D.4 (MaterialPreview)
if(TARGET NF::NovaForgeAdapter)
    add_executable(NF_PhaseDCompletionTests
        Workspace/test_phase_d_completion.cpp
        catch_main.cpp
    )
    target_include_directories(NF_PhaseDCompletionTests PRIVATE
        ${CMAKE_SOURCE_DIR}/NovaForge/Source/EditorAdapter/include
    )
    target_link_libraries(NF_PhaseDCompletionTests PRIVATE
        NF::NovaForgeAdapter NF::Editor NF::Workspace NF::Core
        Catch2::Catch2
    )
    catch_discover_tests(NF_PhaseDCompletionTests)
endif()

# Phase E — Shared PCG Preview Pipeline
#   E.1: PCGRuleSet + PCGDeterministicSeedContext + PCGGeneratorService
#   E.2: PCGPreviewService (binding, regen, seed, stats)
#   E.3: Rule editing via PCGPreviewService (auto-regen, callbacks, domain override)
#   E.4: populatePreviewWorld — NovaForgePreviewWorld from PCG placements
if(TARGET NF::NovaForgeAdapter)
    add_executable(NF_PhaseETests
        Workspace/test_phase_e.cpp
        catch_main.cpp
    )
    target_include_directories(NF_PhaseETests PRIVATE
        ${CMAKE_SOURCE_DIR}/NovaForge/Source/EditorAdapter/include
    )
    target_link_libraries(NF_PhaseETests PRIVATE
        NF::NovaForgeAdapter NF::Editor NF::Workspace NF::Core
        Catch2::Catch2
    )
    catch_discover_tests(NF_PhaseETests)
endif()

# Phase E Completion — E.3 ProcGenRuleEditorPanel + E.4 event-driven PCG tag wiring
if(TARGET NF::NovaForgeAdapter)
    add_executable(NF_PhaseECompletionTests
        Workspace/test_phase_e_completion.cpp
        catch_main.cpp
    )
    target_include_directories(NF_PhaseECompletionTests PRIVATE
        ${CMAKE_SOURCE_DIR}/NovaForge/Source/EditorAdapter/include
    )
    target_link_libraries(NF_PhaseECompletionTests PRIVATE
        NF::NovaForgeAdapter NF::Editor NF::Workspace NF::Core
        Catch2::Catch2
    )
    catch_discover_tests(NF_PhaseECompletionTests)
endif()

# Phase F — Play-In-Editor (PIE)
#   F.1: PIEService lifecycle (enter/exit/pause/resume/step/reset) + diagnostics + perf counters
#   F.2: PIEInputRouter — editor↔game input mode switch + escape handling
#   F.3: PIEExternalLaunch — build→launch pipeline + stdout routing
add_executable(NF_PhaseFTests
    Workspace/test_phase_f.cpp
    catch_main.cpp
)
target_link_libraries(NF_PhaseFTests PRIVATE
    NF::Editor NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_PhaseFTests)

# Phase F — PIE wiring: toolbar controller + console output bridge
#   F.PIE_TOOLBAR: PIEToolbarController — Play/Pause/Stop/Step, read-only gate, callbacks
#   F.PIE_CONSOLE: PIEConsoleBridge — wires PIEExternalLaunch stdout → ConsolePanel
add_executable(NF_PhaseFPIEWiringTests
    Workspace/test_phase_f_pie_wiring.cpp
    catch_main.cpp
)
target_link_libraries(NF_PhaseFPIEWiringTests PRIVATE
    NF::Editor NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_PhaseFPIEWiringTests)

# WorkspaceProjectState normalization tests (Phase G ownership wiring)
#   Tests WorkspaceProjectState as the workspace-level project session spine:
#   document registry, dirty tracking, save/revert, panel binding context,
#   change listeners, and WorkspaceShell::projectState() accessor.
add_executable(NF_WorkspaceProjectStateTests
    Workspace/test_workspace_project_state.cpp
    catch_main.cpp
)
target_link_libraries(NF_WorkspaceProjectStateTests PRIVATE
    NF::Editor NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_WorkspaceProjectStateTests)

# Workspace runtime wiring tests — contract, panel lazy-init, summaryRows, viewport
if(TARGET NF::NovaForgeAdapter)
add_executable(NF_WorkspaceRuntimeWiringTests
    Workspace/test_workspace_wiring.cpp
    catch_main.cpp
)
target_link_libraries(NF_WorkspaceRuntimeWiringTests PRIVATE
    NF::NovaForgeAdapter NF::Editor NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_WorkspaceRuntimeWiringTests)
endif()

# Phase G — Full Tool Wiring (G.1–G.8 document models)
#   G.1: SceneDocument — entity hierarchy, transforms, components, save/load
#   G.2: AssetDocument — LOD/variant/dependency/reimport metadata
#   G.3: MaterialDocument — shader graph nodes, pins, connections, params
#   G.4: AnimationDocument — channels, keyframes (sorted), clip metadata
#   G.5: GraphDocument — visual logic nodes, pins, connections, compile
#   G.6: BuildTaskGraph — DAG, topological order, state, build log, summary
#   G.7: AIRequestContext — context binding, messages, diff proposals, codex
#   G.8: DataTableDocument — columns, rows, cells, validation, CSV export
add_executable(NF_PhaseGTests
    Workspace/test_phase_g.cpp
    catch_main.cpp
)
target_link_libraries(NF_PhaseGTests PRIVATE
    NF::Editor NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_PhaseGTests)

# Phase G.2 — Asset Editor full wiring (3D preview, dependency viewer, reimport pipeline)
#   AssetPreviewState   — transform/material slot/attachment editing
#   AssetDependencyViewer — dependency list with filter and highlight
#   AssetReimportPipeline — import job queue, processing, status, callbacks
add_executable(NF_PhaseG2AssetEditorTests
    Workspace/test_phase_g2_asset_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_PhaseG2AssetEditorTests PRIVATE
    NF::Editor NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_PhaseG2AssetEditorTests)

# Phase H — UX Completion (H.1–H.6)
#   H.1: PreferencesUIController — category nav, editable settings, import/export/reset, scope
#   H.2: KeybindEditorController — rebind capture, conflict detection, reset-to-defaults
#   H.3: LayoutPersistenceController — named presets, built-in presets, auto-save on close
#   H.4: CommandPaletteController — open/close/toggle, fuzzy search, recent commands, navigation
#   H.5: NotificationCenterController — history, toast popups, severity filter, click-to-navigate
#   H.6: ProjectOpenFlowController — recent projects, file open flow, validation, new project wizard
add_executable(NF_PhaseHTests
    Workspace/test_phase_h.cpp
    catch_main.cpp
)
target_link_libraries(NF_PhaseHTests PRIVATE
    NF::Editor NF::Workspace NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_PhaseHTests)

# Phase I — NovaForge End-to-End Project Load Integration
#   I.1: AtlasProjectFileLoader validates the real NovaForge.atlas manifest
#   I.2: NovaForgeProjectBootstrap validates the real project directory layout
#   I.3: ProjectRegistry loads NovaForge via loadProjectFromAtlasFile()
#   I.4: NovaForgeAdapter initializes and exposes all 6 gameplay panels
#   I.5: ProjectOpenFlowController.validate() with real manifest content
if(TARGET NF::NovaForgeAdapter)
    add_executable(NF_PhaseITests
        Workspace/test_phase_i.cpp
        catch_main.cpp
    )
    target_compile_definitions(NF_PhaseITests PRIVATE
        NF_NOVAFORGE_ATLAS_PATH="${CMAKE_SOURCE_DIR}/NovaForge/NovaForge.atlas"
    )
    target_include_directories(NF_PhaseITests PRIVATE
        ${CMAKE_SOURCE_DIR}/NovaForge/Source/EditorAdapter/include
    )
    target_link_libraries(NF_PhaseITests PRIVATE
        NF::NovaForgeAdapter NF::Editor NF::Workspace NF::Core
        Catch2::Catch2
    )
    catch_discover_tests(NF_PhaseITests)
endif()


add_executable(NF_EditorS187Tests
    Editor/Legacy/test_s187_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS187Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS187Tests)

# S188 — ClothSimEditorV1, PostProcessEditorV1, HeightmapEditorV1
add_executable(NF_EditorS188Tests
    Editor/Legacy/test_s188_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS188Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS188Tests)

# S189 — SkeletonEditorV1, MorphTargetEditorV1, IKRigEditorV1
add_executable(NF_EditorS189Tests
    Editor/Legacy/test_s189_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS189Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS189Tests)

# S190 — NavLinkEditorV1, SoundCueEditorV1, LensFlareEditorV1
add_executable(NF_EditorS190Tests
    Editor/Legacy/test_s190_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS190Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS190Tests)

# S191 — WeaponEditorV1, InventoryItemEditorV1, AbilityEditorV1
add_executable(NF_EditorS191Tests
    Editor/Legacy/test_s191_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS191Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS191Tests)

# S192 — DialogueNodeEditorV1, CutsceneEditorV1, LocalizationTableEditorV1
add_executable(NF_EditorS192Tests
    Editor/Legacy/test_s192_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS192Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS192Tests)

# S180 — ProcGenRuleEditorV1, WaveFunctionCollapseEditorV1, GrammarGeneratorEditorV1
add_executable(NF_EditorS180Tests
    Editor/Legacy/test_s180_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS180Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS180Tests)

# S181 — WaterSimEditorV1, WeatherSystemEditorV1, VoxelPaintEditorV1
add_executable(NF_EditorS181Tests
    Editor/Legacy/test_s181_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS181Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS181Tests)

# S182 — UIDesignEditorV1, UndoRedoSystemV1, TypographySystemV1
add_executable(NF_EditorS182Tests
    Editor/Legacy/test_s182_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS182Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS182Tests)

# S183 — TrailEditorV1, VFXGraphEditorV1, TouchInputMapperV1
add_executable(NF_EditorS183Tests
    Editor/Legacy/test_s183_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS183Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS183Tests)

# S184 — CinematicEditorV1, DecalEditorV1, LightingRigEditorV1
add_executable(NF_EditorS184Tests
    Editor/Legacy/test_s184_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS184Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS184Tests)

# S185 — LODEditorV1, MeshBatchEditorV1, ProceduralMeshEditorV1
add_executable(NF_EditorS185Tests
    Editor/Legacy/test_s185_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS185Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS185Tests)

# S186 — GamepadMapperV1, KeyBindingEditorV1, InputProfileEditorV1
add_executable(NF_EditorS186Tests
    Editor/Legacy/test_s186_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS186Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS186Tests)

# S193 — ParticleSystemEditorV1, AudioGraphEditorV1, ShaderEditorV1
add_executable(NF_EditorS193Tests
    Editor/Legacy/test_s193_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS193Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS193Tests)

# S194 — MaterialNodeEditorV1, TextureViewerV1, RenderPassEditorV1
add_executable(NF_EditorS194Tests
    Editor/Legacy/test_s194_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS194Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS194Tests)

# S195 — SequencerTrackV1, TimelineMarkerV1, SoundMixerEditorV1
add_executable(NF_EditorS195Tests
    Editor/Legacy/test_s195_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS195Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS195Tests)

# S196 — SpriteEditorV1, TilemapEditorV1, TexturePainterV1
add_executable(NF_EditorS196Tests
    Editor/Legacy/test_s196_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS196Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS196Tests)

# S197 — PhysicsJointEditorV1, DestructionEditorV1, WindFieldEditorV1
add_executable(NF_EditorS197Tests
    Editor/Legacy/test_s197_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS197Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS197Tests)

# S198 — CameraRigEditorV1, LevelStreamEditorV1, AtmosphereEditorV1
add_executable(NF_EditorS198Tests
    Editor/Legacy/test_s198_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS198Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS198Tests)

# S199 — FogVolumeEditorV1, OcclusionVolumeEditorV1, SkydomeEditorV1
add_executable(NF_EditorS199Tests
    Editor/Legacy/test_s199_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS199Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS199Tests)

# S200 — TagSystemEditorV1, DebugDrawEditorV1, ObjectPoolEditorV1
add_executable(NF_EditorS200Tests
    Editor/Legacy/test_s200_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS200Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS200Tests)

# S201 — TriggerVolumeEditorV1, InteractionEditorV1, HapticEditorV1
add_executable(NF_EditorS201Tests
    Editor/Legacy/test_s201_editor.cpp
    catch_main.cpp
)
target_link_libraries(NF_EditorS201Tests PRIVATE
    NF::Editor NF::Core
    Catch2::Catch2
)
catch_discover_tests(NF_EditorS201Tests)

# Phase 67 (Camera) — Viewport camera fly-cam wiring and software renderer scanline write
add_executable(NF_ViewportCameraTests
    Workspace/test_phase67_viewport_camera.cpp
    catch_main.cpp
)
target_link_libraries(NF_ViewportCameraTests PRIVATE
    NF::Editor NF::Workspace NF::UI NF::Core NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_ViewportCameraTests)

# Phase 73 — PIE Viewport Full Integration (game rendering in PIE mode)
add_executable(NF_Phase73PIEViewportTests
    Workspace/test_phase73_pie_viewport.cpp
    catch_main.cpp
)
target_link_libraries(NF_Phase73PIEViewportTests PRIVATE
    NF::Editor NF::Workspace NF::UI NF::Core NF::Input
    Catch2::Catch2
)
catch_discover_tests(NF_Phase73PIEViewportTests)
