#pragma once
#include "IToolModule.h"
#include <cstdint>
#include <string>

namespace atlas::editor {

/// Abstract interface for editor tool modules.
///
/// Every editor tool (tile editor, modeling, animation, dialogue, etc.)
/// implements this interface.  The editor kernel discovers and loads
/// tool modules at startup, and standalone tool executables can load
/// a single module into a minimal editor shell.
///
/// IEditorToolModule extends the engine's IToolModule with editor-
/// specific registration hooks (panels, menus, modes).  This lets the
/// same tool module run inside the standalone editor *or* inside the
/// in-game ToolingLayer without modification.
///
/// This is the single most important editor architecture interface.
/// If a tool cannot be expressed through IEditorToolModule, the
/// interface must be extended — not bypassed.
class IEditorToolModule : public atlas::tools::IToolModule {
public:
    ~IEditorToolModule() override = default;

    /// Register any UI panels this tool provides.
    virtual void RegisterPanels() = 0;

    /// Register menu items this tool adds to the editor menu bar.
    virtual void RegisterMenus() = 0;

    /// Register editor modes this tool introduces.
    virtual void RegisterModes() = 0;
};

} // namespace atlas::editor
