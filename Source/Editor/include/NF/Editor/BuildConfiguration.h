#pragma once
// NF::Editor — Build profile, build configuration system
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

enum class BuildTarget : uint8_t {
    Executable   = 0,
    SharedLib    = 1,
    StaticLib    = 2,
    HeaderOnly   = 3,
    TestSuite    = 4,
    Plugin       = 5,
    Shader       = 6,
    ContentPack  = 7,
};

inline const char* buildTargetName(BuildTarget t) {
    switch (t) {
        case BuildTarget::Executable:  return "Executable";
        case BuildTarget::SharedLib:   return "SharedLib";
        case BuildTarget::StaticLib:   return "StaticLib";
        case BuildTarget::HeaderOnly:  return "HeaderOnly";
        case BuildTarget::TestSuite:   return "TestSuite";
        case BuildTarget::Plugin:      return "Plugin";
        case BuildTarget::Shader:      return "Shader";
        case BuildTarget::ContentPack: return "ContentPack";
        default:                       return "Unknown";
    }
}

enum class BuildPlatform : uint8_t {
    Windows  = 0,
    Linux    = 1,
    MacOS    = 2,
    WebAsm   = 3,
    Console  = 4,
};

inline const char* buildPlatformName(BuildPlatform p) {
    switch (p) {
        case BuildPlatform::Windows: return "Windows";
        case BuildPlatform::Linux:   return "Linux";
        case BuildPlatform::MacOS:   return "MacOS";
        case BuildPlatform::WebAsm:  return "WebAsm";
        case BuildPlatform::Console: return "Console";
        default:                     return "Unknown";
    }
}

struct BuildConfig {
    std::string   name;
    BuildTarget   target   = BuildTarget::Executable;
    BuildPlatform platform = BuildPlatform::Windows;
    bool          debugSymbols   = false;
    bool          optimized      = false;
    bool          sanitizers     = false;

    std::vector<std::string> defines;
    std::vector<std::string> includePaths;

    [[nodiscard]] bool isDebug()   const { return debugSymbols && !optimized; }
    [[nodiscard]] bool isRelease() const { return optimized && !debugSymbols; }

    bool addDefine(const std::string& def) {
        for (auto& d : defines) if (d == def) return false;
        defines.push_back(def);
        return true;
    }

    bool addIncludePath(const std::string& path) {
        for (auto& p : includePaths) if (p == path) return false;
        includePaths.push_back(path);
        return true;
    }

    [[nodiscard]] size_t defineCount()      const { return defines.size(); }
    [[nodiscard]] size_t includePathCount() const { return includePaths.size(); }
};

class BuildProfile {
public:
    static constexpr size_t MAX_CONFIGS = 64;

    bool addConfig(const BuildConfig& cfg) {
        if (m_configs.size() >= MAX_CONFIGS) return false;
        for (auto& c : m_configs) if (c.name == cfg.name) return false;
        m_configs.push_back(cfg);
        return true;
    }

    bool removeConfig(const std::string& name) {
        for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
            if (it->name == name) { m_configs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] BuildConfig* findConfig(const std::string& name) {
        for (auto& c : m_configs) if (c.name == name) return &c;
        return nullptr;
    }

    [[nodiscard]] const BuildConfig* findConfig(const std::string& name) const {
        for (auto& c : m_configs) if (c.name == name) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t configCount() const { return m_configs.size(); }

    [[nodiscard]] size_t debugConfigCount() const {
        size_t c = 0;
        for (auto& cfg : m_configs) if (cfg.isDebug()) c++;
        return c;
    }

    [[nodiscard]] size_t releaseConfigCount() const {
        size_t c = 0;
        for (auto& cfg : m_configs) if (cfg.isRelease()) c++;
        return c;
    }

    [[nodiscard]] const std::vector<BuildConfig>& configs() const { return m_configs; }

private:
    std::vector<BuildConfig> m_configs;
};

class BuildConfigurationSystem {
public:
    void init() { m_initialized = true; m_activeProfile.clear(); }
    void shutdown() { m_profiles.clear(); m_activeProfile.clear(); m_initialized = false; }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool createProfile(const std::string& name) {
        if (!m_initialized) return false;
        if (m_profiles.size() >= 16) return false;
        for (auto& p : m_profiles) if (p.first == name) return false;
        m_profiles.push_back({name, BuildProfile{}});
        return true;
    }

    bool removeProfile(const std::string& name) {
        if (!m_initialized) return false;
        for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
            if (it->first == name) {
                if (m_activeProfile == name) m_activeProfile.clear();
                m_profiles.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] BuildProfile* findProfile(const std::string& name) {
        for (auto& p : m_profiles) if (p.first == name) return &p.second;
        return nullptr;
    }

    bool setActiveProfile(const std::string& name) {
        if (!m_initialized) return false;
        for (auto& p : m_profiles) {
            if (p.first == name) { m_activeProfile = name; return true; }
        }
        return false;
    }

    [[nodiscard]] const std::string& activeProfileName() const { return m_activeProfile; }

    [[nodiscard]] BuildProfile* activeProfile() {
        if (m_activeProfile.empty()) return nullptr;
        return findProfile(m_activeProfile);
    }

    [[nodiscard]] size_t profileCount() const { return m_profiles.size(); }

    [[nodiscard]] size_t totalConfigCount() const {
        size_t c = 0;
        for (auto& p : m_profiles) c += p.second.configCount();
        return c;
    }

private:
    std::vector<std::pair<std::string, BuildProfile>> m_profiles;
    std::string m_activeProfile;
    bool m_initialized = false;
};

// ============================================================
// S19 — Scene Snapshot System
// ============================================================


} // namespace NF
