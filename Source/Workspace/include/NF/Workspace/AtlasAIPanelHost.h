#pragma once
// NF::Editor — AtlasAI panel host: docking host that manages the AI panel lifecycle
#include "NF/Workspace/AIAssistantPanel.h"
#include "NF/Workspace/NotificationSystem.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── AI Panel Visibility ───────────────────────────────────────────

enum class AIPanelVisibility : uint8_t {
    Hidden,
    Slideout,    // partial reveal from screen edge
    Docked,      // integrated in dock layout
    Floating,    // undocked, free-floating
    Fullscreen,
};

inline const char* aiPanelVisibilityName(AIPanelVisibility v) {
    switch (v) {
        case AIPanelVisibility::Hidden:     return "Hidden";
        case AIPanelVisibility::Slideout:   return "Slideout";
        case AIPanelVisibility::Docked:     return "Docked";
        case AIPanelVisibility::Floating:   return "Floating";
        case AIPanelVisibility::Fullscreen: return "Fullscreen";
    }
    return "Unknown";
}

// ── AI Panel Dock Edge ────────────────────────────────────────────

enum class AIPanelEdge : uint8_t { Right, Bottom, Left };

inline const char* aiPanelEdgeName(AIPanelEdge e) {
    switch (e) {
        case AIPanelEdge::Right:  return "Right";
        case AIPanelEdge::Bottom: return "Bottom";
        case AIPanelEdge::Left:   return "Left";
    }
    return "Unknown";
}

// ── Atlas AI Panel Host ───────────────────────────────────────────
// Manages showing, hiding, resizing, and lifecycle of the AI panel
// within the workspace shell.

class AtlasAIPanelHost {
public:
    explicit AtlasAIPanelHost() = default;

    void initialize() {
        m_initialized = true;
        m_visibility  = AIPanelVisibility::Hidden;
    }

    void show(AIPanelVisibility mode = AIPanelVisibility::Slideout) {
        m_visibility = mode;
        ++m_showCount;
    }

    void hide() {
        m_visibility = AIPanelVisibility::Hidden;
        ++m_hideCount;
    }

    void toggle() {
        if (m_visibility == AIPanelVisibility::Hidden) show();
        else hide();
    }

    void setDockEdge(AIPanelEdge edge)    { m_edge = edge; }
    void setWidthPx(float w)              { m_widthPx  = w > 80.f ? w : 80.f; }
    void setHeightPx(float h)             { m_heightPx = h > 80.f ? h : 80.f; }

    void attachPanel(AIAssistantPanel* panel) { m_panel = panel; }

    // Route an error notification to AI for triage
    bool escalateToAI(const Notification& n) {
        if (!m_panel) return false;
        if (n.severity < NotificationSeverity::Error) return false;

        std::string msg = "[Escalated] " + std::string(notificationSeverityName(n.severity))
                        + ": " + n.title;
        if (!n.message.empty()) msg += " — " + n.message;
        m_panel->session().addSystemMessage(msg);
        ++m_escalatedCount;

        if (m_visibility == AIPanelVisibility::Hidden)
            show(AIPanelVisibility::Slideout);
        return true;
    }

    [[nodiscard]] bool                isInitialized()  const { return m_initialized; }
    [[nodiscard]] AIPanelVisibility   visibility()     const { return m_visibility;  }
    [[nodiscard]] AIPanelEdge         dockEdge()       const { return m_edge;        }
    [[nodiscard]] float               widthPx()        const { return m_widthPx;     }
    [[nodiscard]] float               heightPx()       const { return m_heightPx;    }
    [[nodiscard]] bool                isVisible()      const { return m_visibility != AIPanelVisibility::Hidden; }
    [[nodiscard]] AIAssistantPanel*   panel()          const { return m_panel;       }
    [[nodiscard]] size_t              showCount()      const { return m_showCount;   }
    [[nodiscard]] size_t              hideCount()      const { return m_hideCount;   }
    [[nodiscard]] size_t              escalatedCount() const { return m_escalatedCount; }

private:
    AIAssistantPanel* m_panel      = nullptr;
    AIPanelVisibility m_visibility = AIPanelVisibility::Hidden;
    AIPanelEdge       m_edge       = AIPanelEdge::Right;
    float             m_widthPx    = 380.f;
    float             m_heightPx   = 600.f;
    bool              m_initialized = false;
    size_t            m_showCount  = 0;
    size_t            m_hideCount  = 0;
    size_t            m_escalatedCount = 0;
};

} // namespace NF
