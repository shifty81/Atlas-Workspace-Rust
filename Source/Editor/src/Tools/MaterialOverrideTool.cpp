#include "MaterialOverrideTool.h"

namespace atlas::tools {

void MaterialOverrideTool::Activate() {
    m_active = true;
    m_target = 0;
    m_params.clear();
}

void MaterialOverrideTool::Deactivate() {
    m_active = false;
}

void MaterialOverrideTool::Update(float /*deltaTime*/) {
    // Material overrides are applied immediately via SetParam.
}

void MaterialOverrideTool::SetTarget(uint32_t entityId) {
    m_target = entityId;
    m_params.clear();
}

void MaterialOverrideTool::SetParam(const std::string& name, float value) {
    for (auto& p : m_params) {
        if (p.name == name) { p.value = value; return; }
    }
    m_params.push_back({name, value});
}

float MaterialOverrideTool::GetParam(const std::string& name) const {
    for (const auto& p : m_params) {
        if (p.name == name) return p.value;
    }
    return 0.0f;
}

bool MaterialOverrideTool::HasParam(const std::string& name) const {
    for (const auto& p : m_params) {
        if (p.name == name) return true;
    }
    return false;
}

void MaterialOverrideTool::ResetOverrides() {
    m_params.clear();
}

} // namespace atlas::tools
