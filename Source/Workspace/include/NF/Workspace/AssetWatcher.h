#pragma once
// NF::Workspace — AssetWatcher: logical file-change detection with debounce.
//
// The AssetWatcher tracks a set of registered source paths and provides a
// debounced change notification mechanism. It does NOT perform real filesystem
// I/O — filesystem monitoring is platform-specific and injected externally.
// Instead, the caller notifies the watcher via notifyChanged() whenever the OS
// reports a change, and the watcher applies a debounce window before delivering
// the change event to subscribers.
//
// This keeps the type testable without filesystem dependencies.
//
// Components:
//   ChangeType      — Created / Modified / Deleted / Renamed
//   ChangeEvent     — describes one change notification
//   WatchEntry      — a registered watch (path + enabled + event count)
//   AssetWatcher    — register/unregister paths; notifyChanged; tick; subscribe

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Change Type ────────────────────────────────────────────────────

enum class ChangeType : uint8_t {
    Created,
    Modified,
    Deleted,
    Renamed,
};

inline const char* changeTypeName(ChangeType t) {
    switch (t) {
    case ChangeType::Created:  return "Created";
    case ChangeType::Modified: return "Modified";
    case ChangeType::Deleted:  return "Deleted";
    case ChangeType::Renamed:  return "Renamed";
    }
    return "Unknown";
}

// ── Change Event ───────────────────────────────────────────────────

struct ChangeEvent {
    uint32_t    watchId   = 0;
    std::string path;           // path that changed
    std::string newPath;        // non-empty on Renamed
    ChangeType  type      = ChangeType::Modified;
    uint64_t    timestamp = 0;  // simulated or real time (ms)

    [[nodiscard]] bool isValid() const { return watchId != 0 && !path.empty(); }
};

// ── Watch Entry ────────────────────────────────────────────────────

struct WatchEntry {
    uint32_t    id          = 0;
    std::string path;
    bool        recursive   = false;
    bool        enabled     = true;
    uint32_t    eventCount  = 0;  // total events delivered for this path

    [[nodiscard]] bool isValid() const { return id != 0 && !path.empty(); }
};

// ── Change Callback ────────────────────────────────────────────────
using ChangeCallback = std::function<void(const ChangeEvent&)>;

// ── AssetWatcher ───────────────────────────────────────────────────

class AssetWatcher {
public:
    static constexpr size_t   MAX_WATCHES  = 1024;
    static constexpr size_t   MAX_CALLBACKS = 64;
    static constexpr uint64_t DEFAULT_DEBOUNCE_MS = 100; // 100ms debounce window

    // ── Watch management ──────────────────────────────────────────

    uint32_t addWatch(const std::string& path, bool recursive = false) {
        if (path.empty() || m_watches.size() >= MAX_WATCHES) return 0;
        for (const auto& w : m_watches) if (w.path == path) return w.id; // already watching
        WatchEntry e;
        e.id        = ++m_nextWatchId;
        e.path      = path;
        e.recursive = recursive;
        m_watches.push_back(e);
        return e.id;
    }

    bool removeWatch(uint32_t id) {
        for (auto it = m_watches.begin(); it != m_watches.end(); ++it) {
            if (it->id == id) { m_watches.erase(it); return true; }
        }
        return false;
    }

    bool removeWatchByPath(const std::string& path) {
        for (auto it = m_watches.begin(); it != m_watches.end(); ++it) {
            if (it->path == path) { m_watches.erase(it); return true; }
        }
        return false;
    }

    bool enableWatch(uint32_t id, bool enabled) {
        auto* w = findWatch(id);
        if (!w) return false;
        w->enabled = enabled;
        return true;
    }

    [[nodiscard]] size_t watchCount() const { return m_watches.size(); }
    [[nodiscard]] bool   isWatching(const std::string& path) const {
        for (const auto& w : m_watches) if (w.path == path) return true;
        return false;
    }
    [[nodiscard]] const WatchEntry* findWatch(uint32_t id) const {
        for (const auto& w : m_watches) if (w.id == id) return &w;
        return nullptr;
    }
    WatchEntry* findWatch(uint32_t id) {
        for (auto& w : m_watches) if (w.id == id) return &w;
        return nullptr;
    }

    // ── Change notification (called by platform layer) ────────────

    // Notify the watcher that a file at |path| changed.
    // If the path is not watched (or watch is disabled) the call is ignored.
    // The change is debounced: it only fires after tick() is called with
    // enough simulated/real time elapsed.
    bool notifyChanged(const std::string& path, ChangeType type,
                       uint64_t timestampMs, const std::string& newPath = {}) {
        WatchEntry* entry = nullptr;
        for (auto& w : m_watches) {
            if (!w.enabled) continue;
            // Exact match or prefix match (for recursive watches)
            bool match = (w.path == path)
                      || (w.recursive && path.size() > w.path.size()
                          && path.substr(0, w.path.size()) == w.path);
            if (match) { entry = &w; break; }
        }
        if (!entry) return false;

        // Check if already pending (dedup)
        for (auto& p : m_pending) {
            if (p.path == path && p.type == type) {
                p.timestamp = timestampMs; // reset debounce window
                return true;
            }
        }

        ChangeEvent ev;
        ev.watchId   = entry->id;
        ev.path      = path;
        ev.newPath   = newPath;
        ev.type      = type;
        ev.timestamp = timestampMs;
        m_pending.push_back(ev);
        return true;
    }

    // ── Tick (debounce pump) ──────────────────────────────────────

    // Call each frame with the current time (ms) to deliver debounced events.
    // Events older than (now - debounceMs) are delivered and removed from pending.
    size_t tick(uint64_t nowMs, uint64_t debounceMs = DEFAULT_DEBOUNCE_MS) {
        size_t delivered = 0;
        std::vector<ChangeEvent> survivors;
        for (auto& ev : m_pending) {
            if (nowMs >= ev.timestamp + debounceMs) {
                deliverEvent(ev);
                ++delivered;
                ++m_totalDelivered;
            } else {
                survivors.push_back(ev);
            }
        }
        m_pending = std::move(survivors);
        return delivered;
    }

    // ── Subscriber management ─────────────────────────────────────

    bool subscribe(ChangeCallback cb) {
        if (!cb) return false;
        if (m_callbacks.size() >= MAX_CALLBACKS) return false;
        m_callbacks.push_back(std::move(cb));
        return true;
    }

    void clearCallbacks() { m_callbacks.clear(); }

    // ── Statistics ────────────────────────────────────────────────

    [[nodiscard]] size_t pendingCount()    const { return m_pending.size();    }
    [[nodiscard]] size_t totalDelivered()  const { return m_totalDelivered;    }
    [[nodiscard]] size_t callbackCount()   const { return m_callbacks.size();  }

    void clearPending()                    { m_pending.clear(); }

    void setDebounceMs(uint64_t ms)        { m_defaultDebounce = ms; }
    [[nodiscard]] uint64_t debounceMs()    const { return m_defaultDebounce;   }

private:
    void deliverEvent(const ChangeEvent& ev) {
        // Update watch event count
        for (auto& w : m_watches) {
            if (w.id == ev.watchId) { ++w.eventCount; break; }
        }
        for (auto& cb : m_callbacks) {
            if (cb) cb(ev);
        }
    }

    std::vector<WatchEntry>   m_watches;
    std::vector<ChangeEvent>  m_pending;
    std::vector<ChangeCallback> m_callbacks;
    uint32_t m_nextWatchId    = 0;
    size_t   m_totalDelivered = 0;
    uint64_t m_defaultDebounce = DEFAULT_DEBOUNCE_MS;
};

} // namespace NF
