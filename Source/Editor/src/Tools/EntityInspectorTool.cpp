#include "EntityInspectorTool.h"

namespace atlas::tools {

void EntityInspectorTool::Activate() {
    m_active = true;
    ClearInspection();
}

void EntityInspectorTool::Deactivate() {
    m_active = false;
    ClearInspection();
}

void EntityInspectorTool::Update(float /*deltaTime*/) {
    // Inspector display is rebuilt when Inspect() is called.
}

void EntityInspectorTool::Inspect(uint32_t entityId) {
    m_entityId = entityId;
    m_components.clear();
}

void EntityInspectorTool::AddComponent(
    const std::string& name,
    const std::vector<std::pair<std::string, std::string>>& fields) {
    InspectedComponent comp;
    comp.name = name;
    comp.fields = fields;
    m_components.push_back(std::move(comp));
}

void EntityInspectorTool::ClearInspection() {
    m_entityId = 0;
    m_components.clear();
}

} // namespace atlas::tools
