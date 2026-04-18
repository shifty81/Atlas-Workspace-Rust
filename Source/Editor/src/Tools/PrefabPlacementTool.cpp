#include "PrefabPlacementTool.h"
#include <algorithm>

namespace atlas::tools {

void PrefabPlacementTool::Activate() {
    m_active = true;
}

void PrefabPlacementTool::Deactivate() {
    m_active = false;
}

void PrefabPlacementTool::Update(float /*deltaTime*/) {
    // Placement is event-driven.
}

void PrefabPlacementTool::AddPrefab(const std::string& name) {
    if (std::find(m_catalogue.begin(), m_catalogue.end(), name) == m_catalogue.end()) {
        m_catalogue.push_back(name);
    }
}

void PrefabPlacementTool::SelectPrefab(const std::string& name) {
    m_selected = name;
}

uint32_t PrefabPlacementTool::Place(float x, float y, float z, float rotY, float scale) {
    PlacedPrefab pp;
    pp.instanceId = m_nextInstanceId++;
    pp.prefabName = m_selected;
    pp.posX = x; pp.posY = y; pp.posZ = z;
    pp.rotY = rotY;
    pp.scale = scale;
    m_placed.push_back(pp);
    return pp.instanceId;
}

bool PrefabPlacementTool::UndoLastPlacement() {
    if (m_placed.empty()) return false;
    m_placed.pop_back();
    return true;
}

} // namespace atlas::tools
