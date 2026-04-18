# NF::Workspace — Workspace OS layer (shell, registries, services, AI host)
#
# This module owns the workspace shell infrastructure that is independent of
# specific editor tools. Tools register with the workspace at boot through
# ToolRegistry; they are not hardcoded here.
#
# See Docs/Canon/01_LOCKED_DIRECTION.md — "Atlas Workspace is a generic
# development host, not a game."

add_library(NF_Workspace INTERFACE)
target_include_directories(NF_Workspace INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Workspace INTERFACE NF::Core NF::Engine NF::UI NF::Input NF::GraphVM)
add_library(NF::Workspace ALIAS NF_Workspace)
