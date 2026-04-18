add_library(NF_Game STATIC src/Game.cpp)
target_include_directories(NF_Game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Game PUBLIC NF::Core NF::Engine NF::UI NF::Renderer NF::Physics PRIVATE NF_CompileOptions)
add_library(NF::Game ALIAS NF_Game)
