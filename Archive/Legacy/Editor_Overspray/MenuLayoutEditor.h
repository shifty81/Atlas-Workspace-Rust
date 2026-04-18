#pragma once
// NF::Editor — Menu layout editor
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

enum class MenuScreenType : uint8_t {
    MainMenu, PauseMenu, OptionsMenu, LoadingScreen, Credits, HUDOverlay, GameOver, Victory
};

inline const char* menuScreenTypeName(MenuScreenType t) {
    switch (t) {
        case MenuScreenType::MainMenu:     return "MainMenu";
        case MenuScreenType::PauseMenu:    return "PauseMenu";
        case MenuScreenType::OptionsMenu:  return "OptionsMenu";
        case MenuScreenType::LoadingScreen:return "LoadingScreen";
        case MenuScreenType::Credits:      return "Credits";
        case MenuScreenType::HUDOverlay:   return "HUDOverlay";
        case MenuScreenType::GameOver:     return "GameOver";
        case MenuScreenType::Victory:      return "Victory";
    }
    return "Unknown";
}

enum class MenuTransitionEffect : uint8_t {
    None, Fade, SlideLeft, SlideRight, SlideUp, SlideDown, Zoom, Dissolve
};

inline const char* menuTransitionEffectName(MenuTransitionEffect e) {
    switch (e) {
        case MenuTransitionEffect::None:       return "None";
        case MenuTransitionEffect::Fade:       return "Fade";
        case MenuTransitionEffect::SlideLeft:  return "SlideLeft";
        case MenuTransitionEffect::SlideRight: return "SlideRight";
        case MenuTransitionEffect::SlideUp:    return "SlideUp";
        case MenuTransitionEffect::SlideDown:  return "SlideDown";
        case MenuTransitionEffect::Zoom:       return "Zoom";
        case MenuTransitionEffect::Dissolve:   return "Dissolve";
    }
    return "Unknown";
}

enum class MenuNavigationMode : uint8_t {
    Mouse, Keyboard, Gamepad, Touch, Combined
};

inline const char* menuNavigationModeName(MenuNavigationMode m) {
    switch (m) {
        case MenuNavigationMode::Mouse:    return "Mouse";
        case MenuNavigationMode::Keyboard: return "Keyboard";
        case MenuNavigationMode::Gamepad:  return "Gamepad";
        case MenuNavigationMode::Touch:    return "Touch";
        case MenuNavigationMode::Combined: return "Combined";
    }
    return "Unknown";
}

class MenuScreen {
public:
    explicit MenuScreen(uint32_t id, const std::string& name, MenuScreenType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setTransitionIn(MenuTransitionEffect e)  { m_transitionIn  = e; }
    void setTransitionOut(MenuTransitionEffect e) { m_transitionOut = e; }
    void setTransitionDuration(float v)           { m_transitionDuration = v; }
    void setBlurBackground(bool v)                { m_blurBackground = v; }
    void setPauseGame(bool v)                     { m_pauseGame      = v; }

    [[nodiscard]] uint32_t             id()                 const { return m_id;               }
    [[nodiscard]] const std::string&   name()               const { return m_name;             }
    [[nodiscard]] MenuScreenType       type()               const { return m_type;             }
    [[nodiscard]] MenuTransitionEffect transitionIn()        const { return m_transitionIn;    }
    [[nodiscard]] MenuTransitionEffect transitionOut()       const { return m_transitionOut;   }
    [[nodiscard]] float                transitionDuration() const { return m_transitionDuration;}
    [[nodiscard]] bool                 isBlurBackground()   const { return m_blurBackground;  }
    [[nodiscard]] bool                 isPauseGame()         const { return m_pauseGame;        }

private:
    uint32_t              m_id;
    std::string           m_name;
    MenuScreenType        m_type;
    MenuTransitionEffect  m_transitionIn        = MenuTransitionEffect::Fade;
    MenuTransitionEffect  m_transitionOut       = MenuTransitionEffect::Fade;
    float                 m_transitionDuration  = 0.3f;
    bool                  m_blurBackground      = false;
    bool                  m_pauseGame           = false;
};

class MenuLayoutEditor {
public:
    void setNavigationMode(MenuNavigationMode m)  { m_navigationMode = m; }
    void setActiveScreenId(uint32_t id)           { m_activeScreenId = id; }
    void setShowSafeArea(bool v)                  { m_showSafeArea   = v; }
    void setShowOverlap(bool v)                   { m_showOverlap    = v; }

    bool addScreen(const MenuScreen& screen) {
        for (auto& s : m_screens) if (s.id() == screen.id()) return false;
        m_screens.push_back(screen); return true;
    }
    bool removeScreen(uint32_t id) {
        auto it = std::find_if(m_screens.begin(), m_screens.end(),
            [&](const MenuScreen& s){ return s.id() == id; });
        if (it == m_screens.end()) return false;
        m_screens.erase(it); return true;
    }
    [[nodiscard]] MenuScreen* findScreen(uint32_t id) {
        for (auto& s : m_screens) if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] MenuNavigationMode navigationMode()    const { return m_navigationMode; }
    [[nodiscard]] uint32_t           activeScreenId()   const { return m_activeScreenId; }
    [[nodiscard]] bool               isShowSafeArea()   const { return m_showSafeArea;   }
    [[nodiscard]] bool               isShowOverlap()    const { return m_showOverlap;    }
    [[nodiscard]] size_t             screenCount()      const { return m_screens.size(); }

    [[nodiscard]] size_t countByType(MenuScreenType t) const {
        size_t c = 0; for (auto& s : m_screens) if (s.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countPausingGame() const {
        size_t c = 0; for (auto& s : m_screens) if (s.isPauseGame()) ++c; return c;
    }

private:
    std::vector<MenuScreen> m_screens;
    MenuNavigationMode      m_navigationMode = MenuNavigationMode::Combined;
    uint32_t                m_activeScreenId = 0;
    bool                    m_showSafeArea   = true;
    bool                    m_showOverlap    = false;
};

} // namespace NF
