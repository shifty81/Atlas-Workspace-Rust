# Arena2D Game Module
#
# Built as a STATIC library here because it lives inside the Atlas tree and
# is linked directly into the test executable.  In a standalone game repo
# the target would be SHARED so that the runtime can dlopen/LoadLibrary it:
#
#   find_package(AtlasEngine REQUIRED)
#   find_package(AtlasGameplay REQUIRED)
#   add_library(Arena2DModule SHARED Arena2DModule.cpp)
#   target_link_libraries(Arena2DModule PRIVATE Atlas::AtlasEngine Atlas::AtlasGameplay)

add_library(Arena2DModule STATIC
    Arena2DModule.cpp
)

target_include_directories(Arena2DModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Arena2DModule PUBLIC AtlasEngine AtlasGameplay)
