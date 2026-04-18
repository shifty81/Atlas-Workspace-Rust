add_library(NF_Patch INTERFACE)
target_include_directories(NF_Patch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(NF::Patch ALIAS NF_Patch)
