add_library(NF_Audio STATIC src/Audio.cpp)
target_include_directories(NF_Audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Audio PUBLIC NF::Core PRIVATE NF_CompileOptions)
add_library(NF::Audio ALIAS NF_Audio)
