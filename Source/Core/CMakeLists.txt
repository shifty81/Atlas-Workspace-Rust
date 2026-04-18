add_library(NF_Core STATIC
    src/Core.cpp
)

target_include_directories(NF_Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(NF_Core PUBLIC cxx_std_20)
target_link_libraries(NF_Core PRIVATE NF_CompileOptions)

add_library(NF::Core ALIAS NF_Core)
