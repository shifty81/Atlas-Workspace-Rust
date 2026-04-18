#pragma once
// NF::Workspace — Phase 14: Workspace Plugin System
//
// Workspace plugin infrastructure:
//   PluginState       — lifecycle state machine
//   PluginCapability  — capability flags for sandboxing
//   PluginDescriptor  — plugin identity, version, dependencies, capabilities
//   PluginInstance     — runtime state for a loaded plugin
//   PluginRegistry    — register/discover/manage plugins with dependency resolution
//   PluginSandbox     — capability-based permission model

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// PluginState — lifecycle states
// ═════════════════════════════════════════════════════════════════

enum class PluginState : uint8_t {
    Unloaded    = 0,
    Discovered  = 1,
    Loaded      = 2,
    Activated   = 3,
    Deactivated = 4,
    Error       = 5,
};

inline const char* pluginStateName(PluginState s) {
    switch (s) {
        case PluginState::Unloaded:    return "Unloaded";
        case PluginState::Discovered:  return "Discovered";
        case PluginState::Loaded:      return "Loaded";
        case PluginState::Activated:   return "Activated";
        case PluginState::Deactivated: return "Deactivated";
        case PluginState::Error:       return "Error";
        default:                       return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// PluginCapability — capability flags
// ═════════════════════════════════════════════════════════════════

enum class PluginCapability : uint8_t {
    ReadSettings   = 0,   // Can read workspace settings
    WriteSettings  = 1,   // Can modify workspace settings
    RegisterTools  = 2,   // Can register new tools
    RegisterPanels = 3,   // Can register new panels
    FileSystem     = 4,   // Can access the file system
    Network        = 5,   // Can access the network
    EventBus       = 6,   // Can publish/subscribe workspace events
    Commands       = 7,   // Can register/execute commands
};

inline const char* pluginCapabilityName(PluginCapability c) {
    switch (c) {
        case PluginCapability::ReadSettings:   return "ReadSettings";
        case PluginCapability::WriteSettings:  return "WriteSettings";
        case PluginCapability::RegisterTools:  return "RegisterTools";
        case PluginCapability::RegisterPanels: return "RegisterPanels";
        case PluginCapability::FileSystem:     return "FileSystem";
        case PluginCapability::Network:        return "Network";
        case PluginCapability::EventBus:       return "EventBus";
        case PluginCapability::Commands:       return "Commands";
        default:                               return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// PluginVersion — semantic version for plugins
// ═════════════════════════════════════════════════════════════════

struct PluginVersion {
    uint16_t major = 0;
    uint16_t minor = 0;
    uint16_t patch = 0;

    [[nodiscard]] std::string toString() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    [[nodiscard]] bool isValid() const {
        return major > 0 || minor > 0 || patch > 0;
    }

    bool operator==(const PluginVersion& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
    bool operator!=(const PluginVersion& o) const { return !(*this == o); }
    bool operator<(const PluginVersion& o) const {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        return patch < o.patch;
    }
    bool operator>(const PluginVersion& o) const { return o < *this; }
    bool operator<=(const PluginVersion& o) const { return !(o < *this); }
    bool operator>=(const PluginVersion& o) const { return !(*this < o); }

    static PluginVersion make(uint16_t maj, uint16_t min, uint16_t pat) {
        return {maj, min, pat};
    }

    static bool parse(const std::string& s, PluginVersion& out) {
        auto dot1 = s.find('.');
        if (dot1 == std::string::npos) return false;
        auto dot2 = s.find('.', dot1 + 1);
        if (dot2 == std::string::npos) return false;
        try {
            out.major = static_cast<uint16_t>(std::stoi(s.substr(0, dot1)));
            out.minor = static_cast<uint16_t>(std::stoi(s.substr(dot1 + 1, dot2 - dot1 - 1)));
            out.patch = static_cast<uint16_t>(std::stoi(s.substr(dot2 + 1)));
            return true;
        } catch (...) { return false; }
    }
};

// ═════════════════════════════════════════════════════════════════
// PluginDescriptor — plugin identity and metadata
// ═════════════════════════════════════════════════════════════════

struct PluginDescriptor {
    std::string                     id;
    std::string                     displayName;
    std::string                     author;
    std::string                     description;
    PluginVersion                   version;
    std::vector<std::string>        dependencies;     // plugin IDs this depends on
    std::vector<PluginCapability>   requiredCapabilities;

    [[nodiscard]] bool isValid() const {
        return !id.empty() && !displayName.empty() && version.isValid();
    }

    [[nodiscard]] bool dependsOn(const std::string& pluginId) const {
        for (const auto& d : dependencies) if (d == pluginId) return true;
        return false;
    }

    [[nodiscard]] bool requiresCapability(PluginCapability cap) const {
        for (auto c : requiredCapabilities) if (c == cap) return true;
        return false;
    }
};

// ═════════════════════════════════════════════════════════════════
// PluginInstance — runtime state for a loaded plugin
// ═════════════════════════════════════════════════════════════════

class PluginInstance {
public:
    using ActivateFunc   = std::function<bool()>;
    using DeactivateFunc = std::function<void()>;

    explicit PluginInstance(const PluginDescriptor& desc)
        : m_descriptor(desc), m_state(PluginState::Discovered) {}

    [[nodiscard]] const PluginDescriptor& descriptor() const { return m_descriptor; }
    [[nodiscard]] const std::string& id()              const { return m_descriptor.id; }
    [[nodiscard]] PluginState state()                  const { return m_state; }
    [[nodiscard]] const std::string& errorMessage()    const { return m_error; }
    [[nodiscard]] bool isActive()                      const { return m_state == PluginState::Activated; }

    void setActivateHandler(ActivateFunc fn)   { m_activateFn = std::move(fn); }
    void setDeactivateHandler(DeactivateFunc fn) { m_deactivateFn = std::move(fn); }

    bool load() {
        if (m_state != PluginState::Discovered) return false;
        m_state = PluginState::Loaded;
        return true;
    }

    bool activate() {
        if (m_state != PluginState::Loaded && m_state != PluginState::Deactivated) return false;
        if (m_activateFn) {
            if (!m_activateFn()) {
                m_state = PluginState::Error;
                m_error = "activate failed";
                return false;
            }
        }
        m_state = PluginState::Activated;
        return true;
    }

    bool deactivate() {
        if (m_state != PluginState::Activated) return false;
        if (m_deactivateFn) m_deactivateFn();
        m_state = PluginState::Deactivated;
        return true;
    }

    bool unload() {
        if (m_state == PluginState::Activated) {
            deactivate();
        }
        if (m_state != PluginState::Loaded && m_state != PluginState::Deactivated &&
            m_state != PluginState::Error) return false;
        m_state = PluginState::Unloaded;
        return true;
    }

    void setError(const std::string& msg) {
        m_state = PluginState::Error;
        m_error = msg;
    }

private:
    PluginDescriptor m_descriptor;
    PluginState      m_state;
    std::string      m_error;
    ActivateFunc     m_activateFn;
    DeactivateFunc   m_deactivateFn;
};

// ═════════════════════════════════════════════════════════════════
// PluginSandbox — capability permission model
// ═════════════════════════════════════════════════════════════════

class PluginSandbox {
public:
    static constexpr size_t MAX_GRANTED = 32;

    // Grant a capability to a plugin.
    bool grant(const std::string& pluginId, PluginCapability cap) {
        for (const auto& g : m_grants) {
            if (g.pluginId == pluginId && g.capability == cap) return false; // already granted
        }
        if (m_grants.size() >= MAX_GRANTED) return false;
        m_grants.push_back({pluginId, cap});
        return true;
    }

    // Revoke a capability from a plugin.
    bool revoke(const std::string& pluginId, PluginCapability cap) {
        for (auto it = m_grants.begin(); it != m_grants.end(); ++it) {
            if (it->pluginId == pluginId && it->capability == cap) {
                m_grants.erase(it);
                return true;
            }
        }
        return false;
    }

    // Check if a plugin has a specific capability.
    [[nodiscard]] bool hasCapability(const std::string& pluginId, PluginCapability cap) const {
        for (const auto& g : m_grants) {
            if (g.pluginId == pluginId && g.capability == cap) return true;
        }
        return false;
    }

    // Grant all required capabilities from a descriptor.
    size_t grantRequired(const PluginDescriptor& desc) {
        size_t granted = 0;
        for (auto cap : desc.requiredCapabilities) {
            if (grant(desc.id, cap)) ++granted;
        }
        return granted;
    }

    // Revoke all capabilities for a plugin.
    size_t revokeAll(const std::string& pluginId) {
        size_t removed = 0;
        m_grants.erase(
            std::remove_if(m_grants.begin(), m_grants.end(),
                [&](const Grant& g) {
                    if (g.pluginId == pluginId) { ++removed; return true; }
                    return false;
                }),
            m_grants.end());
        return removed;
    }

    // Count capabilities for a plugin.
    [[nodiscard]] size_t countFor(const std::string& pluginId) const {
        size_t c = 0;
        for (const auto& g : m_grants) if (g.pluginId == pluginId) ++c;
        return c;
    }

    [[nodiscard]] size_t totalGrants() const { return m_grants.size(); }

    void clear() { m_grants.clear(); }

private:
    struct Grant {
        std::string      pluginId;
        PluginCapability capability;
    };
    std::vector<Grant> m_grants;
};

// ═════════════════════════════════════════════════════════════════
// PluginRegistry — plugin management with dependency resolution
// ═════════════════════════════════════════════════════════════════

class PluginRegistry {
public:
    static constexpr size_t MAX_PLUGINS = 128;

    // Register a plugin descriptor. Returns false if duplicate or invalid.
    bool registerPlugin(const PluginDescriptor& desc) {
        if (!desc.isValid()) return false;
        if (m_plugins.size() >= MAX_PLUGINS) return false;
        for (const auto& p : m_plugins) {
            if (p.id() == desc.id) return false;
        }
        m_plugins.emplace_back(desc);
        return true;
    }

    // Unregister a plugin by id. Must be unloaded first.
    bool unregisterPlugin(const std::string& id) {
        for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
            if (it->id() == id) {
                if (it->state() == PluginState::Activated) return false;
                m_plugins.erase(it);
                return true;
            }
        }
        return false;
    }

    // Find a plugin instance by id.
    [[nodiscard]] PluginInstance* find(const std::string& id) {
        for (auto& p : m_plugins) if (p.id() == id) return &p;
        return nullptr;
    }
    [[nodiscard]] const PluginInstance* find(const std::string& id) const {
        for (const auto& p : m_plugins) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] bool isRegistered(const std::string& id) const {
        return find(id) != nullptr;
    }

    // Check if all dependencies of a plugin are registered and activated.
    [[nodiscard]] bool areDependenciesMet(const std::string& id) const {
        auto* plugin = find(id);
        if (!plugin) return false;
        for (const auto& dep : plugin->descriptor().dependencies) {
            auto* depPlugin = find(dep);
            if (!depPlugin || !depPlugin->isActive()) return false;
        }
        return true;
    }

    // Load a plugin (Discovered → Loaded).
    bool loadPlugin(const std::string& id) {
        auto* p = find(id);
        if (!p) return false;
        return p->load();
    }

    // Activate a plugin (Loaded → Activated). Checks dependencies.
    bool activatePlugin(const std::string& id) {
        auto* p = find(id);
        if (!p) return false;
        if (!areDependenciesMet(id)) return false;
        return p->activate();
    }

    // Deactivate a plugin (Activated → Deactivated). Also deactivates dependents recursively.
    bool deactivatePlugin(const std::string& id) {
        auto* p = find(id);
        if (!p) return false;
        // Deactivate any plugins that depend on this one (recursive)
        for (auto& other : m_plugins) {
            if (other.isActive() && other.descriptor().dependsOn(id)) {
                deactivatePlugin(other.id());
            }
        }
        return p->deactivate();
    }

    // Unload a plugin.
    bool unloadPlugin(const std::string& id) {
        auto* p = find(id);
        if (!p) return false;
        return p->unload();
    }

    // Count active plugins.
    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (const auto& p : m_plugins) if (p.isActive()) ++c;
        return c;
    }

    // Get all plugins in a given state.
    [[nodiscard]] std::vector<const PluginInstance*> findByState(PluginState state) const {
        std::vector<const PluginInstance*> result;
        for (const auto& p : m_plugins) if (p.state() == state) result.push_back(&p);
        return result;
    }

    [[nodiscard]] size_t count() const { return m_plugins.size(); }
    [[nodiscard]] bool   empty() const { return m_plugins.empty(); }
    [[nodiscard]] const std::vector<PluginInstance>& all() const { return m_plugins; }

    void clear() { m_plugins.clear(); }

private:
    std::vector<PluginInstance> m_plugins;
};

} // namespace NF
