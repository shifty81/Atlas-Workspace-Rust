#pragma once
// NF::Editor — Prefab asset + scene prefab editor
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

enum class PrefabCategory : uint8_t {
    Prop, Character, Vehicle, Structure, Effect
};
inline const char* prefabCategoryName(PrefabCategory c) {
    switch (c) {
        case PrefabCategory::Prop:      return "Prop";
        case PrefabCategory::Character: return "Character";
        case PrefabCategory::Vehicle:   return "Vehicle";
        case PrefabCategory::Structure: return "Structure";
        case PrefabCategory::Effect:    return "Effect";
    }
    return "Unknown";
}

enum class PrefabState : uint8_t {
    Unloaded, Loaded, Instanced, Modified, Broken
};
inline const char* prefabStateName(PrefabState s) {
    switch (s) {
        case PrefabState::Unloaded:  return "Unloaded";
        case PrefabState::Loaded:    return "Loaded";
        case PrefabState::Instanced: return "Instanced";
        case PrefabState::Modified:  return "Modified";
        case PrefabState::Broken:    return "Broken";
    }
    return "Unknown";
}

enum class PrefabLOD : uint8_t {
    Full, High, Medium, Low, Proxy
};
inline const char* prefabLODName(PrefabLOD l) {
    switch (l) {
        case PrefabLOD::Full:   return "Full";
        case PrefabLOD::High:   return "High";
        case PrefabLOD::Medium: return "Medium";
        case PrefabLOD::Low:    return "Low";
        case PrefabLOD::Proxy:  return "Proxy";
    }
    return "Unknown";
}

class PrefabAsset {
public:
    explicit PrefabAsset(const std::string& name,
                          uint32_t componentCount = 0,
                          uint32_t childCount = 0)
        : m_name(name), m_componentCount(componentCount), m_childCount(childCount) {}

    void setCategory(PrefabCategory c)  { m_category       = c; }
    void setState(PrefabState s)        { m_state           = s; }
    void setLOD(PrefabLOD l)            { m_lod             = l; }
    void setComponentCount(uint32_t v)  { m_componentCount  = v; }
    void setChildCount(uint32_t v)      { m_childCount      = v; }
    void setOverridable(bool v)         { m_overridable     = v; }
    void setNested(bool v)              { m_nested          = v; }
    void setDirty(bool v)               { m_dirty           = v; }

    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] PrefabCategory     category()       const { return m_category;       }
    [[nodiscard]] PrefabState        state()          const { return m_state;          }
    [[nodiscard]] PrefabLOD          lod()            const { return m_lod;            }
    [[nodiscard]] uint32_t           componentCount() const { return m_componentCount; }
    [[nodiscard]] uint32_t           childCount()     const { return m_childCount;     }
    [[nodiscard]] bool               isOverridable()  const { return m_overridable;    }
    [[nodiscard]] bool               isNested()       const { return m_nested;         }
    [[nodiscard]] bool               isDirty()        const { return m_dirty;          }

    [[nodiscard]] bool isInstanced() const { return m_state == PrefabState::Instanced; }
    [[nodiscard]] bool isModified()  const { return m_state == PrefabState::Modified;  }
    [[nodiscard]] bool isBroken()    const { return m_state == PrefabState::Broken;    }
    [[nodiscard]] bool isComplex()   const { return m_componentCount >= 10; }

private:
    std::string     m_name;
    PrefabCategory  m_category       = PrefabCategory::Prop;
    PrefabState     m_state          = PrefabState::Unloaded;
    PrefabLOD       m_lod            = PrefabLOD::Full;
    uint32_t        m_componentCount;
    uint32_t        m_childCount;
    bool            m_overridable    = false;
    bool            m_nested         = false;
    bool            m_dirty          = false;
};

class ScenePrefabEditor {
public:
    static constexpr size_t MAX_PREFABS = 256;

    [[nodiscard]] bool addPrefab(const PrefabAsset& prefab) {
        if (m_prefabs.size() >= MAX_PREFABS) return false;
        for (auto& p : m_prefabs) if (p.name() == prefab.name()) return false;
        m_prefabs.push_back(prefab);
        return true;
    }

    [[nodiscard]] bool removePrefab(const std::string& name) {
        for (auto it = m_prefabs.begin(); it != m_prefabs.end(); ++it) {
            if (it->name() == name) {
                if (m_activePrefab == name) m_activePrefab.clear();
                m_prefabs.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] PrefabAsset* findPrefab(const std::string& name) {
        for (auto& p : m_prefabs) if (p.name() == name) return &p;
        return nullptr;
    }

    [[nodiscard]] bool setActivePrefab(const std::string& name) {
        for (auto& p : m_prefabs)
            if (p.name() == name) { m_activePrefab = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activePrefab()    const { return m_activePrefab;   }
    [[nodiscard]] size_t prefabCount()                 const { return m_prefabs.size();  }
    [[nodiscard]] size_t dirtyCount()                  const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isDirty())       ++n; return n;
    }
    [[nodiscard]] size_t instancedCount()              const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isInstanced())   ++n; return n;
    }
    [[nodiscard]] size_t overridableCount()            const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isOverridable()) ++n; return n;
    }
    [[nodiscard]] size_t nestedCount()                 const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isNested())      ++n; return n;
    }
    [[nodiscard]] size_t complexCount()                const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.isComplex())     ++n; return n;
    }
    [[nodiscard]] size_t countByCategory(PrefabCategory cat) const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.category() == cat) ++n; return n;
    }
    [[nodiscard]] size_t countByState(PrefabState st) const {
        size_t n = 0; for (auto& p : m_prefabs) if (p.state() == st)  ++n; return n;
    }

private:
    std::vector<PrefabAsset> m_prefabs;
    std::string              m_activePrefab;
};

// ── S43 — Animation Blueprint Editor ───────────────────────────


} // namespace NF
