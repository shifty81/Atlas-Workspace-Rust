#pragma once
// NF::PIEViewportBridge — Wires PIEService ↔ WorkspaceViewportManager for PIE.
//
// When the user presses Play, the bridge:
//   1. Unregisters the current editor scene provider from the viewport slot.
//   2. Registers PIEGameSceneProvider as the active scene source.
//   3. Installs a camera override pointing at the game spawn point.
//
// On PIE exit the original editor provider is restored.
//
// Usage:
//   PIEViewportBridge bridge;
//   bridge.connect(pieService, viewportMgr, handle,
//                  &editorProvider, &gameProvider);
//   // ... later:
//   bridge.disconnect();
//
// The bridge installs four callbacks on PIEService (enter/exit/pause/resume).
// Calling connect() again without a prior disconnect() replaces the bindings.
//
// Thread safety: single-threaded; must be called from the editor main thread.
//
// Phase 73 — PIE Viewport Full Integration

#include "NF/Editor/PIEService.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include "NF/Workspace/PIEGameSceneProvider.h"
#include "NF/Workspace/WorkspaceViewportManager.h"
#include <functional>
#include <string>

namespace NF {

// ── PIEViewportMode ────────────────────────────────────────────────────────

/// Tracks which scene source is currently wired to the viewport slot.
enum class PIEViewportMode : uint8_t {
    Editor,  ///< Editor scene provider is active (PIE not running)
    Game,    ///< Game scene provider is active (PIE running or paused)
};

inline const char* pieViewportModeName(PIEViewportMode m) {
    switch (m) {
    case PIEViewportMode::Editor: return "Editor";
    case PIEViewportMode::Game:   return "Game";
    }
    return "Unknown";
}

// ── PIEViewportBridge ──────────────────────────────────────────────────────

class PIEViewportBridge {
public:
    using SimpleCallback = std::function<void()>;

    PIEViewportBridge()  = default;
    ~PIEViewportBridge() { disconnect(); }

    // ── connect ────────────────────────────────────────────────────────────
    //
    // Wire the bridge.  All pointer parameters are borrowed references —
    // the bridge does NOT take ownership of any of them.
    //
    // @param pieService     PIEService to observe (callbacks installed here)
    // @param viewportMgr    WorkspaceViewportManager that owns the slot
    // @param handle         Viewport slot to switch during PIE enter/exit
    // @param editorProvider Scene provider to use in editor mode (may be null)
    // @param gameProvider   Game scene provider to use during PIE
    //
    // Returns false if handle is kInvalidViewportHandle or gameProvider is null.
    bool connect(PIEService&                pieService,
                 WorkspaceViewportManager&  viewportMgr,
                 ViewportHandle             handle,
                 IViewportSceneProvider*    editorProvider,
                 PIEGameSceneProvider*      gameProvider) {
        if (handle == kInvalidViewportHandle) return false;
        if (!gameProvider) return false;

        disconnect(); // clear any previous binding

        m_pieService     = &pieService;
        m_viewportMgr    = &viewportMgr;
        m_handle         = handle;
        m_editorProvider = editorProvider;
        m_gameProvider   = gameProvider;
        m_viewportMode   = PIEViewportMode::Editor;

        // Install PIE lifecycle callbacks.
        pieService.setOnEnter([this]() { onPIEEnter(); });
        pieService.setOnExit ([this]() { onPIEExit();  });
        pieService.setOnPause([this]() { onPIEPause(); });
        pieService.setOnResume([this]() { onPIEResume(); });

        m_connected = true;
        return true;
    }

    // ── disconnect ─────────────────────────────────────────────────────────
    // Remove PIE callbacks and restore editor provider.  Safe to call even
    // if not connected.
    void disconnect() {
        if (!m_connected) return;

        // If PIE was active, force the game provider off and restore editor.
        if (m_viewportMode == PIEViewportMode::Game) {
            swapToEditorProvider();
            if (m_gameProvider) m_gameProvider->exitGame();
        }

        // Clear PIE callbacks so the PIEService no longer holds our lambdas.
        if (m_pieService) {
            m_pieService->setOnEnter(nullptr);
            m_pieService->setOnExit (nullptr);
            m_pieService->setOnPause(nullptr);
            m_pieService->setOnResume(nullptr);
        }

        m_pieService     = nullptr;
        m_viewportMgr    = nullptr;
        m_handle         = kInvalidViewportHandle;
        m_editorProvider = nullptr;
        m_gameProvider   = nullptr;
        m_connected      = false;
        m_viewportMode   = PIEViewportMode::Editor;
    }

    // ── Accessors ──────────────────────────────────────────────────────────

    [[nodiscard]] bool            isConnected()   const { return m_connected; }
    [[nodiscard]] PIEViewportMode viewportMode()  const { return m_viewportMode; }
    [[nodiscard]] ViewportHandle  handle()        const { return m_handle; }

    [[nodiscard]] bool isGameMode()   const { return m_viewportMode == PIEViewportMode::Game; }
    [[nodiscard]] bool isEditorMode() const { return m_viewportMode == PIEViewportMode::Editor; }

    // ── Optional user callbacks ────────────────────────────────────────────
    // Fired after each mode transition.

    void setOnSwitchToGame  (SimpleCallback cb) { m_onSwitchToGame   = std::move(cb); }
    void setOnSwitchToEditor(SimpleCallback cb) { m_onSwitchToEditor = std::move(cb); }

    [[nodiscard]] uint32_t switchCount() const { return m_switchCount; }

private:
    // ── Internal transition helpers ────────────────────────────────────────

    void swapToGameProvider() {
        if (!m_viewportMgr || !m_gameProvider) return;

        const std::string& toolId = toolIdForHandle();

        // Remove editor provider if one is registered.
        if (m_editorProvider)
            m_viewportMgr->sceneRegistry().unregisterProvider(toolId);

        // Register game provider.
        m_viewportMgr->sceneRegistry().registerProvider(toolId, m_gameProvider);
        m_viewportMode = PIEViewportMode::Game;
        ++m_switchCount;

        if (m_onSwitchToGame) m_onSwitchToGame();
    }

    void swapToEditorProvider() {
        if (!m_viewportMgr) return;

        const std::string& toolId = toolIdForHandle();

        // Remove game provider.
        m_viewportMgr->sceneRegistry().unregisterProvider(toolId);

        // Restore editor provider if any.
        if (m_editorProvider)
            m_viewportMgr->sceneRegistry().registerProvider(toolId, m_editorProvider);

        m_viewportMode = PIEViewportMode::Editor;
        ++m_switchCount;

        if (m_onSwitchToEditor) m_onSwitchToEditor();
    }

    [[nodiscard]] const std::string& toolIdForHandle() const {
        if (!m_viewportMgr) {
            static const std::string kEmpty;
            return kEmpty;
        }
        const ViewportSlot* slot = m_viewportMgr->findSlot(m_handle);
        if (!slot) {
            static const std::string kEmpty;
            return kEmpty;
        }
        return slot->toolId;
    }

    // ── PIE callback handlers ──────────────────────────────────────────────

    void onPIEEnter() {
        if (m_gameProvider) m_gameProvider->enterGame();
        swapToGameProvider();
    }

    void onPIEExit() {
        swapToEditorProvider();
        if (m_gameProvider) m_gameProvider->exitGame();
    }

    void onPIEPause() {
        if (m_gameProvider) m_gameProvider->pauseGame();
    }

    void onPIEResume() {
        if (m_gameProvider) m_gameProvider->resumeGame();
    }

    // ── Members ────────────────────────────────────────────────────────────

    PIEService*               m_pieService     = nullptr;
    WorkspaceViewportManager* m_viewportMgr    = nullptr;
    ViewportHandle            m_handle         = kInvalidViewportHandle;
    IViewportSceneProvider*   m_editorProvider = nullptr;
    PIEGameSceneProvider*     m_gameProvider   = nullptr;

    PIEViewportMode           m_viewportMode   = PIEViewportMode::Editor;
    bool                      m_connected      = false;
    uint32_t                  m_switchCount    = 0;

    SimpleCallback            m_onSwitchToGame;
    SimpleCallback            m_onSwitchToEditor;
};

} // namespace NF
