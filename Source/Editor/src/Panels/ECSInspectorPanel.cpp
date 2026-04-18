#include "ECSInspectorPanel.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace atlas::editor {

void ECSInspectorPanel::DestroySelectedEntity() {
    if (m_selectedEntity != 0 && m_world.IsAlive(m_selectedEntity)) {
        m_world.DestroyEntity(m_selectedEntity);
    }
    m_selectedEntity = 0;
}

void ECSInspectorPanel::SetSearchFilter(const std::string& filter) {
    m_searchFilter = filter;
}

size_t ECSInspectorPanel::EntityCountVisible() const {
    auto entities = m_world.GetEntities();
    if (m_searchFilter.empty()) return entities.size();
    size_t count = 0;
    for (auto eid : entities) {
        if (std::to_string(eid).find(m_searchFilter) != std::string::npos) ++count;
    }
    return count;
}

void ECSInspectorPanel::Draw() {
    // Clear dead selection
    if (m_selectedEntity != 0 && !m_world.IsAlive(m_selectedEntity))
        m_selectedEntity = 0;

    m_lastSnapshot.clear();

    auto entities = m_world.GetEntities();
    for (auto eid : entities) {
        InspectorEntry entry;
        entry.entityID = eid;

        auto types = m_world.GetComponentTypes(eid);
        for (const auto& ti : types) {
            // Demangle type name: use raw name as a best-effort label
            entry.componentNames.push_back(ti.name());
        }

        m_lastSnapshot.push_back(std::move(entry));
    }

    // Refresh state block info from WorldState
    m_stateBlocks.clear();
    if (m_worldState) {
        for (const auto& block : m_worldState->RegisteredBlocks()) {
            StateBlockEntry sbe;
            sbe.name = block.name;
            sbe.category = block.category;
            sbe.typeTag = block.typeTag;
            sbe.estimatedSize = block.estimatedSize;
            m_stateBlocks.push_back(std::move(sbe));
        }
    }
}

std::string ECSInspectorPanel::Summary() const {
    std::ostringstream oss;
    oss << "Entities: " << m_lastSnapshot.size();
    size_t totalComponents = 0;
    for (const auto& entry : m_lastSnapshot) {
        totalComponents += entry.componentNames.size();
    }
    oss << ", Components: " << totalComponents;
    return oss.str();
}

std::string ECSInspectorPanel::StateBlockSummary() const {
    std::ostringstream oss;
    size_t simCount = 0, derivedCount = 0, presentationCount = 0, debugCount = 0;
    for (const auto& block : m_stateBlocks) {
        switch (block.category) {
            case sim::StateCategory::Simulated:    ++simCount; break;
            case sim::StateCategory::Derived:      ++derivedCount; break;
            case sim::StateCategory::Presentation: ++presentationCount; break;
            case sim::StateCategory::Debug:        ++debugCount; break;
        }
    }
    oss << "Blocks: " << m_stateBlocks.size()
        << " (Sim=" << simCount
        << " Derived=" << derivedCount
        << " Pres=" << presentationCount
        << " Debug=" << debugCount << ")";
    return oss.str();
}

std::vector<ComponentValueEntry> ECSInspectorPanel::InspectComponentValues(ecs::EntityID id) const {
    std::vector<ComponentValueEntry> values;
    auto types = m_world.GetComponentTypes(id);
    for (const auto& ti : types) {
        ComponentValueEntry entry;
        entry.name = ti.name();

        if (m_world.HasSerializer(ti)) {
            auto data = m_world.SerializeComponent(id, ti);
            entry.sizeBytes = data.size();
            entry.typeTag = m_world.GetTypeTag(ti);

            std::ostringstream oss;
            oss << std::hex;
            for (size_t i = 0; i < std::min(data.size(), size_t(16)); ++i) {
                if (i > 0) oss << " ";
                oss << std::setw(2) << std::setfill('0') << (int)data[i];
            }
            if (data.size() > 16) oss << "...";
            entry.valueString = oss.str();
        } else {
            entry.valueString = "<no serializer>";
        }
        values.push_back(std::move(entry));
    }
    return values;
}

void ECSInspectorPanel::SetParent(ecs::EntityID child, ecs::EntityID parent) {
    m_parentMap[child] = parent;
}

ecs::EntityID ECSInspectorPanel::GetParent(ecs::EntityID child) const {
    auto it = m_parentMap.find(child);
    return it != m_parentMap.end() ? it->second : 0;
}

std::vector<ecs::EntityID> ECSInspectorPanel::GetChildren(ecs::EntityID parent) const {
    std::vector<ecs::EntityID> children;
    for (const auto& [child, par] : m_parentMap) {
        if (par == parent) children.push_back(child);
    }
    return children;
}

std::vector<EntityHierarchyNode> ECSInspectorPanel::BuildHierarchy() const {
    std::vector<EntityHierarchyNode> nodes;
    auto entities = m_world.GetEntities();

    // Cache computed depths to avoid redundant traversals
    std::unordered_map<ecs::EntityID, int> depthCache;
    static constexpr int MAX_HIERARCHY_DEPTH = 100;

    // Recursive-safe depth lookup with caching
    auto computeDepth = [&](ecs::EntityID eid) -> int {
        auto cached = depthCache.find(eid);
        if (cached != depthCache.end()) return cached->second;
        int depth = 0;
        ecs::EntityID cur = eid;
        while (GetParent(cur) != 0) {
            cur = GetParent(cur);
            auto cc = depthCache.find(cur);
            if (cc != depthCache.end()) { depth += 1 + cc->second; break; }
            ++depth;
            if (depth > MAX_HIERARCHY_DEPTH) break;
        }
        depthCache[eid] = depth;
        return depth;
    };

    for (auto eid : entities) {
        EntityHierarchyNode node;
        node.entityID = eid;
        node.parentID = GetParent(eid);
        node.children = GetChildren(eid);
        node.label = "Entity_" + std::to_string(eid);
        node.depth = computeDepth(eid);
        nodes.push_back(std::move(node));
    }
    return nodes;
}

void ECSInspectorPanel::TrackMutations(uint64_t tick) {
    m_mutations.clear();
    auto entities = m_world.GetEntities();
    for (auto eid : entities) {
        auto types = m_world.GetComponentTypes(eid);
        for (const auto& ti : types) {
            std::string name = ti.name();
            uint64_t currentHash = 0;

            if (m_world.HasSerializer(ti)) {
                auto data = m_world.SerializeComponent(eid, ti);
                // FNV-1a hash of component data
                static constexpr uint64_t FNV1A_OFFSET_BASIS_64 = 14695981039346656037ULL;
                static constexpr uint64_t FNV1A_PRIME_64 = 1099511628211ULL;
                currentHash = FNV1A_OFFSET_BASIS_64;
                for (uint8_t byte : data) {
                    currentHash ^= byte;
                    currentHash *= FNV1A_PRIME_64;
                }
            }

            auto prevIt = m_previousHashes.find(eid);
            if (prevIt != m_previousHashes.end()) {
                auto compIt = prevIt->second.find(name);
                if (compIt != prevIt->second.end()) {
                    if (compIt->second != currentHash) {
                        ComponentMutation mut;
                        mut.entityID = eid;
                        mut.componentName = name;
                        mut.previousHash = compIt->second;
                        mut.currentHash = currentHash;
                        mut.tick = tick;
                        m_mutations.push_back(mut);
                    }
                }
            }

            m_previousHashes[eid][name] = currentHash;
        }
    }
}

const std::vector<ComponentMutation>& ECSInspectorPanel::GetMutations() const {
    return m_mutations;
}

bool ECSInspectorPanel::HasMutations() const {
    return !m_mutations.empty();
}

void ECSInspectorPanel::ClearMutations() {
    m_mutations.clear();
    m_previousHashes.clear();
}

}
