#pragma once
// NF::Editor — Hot reload manager v1: module watch registration and reload lifecycle authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Hrmv1ReloadScope  : uint8_t { Script, Shader, Asset, Config, Plugin, All };
enum class Hrmv1WatchState   : uint8_t { Inactive, Watching, Pending, Reloading, Failed };
enum class Hrmv1TriggerMode  : uint8_t { OnSave, OnFocus, Manual, Timed };

inline const char* hrmv1ReloadScopeName(Hrmv1ReloadScope s) {
    switch (s) {
        case Hrmv1ReloadScope::Script: return "Script";
        case Hrmv1ReloadScope::Shader: return "Shader";
        case Hrmv1ReloadScope::Asset:  return "Asset";
        case Hrmv1ReloadScope::Config: return "Config";
        case Hrmv1ReloadScope::Plugin: return "Plugin";
        case Hrmv1ReloadScope::All:    return "All";
    }
    return "Unknown";
}

inline const char* hrmv1WatchStateName(Hrmv1WatchState s) {
    switch (s) {
        case Hrmv1WatchState::Inactive:  return "Inactive";
        case Hrmv1WatchState::Watching:  return "Watching";
        case Hrmv1WatchState::Pending:   return "Pending";
        case Hrmv1WatchState::Reloading: return "Reloading";
        case Hrmv1WatchState::Failed:    return "Failed";
    }
    return "Unknown";
}

inline const char* hrmv1TriggerModeName(Hrmv1TriggerMode m) {
    switch (m) {
        case Hrmv1TriggerMode::OnSave:   return "OnSave";
        case Hrmv1TriggerMode::OnFocus:  return "OnFocus";
        case Hrmv1TriggerMode::Manual:   return "Manual";
        case Hrmv1TriggerMode::Timed:    return "Timed";
    }
    return "Unknown";
}

struct Hrmv1WatchEntry {
    uint64_t           id            = 0;
    std::string        name;
    std::string        watchPath;
    Hrmv1ReloadScope   scope         = Hrmv1ReloadScope::Script;
    Hrmv1WatchState    state         = Hrmv1WatchState::Inactive;
    Hrmv1TriggerMode   triggerMode   = Hrmv1TriggerMode::OnSave;
    uint32_t           reloadCount   = 0;
    uint32_t           failCount     = 0;
    bool               isEnabled     = true;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty() && !watchPath.empty(); }
    [[nodiscard]] bool isWatching()  const { return state == Hrmv1WatchState::Watching; }
    [[nodiscard]] bool isReloading() const { return state == Hrmv1WatchState::Reloading; }
    [[nodiscard]] bool hasFailed()   const { return state == Hrmv1WatchState::Failed; }
};

using Hrmv1ChangeCallback = std::function<void(uint64_t)>;

class HotReloadManagerV1 {
public:
    static constexpr size_t MAX_ENTRIES = 256;

    bool addEntry(const Hrmv1WatchEntry& entry) {
        if (!entry.isValid()) return false;
        for (const auto& e : m_entries) if (e.id == entry.id) return false;
        if (m_entries.size() >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        return true;
    }

    bool removeEntry(uint64_t id) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->id == id) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Hrmv1WatchEntry* findEntry(uint64_t id) {
        for (auto& e : m_entries) if (e.id == id) return &e;
        return nullptr;
    }

    bool setState(uint64_t id, Hrmv1WatchState state) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setEnabled(uint64_t id, bool enabled) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->isEnabled = enabled;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool triggerReload(uint64_t id) {
        auto* e = findEntry(id);
        if (!e || !e->isEnabled) return false;
        e->state = Hrmv1WatchState::Reloading;
        ++e->reloadCount;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool recordFailure(uint64_t id) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->state = Hrmv1WatchState::Failed;
        ++e->failCount;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t entryCount()     const { return m_entries.size(); }
    [[nodiscard]] size_t watchingCount()  const {
        size_t c = 0; for (const auto& e : m_entries) if (e.isWatching())  ++c; return c;
    }
    [[nodiscard]] size_t reloadingCount() const {
        size_t c = 0; for (const auto& e : m_entries) if (e.isReloading()) ++c; return c;
    }
    [[nodiscard]] size_t failedCount()    const {
        size_t c = 0; for (const auto& e : m_entries) if (e.hasFailed())   ++c; return c;
    }
    [[nodiscard]] size_t enabledCount()   const {
        size_t c = 0; for (const auto& e : m_entries) if (e.isEnabled)     ++c; return c;
    }
    [[nodiscard]] size_t countByScope(Hrmv1ReloadScope scope) const {
        size_t c = 0; for (const auto& e : m_entries) if (e.scope == scope) ++c; return c;
    }
    [[nodiscard]] size_t countByTrigger(Hrmv1TriggerMode mode) const {
        size_t c = 0; for (const auto& e : m_entries) if (e.triggerMode == mode) ++c; return c;
    }

    void setOnChange(Hrmv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Hrmv1WatchEntry> m_entries;
    Hrmv1ChangeCallback          m_onChange;
};

} // namespace NF
