#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ecs/ECS.h"
#include "../../engine/sim/WorldState.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace atlas::editor {

struct InspectorEntry {
    ecs::EntityID entityID = 0;
    std::vector<std::string> componentNames;
};

struct StateBlockEntry {
    std::string name;
    sim::StateCategory category = sim::StateCategory::Simulated;
    uint32_t typeTag = 0;
    size_t estimatedSize = 0;
};

struct ComponentValueEntry {
    std::string name;
    std::string valueString;
    size_t sizeBytes = 0;
    uint32_t typeTag = 0;
};

struct EntityHierarchyNode {
    ecs::EntityID entityID = 0;
    ecs::EntityID parentID = 0;
    std::vector<ecs::EntityID> children;
    std::string label;
    int depth = 0;
};

struct ComponentMutation {
    ecs::EntityID entityID = 0;
    std::string componentName;
    uint64_t previousHash = 0;
    uint64_t currentHash = 0;
    uint64_t tick = 0;
};

class ECSInspectorPanel : public EditorPanel {
public:
    explicit ECSInspectorPanel(ecs::World& world) : m_world(world) {}

    const char* Name() const override { return "ECS Inspector"; }
    void Draw() override;

    void SelectEntity(ecs::EntityID id) {
        if (m_world.IsAlive(id)) m_selectedEntity = id;
    }
    ecs::EntityID SelectedEntity() const { return m_selectedEntity; }

    void DestroySelectedEntity();
    void ClearSelection() { m_selectedEntity = 0; }

    const std::string& SearchFilter() const { return m_searchFilter; }
    void SetSearchFilter(const std::string& filter);
    size_t EntityCountVisible() const;

    const std::vector<InspectorEntry>& LastSnapshot() const { return m_lastSnapshot; }

    std::string Summary() const;

    // State block inspector
    void SetWorldState(const sim::WorldState* ws) { m_worldState = ws; }
    const std::vector<StateBlockEntry>& StateBlocks() const { return m_stateBlocks; }
    std::string StateBlockSummary() const;

    // Live component value inspection
    std::vector<ComponentValueEntry> InspectComponentValues(ecs::EntityID id) const;

    // Entity hierarchy
    void SetParent(ecs::EntityID child, ecs::EntityID parent);
    ecs::EntityID GetParent(ecs::EntityID child) const;
    std::vector<ecs::EntityID> GetChildren(ecs::EntityID parent) const;
    std::vector<EntityHierarchyNode> BuildHierarchy() const;

    // Component mutation tracking
    void TrackMutations(uint64_t tick);
    const std::vector<ComponentMutation>& GetMutations() const;
    bool HasMutations() const;
    void ClearMutations();

private:
    ecs::World& m_world;
    ecs::EntityID m_selectedEntity = 0;
    std::string m_searchFilter;
    std::vector<InspectorEntry> m_lastSnapshot;
    const sim::WorldState* m_worldState = nullptr;
    std::vector<StateBlockEntry> m_stateBlocks;
    std::unordered_map<ecs::EntityID, ecs::EntityID> m_parentMap;
    std::vector<ComponentMutation> m_mutations;
    mutable std::unordered_map<ecs::EntityID, std::unordered_map<std::string, uint64_t>> m_previousHashes;
};

}
