///
/// AtlasTileEditor — standalone tile-editing tool.
///
/// Uses the same engine core, renderer, UI, and asset systems as the
/// full editor but launches directly into tile-editing mode.  This
/// allows artists and designers to work on tile maps without loading
/// the complete editor.
///
/// Usage:
///   AtlasTileEditor --project <path>
///

#include "../engine/core/Engine.h"
#include "../engine/core/Logger.h"
#include "../engine/ui/UIManager.h"
#include "../editor/tools/TileEditorModule.h"
#include "../editor/panels/TilePalettePanel.h"
#include "../editor/panels/RuleGraphEditorPanel.h"
#include <cstring>
#include <string>

static std::string ParseStringArg(int argc, char** argv, const char* flag) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], flag) == 0 && i + 1 < argc) {
            return argv[i + 1];
        }
    }
    return {};
}

static bool HasFlag(int argc, char** argv, const char* flag) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], flag) == 0) return true;
    }
    return false;
}

int main(int argc, char** argv) {
    atlas::Logger::Info("AtlasTileEditor starting");

    std::string projectPath = ParseStringArg(argc, argv, "--project");
    if (projectPath.empty()) {
        atlas::Logger::Warn("No --project specified, using current directory");
        projectPath = ".";
    }

    // Initialise engine core (renderer, assets, UI)
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Editor;
    if (HasFlag(argc, argv, "--headless")) {
        cfg.headless = true;
    }
    atlas::Engine engine(cfg);
    engine.InitCore();

    // Register tile editor module
    atlas::editor::TileEditorModule tileEditor;
    tileEditor.OnRegister();
    tileEditor.RegisterPanels();
    tileEditor.RegisterMenus();
    tileEditor.RegisterModes();

    // Create palette and graph panels
    atlas::editor::TilePalettePanel palette;
    atlas::editor::RuleGraphEditorPanel ruleGraph;

    atlas::Logger::Info("AtlasTileEditor ready — project: " + projectPath);

    // Initialise rendering and UI so the tile editor loop can display
    // panels via the standard Engine infrastructure.
    engine.InitRender();
    engine.InitUI();
    engine.InitECS();

    // Run the main loop through the engine (handles window events,
    // tick scheduling, rendering, and the diagnostics overlay).
    engine.Run();

    tileEditor.OnUnregister();
    engine.Shutdown();

    atlas::Logger::Info("AtlasTileEditor shut down");
    return 0;
}
