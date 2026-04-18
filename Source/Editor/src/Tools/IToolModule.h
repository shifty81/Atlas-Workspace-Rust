#pragma once
#include <cstdint>
#include <string>

namespace atlas::tools {

/// Engine-level abstract interface for tool modules.
///
/// This is the engine-side counterpart of the editor's IEditorToolModule.
/// By living in the engine, any executable (editor, runtime, game client)
/// can host tool modules through the ToolingLayer without depending on
/// the editor target.  The editor's IEditorToolModule extends this
/// interface with editor-specific hooks.
class IToolModule {
public:
    virtual ~IToolModule() = default;

    /// Human-readable name of this tool (e.g. "Tile Editor").
    virtual std::string Name() const = 0;

    /// Called when the module is registered with the ToolingLayer.
    virtual void OnRegister() = 0;

    /// Called when the module is about to be unregistered.
    virtual void OnUnregister() = 0;

    /// Handle an input event.  Return true if the tool consumed it.
    virtual bool HandleInput(uint32_t keyCode, bool pressed) = 0;

    /// Per-frame update for the tool.
    virtual void Update(float deltaTime) = 0;

    /// Called when the tool should render its content.
    virtual void Render() = 0;
};

} // namespace atlas::tools
