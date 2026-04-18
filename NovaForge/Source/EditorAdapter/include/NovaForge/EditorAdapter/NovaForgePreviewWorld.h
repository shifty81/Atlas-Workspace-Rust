#pragma once
// NovaForge::NovaForgePreviewWorld — editable preview scene container.
//
// Manages a lightweight set of preview entities with names, parent IDs,
// transforms (position/rotation/scale), mesh tags, and material tags.
// Used by NovaForgePreviewRuntime to back the SceneEditorTool viewport.
//
// Phase D.1 — NovaForge Preview Runtime Bridge

#include <cstdint>
#include <string>
#include <vector>

namespace NovaForge {

// ── EntityId ──────────────────────────────────────────────────────────────────

using EntityId = uint32_t;
static constexpr EntityId kInvalidEntityId = 0;

// ── PreviewVec3 ───────────────────────────────────────────────────────────────

struct PreviewVec3 {
    float x = 0.f, y = 0.f, z = 0.f;

    bool operator==(const PreviewVec3& o) const {
        return x == o.x && y == o.y && z == o.z;
    }
    bool operator!=(const PreviewVec3& o) const { return !(*this == o); }
};

// ── PreviewTransform ──────────────────────────────────────────────────────────

struct PreviewTransform {
    PreviewVec3 position;
    PreviewVec3 rotation; ///< Euler angles in degrees
    PreviewVec3 scale{1.f, 1.f, 1.f};
};

// ── PreviewEntity ─────────────────────────────────────────────────────────────

struct PreviewEntity {
    EntityId         id          = kInvalidEntityId;
    std::string      name;
    EntityId         parentId    = kInvalidEntityId;
    PreviewTransform transform;
    std::string      meshTag;
    std::string      materialTag;
    bool             visible     = true;
};

// ── NovaForgePreviewWorld ─────────────────────────────────────────────────────

class NovaForgePreviewWorld {
public:
    static constexpr uint32_t kMaxEntities = 512;

    NovaForgePreviewWorld() = default;

    // ── Entity lifecycle ──────────────────────────────────────────────────

    /// Create a new entity. Returns kInvalidEntityId if at capacity.
    EntityId createEntity(const std::string& name    = "Entity",
                          EntityId            parentId = kInvalidEntityId) {
        if (m_entities.size() >= kMaxEntities) return kInvalidEntityId;
        PreviewEntity e;
        e.id       = m_nextId++;
        e.name     = name;
        e.parentId = parentId;
        m_entities.push_back(e);
        m_dirty = true;
        return e.id;
    }

    /// Destroy entity and deselect if selected. Returns false if not found.
    bool destroyEntity(EntityId id) {
        for (auto it = m_entities.begin(); it != m_entities.end(); ++it) {
            if (it->id == id) {
                if (m_selectedId == id) m_selectedId = kInvalidEntityId;
                m_entities.erase(it);
                m_dirty = true;
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool hasEntity(EntityId id) const { return find(id) != nullptr; }

    /// Remove all entities and clear selection.
    void clearEntities() {
        m_entities.clear();
        m_selectedId = kInvalidEntityId;
        m_dirty = true;
    }

    // ── Entity access ─────────────────────────────────────────────────────

    [[nodiscard]] const PreviewEntity* find(EntityId id) const {
        for (const auto& e : m_entities)
            if (e.id == id) return &e;
        return nullptr;
    }

    [[nodiscard]] PreviewEntity* find(EntityId id) {
        for (auto& e : m_entities)
            if (e.id == id) return &e;
        return nullptr;
    }

    [[nodiscard]] const std::vector<PreviewEntity>& entities() const { return m_entities; }
    [[nodiscard]] uint32_t entityCount() const { return static_cast<uint32_t>(m_entities.size()); }

    // ── Transform ─────────────────────────────────────────────────────────

    bool setTransform(EntityId id, const PreviewTransform& t) {
        if (auto* e = find(id)) { e->transform = t; m_dirty = true; return true; }
        return false;
    }

    bool setPosition(EntityId id, const PreviewVec3& pos) {
        if (auto* e = find(id)) { e->transform.position = pos; m_dirty = true; return true; }
        return false;
    }

    bool setRotation(EntityId id, const PreviewVec3& rot) {
        if (auto* e = find(id)) { e->transform.rotation = rot; m_dirty = true; return true; }
        return false;
    }

    bool setScale(EntityId id, const PreviewVec3& scl) {
        if (auto* e = find(id)) { e->transform.scale = scl; m_dirty = true; return true; }
        return false;
    }

    // ── Mesh / Material / Visibility ──────────────────────────────────────

    bool setMesh(EntityId id, const std::string& meshTag) {
        if (auto* e = find(id)) { e->meshTag = meshTag; m_dirty = true; return true; }
        return false;
    }

    bool setMaterial(EntityId id, const std::string& materialTag) {
        if (auto* e = find(id)) { e->materialTag = materialTag; m_dirty = true; return true; }
        return false;
    }

    bool setVisible(EntityId id, bool visible) {
        if (auto* e = find(id)) { e->visible = visible; m_dirty = true; return true; }
        return false;
    }

    // ── Selection ─────────────────────────────────────────────────────────

    /// Select entity. Returns false if entity does not exist.
    bool selectEntity(EntityId id) {
        if (id == kInvalidEntityId || hasEntity(id)) {
            m_selectedId = id;
            return true;
        }
        return false;
    }

    void deselectAll() { m_selectedId = kInvalidEntityId; }

    [[nodiscard]] EntityId selectedEntityId() const { return m_selectedId; }
    [[nodiscard]] bool     hasSelection()     const { return m_selectedId != kInvalidEntityId; }

    [[nodiscard]] const PreviewEntity* selectedEntity() const {
        return m_selectedId != kInvalidEntityId ? find(m_selectedId) : nullptr;
    }

    // ── Dirty tracking ────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty()                  { m_dirty = false; }

private:
    std::vector<PreviewEntity> m_entities;
    EntityId                   m_nextId     = 1;
    EntityId                   m_selectedId = kInvalidEntityId;
    bool                       m_dirty      = false;
};

} // namespace NovaForge
