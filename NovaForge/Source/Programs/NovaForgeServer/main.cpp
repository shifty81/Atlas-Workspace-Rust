// NovaForge Server — Dedicated headless server
// Accepts launch arguments from AtlasWorkspace via WorkspaceLaunchContract:
//   --headless              (default; no UI)
//   --hosted                (launched by Workspace; same as headless)
//   --project=<path>        (project directory path)
//   --workspace-root=<path>
//   --session-id=<id>
//   --port=<number>         (listen port; default 7777)
//   --maxplayers=<number>   (max connected clients; default 16)
//   --map=<name>            (map/world seed name; default "default")
//
// Structured status lines are printed to stdout so AtlasWorkspace can
// parse the server's state without a full IPC channel:
//   [STATUS] state=<ready|running|shutdown> port=<n> players=<n>/<max>
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Networking/Networking.h"
#include "NF/Game/Game.h"
#include "NF/World/World.h"
#include "NF/AI/AI.h"
#include <cstdio>
#include <string>
#include <string_view>
#ifdef _WIN32
#  include <windows.h>
#endif
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>

static std::atomic<bool> g_running{true};

static void handleSignal(int) {
    g_running = false;
}

// Print a structured status line that AtlasWorkspace (or any log parser)
// can grep for to know the server's current state.
static void printStatus(const char* state, uint16_t port,
                        uint32_t players, uint32_t maxPlayers)
{
    std::printf("[STATUS] state=%s port=%u players=%u/%u\n",
                state, port, players, maxPlayers);
    std::fflush(stdout);
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT,  handleSignal);
    std::signal(SIGTERM, handleSignal);

    // ── CLI argument parsing ──────────────────────────────────────
    uint16_t    listenPort  = 7777;
    uint32_t    maxPlayers  = 16;
    std::string mapName     = "default";
    std::string projectPath;
    std::string workspaceRoot;
    std::string sessionId;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg == "--headless" || arg == "--hosted") {
            // Both mean headless server; no-op.
        } else if (arg.substr(0, 7) == "--port=") {
            try {
                unsigned long val = std::stoul(std::string(arg.substr(7)));
                if (val < 1 || val > 65535) {
                    std::fprintf(stderr, "[WARN] --port value %lu out of range (1-65535); using default %u\n",
                                 val, listenPort);
                } else {
                    listenPort = static_cast<uint16_t>(val);
                }
            } catch (const std::exception&) {
                std::fprintf(stderr, "[WARN] invalid --port value; using default %u\n", listenPort);
            }
        } else if (arg.substr(0, 13) == "--maxplayers=") {
            try {
                unsigned long val = std::stoul(std::string(arg.substr(13)));
                if (val < 1 || val > 1000) {
                    std::fprintf(stderr, "[WARN] --maxplayers value %lu out of range (1-1000); using default %u\n",
                                 val, maxPlayers);
                } else {
                    maxPlayers = static_cast<uint32_t>(val);
                }
            } catch (const std::exception&) {
                std::fprintf(stderr, "[WARN] invalid --maxplayers value; using default %u\n", maxPlayers);
            }
        } else if (arg.substr(0, 6) == "--map=") {
            mapName = std::string(arg.substr(6));
        } else if (arg.substr(0, 10) == "--project=") {
            projectPath = std::string(arg.substr(10));
        } else if (arg.substr(0, 17) == "--workspace-root=") {
            workspaceRoot = std::string(arg.substr(17));
        } else if (arg.substr(0, 13) == "--session-id=") {
            sessionId = std::string(arg.substr(13));
        }
        // Unknown args silently ignored for forward-compat.
    }

    NF::coreInit();
    NF_LOG_INFO("Main","=== NovaForge Dedicated Server ===");
    NF_LOG_INFO("Main", std::string("Version: ") + NF::NF_VERSION_STRING);
    NF_LOG_INFO("Main", std::string("Port: ")       + std::to_string(listenPort));
    NF_LOG_INFO("Main", std::string("MaxPlayers: ") + std::to_string(maxPlayers));
    NF_LOG_INFO("Main", std::string("Map: ")        + mapName);
    if (!projectPath.empty())
        NF_LOG_INFO("Main", "Project: " + projectPath);
    if (!sessionId.empty())
        NF_LOG_INFO("Main", "Session: " + sessionId);

    NF::NetworkManager network;
    network.init(NF::NetRole::Server);

    NF::WorldGenerator worldGen;
    worldGen.init(42);

    NF::AISystem ai;
    ai.init();

    NF_LOG_INFO("Main", std::string("Server ready — listening on port ")
                        + std::to_string(listenPort));
    NF_LOG_INFO("Main","Press Ctrl+C to stop");
    printStatus("ready", listenPort, 0, maxPlayers);

    using Clock = std::chrono::high_resolution_clock;
    auto lastTick   = Clock::now();
    auto lastStatus = Clock::now();
    constexpr float kTickRate        = 1.f / 30.f; // 30 Hz server tick
    constexpr float kStatusIntervalS = 5.f;         // status line every 5 s

    while (g_running) {
        auto now = Clock::now();
        float dt = std::chrono::duration<float>(now - lastTick).count();
        lastTick = now;

        network.tick(dt);
        ai.update(dt);

        // Print a periodic status line so the workspace can poll liveness.
        float statusElapsed =
            std::chrono::duration<float>(now - lastStatus).count();
        if (statusElapsed >= kStatusIntervalS) {
            printStatus("running", listenPort,
                        static_cast<uint32_t>(
                            network.connectionManager().connectionCount()),
                        maxPlayers);
            lastStatus = now;
        }

#ifdef _WIN32
        // Check for ESC key on Windows console
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            g_running = false;
        Sleep(static_cast<DWORD>(kTickRate * 1000.f));
#else
        std::this_thread::sleep_for(
            std::chrono::duration<float>(kTickRate));
#endif
    }

    printStatus("shutdown", listenPort, 0, maxPlayers);
    NF_LOG_INFO("Main","Server shutting down...");
    ai.shutdown();
    worldGen.shutdown();
    network.shutdown();
    NF::coreShutdown();
    return 0;
}
