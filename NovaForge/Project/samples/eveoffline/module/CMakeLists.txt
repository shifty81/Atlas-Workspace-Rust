# EveOffline Game Module
#
# Built as a STATIC library here because it lives inside the Atlas tree and
# is linked directly into the test executable.  In a standalone game repo
# the target would be SHARED so that the runtime can dlopen/LoadLibrary it:
#
#   find_package(AtlasEngine REQUIRED)
#   find_package(AtlasGameplay REQUIRED)
#   add_library(EveOfflineModule SHARED EveOfflineModule.cpp)
#   target_link_libraries(EveOfflineModule PRIVATE Atlas::AtlasEngine Atlas::AtlasGameplay)

add_library(EveOfflineModule STATIC
    EveOfflineModule.cpp
)

target_include_directories(EveOfflineModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(EveOfflineModule PUBLIC AtlasEngine AtlasGameplay)
