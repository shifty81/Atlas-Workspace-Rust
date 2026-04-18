add_library(AtlasGameplay STATIC
    FactionSystem.cpp
    CombatFramework.cpp
    EconomySystem.cpp
)

target_include_directories(AtlasGameplay PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/atlas_gameplay>
)
target_link_libraries(AtlasGameplay PUBLIC AtlasEngine)

# --- Install / Export ---
install(TARGETS AtlasGameplay
    EXPORT AtlasGameplayTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/atlas_gameplay
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT AtlasGameplayTargets
    FILE AtlasGameplayTargets.cmake
    NAMESPACE Atlas::
    DESTINATION lib/cmake/AtlasGameplay
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/AtlasGameplayConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/AtlasGameplayConfig.cmake"
    INSTALL_DESTINATION lib/cmake/AtlasGameplay
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/AtlasGameplayConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AtlasGameplayConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/AtlasGameplayConfigVersion.cmake"
    DESTINATION lib/cmake/AtlasGameplay
)
