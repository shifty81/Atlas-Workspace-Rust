#pragma once
// NF::Editor — Plugin system v1: plugin registration, loading, lifecycle management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Psyv1PluginState    : uint8_t { Unloaded, Loading, Loaded, Failed, Disabled };
enum class Psyv1PluginCategory : uint8_t { Importer, Exporter, Tool, Theme, Extension };

inline const char* psyv1PluginStateName(Psyv1PluginState s) {
    switch (s) {
        case Psyv1PluginState::Unloaded:  return "Unloaded";
        case Psyv1PluginState::Loading:   return "Loading";
        case Psyv1PluginState::Loaded:    return "Loaded";
        case Psyv1PluginState::Failed:    return "Failed";
        case Psyv1PluginState::Disabled:  return "Disabled";
    }
    return "Unknown";
}

inline const char* psyv1PluginCategoryName(Psyv1PluginCategory c) {
    switch (c) {
        case Psyv1PluginCategory::Importer:  return "Importer";
        case Psyv1PluginCategory::Exporter:  return "Exporter";
        case Psyv1PluginCategory::Tool:      return "Tool";
        case Psyv1PluginCategory::Theme:     return "Theme";
        case Psyv1PluginCategory::Extension: return "Extension";
    }
    return "Unknown";
}

struct Psyv1PluginDescriptor {
    uint64_t             id       = 0;
    std::string          name;
    std::string          version;
    Psyv1PluginCategory  category = Psyv1PluginCategory::Extension;
    Psyv1PluginState     state    = Psyv1PluginState::Unloaded;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isLoaded()   const { return state == Psyv1PluginState::Loaded; }
    [[nodiscard]] bool hasFailed()  const { return state == Psyv1PluginState::Failed; }
    [[nodiscard]] bool isDisabled() const { return state == Psyv1PluginState::Disabled; }
};

using Psyv1StateChangeCallback = std::function<void(uint64_t)>;

class PluginSystemV1 {
public:
    static constexpr size_t MAX_PLUGINS = 256;

    bool registerPlugin(const Psyv1PluginDescriptor& plugin) {
        if (!plugin.isValid()) return false;
        for (const auto& p : m_plugins) if (p.id == plugin.id) return false;
        if (m_plugins.size() >= MAX_PLUGINS) return false;
        m_plugins.push_back(plugin);
        return true;
    }

    bool unregisterPlugin(uint64_t id) {
        for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
            if (it->id == id) { m_plugins.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Psyv1PluginDescriptor* findPlugin(uint64_t id) {
        for (auto& p : m_plugins) if (p.id == id) return &p;
        return nullptr;
    }

    bool loadPlugin(uint64_t id) {
        auto* p = findPlugin(id);
        if (!p) return false;
        if (p->isDisabled()) return false;
        p->state = Psyv1PluginState::Loaded;
        if (m_onStateChange) m_onStateChange(id);
        return true;
    }

    bool unloadPlugin(uint64_t id) {
        auto* p = findPlugin(id);
        if (!p) return false;
        p->state = Psyv1PluginState::Unloaded;
        if (m_onStateChange) m_onStateChange(id);
        return true;
    }

    bool setState(uint64_t id, Psyv1PluginState state) {
        auto* p = findPlugin(id);
        if (!p) return false;
        p->state = state;
        if (m_onStateChange) m_onStateChange(id);
        return true;
    }

    [[nodiscard]] size_t pluginCount()  const { return m_plugins.size(); }
    [[nodiscard]] size_t loadedCount()  const {
        size_t c = 0; for (const auto& p : m_plugins) if (p.isLoaded())  ++c; return c;
    }
    [[nodiscard]] size_t failedCount()  const {
        size_t c = 0; for (const auto& p : m_plugins) if (p.hasFailed()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(Psyv1PluginCategory category) const {
        size_t c = 0; for (const auto& p : m_plugins) if (p.category == category) ++c; return c;
    }

    void setOnStateChange(Psyv1StateChangeCallback cb) { m_onStateChange = std::move(cb); }

private:
    std::vector<Psyv1PluginDescriptor> m_plugins;
    Psyv1StateChangeCallback           m_onStateChange;
};

} // namespace NF
