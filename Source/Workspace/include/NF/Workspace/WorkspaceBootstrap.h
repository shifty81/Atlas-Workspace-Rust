#pragma once
// NF::WorkspaceBootstrap — Typed workspace startup configuration and bootstrap sequence.
//
// WorkspaceBootstrap encapsulates everything needed to bring a WorkspaceShell
// from zero to Ready in a single, testable step. It separates the WHAT (config)
// from the HOW (the shell lifecycle) and from the platform layer (window/backend).
//
// Usage (typical headless test / CI):
//
//   WorkspaceBootstrapConfig cfg;
//   cfg.launchMode = WorkspaceLaunchMode::Headless;
//   cfg.windowConfig = {1280, 800, "Atlas Workspace"};
//   cfg.toolFactories.push_back([...]{ return make_unique<MyTool>(); });
//
//   WorkspaceBootstrap boot;
//   auto result = boot.run(cfg, shell);
//   assert(result.succeeded());
//
// The bootstrap does NOT own a window or backend — those remain platform-specific
// and are injected separately by the executable entrypoint (main.cpp / WinMain).
//
// Architecture:
//   WorkspaceBootstrapConfig   — all startup knobs in one place
//   WorkspaceWindowConfig      — window size, title, fullscreen toggle
//   WorkspaceBackendChoice     — which UI backend to prefer
//   WorkspaceBootstrapResult   — typed result with error catalog
//   WorkspaceBootstrap         — stateless runner; applies config to shell

#include "NF/Workspace/WorkspaceShell.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Launch Mode ────────────────────────────────────────────────────
// Mirrors WorkspaceLaunchMode from WorkspaceLaunchContract.h but lives here
// so WorkspaceBootstrap has no dependency on the launch service itself.

enum class WorkspaceStartupMode : uint8_t {
    Hosted,     // full UI, real window, user-facing
    Headless,   // no window, CI / server / test
    Preview,    // lightweight in-editor preview loop
};

inline const char* workspaceStartupModeName(WorkspaceStartupMode m) {
    switch (m) {
    case WorkspaceStartupMode::Hosted:   return "Hosted";
    case WorkspaceStartupMode::Headless: return "Headless";
    case WorkspaceStartupMode::Preview:  return "Preview";
    }
    return "Unknown";
}

// ── Window Config ──────────────────────────────────────────────────

struct WorkspaceWindowConfig {
    int         width      = 1280;
    int         height     = 800;
    std::string title      = "Atlas Workspace";
    bool        fullscreen = false;
    bool        resizable  = true;

    [[nodiscard]] bool isValid() const {
        return width > 0 && height > 0 && !title.empty();
    }
    [[nodiscard]] float aspectRatio() const {
        return height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.f;
    }
};

// ── Backend Choice ─────────────────────────────────────────────────

enum class WorkspaceBackendChoice : uint8_t {
    Auto,       // selector chooses best available
    D3D11,      // prefer Direct3D 11
    OpenGL,     // prefer OpenGL (compat)
    GDI,        // GDI fallback (Win32 only)
    Null,       // no-op backend (headless / testing)
};

inline const char* workspaceBackendChoiceName(WorkspaceBackendChoice c) {
    switch (c) {
    case WorkspaceBackendChoice::Auto:   return "Auto";
    case WorkspaceBackendChoice::D3D11:  return "D3D11";
    case WorkspaceBackendChoice::OpenGL: return "OpenGL";
    case WorkspaceBackendChoice::GDI:    return "GDI";
    case WorkspaceBackendChoice::Null:   return "Null";
    }
    return "Unknown";
}

// ── Bootstrap Config ───────────────────────────────────────────────

struct WorkspaceBootstrapConfig {
    WorkspaceStartupMode   launchMode    = WorkspaceStartupMode::Hosted;
    WorkspaceWindowConfig  windowConfig;
    WorkspaceBackendChoice backendChoice = WorkspaceBackendChoice::Auto;

    // Optional tool factories — called during shell.initialize()
    std::vector<ToolFactory> toolFactories;

    // Optional startup notifications to post after Ready
    std::vector<std::string> startupMessages;

    // If true, WorkspaceBootstrap will call registerDefaultTools()
    // (i.e. registerCoreTools if available) as part of the sequence.
    bool registerDefaultTools = false;

    [[nodiscard]] bool isValid() const {
        if (launchMode == WorkspaceStartupMode::Hosted)
            return windowConfig.isValid();
        return true; // headless/preview don't need a window
    }
};

// ── Bootstrap Error ────────────────────────────────────────────────

enum class WorkspaceBootstrapError : uint8_t {
    None,
    InvalidConfig,      // config.isValid() returned false
    AlreadyInitialized, // shell is not in Created phase
    ShellInitFailed,    // WorkspaceShell::initialize() returned false
    ToolRegistrationFailed, // a factory threw or returned nullptr
};

inline const char* workspaceBootstrapErrorName(WorkspaceBootstrapError e) {
    switch (e) {
    case WorkspaceBootstrapError::None:                 return "None";
    case WorkspaceBootstrapError::InvalidConfig:        return "InvalidConfig";
    case WorkspaceBootstrapError::AlreadyInitialized:   return "AlreadyInitialized";
    case WorkspaceBootstrapError::ShellInitFailed:      return "ShellInitFailed";
    case WorkspaceBootstrapError::ToolRegistrationFailed: return "ToolRegistrationFailed";
    }
    return "Unknown";
}

// ── Bootstrap Result ───────────────────────────────────────────────

struct WorkspaceBootstrapResult {
    WorkspaceBootstrapError error          = WorkspaceBootstrapError::None;
    std::string             errorDetail;
    size_t                  toolsRegistered = 0;

    [[nodiscard]] bool succeeded() const {
        return error == WorkspaceBootstrapError::None;
    }
    [[nodiscard]] bool failed() const { return !succeeded(); }
    [[nodiscard]] const char* errorName() const {
        return workspaceBootstrapErrorName(error);
    }
};

// ── WorkspaceBootstrap ─────────────────────────────────────────────
// Stateless bootstrap runner. Applies a config to a shell.
// Does not own the shell — the caller retains ownership.
//
// Thread-safety: single-threaded only (same as WorkspaceShell).

class WorkspaceBootstrap {
public:
    // Run the bootstrap sequence on |shell| using |cfg|.
    // Returns a result describing success or the first failure.
    [[nodiscard]] WorkspaceBootstrapResult run(WorkspaceBootstrapConfig cfg,
                                               WorkspaceShell&          shell)
    {
        WorkspaceBootstrapResult result;

        // 1. Validate config
        if (!cfg.isValid()) {
            result.error       = WorkspaceBootstrapError::InvalidConfig;
            result.errorDetail = "WorkspaceBootstrapConfig::isValid() failed";
            return result;
        }

        // 2. Check shell precondition
        if (shell.phase() != ShellPhase::Created) {
            result.error       = WorkspaceBootstrapError::AlreadyInitialized;
            result.errorDetail = std::string("Shell is in phase: ")
                               + shellPhaseName(shell.phase());
            return result;
        }

        // 3. Register tool factories from config
        for (auto& factory : cfg.toolFactories) {
            if (!factory) {
                result.error       = WorkspaceBootstrapError::ToolRegistrationFailed;
                result.errorDetail = "Null tool factory in bootstrap config";
                return result;
            }
            shell.registerToolFactory(std::move(factory));
        }
        cfg.toolFactories.clear();

        // 4. Initialize the shell (this also invokes registered factories)
        if (!shell.initialize()) {
            result.error       = WorkspaceBootstrapError::ShellInitFailed;
            result.errorDetail = "WorkspaceShell::initialize() returned false";
            return result;
        }

        result.toolsRegistered = shell.toolRegistry().count();

        // 5. Post startup notifications
        for (const auto& msg : cfg.startupMessages) {
            Notification n;
            n.title    = "Bootstrap";
            n.message  = msg;
            n.severity = NotificationSeverity::Info;
            shell.shellContract().postNotification(n);
        }

        m_lastConfig = std::move(cfg);
        m_runCount++;
        return result;
    }

    [[nodiscard]] size_t              runCount()   const { return m_runCount;   }
    [[nodiscard]] const WorkspaceBootstrapConfig& lastConfig() const { return m_lastConfig; }

private:
    WorkspaceBootstrapConfig m_lastConfig;
    size_t                   m_runCount = 0;
};

} // namespace NF
