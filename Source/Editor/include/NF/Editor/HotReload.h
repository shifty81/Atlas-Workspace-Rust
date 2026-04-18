#pragma once
// NF::Editor — Hot reload watcher, dispatcher, system
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

enum class HotReloadAssetType : uint8_t {
    Script   = 0,
    Shader   = 1,
    Texture  = 2,
    Mesh     = 3,
    Audio    = 4,
    Config   = 5,
    Level    = 6,
    Material = 7
};

inline const char* hotReloadAssetTypeName(HotReloadAssetType t) {
    switch (t) {
        case HotReloadAssetType::Script:   return "Script";
        case HotReloadAssetType::Shader:   return "Shader";
        case HotReloadAssetType::Texture:  return "Texture";
        case HotReloadAssetType::Mesh:     return "Mesh";
        case HotReloadAssetType::Audio:    return "Audio";
        case HotReloadAssetType::Config:   return "Config";
        case HotReloadAssetType::Level:    return "Level";
        case HotReloadAssetType::Material: return "Material";
        default:                           return "Unknown";
    }
}

enum class HotReloadStatus : uint8_t {
    Idle      = 0,
    Pending   = 1,
    Reloading = 2,
    Success   = 3,
    Failed    = 4
};

struct HotReloadEntry {
    std::string       assetPath;
    HotReloadAssetType assetType  = HotReloadAssetType::Script;
    HotReloadStatus   status      = HotReloadStatus::Idle;
    size_t            reloadCount = 0;
    std::string       errorMessage;

    [[nodiscard]] bool isPending()   const { return status == HotReloadStatus::Pending; }
    [[nodiscard]] bool isReloading() const { return status == HotReloadStatus::Reloading; }
    [[nodiscard]] bool hasError()    const { return status == HotReloadStatus::Failed; }
    [[nodiscard]] bool isSuccess()   const { return status == HotReloadStatus::Success; }

    void markPending() { status = HotReloadStatus::Pending; errorMessage.clear(); }
    void markSuccess() { status = HotReloadStatus::Success; reloadCount++; errorMessage.clear(); }
    void markFailed(const std::string& err) { status = HotReloadStatus::Failed; errorMessage = err; }
};

class HotReloadWatcher {
public:
    static constexpr size_t kMaxEntries = 256;

    bool watch(const std::string& assetPath, HotReloadAssetType type) {
        if (m_entries.size() >= kMaxEntries) return false;
        for (const auto& e : m_entries) { if (e.assetPath == assetPath) return false; }
        HotReloadEntry entry;
        entry.assetPath = assetPath;
        entry.assetType = type;
        m_entries.push_back(entry);
        return true;
    }

    bool unwatch(const std::string& assetPath) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->assetPath == assetPath) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] HotReloadEntry* findEntry(const std::string& assetPath) {
        for (auto& e : m_entries) { if (e.assetPath == assetPath) return &e; }
        return nullptr;
    }

    bool triggerReload(const std::string& assetPath) {
        auto* e = findEntry(assetPath);
        if (!e) return false;
        e->markPending();
        return true;
    }

    [[nodiscard]] size_t entryCount()   const { return m_entries.size(); }
    [[nodiscard]] size_t pendingCount() const {
        size_t n = 0;
        for (const auto& e : m_entries) { if (e.isPending()) ++n; }
        return n;
    }

    [[nodiscard]] const std::vector<HotReloadEntry>& entries() const { return m_entries; }

private:
    std::vector<HotReloadEntry> m_entries;
};

class HotReloadDispatcher {
public:
    size_t dispatchPending(HotReloadWatcher& watcher) {
        size_t dispatched = 0;
        for (auto& e : const_cast<std::vector<HotReloadEntry>&>(watcher.entries())) {
            if (e.isPending()) {
                e.status = HotReloadStatus::Reloading;
                // Simulate successful reload
                e.markSuccess();
                m_totalDispatched++;
                dispatched++;
            }
        }
        return dispatched;
    }

    [[nodiscard]] size_t totalDispatched() const { return m_totalDispatched; }

private:
    size_t m_totalDispatched = 0;
};

class HotReloadSystem {
public:
    void init() { m_initialized = true; }

    void shutdown() {
        m_watcher    = HotReloadWatcher{};
        m_dispatcher = HotReloadDispatcher{};
        m_tickCount  = 0;
        m_initialized = false;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool watch(const std::string& assetPath, HotReloadAssetType type) {
        if (!m_initialized) return false;
        return m_watcher.watch(assetPath, type);
    }

    bool unwatch(const std::string& assetPath) {
        if (!m_initialized) return false;
        return m_watcher.unwatch(assetPath);
    }

    bool triggerReload(const std::string& assetPath) {
        if (!m_initialized) return false;
        return m_watcher.triggerReload(assetPath);
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_dispatcher.dispatchPending(m_watcher);
        m_tickCount++;
    }

    [[nodiscard]] HotReloadEntry* findEntry(const std::string& assetPath) {
        return m_watcher.findEntry(assetPath);
    }

    [[nodiscard]] size_t watchedCount()      const { return m_watcher.entryCount(); }
    [[nodiscard]] size_t pendingCount()      const { return m_watcher.pendingCount(); }
    [[nodiscard]] size_t totalDispatched()   const { return m_dispatcher.totalDispatched(); }
    [[nodiscard]] size_t tickCount()         const { return m_tickCount; }
    [[nodiscard]] HotReloadWatcher&          watcher()          { return m_watcher; }
    [[nodiscard]] const HotReloadWatcher&    watcher()    const { return m_watcher; }
    [[nodiscard]] HotReloadDispatcher&       dispatcher()       { return m_dispatcher; }

private:
    HotReloadWatcher    m_watcher;
    HotReloadDispatcher m_dispatcher;
    bool   m_initialized = false;
    size_t m_tickCount   = 0;
};

// ============================================================
// S17 — Asset Dependency Tracker
// ============================================================


} // namespace NF
