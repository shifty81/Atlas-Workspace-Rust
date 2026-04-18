#pragma once
// NF::Workspace — Phase 52: Workspace Window State
//
// Persistent window geometry and monitor-aware restore.
// UI-agnostic data model — no platform types.
//
//   WindowBounds        — x/y/w/h + isMaximized + isMinimized;
//                         isValid() (w>0, h>0)
//   MonitorInfo         — id + name + bounds (left/top/w/h) + scaleFactor +
//                         isPrimary; isValid()
//   WindowStateEntry    — windowId + bounds + monitorId + workspaceId +
//                         lastSavedMs; isValid()
//   WindowStateManager  — MonitorInfo registry; WindowStateEntry registry;
//                         save(entry)/restore(windowId)/remove(windowId);
//                         addMonitor/removeMonitor/findMonitor; primaryMonitor();
//                         isOnMonitor(entry, monitorId); observer on change
//                         (MAX_OBSERVERS=16); serialize/deserialize

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════════════
// WindowBounds
// ═════════════════════════════════════════════════════════════════════════

struct WindowBounds {
    int  x           = 0;
    int  y           = 0;
    int  width       = 0;
    int  height      = 0;
    bool isMaximized = false;
    bool isMinimized = false;

    [[nodiscard]] bool isValid() const { return width > 0 && height > 0; }

    [[nodiscard]] bool contains(int px, int py) const {
        return px >= x && py >= y && px < x + width && py < y + height;
    }

    bool operator==(const WindowBounds& o) const {
        return x == o.x && y == o.y && width == o.width && height == o.height
            && isMaximized == o.isMaximized && isMinimized == o.isMinimized;
    }
    bool operator!=(const WindowBounds& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════════════
// MonitorInfo
// ═════════════════════════════════════════════════════════════════════════

struct MonitorInfo {
    std::string  id;
    std::string  name;
    WindowBounds bounds;       // work area (not desktop coords)
    float        scaleFactor = 1.0f;
    bool         isPrimary   = false;

    [[nodiscard]] bool isValid() const { return !id.empty() && bounds.isValid(); }
};

// ═════════════════════════════════════════════════════════════════════════
// WindowStateEntry
// ═════════════════════════════════════════════════════════════════════════

struct WindowStateEntry {
    std::string  windowId;
    WindowBounds bounds;
    std::string  monitorId;     // which monitor it was on when saved
    std::string  workspaceId;   // optional — which workspace/project
    uint64_t     lastSavedMs = 0;

    [[nodiscard]] bool isValid() const { return !windowId.empty() && bounds.isValid(); }
};

// ═════════════════════════════════════════════════════════════════════════
// WindowStateManager
// ═════════════════════════════════════════════════════════════════════════

using WindowStateObserver = std::function<void()>;

class WindowStateManager {
public:
    static constexpr size_t MAX_MONITORS  = 8;
    static constexpr size_t MAX_ENTRIES   = 64;
    static constexpr size_t MAX_OBSERVERS = 16;

    // ── Monitor registry ──────────────────────────────────────────

    bool addMonitor(const MonitorInfo& info) {
        if (!info.isValid()) return false;
        if (m_monitors.size() >= MAX_MONITORS) return false;
        for (const auto& m : m_monitors)
            if (m.id == info.id) return false;
        // Only one primary allowed: if info.isPrimary, clear others
        if (info.isPrimary) {
            for (auto& m : m_monitors) m.isPrimary = false;
        }
        m_monitors.push_back(info);
        notify();
        return true;
    }

    bool removeMonitor(const std::string& id) {
        auto it = std::find_if(m_monitors.begin(), m_monitors.end(),
                               [&](const MonitorInfo& m){ return m.id == id; });
        if (it == m_monitors.end()) return false;
        m_monitors.erase(it);
        notify();
        return true;
    }

    [[nodiscard]] const MonitorInfo* findMonitor(const std::string& id) const {
        for (const auto& m : m_monitors)
            if (m.id == id) return &m;
        return nullptr;
    }

    [[nodiscard]] const MonitorInfo* primaryMonitor() const {
        for (const auto& m : m_monitors)
            if (m.isPrimary) return &m;
        return m_monitors.empty() ? nullptr : &m_monitors.front();
    }

    [[nodiscard]] size_t monitorCount()  const { return m_monitors.size(); }
    [[nodiscard]] const std::vector<MonitorInfo>& monitors() const { return m_monitors; }

    // ── Window state ──────────────────────────────────────────────

    // Save or update an entry. Returns false if invalid or at capacity (new).
    bool save(const WindowStateEntry& entry) {
        if (!entry.isValid()) return false;
        for (auto& e : m_entries) {
            if (e.windowId == entry.windowId) {
                e = entry;
                notify();
                return true;
            }
        }
        if (m_entries.size() >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        notify();
        return true;
    }

    [[nodiscard]] const WindowStateEntry* restore(const std::string& windowId) const {
        for (const auto& e : m_entries)
            if (e.windowId == windowId) return &e;
        return nullptr;
    }

    bool remove(const std::string& windowId) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
                               [&](const WindowStateEntry& e){
                                   return e.windowId == windowId;
                               });
        if (it == m_entries.end()) return false;
        m_entries.erase(it);
        notify();
        return true;
    }

    [[nodiscard]] bool has(const std::string& windowId) const {
        return restore(windowId) != nullptr;
    }

    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }
    [[nodiscard]] const std::vector<WindowStateEntry>& entries() const { return m_entries; }

    // Check if the entry's bounds overlap the given monitor's work area.
    [[nodiscard]] bool isOnMonitor(const WindowStateEntry& entry,
                                   const std::string& monitorId) const {
        const auto* mon = findMonitor(monitorId);
        if (!mon) return false;
        const auto& b = entry.bounds;
        const auto& mb = mon->bounds;
        // Center of window must be inside monitor bounds
        int cx = b.x + b.width  / 2;
        int cy = b.y + b.height / 2;
        return mb.contains(cx, cy);
    }

    // Find the monitor that best contains the given entry (by center point).
    [[nodiscard]] const MonitorInfo* monitorForEntry(const WindowStateEntry& entry) const {
        for (const auto& m : m_monitors)
            if (isOnMonitor(entry, m.id)) return &m;
        return primaryMonitor(); // fallback
    }

    // ── Serialization ─────────────────────────────────────────────
    // Window entries — format per entry (one line):
    //   <windowId>\t<x>\t<y>\t<w>\t<h>\t<maxim>\t<minim>\t<monitorId>\t<workspaceId>\t<ts>

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (const auto& e : m_entries) {
            out += e.windowId                           + "\t";
            out += std::to_string(e.bounds.x)           + "\t";
            out += std::to_string(e.bounds.y)           + "\t";
            out += std::to_string(e.bounds.width)        + "\t";
            out += std::to_string(e.bounds.height)       + "\t";
            out += std::string(e.bounds.isMaximized ? "1" : "0") + "\t";
            out += std::string(e.bounds.isMinimized ? "1" : "0") + "\t";
            out += e.monitorId                          + "\t";
            out += e.workspaceId                        + "\t";
            out += std::to_string(e.lastSavedMs)        + "\n";
        }
        return out;
    }

    // Clears existing entries and replaces with deserialized data.
    int deserialize(const std::string& text) {
        m_entries.clear();
        int loaded = 0;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t nl   = text.find('\n', pos);
            std::string line = text.substr(pos, nl == std::string::npos
                                              ? text.size() - pos : nl - pos);
            pos = (nl == std::string::npos) ? text.size() : nl + 1;
            if (line.empty()) continue;

            std::vector<std::string> f;
            size_t start = 0;
            for (int i = 0; i < 9; ++i) {
                size_t tab = line.find('\t', start);
                if (tab == std::string::npos) break;
                f.push_back(line.substr(start, tab - start));
                start = tab + 1;
            }
            f.push_back(line.substr(start));
            if (f.size() != 10) continue;

            WindowStateEntry entry;
            entry.windowId           = f[0];
            entry.bounds.x           = std::stoi(f[1]);
            entry.bounds.y           = std::stoi(f[2]);
            entry.bounds.width       = std::stoi(f[3]);
            entry.bounds.height      = std::stoi(f[4]);
            entry.bounds.isMaximized = f[5] == "1";
            entry.bounds.isMinimized = f[6] == "1";
            entry.monitorId          = f[7];
            entry.workspaceId        = f[8];
            entry.lastSavedMs        = static_cast<uint64_t>(std::stoull(f[9]));

            if (!entry.isValid()) continue;
            if (m_entries.size() < MAX_ENTRIES) {
                m_entries.push_back(entry);
                ++loaded;
            }
        }
        return loaded;
    }

    // ── Observers ─────────────────────────────────────────────────

    bool addObserver(WindowStateObserver obs) {
        if (m_observers.size() >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(obs));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    void clear() {
        m_monitors.clear();
        m_entries.clear();
        notify();
    }

private:
    void notify() {
        for (auto& obs : m_observers) obs();
    }

    std::vector<MonitorInfo>        m_monitors;
    std::vector<WindowStateEntry>   m_entries;
    std::vector<WindowStateObserver> m_observers;
};

} // namespace NF
