#include "core/Engine.h"
#include "core/Logger.h"
#include "bootstrap/RuntimeBootstrap.h"
#ifdef ATLAS_INCLUDE_TOOLS
#include "ui/DiagnosticsOverlay.h"
#endif
#include <iostream>
#include <string>

static void PrintUsage() {
    std::cout << "Atlas Client" << std::endl;
    std::cout << "Usage: AtlasClient [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --project <path>   Load a .atlas project file" << std::endl;
    std::cout << "  --module <path>    Load a game module (shared library)" << std::endl;
    std::cout << "  --help             Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    auto args = atlas::bootstrap::ParseCommandLine(argc, argv);

    if (args.showHelp) {
        PrintUsage();
        return 0;
    }

    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Client;

    // Load project descriptor if specified
    if (!args.projectPath.empty()) {
        atlas::Logger::Init();
        if (!atlas::project::ProjectManager::Get().Load(args.projectPath)) {
            std::cerr << "Failed to load project: " << args.projectPath << std::endl;
            return 1;
        }
        atlas::bootstrap::ApplyProjectConfig(cfg, atlas::EngineMode::Client);
    }

    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();

    // Load game module if specified
    if (!args.modulePath.empty()) {
        atlas::module::ModuleLoader moduleLoader;
        atlas::net::ReplicationManager replication;
        atlas::asset::AssetRegistry assetRegistry;
        replication.SetWorld(&engine.GetWorld());

        if (!atlas::bootstrap::LoadAndStartModule(
                moduleLoader, args.modulePath, engine, replication,
                assetRegistry, /*configureServerRules=*/false)) {
            return 1;
        }

        // Diagnostics overlay is available (off by default in client;
        // toggle at runtime with Ctrl+`).

        engine.Run();

        atlas::bootstrap::ShutdownModule(moduleLoader, engine, replication, assetRegistry);
    } else {
        // Diagnostics overlay is available (off by default in client;
        // toggle at runtime with Ctrl+`).

        engine.Run();
    }

    return 0;
}
