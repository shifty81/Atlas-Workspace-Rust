#pragma once
// NF::Editor — Entity placement tool
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

struct PlacedEntity {
    EntityID    entityId     = INVALID_ENTITY;
    std::string templateName;
    Vec3        position;
    Vec3        rotation;
    Vec3        scale{1.f, 1.f, 1.f};
};

class EntityPlacementTool {
public:
    void setActiveTemplate(const std::string& tplName) { m_activeTemplate = tplName; }
    [[nodiscard]] const std::string& activeTemplate() const { return m_activeTemplate; }

    EntityID placeEntity(const Vec3& pos, const Vec3& rot = {}, const Vec3& scl = {1.f, 1.f, 1.f}) {
        PlacedEntity e;
        e.entityId = m_nextId++;
        e.templateName = m_activeTemplate;
        e.position = m_gridSnap ? snapToGrid(pos) : pos;
        e.rotation = rot;
        e.scale = scl;
        m_entities.push_back(e);
        return e.entityId;
    }

    void addEntity(const PlacedEntity& e) {
        m_entities.push_back(e);
        if (e.entityId >= m_nextId) m_nextId = e.entityId + 1;
    }

    bool removeEntity(EntityID id) {
        auto it = std::find_if(m_entities.begin(), m_entities.end(),
                               [id](const PlacedEntity& pe) { return pe.entityId == id; });
        if (it == m_entities.end()) return false;
        m_entities.erase(it);
        return true;
    }

    [[nodiscard]] const std::vector<PlacedEntity>& placedEntities() const { return m_entities; }
    [[nodiscard]] size_t placedCount() const { return m_entities.size(); }

    void setGridSnap(bool enabled) { m_gridSnap = enabled; }
    void setGridSize(float size) { m_gridSize = size; }
    [[nodiscard]] bool isGridSnapEnabled() const { return m_gridSnap; }
    [[nodiscard]] float gridSize() const { return m_gridSize; }

    [[nodiscard]] Vec3 snapToGrid(const Vec3& v) const {
        if (m_gridSize <= 0.f) return v;
        return {
            std::round(v.x / m_gridSize) * m_gridSize,
            std::round(v.y / m_gridSize) * m_gridSize,
            std::round(v.z / m_gridSize) * m_gridSize
        };
    }

    void clear() { m_entities.clear(); }

private:
    std::vector<PlacedEntity> m_entities;
    std::string m_activeTemplate;
    EntityID m_nextId = 1;
    bool  m_gridSnap = false;
    float m_gridSize = 1.0f;
};

// ── Voxel Paint ─────────────────────────────────────────────────


} // namespace NF
