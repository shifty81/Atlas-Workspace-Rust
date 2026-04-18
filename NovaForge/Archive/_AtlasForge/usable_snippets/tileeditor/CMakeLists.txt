add_executable(AtlasTileEditor
    main.cpp
    ${CMAKE_SOURCE_DIR}/editor/tools/TileEditorModule.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/TilePalettePanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/panels/RuleGraphEditorPanel.cpp
    ${CMAKE_SOURCE_DIR}/editor/ui/EditorLayout.cpp
)

target_link_libraries(AtlasTileEditor AtlasEngine)
target_include_directories(AtlasTileEditor PRIVATE ${CMAKE_SOURCE_DIR}/editor)
