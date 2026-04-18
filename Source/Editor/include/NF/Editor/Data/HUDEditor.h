#pragma once
// NF::Editor — HUD editor
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

enum class HUDElementCategory : uint8_t {
    Health, Stamina, Ammo, Minimap, Crosshair, Objective, Score, Timer, Notification, Debug
};

inline const char* hudElementCategoryName(HUDElementCategory c) {
    switch (c) {
        case HUDElementCategory::Health:       return "Health";
        case HUDElementCategory::Stamina:      return "Stamina";
        case HUDElementCategory::Ammo:         return "Ammo";
        case HUDElementCategory::Minimap:      return "Minimap";
        case HUDElementCategory::Crosshair:    return "Crosshair";
        case HUDElementCategory::Objective:    return "Objective";
        case HUDElementCategory::Score:        return "Score";
        case HUDElementCategory::Timer:        return "Timer";
        case HUDElementCategory::Notification: return "Notification";
        case HUDElementCategory::Debug:        return "Debug";
    }
    return "Unknown";
}

enum class HUDVisibilityMode : uint8_t {
    AlwaysVisible, OnDamage, OnLowHealth, WhenAiming, WhenInteracting, Custom
};

inline const char* hudVisibilityModeName(HUDVisibilityMode m) {
    switch (m) {
        case HUDVisibilityMode::AlwaysVisible:    return "AlwaysVisible";
        case HUDVisibilityMode::OnDamage:         return "OnDamage";
        case HUDVisibilityMode::OnLowHealth:      return "OnLowHealth";
        case HUDVisibilityMode::WhenAiming:       return "WhenAiming";
        case HUDVisibilityMode::WhenInteracting:  return "WhenInteracting";
        case HUDVisibilityMode::Custom:           return "Custom";
    }
    return "Unknown";
}

enum class HUDLayoutPreset : uint8_t {
    Standard, Minimal, Immersive, Accessibility, Custom
};

inline const char* hudLayoutPresetName(HUDLayoutPreset p) {
    switch (p) {
        case HUDLayoutPreset::Standard:      return "Standard";
        case HUDLayoutPreset::Minimal:       return "Minimal";
        case HUDLayoutPreset::Immersive:     return "Immersive";
        case HUDLayoutPreset::Accessibility: return "Accessibility";
        case HUDLayoutPreset::Custom:        return "Custom";
    }
    return "Unknown";
}

class HUDElement {
public:
    explicit HUDElement(uint32_t id, const std::string& name, HUDElementCategory cat)
        : m_id(id), m_name(name), m_category(cat) {}

    void setVisibilityMode(HUDVisibilityMode m) { m_visibilityMode = m; }
    void setEnabled(bool v)                     { m_enabled        = v; }
    void setOpacity(float v)                    { m_opacity        = v; }
    void setScale(float v)                      { m_scale          = v; }

    [[nodiscard]] uint32_t             id()             const { return m_id;            }
    [[nodiscard]] const std::string&   name()           const { return m_name;          }
    [[nodiscard]] HUDElementCategory   category()       const { return m_category;      }
    [[nodiscard]] HUDVisibilityMode    visibilityMode() const { return m_visibilityMode;}
    [[nodiscard]] bool                 isEnabled()      const { return m_enabled;       }
    [[nodiscard]] float                opacity()        const { return m_opacity;       }
    [[nodiscard]] float                scale()          const { return m_scale;         }

private:
    uint32_t            m_id;
    std::string         m_name;
    HUDElementCategory  m_category;
    HUDVisibilityMode   m_visibilityMode = HUDVisibilityMode::AlwaysVisible;
    bool                m_enabled        = true;
    float               m_opacity        = 1.0f;
    float               m_scale          = 1.0f;
};

class HUDEditor {
public:
    void setLayoutPreset(HUDLayoutPreset p) { m_layoutPreset = p; }
    void setPreviewEnabled(bool v)          { m_preview      = v; }
    void setSafeZoneEnabled(bool v)         { m_safeZone     = v; }

    bool addElement(const HUDElement& el) {
        for (auto& e : m_elements) if (e.id() == el.id()) return false;
        m_elements.push_back(el); return true;
    }
    bool removeElement(uint32_t id) {
        auto it = std::find_if(m_elements.begin(), m_elements.end(),
            [&](const HUDElement& e){ return e.id() == id; });
        if (it == m_elements.end()) return false;
        m_elements.erase(it); return true;
    }
    [[nodiscard]] HUDElement* findElement(uint32_t id) {
        for (auto& e : m_elements) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] HUDLayoutPreset layoutPreset()     const { return m_layoutPreset; }
    [[nodiscard]] bool            isPreviewEnabled() const { return m_preview;      }
    [[nodiscard]] bool            isSafeZoneEnabled()const { return m_safeZone;     }
    [[nodiscard]] size_t          elementCount()     const { return m_elements.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& e : m_elements) if (e.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(HUDElementCategory cat) const {
        size_t c = 0; for (auto& e : m_elements) if (e.category() == cat) ++c; return c;
    }

private:
    std::vector<HUDElement> m_elements;
    HUDLayoutPreset         m_layoutPreset = HUDLayoutPreset::Standard;
    bool                    m_preview      = false;
    bool                    m_safeZone     = true;
};

} // namespace NF
