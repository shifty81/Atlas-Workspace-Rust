#pragma once
#include "IToolModule.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace atlas::tools {

/// Central registry for tool modules.
///
/// The ToolModuleRegistry owns a set of IToolModule instances and manages
/// their lifecycle (register → OnRegister / unregister → OnUnregister).
/// It is used by both the standalone editor executable and the in-game
/// ToolingLayer so that tools can be shared across all runtime modes.
class ToolModuleRegistry {
public:
    /// Register a tool module.  Calls OnRegister() on success.
    /// Returns false if a module with the same name is already registered.
    bool Register(std::unique_ptr<IToolModule> module);

    /// Unregister a tool module by name.  Calls OnUnregister() before
    /// removal.  Returns false if not found.
    bool Unregister(const std::string& name);

    /// Look up a tool module by name.  Returns nullptr if not found.
    IToolModule* Find(const std::string& name) const;

    /// Number of registered modules.
    size_t Count() const;

    /// Read-only access to all registered modules.
    const std::vector<std::unique_ptr<IToolModule>>& Modules() const;

    /// Broadcast an input event to all registered modules.
    /// Returns true if any module consumed the event.
    bool BroadcastInput(uint32_t keyCode, bool pressed);

    /// Update all registered modules.
    void UpdateAll(float deltaTime);

    /// Render all registered modules.
    void RenderAll();

    /// Remove all modules (calls OnUnregister on each).
    void Clear();

private:
    std::vector<std::unique_ptr<IToolModule>> m_modules;
};

} // namespace atlas::tools
