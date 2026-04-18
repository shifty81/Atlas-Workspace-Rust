#pragma once
#include <string>

namespace atlas::tools {

/// Lightweight in-client editor tool interface.
///
/// Unlike IEditorToolModule (editor kernel tools with panels, menus, and
/// modes), ITool describes a single, toggleable in-client overlay tool
/// that can be compiled out via ATLAS_EDITOR_TOOLS.
///
/// Every ITool implementation must be stateless across activate/deactivate
/// cycles — any persistent state should be stored externally (e.g. in a
/// SceneBookmarkManager or LayerTagSystem).
class ITool {
public:
    virtual ~ITool() = default;

    /// Human-readable name of this tool.
    virtual std::string Name() const = 0;

    /// Called when the tool is activated (e.g. selected from tool palette).
    virtual void Activate() = 0;

    /// Called when the tool is deactivated.
    virtual void Deactivate() = 0;

    /// Per-frame update while the tool is active.
    virtual void Update(float deltaTime) = 0;

    /// Whether the tool is currently active.
    virtual bool IsActive() const = 0;
};

} // namespace atlas::tools
