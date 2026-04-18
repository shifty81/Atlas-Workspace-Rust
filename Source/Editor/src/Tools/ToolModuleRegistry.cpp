#include "ToolModuleRegistry.h"
#include "../core/Logger.h"
#include <algorithm>

namespace atlas::tools {

bool ToolModuleRegistry::Register(std::unique_ptr<IToolModule> module) {
    if (!module) return false;

    const std::string name = module->Name();
    if (Find(name) != nullptr) {
        Logger::Warn("ToolModuleRegistry: duplicate module '" + name + "'");
        return false;
    }

    module->OnRegister();
    m_modules.push_back(std::move(module));
    Logger::Info("ToolModuleRegistry: registered '" + name + "'");
    return true;
}

bool ToolModuleRegistry::Unregister(const std::string& name) {
    auto it = std::find_if(m_modules.begin(), m_modules.end(),
        [&name](const std::unique_ptr<IToolModule>& m) {
            return m->Name() == name;
        });

    if (it == m_modules.end()) return false;

    (*it)->OnUnregister();
    m_modules.erase(it);
    Logger::Info("ToolModuleRegistry: unregistered '" + name + "'");
    return true;
}

IToolModule* ToolModuleRegistry::Find(const std::string& name) const {
    for (const auto& m : m_modules) {
        if (m->Name() == name) return m.get();
    }
    return nullptr;
}

size_t ToolModuleRegistry::Count() const {
    return m_modules.size();
}

const std::vector<std::unique_ptr<IToolModule>>& ToolModuleRegistry::Modules() const {
    return m_modules;
}

bool ToolModuleRegistry::BroadcastInput(uint32_t keyCode, bool pressed) {
    for (auto& m : m_modules) {
        if (m->HandleInput(keyCode, pressed))
            return true;
    }
    return false;
}

void ToolModuleRegistry::UpdateAll(float deltaTime) {
    for (auto& m : m_modules)
        m->Update(deltaTime);
}

void ToolModuleRegistry::RenderAll() {
    for (auto& m : m_modules)
        m->Render();
}

void ToolModuleRegistry::Clear() {
    // Unregister in reverse order for clean teardown.
    for (auto it = m_modules.rbegin(); it != m_modules.rend(); ++it)
        (*it)->OnUnregister();
    m_modules.clear();
}

} // namespace atlas::tools
