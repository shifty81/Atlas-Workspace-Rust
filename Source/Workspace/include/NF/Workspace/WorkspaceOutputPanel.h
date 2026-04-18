#pragma once
// NF::Workspace — Phase 58: Workspace Output Panel
//
// Structured output panel for build output, command results, and tool
// messages with filtering, channel management, and severity levels.
//
//   OutputSeverity  — Info / Warning / Error / Debug / Trace
//   OutputEntry     — id + channel + text + severity + timestampMs; isValid()
//   OutputChannel   — named channel with entries (MAX_ENTRIES=1024); add/clear/filter
//   OutputPanel     — channel registry (MAX_CHANNELS=32); add/remove/find channels;
//                     write shortcut; search; observer callbacks; serialize()/deserialize()

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// OutputSeverity
// ═════════════════════════════════════════════════════════════════

enum class OutputSeverity : uint8_t {
    Info    = 0,
    Warning = 1,
    Error   = 2,
    Debug   = 3,
    Trace   = 4,
};

inline const char* outputSeverityName(OutputSeverity s) {
    switch (s) {
        case OutputSeverity::Info:    return "Info";
        case OutputSeverity::Warning: return "Warning";
        case OutputSeverity::Error:   return "Error";
        case OutputSeverity::Debug:   return "Debug";
        case OutputSeverity::Trace:   return "Trace";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// OutputEntry
// ═════════════════════════════════════════════════════════════════

struct OutputEntry {
    std::string    id;
    std::string    channel;
    std::string    text;
    OutputSeverity severity    = OutputSeverity::Info;
    uint64_t       timestampMs = 0;

    [[nodiscard]] bool isValid() const { return !id.empty() && !text.empty(); }

    bool operator==(const OutputEntry& o) const { return id == o.id; }
    bool operator!=(const OutputEntry& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// OutputChannel
// ═════════════════════════════════════════════════════════════════

class OutputChannel {
public:
    static constexpr int MAX_ENTRIES = 1024;

    std::string id;
    std::string name;

    [[nodiscard]] bool isValid() const { return !id.empty() && !name.empty(); }

    bool addEntry(const OutputEntry& entry) {
        if (!entry.isValid()) return false;
        if (static_cast<int>(m_entries.size()) >= MAX_ENTRIES) {
            m_entries.erase(m_entries.begin());
        }
        m_entries.push_back(entry);
        return true;
    }

    void clear() { m_entries.clear(); }

    [[nodiscard]] int                             count()   const { return static_cast<int>(m_entries.size()); }
    [[nodiscard]] bool                            empty()   const { return m_entries.empty(); }
    [[nodiscard]] const std::vector<OutputEntry>& entries() const { return m_entries; }

    [[nodiscard]] std::vector<const OutputEntry*> filterBySeverity(OutputSeverity sev) const {
        std::vector<const OutputEntry*> results;
        for (const auto& e : m_entries) {
            if (e.severity == sev) results.push_back(&e);
        }
        return results;
    }

    [[nodiscard]] int countBySeverity(OutputSeverity sev) const {
        int n = 0;
        for (const auto& e : m_entries) {
            if (e.severity == sev) ++n;
        }
        return n;
    }

    [[nodiscard]] const OutputEntry* lastEntry() const {
        return m_entries.empty() ? nullptr : &m_entries.back();
    }

private:
    std::vector<OutputEntry> m_entries;
};

// ═════════════════════════════════════════════════════════════════
// OutputPanel
// ═════════════════════════════════════════════════════════════════

class OutputPanel {
public:
    using OutputObserver = std::function<void(const OutputEntry&)>;

    static constexpr int MAX_CHANNELS  = 32;
    static constexpr int MAX_OBSERVERS = 16;

    // Channel management ───────────────────────────────────────

    bool addChannel(const OutputChannel& ch) {
        if (!ch.isValid()) return false;
        if (findChannel(ch.id)) return false;
        if (static_cast<int>(m_channels.size()) >= MAX_CHANNELS) return false;
        m_channels.push_back(ch);
        return true;
    }

    bool removeChannel(const std::string& channelId) {
        auto it = findChannelIt(channelId);
        if (it == m_channels.end()) return false;
        m_channels.erase(it);
        return true;
    }

    OutputChannel* findChannel(const std::string& channelId) {
        auto it = findChannelIt(channelId);
        return it != m_channels.end() ? &(*it) : nullptr;
    }

    const OutputChannel* findChannel(const std::string& channelId) const {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
                               [&](const OutputChannel& c) { return c.id == channelId; });
        return it != m_channels.end() ? &(*it) : nullptr;
    }

    bool hasChannel(const std::string& channelId) const { return findChannel(channelId) != nullptr; }

    [[nodiscard]] int channelCount() const { return static_cast<int>(m_channels.size()); }

    [[nodiscard]] const std::vector<OutputChannel>& channels() const { return m_channels; }

    // Write shortcut ───────────────────────────────────────────

    bool write(const std::string& channelId, const OutputEntry& entry) {
        auto* ch = findChannel(channelId);
        if (!ch) return false;
        if (!ch->addEntry(entry)) return false;
        notifyObservers(entry);
        return true;
    }

    // Search ───────────────────────────────────────────────────

    [[nodiscard]] std::vector<const OutputEntry*> searchByText(const std::string& query) const {
        std::vector<const OutputEntry*> results;
        if (query.empty()) return results;
        std::string lq = toLower(query);
        for (const auto& ch : m_channels) {
            for (const auto& entry : ch.entries()) {
                if (toLower(entry.text).find(lq) != std::string::npos) {
                    results.push_back(&entry);
                }
            }
        }
        return results;
    }

    [[nodiscard]] std::vector<const OutputEntry*> searchBySeverity(OutputSeverity sev) const {
        std::vector<const OutputEntry*> results;
        for (const auto& ch : m_channels) {
            for (const auto& entry : ch.entries()) {
                if (entry.severity == sev) {
                    results.push_back(&entry);
                }
            }
        }
        return results;
    }

    [[nodiscard]] int totalEntries() const {
        int total = 0;
        for (const auto& ch : m_channels) total += ch.count();
        return total;
    }

    // Observers ────────────────────────────────────────────────

    bool addObserver(OutputObserver cb) {
        if (!cb) return false;
        if (static_cast<int>(m_observers.size()) >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // Serialization ────────────────────────────────────────────

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (const auto& ch : m_channels) {
            out += "[" + esc(ch.id) + "|" + esc(ch.name) + "]\n";
            for (const auto& entry : ch.entries()) {
                out += esc(entry.id) + "|"
                     + esc(entry.channel) + "|"
                     + std::to_string(static_cast<int>(entry.severity)) + "|"
                     + std::to_string(entry.timestampMs) + "|"
                     + esc(entry.text) + "\n";
            }
        }
        return out;
    }

    bool deserialize(const std::string& text) {
        m_channels.clear();
        if (text.empty()) return true;

        OutputChannel* current = nullptr;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t eol = text.find('\n', pos);
            if (eol == std::string::npos) eol = text.size();
            std::string line = text.substr(pos, eol - pos);
            pos = eol + 1;
            if (line.empty()) continue;

            if (line.front() == '[' && line.back() == ']') {
                std::string inner = line.substr(1, line.size() - 2);
                auto sep = findPipe(inner, 0);
                if (sep == std::string::npos) continue;
                OutputChannel ch;
                ch.id   = unesc(inner.substr(0, sep));
                ch.name = unesc(inner.substr(sep + 1));
                m_channels.push_back(std::move(ch));
                current = &m_channels.back();
            } else if (current) {
                auto fields = splitPipe(line);
                if (fields.size() < 5) continue;
                OutputEntry entry;
                entry.id          = unesc(fields[0]);
                entry.channel     = unesc(fields[1]);
                entry.severity    = static_cast<OutputSeverity>(std::stoi(fields[2]));
                entry.timestampMs = std::stoull(fields[3]);
                entry.text        = unesc(fields[4]);
                current->addEntry(entry);
            }
        }
        return true;
    }

    void clear() {
        m_channels.clear();
        m_observers.clear();
    }

    void clearAllEntries() {
        for (auto& ch : m_channels) ch.clear();
    }

private:
    std::vector<OutputChannel>  m_channels;
    std::vector<OutputObserver> m_observers;

    std::vector<OutputChannel>::iterator findChannelIt(const std::string& id) {
        return std::find_if(m_channels.begin(), m_channels.end(),
                            [&](const OutputChannel& c) { return c.id == id; });
    }

    void notifyObservers(const OutputEntry& entry) {
        for (auto& cb : m_observers) {
            if (cb) cb(entry);
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

    static std::string toLower(const std::string& s) {
        std::string out = s;
        for (auto& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    }
};

} // namespace NF
