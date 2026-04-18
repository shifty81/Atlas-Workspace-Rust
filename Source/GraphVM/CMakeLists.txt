add_library(NF_GraphVM STATIC src/GraphVM.cpp)
target_include_directories(NF_GraphVM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_GraphVM PUBLIC NF::Core PRIVATE NF_CompileOptions)
add_library(NF::GraphVM ALIAS NF_GraphVM)
