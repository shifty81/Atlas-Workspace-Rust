add_library(NF_Physics STATIC src/Physics.cpp)
target_include_directories(NF_Physics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Physics PUBLIC NF::Core NF::Engine PRIVATE NF_CompileOptions)
add_library(NF::Physics ALIAS NF_Physics)
