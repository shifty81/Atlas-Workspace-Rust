#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include "../ecs/ECS.h"
#include "../net/NetContext.h"
#include "../sim/TickScheduler.h"
#include "../sim/TimeModel.h"
#include "../sim/WorldState.h"
#include "../sim/SaveSystem.h"
#include "../ui/UIManager.h"
#include "../ui/UIEventRouter.h"
#include "../platform/PlatformWindow.h"
#include "../render/RenderAPI.h"
#include "../render/EditorViewportFramebuffer.h"

namespace atlas::ui { class UIRenderer; }

namespace atlas {

enum class RuntimeRole {
    Editor,
    Client,
    Server
};

enum class EngineMode {
    Editor,
    Client,
    Server
};

enum class Capability {
    AssetWrite,
    Rendering,
    Physics,
    GraphEdit,
    GraphExecute,
    NetAuthority,
    HotReload
};

struct EngineConfig {
    EngineMode mode = EngineMode::Client;
    std::string assetRoot = "assets";
    uint32_t tickRate = 30;
    uint32_t maxTicks = 0; // 0 = unlimited (run forever), >0 = stop after N ticks
    render::RenderAPI renderAPI = render::RenderAPI::OpenGL;
    int32_t windowWidth = 1280;
    int32_t windowHeight = 720;
    bool headless = false;
    uint32_t autosaveInterval = 0;            // 0 = disabled, >0 = autosave every N ticks
    std::string autosavePath = "autosave.asav";
};

class Engine {
public:
    explicit Engine(const EngineConfig& cfg);
    ~Engine();

    void InitCore();
    void InitRender();
    void InitUI();
    void InitECS();
    void InitNetworking();
    void InitEditor();

    void Run();
    void RunEditor();
    void RunClient();
    void RunServer();

    bool LoadAndReplay(const std::string& savePath);
    bool ReplayFromSave(const std::string& savePath, const std::string& replayPath);
    bool RollbackToTick(uint64_t tick);

    /// Rollback to a snapshot tick, resimulate forward to the target tick,
    /// and verify that the resulting state hash matches the original snapshot.
    /// Returns true if hashes match (determinism verified).
    bool RollbackAndVerify(uint64_t snapshotTick, uint64_t targetTick);

    /// Save the current world state, reload it, resimulate for the given
    /// number of ticks, and compare the resulting state hash against a
    /// fresh simulation run.  Returns true if hashes match.
    bool VerifySaveLoadDeterminism(const std::string& tmpPath, uint32_t extraTicks);

    bool Running() const;
    void Shutdown();

    /// Request the engine to exit at the end of the current frame.
    void RequestExit();

    bool Can(Capability cap) const;

    const EngineConfig& Config() const { return m_config; }

    ecs::World& GetWorld() { return m_world; }
    net::NetContext& GetNet() { return m_net; }
    sim::TickScheduler& GetScheduler() { return m_scheduler; }
    sim::TimeModel& GetTimeModel() { return m_timeModel; }
    sim::WorldState& GetWorldState() { return m_worldState; }
    sim::SaveSystem& GetSaveSystem() { return m_saveSystem; }
    ui::UIManager& GetUIManager() { return m_uiManager; }
    ui::UIEventRouter& GetEventRouter() { return m_eventRouter; }

    platform::PlatformWindow* GetWindow() { return m_window.get(); }
    ui::UIRenderer* GetRenderer() { return m_renderer.get(); }
    render::EditorViewportFramebuffer* GetViewportFramebuffer() { return m_viewportFB.get(); }

    int32_t MouseX() const { return m_mouseX; }
    int32_t MouseY() const { return m_mouseY; }

    /// Returns the names of systems in their registered execution order.
    const std::vector<std::string>& SystemExecutionOrder() const;

    /// Register a named simulation system for execution-order tracking.
    void RegisterSystem(const std::string& name);

    /// Set a callback invoked each tick with the delta time.
    void SetFrameCallback(std::function<void(float)> cb);

    /// Returns the number of ticks executed so far.
    uint64_t TickCount() const { return m_tickCount; }

private:
    void ProcessWindowEvents();
    void PerformAutosaveIfNeeded(uint64_t tickCount);

    EngineConfig m_config;
    bool m_running = false;
    ecs::World m_world;
    net::NetContext m_net;
    sim::TickScheduler m_scheduler;
    sim::TimeModel m_timeModel;
    sim::WorldState m_worldState;
    sim::SaveSystem m_saveSystem;
    ui::UIManager m_uiManager;
    ui::UIEventRouter m_eventRouter;
    std::unique_ptr<platform::PlatformWindow> m_window;
    std::unique_ptr<ui::UIRenderer> m_renderer;
    std::unique_ptr<render::EditorViewportFramebuffer> m_viewportFB;
    std::vector<std::string> m_systemOrder;
    std::function<void(float)> m_frameCallback;
    uint64_t m_tickCount = 0;
    int32_t m_mouseX = 0;
    int32_t m_mouseY = 0;
};

}
