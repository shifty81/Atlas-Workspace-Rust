#pragma once
#include "ITool.h"
#include <cstdint>
#include <vector>

namespace atlas::tools {

/// In-client overlay tool for mass entity transformations.
///
/// Select multiple entities and apply batch translate, rotate, scale,
/// or property set operations.
class BatchTransformTool : public ITool {
public:
    std::string Name() const override { return "Batch Transform"; }
    void Activate() override;
    void Deactivate() override;
    void Update(float deltaTime) override;
    bool IsActive() const override { return m_active; }

    // Selection
    void Select(uint32_t entityId);
    void Deselect(uint32_t entityId);
    void ClearSelection();
    const std::vector<uint32_t>& Selection() const { return m_selection; }
    size_t SelectionCount() const { return m_selection.size(); }
    bool IsSelected(uint32_t entityId) const;

    // Batch operations (applied to all selected entities)
    void ApplyTranslate(float dx, float dy, float dz);
    void ApplyScale(float sx, float sy, float sz);
    void ApplyRotate(float degreesY);

    // Counters
    uint32_t OperationCount() const { return m_opCount; }
    float LastTranslateX() const { return m_lastDx; }
    float LastTranslateY() const { return m_lastDy; }
    float LastTranslateZ() const { return m_lastDz; }

private:
    bool m_active = false;
    std::vector<uint32_t> m_selection;
    uint32_t m_opCount = 0;
    float m_lastDx = 0, m_lastDy = 0, m_lastDz = 0;
};

} // namespace atlas::tools
