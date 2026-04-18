add_library(NF_Networking STATIC src/Networking.cpp)
target_include_directories(NF_Networking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Networking PUBLIC NF::Core NF::Engine PRIVATE NF_CompileOptions)
add_library(NF::Networking ALIAS NF_Networking)
