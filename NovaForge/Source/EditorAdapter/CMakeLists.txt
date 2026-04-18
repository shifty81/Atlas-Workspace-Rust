# NF::NovaForgeAdapter — NovaForge project adapter for Atlas Workspace
#
# INTERFACE library: header-only. All implementation lives in
# NovaForgeAdapter.h (inline). Future .cpp files can be added here.
#
# Consumers link this target to gain access to NovaForge::NovaForgeAdapter
# and its transitive dependency on NF::Workspace (IGameProjectAdapter, etc.).
#
# nlohmann-json is linked optionally: when available (via vcpkg or system),
# the gameplay panels load real project JSON data on project open.
# When absent the panels fall back to built-in defaults at compile time
# via the NF_*_HAS_JSON guards (no runtime error, no broken build).

add_library(NF_NovaForgeAdapter INTERFACE)
target_include_directories(NF_NovaForgeAdapter INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(NF_NovaForgeAdapter INTERFACE NF::Workspace)

# Optional: nlohmann-json for real project-data loading in gameplay panels.
find_package(nlohmann_json 3 QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(NF_NovaForgeAdapter INTERFACE nlohmann_json::nlohmann_json)
    message(STATUS "NovaForgeAdapter: nlohmann_json found — panels will load real project JSON")
else()
    message(STATUS "NovaForgeAdapter: nlohmann_json not found — panels use built-in defaults")
endif()

add_library(NF::NovaForgeAdapter ALIAS NF_NovaForgeAdapter)
