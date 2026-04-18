add_library(NF_World STATIC src/World.cpp)
target_include_directories(NF_World PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_World PUBLIC NF::Core NF::Engine NF::Game PRIVATE NF_CompileOptions)
add_library(NF::World ALIAS NF_World)
