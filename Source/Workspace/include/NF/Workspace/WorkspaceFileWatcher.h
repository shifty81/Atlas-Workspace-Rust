#pragma once
// NF::Workspace — Phase 60: Workspace File Watcher
//
// File system monitoring with watch registrations, debouncing, event types,
// and observer callbacks for live asset/file reload.
//
//   FileEventType   — Created / Modified / Deleted / Renamed
//   FileEvent       — path + type + timestampMs; isValid()
//   WatchEntry      — id + path + recursive + enabled; isValid()
//   FileWatcher     — watch registry (MAX_WATCHES=64); add/remove/find watches;
//                     push events; poll/consume; debouncing; observer callbacks;
//                     serialize()/deserialize()

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// FileEventType
// ═════════════════════════════════════════════════════════════════

enum class FileEventType : uint8_t {
    Created  = 0,
    Modified = 1,
    Deleted  = 2,
    Renamed  = 3,
};

inline const char* fileEventTypeName(FileEventType t) {
    switch (t) {
        case FileEventType::Created:  return "Created";
        case FileEventType::Modified: return "Modified";
        case FileEventType::Deleted:  return "Deleted";
        case FileEventType::Renamed:  return "Renamed";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// FileEvent
// ═════════════════════════════════════════════════════════════════

struct FileEvent {
    std::string   path;
    FileEventType type        = FileEventType::Modified;
    uint64_t      timestampMs = 0;
    std::string   oldPath;        // for Renamed events

    [[nodiscard]] bool isValid() const { return !path.empty(); }

    bool operator==(const FileEvent& o) const {
        return path == o.path && type == o.type && timestampMs == o.timestampMs;
    }
    bool operator!=(const FileEvent& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════
// WatchEntry
// ═════════════════════════════════════════════════════════════════

struct WatchEntry {
    std::string id;
    std::string path;
    bool        recursive = false;
    bool        enabled   = true;
    std::string filter;            // glob filter (e.g. "*.cpp")

    [[nodiscard]] bool isValid() const { return !id.empty() && !path.empty(); }

    bool operator==(const WatchEntry& o) const { return id == o.id; }
    bool operator!=(const WatchEntry& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// FileWatcher
// ═════════════════════════════════════════════════════════════════

class FileWatcher {
public:
    using FileObserver = std::function<void(const FileEvent&)>;

    static constexpr int MAX_WATCHES   = 64;
    static constexpr int MAX_EVENTS    = 512;
    static constexpr int MAX_OBSERVERS = 16;

    // Watch management ─────────────────────────────────────────

    bool addWatch(const WatchEntry& watch) {
        if (!watch.isValid()) return false;
        if (findWatch(watch.id)) return false;
        if (static_cast<int>(m_watches.size()) >= MAX_WATCHES) return false;
        m_watches.push_back(watch);
        return true;
    }

    bool removeWatch(const std::string& watchId) {
        auto it = findWatchIt(watchId);
        if (it == m_watches.end()) return false;
        m_watches.erase(it);
        return true;
    }

    WatchEntry* findWatch(const std::string& watchId) {
        auto it = findWatchIt(watchId);
        return it != m_watches.end() ? &(*it) : nullptr;
    }

    const WatchEntry* findWatch(const std::string& watchId) const {
        auto it = std::find_if(m_watches.begin(), m_watches.end(),
                               [&](const WatchEntry& w) { return w.id == watchId; });
        return it != m_watches.end() ? &(*it) : nullptr;
    }

    bool hasWatch(const std::string& watchId) const { return findWatch(watchId) != nullptr; }

    bool enableWatch(const std::string& watchId, bool enabled) {
        auto* w = findWatch(watchId);
        if (!w) return false;
        w->enabled = enabled;
        return true;
    }

    [[nodiscard]] int watchCount() const { return static_cast<int>(m_watches.size()); }

    [[nodiscard]] const std::vector<WatchEntry>& watches() const { return m_watches; }

    // Events ───────────────────────────────────────────────────

    bool pushEvent(const FileEvent& event) {
        if (!event.isValid()) return false;
        // Debounce: skip if identical event already pending
        for (const auto& e : m_pendingEvents) {
            if (e.path == event.path && e.type == event.type) return false;
        }
        if (static_cast<int>(m_pendingEvents.size()) >= MAX_EVENTS) {
            m_pendingEvents.erase(m_pendingEvents.begin());
        }
        m_pendingEvents.push_back(event);
        return true;
    }

    [[nodiscard]] int pendingCount() const { return static_cast<int>(m_pendingEvents.size()); }

    [[nodiscard]] const std::vector<FileEvent>& pendingEvents() const { return m_pendingEvents; }

    std::vector<FileEvent> consumeEvents() {
        std::vector<FileEvent> events = std::move(m_pendingEvents);
        m_pendingEvents.clear();
        return events;
    }

    void processPending() {
        auto events = consumeEvents();
        for (const auto& ev : events) {
            notifyObservers(ev);
        }
    }

    void clearPending() { m_pendingEvents.clear(); }

    // Filter by type ───────────────────────────────────────────

    [[nodiscard]] std::vector<const FileEvent*> filterByType(FileEventType type) const {
        std::vector<const FileEvent*> results;
        for (const auto& e : m_pendingEvents) {
            if (e.type == type) results.push_back(&e);
        }
        return results;
    }

    [[nodiscard]] int countByType(FileEventType type) const {
        int n = 0;
        for (const auto& e : m_pendingEvents) {
            if (e.type == type) ++n;
        }
        return n;
    }

    // Observers ────────────────────────────────────────────────

    bool addObserver(FileObserver cb) {
        if (!cb) return false;
        if (static_cast<int>(m_observers.size()) >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // Serialization ────────────────────────────────────────────

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (const auto& w : m_watches) {
            out += esc(w.id) + "|"
                 + esc(w.path) + "|"
                 + (w.recursive ? "1" : "0") + "|"
                 + (w.enabled ? "1" : "0") + "|"
                 + esc(w.filter) + "\n";
        }
        return out;
    }

    bool deserialize(const std::string& text) {
        m_watches.clear();
        if (text.empty()) return true;

        size_t pos = 0;
        while (pos < text.size()) {
            size_t eol = text.find('\n', pos);
            if (eol == std::string::npos) eol = text.size();
            std::string line = text.substr(pos, eol - pos);
            pos = eol + 1;
            if (line.empty()) continue;

            auto fields = splitPipe(line);
            if (fields.size() < 5) continue;
            WatchEntry w;
            w.id        = unesc(fields[0]);
            w.path      = unesc(fields[1]);
            w.recursive = (fields[2] == "1");
            w.enabled   = (fields[3] == "1");
            w.filter    = unesc(fields[4]);
            m_watches.push_back(std::move(w));
        }
        return true;
    }

    void clear() {
        m_watches.clear();
        m_pendingEvents.clear();
        m_observers.clear();
    }

private:
    std::vector<WatchEntry>   m_watches;
    std::vector<FileEvent>    m_pendingEvents;
    std::vector<FileObserver> m_observers;

    std::vector<WatchEntry>::iterator findWatchIt(const std::string& id) {
        return std::find_if(m_watches.begin(), m_watches.end(),
                            [&](const WatchEntry& w) { return w.id == id; });
    }

    void notifyObservers(const FileEvent& ev) {
        for (auto& cb : m_observers) {
            if (cb) cb(ev);
        }
    }

    static std::string esc(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '|')       out += "\\P";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\N";
            else                out += c;
        }
        return out;
    }

    static std::string unesc(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                if (s[i + 1] == 'P')  { out += '|'; ++i; }
                else if (s[i + 1] == '\\') { out += '\\'; ++i; }
                else if (s[i + 1] == 'N')  { out += '\n'; ++i; }
                else out += s[i];
            } else {
                out += s[i];
            }
        }
        return out;
    }

    static size_t findPipe(const std::string& s, size_t start) {
        for (size_t i = start; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) { ++i; continue; }
            if (s[i] == '|') return i;
        }
        return std::string::npos;
    }

    static std::vector<std::string> splitPipe(const std::string& s) {
        std::vector<std::string> fields;
        size_t start = 0;
        while (start <= s.size()) {
            auto p = findPipe(s, start);
            if (p == std::string::npos) {
                fields.push_back(s.substr(start));
                break;
            }
            fields.push_back(s.substr(start, p - start));
            start = p + 1;
        }
        return fields;
    }
};

} // namespace NF
