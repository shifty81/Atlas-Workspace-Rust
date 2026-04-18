#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <algorithm>

namespace atlas::editor {

struct PrefabComponent {
    std::string type;
    std::vector<std::pair<std::string, std::string>> properties;

    std::string GetProperty(const std::string& key, const std::string& def = "") const {
        for (const auto& p : properties) {
            if (p.first == key) return p.second;
        }
        return def;
    }
};

struct PrefabEntity {
    uint32_t id = 0;
    std::string name;
    uint32_t parent = 0;  // 0 = root
    std::vector<PrefabComponent> components;
};

class PrefabEditorPanel : public EditorPanel {
public:
    const char* Name() const override { return "Prefab Editor"; }
    void Draw() override;

    uint32_t AddEntity(const std::string& name) {
        PrefabEntity entity;
        entity.id = m_nextID++;
        entity.name = name;
        m_entities[entity.id] = entity;
        m_dirty = true;
        return entity.id;
    }

    void RemoveEntity(uint32_t id) {
        // Recursively collect all descendants
        std::vector<uint32_t> toRemove;
        toRemove.push_back(id);
        for (size_t i = 0; i < toRemove.size(); ++i) {
            uint32_t current = toRemove[i];
            for (const auto& [eid, e] : m_entities) {
                if (e.parent == current) toRemove.push_back(eid);
            }
        }
        for (uint32_t rid : toRemove) {
            if (m_selectedEntity == rid) m_selectedEntity = 0;
            m_entities.erase(rid);
        }
        m_dirty = true;
    }

    const PrefabEntity* GetEntity(uint32_t id) const {
        auto it = m_entities.find(id);
        return it != m_entities.end() ? &it->second : nullptr;
    }

    void SetParent(uint32_t childID, uint32_t parentID) {
        auto it = m_entities.find(childID);
        if (it != m_entities.end()) {
            it->second.parent = parentID;
            m_dirty = true;
        }
    }

    void AddComponent(uint32_t entityID, const PrefabComponent& comp) {
        auto it = m_entities.find(entityID);
        if (it != m_entities.end()) {
            it->second.components.push_back(comp);
            m_dirty = true;
        }
    }

    void RemoveComponent(uint32_t entityID, const std::string& type) {
        auto it = m_entities.find(entityID);
        if (it == m_entities.end()) return;
        auto& comps = it->second.components;
        comps.erase(
            std::remove_if(comps.begin(), comps.end(),
                [&](const PrefabComponent& c) { return c.type == type; }),
            comps.end());
        m_dirty = true;
    }

    size_t EntityCount() const { return m_entities.size(); }

    std::vector<uint32_t> RootEntities() const {
        std::vector<uint32_t> roots;
        for (const auto& [id, e] : m_entities) {
            if (e.parent == 0) roots.push_back(id);
        }
        std::sort(roots.begin(), roots.end());
        return roots;
    }

    std::vector<uint32_t> Children(uint32_t parentID) const {
        std::vector<uint32_t> kids;
        for (const auto& [id, e] : m_entities) {
            if (e.parent == parentID) kids.push_back(id);
        }
        std::sort(kids.begin(), kids.end());
        return kids;
    }

    void SelectEntity(uint32_t id) { m_selectedEntity = id; }
    uint32_t SelectedEntity() const { return m_selectedEntity; }

    bool IsDirty() const { return m_dirty; }
    void MarkClean() { m_dirty = false; }

    void Clear() {
        m_entities.clear();
        m_nextID = 1;
        m_selectedEntity = 0;
        m_dirty = false;
    }

    std::string Summary() const {
        if (m_entities.empty()) return "Empty prefab";
        std::ostringstream oss;
        oss << "Entities: " << m_entities.size();
        size_t totalComps = 0;
        for (const auto& [id, e] : m_entities) {
            totalComps += e.components.size();
        }
        oss << ", Components: " << totalComps;
        if (m_dirty) oss << " (modified)";
        return oss.str();
    }

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::unordered_map<uint32_t, PrefabEntity> m_entities;
    uint32_t m_nextID = 1;
    uint32_t m_selectedEntity = 0;
    bool m_dirty = false;
    atlas::ui::UIDrawList m_drawList;

    int32_t DrawEntityRow(uint32_t entityId, int depth, int32_t y);
};

} // namespace atlas::editor
