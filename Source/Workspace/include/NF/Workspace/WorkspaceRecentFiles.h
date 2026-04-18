#pragma once
// NF::Workspace — Phase 49: Workspace Recent Files
//
// Unified recent-files / recent-projects manager.
//
//   RecentFileKind    — Project / Scene / Asset / Script / Config / Custom
//   RecentFileEntry   — path + displayName + kind + lastOpenedMs + pinned + accessCount
//   RecentFileList    — MRU ring per kind (MAX_ENTRIES=64); front-dedup by path;
//                       pinned items survive cap eviction; query: all/byKind/pinned/
//                       unpinned/findByPath/mostRecent; pin/unpin/remove/clear/
//                       clearUnpinned; compact (remove non-existing stubs)
//   RecentFilesManager— multi-kind registry (one list per kind); record/remove/pin/unpin;
//                       globalRecent (across all kinds, sorted by lastOpenedMs desc,
//                       capped at MAX_GLOBAL=32); observer callbacks on record/remove
//                       (MAX_OBSERVERS=16); save/load through WorkspaceProjectFile
//                       "RecentFiles" section (text-format: pipe-delimited records)

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════════════
// RecentFileKind
// ═════════════════════════════════════════════════════════════════════════

enum class RecentFileKind : uint8_t {
    Project,
    Scene,
    Asset,
    Script,
    Config,
    Custom,
};

inline const char* recentFileKindName(RecentFileKind k) {
    switch (k) {
        case RecentFileKind::Project: return "Project";
        case RecentFileKind::Scene:   return "Scene";
        case RecentFileKind::Asset:   return "Asset";
        case RecentFileKind::Script:  return "Script";
        case RecentFileKind::Config:  return "Config";
        case RecentFileKind::Custom:  return "Custom";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════════════
// RecentFileEntry
// ═════════════════════════════════════════════════════════════════════════

struct RecentFileEntry {
    std::string    path;
    std::string    displayName;
    RecentFileKind kind          = RecentFileKind::Custom;
    uint64_t       lastOpenedMs  = 0;
    bool           pinned        = false;
    uint32_t       accessCount   = 0;

    [[nodiscard]] bool isValid() const { return !path.empty(); }

    bool operator==(const RecentFileEntry& o) const { return path == o.path; }
    bool operator!=(const RecentFileEntry& o) const { return path != o.path; }
};

// ═════════════════════════════════════════════════════════════════════════
// RecentFileList — MRU ring for one kind
// ═════════════════════════════════════════════════════════════════════════

class RecentFileList {
public:
    static constexpr size_t MAX_ENTRIES = 64;

    // Record an access to path. Bumps to front; creates if new.
    // Pinned entries are never evicted; unpinned oldest entry is dropped if at cap.
    // Returns true if the entry was added or updated.
    bool record(const std::string& path, const std::string& displayName,
                RecentFileKind kind, uint64_t timestampMs) {
        if (path.empty()) return false;

        // Check if already exists
        auto it = findIt(path);
        if (it != m_entries.end()) {
            it->lastOpenedMs = timestampMs;
            it->displayName  = displayName.empty() ? it->displayName : displayName;
            it->accessCount++;
            // Move to front
            RecentFileEntry entry = std::move(*it);
            m_entries.erase(it);
            m_entries.insert(m_entries.begin(), std::move(entry));
            return true;
        }

        // New entry — evict if at capacity
        if (m_entries.size() >= MAX_ENTRIES) {
            // Evict oldest unpinned from the back
            for (auto rit = m_entries.rbegin(); rit != m_entries.rend(); ++rit) {
                if (!rit->pinned) {
                    m_entries.erase((rit + 1).base());
                    break;
                }
            }
            // If still at capacity (all pinned), reject the new entry
            if (m_entries.size() >= MAX_ENTRIES) return false;
        }

        RecentFileEntry entry;
        entry.path         = path;
        entry.displayName  = displayName.empty() ? path : displayName;
        entry.kind         = kind;
        entry.lastOpenedMs = timestampMs;
        entry.accessCount  = 1;
        m_entries.insert(m_entries.begin(), std::move(entry));
        return true;
    }

    // Remove by path. Returns false if not found.
    bool remove(const std::string& path) {
        auto it = findIt(path);
        if (it == m_entries.end()) return false;
        m_entries.erase(it);
        return true;
    }

    // Pin / unpin. Returns false if not found.
    bool pin(const std::string& path, bool pinned) {
        auto it = findIt(path);
        if (it == m_entries.end()) return false;
        it->pinned = pinned;
        return true;
    }

    [[nodiscard]] const RecentFileEntry* findByPath(const std::string& path) const {
        for (const auto& e : m_entries)
            if (e.path == path) return &e;
        return nullptr;
    }

    [[nodiscard]] bool contains(const std::string& path) const {
        return findByPath(path) != nullptr;
    }

    // Most recent (front of list), nullptr if empty.
    [[nodiscard]] const RecentFileEntry* mostRecent() const {
        return m_entries.empty() ? nullptr : &m_entries.front();
    }

    [[nodiscard]] const std::vector<RecentFileEntry>& all() const { return m_entries; }

    [[nodiscard]] std::vector<const RecentFileEntry*> pinned() const {
        std::vector<const RecentFileEntry*> out;
        for (const auto& e : m_entries)
            if (e.pinned) out.push_back(&e);
        return out;
    }

    [[nodiscard]] std::vector<const RecentFileEntry*> unpinned() const {
        std::vector<const RecentFileEntry*> out;
        for (const auto& e : m_entries)
            if (!e.pinned) out.push_back(&e);
        return out;
    }

    [[nodiscard]] size_t count()       const { return m_entries.size(); }
    [[nodiscard]] bool   empty()       const { return m_entries.empty(); }
    [[nodiscard]] size_t pinnedCount() const {
        size_t n = 0;
        for (const auto& e : m_entries) if (e.pinned) ++n;
        return n;
    }

    // Remove all unpinned entries.
    void clearUnpinned() {
        m_entries.erase(
            std::remove_if(m_entries.begin(), m_entries.end(),
                           [](const RecentFileEntry& e){ return !e.pinned; }),
            m_entries.end());
    }

    // Remove all entries (including pinned).
    void clear() { m_entries.clear(); }

    // Internal: append without MRU reordering (used by deserialization)
    void appendDirect(const RecentFileEntry& entry) {
        if (m_entries.size() < MAX_ENTRIES)
            m_entries.push_back(entry);
    }

private:
    std::vector<RecentFileEntry>::iterator findIt(const std::string& path) {
        return std::find_if(m_entries.begin(), m_entries.end(),
                            [&](const RecentFileEntry& e){ return e.path == path; });
    }

    std::vector<RecentFileEntry> m_entries;
};

// ═════════════════════════════════════════════════════════════════════════
// RecentFilesManager
// ═════════════════════════════════════════════════════════════════════════

using RecentFilesObserver = std::function<void(const RecentFileEntry& /*entry*/,
                                               bool                  /*recorded – true, removed – false*/)>;

class RecentFilesManager {
public:
    static constexpr size_t MAX_GLOBAL    = 32;
    static constexpr size_t MAX_OBSERVERS = 16;

    // Record a file access. timestampMs=0 is allowed (treated as "unknown").
    // Returns true on success.
    bool record(const std::string& path,
                const std::string& displayName,
                RecentFileKind kind,
                uint64_t timestampMs = 0) {
        auto& list = listFor(kind);
        bool ok = list.record(path, displayName, kind, timestampMs);
        if (ok) {
            const auto* entry = list.findByPath(path);
            if (entry) fireObservers(*entry, true);
        }
        return ok;
    }

    // Remove a file from its kind list. Returns false if not found.
    bool remove(const std::string& path, RecentFileKind kind) {
        auto& list = listFor(kind);
        const auto* entry = list.findByPath(path);
        if (!entry) return false;
        RecentFileEntry copy = *entry;
        bool ok = list.remove(path);
        if (ok) fireObservers(copy, false);
        return ok;
    }

    // Pin / unpin. Returns false if not found.
    bool pin(const std::string& path, RecentFileKind kind, bool pinned) {
        return listFor(kind).pin(path, pinned);
    }

    // Look up entry (read-only).
    [[nodiscard]] const RecentFileEntry* find(const std::string& path,
                                              RecentFileKind kind) const {
        return listFor(kind).findByPath(path);
    }

    // Per-kind list access (read-only).
    [[nodiscard]] const RecentFileList& listForKind(RecentFileKind kind) const {
        return listFor(kind);
    }

    // Global recent: all entries across all kinds, sorted by lastOpenedMs descending,
    // capped at MAX_GLOBAL.
    [[nodiscard]] std::vector<RecentFileEntry> globalRecent() const {
        std::vector<RecentFileEntry> all;
        for (const auto& list : m_lists)
            for (const auto& e : list.all())
                all.push_back(e);

        std::stable_sort(all.begin(), all.end(),
                         [](const RecentFileEntry& a, const RecentFileEntry& b){
                             return a.lastOpenedMs > b.lastOpenedMs;
                         });
        if (all.size() > MAX_GLOBAL)
            all.resize(MAX_GLOBAL);
        return all;
    }

    // Clear all entries for a specific kind.
    void clearKind(RecentFileKind kind) { listFor(kind).clear(); }

    // Clear all entries across all kinds.
    void clearAll() {
        for (auto& list : m_lists) list.clear();
    }

    // Clear only unpinned entries across all kinds.
    void clearAllUnpinned() {
        for (auto& list : m_lists) list.clearUnpinned();
    }

    // ── Observers ─────────────────────────────────────────────────

    bool addObserver(RecentFilesObserver obs) {
        if (m_observers.size() >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(obs));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // ── Serialization to/from flat text ───────────────────────────
    // Wire format per entry (one line):
    //   <kind_idx>|<lastOpenedMs>|<accessCount>|<pinned>|<path>|<displayName>
    // Pipe in path or displayName is escaped as \P.

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (size_t ki = 0; ki < kKindCount; ++ki) {
            for (const auto& e : m_lists[ki].all()) {
                out += std::to_string(ki)                    + "|";
                out += std::to_string(e.lastOpenedMs)        + "|";
                out += std::to_string(e.accessCount)         + "|";
                out += std::string(e.pinned ? "1" : "0")             + "|";
                out += escapePipe(e.path)                    + "|";
                out += escapePipe(e.displayName)             + "\n";
            }
        }
        return out;
    }

    // Deserialize from serialized text, replacing all current data.
    // Returns count of entries successfully parsed.
    int deserialize(const std::string& text) {
        clearAll();
        int loaded = 0;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t nl = text.find('\n', pos);
            std::string line = text.substr(pos, nl == std::string::npos ? text.size() - pos
                                                                        : nl - pos);
            pos = (nl == std::string::npos) ? text.size() : nl + 1;
            if (line.empty()) continue;

            // Split on '|', 6 fields
            std::vector<std::string> fields;
            size_t start = 0;
            for (int i = 0; i < 5; ++i) {
                size_t pipe = line.find('|', start);
                if (pipe == std::string::npos) break;
                fields.push_back(line.substr(start, pipe - start));
                start = pipe + 1;
            }
            fields.push_back(line.substr(start)); // last field

            if (fields.size() != 6) continue;

            size_t ki = static_cast<size_t>(std::stoul(fields[0]));
            if (ki >= kKindCount) continue;
            auto kind = static_cast<RecentFileKind>(ki);

            uint64_t ts     = static_cast<uint64_t>(std::stoull(fields[1]));
            uint32_t ac     = static_cast<uint32_t>(std::stoul(fields[2]));
            bool     pinned = fields[3] == "1";
            std::string path = unescapePipe(fields[4]);
            std::string name = unescapePipe(fields[5]);

            if (path.empty()) continue;

            // Insert directly to preserve order (already MRU-sorted in file)
            RecentFileEntry entry;
            entry.path        = path;
            entry.displayName = name;
            entry.kind        = kind;
            entry.lastOpenedMs= ts;
            entry.accessCount = ac;
            entry.pinned      = pinned;
            internalInsertBack(ki, entry);
            ++loaded;
        }
        return loaded;
    }

private:
    static constexpr size_t kKindCount = 6; // must match RecentFileKind enum count

    static std::string escapePipe(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c == '|') { out += "\\P"; }
            else           { out += c;    }
        }
        return out;
    }

    static std::string unescapePipe(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size() && s[i+1] == 'P') {
                out += '|'; ++i;
            } else {
                out += s[i];
            }
        }
        return out;
    }

    void internalInsertBack(size_t ki, const RecentFileEntry& entry) {
        // Append to back without MRU reordering — used during deserialization
        // where the file is already in MRU order (front = newest).
        // We rebuild the vector by appending, then the natural iteration order is preserved.
        m_lists[ki].appendDirect(entry);
    }

    void fireObservers(const RecentFileEntry& entry, bool recorded) {
        for (auto& obs : m_observers)
            obs(entry, recorded);
    }

    RecentFileList& listFor(RecentFileKind kind) {
        return m_lists[static_cast<size_t>(kind)];
    }
    const RecentFileList& listFor(RecentFileKind kind) const {
        return m_lists[static_cast<size_t>(kind)];
    }

    RecentFileList                  m_lists[kKindCount];
    std::vector<RecentFilesObserver> m_observers;
};

} // namespace NF
