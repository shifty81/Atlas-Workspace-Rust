// AtlasWorkspace — primary executable entrypoint
//
// AtlasWorkspace.exe is the outer workspace shell — NOT a game editor.
// It hosts and launches child tools (NovaForgeEditor, NovaForgeGame,
// NovaForgeServer, …) via WorkspaceLaunchService.
//
// Render pipeline:
//   WorkspaceShell (state) → WorkspaceRenderer → UIRenderer → GDIBackend → Win32 window
//
// Rule: no child process may be launched outside WorkspaceLaunchService.
#include "NF/Core/Core.h"
#include "NF/UI/UI.h"
#include "NF/UI/UIBackend.h"
#include "NF/UI/UIWidgets.h"
#include "NF/Input/Input.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/WorkspaceBootstrap.h"
#include "NF/Workspace/WorkspaceFrameController.h"
#include "NF/Workspace/WorkspaceRenderer.h"
#include "NF/Workspace/WorkspaceAppRegistry.h"
#include "NF/Workspace/WorkspaceLaunchContract.h"
#include "NF/Workspace/IGameProjectAdapter.h"
#include "NF/Workspace/WorkspaceViewportBridge.h"
#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/WorkspacePanelHost.h"
#include "NF/Workspace/IViewportSurface.h"
#include "NF/Editor/CoreToolRoster.h"
#include "NF/Editor/SceneEditorTool.h"
#include "NF/Editor/AssetEditorTool.h"
#include "NF/Editor/MaterialEditorTool.h"
#include "NF/Editor/AnimationEditorTool.h"
#include "NovaForge/EditorAdapter/NovaForgeAdapter.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewRuntime.h"
#include "NovaForge/EditorAdapter/NovaForgeAssetPreview.h"
#include "NovaForge/EditorAdapter/NovaForgeMaterialPreview.h"
#include "LocalProjectAdapter.h"
#if defined(_WIN32)
#  include <windows.h>
#  include <shellapi.h>   // DragAcceptFiles / DragQueryFileW / DragFinish
#  include <shlobj.h>     // SHBrowseForFolderW / SHGetPathFromIDListW
#  include <commdlg.h>    // GetOpenFileNameW
#  include "NF/Input/Win32InputAdapter.h"
#  include "NF/UI/GDIBackend.h"
#endif
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <string>

static NF::WorkspacePanelHost*     g_panelHost = nullptr;
static NF::DockLayout*             g_toolDock  = nullptr;

// ── Global state for Win32 WndProc ───────────────────────────────
#if defined(_WIN32)

// ── String conversion helpers ─────────────────────────────────────

static std::string wideToUtf8(const wchar_t* ws) {
    if (!ws || !*ws) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string s(static_cast<size_t>(len), '\0');
    int written = WideCharToMultiByte(CP_UTF8, 0, ws, -1, s.data(), len, nullptr, nullptr);
    if (written <= 0) return {};
    // WideCharToMultiByte with -1 includes the null terminator in the count.
    if (!s.empty() && s.back() == '\0') s.pop_back();
    return s;
}

static std::wstring utf8ToWide(const std::string& u8) {
    if (u8.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring ws(static_cast<size_t>(len), L'\0');
    int written = MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, ws.data(), len);
    if (written <= 0) return {};
    if (!ws.empty() && ws.back() == L'\0') ws.pop_back();
    return ws;
}

// ── NovaForge project-root detection ─────────────────────────────
// Returns true if |root| is a NovaForge project directory.
// Accepts the canonical uppercase form (NovaForge.atlas), the lowercase form
// used by the samples directory (novaforge.atlas), and the legacy JSON marker.

static bool isNovaForgeProjectRoot(const std::filesystem::path& root) {
    return std::filesystem::exists(root / "NovaForge.atlas") ||
           std::filesystem::exists(root / "novaforge.atlas")  ||
           std::filesystem::exists(root / "novaforge.project.json");
}

// ── Win32 project dialogs ─────────────────────────────────────────
// Both helpers open a modal dialog and, on success, load a project adapter
// into the shell.  They must be called from OUTSIDE WM_PAINT (the main loop)
// so that the nested COM/common-dialog message pump does not corrupt paint state.

static void doNewProject(NF::WorkspaceShell& shell, HWND hwnd) {
    BROWSEINFOW bi{};
    bi.hwndOwner  = hwnd;
    bi.lpszTitle  = L"Select a folder for the new project";
    bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderW(&bi);
    if (!pidl) return; // user cancelled

    wchar_t path[MAX_PATH] = {};
    if (!SHGetPathFromIDListW(pidl, path)) {
        CoTaskMemFree(pidl);
        MessageBoxW(hwnd,
            L"Could not resolve the selected folder path.",
            L"New Project", MB_OK | MB_ICONERROR);
        return;
    }
    CoTaskMemFree(pidl);

    // Derive project name from the chosen folder's last path component.
    std::wstring wpath(path);
    auto sep = wpath.find_last_of(L"\\/");
    std::wstring wname = (sep != std::wstring::npos) ? wpath.substr(sep + 1) : wpath;

    std::string name    = wideToUtf8(wname.c_str());
    std::string pathStr = wideToUtf8(path);

    // If the selected folder is a NovaForge project root, use the NovaForgeAdapter
    // so its gameplay panels, content roots, and commands are registered correctly.
    std::unique_ptr<NF::IGameProjectAdapter> adapter;
    if (isNovaForgeProjectRoot(std::filesystem::path(pathStr))) {
        adapter = std::make_unique<NovaForge::NovaForgeAdapter>(pathStr);
    } else {
        adapter = std::make_unique<LocalProjectAdapter>("local." + name, name, pathStr);
    }

    if (!shell.loadProject(std::move(adapter))) {
        MessageBoxW(hwnd,
            L"The workspace could not load the selected folder as a project.",
            L"New Project", MB_OK | MB_ICONERROR);
    } else {
        NF_LOG_INFO("AtlasWorkspace", "New project loaded: " + name + " @ " + pathStr);
        // Record in recent files so it appears in the welcome screen and File menu.
        shell.recentFiles().record(pathStr, name, NF::RecentFileKind::Project,
                                   static_cast<uint64_t>(std::time(nullptr)) * 1000ULL);
    }
}

static void doOpenProject(NF::WorkspaceShell& shell, HWND hwnd) {
    wchar_t buf[MAX_PATH] = {};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner   = hwnd;
    // Filter: show .atlas files primarily, but allow all files as a fallback.
    ofn.lpstrFilter = L"Atlas Project (*.atlas)\0*.atlas\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile   = buf;
    ofn.nMaxFile    = MAX_PATH;
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle  = L"Open Atlas Project";

    if (!GetOpenFileNameW(&ofn)) return; // user cancelled or error

    std::wstring wpath(buf);
    // Use the file stem (no extension) as the display name.
    auto sep = wpath.find_last_of(L"\\/");
    std::wstring wname = (sep != std::wstring::npos) ? wpath.substr(sep + 1) : wpath;
    auto dot = wname.rfind(L'.');
    if (dot != std::wstring::npos) wname = wname.substr(0, dot);

    std::string name    = wideToUtf8(wname.c_str());
    std::string pathStr = wideToUtf8(buf);

    // If the opened .atlas file belongs to the NovaForge project root, use the
    // NovaForgeAdapter so its gameplay panels and content roots are registered.
    // For any other project file, fall back to the generic LocalProjectAdapter.
    std::filesystem::path atlaspath(pathStr);
    std::filesystem::path projectDir = atlaspath.parent_path();

    std::unique_ptr<NF::IGameProjectAdapter> adapter;
    if (isNovaForgeProjectRoot(projectDir)) {
        adapter = std::make_unique<NovaForge::NovaForgeAdapter>(projectDir.string());
    } else {
        adapter = std::make_unique<LocalProjectAdapter>("local." + name, name, pathStr);
    }

    if (!shell.loadProject(std::move(adapter))) {
        MessageBoxW(hwnd,
            L"The workspace could not load the selected file as a project.",
            L"Open Project", MB_OK | MB_ICONERROR);
    } else {
        NF_LOG_INFO("AtlasWorkspace", "Project opened: " + name + " @ " + pathStr);
        // Record in recent files so it appears in the welcome screen and File menu.
        shell.recentFiles().record(pathStr, name, NF::RecentFileKind::Project,
                                   static_cast<uint64_t>(std::time(nullptr)) * 1000ULL);
    }
}

// ── Handle deferred workspace actions ────────────────────────────
// Called from the main loop after PeekMessage dispatch so that modal dialogs
// open outside WM_PAINT and do not create unsafe nested message loops.

static void handlePendingWorkspaceActions(NF::WorkspaceRenderer& renderer,
                                          NF::WorkspaceShell& shell, HWND hwnd)
{
    // Show any launch or runtime error dialog first (non-blocking, informational).
    NF::WorkspacePendingError err = renderer.takePendingError();
    if (!err.empty()) {
        std::wstring wtitle = utf8ToWide(err.title);
        std::wstring wmsg   = utf8ToWide(err.message);
        MessageBoxW(hwnd, wmsg.c_str(), wtitle.c_str(), MB_OK | MB_ICONWARNING);
    }

    // Then handle any queued project action.
    NF::WorkspaceAction action = renderer.takePendingAction();
    switch (action) {
        case NF::WorkspaceAction::NewProject:
            doNewProject(shell, hwnd);
            break;
        case NF::WorkspaceAction::OpenProject:
            doOpenProject(shell, hwnd);
            break;
        case NF::WorkspaceAction::Exit:
            DestroyWindow(hwnd);
            break;
        default:
            break;
    }
}

static NF::WorkspaceShell*    g_shell       = nullptr;
static NF::WorkspaceRenderer* g_renderer    = nullptr;
static NF::UIRenderer*        g_ui          = nullptr;
static NF::Win32InputAdapter* g_inputAdapter = nullptr;
static NF::GDIBackend*        g_gdiBackend  = nullptr;
static NF::InputSystem*       g_inputSystem = nullptr;
static NF::WorkspaceLaunchService* g_launchSvc = nullptr;
static int g_clientW = 1280, g_clientH = 800;

// Per-frame left-button state for edge detection (leftPressed / leftReleased).
// Updated each WM_PAINT so transitions fire for exactly one rendered frame.

static LRESULT CALLBACK WorkspaceWndProc(HWND hwnd, UINT msg,
                                          WPARAM wParam, LPARAM lParam) {
    if (g_inputAdapter)
        g_inputAdapter->processMessage(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (g_shell && g_renderer && g_ui && g_gdiBackend && g_inputSystem) {
            // Route input through the shell's InputRouter.  beginFrame() extracts
            // mouse position, button edge transitions, scroll, and typed text from
            // the InputSystem snapshot.  The shell owns the prevLeftDown tracking.
            g_shell->inputRouter().beginFrame(g_inputSystem->state());

            const NF::UIMouseState& mouse = g_shell->inputRouter().mouseState();

            g_gdiBackend->setTargetDC(hdc);
            g_gdiBackend->beginFrame(g_clientW, g_clientH);
            g_renderer->render(*g_ui,
                               static_cast<float>(g_clientW),
                               static_cast<float>(g_clientH),
                               *g_shell, mouse, g_launchSvc);

            // ── WorkspacePanelHost: AtlasUI panels drawn over chrome ──
            // Only used for the Scene Editor — it owns the Hierarchy, Viewport,
            // Inspector, and ContentBrowser panels.  All other tools render their
            // own custom layout via renderToolView() in WorkspaceRenderer.
            if (g_panelHost && g_toolDock) {
                const NF::IHostedTool* activeTool =
                    g_shell->toolRegistry().activeTool();
                if (activeTool &&
                    activeTool->descriptor().category ==
                        NF::HostedToolCategory::SceneEditing) {
                    const bool consoleVis =
                        g_shell->panelRegistry().isPanelVisible("console");
                    NF::Rect area = NF::WorkspaceRenderer::contentAreaBounds(
                        static_cast<float>(g_clientW),
                        static_cast<float>(g_clientH),
                        true, consoleVis);

                    // Four-zone layout:
                    //  Top row: Hierarchy | Viewport | Inspector (fills most of area.h)
                    //  Bottom row: ContentBrowser (fixed 180 px)
                    static constexpr float kHierW = 250.f;
                    static constexpr float kInspW = 300.f;
                    static constexpr float kCBH   = 180.f;
                    const float topH  = std::max(0.f, area.h - kCBH);
                    const float viewW = std::max(0.f, area.w - kHierW - kInspW);

                    if (auto* dp = g_toolDock->findPanel("Hierarchy")) {
                        dp->visible = true;
                        dp->bounds  = {area.x, area.y, kHierW, topH};
                    }
                    if (auto* dp = g_toolDock->findPanel("Viewport")) {
                        dp->visible = true;
                        dp->bounds  = {area.x + kHierW, area.y, viewW, topH};
                    }
                    if (auto* dp = g_toolDock->findPanel("Inspector")) {
                        dp->visible = true;
                        dp->bounds  = {area.x + kHierW + viewW, area.y, kInspW, topH};
                    }
                    if (auto* dp = g_toolDock->findPanel("ContentBrowser")) {
                        dp->visible = true;
                        dp->bounds  = {area.x, area.y + topH, area.w, kCBH};
                    }

                    // Populate ContentBrowser when the loaded project changes.
                    static std::string s_lastCBProject;
                    const std::string  curProjId =
                        (g_shell->hasProject() && g_shell->projectAdapter())
                            ? g_shell->projectAdapter()->projectId()
                            : std::string{};
                    if (curProjId != s_lastCBProject) {
                        s_lastCBProject = curProjId;
                        auto& cb = g_panelHost->contentBrowser();
                        cb.clearEntries();
                        if (!curProjId.empty() && g_shell->projectAdapter()) {
                            const auto roots = g_shell->projectAdapter()->contentRoots();
                            if (!roots.empty()) {
                                cb.setCurrentPath(roots[0]);
                                try {
                                    namespace fs = std::filesystem;
                                    fs::path dir(roots[0]);
                                    if (fs::exists(dir) && fs::is_directory(dir)) {
                                        for (const auto& de : fs::directory_iterator(dir)) {
                                            cb.addEntry(de.path().filename().string(),
                                                        de.is_directory());
                                        }
                                    }
                                } catch (...) {}
                            } else {
                                cb.setCurrentPath("(no content roots)");
                            }
                        }
                    }

                    activeTool->syncPanels(*g_panelHost);
                    g_panelHost->renderPanels(*g_toolDock);
                    g_panelHost->handleInput(*g_toolDock, *g_inputSystem);
                }
            }

            g_gdiBackend->endFrame();
            g_shell->inputRouter().endFrame();
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_SIZE:
        g_clientW = LOWORD(lParam);
        g_clientH = HIWORD(lParam);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    // Mouse-button state changes need an immediate repaint for click feedback.
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    // WM_MOUSEMOVE does NOT request an extra repaint: the frame loop already
    // calls InvalidateRect once per tick (~60 FPS), which is sufficient to
    // refresh hover states without flooding the paint queue.
    case WM_MOUSEMOVE:
        break;
    // Dispatch workspace hotkeys from keyboard events, then repaint.
    case WM_KEYDOWN:
        if (g_shell) {
            // Read modifier state at the time of the key press.
            bool ctrl  = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            bool shift = (GetKeyState(VK_SHIFT)   & 0x8000) != 0;

            if (ctrl && shift && wParam == 'P') {
                // Ctrl+Shift+P → Command Palette
                (void)g_shell->commandBus().execute("workspace.command_palette");
            } else if (ctrl && !shift && wParam == 'S') {
                // Ctrl+S → Save
                (void)g_shell->commandBus().execute("workspace.save");
            } else if (ctrl && !shift && wParam == 'B') {
                // Ctrl+B → Build
                (void)g_shell->commandBus().execute("workspace.build");
            } else if (ctrl && !shift && wParam == 'Z') {
                // Ctrl+Z → Undo
                (void)g_shell->commandBus().execute("workspace.undo");
            } else if (ctrl && !shift && (wParam == 'Y')) {
                // Ctrl+Y → Redo
                (void)g_shell->commandBus().execute("workspace.redo");
            } else if (ctrl && shift && wParam == 'Z') {
                // Ctrl+Shift+Z → Redo (alternate binding)
                (void)g_shell->commandBus().execute("workspace.redo");
            } else if (ctrl && !shift && wParam == 'W') {
                // Ctrl+W → Close active document
                (void)g_shell->commandBus().execute("workspace.close_document");
            } else if (wParam == VK_ESCAPE) {
                // Escape → Dismiss command palette / cancel modal
                (void)g_shell->commandBus().execute("workspace.dismiss");
            }
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    case WM_CHAR:
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    case WM_DROPFILES:
        // Route dropped files to the workspace shell for asset intake.
        if (g_shell) {
            HDROP hDrop = reinterpret_cast<HDROP>(wParam);
            // Passing 0xFFFFFFFF as the file index queries the total count of dropped files.
            static constexpr UINT kQueryFileCount = 0xFFFFFFFFu;
            UINT count = DragQueryFileW(hDrop, kQueryFileCount, nullptr, 0);
            for (UINT i = 0; i < count; ++i) {
                wchar_t buf[MAX_PATH] = {};
                if (DragQueryFileW(hDrop, i, buf, MAX_PATH) > 0) {
                    // Convert to UTF-8 and post as a workspace.import command.
                    std::string path = wideToUtf8(buf);
                    // The asset reimport pipeline is invoked via command bus;
                    // the handler parses the path argument.
                    (void)g_shell->commandBus().execute("workspace.import_file:" + path);
                }
            }
            DragFinish(hDrop);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_ACTIVATE:
        // On deactivation (losing focus), reset input router focus to workspace chrome
        // so that stale TextInput focus doesn't persist into the next activation.
        if (LOWORD(wParam) == WA_INACTIVE && g_shell) {
            g_shell->inputRouter().clearFocus();
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
#endif // _WIN32

// ── Register child apps ───────────────────────────────────────────
// Registers all known child tool executables into the shell's app registry.
// Executable paths are relative to the workspace binary directory.
static void registerApps(NF::WorkspaceAppRegistry& registry) {
    {
        NF::WorkspaceAppDescriptor d;
        d.id              = NF::WorkspaceAppId::NovaForgeEditor;
        d.name            = "NovaForge Editor";
        d.executablePath  = "NovaForgeEditor.exe";
        d.isProjectScoped = true;
        d.allowDirectLaunch = false;
        d.defaultArgs     = { "--hosted" };
        registry.registerApp(std::move(d));
    }
    {
        NF::WorkspaceAppDescriptor d;
        d.id              = NF::WorkspaceAppId::NovaForgeGame;
        d.name            = "NovaForge Game";
        d.executablePath  = "NovaForgeGame.exe";
        d.isProjectScoped = true;
        d.allowDirectLaunch = false;
        d.defaultArgs     = { "--hosted" };
        registry.registerApp(std::move(d));
    }
    {
        NF::WorkspaceAppDescriptor d;
        d.id              = NF::WorkspaceAppId::NovaForgeServer;
        d.name            = "NovaForge Server";
        d.executablePath  = "NovaForgeServer.exe";
        d.isProjectScoped = true;
        d.allowDirectLaunch = false;
        d.defaultArgs     = { "--headless" };
        registry.registerApp(std::move(d));
    }
}

// ── main ─────────────────────────────────────────────────────────

// ── Resolve the directory that contains AtlasWorkspace.exe ───────
// Child executables (NovaForgeEditor.exe etc.) are expected to live in
// the same directory as the host binary.  We derive that directory from
// argv[0] so the launch service can build absolute paths at runtime.
static std::string resolveBaseDir(const char* argv0) {
#if defined(_WIN32)
    // GetModuleFileNameW is the most reliable source on Windows.
    wchar_t buf[MAX_PATH] = {};
    DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        // Strip the filename to get the directory.
        wchar_t* lastSep = wcsrchr(buf, L'\\');
        if (lastSep) *lastSep = L'\0';
        // Convert back to UTF-8.
        int mbLen = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
        std::string dir(static_cast<size_t>(mbLen), '\0');
        WideCharToMultiByte(CP_UTF8, 0, buf, -1, dir.data(), mbLen, nullptr, nullptr);
        // Remove trailing null inserted by WideCharToMultiByte.
        if (!dir.empty() && dir.back() == '\0') dir.pop_back();
        return dir;
    }
#endif
    // Fallback: derive from argv[0].
    if (argv0 && *argv0) {
        std::string p(argv0);
        auto pos = p.find_last_of("/\\");
        if (pos != std::string::npos) return p.substr(0, pos);
    }
    return ".";
}

int main(int argc, char* argv[]) {
#if defined(_WIN32)
    // Suppress the external Win32 console window so all log output is funnelled
    // exclusively into the Workspace SYSTEM tab (WorkspaceRenderer registers a
    // Logger sink for this in its constructor).  This must be the very first call
    // so no console window ever flashes on screen, even during startup.
    // NOTE: stdout/stderr writes after this point are silently discarded.  Use
    //       NF_LOG_* macros (which write to the Logger sink and the SYSTEM tab)
    //       for all diagnostic output.
    FreeConsole();
#endif
    // Resolve the directory containing AtlasWorkspace.exe so the launch
    // service can locate sibling executables (NovaForgeEditor.exe etc.).
    std::string binDir = resolveBaseDir(argc > 0 ? argv[0] : nullptr);
    NF_LOG_INFO("AtlasWorkspace", "Binary directory: " + binDir);

    NF::coreInit();
    NF_LOG_INFO("AtlasWorkspace", "=== Atlas Workspace ===");
    NF_LOG_INFO("AtlasWorkspace", std::string("Version: ") + NF::NF_VERSION_STRING);

#if defined(_WIN32)
    // COM is required by SHBrowseForFolderW (folder picker) and CoTaskMemFree.
    // COINIT_APARTMENTTHREADED matches the Win32 message-loop threading model.
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
#endif

    // ── Workspace shell ───────────────────────────────────────────
    NF::WorkspaceShell shell;

    // Register child apps before bootstrap (bootstrap only calls initialize()).
    registerApps(shell.appRegistry());
    NF_LOG_INFO("AtlasWorkspace",
        std::string("App registry: ") + std::to_string(shell.appRegistry().count())
        + " registered apps");

    // Register the canonical primary tool roster (SceneEditor, AssetEditor,
    // MaterialEditor, AnimationEditor, DataEditor, VisualLogicEditor, BuildTool,
    // AtlasAI).  Factories are invoked by WorkspaceBootstrap during initialize().
    NF::registerCoreTools(shell);

    // Bootstrap the shell (validates config, invokes tool factories, initializes).
    NF::WorkspaceBootstrapConfig bootCfg;
    bootCfg.launchMode      = NF::WorkspaceStartupMode::Hosted;
    bootCfg.startupMessages = { "Workspace initialized." };
    // windowConfig defaults to {1280, 800, "Atlas Workspace"} — no override needed

    NF::WorkspaceBootstrap bootstrap;
    auto bootResult = bootstrap.run(bootCfg, shell);
    if (bootResult.failed()) {
        NF_LOG_ERROR("AtlasWorkspace",
            std::string("Bootstrap failed: ") + bootResult.errorName()
            + " — " + bootResult.errorDetail);
        NF::coreShutdown();
        return 1;
    }
    NF_LOG_INFO("AtlasWorkspace",
        std::string("Bootstrap complete. Tools: ")
        + std::to_string(bootResult.toolsRegistered));

    // ── Tool-specific command wiring ──────────────────────────────
    // Register real handlers for commands that need direct tool access.
    // These supplement the generic handlers created by registerAllToolCommands().
    {
        using namespace NF;

        // build.start — activate BuildTool and request a cmake build.
        // Note: build.start is NOT in BuildTool's descriptor.commands, so it has
        // no generic handler.  We register the authoritative handler here.
        if (auto* rawBuildTool = shell.toolRegistry().find(HostToolId::BuildTool)) {
            if (auto* bt = dynamic_cast<BuildTool*>(rawBuildTool)) {
                ConsoleCommand buildCmd("build.start", ConsoleCmdScope::Global,
                                       ConsoleCmdArgType::None);
                buildCmd.setDescription("Start a build of the active project");
                buildCmd.setEnabled(true);
                (void)shell.commandBus().registerCommand(buildCmd,
                    [&shell, bt]() -> ConsoleCmdExecResult {
                        shell.toolRegistry().activateTool(HostToolId::BuildTool);
                        bt->requestBuild();
                        return ConsoleCmdExecResult::Ok;
                    });
                NF_LOG_INFO("AtlasWorkspace", "build.start command wired to BuildTool");
            }
        }

        // tools.launch_atlas_ai — activate AtlasAITool.
        // This is a menu-level command not declared in AtlasAITool's descriptor.
        {
            ConsoleCommand aiCmd("tools.launch_atlas_ai", ConsoleCmdScope::Global,
                                  ConsoleCmdArgType::None);
            aiCmd.setDescription("Open the AtlasAI assistant");
            aiCmd.setEnabled(true);
            (void)shell.commandBus().registerCommand(aiCmd,
                [&shell]() -> ConsoleCmdExecResult {
                    shell.toolRegistry().activateTool(HostToolId::AtlasAI);
                    return ConsoleCmdExecResult::Ok;
                });
            NF_LOG_INFO("AtlasWorkspace", "tools.launch_atlas_ai command wired to AtlasAITool");
        }
    }

    // ── Preview runtime instances ─────────────────────────────────
    // All preview objects are stack-allocated here and passed by pointer to their tools.
    // The tools do NOT own these objects — they hold non-owning pointers.
    // Cleanup is automatic via stack unwinding after shell.shutdown() below.
    NovaForge::NovaForgePreviewRuntime novaPreviewRuntime;
    novaPreviewRuntime.start();

    NovaForge::NovaForgeAssetPreview    novaAssetPreview;
    NovaForge::NovaForgeMaterialPreview novaMaterialPreview;

    // ── Viewport wiring ───────────────────────────────────────────
    // Attach the workspace viewport manager to each tool that manages a
    // viewport slot.  The tools own the slot lifecycle via activate()/suspend().
    {
        using namespace NF;

        // Install the software renderer as a baseline — provides a visible
        // grid overlay into each viewport so the pipeline works end-to-end
        // even without a GPU backend (D3D11/OpenGL) connected yet.
        shell.viewportManager().installSoftwareRenderer();
        NF_LOG_INFO("AtlasWorkspace", "Software viewport renderer installed");

        // SceneEditorTool — primary 3D scene viewport.
        IHostedTool* rawSceneTool = shell.toolRegistry().find(HostToolId::SceneEditor);
        if (auto* sceneTool = dynamic_cast<SceneEditorTool*>(rawSceneTool)) {
            sceneTool->attachViewportManager(&shell.viewportManager());
            sceneTool->attachSceneProvider(&novaPreviewRuntime);
            // Activate SceneEditor as the default primary tool so it immediately
            // requests its viewport slot.
            shell.activateTool(HostToolId::SceneEditor);
            NF_LOG_INFO("AtlasWorkspace", "SceneEditorTool wired (viewport slot owned by tool)");
        }

        // AssetEditorTool — asset preview viewport.
        IHostedTool* rawAssetTool = shell.toolRegistry().find(HostToolId::AssetEditor);
        if (auto* assetTool = dynamic_cast<AssetEditorTool*>(rawAssetTool)) {
            assetTool->attachAssetPreviewProvider(&novaAssetPreview);
            NF_LOG_INFO("AtlasWorkspace", "AssetEditorTool preview provider wired");
        }

        // MaterialEditorTool — material preview viewport.
        IHostedTool* rawMatTool = shell.toolRegistry().find(HostToolId::MaterialEditor);
        if (auto* matTool = dynamic_cast<MaterialEditorTool*>(rawMatTool)) {
            matTool->attachMaterialPreviewProvider(&novaMaterialPreview);
            NF_LOG_INFO("AtlasWorkspace", "MaterialEditorTool preview provider wired");
        }

        // AnimationEditorTool — animation preview viewport.
        IHostedTool* rawAnimTool = shell.toolRegistry().find(HostToolId::AnimationEditor);
        if (auto* animTool = dynamic_cast<AnimationEditorTool*>(rawAnimTool)) {
            animTool->attachViewportManager(&shell.viewportManager());
            NF_LOG_INFO("AtlasWorkspace", "AnimationEditorTool viewport manager wired");
        }
    }

    // ── UI renderer ───────────────────────────────────────────────
    NF::UIRenderer ui;
    ui.init();

    // ── Input ─────────────────────────────────────────────────────
    NF::InputSystem input;
    input.init();

    // ── Frame controller ──────────────────────────────────────────
    NF::WorkspaceFrameController frameCtrl;
    frameCtrl.setTargetFPS(60.f);

    // ── WorkspacePanelHost + DockLayout ───────────────────────────
    // Created early so the objects live on the stack for the full session.
    // setRenderer() is called after ui.setBackend() below.
    NF::WorkspacePanelHost panelHost;
    NF::DockLayout         toolDock;
    toolDock.addPanel("Hierarchy",      NF::DockSlot::Left);
    toolDock.addPanel("Viewport",       NF::DockSlot::Center);
    toolDock.addPanel("Inspector",      NF::DockSlot::Right);
    toolDock.addPanel("ContentBrowser", NF::DockSlot::Bottom);
    // NullViewportSurface — headless surface registered so ViewportFrameLoop
    // has a valid bind/unbind target until a real GPU surface is attached.
    NF::NullViewportSurface viewportSurface(1280, 800);

#if defined(_WIN32)
    NF::WorkspaceRenderer wsRenderer;

    // GDI fallback backend — the active Windows render path.
    // See Docs/Canon/04_UI_BACKEND_STRATEGY.md for the D3D11 target direction.
    NF::GDIBackend gdiBackend;
    gdiBackend.init(1280, 800);
    ui.setBackend(&gdiBackend);

    // Win32LaunchService: spawns real child processes from the binary directory.
    // Falls back to NullLaunchService on non-Windows platforms.
    NF::Win32LaunchService win32LaunchSvc(binDir);

    g_shell       = &shell;
    g_renderer    = &wsRenderer;
    g_ui          = &ui;
    g_gdiBackend  = &gdiBackend;
    g_inputSystem = &input;
    g_launchSvc   = &win32LaunchSvc;

    NF::Win32InputAdapter inputAdapter(input);
    g_inputAdapter = &inputAdapter;

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WorkspaceWndProc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"AtlasWorkspaceWnd";
    RegisterClassExW(&wc);

    RECT wr{0, 0, 1280, 800};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExW(0, L"AtlasWorkspaceWnd",
        L"Atlas Workspace",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, wc.hInstance, nullptr);

    gdiBackend.setWindowHandle(hwnd);
    inputAdapter.setWindowHandle(hwnd);  // enable mouse capture on click
    DragAcceptFiles(hwnd, TRUE);          // enable WM_DROPFILES for asset drag-and-drop
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    NF_LOG_INFO("AtlasWorkspace", "Workspace window created (1280x800)");
#else
    (void)binDir;
    NF::NullBackend nullBackend;
    nullBackend.init(1280, 800);
    ui.setBackend(&nullBackend);
#endif

    // ── Finalise WorkspacePanelHost wiring ────────────────────────
    // Must happen after ui.setBackend() so DrawListDispatcher has a backend.
    panelHost.setRenderer(&ui);
    g_panelHost = &panelHost;
    g_toolDock  = &toolDock;

    // Connect ViewportPanel → WorkspaceViewportBridge → ViewportFrameLoop.
    // The NullViewportSurface gives the frame loop a valid bind/unbind target
    // on all platforms, including those without a real GPU surface attached yet.
    // The handle is obtained from the SceneEditorTool (which owns the slot).
    {
        NF::ViewportHandle sceneVh = NF::kInvalidViewportHandle;
        if (auto* rawTool = shell.toolRegistry().find(NF::HostToolId::SceneEditor)) {
            if (auto* sceneTool = dynamic_cast<NF::SceneEditorTool*>(rawTool)) {
                sceneVh = sceneTool->viewportHandle();
            }
        }
        if (sceneVh != NF::kInvalidViewportHandle) {
            NF::WorkspaceViewportBridge::connect(
                &panelHost.viewport(), shell.viewportManager(),
                sceneVh, &viewportSurface);
            NF_LOG_INFO("AtlasWorkspace",
                "WorkspaceViewportBridge connected (handle=" + std::to_string(sceneVh) + ")");
        }
    }

    // Attach input system to the SceneEditorTool so the fly-camera
    // controller can read WASD and mouse input each frame.
    if (auto* rawTool = shell.toolRegistry().find(NF::HostToolId::SceneEditor)) {
        if (auto* sceneTool = dynamic_cast<NF::SceneEditorTool*>(rawTool)) {
            sceneTool->onAttachInput(&input);
            NF_LOG_INFO("AtlasWorkspace", "SceneEditorTool input attached (fly-cam enabled)");
        }
    }

    // Wire HierarchyPanel entity-selection callback → SceneEditorTool + SelectionService.
    // When the user clicks an entity row in the Hierarchy panel, this callback:
    //  1. Updates SceneEditorTool::m_viewSelectedEntity so the Inspector shows the
    //     correct transform on the next syncPanels() call (one-frame update latency).
    //  2. Notifies the SelectionService so other panels and tools can observe it.
    {
        NF::SceneEditorTool* sceneToolPtr = nullptr;
        if (auto* rawTool = shell.toolRegistry().find(NF::HostToolId::SceneEditor))
            sceneToolPtr = dynamic_cast<NF::SceneEditorTool*>(rawTool);

        if (sceneToolPtr) {
            // Capture raw pointers — both sceneToolPtr and &shell outlive the callback
            // (panelHost, which owns the callback, is destroyed before shell).
            panelHost.hierarchy().setOnSelect(
                [sceneToolPtr, &shell](int entityId) {
                    // entityId from HierarchyPanel is 1-based (#id in the entry).
                    // m_viewSelectedEntity is 0-based (array index).
                    sceneToolPtr->setViewSelectedEntity(entityId - 1);
                    shell.selectionService().selectExclusive(
                        static_cast<NF::EntityID>(entityId));
                });
            NF_LOG_INFO("AtlasWorkspace", "HierarchyPanel selection callback wired");
        }
    }

    NF_LOG_INFO("AtlasWorkspace", "Workspace ready — entering main loop");

    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running  = true;

    while (running) {
#if defined(_WIN32)
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;

        // Handle any UI actions or errors that were queued during WM_PAINT.
        // These are processed here — outside WM_PAINT — so that modal dialogs
        // (SHBrowseForFolderW, GetOpenFileNameW, MessageBoxW) can open safely
        // without creating a nested message loop inside the paint callback.
        handlePendingWorkspaceActions(*g_renderer, *g_shell, hwnd);
#else
        running = false; // headless: single frame then exit
#endif
        auto now  = std::chrono::high_resolution_clock::now();
        float rawDt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        auto fr = frameCtrl.beginFrame(rawDt);

        input.update();
        shell.update(fr.dt);

        // Tick the viewport frame loop — executes the software renderer callback
        // for each active slot and returns per-slot results.  Results are forwarded
        // to ViewportPanel::setColorAttachment() so the panel blits any live texture.
        {
            auto frameResults = shell.viewportManager().renderFrame();
            if (g_panelHost)
                NF::WorkspaceViewportBridge::forwardFrameResults(
                    &g_panelHost->viewport(), frameResults);
        }

        frameCtrl.markUpdateDone();

#if defined(_WIN32)
        if (g_renderer) g_renderer->setLastFrameMs(fr.dt * 1000.f);
        InvalidateRect(hwnd, nullptr, FALSE);

        // Sleep to pace to ~60 FPS (render happens in WM_PAINT).
        float sleepMs = frameCtrl.sleepMs(rawDt * 1000.f);
        if (sleepMs > 1.f)
            Sleep(static_cast<DWORD>(sleepMs));
#endif
        frameCtrl.markRenderDone();
        frameCtrl.endFrame();
    }

#if defined(_WIN32)
    gdiBackend.shutdown();
    g_shell       = nullptr;
    g_renderer    = nullptr;
    g_ui          = nullptr;
    g_inputAdapter = nullptr;
    g_inputSystem  = nullptr;
    g_launchSvc    = nullptr;
    g_gdiBackend  = nullptr;
    g_panelHost   = nullptr;
    g_toolDock    = nullptr;
#endif

    // Detach input from tools before shutting down the input system.
    if (auto* rawTool = shell.toolRegistry().find(NF::HostToolId::SceneEditor)) {
        if (auto* sceneTool = dynamic_cast<NF::SceneEditorTool*>(rawTool))
            sceneTool->onDetachInput();
    }

    input.shutdown();
    shell.shutdown();
    ui.shutdown();
#if defined(_WIN32)
    CoUninitialize();
#endif
    NF::coreShutdown();
    return 0;
}
