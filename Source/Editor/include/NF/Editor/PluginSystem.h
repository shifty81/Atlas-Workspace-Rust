#pragma once
// NF::Editor — Plugin registry, loader, plugin system
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

enum class PluginState : uint8_t {
    Unloaded = 0,
    Loading,
    Loaded,
    Active,
    Suspended,
    Error,
    Disabled,
    Unloading
};

inline const char* pluginStateName(PluginState s) {
    switch (s) {
        case PluginState::Unloaded:   return "Unloaded";
        case PluginState::Loading:    return "Loading";
        case PluginState::Loaded:     return "Loaded";
        case PluginState::Active:     return "Active";
        case PluginState::Suspended:  return "Suspended";
        case PluginState::Error:      return "Error";
        case PluginState::Disabled:   return "Disabled";
        case PluginState::Unloading:  return "Unloading";
        default:                      return "Unknown";
    }
}

struct PluginManifest {
    std::string id;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    std::vector<std::string> dependencies;

    [[nodiscard]] bool isValid() const { return !id.empty() && !name.empty() && !version.empty(); }
};

struct PluginInstance {
    PluginManifest manifest;
    PluginState state = PluginState::Unloaded;
    float loadTime = 0.f;
    std::string errorMessage;

    [[nodiscard]] bool isLoaded()   const { return state == PluginState::Loaded || state == PluginState::Active || state == PluginState::Suspended; }
    [[nodiscard]] bool isActive()   const { return state == PluginState::Active; }
    [[nodiscard]] bool hasError()   const { return state == PluginState::Error; }
    [[nodiscard]] bool isDisabled() const { return state == PluginState::Disabled; }

    bool activate() {
        if (state == PluginState::Loaded || state == PluginState::Suspended) {
            state = PluginState::Active;
            return true;
        }
        return false;
    }

    bool suspend() {
        if (state == PluginState::Active) {
            state = PluginState::Suspended;
            return true;
        }
        return false;
    }

    bool disable() {
        if (state != PluginState::Unloaded && state != PluginState::Error) {
            state = PluginState::Disabled;
            return true;
        }
        return false;
    }

    void setError(const std::string& msg) {
        state = PluginState::Error;
        errorMessage = msg;
    }
};

class PluginRegistry {
public:
    static constexpr size_t kMaxPlugins = 64;

    bool registerPlugin(const PluginManifest& manifest) {
        if (!manifest.isValid()) return false;
        if (m_plugins.size() >= kMaxPlugins) return false;
        for (const auto& p : m_plugins) {
            if (p.manifest.id == manifest.id) return false;
        }
        PluginInstance inst;
        inst.manifest = manifest;
        inst.state = PluginState::Unloaded;
        m_plugins.push_back(std::move(inst));
        return true;
    }

    bool unregisterPlugin(const std::string& pluginId) {
        for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
            if (it->manifest.id == pluginId) {
                m_plugins.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] PluginInstance* findPlugin(const std::string& pluginId) {
        for (auto& p : m_plugins) {
            if (p.manifest.id == pluginId) return &p;
        }
        return nullptr;
    }

    [[nodiscard]] const PluginInstance* findPlugin(const std::string& pluginId) const {
        for (const auto& p : m_plugins) {
            if (p.manifest.id == pluginId) return &p;
        }
        return nullptr;
    }

    [[nodiscard]] size_t pluginCount() const { return m_plugins.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t count = 0;
        for (const auto& p : m_plugins) {
            if (p.state != PluginState::Disabled && p.state != PluginState::Error) ++count;
        }
        return count;
    }

    [[nodiscard]] std::vector<const PluginInstance*> pluginsByState(PluginState s) const {
        std::vector<const PluginInstance*> result;
        for (const auto& p : m_plugins) {
            if (p.state == s) result.push_back(&p);
        }
        return result;
    }

    [[nodiscard]] const std::vector<PluginInstance>& plugins() const { return m_plugins; }

private:
    std::vector<PluginInstance> m_plugins;
};

class PluginLoader {
public:
    bool load(PluginRegistry& registry, const std::string& pluginId) {
        auto* inst = registry.findPlugin(pluginId);
        if (!inst) return false;
        if (inst->state != PluginState::Unloaded && inst->state != PluginState::Disabled) return false;
        inst->state = PluginState::Loading;
        inst->state = PluginState::Loaded;
        inst->loadTime = 0.f;
        inst->errorMessage.clear();
        m_loadCount++;
        return true;
    }

    bool unload(PluginRegistry& registry, const std::string& pluginId) {
        auto* inst = registry.findPlugin(pluginId);
        if (!inst) return false;
        if (!inst->isLoaded()) return false;
        inst->state = PluginState::Unloading;
        inst->state = PluginState::Unloaded;
        m_unloadCount++;
        return true;
    }

    bool reload(PluginRegistry& registry, const std::string& pluginId) {
        if (!unload(registry, pluginId)) return false;
        return load(registry, pluginId);
    }

    [[nodiscard]] size_t loadCount()   const { return m_loadCount; }
    [[nodiscard]] size_t unloadCount() const { return m_unloadCount; }
    [[nodiscard]] size_t errorCount()  const { return m_errorCount; }

private:
    size_t m_loadCount   = 0;
    size_t m_unloadCount = 0;
    size_t m_errorCount  = 0;
};

struct PluginSystemConfig {
    size_t maxPlugins = 32;
    bool autoActivateOnLoad = false;
};

class PluginSystem {
public:
    void init(const PluginSystemConfig& config = {}) {
        m_config = config;
        m_initialized = true;
    }

    void shutdown() {
        m_registry = PluginRegistry{};
        m_loader = PluginLoader{};
        m_initialized = false;
        m_tickCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool registerPlugin(const PluginManifest& manifest) {
        if (!m_initialized) return false;
        if (m_registry.pluginCount() >= m_config.maxPlugins) return false;
        return m_registry.registerPlugin(manifest);
    }

    bool loadPlugin(const std::string& pluginId) {
        if (!m_initialized) return false;
        if (!m_loader.load(m_registry, pluginId)) return false;
        if (m_config.autoActivateOnLoad) {
            auto* inst = m_registry.findPlugin(pluginId);
            if (inst) inst->activate();
        }
        return true;
    }

    bool unloadPlugin(const std::string& pluginId) {
        if (!m_initialized) return false;
        return m_loader.unload(m_registry, pluginId);
    }

    bool activatePlugin(const std::string& pluginId) {
        if (!m_initialized) return false;
        auto* inst = m_registry.findPlugin(pluginId);
        if (!inst) return false;
        return inst->activate();
    }

    bool suspendPlugin(const std::string& pluginId) {
        if (!m_initialized) return false;
        auto* inst = m_registry.findPlugin(pluginId);
        if (!inst) return false;
        return inst->suspend();
    }

    [[nodiscard]] PluginInstance* findPlugin(const std::string& pluginId) {
        return m_registry.findPlugin(pluginId);
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t activePluginCount() const {
        return m_registry.pluginsByState(PluginState::Active).size();
    }

    [[nodiscard]] size_t totalPluginCount() const { return m_registry.pluginCount(); }
    [[nodiscard]] size_t tickCount()         const { return m_tickCount; }
    [[nodiscard]] PluginRegistry&       registry()       { return m_registry; }
    [[nodiscard]] const PluginRegistry& registry() const { return m_registry; }
    [[nodiscard]] PluginLoader&         loader()         { return m_loader; }

private:
    PluginSystemConfig m_config;
    PluginRegistry     m_registry;
    PluginLoader       m_loader;
    bool               m_initialized = false;
    size_t             m_tickCount   = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// S15 — Scripting Console
// ─────────────────────────────────────────────────────────────────────────────


} // namespace NF
