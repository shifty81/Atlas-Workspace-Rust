#pragma once
// NF::Editor — display/monitor mode management editor
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

enum class DisplayApi : uint8_t { DirectX11, DirectX12, Vulkan, Metal, OpenGL };
inline const char* displayApiName(DisplayApi v) {
    switch (v) {
        case DisplayApi::DirectX11: return "DirectX11";
        case DisplayApi::DirectX12: return "DirectX12";
        case DisplayApi::Vulkan:    return "Vulkan";
        case DisplayApi::Metal:     return "Metal";
        case DisplayApi::OpenGL:    return "OpenGL";
    }
    return "Unknown";
}

enum class DisplaySync : uint8_t { Off, VSync, AdaptiveSync, FreeSync, GSync };
inline const char* displaySyncName(DisplaySync v) {
    switch (v) {
        case DisplaySync::Off:          return "Off";
        case DisplaySync::VSync:        return "VSync";
        case DisplaySync::AdaptiveSync: return "AdaptiveSync";
        case DisplaySync::FreeSync:     return "FreeSync";
        case DisplaySync::GSync:        return "GSync";
    }
    return "Unknown";
}

class DisplayMode {
public:
    explicit DisplayMode(uint32_t id, const std::string& name,
                          DisplayApi api, DisplaySync sync)
        : m_id(id), m_name(name), m_api(api), m_sync(sync) {}

    void setRefreshRate(uint32_t v)  { m_refreshRate  = v; }
    void setIsFullscreen(bool v)     { m_isFullscreen = v; }
    void setIsEnabled(bool v)        { m_isEnabled    = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] DisplayApi         api()         const { return m_api;         }
    [[nodiscard]] DisplaySync        sync()        const { return m_sync;        }
    [[nodiscard]] uint32_t           refreshRate() const { return m_refreshRate; }
    [[nodiscard]] bool               isFullscreen()const { return m_isFullscreen;}
    [[nodiscard]] bool               isEnabled()   const { return m_isEnabled;   }

private:
    uint32_t    m_id;
    std::string m_name;
    DisplayApi  m_api;
    DisplaySync m_sync;
    uint32_t    m_refreshRate  = 60u;
    bool        m_isFullscreen = false;
    bool        m_isEnabled    = true;
};

class DisplayModeEditor {
public:
    void setIsShowDisabled(bool v)       { m_isShowDisabled    = v; }
    void setIsGroupByApi(bool v)         { m_isGroupByApi      = v; }
    void setDefaultRefreshRate(uint32_t v){ m_defaultRefreshRate = v; }

    bool addMode(const DisplayMode& m) {
        for (auto& x : m_modes) if (x.id() == m.id()) return false;
        m_modes.push_back(m); return true;
    }
    bool removeMode(uint32_t id) {
        auto it = std::find_if(m_modes.begin(), m_modes.end(),
            [&](const DisplayMode& m){ return m.id() == id; });
        if (it == m_modes.end()) return false;
        m_modes.erase(it); return true;
    }
    [[nodiscard]] DisplayMode* findMode(uint32_t id) {
        for (auto& m : m_modes) if (m.id() == id) return &m;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()    const { return m_isShowDisabled;    }
    [[nodiscard]] bool     isGroupByApi()      const { return m_isGroupByApi;      }
    [[nodiscard]] uint32_t defaultRefreshRate()const { return m_defaultRefreshRate;}
    [[nodiscard]] size_t   modeCount()         const { return m_modes.size();      }

    [[nodiscard]] size_t countByApi(DisplayApi a) const {
        size_t n = 0; for (auto& m : m_modes) if (m.api() == a) ++n; return n;
    }
    [[nodiscard]] size_t countBySync(DisplaySync s) const {
        size_t n = 0; for (auto& m : m_modes) if (m.sync() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& m : m_modes) if (m.isEnabled()) ++n; return n;
    }

private:
    std::vector<DisplayMode> m_modes;
    bool     m_isShowDisabled     = false;
    bool     m_isGroupByApi       = false;
    uint32_t m_defaultRefreshRate = 60u;
};

} // namespace NF
