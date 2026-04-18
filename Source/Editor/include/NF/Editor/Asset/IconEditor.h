#pragma once
// NF::Editor — Icon asset + editor
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

enum class IconSize : uint8_t {
    XSmall, Small, Medium, Large, XLarge
};

inline const char* iconSizeName(IconSize s) {
    switch (s) {
        case IconSize::XSmall: return "XSmall";
        case IconSize::Small:  return "Small";
        case IconSize::Medium: return "Medium";
        case IconSize::Large:  return "Large";
        case IconSize::XLarge: return "XLarge";
    }
    return "Unknown";
}

enum class IconTheme : uint8_t {
    Light, Dark, HighContrast, Monochrome
};

inline const char* iconThemeName(IconTheme t) {
    switch (t) {
        case IconTheme::Light:        return "Light";
        case IconTheme::Dark:         return "Dark";
        case IconTheme::HighContrast: return "HighContrast";
        case IconTheme::Monochrome:   return "Monochrome";
    }
    return "Unknown";
}

enum class IconState : uint8_t {
    Normal, Hover, Pressed, Disabled, Selected
};

inline const char* iconStateName(IconState s) {
    switch (s) {
        case IconState::Normal:   return "Normal";
        case IconState::Hover:    return "Hover";
        case IconState::Pressed:  return "Pressed";
        case IconState::Disabled: return "Disabled";
        case IconState::Selected: return "Selected";
    }
    return "Unknown";
}

class IconAsset {
public:
    explicit IconAsset(const std::string& name,
                       IconSize           size = IconSize::Medium)
        : m_name(name), m_size(size) {}

    void setTheme(IconTheme t)  { m_theme = t; }
    void setState(IconState s)  { m_state = s; }
    void setSize(IconSize s)    { m_size  = s; }
    void setScalable(bool v)    { m_scalable = v; }
    void setDirty(bool v)       { m_dirty    = v; }
    void setPixelDensity(float f){ m_pixelDensity = f; }

    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] IconSize           size()          const { return m_size;         }
    [[nodiscard]] IconTheme          theme()         const { return m_theme;        }
    [[nodiscard]] IconState          state()         const { return m_state;        }
    [[nodiscard]] bool               isScalable()   const { return m_scalable;     }
    [[nodiscard]] bool               isDirty()      const { return m_dirty;        }
    [[nodiscard]] float              pixelDensity() const { return m_pixelDensity; }

    [[nodiscard]] bool isDisabled()  const { return m_state == IconState::Disabled; }
    [[nodiscard]] bool isSelected()  const { return m_state == IconState::Selected; }
    [[nodiscard]] bool isHighDPI()   const { return m_pixelDensity >= 2.0f; }

private:
    std::string m_name;
    IconSize    m_size         = IconSize::Medium;
    IconTheme   m_theme        = IconTheme::Light;
    IconState   m_state        = IconState::Normal;
    float       m_pixelDensity = 1.0f;
    bool        m_scalable     = false;
    bool        m_dirty        = false;
};

class IconEditor {
public:
    static constexpr size_t MAX_ICONS = 256;

    [[nodiscard]] bool addIcon(const IconAsset& icon) {
        for (auto& i : m_icons) if (i.name() == icon.name()) return false;
        if (m_icons.size() >= MAX_ICONS) return false;
        m_icons.push_back(icon);
        return true;
    }

    [[nodiscard]] bool removeIcon(const std::string& name) {
        for (auto it = m_icons.begin(); it != m_icons.end(); ++it) {
            if (it->name() == name) {
                if (m_activeIcon == name) m_activeIcon.clear();
                m_icons.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] IconAsset* findIcon(const std::string& name) {
        for (auto& i : m_icons) if (i.name() == name) return &i;
        return nullptr;
    }

    [[nodiscard]] bool setActiveIcon(const std::string& name) {
        for (auto& i : m_icons) {
            if (i.name() == name) { m_activeIcon = name; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t iconCount()      const { return m_icons.size(); }
    [[nodiscard]] const std::string& activeIcon() const { return m_activeIcon; }

    [[nodiscard]] size_t dirtyCount()    const {
        size_t c = 0; for (auto& i : m_icons) if (i.isDirty())    ++c; return c;
    }
    [[nodiscard]] size_t scalableCount() const {
        size_t c = 0; for (auto& i : m_icons) if (i.isScalable()) ++c; return c;
    }
    [[nodiscard]] size_t disabledCount() const {
        size_t c = 0; for (auto& i : m_icons) if (i.isDisabled()) ++c; return c;
    }
    [[nodiscard]] size_t highDPICount()  const {
        size_t c = 0; for (auto& i : m_icons) if (i.isHighDPI())  ++c; return c;
    }
    [[nodiscard]] size_t countByTheme(IconTheme t) const {
        size_t c = 0; for (auto& i : m_icons) if (i.theme() == t) ++c; return c;
    }
    [[nodiscard]] size_t countBySize(IconSize s) const {
        size_t c = 0; for (auto& i : m_icons) if (i.size() == s)  ++c; return c;
    }

private:
    std::vector<IconAsset> m_icons;
    std::string            m_activeIcon;
};

// ── S38 — Sprite Editor ───────────────────────────────────────────


} // namespace NF
