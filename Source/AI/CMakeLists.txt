add_library(NF_AI STATIC src/AI.cpp)
target_include_directories(NF_AI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_AI PUBLIC NF::Core NF::Engine NF::GraphVM PRIVATE NF_CompileOptions)
add_library(NF::AI ALIAS NF_AI)
