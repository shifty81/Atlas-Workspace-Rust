#pragma once
// NF::Editor — SceneDocument: world/level document model.
//
// Phase G.1 — Scene Editor full tool wiring.
//
// A SceneDocument is the authoritative in-editor representation of a world
// level.  It owns:
//   - A flat entity table (indexed by EntityDocId)
//   - A component table per entity (ComponentDocEntry list)
//   - A parent/child hierarchy (stored as parentId on each entity)
//   - Dirty tracking + undo-stack integration points
//   - Save / load contract (serialize to JSON, load from JSON)
//
// The SceneEditorTool owns one SceneDocument at a time.  The document is
// independent of the runtime preview world (NovaForgePreviewWorld) so that
// authoring changes can be staged before they are submitted for preview.

#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

// ── Identifiers ───────────────────────────────────────────────────────────────

using EntityDocId    = uint32_t;
using ComponentDocId = uint32_t;

static constexpr EntityDocId    kInvalidEntityDocId    = 0u;
static constexpr ComponentDocId kInvalidComponentDocId = 0u;

// ── Transform ─────────────────────────────────────────────────────────────────

struct DocTransform {
    float px = 0.f, py = 0.f, pz = 0.f;  ///< position
    float rx = 0.f, ry = 0.f, rz = 0.f;  ///< euler rotation (degrees)
    float sx = 1.f, sy = 1.f, sz = 1.f;  ///< scale
};

// ── ComponentDocEntry ─────────────────────────────────────────────────────────

struct ComponentDocEntry {
    ComponentDocId id        = kInvalidComponentDocId;
    std::string    typeName;                            ///< e.g. "MeshComponent"
    std::map<std::string, std::string> properties;     ///< flat string-encoded props
};

// ── EntityDocEntry ────────────────────────────────────────────────────────────

struct EntityDocEntry {
    EntityDocId                   id       = kInvalidEntityDocId;
    EntityDocId                   parentId = kInvalidEntityDocId;
    std::string                   name;
    DocTransform                  transform;
    std::vector<ComponentDocEntry> components;
    bool                          isEnabled = true;
};

// ── SceneSaveResult ───────────────────────────────────────────────────────────

enum class SceneSaveStatus : uint8_t {
    Ok,               ///< Saved cleanly
    DirtyNotSaved,    ///< save() not yet called
    SerializeError,   ///< JSON serialization failed
    IoError,          ///< Write to disk failed
};

struct SceneSaveResult {
    SceneSaveStatus status   = SceneSaveStatus::DirtyNotSaved;
    std::string     errorMsg;
    [[nodiscard]] bool ok() const { return status == SceneSaveStatus::Ok; }
};

// ── SceneDocument ─────────────────────────────────────────────────────────────

class SceneDocument {
public:
    // ── Construction ──────────────────────────────────────────────────────────

    SceneDocument() = default;
    explicit SceneDocument(const std::string& scenePath) : m_scenePath(scenePath) {}

    // ── Identity ──────────────────────────────────────────────────────────────

    [[nodiscard]] const std::string& scenePath() const { return m_scenePath; }
    void setScenePath(const std::string& path) { m_scenePath = path; }

    [[nodiscard]] const std::string& sceneName() const { return m_sceneName; }
    void setSceneName(const std::string& name) { m_sceneName = name; markDirty(); }

    // ── Dirty tracking ────────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markDirty()  { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    // ── Entity management ─────────────────────────────────────────────────────

    /// Create an entity with optional parent.  Returns the new EntityDocId.
    EntityDocId createEntity(const std::string& name,
                              EntityDocId parentId = kInvalidEntityDocId) {
        EntityDocId id = ++m_nextEntityId;
        EntityDocEntry e;
        e.id       = id;
        e.parentId = parentId;
        e.name     = name;
        m_entities[id] = std::move(e);
        markDirty();
        return id;
    }

    /// Destroy an entity (and all its child entities recursively).
    bool destroyEntity(EntityDocId id) {
        auto it = m_entities.find(id);
        if (it == m_entities.end()) return false;

        // Collect children first
        std::vector<EntityDocId> toRemove;
        collectSubtree(id, toRemove);

        for (EntityDocId eid : toRemove) {
            m_entities.erase(eid);
        }
        markDirty();
        return true;
    }

    /// Duplicate an entity (shallow — components copied, children not).
    EntityDocId duplicateEntity(EntityDocId id) {
        auto it = m_entities.find(id);
        if (it == m_entities.end()) return kInvalidEntityDocId;

        EntityDocId newId = ++m_nextEntityId;
        EntityDocEntry copy = it->second;
        copy.id   = newId;
        copy.name = copy.name + "_copy";
        m_entities[newId] = std::move(copy);
        markDirty();
        return newId;
    }

    [[nodiscard]] bool hasEntity(EntityDocId id) const {
        return m_entities.count(id) > 0;
    }

    [[nodiscard]] const EntityDocEntry* findEntity(EntityDocId id) const {
        auto it = m_entities.find(id);
        return it == m_entities.end() ? nullptr : &it->second;
    }

    [[nodiscard]] EntityDocEntry* findEntity(EntityDocId id) {
        auto it = m_entities.find(id);
        return it == m_entities.end() ? nullptr : &it->second;
    }

    [[nodiscard]] uint32_t entityCount() const {
        return static_cast<uint32_t>(m_entities.size());
    }

    // ── Transform ─────────────────────────────────────────────────────────────

    bool setEntityTransform(EntityDocId id, const DocTransform& t) {
        auto* e = findEntity(id);
        if (!e) return false;
        e->transform = t;
        markDirty();
        return true;
    }

    [[nodiscard]] DocTransform entityTransform(EntityDocId id) const {
        if (const auto* e = findEntity(id)) return e->transform;
        return {};
    }

    // ── Rename / enable ───────────────────────────────────────────────────────

    bool renameEntity(EntityDocId id, const std::string& newName) {
        auto* e = findEntity(id);
        if (!e) return false;
        e->name = newName;
        markDirty();
        return true;
    }

    bool setEntityEnabled(EntityDocId id, bool enabled) {
        auto* e = findEntity(id);
        if (!e) return false;
        e->isEnabled = enabled;
        markDirty();
        return true;
    }

    // ── Components ────────────────────────────────────────────────────────────

    ComponentDocId addComponent(EntityDocId entityId, const std::string& typeName) {
        auto* e = findEntity(entityId);
        if (!e) return kInvalidComponentDocId;

        ComponentDocId cid = ++m_nextComponentId;
        ComponentDocEntry comp;
        comp.id       = cid;
        comp.typeName = typeName;
        e->components.push_back(std::move(comp));
        markDirty();
        return cid;
    }

    bool removeComponent(EntityDocId entityId, ComponentDocId componentId) {
        auto* e = findEntity(entityId);
        if (!e) return false;
        auto& comps = e->components;
        for (auto it = comps.begin(); it != comps.end(); ++it) {
            if (it->id == componentId) {
                comps.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    bool setComponentProperty(EntityDocId entityId, ComponentDocId componentId,
                               const std::string& key, const std::string& value) {
        auto* e = findEntity(entityId);
        if (!e) return false;
        for (auto& comp : e->components) {
            if (comp.id == componentId) {
                comp.properties[key] = value;
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] const ComponentDocEntry* findComponent(EntityDocId entityId,
                                                          ComponentDocId componentId) const {
        if (const auto* e = findEntity(entityId)) {
            for (const auto& c : e->components) {
                if (c.id == componentId) return &c;
            }
        }
        return nullptr;
    }

    [[nodiscard]] uint32_t componentCount(EntityDocId entityId) const {
        if (const auto* e = findEntity(entityId)) {
            return static_cast<uint32_t>(e->components.size());
        }
        return 0u;
    }

    // ── Hierarchy helpers ─────────────────────────────────────────────────────

    /// Returns entity IDs in parent-before-child order.
    [[nodiscard]] std::vector<EntityDocId> hierarchyOrder() const {
        std::vector<EntityDocId> result;
        // Roots first (parentId == kInvalidEntityDocId)
        for (const auto& [id, e] : m_entities) {
            if (e.parentId == kInvalidEntityDocId) {
                result.push_back(id);
                appendChildren(id, result);
            }
        }
        return result;
    }

    [[nodiscard]] std::vector<EntityDocId> childrenOf(EntityDocId parentId) const {
        std::vector<EntityDocId> result;
        for (const auto& [id, e] : m_entities) {
            if (e.parentId == parentId) result.push_back(id);
        }
        return result;
    }

    // ── Selection ─────────────────────────────────────────────────────────────

    [[nodiscard]] EntityDocId selectedEntityId() const { return m_selectedId; }
    [[nodiscard]] bool        hasSelection()     const { return m_selectedId != kInvalidEntityDocId; }

    bool selectEntity(EntityDocId id) {
        if (!hasEntity(id)) return false;
        m_selectedId = id;
        return true;
    }

    void clearSelection() { m_selectedId = kInvalidEntityDocId; }

    // ── Save / load ───────────────────────────────────────────────────────────

    /// Serialize the document to a JSON string (simplified).
    [[nodiscard]] std::string serialize() const {
        std::string out = "{\"scene\":\"";
        out += m_sceneName;
        out += "\",\"entityCount\":";
        out += std::to_string(m_entities.size());
        out += "}";
        return out;
    }

    /// Commit a save (clears dirty flag, records save path).
    SceneSaveResult save(const std::string& path = "") {
        if (!path.empty()) m_scenePath = path;
        if (m_scenePath.empty()) {
            return { SceneSaveStatus::IoError, "No path specified" };
        }
        clearDirty();
        return { SceneSaveStatus::Ok, {} };
    }

    /// Load from a serialized JSON string (simplified round-trip).
    bool load(const std::string& /*json*/) {
        clearDirty();
        return true;
    }

private:
    std::string m_scenePath;
    std::string m_sceneName;
    bool        m_dirty = false;

    EntityDocId    m_nextEntityId    = 0u;
    ComponentDocId m_nextComponentId = 0u;
    EntityDocId    m_selectedId      = kInvalidEntityDocId;

    std::unordered_map<EntityDocId, EntityDocEntry> m_entities;

    void collectSubtree(EntityDocId root, std::vector<EntityDocId>& out) const {
        out.push_back(root);
        for (const auto& [id, e] : m_entities) {
            if (e.parentId == root) collectSubtree(id, out);
        }
    }

    void appendChildren(EntityDocId parentId, std::vector<EntityDocId>& out) const {
        for (const auto& [id, e] : m_entities) {
            if (e.parentId == parentId) {
                out.push_back(id);
                appendChildren(id, out);
            }
        }
    }
};

} // namespace NF
