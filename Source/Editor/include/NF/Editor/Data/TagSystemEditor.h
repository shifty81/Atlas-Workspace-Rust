#pragma once
// NF::Editor — asset/entity tag management editor
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

enum class TagCategory : uint8_t { Asset, Scene, Entity, Component, Custom };
inline const char* tagCategoryName(TagCategory v) {
    switch (v) {
        case TagCategory::Asset:     return "Asset";
        case TagCategory::Scene:     return "Scene";
        case TagCategory::Entity:    return "Entity";
        case TagCategory::Component: return "Component";
        case TagCategory::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class TagScope : uint8_t { Project, Scene, Local, Global };
inline const char* tagScopeName(TagScope v) {
    switch (v) {
        case TagScope::Project: return "Project";
        case TagScope::Scene:   return "Scene";
        case TagScope::Local:   return "Local";
        case TagScope::Global:  return "Global";
    }
    return "Unknown";
}

class TagDefinition {
public:
    explicit TagDefinition(uint32_t id, const std::string& name, TagCategory category, TagScope scope)
        : m_id(id), m_name(name), m_category(category), m_scope(scope) {}

    void setColor(uint32_t v)          { m_color          = v; }
    void setIsHierarchical(bool v)     { m_isHierarchical  = v; }
    void setIsEnabled(bool v)          { m_isEnabled       = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] TagCategory        category()       const { return m_category;       }
    [[nodiscard]] TagScope           scope()          const { return m_scope;          }
    [[nodiscard]] uint32_t           color()          const { return m_color;          }
    [[nodiscard]] bool               isHierarchical() const { return m_isHierarchical;  }
    [[nodiscard]] bool               isEnabled()      const { return m_isEnabled;      }

private:
    uint32_t    m_id;
    std::string m_name;
    TagCategory m_category;
    TagScope    m_scope;
    uint32_t    m_color          = 0u;
    bool        m_isHierarchical  = false;
    bool        m_isEnabled       = true;
};

class TagSystemEditor {
public:
    void setIsShowDisabled(bool v)    { m_isShowDisabled    = v; }
    void setIsGroupByCategory(bool v) { m_isGroupByCategory = v; }
    void setMaxTagsPerItem(uint32_t v){ m_maxTagsPerItem    = v; }

    bool addTag(const TagDefinition& t) {
        for (auto& x : m_tags) if (x.id() == t.id()) return false;
        m_tags.push_back(t); return true;
    }
    bool removeTag(uint32_t id) {
        auto it = std::find_if(m_tags.begin(), m_tags.end(),
            [&](const TagDefinition& t){ return t.id() == id; });
        if (it == m_tags.end()) return false;
        m_tags.erase(it); return true;
    }
    [[nodiscard]] TagDefinition* findTag(uint32_t id) {
        for (auto& t : m_tags) if (t.id() == id) return &t;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()    const { return m_isShowDisabled;    }
    [[nodiscard]] bool     isGroupByCategory() const { return m_isGroupByCategory; }
    [[nodiscard]] uint32_t maxTagsPerItem()    const { return m_maxTagsPerItem;    }
    [[nodiscard]] size_t   tagCount()          const { return m_tags.size();       }

    [[nodiscard]] size_t countByCategory(TagCategory c) const {
        size_t n = 0; for (auto& t : m_tags) if (t.category() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByScope(TagScope s) const {
        size_t n = 0; for (auto& t : m_tags) if (t.scope() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& t : m_tags) if (t.isEnabled()) ++n; return n;
    }

private:
    std::vector<TagDefinition> m_tags;
    bool     m_isShowDisabled    = false;
    bool     m_isGroupByCategory = true;
    uint32_t m_maxTagsPerItem    = 32u;
};

} // namespace NF
