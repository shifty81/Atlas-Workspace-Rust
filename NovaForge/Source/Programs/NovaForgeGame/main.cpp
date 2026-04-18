// NovaForge Game — Standalone game client
// Accepts launch arguments from AtlasWorkspace via WorkspaceLaunchContract:
//   --hosted              (launched by Workspace; default)
//   --headless            (no window; CI / server-side testing)
//   --project=<path>      (project directory or .atlas file path)
//   --workspace-root=<path>
//   --session-id=<id>
// Uses the unified UIRenderer pipeline for HUD rendering.
#include "NF/Core/Core.h"
#include "NF/Game/Game.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/UI/UIBackend.h"
#include "NF/UI/UIWidgets.h"
#include "NF/Input/Input.h"
#include <string>
#include <string_view>
#ifdef _WIN32
#  include "NF/Input/Win32InputAdapter.h"
#  include "NF/UI/GDIBackend.h"
#  include <windows.h>
#  include <chrono>
#endif

#ifdef _WIN32
static NF::InputSystem* g_input = nullptr;
static NF::Win32InputAdapter* g_adapter = nullptr;
static NF::UIRenderer* g_uiRenderer = nullptr;
static NF::GDIBackend* g_gdiBackend = nullptr;
static int g_w = 1920, g_h = 1080;

// Game HUD state
static float g_hp = 50.f, g_maxHp = 100.f;
static float g_energy = 70.f, g_maxEnergy = 100.f;

static void renderGameHUD(NF::UIRenderer& ui, int w, int h) {
    float fw = static_cast<float>(w);
    float fh = static_cast<float>(h);

    ui.beginFrame(fw, fh);

    // World background (dark space)
    ui.drawRect({0.f, 0.f, fw, fh}, 0x05080CFF);

    // Crosshair
    float cx = fw * 0.5f, cy = fh * 0.5f;
    ui.drawRect({cx - 10.f, cy - 0.5f, 20.f, 1.f}, 0xC8C8C8FF);  // horizontal
    ui.drawRect({cx - 0.5f, cy - 10.f, 1.f, 20.f}, 0xC8C8C8FF);  // vertical

    // HP bar (bottom-left)
    float barW = 200.f, barH = 16.f;
    float barX = 20.f, barY = fh - 60.f;
    ui.drawRect({barX, barY, barW, barH}, 0x281414FF);  // dark red bg
    float hpFill = barW * (g_hp / g_maxHp);
    ui.drawRect({barX, barY, hpFill, barH}, 0xB41E1EFF);  // red fill
    ui.drawRectOutline({barX, barY, barW, barH}, 0x555555FF, 1.f);
    ui.drawText(barX + 4.f, barY + 1.f, "HP", 0xDCDCDCFF);
    char hpText[16];
    std::snprintf(hpText, sizeof(hpText), "%.0f/%.0f", g_hp, g_maxHp);
    ui.drawText(barX + barW + 8.f, barY + 1.f, hpText, 0xDCDCDCFF);

    // Energy bar
    float enY = fh - 38.f;
    ui.drawRect({barX, enY, barW, barH}, 0x141432FF);  // dark blue bg
    float enFill = barW * (g_energy / g_maxEnergy);
    ui.drawRect({barX, enY, enFill, barH}, 0x1E50C8FF);  // blue fill
    ui.drawRectOutline({barX, enY, barW, barH}, 0x555555FF, 1.f);
    ui.drawText(barX + 4.f, enY + 1.f, "EN", 0xDCDCDCFF);
    char enText[16];
    std::snprintf(enText, sizeof(enText), "%.0f/%.0f", g_energy, g_maxEnergy);
    ui.drawText(barX + barW + 8.f, enY + 1.f, enText, 0xDCDCDCFF);

    // Version watermark (bottom-right)
    std::string verText = std::string("NovaForge v") + NF::NF_VERSION_STRING;
    float verW = static_cast<float>(verText.size()) * 8.f;
    ui.drawText(fw - verW - 12.f, fh - 20.f, verText, 0x323232FF);

    // Mini-map placeholder (top-right)
    float mmSize = 120.f;
    ui.drawRect({fw - mmSize - 12.f, 12.f, mmSize, mmSize}, 0x1A1A1AFF);
    ui.drawRectOutline({fw - mmSize - 12.f, 12.f, mmSize, mmSize}, 0x555555FF, 1.f);
    ui.drawText(fw - mmSize - 4.f, 16.f, "Map", 0x888888FF);

    ui.endFrame();
}

static LRESULT CALLBACK GameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_adapter) g_adapter->processMessage(hwnd,msg,wParam,lParam);
    switch(msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc=BeginPaint(hwnd,&ps);

        if (g_uiRenderer && g_gdiBackend) {
            g_gdiBackend->setTargetDC(hdc);
            g_gdiBackend->beginFrame(g_w, g_h);
            renderGameHUD(*g_uiRenderer, g_w, g_h);
            g_gdiBackend->endFrame();
        }

        EndPaint(hwnd,&ps); return 0;
    }
    case WM_SIZE: g_w=LOWORD(lParam); g_h=HIWORD(lParam); InvalidateRect(hwnd,nullptr,FALSE); return 0;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd,msg,wParam,lParam);
}
#endif

int main(int argc, char* argv[]) {
    // ── CLI argument parsing ──────────────────────────────────────
    bool        headless      = false;
    std::string projectPath;
    std::string workspaceRoot;
    std::string sessionId;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg == "--headless") {
            headless = true;
        } else if (arg == "--hosted") {
            // default mode; no-op
        } else if (arg.substr(0, 10) == "--project=") {
            projectPath = std::string(arg.substr(10));
        } else if (arg.substr(0, 17) == "--workspace-root=") {
            workspaceRoot = std::string(arg.substr(17));
        } else if (arg.substr(0, 13) == "--session-id=") {
            sessionId = std::string(arg.substr(13));
        }
        // Unknown args are silently ignored to allow forward-compat.
    }

    NF::coreInit();
    NF_LOG_INFO("Main","=== NovaForge Game ===");
    NF_LOG_INFO("Main", std::string("Version: ") + NF::NF_VERSION_STRING);
    if (!projectPath.empty())
        NF_LOG_INFO("Main", "Project: " + projectPath);
    if (!sessionId.empty())
        NF_LOG_INFO("Main", "Session: " + sessionId);
    if (headless)
        NF_LOG_INFO("Main", "Mode: headless");

    NF::Renderer renderer;
    if (!renderer.init(1920, 1080)) {
        NF_LOG_ERROR("Main","Failed to initialize renderer"); return 1;
    }

    // ── Game session ──────────────────────────────────────────────
    NF::GameSession gameSession;
    gameSession.init(42);

    NF::UIRenderer uiRenderer;
    uiRenderer.init();

    NF::InputSystem input;
    input.init();

    // ── Backend setup ────────────────────────────────────────────
#ifdef _WIN32
    NF::GDIBackend gdiBackend;
    gdiBackend.init(1280, 720);
    uiRenderer.setBackend(&gdiBackend);
    g_uiRenderer = &uiRenderer;
    g_gdiBackend = &gdiBackend;
    NF_LOG_INFO("Main", "GDI backend active for game HUD");
#else
    NF::NullBackend nullBackend;
    nullBackend.init(1280, 720);
    uiRenderer.setBackend(&nullBackend);
    NF_LOG_INFO("Main", "Null backend active (headless)");
#endif

#ifdef _WIN32
    g_input = &input;
    NF::Win32InputAdapter adapter(input);
    g_adapter = &adapter;

    HWND hwnd = nullptr;
    if (!headless) {
        WNDCLASSEXW wc{};
        wc.cbSize=sizeof(wc); wc.style=CS_HREDRAW|CS_VREDRAW;
        wc.lpfnWndProc=GameWndProc; wc.hInstance=GetModuleHandleW(nullptr);
        wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
        wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
        wc.lpszClassName=L"NovaForgeGameWnd";
        RegisterClassExW(&wc);
        RECT wr{0,0,1280,720}; AdjustWindowRect(&wr,WS_OVERLAPPEDWINDOW,FALSE);
        std::wstring gameTitle = std::wstring(L"NovaForge  v") + std::wstring(NF::NF_VERSION_STRING, NF::NF_VERSION_STRING + strlen(NF::NF_VERSION_STRING));
        hwnd=CreateWindowExW(0,L"NovaForgeGameWnd",gameTitle.c_str(),
            WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,
            wr.right-wr.left,wr.bottom-wr.top,nullptr,nullptr,wc.hInstance,nullptr);

        gdiBackend.setWindowHandle(hwnd);
        ShowWindow(hwnd,SW_SHOW); UpdateWindow(hwnd);
        NF_LOG_INFO("Main","Game window created");
    } else {
        NF_LOG_INFO("Main","Headless mode — no window created");
    }
#endif

    NF_LOG_INFO("Main","Game ready — entering main loop");
    auto last=std::chrono::high_resolution_clock::now();
    bool running=true;
    while(running){
#ifdef _WIN32
        if (headless) {
            // Headless mode: tick game logic for a few frames then exit.
            // In a real session the workspace would control lifetime via IPC.
            auto now=std::chrono::high_resolution_clock::now();
            float dt=std::chrono::duration<float>(now-last).count();
            last=now; if(dt>0.1f)dt=0.1f;
            gameSession.tick(dt);
            Sleep(16);
            running=false; // single-frame headless run
        } else {
            MSG msg{};
            while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){
                if(msg.message==WM_QUIT){running=false;break;}
                TranslateMessage(&msg); DispatchMessageW(&msg);
            }
            if(!running) break;
            auto now=std::chrono::high_resolution_clock::now();
            float dt=std::chrono::duration<float>(now-last).count();
            last=now; if(dt>0.1f)dt=0.1f;
            gameSession.tick(dt);
            input.update();
            InvalidateRect(hwnd,nullptr,FALSE);
            Sleep(16);
        }
#else
        // Non-Windows headless path
        auto now=std::chrono::high_resolution_clock::now();
        float dt=std::chrono::duration<float>(now-last).count();
        last=now; if(dt>0.1f)dt=0.1f;
        gameSession.tick(dt);
        running=false;
#endif
    }

#ifdef _WIN32
    gdiBackend.shutdown();
    g_input=nullptr; g_adapter=nullptr;
    g_uiRenderer=nullptr; g_gdiBackend=nullptr;
#endif
    uiRenderer.shutdown();
    input.shutdown();
    renderer.shutdown();
    NF::coreShutdown();
    return 0;
}
