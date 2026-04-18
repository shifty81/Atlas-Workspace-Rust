#include "BatchTransformTool.h"
#include <algorithm>

namespace atlas::tools {

void BatchTransformTool::Activate() {
    m_active = true;
    m_opCount = 0;
}

void BatchTransformTool::Deactivate() {
    m_active = false;
    ClearSelection();
}

void BatchTransformTool::Update(float /*deltaTime*/) {
    // Batch tool is driven by explicit operations, not per-frame logic.
}

void BatchTransformTool::Select(uint32_t entityId) {
    if (!IsSelected(entityId)) {
        m_selection.push_back(entityId);
    }
}

void BatchTransformTool::Deselect(uint32_t entityId) {
    m_selection.erase(
        std::remove(m_selection.begin(), m_selection.end(), entityId),
        m_selection.end());
}

void BatchTransformTool::ClearSelection() {
    m_selection.clear();
}

bool BatchTransformTool::IsSelected(uint32_t entityId) const {
    return std::find(m_selection.begin(), m_selection.end(), entityId) != m_selection.end();
}

void BatchTransformTool::ApplyTranslate(float dx, float dy, float dz) {
    if (m_selection.empty()) return;
    m_lastDx = dx; m_lastDy = dy; m_lastDz = dz;
    m_opCount++;
}

void BatchTransformTool::ApplyScale(float /*sx*/, float /*sy*/, float /*sz*/) {
    if (m_selection.empty()) return;
    m_opCount++;
}

void BatchTransformTool::ApplyRotate(float /*degreesY*/) {
    if (m_selection.empty()) return;
    m_opCount++;
}

} // namespace atlas::tools
