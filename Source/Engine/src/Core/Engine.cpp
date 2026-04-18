#include "Engine.h"
#include "Logger.h"
#include "../render/VulkanRenderer.h"
#include "../sim/StateHasher.h"
#include "../sim/ReplayRecorder.h"
#ifdef ATLAS_INCLUDE_TOOLS
#include "../ui/DiagnosticsOverlay.h"
#endif

#ifdef ATLAS_HAS_X11
#include "../platform/X11Window.h"
#endif

#ifdef ATLAS_HAS_WIN32
#include "../platform/Win32Window.h"
#endif

#if !defined(__linux__) || defined(ATLAS_HAS_X11)
#include "../render/GLRenderer.h"
#include "../render/GLViewportFramebuffer.h"
#endif

namespace atlas {

Engine::Engine(const EngineConfig& cfg) : m_config(cfg) {
}

Engine::~Engine() {
    Shutdown();
}

void Engine::InitCore() {
    Logger::Init();
    Logger::Info("Engine core initialized");
    m_running = true;
    RegisterSystem("Core");
}

void Engine::InitRender() {
    if (m_config.mode == EngineMode::Server) {
        Logger::Info("Server mode: rendering disabled");
        return;
    }

    if (m_config.headless) {
        Logger::Info("Headless mode: rendering disabled");
        return;
    }

#if defined(ATLAS_HAS_X11) || defined(ATLAS_HAS_WIN32)
    {
#if defined(ATLAS_HAS_X11)
        auto window = std::make_unique<platform::X11Window>();
#elif defined(ATLAS_HAS_WIN32)
        auto window = std::make_unique<platform::Win32Window>();
#endif

        platform::PlatformWindowConfig winCfg;
        switch (m_config.mode) {
            case EngineMode::Editor: winCfg.title = "Atlas Editor"; break;
            case EngineMode::Client: winCfg.title = "Atlas Client"; break;
            default: winCfg.title = "Atlas Engine"; break;
        }
        winCfg.width = m_config.windowWidth;
        winCfg.height = m_config.windowHeight;
        winCfg.resizable = true;

        if (!window->Init(winCfg)) {
            Logger::Error("Failed to create platform window");
            return;
        }

        m_window = std::move(window);
    }
#else
    Logger::Error("No platform window implementation for this OS");
    return;
#endif

    if (m_config.renderAPI == render::RenderAPI::OpenGL) {
#if !defined(__linux__) || defined(ATLAS_HAS_X11)
        auto gl = std::make_unique<render::GLRenderer>();
        gl->SetViewport(m_config.windowWidth, m_config.windowHeight);
        m_renderer = std::move(gl);
        Logger::Info("OpenGL renderer initialized");
#else
        Logger::Error("OpenGL renderer not available (no GL support)");
        return;
#endif
    } else if (m_config.renderAPI == render::RenderAPI::Vulkan) {
        auto vk = std::make_unique<render::VulkanRenderer>();
        vk->SetViewport(m_config.windowWidth, m_config.windowHeight);
        m_renderer = std::move(vk);
        Logger::Info("Vulkan renderer initialized (stub)");
    }

    Logger::Info("Render system initialized");
}

void Engine::InitUI() {
    ui::GUIContext guiCtx;
    switch (m_config.mode) {
        case EngineMode::Editor: guiCtx = ui::GUIContext::Editor; break;
        case EngineMode::Client: guiCtx = ui::GUIContext::Game;   break;
        case EngineMode::Server: guiCtx = ui::GUIContext::Server; break;
    }
    m_uiManager.Init(guiCtx);
    if (m_renderer) {
        m_uiManager.SetRenderer(m_renderer.get());
    }
    Logger::Info("UI system initialized");
}

void Engine::InitECS() {
    Logger::Info("ECS initialized (empty world)");
    RegisterSystem("ECS");
}

void Engine::InitNetworking() {
    net::NetMode netMode = net::NetMode::Standalone;
    if (m_config.mode == EngineMode::Server) {
        netMode = net::NetMode::Server;
    } else if (m_config.mode == EngineMode::Client) {
        netMode = net::NetMode::Client;
    }
    m_net.Init(netMode);
    Logger::Info("Networking initialized");
    RegisterSystem("Networking");
}

void Engine::InitEditor() {
    if (m_config.mode != EngineMode::Editor) {
        return;
    }

    // Create the editor viewport framebuffer for offscreen scene rendering.
    // This fixes the GUI issue where the scene would render directly to the
    // swapchain backbuffer (appearing behind the editor UI) instead of into
    // the viewport panel.  See gui_issues.txt for the full diagnosis.
    //
    // When an OpenGL context is available, use a real FBO so the scene can
    // be rendered into a texture and displayed inside the viewport panel.
    // Otherwise fall back to the Null implementation for headless / CI builds.
#if !defined(__linux__) || defined(ATLAS_HAS_X11)
    if (m_renderer && m_config.renderAPI == render::RenderAPI::OpenGL) {
        auto glFB = std::make_unique<render::GLViewportFramebuffer>();
        if (glFB->Create(static_cast<uint32_t>(m_config.windowWidth),
                         static_cast<uint32_t>(m_config.windowHeight))) {
            m_viewportFB = std::move(glFB);
        } else {
            m_viewportFB = std::make_unique<render::NullViewportFramebuffer>(
                static_cast<uint32_t>(m_config.windowWidth),
                static_cast<uint32_t>(m_config.windowHeight));
        }
    } else
#endif
    {
        m_viewportFB = std::make_unique<render::NullViewportFramebuffer>(
            static_cast<uint32_t>(m_config.windowWidth),
            static_cast<uint32_t>(m_config.windowHeight));
    }

    // Set initial viewport size so the UI layout knows its starting dimensions
    m_uiManager.SetViewportSize(
        static_cast<float>(m_config.windowWidth),
        static_cast<float>(m_config.windowHeight));

    Logger::Info("Editor tools initialized");
}

void Engine::Run() {
    m_scheduler.SetTickRate(m_config.tickRate);
    m_scheduler.LockTickRate();
    m_timeModel.SetTickRate(m_config.tickRate);

    switch (m_config.mode) {
        case EngineMode::Editor: RunEditor(); break;
        case EngineMode::Client: RunClient(); break;
        case EngineMode::Server: RunServer(); break;
    }
}

void Engine::PerformAutosaveIfNeeded(uint64_t tickCount) {
    if (m_config.autosaveInterval > 0 && tickCount % m_config.autosaveInterval == 0) {
        auto ecsData = m_world.Serialize();
        m_saveSystem.Save(m_config.autosavePath,
                          m_timeModel.Context().sim.tick,
                          m_config.tickRate, 0, ecsData);
    }
}

void Engine::ProcessWindowEvents() {
    if (!m_window) return;

    platform::WindowEvent event;
    while (m_window->PollEvent(event)) {
        switch (event.type) {
            case platform::WindowEvent::Type::Close:
                m_running = false;
                break;
            case platform::WindowEvent::Type::Resize:
                if (m_renderer) {
#if !defined(__linux__) || defined(ATLAS_HAS_X11)
                    if (m_config.renderAPI == render::RenderAPI::OpenGL) {
                        static_cast<render::GLRenderer*>(m_renderer.get())
                            ->SetViewport(event.width, event.height);
                    } else {
                        static_cast<render::VulkanRenderer*>(m_renderer.get())
                            ->SetViewport(event.width, event.height);
                    }
#else
                    static_cast<render::VulkanRenderer*>(m_renderer.get())
                        ->SetViewport(event.width, event.height);
#endif
                }
                // Resize the editor viewport framebuffer to match
                if (m_viewportFB && event.width > 0 && event.height > 0) {
                    m_viewportFB->Resize(
                        static_cast<uint32_t>(event.width),
                        static_cast<uint32_t>(event.height));
                }
                // Propagate viewport size to the UI so the widget layout scales
                if (event.width > 0 && event.height > 0) {
                    m_uiManager.SetViewportSize(
                        static_cast<float>(event.width),
                        static_cast<float>(event.height));
                }
                break;
            case platform::WindowEvent::Type::KeyDown: {
#ifdef ATLAS_INCLUDE_TOOLS
                // Ctrl+Backtick or F3 toggles diagnostics overlay
                if ((event.keyCode == '`' && (event.modifiers & platform::kModCtrl)) ||
                    event.keyCode == platform::kKeyF3) {
                    ui::DiagnosticsOverlay::Toggle();
                }
#endif
                ui::UIEvent uiEvent;
                uiEvent.type = ui::UIEvent::Type::KeyDown;
                uiEvent.keyCode = event.keyCode;
                uiEvent.textChar = event.textChar;
                uiEvent.modifiers = event.modifiers;
                m_uiManager.DispatchEvent(uiEvent);
                m_eventRouter.Dispatch(uiEvent);
                // Synthesize a TextInput event for printable ASCII characters.
                // X11 only produces KeyPress (no separate TextInput), so
                // this ensures InputFieldManager receives character data.
                // Win32's KeyDown does not set textChar (WM_CHAR handles
                // that separately), so this is effectively a no-op there.
                // Non-ASCII characters (> 0x7F) are not handled here; full
                // Unicode/IME support would require XIM integration.
                // 0x20 = space (first printable ASCII), 0x7F = DEL control char.
                if (event.textChar >= 0x20 && event.textChar != 0x7F) {
                    ui::UIEvent textEvent;
                    textEvent.type = ui::UIEvent::Type::TextInput;
                    textEvent.textChar = event.textChar;
                    m_uiManager.DispatchEvent(textEvent);
                    m_eventRouter.Dispatch(textEvent);
                }
                break;
            }
            case platform::WindowEvent::Type::KeyUp: {
                ui::UIEvent uiEvent;
                uiEvent.type = ui::UIEvent::Type::KeyUp;
                uiEvent.keyCode = event.keyCode;
                uiEvent.modifiers = event.modifiers;
                m_uiManager.DispatchEvent(uiEvent);
                m_eventRouter.Dispatch(uiEvent);
                break;
            }
            case platform::WindowEvent::Type::MouseMove: {
                m_mouseX = event.mouseX;
                m_mouseY = event.mouseY;
                ui::UIEvent uiEvent;
                uiEvent.type = ui::UIEvent::Type::MouseMove;
                uiEvent.x = event.mouseX;
                uiEvent.y = event.mouseY;
                m_uiManager.DispatchEvent(uiEvent);
                m_eventRouter.Dispatch(uiEvent);
                break;
            }
            case platform::WindowEvent::Type::MouseButtonDown: {
                m_mouseX = event.mouseX;
                m_mouseY = event.mouseY;
                ui::UIEvent uiEvent;
                uiEvent.type = ui::UIEvent::Type::MouseDown;
                uiEvent.x = event.mouseX;
                uiEvent.y = event.mouseY;
                uiEvent.mouseButton = event.mouseButton;
                m_uiManager.DispatchEvent(uiEvent);
                m_eventRouter.Dispatch(uiEvent);
                // Also enqueue on UI command bus for backward compat
                ui::UICommand cmd;
                cmd.type = ui::UICommandType::ButtonPress;
                cmd.valueFloat = static_cast<float>(event.mouseX);
                cmd.valueString = std::to_string(event.mouseY);
                m_uiManager.GetCommandBus().Enqueue(std::move(cmd));
                break;
            }
            case platform::WindowEvent::Type::MouseButtonUp: {
                m_mouseX = event.mouseX;
                m_mouseY = event.mouseY;
                ui::UIEvent uiEvent;
                uiEvent.type = ui::UIEvent::Type::MouseUp;
                uiEvent.x = event.mouseX;
                uiEvent.y = event.mouseY;
                uiEvent.mouseButton = event.mouseButton;
                m_uiManager.DispatchEvent(uiEvent);
                m_eventRouter.Dispatch(uiEvent);
                break;
            }
            case platform::WindowEvent::Type::TextInput: {
                ui::UIEvent uiEvent;
                uiEvent.type = ui::UIEvent::Type::TextInput;
                uiEvent.textChar = event.textChar;
                m_uiManager.DispatchEvent(uiEvent);
                m_eventRouter.Dispatch(uiEvent);
                break;
            }
            case platform::WindowEvent::Type::ScrollWheel: {
                m_mouseX = event.mouseX;
                m_mouseY = event.mouseY;
                ui::UIEvent uiEvent;
                uiEvent.type = ui::UIEvent::Type::ScrollWheel;
                uiEvent.x = event.mouseX;
                uiEvent.y = event.mouseY;
                uiEvent.scrollDelta = event.scrollDelta;
                m_uiManager.DispatchEvent(uiEvent);
                m_eventRouter.Dispatch(uiEvent);
                break;
            }
            default:
                break;
        }
    }
}

void Engine::RunEditor() {
    Logger::Info("Running Atlas Editor");
    m_tickCount = 0;
    while (m_running) {
        ProcessWindowEvents();
        m_net.Poll();
        m_scheduler.Tick([this](float dt) {
            m_timeModel.AdvanceTick();
            const auto& timeCtx = m_timeModel.Context();
            m_world.Update(timeCtx.sim.fixedDeltaTime);
            ui::UIContext uiCtx{};
            if (m_window) {
                uiCtx.screenWidth = static_cast<float>(m_window->Width());
                uiCtx.screenHeight = static_cast<float>(m_window->Height());
            }
            uiCtx.deltaTime = timeCtx.sim.fixedDeltaTime;
            uiCtx.tick = static_cast<uint32_t>(timeCtx.sim.tick);
            m_uiManager.Update(uiCtx);
            m_uiManager.GetTooltipManager().Update(m_mouseX, m_mouseY,
                                                    timeCtx.sim.fixedDeltaTime);
            if (m_frameCallback) m_frameCallback(dt);
        });

        if (m_renderer && m_window && m_window->IsOpen()) {
            // -------------------------------------------------------
            // PASS 1: Render scene into the viewport framebuffer
            // -------------------------------------------------------
            // This fixes the root cause identified in gui_issues.txt:
            // the scene was previously rendered directly to the swapchain
            // backbuffer, appearing behind the editor UI.  By rendering
            // into an offscreen framebuffer first, the viewport panel can
            // display the scene as a textured quad.
            if (m_viewportFB && m_viewportFB->IsValid()) {
                m_viewportFB->Bind();
                // Scene rendering would go here (world, editor camera, etc.)
                m_viewportFB->Unbind();
            }

            // -------------------------------------------------------
            // PASS 2: Render editor UI to the swapchain backbuffer
            // -------------------------------------------------------
            m_renderer->BeginFrame();
            m_uiManager.Render(m_renderer.get());
            ui::UIContext overlayCtx{};
            if (m_window) {
                overlayCtx.screenWidth = static_cast<float>(m_window->Width());
                overlayCtx.screenHeight = static_cast<float>(m_window->Height());
            }
            overlayCtx.deltaTime = m_timeModel.Context().sim.fixedDeltaTime;
            overlayCtx.tick = static_cast<uint32_t>(m_timeModel.Context().sim.tick);
#ifdef ATLAS_INCLUDE_TOOLS
            ui::DiagnosticsOverlay::Render(m_renderer.get(), overlayCtx, 1.0f,
                                           m_mouseX, m_mouseY);
#endif
            m_renderer->EndFrame();
            m_window->SwapBuffers();
        }

        m_tickCount++;
        if (m_config.maxTicks > 0 && m_tickCount >= m_config.maxTicks) {
            m_running = false;
        }
    }
}

void Engine::RunClient() {
    Logger::Info("Running Atlas Client");
    m_tickCount = 0;
    while (m_running) {
        ProcessWindowEvents();
        m_net.Poll();
        m_scheduler.Tick([this](float dt) {
            m_timeModel.AdvanceTick();
            const auto& timeCtx = m_timeModel.Context();
            m_world.Update(timeCtx.sim.fixedDeltaTime);
            ui::UIContext uiCtx{};
            if (m_window) {
                uiCtx.screenWidth = static_cast<float>(m_window->Width());
                uiCtx.screenHeight = static_cast<float>(m_window->Height());
            }
            uiCtx.deltaTime = timeCtx.sim.fixedDeltaTime;
            uiCtx.tick = static_cast<uint32_t>(timeCtx.sim.tick);
            m_uiManager.Update(uiCtx);
            m_uiManager.GetTooltipManager().Update(m_mouseX, m_mouseY,
                                                    timeCtx.sim.fixedDeltaTime);
            if (m_frameCallback) m_frameCallback(dt);
        });

        if (m_renderer && m_window && m_window->IsOpen()) {
            m_renderer->BeginFrame();
            m_uiManager.Render(m_renderer.get());
            ui::UIContext overlayCtx{};
            if (m_window) {
                overlayCtx.screenWidth = static_cast<float>(m_window->Width());
                overlayCtx.screenHeight = static_cast<float>(m_window->Height());
            }
            overlayCtx.deltaTime = m_timeModel.Context().sim.fixedDeltaTime;
            overlayCtx.tick = static_cast<uint32_t>(m_timeModel.Context().sim.tick);
#ifdef ATLAS_INCLUDE_TOOLS
            ui::DiagnosticsOverlay::Render(m_renderer.get(), overlayCtx, 1.0f,
                                           m_mouseX, m_mouseY);
#endif
            m_renderer->EndFrame();
            m_window->SwapBuffers();
        }

        m_tickCount++;

        PerformAutosaveIfNeeded(m_tickCount);

        if (m_config.maxTicks > 0 && m_tickCount >= m_config.maxTicks) {
            m_running = false;
        }
    }
}

void Engine::RunServer() {
    Logger::Info("Running Atlas Server");
    m_tickCount = 0;
    while (m_running) {
        m_net.Poll();
        m_scheduler.Tick([this](float dt) {
            m_timeModel.AdvanceTick();
            const auto& timeCtx = m_timeModel.Context();
            m_world.Update(timeCtx.sim.fixedDeltaTime);

            // Periodically snapshot world state for rollback support.
            // Snapshot every tick so the server can roll back as needed.
            auto ecsData = m_world.Serialize();
            auto snapshot = m_worldState.TakeSnapshot(timeCtx.sim.tick, ecsData);
            m_worldState.PushSnapshot(std::move(snapshot));
            if (m_frameCallback) m_frameCallback(dt);
        });
        m_net.Flush();

        m_tickCount++;

        PerformAutosaveIfNeeded(m_tickCount);

        if (m_config.maxTicks > 0 && m_tickCount >= m_config.maxTicks) {
            m_running = false;
        }
    }
}

bool Engine::RollbackToTick(uint64_t tick) {
    const auto* snapshot = m_worldState.SnapshotAtTick(tick);
    if (!snapshot) return false;

    if (!m_world.Deserialize(snapshot->ecsData)) return false;

    m_timeModel.SetTick(tick);
    return true;
}

bool Engine::RollbackAndVerify(uint64_t snapshotTick, uint64_t targetTick) {
    if (snapshotTick >= targetTick) return false;

    // Capture the target snapshot hash before rollback.
    const auto* targetSnap = m_worldState.SnapshotAtTick(targetTick);
    if (!targetSnap) return false;
    uint64_t expectedHash = targetSnap->stateHash;

    // Rollback to the earlier snapshot.
    if (!RollbackToTick(snapshotTick)) return false;

    // Resimulate forward from snapshotTick to targetTick.
    m_scheduler.SetFramePacing(false);
    while (m_timeModel.Context().sim.tick < targetTick) {
        m_timeModel.AdvanceTick();
        m_world.Update(m_timeModel.Context().sim.fixedDeltaTime);
    }

    // Take a fresh snapshot and compare hashes.
    auto ecsData = m_world.Serialize();
    auto replaySnap = m_worldState.TakeSnapshot(targetTick, ecsData);

    return replaySnap.stateHash == expectedHash;
}

bool Engine::LoadAndReplay(const std::string& savePath) {
    auto result = m_saveSystem.Load(savePath);
    if (result != sim::SaveResult::Success) {
        return false;
    }

    if (!m_world.Deserialize(m_saveSystem.ECSData())) {
        return false;
    }

    m_timeModel.SetTick(m_saveSystem.Header().saveTick);
    return true;
}

bool Engine::ReplayFromSave(const std::string& savePath, const std::string& replayPath) {
    // Step 1: Load save file to restore world state
    auto result = m_saveSystem.Load(savePath);
    if (result != sim::SaveResult::Success) {
        return false;
    }

    if (!m_world.Deserialize(m_saveSystem.ECSData())) {
        return false;
    }

    uint64_t saveTick = m_saveSystem.Header().saveTick;
    m_timeModel.SetTick(saveTick);

    // Step 2: Load replay file
    sim::ReplayRecorder replay;
    if (!replay.LoadReplay(replayPath)) {
        return false;
    }

    // Step 3: Apply replay inputs from save tick forward
    m_scheduler.SetTickRate(m_config.tickRate);
    m_scheduler.SetFramePacing(false);

    uint64_t appliedFrames = 0;
    for (const auto& frame : replay.Frames()) {
        if (frame.tick <= saveTick) continue;  // Skip frames before/at save point

        m_timeModel.AdvanceTick();
        m_world.Update(m_timeModel.Context().sim.fixedDeltaTime);
        ++appliedFrames;
    }

    return appliedFrames > 0;
}

bool Engine::VerifySaveLoadDeterminism(const std::string& tmpPath, uint32_t extraTicks) {
    // Step 1: Snapshot current state.
    uint64_t currentTick = m_timeModel.Context().sim.tick;
    auto ecsDataBefore = m_world.Serialize();
    auto snapBefore = m_worldState.TakeSnapshot(currentTick, ecsDataBefore);

    // Step 2: Simulate extra ticks and record the resulting hash.
    m_scheduler.SetFramePacing(false);
    for (uint32_t i = 0; i < extraTicks; ++i) {
        m_timeModel.AdvanceTick();
        m_world.Update(m_timeModel.Context().sim.fixedDeltaTime);
    }
    auto ecsDataAfter = m_world.Serialize();
    uint64_t targetTick = m_timeModel.Context().sim.tick;
    auto snapAfter = m_worldState.TakeSnapshot(targetTick, ecsDataAfter);
    uint64_t expectedHash = snapAfter.stateHash;

    // Step 3: Save the state we captured at currentTick.
    auto saveResult = m_saveSystem.Save(tmpPath, currentTick,
                                         m_config.tickRate, 0, ecsDataBefore);
    if (saveResult != sim::SaveResult::Success) return false;

    // Step 4: Load the save back and restore world state.
    auto loadResult = m_saveSystem.Load(tmpPath);
    if (loadResult != sim::SaveResult::Success) return false;

    if (!m_world.Deserialize(m_saveSystem.ECSData())) return false;
    m_timeModel.SetTick(currentTick);

    // Step 5: Resimulate the same extra ticks.
    for (uint32_t i = 0; i < extraTicks; ++i) {
        m_timeModel.AdvanceTick();
        m_world.Update(m_timeModel.Context().sim.fixedDeltaTime);
    }

    // Step 6: Compare hashes.
    auto ecsDataReplay = m_world.Serialize();
    auto snapReplay = m_worldState.TakeSnapshot(targetTick, ecsDataReplay);

    return snapReplay.stateHash == expectedHash;
}

bool Engine::Running() const {
    return m_running;
}

void Engine::RequestExit() {
    m_running = false;
}

void Engine::Shutdown() {
    if (m_running) {
        Logger::Info("Engine shutting down");
        m_uiManager.Shutdown();
        m_net.Shutdown();
        m_renderer.reset();
        if (m_window) {
            m_window->Shutdown();
            m_window.reset();
        }
        m_running = false;
        Logger::Shutdown();
    }
}

bool Engine::Can(Capability cap) const {
    switch (cap) {
        case Capability::AssetWrite:
            return m_config.mode == EngineMode::Editor;
        case Capability::Rendering:
            return m_config.mode != EngineMode::Server;
        case Capability::Physics:
            return true;
        case Capability::GraphEdit:
            return m_config.mode == EngineMode::Editor;
        case Capability::GraphExecute:
            return true;
        case Capability::NetAuthority:
            return m_config.mode == EngineMode::Server;
        case Capability::HotReload:
            return m_config.mode == EngineMode::Editor;
    }
    return false;
}

const std::vector<std::string>& Engine::SystemExecutionOrder() const {
    return m_systemOrder;
}

void Engine::RegisterSystem(const std::string& name) {
    m_systemOrder.push_back(name);
}

void Engine::SetFrameCallback(std::function<void(float)> cb) {
    m_frameCallback = std::move(cb);
}

}
