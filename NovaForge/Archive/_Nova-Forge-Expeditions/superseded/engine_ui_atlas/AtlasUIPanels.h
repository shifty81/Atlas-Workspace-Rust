#pragma once
#include "AtlasUI.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace atlas::ui::atlas {

/// Persistent panel state tracked across frames.
struct PanelState {
    std::string title;
    AtlasRect   rect       = {50, 50, 300, 200};
    bool        open       = true;
    bool        collapsed  = false;
    bool        dragging   = false;
    float       dragOffX   = 0;
    float       dragOffY   = 0;
    int32_t     zOrder     = 0;

    /// Content-area rectangle (excludes title bar).
    AtlasRect ContentRect(int32_t titleBarH) const {
        return {rect.x, rect.y + titleBarH,
                rect.w, rect.h - titleBarH};
    }
};

/// Panel manager — tracks open panels, handles dragging, Z-ordering,
/// and provides Begin/End panel calls for immediate-mode usage.
///
///   panelMgr.BeginPanel(ctx, "Inventory", 300, 200);
///   // ... draw widgets inside ...
///   panelMgr.EndPanel(ctx);
///
class AtlasPanelManager {
public:
    AtlasPanelManager();
    ~AtlasPanelManager();

    /// Begin a panel.  Returns true if the panel is open and visible
    /// (if false, skip widget calls).  `defaultW` / `defaultH` are
    /// used only on first creation.
    bool BeginPanel(AtlasUIContext& ctx, const std::string& title,
                    float defaultW = 300, float defaultH = 200);

    /// End the current panel (must pair with BeginPanel).
    void EndPanel(AtlasUIContext& ctx);

    /// Close a panel by title.
    void ClosePanel(const std::string& title);

    /// Open / show a panel by title.
    void OpenPanel(const std::string& title);

    /// Toggle panel visibility.
    void TogglePanel(const std::string& title);

    /// Check if a panel exists and is open.
    bool IsPanelOpen(const std::string& title) const;

    /// Get the number of tracked panels.
    size_t PanelCount() const { return m_panels.size(); }

    /// Move a panel to the given position.
    void SetPanelPosition(const std::string& title, float x, float y);

    /// Retrieve panel state (returns nullptr if not found).
    const PanelState* GetPanel(const std::string& title) const;

private:
    std::unordered_map<std::string, PanelState> m_panels;
    std::string m_currentPanel;  // title of panel inside Begin/End
    int32_t m_nextZOrder = 0;

    PanelState& GetOrCreate(const std::string& title, float w, float h);
    void BringToFront(PanelState& panel);
};

} // namespace atlas::ui::atlas
