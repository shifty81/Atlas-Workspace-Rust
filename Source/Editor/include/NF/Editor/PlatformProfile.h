#pragma once
// NF::Editor — Platform-specific build profile
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

enum class PlatformFeatureSet : uint8_t {
    Desktop, Mobile, Console, XR, Server, Embedded
};

inline const char* platformFeatureSetName(PlatformFeatureSet f) {
    switch (f) {
        case PlatformFeatureSet::Desktop:  return "Desktop";
        case PlatformFeatureSet::Mobile:   return "Mobile";
        case PlatformFeatureSet::Console:  return "Console";
        case PlatformFeatureSet::XR:       return "XR";
        case PlatformFeatureSet::Server:   return "Server";
        case PlatformFeatureSet::Embedded: return "Embedded";
    }
    return "Unknown";
}

enum class PlatformAPILevel : uint8_t {
    Minimum, Recommended, Latest, Custom
};

inline const char* platformAPILevelName(PlatformAPILevel l) {
    switch (l) {
        case PlatformAPILevel::Minimum:     return "Minimum";
        case PlatformAPILevel::Recommended: return "Recommended";
        case PlatformAPILevel::Latest:      return "Latest";
        case PlatformAPILevel::Custom:      return "Custom";
    }
    return "Unknown";
}

enum class PlatformOrientation : uint8_t {
    Portrait, Landscape, Both, Auto
};

inline const char* platformOrientationName(PlatformOrientation o) {
    switch (o) {
        case PlatformOrientation::Portrait:  return "Portrait";
        case PlatformOrientation::Landscape: return "Landscape";
        case PlatformOrientation::Both:      return "Both";
        case PlatformOrientation::Auto:      return "Auto";
    }
    return "Unknown";
}

class PlatformCapability {
public:
    explicit PlatformCapability(const std::string& name, bool required)
        : m_name(name), m_required(required) {}

    void setEnabled(bool v)               { m_enabled  = v; }
    void setDescription(const std::string& d) { m_description = d; }

    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] const std::string& description() const { return m_description; }
    [[nodiscard]] bool               isRequired()  const { return m_required;    }
    [[nodiscard]] bool               isEnabled()   const { return m_enabled;     }

private:
    std::string m_name;
    std::string m_description;
    bool        m_required = false;
    bool        m_enabled  = true;
};

class PlatformProfile {
public:
    static constexpr size_t MAX_CAPABILITIES = 64;

    explicit PlatformProfile(const std::string& name, PlatformFeatureSet featureSet)
        : m_name(name), m_featureSet(featureSet) {}

    [[nodiscard]] bool addCapability(const PlatformCapability& cap) {
        for (auto& c : m_capabilities) if (c.name() == cap.name()) return false;
        if (m_capabilities.size() >= MAX_CAPABILITIES) return false;
        m_capabilities.push_back(cap);
        return true;
    }

    [[nodiscard]] bool removeCapability(const std::string& name) {
        for (auto it = m_capabilities.begin(); it != m_capabilities.end(); ++it) {
            if (it->name() == name) { m_capabilities.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] PlatformCapability* findCapability(const std::string& name) {
        for (auto& c : m_capabilities) if (c.name() == name) return &c;
        return nullptr;
    }

    void setAPILevel(PlatformAPILevel l)        { m_apiLevel     = l; }
    void setOrientation(PlatformOrientation o)  { m_orientation  = o; }
    void setActive(bool v)                      { m_active       = v; }

    [[nodiscard]] const std::string&  name()             const { return m_name;        }
    [[nodiscard]] PlatformFeatureSet  featureSet()       const { return m_featureSet;  }
    [[nodiscard]] PlatformAPILevel    apiLevel()         const { return m_apiLevel;    }
    [[nodiscard]] PlatformOrientation orientation()      const { return m_orientation; }
    [[nodiscard]] bool                isActive()         const { return m_active;      }
    [[nodiscard]] size_t              capabilityCount()  const { return m_capabilities.size(); }

    [[nodiscard]] size_t requiredCapabilityCount() const {
        size_t c = 0; for (auto& cap : m_capabilities) if (cap.isRequired()) ++c; return c;
    }
    [[nodiscard]] size_t enabledCapabilityCount() const {
        size_t c = 0; for (auto& cap : m_capabilities) if (cap.isEnabled()) ++c; return c;
    }

private:
    std::string                  m_name;
    PlatformFeatureSet           m_featureSet;
    std::vector<PlatformCapability> m_capabilities;
    PlatformAPILevel             m_apiLevel     = PlatformAPILevel::Recommended;
    PlatformOrientation          m_orientation  = PlatformOrientation::Landscape;
    bool                         m_active       = false;
};

} // namespace NF
