add_library(NF_Animation STATIC src/Animation.cpp)
target_include_directories(NF_Animation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Animation PUBLIC NF::Core NF::Engine PRIVATE NF_CompileOptions)
add_library(NF::Animation ALIAS NF_Animation)
