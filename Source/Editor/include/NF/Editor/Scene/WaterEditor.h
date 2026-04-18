#pragma once
// NF::Editor — Water surface editor
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

enum class WaterBodyType : uint8_t {
    Ocean, Lake, River, Pond, Waterfall
};

inline const char* waterBodyTypeName(WaterBodyType t) {
    switch (t) {
        case WaterBodyType::Ocean:     return "Ocean";
        case WaterBodyType::Lake:      return "Lake";
        case WaterBodyType::River:     return "River";
        case WaterBodyType::Pond:      return "Pond";
        case WaterBodyType::Waterfall: return "Waterfall";
    }
    return "Unknown";
}

enum class WaterRenderMode : uint8_t {
    Simple, FFT, Gerstner, Planar, Volumetric
};

inline const char* waterRenderModeName(WaterRenderMode m) {
    switch (m) {
        case WaterRenderMode::Simple:    return "Simple";
        case WaterRenderMode::FFT:       return "FFT";
        case WaterRenderMode::Gerstner:  return "Gerstner";
        case WaterRenderMode::Planar:    return "Planar";
        case WaterRenderMode::Volumetric:return "Volumetric";
    }
    return "Unknown";
}

enum class WaterFlowDir : uint8_t {
    North, South, East, West, Circular, None
};

inline const char* waterFlowDirName(WaterFlowDir d) {
    switch (d) {
        case WaterFlowDir::North:    return "North";
        case WaterFlowDir::South:    return "South";
        case WaterFlowDir::East:     return "East";
        case WaterFlowDir::West:     return "West";
        case WaterFlowDir::Circular: return "Circular";
        case WaterFlowDir::None:     return "None";
    }
    return "Unknown";
}

class WaterBody {
public:
    explicit WaterBody(const std::string& name, WaterBodyType type)
        : m_name(name), m_type(type) {}

    void setRenderMode(WaterRenderMode m) { m_renderMode = m;    }
    void setFlowDir(WaterFlowDir d)       { m_flowDir    = d;    }
    void setDepth(float d)                { m_depth      = d;    }
    void setWaveHeight(float h)           { m_waveHeight = h;    }
    void setFlowSpeed(float s)            { m_flowSpeed  = s;    }
    void setTransparency(float t)         { m_transparency = t;  }
    void setEnabled(bool v)               { m_enabled    = v;    }

    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] WaterBodyType      type()         const { return m_type;         }
    [[nodiscard]] WaterRenderMode    renderMode()   const { return m_renderMode;   }
    [[nodiscard]] WaterFlowDir       flowDir()      const { return m_flowDir;      }
    [[nodiscard]] float              depth()        const { return m_depth;        }
    [[nodiscard]] float              waveHeight()   const { return m_waveHeight;   }
    [[nodiscard]] float              flowSpeed()    const { return m_flowSpeed;    }
    [[nodiscard]] float              transparency() const { return m_transparency; }
    [[nodiscard]] bool               isEnabled()    const { return m_enabled;      }

private:
    std::string     m_name;
    WaterBodyType   m_type;
    WaterRenderMode m_renderMode  = WaterRenderMode::Gerstner;
    WaterFlowDir    m_flowDir     = WaterFlowDir::None;
    float           m_depth       = 10.0f;
    float           m_waveHeight  = 0.5f;
    float           m_flowSpeed   = 1.0f;
    float           m_transparency = 0.7f;
    bool            m_enabled     = true;
};

class WaterEditor {
public:
    static constexpr size_t MAX_BODIES = 64;

    [[nodiscard]] bool addBody(const WaterBody& body) {
        for (auto& b : m_bodies) if (b.name() == body.name()) return false;
        if (m_bodies.size() >= MAX_BODIES) return false;
        m_bodies.push_back(body);
        return true;
    }

    [[nodiscard]] bool removeBody(const std::string& name) {
        for (auto it = m_bodies.begin(); it != m_bodies.end(); ++it) {
            if (it->name() == name) {
                if (m_activeBody == name) m_activeBody.clear();
                m_bodies.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] WaterBody* findBody(const std::string& name) {
        for (auto& b : m_bodies) if (b.name() == name) return &b;
        return nullptr;
    }

    [[nodiscard]] bool setActiveBody(const std::string& name) {
        for (auto& b : m_bodies) if (b.name() == name) { m_activeBody = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeBody()  const { return m_activeBody; }
    [[nodiscard]] size_t             bodyCount()   const { return m_bodies.size(); }

    [[nodiscard]] size_t countByType(WaterBodyType t) const {
        size_t c = 0; for (auto& b : m_bodies) if (b.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByRenderMode(WaterRenderMode m) const {
        size_t c = 0; for (auto& b : m_bodies) if (b.renderMode() == m) ++c; return c;
    }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& b : m_bodies) if (b.isEnabled()) ++c; return c;
    }

private:
    std::vector<WaterBody> m_bodies;
    std::string            m_activeBody;
};

} // namespace NF
