#pragma once
#include "ITool.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::tools {

/// Record of a placed prefab instance.
struct PlacedPrefab {
    uint32_t instanceId = 0;
    std::string prefabName;
    float posX = 0, posY = 0, posZ = 0;
    float rotY = 0;  ///< Rotation around Y axis (degrees)
    float scale = 1.0f;
};

/// In-client overlay tool for drag-and-drop prefab placement.
///
/// Browse a catalogue of reusable prefabs and place instances into
/// the scene with position, rotation, and scale controls.
class PrefabPlacementTool : public ITool {
public:
    std::string Name() const override { return "Prefab Placement"; }
    void Activate() override;
    void Deactivate() override;
    void Update(float deltaTime) override;
    bool IsActive() const override { return m_active; }

    // Catalogue
    void AddPrefab(const std::string& name);
    const std::vector<std::string>& PrefabCatalogue() const { return m_catalogue; }
    size_t CatalogueSize() const { return m_catalogue.size(); }

    // Selected prefab for placement
    void SelectPrefab(const std::string& name);
    std::string SelectedPrefab() const { return m_selected; }

    // Place an instance
    uint32_t Place(float x, float y, float z, float rotY = 0.0f, float scale = 1.0f);

    // History
    const std::vector<PlacedPrefab>& PlacedInstances() const { return m_placed; }
    size_t PlacedCount() const { return m_placed.size(); }

    // Undo last placement
    bool UndoLastPlacement();

private:
    bool m_active = false;
    std::vector<std::string> m_catalogue;
    std::string m_selected;
    std::vector<PlacedPrefab> m_placed;
    uint32_t m_nextInstanceId = 1;
};

} // namespace atlas::tools
