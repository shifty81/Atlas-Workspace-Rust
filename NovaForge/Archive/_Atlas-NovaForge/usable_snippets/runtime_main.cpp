#include "core/Engine.h"
#include "core/Logger.h"
#include "bootstrap/RuntimeBootstrap.h"
#include <iostream>
#include <string>

static void PrintUsage() {
    std::cout << "Atlas Runtime v1.0.0" << std::endl;
    std::cout << "Usage: atlas_runtime [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --project <path>     Load a .atlas project file" << std::endl;
    std::cout << "  --module <path>      Load a game module (shared library)" << std::endl;
    std::cout << "  --mode <mode>        Runtime mode: client, server (default: client)" << std::endl;
    std::cout << "  --validate-only      Validate project and exit" << std::endl;
    std::cout << "  --help               Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    auto args = atlas::bootstrap::ParseCommandLine(argc, argv);

    if (args.showHelp) {
        PrintUsage();
        return 0;
    }

    atlas::Logger::Init();

    // Load project if specified
    if (!args.projectPath.empty()) {
        if (!atlas::project::ProjectManager::Get().Load(args.projectPath)) {
            std::cerr << "Failed to load project: " << args.projectPath << std::endl;
            return 1;
        }

        if (args.validateOnly) {
            std::cout << "Project validation passed: "
                      << atlas::project::ProjectManager::Get().Descriptor().name << std::endl;
            return 0;
        }
    }

    // Determine engine mode
    atlas::EngineMode engineMode = atlas::EngineMode::Client;
    if (args.modeStr == "server") {
        engineMode = atlas::EngineMode::Server;
    }

    // Configure engine
    atlas::EngineConfig cfg;
    atlas::bootstrap::ApplyProjectConfig(cfg, engineMode);

    // Initialize and run
    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    // Load game module if specified
    atlas::module::ModuleLoader moduleLoader;
    atlas::net::ReplicationManager replication;
    atlas::asset::AssetRegistry assetRegistry;
    replication.SetWorld(&engine.GetWorld());

    if (!args.modulePath.empty()) {
        if (!atlas::bootstrap::LoadAndStartModule(
                moduleLoader, args.modulePath, engine, replication, assetRegistry)) {
            return 1;
        }
    }

    atlas::Logger::Info("Atlas Runtime starting...");
    engine.Run();

    atlas::bootstrap::ShutdownModule(moduleLoader, engine, replication, assetRegistry);

    return 0;
}
