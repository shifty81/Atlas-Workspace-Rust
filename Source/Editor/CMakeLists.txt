add_library(NF_Editor STATIC
    src/Editor.cpp
    src/SceneEditorTool.cpp
    src/AssetEditorTool.cpp
    src/MaterialEditorTool.cpp
    src/AnimationEditorTool.cpp
    src/DataEditorTool.cpp
    src/VisualLogicEditorTool.cpp
    src/BuildTool.cpp
    src/AtlasAITool.cpp
    src/IDETool.cpp
    src/Tools/ConsoleToolModule.cpp
    src/Tools/ProfilerToolModule.cpp
    src/Tools/ECSInspectorToolModule.cpp
    src/Tools/MaterialEditorToolModule.cpp
    src/Tools/MeshViewerToolModule.cpp
    src/Tools/PrefabEditorToolModule.cpp
    src/Tools/AIDebuggerToolModule.cpp
    src/Tools/WorldGraphToolModule.cpp
    src/Tools/SceneGraphToolModule.cpp
    src/Tools/PhysicsTunerToolModule.cpp
)
target_include_directories(NF_Editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_Editor PUBLIC
    NF::Core NF::Engine NF::Renderer NF::UI NF::GraphVM NF::Input NF::Pipeline NF::Workspace
    PRIVATE NF_CompileOptions)
add_library(NF::Editor ALIAS NF_Editor)
