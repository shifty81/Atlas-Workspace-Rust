#include "EditorToolRegistry.h"

namespace atlas::tools {

void EditorToolRegistry::Register(std::unique_ptr<ITool> tool) {
    if (tool) m_tools.push_back(std::move(tool));
}

size_t EditorToolRegistry::Count() const {
    return m_tools.size();
}

ITool* EditorToolRegistry::Get(size_t index) const {
    if (index >= m_tools.size()) return nullptr;
    return m_tools[index].get();
}

ITool* EditorToolRegistry::FindByName(const std::string& name) const {
    for (const auto& t : m_tools) {
        if (t->Name() == name) return t.get();
    }
    return nullptr;
}

bool EditorToolRegistry::Activate(const std::string& name) {
    ITool* tool = FindByName(name);
    if (!tool) return false;

    DeactivateCurrent();
    tool->Activate();
    m_active = tool;
    return true;
}

void EditorToolRegistry::DeactivateCurrent() {
    if (m_active) {
        m_active->Deactivate();
        m_active = nullptr;
    }
}

ITool* EditorToolRegistry::ActiveTool() const {
    return m_active;
}

void EditorToolRegistry::UpdateActive(float deltaTime) {
    if (m_active && m_active->IsActive()) {
        m_active->Update(deltaTime);
    }
}

std::vector<std::string> EditorToolRegistry::ToolNames() const {
    std::vector<std::string> names;
    names.reserve(m_tools.size());
    for (const auto& t : m_tools) {
        names.push_back(t->Name());
    }
    return names;
}

void EditorToolRegistry::Clear() {
    DeactivateCurrent();
    m_tools.clear();
}

} // namespace atlas::tools
