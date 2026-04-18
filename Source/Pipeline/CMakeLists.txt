add_library(NF_Pipeline STATIC src/Pipeline.cpp src/ToolWiring.cpp src/BlenderBridge.cpp src/WorkspaceBroker.cpp src/AtlasAIReasoner.cpp)
target_include_directories(NF_Pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Pipeline PUBLIC NF::Core PRIVATE NF_CompileOptions)
add_library(NF::Pipeline ALIAS NF_Pipeline)
