#pragma once
// NF::Workspace — Phase 61: Workspace Changelog
//
// Versioned change records with categories, severity, entry management,
// filtering, search, and serialization.
//
//   ChangeCategory  — Feature / Bugfix / Breaking / Deprecated / Internal / Security
//   ChangeSeverity  — Minor / Major / Critical / Patch
//   ChangeEntry     — id + version + summary + detail + category + severity + timestampMs; isValid()
//   ChangeVersion   — semantic version string + entries (MAX_ENTRIES=256)
//   Changelog       — version registry (MAX_VERSIONS=128); add/find/remove versions;
//                     addEntry shortcut; searchByText (case-insensitive); filterByCategory/Severity;
//                     observer callbacks; serialize()/deserialize()

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// ChangeCategory / ChangeSeverity
// ═════════════════════════════════════════════════════════════════

enum class ChangeCategory : uint8_t {
    Feature    = 0,
    Bugfix     = 1,
    Breaking   = 2,
    Deprecated = 3,
    Internal   = 4,
    Security   = 5,
};

inline const char* changeCategoryName(ChangeCategory c) {
    switch (c) {
        case ChangeCategory::Feature:    return "Feature";
        case ChangeCategory::Bugfix:     return "Bugfix";
        case ChangeCategory::Breaking:   return "Breaking";
        case ChangeCategory::Deprecated: return "Deprecated";
        case ChangeCategory::Internal:   return "Internal";
        case ChangeCategory::Security:   return "Security";
        default:                         return "Unknown";
    }
}

enum class ChangeSeverity : uint8_t {
    Patch    = 0,
    Minor    = 1,
    Major    = 2,
    Critical = 3,
};

inline const char* changeSeverityName(ChangeSeverity s) {
    switch (s) {
        case ChangeSeverity::Patch:    return "Patch";
        case ChangeSeverity::Minor:    return "Minor";
        case ChangeSeverity::Major:    return "Major";
        case ChangeSeverity::Critical: return "Critical";
        default:                       return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// ChangeEntry
// ═════════════════════════════════════════════════════════════════

struct ChangeEntry {
    std::string    id;
    std::string    version;       // e.g. "1.2.0"
    std::string    summary;
    std::string    detail;
    ChangeCategory category   = ChangeCategory::Internal;
    ChangeSeverity severity   = ChangeSeverity::Patch;
    uint64_t       timestampMs = 0;

    [[nodiscard]] bool isValid() const { return !id.empty() && !summary.empty(); }

    bool operator==(const ChangeEntry& o) const { return id == o.id; }
    bool operator!=(const ChangeEntry& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// ChangeVersion
// ═════════════════════════════════════════════════════════════════

class ChangeVersion {
public:
    static constexpr int MAX_ENTRIES = 256;

    std::string version;   // e.g. "1.2.0"
    std::string releaseDate;
    std::string notes;

    [[nodiscard]] bool isValid() const { return !version.empty(); }

    bool addEntry(const ChangeEntry& entry) {
        if (!entry.isValid()) return false;
        if (findEntry(entry.id)) return false;
        if (static_cast<int>(m_entries.size()) >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        return true;
    }

    bool removeEntry(const std::string& entryId) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
                               [&](const ChangeEntry& e) { return e.id == entryId; });
        if (it == m_entries.end()) return false;
        m_entries.erase(it);
        return true;
    }

    const ChangeEntry* findEntry(const std::string& entryId) const {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
                               [&](const ChangeEntry& e) { return e.id == entryId; });
        return it != m_entries.end() ? &(*it) : nullptr;
    }

    [[nodiscard]] std::vector<const ChangeEntry*> filterByCategory(ChangeCategory cat) const {
        std::vector<const ChangeEntry*> result;
        for (const auto& e : m_entries) {
            if (e.category == cat) result.push_back(&e);
        }
        return result;
    }

    [[nodiscard]] std::vector<const ChangeEntry*> filterBySeverity(ChangeSeverity sev) const {
        std::vector<const ChangeEntry*> result;
        for (const auto& e : m_entries) {
            if (e.severity == sev) result.push_back(&e);
        }
        return result;
    }

    [[nodiscard]] int countByCategory(ChangeCategory cat) const {
        int n = 0;
        for (const auto& e : m_entries) if (e.category == cat) ++n;
        return n;
    }

    [[nodiscard]] int                              count()   const { return static_cast<int>(m_entries.size()); }
    [[nodiscard]] bool                             empty()   const { return m_entries.empty(); }
    [[nodiscard]] const std::vector<ChangeEntry>&  entries() const { return m_entries; }

    void clear() { m_entries.clear(); }

private:
    std::vector<ChangeEntry> m_entries;
};

// ═════════════════════════════════════════════════════════════════
// Changelog
// ═════════════════════════════════════════════════════════════════

class Changelog {
public:
    using ChangeObserver = std::function<void(const ChangeEntry&)>;

    static constexpr int MAX_VERSIONS  = 128;
    static constexpr int MAX_OBSERVERS = 16;

    // Version management ───────────────────────────────────────

    bool addVersion(const ChangeVersion& ver) {
        if (!ver.isValid()) return false;
        if (findVersion(ver.version)) return false;
        if (static_cast<int>(m_versions.size()) >= MAX_VERSIONS) return false;
        m_versions.push_back(ver);
        return true;
    }

    bool removeVersion(const std::string& version) {
        auto it = findVersionIt(version);
        if (it == m_versions.end()) return false;
        m_versions.erase(it);
        return true;
    }

    ChangeVersion* findVersion(const std::string& version) {
        auto it = findVersionIt(version);
        return it != m_versions.end() ? &(*it) : nullptr;
    }

    const ChangeVersion* findVersion(const std::string& version) const {
        auto it = std::find_if(m_versions.begin(), m_versions.end(),
                               [&](const ChangeVersion& v) { return v.version == version; });
        return it != m_versions.end() ? &(*it) : nullptr;
    }

    bool hasVersion(const std::string& version) const { return findVersion(version) != nullptr; }

    [[nodiscard]] int versionCount() const { return static_cast<int>(m_versions.size()); }

    [[nodiscard]] const std::vector<ChangeVersion>& versions() const { return m_versions; }

    // Entry shortcut ───────────────────────────────────────────

    bool addEntry(const std::string& version, const ChangeEntry& entry) {
        auto* ver = findVersion(version);
        if (!ver) return false;
        if (!ver->addEntry(entry)) return false;
        notifyObservers(entry);
        return true;
    }

    // Search & filter ──────────────────────────────────────────

    [[nodiscard]] std::vector<const ChangeEntry*> searchByText(const std::string& query) const {
        std::vector<const ChangeEntry*> results;
        if (query.empty()) return results;
        std::string lq = toLower(query);
        for (const auto& ver : m_versions) {
            for (const auto& entry : ver.entries()) {
                if (toLower(entry.summary).find(lq) != std::string::npos
                    || toLower(entry.detail).find(lq) != std::string::npos) {
                    results.push_back(&entry);
                }
            }
        }
        return results;
    }

    [[nodiscard]] std::vector<const ChangeEntry*> filterByCategory(ChangeCategory cat) const {
        std::vector<const ChangeEntry*> results;
        for (const auto& ver : m_versions) {
            for (const auto& entry : ver.entries()) {
                if (entry.category == cat) results.push_back(&entry);
            }
        }
        return results;
    }

    [[nodiscard]] std::vector<const ChangeEntry*> filterBySeverity(ChangeSeverity sev) const {
        std::vector<const ChangeEntry*> results;
        for (const auto& ver : m_versions) {
            for (const auto& entry : ver.entries()) {
                if (entry.severity == sev) results.push_back(&entry);
            }
        }
        return results;
    }

    [[nodiscard]] int totalEntries() const {
        int total = 0;
        for (const auto& ver : m_versions) total += ver.count();
        return total;
    }

    // Observers ────────────────────────────────────────────────

    bool addObserver(ChangeObserver cb) {
        if (!cb) return false;
        if (static_cast<int>(m_observers.size()) >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // Serialization ────────────────────────────────────────────

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (const auto& ver : m_versions) {
            out += "[" + esc(ver.version) + "|" + esc(ver.releaseDate) + "|" + esc(ver.notes) + "]\n";
            for (const auto& entry : ver.entries()) {
                out += esc(entry.id) + "|"
                     + esc(entry.version) + "|"
                     + esc(entry.summary) + "|"
                     + std::to_string(static_cast<int>(entry.category)) + "|"
                     + std::to_string(static_cast<int>(entry.severity)) + "|"
                     + std::to_string(entry.timestampMs) + "|"
                     + esc(entry.detail) + "\n";
            }
        }
        return out;
    }

    bool deserialize(const std::string& text) {
        m_versions.clear();
        if (text.empty()) return true;

        ChangeVersion* current = nullptr;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t eol = text.find('\n', pos);
            if (eol == std::string::npos) eol = text.size();
            std::string line = text.substr(pos, eol - pos);
            pos = eol + 1;
            if (line.empty()) continue;

            if (line.front() == '[' && line.back() == ']') {
                std::string inner = line.substr(1, line.size() - 2);
                auto fields = splitPipe(inner);
                if (fields.empty()) continue;
                ChangeVersion ver;
                ver.version     = unesc(fields[0]);
                ver.releaseDate = fields.size() > 1 ? unesc(fields[1]) : "";
                ver.notes       = fields.size() > 2 ? unesc(fields[2]) : "";
                m_versions.push_back(std::move(ver));
                current = &m_versions.back();
            } else if (current) {
                auto fields = splitPipe(line);
                if (fields.size() < 7) continue;
                ChangeEntry entry;
                entry.id          = unesc(fields[0]);
                entry.version     = unesc(fields[1]);
                entry.summary     = unesc(fields[2]);
                entry.category    = static_cast<ChangeCategory>(std::stoi(fields[3]));
                entry.severity    = static_cast<ChangeSeverity>(std::stoi(fields[4]));
                entry.timestampMs = std::stoull(fields[5]);
                entry.detail      = unesc(fields[6]);
                current->addEntry(entry);
            }
        }
        return true;
    }

    void clear() {
        m_versions.clear();
        m_observers.clear();
    }

private:
    std::vector<ChangeVersion>  m_versions;
    std::vector<ChangeObserver> m_observers;

    std::vector<ChangeVersion>::iterator findVersionIt(const std::string& version) {
        return std::find_if(m_versions.begin(), m_versions.end(),
                            [&](const ChangeVersion& v) { return v.version == version; });
    }

    void notifyObservers(const ChangeEntry& entry) {
        for (auto& cb : m_observers) if (cb) cb(entry);
    }

    static std::string esc(const std::string& s) {
        std::string out; out.reserve(s.size());
        for (char c : s) {
            if (c == '|')       out += "\\P";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\N";
            else                out += c;
        }
        return out;
    }

    static std::string unesc(const std::string& s) {
        std::string out; out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                if      (s[i+1] == 'P')  { out += '|';  ++i; }
                else if (s[i+1] == '\\') { out += '\\'; ++i; }
                else if (s[i+1] == 'N')  { out += '\n'; ++i; }
                else out += s[i];
            } else { out += s[i]; }
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
            if (p == std::string::npos) { fields.push_back(s.substr(start)); break; }
            fields.push_back(s.substr(start, p - start));
            start = p + 1;
        }
        return fields;
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        for (auto& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    }
};

} // namespace NF
