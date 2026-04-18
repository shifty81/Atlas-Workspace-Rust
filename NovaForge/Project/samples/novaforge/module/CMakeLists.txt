# NovaForge Game Module
#
# Built as a STATIC library here because it lives inside the Atlas tree and
# is linked directly into the test executable.  In a standalone game repo
# the target would be SHARED so that the runtime can dlopen/LoadLibrary it:
#
#   find_package(AtlasEngine REQUIRED)
#   find_package(AtlasGameplay REQUIRED)
#   add_library(NovaForgeModule SHARED NovaForgeModule.cpp)
#   target_link_libraries(NovaForgeModule PRIVATE Atlas::AtlasEngine Atlas::AtlasGameplay)

add_library(NovaForgeModule STATIC
    NovaForgeModule.cpp
)

target_include_directories(NovaForgeModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(NovaForgeModule PUBLIC AtlasEngine AtlasGameplay)
