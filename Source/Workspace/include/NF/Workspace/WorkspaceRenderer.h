#pragma once
// NF::WorkspaceRenderer — Workspace shell chrome renderer.
//
// Renders the workspace-level UI (title bar, tool launcher sidebar, project
// dashboard, and status bar) into a UIRenderer (GDI-backed on Windows).
//
// This is the WORKSPACE outer shell UI — NOT the game editor surface.
// The workspace lists and launches hosted tools (NovaForgeEditor, NovaForgeGame,
// NovaForgeServer, …) as child processes via WorkspaceLaunchService.
//
// WorkspaceRenderer maintains a UIContext for interactive hit regions.
// Mouse state (UIMouseState) must be supplied every frame so that hover
// highlights and click detection work correctly.
//
// ── Deferred action pattern ───────────────────────────────────────────────
// Clicks detected inside render() (which runs in WM_PAINT on Windows) must
// not open modal dialogs directly — doing so creates a nested message loop
// inside WM_PAINT and can deadlock or corrupt paint state.
//
// Instead, render() stores a WorkspaceAction in m_pendingAction.  The main
// loop calls takePendingAction() after each WM_PAINT dispatch and opens the
// appropriate dialog safely outside the paint callback.
//
// Similarly, launch errors queue a WorkspacePendingError so MessageBoxW can
// be shown from the main loop rather than from inside WM_PAINT.
//
// Usage (main loop):
//
//   NF::WorkspaceRenderer wsRenderer;
//   NF::NullLaunchService launchSvc;
//   while (running) {
//       ui.beginFrame(w, h);          // called internally by render()
//       wsRenderer.render(ui, w, h, shell, mouse, &launchSvc);
//       // ui.endFrame() is called by render() before returning
//       // After message dispatch, handle deferred actions:
//       auto action = wsRenderer.takePendingAction();   // WorkspaceAction
//       auto err    = wsRenderer.takePendingError();    // WorkspacePendingError
//   }
//
// Color format: RRGGBBAA (matches UIRenderer / GDIBackend convention).

#include "NF/UI/UI.h"
#include "NF/UI/UIWidgets.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/WorkspaceActivityBar.h"
#include "NF/Workspace/WorkspaceLaunchContract.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <array>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace NF {

// ── WorkspaceAction ───────────────────────────────────────────────
// UI actions queued by render() to be handled by the main loop outside
// WM_PAINT so that modal dialogs can be opened safely.

enum class WorkspaceAction : uint8_t {
    None,
    NewProject,     // open folder-browser dialog, create and load a new project
    OpenProject,    // open file-open dialog for .atlas files
    Exit,           // request application exit
};

// ── WorkspacePendingError ─────────────────────────────────────────
// A title + message pair queued by render() when a launch fails or
// another recoverable error occurs that the user should see.

struct WorkspacePendingError {
    std::string title;
    std::string message;
    [[nodiscard]] bool empty() const { return title.empty(); }
};

class WorkspaceRenderer {
public:
    WorkspaceRenderer() {
        initTheme();
        initMenus();
        // Register a Logger sink so every NF_LOG_* line feeds the System tab.
        m_logSinkId = Logger::instance().addSink(
            [this](LogLevel level, std::string_view category, std::string_view message) {
                // Build a display line matching the console window format.
                const char* lvlStr =
                    (level == LogLevel::Warn)  ? "WARN" :
                    (level == LogLevel::Error)  ? "ERR " :
                    (level == LogLevel::Fatal)  ? "FATL" : "INFO";
                std::string line = std::string("[") + lvlStr + "] [";
                line += category;
                line += "] ";
                line += message;
                std::lock_guard<std::mutex> lk(m_sysLogMutex);
                m_sysLog.push_back(std::move(line));
                if (m_sysLog.size() > kSysLogMax)
                    m_sysLog.pop_front();
            });
    }

    ~WorkspaceRenderer() {
        Logger::instance().removeSink(m_logSinkId);
    }

    // ── Layout constants (pixels) ─────────────────────────────────
    static constexpr float kTitleH   = 28.f;
    static constexpr float kToolbarH = 28.f;
    static constexpr float kStatusH  = 24.f;
    static constexpr float kSidebarW = 224.f;
    static constexpr float kContentY = kToolbarH;

    // ── Colors (RRGGBBAA) ─────────────────────────────────────────
    static constexpr uint32_t kBackground    = 0x1E1E1EFF;
    static constexpr uint32_t kSurface       = 0x252525FF;
    static constexpr uint32_t kBorder        = 0x3C3C3CFF;
    static constexpr uint32_t kAccentBlue    = 0x007ACCFF;
    static constexpr uint32_t kAccentDark    = 0x005A9EFF;
    static constexpr uint32_t kCardBg        = 0x2E2E2EFF;
    static constexpr uint32_t kButtonBg      = 0x3A3A3AFF;
    static constexpr uint32_t kTextPrimary   = 0xE0E0E0FF;
    static constexpr uint32_t kTextSecondary = 0x888888FF;
    static constexpr uint32_t kTextMuted     = 0x555555FF;
    static constexpr uint32_t kGreen         = 0x4EC94EFF;

    // ── render ────────────────────────────────────────────────────
    // Main entry point. Call once per frame from the platform render callback.
    // |mouse| must reflect the current frame's mouse position and button
    // edge-transitions (leftPressed / leftReleased) for clicks to register.
    // |launchSvc| is used to spawn child apps from the sidebar; pass nullptr
    // to disable launching (actions are still logged).
    void render(UIRenderer& ui, float width, float height,
                WorkspaceShell& shell, const UIMouseState& mouse,
                WorkspaceLaunchService* launchSvc = nullptr)
    {
        // While a dropdown is open, or for the single frame immediately after it
        // is dismissed, block click events from reaching the sidebar and main-area
        // so the dismissal click cannot also trigger an underlying element.
        bool suppressClicks = (m_openMenuIdx >= 0) || m_menuDismissed;
        m_menuDismissed = false;  // reset for this frame; renderDropdownOverlay will re-set it if a menu closes

        UIMouseState effectiveMouse = mouse;
        if (suppressClicks) {
            effectiveMouse.leftPressed  = false;
            effectiveMouse.leftReleased = false;
        }

        ui.beginFrame(width, height);
        renderBackground(ui, width, height);
        renderToolbar(ui, width, shell, mouse);            // real mouse — menu toggles must always work
        renderSidebar(ui, height, shell, effectiveMouse, launchSvc);
        renderMainArea(ui, width, height, shell, effectiveMouse);
        renderStatusBar(ui, width, height, shell);
        renderDropdownOverlay(ui, mouse, shell);           // real mouse — dropdown owns its own hit-testing
        ui.endFrame();
    }

    // ── Deferred action accessors ─────────────────────────────────
    // Call these from the main loop AFTER dispatching WM_PAINT.
    // Each call returns the pending value and resets it to its default.

    /// Returns any UI action queued during the last render() call and clears it.
    WorkspaceAction takePendingAction() {
        auto a = m_pendingAction;
        m_pendingAction = WorkspaceAction::None;
        return a;
    }

    /// Returns any error queued during the last render() call and clears it.
    WorkspacePendingError takePendingError() {
        auto e = m_pendingError;
        m_pendingError = {};
        return e;
    }

    /// Update the last frame time (milliseconds) shown in the status bar.
    /// Call this from the main loop with the delta time for each frame.
    void setLastFrameMs(float ms) { m_lastFrameMs = ms; }

    // ── Content area geometry ─────────────────────────────────────
    // Returns the bounding rect of the tool content area (right of sidebar,
    // below toolbar + header bar, above status bar + console strip).
    // Pass the current window dimensions and whether the console is visible.
    // Main.cpp uses this to position WorkspacePanelHost panels.
    [[nodiscard]] static Rect contentAreaBounds(float width, float height,
                                                    bool hasActiveTool,
                                                    bool consoleVisible) {
        static constexpr float kHeaderH = 38.f;
        static constexpr float kBottomH = 130.f;
        float topY = kToolbarH + (hasActiveTool ? kHeaderH : 0.f);
        float botH = hasActiveTool && consoleVisible ? kBottomH + kStatusH : kStatusH;
        return {kSidebarW, topY, width - kSidebarW, height - topY - botH};
    }

private:
    // ── Dropdown menu item ────────────────────────────────────────
    struct DropItem {
        std::string    label;                          // empty string = separator
        WorkspaceAction action  = WorkspaceAction::None;
        std::string    command;                        // command bus key (fallback)
        bool           enabled  = true;
    };
    struct DropMenu { std::vector<DropItem> items; };

    // ── Selection toggle helper ────────────────────────────────────
    // Selects |candidate| in |current|, or deselects if it is already selected.
    static void toggleSelection(int& current, int candidate) {
        current = (current == candidate) ? -1 : candidate;
    }

    // ── UITheme matching workspace dark palette ────────────────────
    void initTheme() {
        m_wsTheme.hoverHighlight    = 0x3D3D3DFF;  // subtle card hover tint
        m_wsTheme.panelBackground   = kSurface;
        m_wsTheme.panelBorder       = kBorder;
        m_wsTheme.panelHeader       = kSurface;
        m_wsTheme.panelText         = kTextPrimary;
        m_wsTheme.buttonBackground  = kCardBg;
        m_wsTheme.buttonHover       = kButtonBg;
        m_wsTheme.buttonPressed     = kAccentBlue;
        m_wsTheme.buttonText        = kTextPrimary;
        m_wsTheme.itemSpacing       = 0.f;
    }

    void initMenus() {
        // File menu
        m_dropMenus[0].items = {
            {"New Project",  WorkspaceAction::NewProject,  "", true},
            {"Open Project", WorkspaceAction::OpenProject, "", true},
            {"", WorkspaceAction::None, "", false},  // separator
            {"Exit",         WorkspaceAction::Exit,        "", true},
        };
        // Project menu
        m_dropMenus[1].items = {
            {"Close Project",    WorkspaceAction::None, "workspace.project.close",    true},
            {"", WorkspaceAction::None, "", false},  // separator
            {"Project Settings", WorkspaceAction::None, "workspace.project.settings", true},
        };
        // Tools menu
        m_dropMenus[2].items = {
            {"Preferences",     WorkspaceAction::None, "workspace.preferences",      true},
            {"Command Palette", WorkspaceAction::None, "workspace.command_palette",  true},
            {"", WorkspaceAction::None, "", false},  // separator
            {"Diagnostics",     WorkspaceAction::None, "workspace.diagnostics",      true},
        };
        // View menu
        m_dropMenus[3].items = {
            {"Content Browser", WorkspaceAction::None, "workspace.view.content_browser", true},
            {"Inspector",       WorkspaceAction::None, "workspace.view.inspector",        true},
            {"Outliner",        WorkspaceAction::None, "workspace.view.outliner",         true},
            {"Console",         WorkspaceAction::None, "workspace.view.console",          true},
        };
        // Help menu
        m_dropMenus[4].items = {
            {"Documentation", WorkspaceAction::None, "workspace.help.docs",  true},
            {"About",         WorkspaceAction::None, "workspace.help.about", true},
        };
    }

    // ── Background ────────────────────────────────────────────────
    void renderBackground(UIRenderer& ui, float w, float h) {
        ui.drawRect({0.f, 0.f, w, h}, kBackground);
    }

    // ── Toolbar ───────────────────────────────────────────────────
    void renderToolbar(UIRenderer& ui, float width, WorkspaceShell& shell,
                       const UIMouseState& mouse)
    {
        float y = 0.f;
        ui.drawRect({0.f, y, width, kToolbarH}, kSurface);
        // Bottom separator
        ui.drawRect({0.f, y + kToolbarH - 1.f, width, 1.f}, kBorder);
        // Left accent stripe
        ui.drawRect({0.f, y, 3.f, kToolbarH}, kAccentBlue);

        static const char* kMenuItems[] = {"File", "Project", "Tools", "View", "Help"};

        // Begin widget pass for this frame's toolbar hit regions
        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);

        float mx = 10.f;
        for (size_t i = 0; i < 5; ++i) {
            const char* item = kMenuItems[i];
            float iw = static_cast<float>(std::strlen(item)) * 8.f + 16.f;
            Rect btnR{mx, y + 4.f, iw, 20.f};

            // Determine visual state: open > hovered > normal.
            // Non-hovered items use the toolbar surface color so they blend
            // in; hovered items lift to kButtonBg; the open item uses the
            // accent color to show which menu is active.
            bool menuOpen = (m_openMenuIdx == static_cast<int>(i));
            bool hovered  = btnR.contains(mouse.x, mouse.y);
            uint32_t btnBg = menuOpen ? kAccentBlue
                           : (hovered  ? kButtonBg : kSurface);
            ui.drawRect(btnR, btnBg);
            ui.drawText(mx + 6.f, y + 7.f, item,
                        menuOpen ? 0xFFFFFFFF : kTextPrimary);

            // Click: toggle the corresponding dropdown.
            if (m_ctx.hitRegion(btnR, false)) {
                NF_LOG_INFO("WorkspaceUI", std::string("Menu clicked: ") + item);
                m_openMenuIdx = (m_openMenuIdx == static_cast<int>(i))
                                ? -1 : static_cast<int>(i);
            }

            mx += iw + 4.f;
        }

        m_ctx.end();

        // Phase indicator aligned to the right of the toolbar row
        std::string phase = std::string("[") + shellPhaseName(shell.phase()) + "]";
        float phaseX = width - static_cast<float>(phase.size()) * 8.f - 12.f;
        if (phaseX > mx + 8.f)
            ui.drawText(phaseX, y + 7.f, phase, kTextSecondary);
    }

    // ── Left sidebar: tool launcher ───────────────────────────────
    void renderSidebar(UIRenderer& ui, float height, WorkspaceShell& shell,
                       const UIMouseState& mouse, WorkspaceLaunchService* launchSvc)
    {
        float y = kContentY;
        float h = height - y - kStatusH;

        ui.drawRect({0.f, y, kSidebarW, h}, kSurface);
        // Right border
        ui.drawRect({kSidebarW - 1.f, y, 1.f, h}, kBorder);

        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);
        float ay = y + 8.f;

        // ── Section 1: Registered Tools (in-process IHostedTool) ──────
        if (!shell.toolRegistry().empty()) {
            ui.drawText(8.f, ay, "TOOLS", kTextSecondary);
            ui.drawRect({8.f, ay + 14.f, kSidebarW - 16.f, 1.f}, kBorder);
            ay += 20.f;

            const std::string& activeId = shell.toolRegistry().activeToolId();
            for (const auto* desc : shell.toolRegistry().allDescriptors()) {
                if (ay + 34.f > y + h - 4.f) break;

                bool active  = (desc->toolId == activeId);
                Rect cardR{4.f, ay, kSidebarW - 8.f, 30.f};
                bool hovered = cardR.contains(mouse.x, mouse.y);

                // Background — active tint / hover tint / default
                uint32_t bg = active  ? kAccentDark
                            : (hovered ? m_wsTheme.hoverHighlight : 0x252525FF);
                ui.drawRect(cardR, bg);
                ui.drawRectOutline(cardR, active ? kAccentBlue : kBorder, 1.f);
                // Left accent stripe
                ui.drawRect({4.f, ay, 3.f, 30.f}, active ? kAccentBlue : 0x404040FF);

                // Tool label (truncate to sidebar width)
                std::string lbl = desc->displayName;
                if (lbl.size() > 16) lbl = lbl.substr(0, 13) + "...";
                ui.drawText(12.f, ay + 8.f, lbl, active ? kTextPrimary : kTextSecondary);

                // "✓" for active
                if (active)
                    ui.drawText(kSidebarW - 14.f, ay + 8.f, "*", kAccentBlue);

                // Click: activate when inactive, deactivate when active
                if (m_ctx.hitRegion(cardR, false)) {
                    if (active) {
                        NF_LOG_INFO("WorkspaceUI",
                            std::string("ActivityBar: deactivating ") + desc->toolId);
                        shell.deactivateTool();
                    } else {
                        NF_LOG_INFO("WorkspaceUI",
                            std::string("ActivityBar: activating ") + desc->toolId);
                        shell.activateTool(desc->toolId);
                    }
                }

                ay += 34.f;
            }

            // Separator between Tools and Project Systems sections
            ui.drawRect({8.f, ay + 4.f, kSidebarW - 16.f, 1.f}, kBorder);
            ay += 14.f;
        }

        // ── Section 2: Project-specific gameplay panels ───────────────
        if (shell.hasProject() && shell.projectSystemsTool().panelCount() > 0) {
            ui.drawText(8.f, ay, "PROJECT SYSTEMS", kTextSecondary);
            ui.drawRect({8.f, ay + 14.f, kSidebarW - 16.f, 1.f}, kBorder);
            ay += 20.f;

            for (const auto& panelDesc : shell.projectSystemsTool().panels()) {
                if (!panelDesc.enabled) continue;
                if (ay + 34.f > y + h - 4.f) break;

                bool active  = (m_activeProjectPanelId == panelDesc.id);
                Rect cardR{4.f, ay, kSidebarW - 8.f, 30.f};
                bool hovered = cardR.contains(mouse.x, mouse.y);

                uint32_t bg = active  ? kAccentDark
                            : (hovered ? m_wsTheme.hoverHighlight : 0x252525FF);
                ui.drawRect(cardR, bg);
                ui.drawRectOutline(cardR, active ? kAccentBlue : kBorder, 1.f);
                ui.drawRect({4.f, ay, 3.f, 30.f}, active ? kAccentBlue : 0x404040FF);

                std::string lbl = panelDesc.displayName;
                if (lbl.size() > 16) lbl = lbl.substr(0, 13) + "...";
                ui.drawText(12.f, ay + 8.f, lbl,
                            active ? kTextPrimary : kTextSecondary);

                if (active)
                    ui.drawText(kSidebarW - 14.f, ay + 8.f, "*", kAccentBlue);

                if (m_ctx.hitRegion(cardR, false)) {
                    if (active) {
                        m_activeProjectPanelId.clear();
                    } else {
                        m_activeProjectPanelId = panelDesc.id;
                        shell.deactivateTool();
                    }
                }

                ay += 34.f;
            }

            ui.drawRect({8.f, ay + 4.f, kSidebarW - 16.f, 1.f}, kBorder);
            ay += 14.f;
        }

        // ── Section 3: Launch Apps (external executables) ─────────────
        ui.drawText(8.f, ay, "LAUNCH TOOL", kTextSecondary);
        ui.drawRect({8.f, ay + 14.f, kSidebarW - 16.f, 1.f}, kBorder);
        ay += 20.f;

        for (const auto& desc : shell.appRegistry().apps()) {
            if (ay + 46.f > y + h) break;

            Rect cardR{4.f, ay, kSidebarW - 8.f, 38.f};

            // Determine running state so we can show an indicator and
            // let a click on a running app stop it.
            bool running = launchSvc && launchSvc->isRunning(desc.id);

            // Draw card background with hover tint BEFORE text so the
            // highlight does not overwrite the card content.
            bool hovered = cardR.contains(mouse.x, mouse.y);
            ui.drawRect(cardR, hovered ? m_wsTheme.hoverHighlight : kCardBg);
            ui.drawRectOutline(cardR, running ? kGreen : kBorder, 1.f);

            // Running status dot (6x6 filled rect in top-right corner)
            if (running) {
                ui.drawRect({cardR.x + cardR.w - 10.f, ay + 4.f, 6.f, 6.f}, kGreen);
            }

            // App name
            ui.drawText(12.f, ay + 5.f, desc.name, kTextPrimary);

            // Executable path (truncate to fit); green when running
            std::string exePath = desc.executablePath;
            if (exePath.size() > 24) exePath = exePath.substr(0, 21) + "...";
            ui.drawText(12.f, ay + 20.f, exePath, running ? kGreen : kTextMuted);

            // Click: launch when stopped, stop when running.
            if (m_ctx.hitRegion(cardR, false)) {
                if (running && launchSvc) {
                    NF_LOG_INFO("WorkspaceUI",
                        std::string("Stopping: ") + desc.name);
                    launchSvc->shutdownApp(desc.id);
                    Notification n;
                    n.title   = std::string("Stopped: ") + desc.name;
                    n.message = desc.executablePath;
                    shell.shellContract().postNotification(n);
                } else {
                    NF_LOG_INFO("WorkspaceUI",
                        std::string("Launching: ") + desc.name
                        + "  (" + desc.executablePath + ")");

                    if (launchSvc) {
                        // Build launch context from the active project adapter so
                        // child executables receive the correct project path.
                        WorkspaceLaunchContext ctx;
                        if (shell.hasProject() && shell.projectAdapter()) {
                            const auto* adapter = shell.projectAdapter();
                            auto roots = adapter->contentRoots();
                            ctx.projectPath = roots.empty() ? "." : roots[0];
                            auto lastSepPos = ctx.projectPath.find_last_of("/\\");
                            ctx.workspaceRoot = (lastSepPos != std::string::npos)
                                                ? ctx.projectPath.substr(0, lastSepPos) : ".";
                        } else {
                            ctx.workspaceRoot = ".";
                            ctx.projectPath   = desc.isProjectScoped
                                                ? kStubProjectPath : ".";
                        }
                        ctx.sessionId = "workspace-session-" + desc.name
                                        + "-" + std::to_string(++m_launchCounter);
                        ctx.mode = WorkspaceLaunchMode::Hosted;
                        auto result = launchSvc->launchApp(desc, ctx);
                        if (result.succeeded()) {
                            NF_LOG_INFO("WorkspaceUI",
                                std::string("Launch OK: ") + desc.name
                                + "  pid=" + std::to_string(result.pid));
                            Notification n;
                            n.title   = std::string("Launched: ") + desc.name;
                            n.message = desc.executablePath;
                            shell.shellContract().postNotification(n);
                        } else {
                            NF_LOG_WARN("WorkspaceUI",
                                std::string("Launch failed: ") + desc.name
                                + "  status=" + workspaceLaunchStatusName(result.status));
                            if (m_pendingError.empty()) {
                                m_pendingError.title   = "Launch Failed: " + desc.name;
                                m_pendingError.message =
                                    "Could not launch " + desc.name + ".\n\n"
                                    + result.errorDetail + "\n\n"
                                    "Place the executable next to AtlasWorkspace.exe "
                                    "and try again.";
                            }
                        }
                    } else {
                        Notification n;
                        n.title   = std::string("Launch requested: ") + desc.name;
                        n.message = "No launch service wired";
                        shell.shellContract().postNotification(n);
                    }
                }
            }

            ay += 46.f;
        }

        m_ctx.end();

        if (shell.appRegistry().empty() && shell.toolRegistry().empty()) {
            ui.drawText(8.f, ay + 8.f, "(no tools registered)", kTextMuted);
        }
    }

    // ── Main area: welcome, project dashboard, or active tool view ──
    void renderMainArea(UIRenderer& ui, float width, float height,
                        WorkspaceShell& shell, const UIMouseState& mouse)
    {
        float x = kSidebarW;
        float y = kContentY;
        float w = width - x;
        float h = height - y - kStatusH;

        ui.drawRect({x, y, w, h}, kBackground);

        if (!shell.hasProject()) {
            m_activeProjectPanelId.clear();
            renderWelcomeScreen(ui, x, y, w, h, mouse);
        } else if (shell.toolRegistry().activeTool() != nullptr) {
            m_activeProjectPanelId.clear();
            renderActiveToolView(ui, x, y, w, h, shell, mouse);
        } else if (!m_activeProjectPanelId.empty()) {
            renderProjectPanelView(ui, x, y, w, h, shell, mouse);
        } else {
            renderProjectDashboard(ui, x, y, w, h, shell, mouse);
        }
    }

    void renderWelcomeScreen(UIRenderer& ui, float x, float y, float w, float h,
                             const UIMouseState& mouse)
    {
        float cx = x + w * 0.5f;
        float cy = y + h * 0.5f;

        // Accent line above heading
        float lineW = 120.f;
        ui.drawRect({cx - lineW * 0.5f, cy - 54.f, lineW, 3.f}, kAccentBlue);

        // Heading and subheading
        ui.drawText(cx - 64.f, cy - 44.f, "Atlas Workspace", kTextPrimary);
        ui.drawText(cx - 88.f, cy - 26.f, "Open or create a project", kTextSecondary);
        ui.drawText(cx - 60.f, cy - 10.f, "to get started.", kTextMuted);

        // Action cards
        float cardW = 160.f;
        float cardH = 46.f;
        float cardY = cy + 12.f;
        float card1X = cx - cardW - 8.f;
        float card2X = cx + 8.f;

        Rect newProjR{card1X, cardY, cardW, cardH};
        Rect openProjR{card2X, cardY, cardW, cardH};

        // Interactive layer for welcome cards — begin before drawing so hover
        // state can be used to tint the background before the text is drawn.
        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);

        // New Project card
        bool newHovered  = newProjR.contains(mouse.x, mouse.y);
        ui.drawRect(newProjR,  newHovered  ? m_wsTheme.hoverHighlight : kCardBg);
        ui.drawRectOutline(newProjR, kBorder, 1.f);
        ui.drawRect({card1X, cardY, cardW, 3.f}, kAccentBlue);
        ui.drawText(card1X + 10.f, cardY + 9.f,  "New Project",         kTextPrimary);
        ui.drawText(card1X + 10.f, cardY + 26.f, "File > New Project",  kTextMuted);

        // Open Project card
        bool openHovered = openProjR.contains(mouse.x, mouse.y);
        ui.drawRect(openProjR, openHovered ? m_wsTheme.hoverHighlight : kCardBg);
        ui.drawRectOutline(openProjR, kBorder, 1.f);
        ui.drawRect({card2X, cardY, cardW, 3.f}, kButtonBg);
        ui.drawText(card2X + 10.f, cardY + 9.f,  "Open Project",        kTextPrimary);
        ui.drawText(card2X + 10.f, cardY + 26.f, "File > Open Project", kTextMuted);

        if (m_ctx.hitRegion(newProjR, false)) {
            NF_LOG_INFO("WorkspaceUI", "New Project clicked");
            // Queue the action; the main loop opens the folder dialog outside WM_PAINT.
            m_pendingAction = WorkspaceAction::NewProject;
        }
        if (m_ctx.hitRegion(openProjR, false)) {
            NF_LOG_INFO("WorkspaceUI", "Open Project clicked");
            // Queue the action; the main loop opens the file-open dialog outside WM_PAINT.
            m_pendingAction = WorkspaceAction::OpenProject;
        }

        m_ctx.end();
    }

    void renderProjectDashboard(UIRenderer& ui, float x, float y, float w, float h,
                                WorkspaceShell& shell, const UIMouseState& mouse)
    {
        (void)h;
        float rx = x + 16.f;
        float ry = y + 16.f;
        const auto* adapter = shell.projectAdapter();

        // Project name + id
        ui.drawText(rx, ry, "Project: " + adapter->projectDisplayName(), kTextPrimary);
        ry += 18.f;
        ui.drawText(rx, ry, "ID: " + adapter->projectId(), kTextSecondary);
        ry += 28.f;

        // Stat cards
        auto drawStatCard = [&](float cx2, float cy2, const std::string& label,
                                const std::string& value) {
            float cw = 140.f, ch = 52.f;
            ui.drawRect({cx2, cy2, cw, ch}, kCardBg);
            ui.drawRectOutline({cx2, cy2, cw, ch}, kBorder, 1.f);
            ui.drawText(cx2 + 8.f, cy2 + 7.f,  label, kTextSecondary);
            ui.drawText(cx2 + 8.f, cy2 + 26.f, value, kTextPrimary);
        };

        drawStatCard(rx,          ry, "Tools",  std::to_string(shell.toolRegistry().count()));
        drawStatCard(rx + 148.f,  ry, "Panels", std::to_string(shell.panelRegistry().count()));
        drawStatCard(rx + 296.f,  ry, "Apps",   std::to_string(shell.appRegistry().count()));
        ry += 68.f;

        // Registered tools list — clickable rows to activate each tool
        ui.drawText(rx, ry, "Registered Tools", kTextSecondary);
        ui.drawText(rx + w - 120.f, ry, "Click a tool to Open it", kTextMuted);
        ui.drawRect({rx, ry + 16.f, w - 32.f, 1.f}, kBorder);
        ry += 22.f;

        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);

        const std::string& activeId = shell.toolRegistry().activeToolId();
        for (const auto* desc : shell.toolRegistry().allDescriptors()) {
            float rowH = 28.f;
            float rowW = w - 32.f;
            Rect  rowR = {rx, ry, rowW, rowH};
            bool  active  = (desc->toolId == activeId);
            bool  hovered = rowR.contains(mouse.x, mouse.y);

            uint32_t bg = active  ? kAccentDark
                        : (hovered ? m_wsTheme.hoverHighlight : 0x272727FF);
            ui.drawRect(rowR, bg);
            ui.drawRectOutline(rowR, kBorder, 1.f);
            // Left accent stripe indicates active tool
            ui.drawRect({rx, ry, 3.f, rowH}, active ? kAccentBlue : 0x444444FF);
            ui.drawText(rx + 10.f, ry + 7.f,
                        desc->displayName + "  (" + desc->toolId + ")", kTextPrimary);
            if (!active)
                ui.drawText(rx + rowW - 58.f, ry + 7.f, "Open >", kTextSecondary);

            if (m_ctx.hitRegion(rowR, false)) {
                NF_LOG_INFO("WorkspaceUI",
                    std::string("Activating tool: ") + desc->toolId);
                shell.activateTool(desc->toolId);
            }

            ry += rowH + 2.f;
        }

        m_ctx.end();

        if (shell.toolRegistry().empty()) {
            ui.drawText(rx, ry + 4.f, "(no tools registered)", kTextMuted);
        }
    }

    // ── Dropdown overlay ──────────────────────────────────────────
    // Rendered after all other chrome so the open dropdown floats on top.
    // Also handles close-on-click-outside and item selection.
    void renderDropdownOverlay(UIRenderer& ui, const UIMouseState& mouse,
                               WorkspaceShell& shell)
    {
        if (m_openMenuIdx < 0 || m_openMenuIdx >= static_cast<int>(m_dropMenus.size()))
            return;

        const DropMenu& menu = m_dropMenus[m_openMenuIdx];

        // Compute X origin by summing widths of preceding menu labels.
        static const char* kMenuItems[] = {"File", "Project", "Tools", "View", "Help"};
        float dx = 10.f;
        for (int j = 0; j < m_openMenuIdx; ++j) {
            float jw = static_cast<float>(std::strlen(kMenuItems[j])) * 8.f + 16.f;
            dx += jw + 4.f;
        }
        float dy = kToolbarH;  // dropdown opens just below the toolbar

        // Compute dropdown dimensions
        float dw = 180.f;
        for (auto& it : menu.items) {
            if (!it.label.empty()) {
                float iw = static_cast<float>(it.label.size()) * 8.f + 24.f;
                if (iw > dw) dw = iw;
            }
        }
        float dh = 6.f;
        for (auto& it : menu.items)
            dh += it.label.empty() ? 9.f : 22.f;

        // Draw background and border
        ui.drawRect({dx, dy, dw, dh}, kSurface);
        ui.drawRectOutline({dx, dy, dw, dh}, kBorder, 1.f);

        // Draw items and handle clicks
        float iy = dy + 4.f;
        bool itemConsumedClick = false;
        for (auto& it : menu.items) {
            if (it.label.empty()) {
                // Separator line
                ui.drawRect({dx + 8.f, iy + 3.f, dw - 16.f, 1.f}, kBorder);
                iy += 9.f;
            } else {
                Rect itemR{dx + 2.f, iy, dw - 4.f, 20.f};
                bool iHover = itemR.contains(mouse.x, mouse.y);
                if (iHover && it.enabled)
                    ui.drawRect(itemR, kButtonBg);

                uint32_t textCol = it.enabled ? kTextPrimary : kTextMuted;
                ui.drawText(dx + 10.f, iy + 3.f, it.label, textCol);

                if (iHover && mouse.leftReleased) {
                    itemConsumedClick = true;   // always consume — disabled items must not fall through
                    if (it.enabled) {
                        if (it.action != WorkspaceAction::None) {
                            m_pendingAction = it.action;
                        } else if (!it.command.empty()) {
                            auto cmdResult = shell.commandBus().execute(it.command);
                            if (cmdResult != ConsoleCmdExecResult::Ok &&
                                cmdResult != ConsoleCmdExecResult::PermissionDenied) {
                                m_pendingError = {
                                    "Command Failed",
                                    std::string("Could not execute: ") + it.command
                                };
                            }
                        }
                        m_openMenuIdx    = -1;
                        m_menuDismissed  = true;  // prevents dismissal click from activating underlying elements
                    }
                }
                iy += 22.f;
            }
        }

        // Close the dropdown when the user clicks outside it (and outside the toolbar).
        if (!itemConsumedClick && mouse.leftReleased) {
            bool inDropdown = (mouse.x >= dx && mouse.x < dx + dw &&
                               mouse.y >= dy && mouse.y < dy + dh);
            bool inToolbar  = (mouse.y >= 0.f && mouse.y < kToolbarH);
            if (!inDropdown && !inToolbar) {
                m_openMenuIdx   = -1;
                m_menuDismissed = true;  // prevents dismissal click from activating underlying elements
            }
        }
    }

    // ── Status bar ────────────────────────────────────────────────
    void renderStatusBar(UIRenderer& ui, float width, float height,
                         const WorkspaceShell& shell)
    {
        float y = height - kStatusH;
        ui.drawRect({0.f, y, width, kStatusH}, kAccentBlue);

        // Left: workspace phase
        std::string left = std::string("  ") + shellPhaseName(shell.phase());
        ui.drawText(8.f, y + 5.f, left, 0xFFFFFFFF);

        // Centre-left: FPS indicator derived from last frame time.
        if (m_lastFrameMs > 0.001f) {
            char fpsBuf[32];
            float fps = 1000.f / m_lastFrameMs;
            std::snprintf(fpsBuf, sizeof(fpsBuf), "%.0f FPS  %.1f ms", fps, m_lastFrameMs);
            ui.drawText(160.f, y + 5.f, fpsBuf, 0xFFFFFFFF);
        }

        // Right: counts
        std::string right =
            "Tools: "  + std::to_string(shell.toolRegistry().count())  +
            "  Panels: " + std::to_string(shell.panelRegistry().count()) +
            "  Apps: "   + std::to_string(shell.appRegistry().count());
        float rw = static_cast<float>(right.size()) * 8.f;
        ui.drawText(width - rw - 8.f, y + 5.f, right, 0xFFFFFFFF);
    }

    // ── Active tool editor view ───────────────────────────────────
    // Shown in the main area when a tool has been activated from the dashboard.
    // Displays a per-tool panel layout, a header bar with a "back" button,
    // and a bottom strip with Console + Metrics panels.
    void renderActiveToolView(UIRenderer& ui, float x, float y, float w, float h,
                              WorkspaceShell& shell, const UIMouseState& mouse)
    {
        const IHostedTool* tool = shell.toolRegistry().activeTool();
        if (!tool) return;
        const auto& desc = tool->descriptor();

        static constexpr float kHeaderH = 38.f;
        static constexpr float kBottomH = 130.f;

        // Respect PanelRegistry visibility — when console is hidden the tool
        // gets the full remaining height instead of a fixed 130px bottom strip.
        const bool consoleVisible = shell.panelRegistry().isPanelVisible("console");
        const float effectiveBottomH = consoleVisible ? kBottomH : 0.f;
        const float kMainH = h - kHeaderH - effectiveBottomH;

        // ── Tool header bar ────────────────────────────────────────
        ui.drawRect({x, y, w, kHeaderH}, kSurface);
        ui.drawRect({x, y + kHeaderH - 1.f, w, 1.f}, kBorder);
        ui.drawRect({x, y, 3.f, kHeaderH}, kAccentBlue);

        ui.drawText(x + 14.f, y + 11.f, desc.displayName, kTextPrimary);

        // Project breadcrumb
        if (shell.hasProject() && shell.projectAdapter()) {
            float nameW = static_cast<float>(desc.displayName.size()) * 8.f + 14.f;
            std::string crumb = "  /  " + shell.projectAdapter()->projectDisplayName();
            ui.drawText(x + nameW, y + 11.f, crumb, kTextSecondary);
        }

        // "< Dashboard" back button
        static constexpr float kBackW = 96.f;
        Rect backR{x + w - kBackW - 8.f, y + (kHeaderH - 22.f) * 0.5f, kBackW, 22.f};
        bool backHovered = backR.contains(mouse.x, mouse.y);
        ui.drawRect(backR, backHovered ? kButtonBg : kCardBg);
        ui.drawRectOutline(backR, kBorder, 1.f);
        ui.drawText(backR.x + 8.f, backR.y + 4.f, "< Dashboard", kTextSecondary);

        // ── Per-tool panel layout ──────────────────────────────────
        // Delegate to the tool's own render contract.
        // The tool renders its own panel layout backed by live runtime state.
        // SceneEditorTool's panels are driven by WorkspacePanelHost (main.cpp),
        // so its renderToolView is a no-op — all other tools draw their custom
        // layout here (data tables, timelines, code views, etc.).
        {
            m_ctx.begin(ui, mouse, m_wsTheme, 0.f);
            ToolViewRenderContext toolCtx{ui, mouse,
                                          x, y + kHeaderH, w, kMainH,
                                          &shell, &m_ctx};
            tool->renderToolView(toolCtx);
            m_ctx.end();
        }

        // ── Bottom strip: Console + Metrics ───────────────────────
        // Only rendered when the console panel is visible in PanelRegistry.
        if (consoleVisible)
            renderBottomStrip(ui, x, y + kHeaderH + kMainH, w, kBottomH, shell, mouse);

        // ── Click handling ─────────────────────────────────────────
        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);
        if (m_ctx.hitRegion(backR, false)) {
            NF_LOG_INFO("WorkspaceUI", "Tool deactivated: back to dashboard");
            shell.deactivateTool();
        }
        m_ctx.end();
    }

    // ── Project Systems panel view ────────────────────────────────
    // Renders the full-screen content view for a selected project gameplay panel
    // (e.g. Economy, Inventory Rules) when one is chosen from the PROJECT SYSTEMS
    // sidebar section.
    void renderProjectPanelView(UIRenderer& ui, float x, float y, float w, float h,
                                WorkspaceShell& shell, const UIMouseState& mouse)
    {
        const GameplaySystemPanelDescriptor* desc =
            shell.projectSystemsTool().findPanel(m_activeProjectPanelId);
        if (!desc) { m_activeProjectPanelId.clear(); return; }

        static constexpr float kHeaderH = 38.f;
        static constexpr float kBottomH = 130.f;

        // Respect PanelRegistry visibility for the console bottom strip.
        const bool consoleVisible = shell.panelRegistry().isPanelVisible("console");
        const float effectiveBottomH = consoleVisible ? kBottomH : 0.f;
        const float kContentH = h - kHeaderH - effectiveBottomH;

        // ── Header bar ────────────────────────────────────────────────
        ui.drawRect({x, y, w, kHeaderH}, kSurface);
        ui.drawRect({x, y + kHeaderH - 1.f, w, 1.f}, kBorder);
        ui.drawRect({x, y, 3.f, kHeaderH}, kAccentBlue);
        ui.drawText(x + 14.f, y + 11.f, desc->displayName, kTextPrimary);

        if (shell.projectAdapter()) {
            float nameW = static_cast<float>(desc->displayName.size()) * 8.f + 14.f;
            ui.drawText(x + nameW, y + 11.f,
                        "  /  " + shell.projectAdapter()->projectDisplayName(),
                        kTextSecondary);
        }

        // Back button
        static constexpr float kBackW = 96.f;
        Rect backR{x + w - kBackW - 8.f, y + (kHeaderH - 22.f) * 0.5f, kBackW, 22.f};
        bool backHov = backR.contains(mouse.x, mouse.y);
        ui.drawRect(backR, backHov ? kButtonBg : kCardBg);
        ui.drawRectOutline(backR, kBorder, 1.f);
        ui.drawText(backR.x + 8.f, backR.y + 4.f, "< Dashboard", kTextSecondary);

        // ── Content area ──────────────────────────────────────────────
        float cy = y + kHeaderH;
        ui.drawRect({x, cy, w, kContentH}, kBackground);

        IEditorPanel* panel = shell.projectSystemsTool().getOrCreatePanel(desc->id);
        if (!panel) {
            ui.drawText(x + 16.f, cy + 20.f, "Panel unavailable.", kTextMuted);
        } else {
            static constexpr float kLabelColW = 180.f;
            float rx = x + 24.f;
            float ry = cy + 16.f;

            // ── Title / status row ────────────────────────────────────────
            ui.drawText(rx, ry, panel->panelTitle(), kTextPrimary);
            const uint32_t statusColor = panel->isReady() ? kGreen : kTextMuted;
            const char*    statusText  = panel->isReady() ? "Ready" : "Loading…";
            ui.drawText(rx + kLabelColW, ry, statusText, statusColor);
            ry += 22.f;
            ui.drawRect({x + 12.f, ry, w - 24.f, 1.f}, kBorder);
            ry += 10.f;

            // ── Real panel data rows ──────────────────────────────────────
            // summaryRows() returns (label, value) pairs populated from the
            // panel's live data (loaded from project files on project open).
            const auto rows = panel->summaryRows();
            if (rows.empty()) {
                ui.drawText(rx, ry, "No data loaded.", kTextMuted);
            } else {
                static constexpr float kRowOverflowMargin = 10.f; // bottom margin guard
                for (const auto& [label, value] : rows) {
                    ui.drawText(rx, ry, label, kTextSecondary);
                    ui.drawText(rx + kLabelColW, ry, value, kTextPrimary);
                    ry += 18.f;
                    if (ry > cy + kContentH - kRowOverflowMargin) break;
                }
            }

            // ── Project root hint ─────────────────────────────────────────
            ry += 6.f;
            ui.drawRect({x + 12.f, ry, w - 24.f, 1.f}, kBorder);
            ry += 10.f;
            if (shell.hasProject() && shell.projectAdapter()) {
                const auto roots = shell.projectAdapter()->contentRoots();
                const std::string rootHint = roots.empty()
                    ? shell.projectAdapter()->projectId()
                    : roots[0];
                ui.drawText(rx, ry, "Project root:", kTextMuted);
                ui.drawText(rx + kLabelColW, ry, rootHint, kTextMuted);
            }
        }

        // ── Bottom strip ───────────────────────────────────────────────
        // Only rendered when the console panel is visible in PanelRegistry.
        if (consoleVisible)
            renderBottomStrip(ui, x, cy + kContentH, w, kBottomH, shell, mouse);

        // ── Back button click ──────────────────────────────────────────
        m_ctx.begin(ui, mouse, m_wsTheme, 0.f);
        if (m_ctx.hitRegion(backR, false)) {
            NF_LOG_INFO("WorkspaceUI", "Project panel closed: back to dashboard");
            m_activeProjectPanelId.clear();
        }
        m_ctx.end();
    }

    // ── Bottom strip: Console + Metrics ──────────────────────────
    // Rendered below the per-tool panel layout in the active tool view.
    // Console shows project-context log lines; Metrics shows live shell stats
    // and the most recent workspace notification.
    void renderBottomStrip(UIRenderer& ui, float x, float y, float w, float h,
                           const WorkspaceShell& shell, const UIMouseState& mouse)
    {
        float consoleW  = w * 0.65f;
        float metricsW  = w - consoleW;

        // ── Console ────────────────────────────────────────────────
        ui.drawRect({x, y, consoleW, h}, 0x161616FF);
        ui.drawRectOutline({x, y, consoleW, h}, kBorder, 1.f);
        ui.drawRect({x, y, consoleW, 22.f}, kSurface);
        ui.drawRect({x, y + 22.f, consoleW, 1.f}, kBorder);

        // ── Tab buttons: CONSOLE | SYSTEM ─────────────────────────
        constexpr float kTabW = 72.f;
        constexpr float kTabH = 22.f;
        const NF::Rect consoleTabR = {x,           y, kTabW,      kTabH};
        const NF::Rect systemTabR  = {x + kTabW,   y, kTabW,      kTabH};

        // Detect clicks — switch active tab.
        if (systemTabR.contains(mouse.x, mouse.y) && mouse.leftReleased)
            m_consoleTab = 1;
        if (consoleTabR.contains(mouse.x, mouse.y) && mouse.leftReleased)
            m_consoleTab = 0;

        // Draw CONSOLE tab
        const uint32_t consoleTabBg = (m_consoleTab == 0) ? kSurface : 0x1A1A1AFFu;
        const uint32_t consoleTabFg = (m_consoleTab == 0) ? kAccentBlue : kTextSecondary;
        ui.drawRect(consoleTabR, consoleTabBg);
        ui.drawText(x + 8.f, y + 4.f, "CONSOLE", consoleTabFg);
        if (m_consoleTab == 0)
            ui.drawRect({x, y + kTabH - 2.f, kTabW, 2.f}, kAccentBlue);

        // Draw SYSTEM tab
        const uint32_t sysTabBg = (m_consoleTab == 1) ? kSurface : 0x1A1A1AFFu;
        const uint32_t sysTabFg = (m_consoleTab == 1) ? kAccentBlue : kTextSecondary;
        ui.drawRect(systemTabR, sysTabBg);
        ui.drawText(x + kTabW + 8.f, y + 4.f, "SYSTEM", sysTabFg);
        if (m_consoleTab == 1)
            ui.drawRect({x + kTabW, y + kTabH - 2.f, kTabW, 2.f}, kAccentBlue);

        // Divider between tabs and the rest of the title area.
        ui.drawRect({x + kTabW * 2.f, y, consoleW - kTabW * 2.f, kTabH}, kSurface);

        // ── Console content ────────────────────────────────────────
        float cy = y + 28.f;
        if (m_consoleTab == 0) {
            // Project context messages.
            if (shell.hasProject() && shell.projectAdapter()) {
                std::string projMsg = "[INFO]  Project: "
                    + shell.projectAdapter()->projectDisplayName()
                    + "  (id=" + shell.projectAdapter()->projectId() + ")";
                ui.drawText(x + 8.f, cy, projMsg, kGreen);
                cy += 18.f;
                for (const auto& root : shell.projectAdapter()->contentRoots()) {
                    std::string rootMsg = "[INFO]  Content root: " + root;
                    if (rootMsg.size() > 72) rootMsg = rootMsg.substr(0, 69) + "...";
                    ui.drawText(x + 8.f, cy, rootMsg, kTextSecondary);
                    cy += 16.f;
                    if (cy + 16.f > y + h - 4.f) break;
                }
            } else {
                ui.drawText(x + 8.f, cy, "[INFO]  No project loaded", kTextMuted);
                cy += 18.f;
            }
            if (const auto* tool = shell.toolRegistry().activeTool()) {
                if (cy + 14.f < y + h - 4.f) {
                    std::string toolMsg = "[INFO]  Active tool: "
                        + tool->descriptor().displayName
                        + "  (" + tool->toolId() + ")";
                    ui.drawText(x + 8.f, cy, toolMsg, kTextSecondary);
                }
            }
        } else {
            // System log — show most-recent lines that fit.
            std::vector<std::string> lines;
            {
                std::lock_guard<std::mutex> lk(m_sysLogMutex);
                lines.assign(m_sysLog.begin(), m_sysLog.end());
            }
            // Determine how many rows fit and show tail.
            const float rowH   = 14.f;
            const float bottom = y + h - 4.f;
            const int   maxRows = static_cast<int>((bottom - cy) / rowH);
            const int   start   = static_cast<int>(lines.size()) > maxRows
                                    ? static_cast<int>(lines.size()) - maxRows : 0;
            for (int i = start; i < static_cast<int>(lines.size()); ++i) {
                if (cy + rowH > bottom) break;
                // Colour by level prefix.
                const std::string& ln = lines[static_cast<size_t>(i)];
                uint32_t fg = kTextSecondary;
                if (ln.size() >= 5) {
                    std::string_view tag(ln.data() + 1, 4);
                    if (tag == "WARN") fg = 0xFFE8A435;
                    else if (tag == "ERR " || tag == "FATL") fg = 0xFFF44747;
                    else if (tag == "INFO") fg = kTextSecondary;
                }
                // Truncate long lines to panel width.
                const size_t maxChars = static_cast<size_t>((consoleW - 16.f) / 7.f);
                const std::string display = ln.size() > maxChars
                    ? ln.substr(0, maxChars - 1) + "…" : ln;
                ui.drawText(x + 8.f, cy, display, fg);
                cy += rowH;
            }
            if (lines.empty()) {
                ui.drawText(x + 8.f, cy, "(no system output yet)", kTextMuted);
            }
        }

        // ── Metrics ────────────────────────────────────────────────
        float mx = x + consoleW;
        ui.drawRect({mx, y, metricsW, h}, kCardBg);
        ui.drawRectOutline({mx, y, metricsW, h}, kBorder, 1.f);
        ui.drawRect({mx, y, metricsW, 22.f}, kSurface);
        ui.drawRect({mx, y + 22.f, metricsW, 1.f}, kBorder);
        ui.drawText(mx + 8.f, y + 4.f, "METRICS", kTextSecondary);

        float mcy = y + 28.f;
        ui.drawText(mx + 8.f, mcy,
            std::string("Workspace: ") + shellPhaseName(shell.phase()),
            kTextSecondary);
        mcy += 18.f;
        ui.drawText(mx + 8.f, mcy,
            "Tools: "  + std::to_string(shell.toolRegistry().count()),  kTextSecondary);
        mcy += 16.f;
        ui.drawText(mx + 8.f, mcy,
            "Panels: " + std::to_string(shell.panelRegistry().count()), kTextSecondary);
        mcy += 16.f;
        ui.drawText(mx + 8.f, mcy,
            "Apps: "   + std::to_string(shell.appRegistry().count()),   kTextSecondary);
        mcy += 16.f;
        if (const auto* tool = shell.toolRegistry().activeTool()) {
            ui.drawText(mx + 8.f, mcy,
                std::string("State: ") + hostedToolStateName(tool->state()), kGreen);
            mcy += 16.f;
        }
        // Most recent workspace notification
        const auto& notifs = shell.shellContract().recentNotifications();
        if (!notifs.empty() && mcy + 14.f < y + h - 4.f) {
            const auto& last = notifs.back();
            std::string nstr = "[!] " + last.title;
            if (nstr.size() > 28) nstr = nstr.substr(0, 25) + "...";
            ui.drawText(mx + 8.f, mcy, nstr, kAccentBlue);
        }
    }

    // ── Per-instance state ────────────────────────────────────────
    UIContext m_ctx;      // maintains scroll state across frames
    UITheme   m_wsTheme;  // workspace-palette theme for UIContext

    // Monotonically increasing counter used to build unique session IDs.
    // Each sidebar launch increments this so repeated launches of the same
    // app produce distinct session identifiers.
    uint32_t m_launchCounter = 0;

    // Deferred action queued during render() for the main loop to handle.
    // Only one action can be pending at a time (last write wins within a frame).
    WorkspaceAction      m_pendingAction = WorkspaceAction::None;
    WorkspacePendingError m_pendingError;

    // Dropdown menu data — one entry per menu label (File/Project/Tools/View/Help).
    std::array<DropMenu, 5> m_dropMenus;
    int  m_openMenuIdx      = -1;    // index of the currently open dropdown, -1 = none
    bool m_menuDismissed    = false; // true for the frame after a dropdown was closed,
                                     // so the dismissal click cannot also hit underlying elements

    // Active project gameplay panel (PROJECT SYSTEMS sidebar selection).
    // Empty string = none selected (shows project dashboard).
    std::string m_activeProjectPanelId;

    // Fallback project path for project-scoped apps when no project is loaded.
    static constexpr const char* kStubProjectPath = "stub.atlas";

    // Last frame time in milliseconds — set by the main loop via setLastFrameMs().
    // Displayed in the status bar as an FPS indicator.
    float m_lastFrameMs = 0.f;

    // ── Console / System tab state ────────────────────────────────
    // 0 = Console (project context), 1 = System (full Logger stream).
    // Default to 1 (System) so live log output is visible immediately on startup.
    int m_consoleTab = 1;

    // System log: all NF_LOG_* lines captured via Logger sink.
    static constexpr size_t kSysLogMax = 500;
    std::deque<std::string> m_sysLog;
    mutable std::mutex      m_sysLogMutex;
    size_t                  m_logSinkId = static_cast<size_t>(-1);
};

} // namespace NF
