#include "core/Engine.h"
#include "core/Logger.h"
#include "project/ProjectManager.h"
#include "module/ModuleLoader.h"
#include "module/IGameModule.h"
#include "net/Replication.h"
#include "rules/ServerRules.h"
#include "assets/AssetRegistry.h"
#include <iostream>
#include <string>

static void PrintUsage() {
    std::cout << "Atlas Server" << std::endl;
    std::cout << "Usage: AtlasServer [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --project <path>   Load a .atlas project file" << std::endl;
    std::cout << "  --module <path>    Load a game module (shared library)" << std::endl;
    std::cout << "  --help             Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string projectPath;
    std::string modulePath;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--project" && i + 1 < argc) {
            projectPath = argv[++i];
        } else if (arg == "--module" && i + 1 < argc) {
            modulePath = argv[++i];
        } else if (arg == "--help") {
            PrintUsage();
            return 0;
        }
    }

    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Server;

    // Load project descriptor if specified
    if (!projectPath.empty()) {
        atlas::Logger::Init();
        if (!atlas::project::ProjectManager::Get().Load(projectPath)) {
            std::cerr << "Failed to load project: " << projectPath << std::endl;
            return 1;
        }
        cfg.tickRate = atlas::project::ProjectManager::Get().Descriptor().runtime.tickRate;
        cfg.assetRoot = atlas::project::ProjectManager::Get().Descriptor().assets.root;
    }

    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    // Load game module if specified
    if (!modulePath.empty()) {
        atlas::module::ModuleLoader moduleLoader;
        atlas::net::ReplicationManager replication;
        atlas::asset::AssetRegistry assetRegistry;
        replication.SetWorld(&engine.GetWorld());

        auto result = moduleLoader.Load(modulePath);
        if (result != atlas::module::ModuleLoadResult::Success) {
            std::cerr << "Failed to load game module: " << modulePath << std::endl;
            return 1;
        }

        atlas::module::GameModuleContext ctx{
            engine.GetWorld(),
            engine.GetNet(),
            replication,
            atlas::rules::ServerRules::Get(),
            assetRegistry,
            atlas::project::ProjectManager::Get().Descriptor(),
            &engine.GetPhysics()
        };

        auto* mod = moduleLoader.GetModule();
        mod->RegisterTypes(ctx);
        mod->ConfigureReplication(ctx);
        mod->ConfigureServerRules(ctx);
        mod->OnStart(ctx);

        auto desc = mod->Describe();
        atlas::Logger::Info(std::string("Game module loaded: ") + desc.name);

        // Attach module to engine so OnTick is called each frame
        engine.SetGameModule(mod, &ctx);

        engine.Run();

        mod->OnShutdown(ctx);
    } else {
        engine.Run();
    }

    return 0;
}
