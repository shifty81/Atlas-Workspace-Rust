# ── AtlasWorkspace — primary executable ──────────────────────────
# AtlasWorkspace.exe is the single authoritative entrypoint for all
# development workflows. Child apps are launched through the registry.

option(ATLAS_WORKSPACE_WIN32_APP "Build as WIN32 subsystem (no console window)" OFF)

if(WIN32 AND ATLAS_WORKSPACE_WIN32_APP)
    set(_ws_subsystem WIN32)
else()
    set(_ws_subsystem "")
endif()

add_executable(AtlasWorkspace ${_ws_subsystem}
    main.cpp
)

target_link_libraries(AtlasWorkspace PRIVATE
    NF::NovaForgeAdapter
    NF::Editor
    NF::Workspace
    NF::Pipeline
    NF::AI
    NF::UI
    NF::Input
    NF::Renderer
    NF::Engine
    NF::Core
    NF_CompileOptions
)

# Place the workspace binary at the root of bin/ (not bin/Tests/)
set_target_properties(AtlasWorkspace PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY                "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${CMAKE_BINARY_DIR}/bin/MinSizeRel"
)
