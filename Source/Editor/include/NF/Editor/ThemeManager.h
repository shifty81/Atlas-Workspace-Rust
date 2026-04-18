#pragma once
// NF::Editor — Theme + theme manager
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

enum class ThemeMode : uint8_t { Light, Dark, HighContrast, Custom };

inline const char* themeModeName(ThemeMode m) {
    switch (m) {
        case ThemeMode::Light:       return "Light";
        case ThemeMode::Dark:        return "Dark";
        case ThemeMode::HighContrast:return "HighContrast";
        case ThemeMode::Custom:      return "Custom";
        default:                     return "Unknown";
    }
}

enum class ThemeColor : uint8_t { Background, Foreground, Primary, Secondary, Accent, Border, Error, Warning };

inline const char* themeColorName(ThemeColor c) {
    switch (c) {
        case ThemeColor::Background: return "Background";
        case ThemeColor::Foreground: return "Foreground";
        case ThemeColor::Primary:    return "Primary";
        case ThemeColor::Secondary:  return "Secondary";
        case ThemeColor::Accent:     return "Accent";
        case ThemeColor::Border:     return "Border";
        case ThemeColor::Error:      return "Error";
        case ThemeColor::Warning:    return "Warning";
        default:                     return "Unknown";
    }
}

struct ThemeToken {
    std::string key;
    std::string value;
    ThemeMode   mode = ThemeMode::Dark;

    [[nodiscard]] bool matches(ThemeMode m) const { return mode == m; }
    void update(const std::string& newValue)      { value = newValue; }
};

struct Theme {
    std::string name;
    ThemeMode   m_mode    = ThemeMode::Dark;
    uint32_t    version   = 1;

    bool addToken(ThemeToken t) {
        for (auto& existing : m_tokens) if (existing.key == t.key) return false;
        m_tokens.push_back(std::move(t));
        return true;
    }

    bool removeToken(const std::string& key) {
        for (auto it = m_tokens.begin(); it != m_tokens.end(); ++it) {
            if (it->key == key) { m_tokens.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ThemeToken* findToken(const std::string& key) {
        for (auto& t : m_tokens) if (t.key == key) return &t;
        return nullptr;
    }

    [[nodiscard]] size_t    tokenCount() const { return m_tokens.size(); }
    [[nodiscard]] ThemeMode mode()       const { return m_mode;          }
    [[nodiscard]] const std::string& name_() const { return name;        }

    void setMode(ThemeMode m) { m_mode = m; }
    void bumpVersion()        { version++;  }

private:
    std::vector<ThemeToken> m_tokens;
};

class ThemeManager {
public:
    static constexpr size_t MAX_THEMES = 32;

    bool addTheme(Theme t) {
        if (m_themes.size() >= MAX_THEMES) return false;
        for (auto& existing : m_themes) if (existing.name == t.name) return false;
        m_themes.push_back(std::move(t));
        return true;
    }

    bool removeTheme(const std::string& name) {
        for (auto it = m_themes.begin(); it != m_themes.end(); ++it) {
            if (it->name == name) {
                if (m_active == &*it) m_active = nullptr;
                m_themes.erase(it);
                return true;
            }
        }
        return false;
    }

    bool setActive(const std::string& name) {
        for (auto& t : m_themes) {
            if (t.name == name) { m_active = &t; return true; }
        }
        return false;
    }

    [[nodiscard]] Theme* active() { return m_active; }

    [[nodiscard]] Theme* find(const std::string& name) {
        for (auto& t : m_themes) if (t.name == name) return &t;
        return nullptr;
    }

    [[nodiscard]] size_t themeCount() const { return m_themes.size(); }
    [[nodiscard]] bool   hasActive()  const { return m_active != nullptr; }

    bool applyMode(ThemeMode m) {
        if (!m_active) return false;
        m_active->setMode(m);
        return true;
    }

private:
    std::vector<Theme> m_themes;
    Theme*             m_active = nullptr;
};

// ============================================================
// S28 — Keyframe Animation Editor
// ============================================================


} // namespace NF
