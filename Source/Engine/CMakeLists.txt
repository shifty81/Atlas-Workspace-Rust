add_library(NF_Engine STATIC
    src/Engine.cpp
)

target_include_directories(NF_Engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(NF_Engine PUBLIC NF::Core PRIVATE NF_CompileOptions)

add_library(NF::Engine ALIAS NF_Engine)
