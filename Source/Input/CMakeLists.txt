add_library(NF_Input STATIC src/Input.cpp)
target_include_directories(NF_Input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Input PUBLIC NF::Core PRIVATE NF_CompileOptions)
add_library(NF::Input ALIAS NF_Input)
