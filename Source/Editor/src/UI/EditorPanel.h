#pragma once
#include <string>
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"

namespace atlas::editor {

struct DockRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

/// Returns the global AtlasContext, or nullptr in headless/test mode.
inline atlas::AtlasContext* GetContext() { return nullptr; }

/// Apply dock bounds from EditorPanel to a PanelState (no-op if no context).
inline void ApplyDockBounds(atlas::PanelState&) {}

class EditorPanel {
public:
    virtual ~EditorPanel() = default;
    virtual const char* Name() const = 0;
    virtual void Draw() = 0;
    virtual bool IsVisible() const { return m_visible; }
    virtual void SetVisible(bool visible) { m_visible = visible; }

    /// Called when an asset is reloaded on disk.
    virtual void OnAssetReloaded(const std::string& assetId,
                                 const std::string& path) { (void)assetId; (void)path; }

    /// Returns true if this panel cannot be closed by the user.
    /// Non-closable panels may be collapsed or tabbed but never removed
    /// from the dock hierarchy.
    bool IsClosable() const { return m_closable; }
    void SetClosable(bool closable) { m_closable = closable; }

    bool HasDockBounds() const { return m_hasDockBounds; }
    const DockRect& DockBounds() const { return m_dockBounds; }
    void SetDockBounds(const DockRect& bounds) { m_dockBounds = bounds; m_hasDockBounds = true; }

private:
    bool m_visible = true;
    bool m_closable = true;
    bool m_hasDockBounds = false;
    DockRect m_dockBounds;
};

}
