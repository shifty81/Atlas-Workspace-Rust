#pragma once
// NF::Editor — isolated scene context editor
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

enum class IsolationScope : uint8_t { World, SubScene, Prefab, Layer, Custom };
inline const char* isolationScopeName(IsolationScope v) {
    switch (v) {
        case IsolationScope::World:    return "World";
        case IsolationScope::SubScene: return "SubScene";
        case IsolationScope::Prefab:   return "Prefab";
        case IsolationScope::Layer:    return "Layer";
        case IsolationScope::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class IsolationPolicy : uint8_t { ReadOnly, CopyOnWrite, Exclusive, Shared };
inline const char* isolationPolicyName(IsolationPolicy v) {
    switch (v) {
        case IsolationPolicy::ReadOnly:    return "ReadOnly";
        case IsolationPolicy::CopyOnWrite: return "CopyOnWrite";
        case IsolationPolicy::Exclusive:   return "Exclusive";
        case IsolationPolicy::Shared:      return "Shared";
    }
    return "Unknown";
}

class IsolatedScene {
public:
    explicit IsolatedScene(uint32_t id, const std::string& name, IsolationScope scope, IsolationPolicy policy)
        : m_id(id), m_name(name), m_scope(scope), m_policy(policy) {}

    void setIsActive(bool v)           { m_isActive       = v; }
    void setMemoryBudgetMB(uint32_t v) { m_memoryBudgetMB = v; }
    void setIsEnabled(bool v)          { m_isEnabled      = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] IsolationScope     scope()          const { return m_scope;          }
    [[nodiscard]] IsolationPolicy    policy()         const { return m_policy;         }
    [[nodiscard]] bool               isActive()       const { return m_isActive;       }
    [[nodiscard]] uint32_t           memoryBudgetMB() const { return m_memoryBudgetMB; }
    [[nodiscard]] bool               isEnabled()      const { return m_isEnabled;      }

private:
    uint32_t         m_id;
    std::string      m_name;
    IsolationScope   m_scope;
    IsolationPolicy  m_policy;
    bool     m_isActive       = false;
    uint32_t m_memoryBudgetMB = 256u;
    bool     m_isEnabled      = true;
};

class SceneIsolationEditor {
public:
    void setIsShowInactive(bool v)      { m_isShowInactive = v; }
    void setIsGroupByScope(bool v)      { m_isGroupByScope = v; }
    void setDefaultBudgetMB(uint32_t v) { m_defaultBudgetMB = v; }

    bool addScene(const IsolatedScene& s) {
        for (auto& x : m_scenes) if (x.id() == s.id()) return false;
        m_scenes.push_back(s); return true;
    }
    bool removeScene(uint32_t id) {
        auto it = std::find_if(m_scenes.begin(), m_scenes.end(),
            [&](const IsolatedScene& s){ return s.id() == id; });
        if (it == m_scenes.end()) return false;
        m_scenes.erase(it); return true;
    }
    [[nodiscard]] IsolatedScene* findScene(uint32_t id) {
        for (auto& s : m_scenes) if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] bool     isShowInactive()  const { return m_isShowInactive;  }
    [[nodiscard]] bool     isGroupByScope()  const { return m_isGroupByScope;  }
    [[nodiscard]] uint32_t defaultBudgetMB() const { return m_defaultBudgetMB; }
    [[nodiscard]] size_t   sceneCount()      const { return m_scenes.size();   }

    [[nodiscard]] size_t countByScope(IsolationScope s) const {
        size_t n = 0; for (auto& sc : m_scenes) if (sc.scope() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByPolicy(IsolationPolicy p) const {
        size_t n = 0; for (auto& sc : m_scenes) if (sc.policy() == p) ++n; return n;
    }
    [[nodiscard]] size_t countActive() const {
        size_t n = 0; for (auto& sc : m_scenes) if (sc.isActive()) ++n; return n;
    }

private:
    std::vector<IsolatedScene> m_scenes;
    bool     m_isShowInactive  = true;
    bool     m_isGroupByScope  = false;
    uint32_t m_defaultBudgetMB = 512u;
};

} // namespace NF
