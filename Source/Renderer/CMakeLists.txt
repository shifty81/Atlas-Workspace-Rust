add_library(NF_Renderer STATIC
    src/Renderer.cpp
)

target_include_directories(NF_Renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(NF_Renderer PUBLIC NF::Core NF::Engine PRIVATE NF_CompileOptions)

add_library(NF::Renderer ALIAS NF_Renderer)
