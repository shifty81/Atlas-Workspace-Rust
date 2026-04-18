#include "../engine/core/Engine.h"
#include "../engine/core/Logger.h"
#include "../engine/project/ProjectManager.h"
#include "../engine/module/ModuleLoader.h"
#include "../engine/module/IGameModule.h"
#include "../engine/net/Replication.h"
#include "../engine/rules/ServerRules.h"
#include "../engine/assets/AssetRegistry.h"
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
    std::string projectPath;
    std::string modulePath;
    std::string modeStr = "client";
    bool validateOnly = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--project" && i + 1 < argc) {
            projectPath = argv[++i];
        } else if (arg == "--module" && i + 1 < argc) {
            modulePath = argv[++i];
        } else if (arg == "--mode" && i + 1 < argc) {
            modeStr = argv[++i];
        } else if (arg == "--validate-only") {
            validateOnly = true;
        } else if (arg == "--help") {
            PrintUsage();
            return 0;
        }
    }

    atlas::Logger::Init();

    // Load project if specified
    if (!projectPath.empty()) {
        if (!atlas::project::ProjectManager::Get().Load(projectPath)) {
            std::cerr << "Failed to load project: " << projectPath << std::endl;
            return 1;
        }

        if (validateOnly) {
            std::cout << "Project validation passed: "
                      << atlas::project::ProjectManager::Get().Descriptor().name << std::endl;
            return 0;
        }
    }

    // Determine engine mode
    atlas::EngineMode engineMode = atlas::EngineMode::Client;
    if (modeStr == "server") {
        engineMode = atlas::EngineMode::Server;
    }

    // Configure engine
    atlas::EngineConfig cfg;
    cfg.mode = engineMode;

    if (atlas::project::ProjectManager::Get().IsLoaded()) {
        cfg.tickRate = atlas::project::ProjectManager::Get().Descriptor().runtime.tickRate;
        cfg.assetRoot = atlas::project::ProjectManager::Get().Descriptor().assets.root;
    }

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

    if (!modulePath.empty()) {
        auto result = moduleLoader.Load(modulePath);
        if (result != atlas::module::ModuleLoadResult::Success) {
            std::cerr << "Failed to load game module: " << modulePath << std::endl;
            return 1;
        }
    }

    atlas::module::GameModuleContext moduleCtx{
        engine.GetWorld(),
        engine.GetNet(),
        replication,
        atlas::rules::ServerRules::Get(),
        assetRegistry,
        atlas::project::ProjectManager::Get().Descriptor(),
        &engine.GetPhysics()
    };

    if (moduleLoader.IsLoaded()) {
        auto* mod = moduleLoader.GetModule();
        mod->RegisterTypes(moduleCtx);
        mod->ConfigureReplication(moduleCtx);
        mod->ConfigureServerRules(moduleCtx);
        mod->OnStart(moduleCtx);

        auto desc = mod->Describe();
        atlas::Logger::Info(std::string("Game module loaded: ") + desc.name);

        // Attach module to engine so OnTick is called each frame
        engine.SetGameModule(mod, &moduleCtx);
    }

    atlas::Logger::Info("Atlas Runtime starting...");
    engine.Run();

    if (moduleLoader.IsLoaded()) {
        moduleLoader.GetModule()->OnShutdown(moduleCtx);
    }

    return 0;
}
