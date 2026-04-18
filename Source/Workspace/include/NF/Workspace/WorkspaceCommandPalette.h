#pragma once
// NF::Workspace — Phase 63: Workspace Command Palette
//
// Searchable command registry with categories, keyboard shortcuts,
// history, and execution callbacks.
//
//   CommandCategory   — General / Tool / Edit / View / Build / Navigate / Debug / Plugin
//   CommandEntry      — id + label + category + shortcut + description + handler; isValid()
//   CommandHistory    — ring buffer of recently executed command ids (MAX=64)
//   CommandPalette    — command registry (MAX_COMMANDS=512); register/remove/find;
//                       execute; searchByLabel (case-insensitive)/filterByCategory;
//                       history; observer callbacks; serialize()/deserialize() (registry only)

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// CommandCategory
// ═════════════════════════════════════════════════════════════════

enum class CommandCategory : uint8_t {
    General  = 0,
    Tool     = 1,
    Edit     = 2,
    View     = 3,
    Build    = 4,
    Navigate = 5,
    Debug    = 6,
    Plugin   = 7,
};

inline const char* commandCategoryName(CommandCategory c) {
    switch (c) {
        case CommandCategory::General:  return "General";
        case CommandCategory::Tool:     return "Tool";
        case CommandCategory::Edit:     return "Edit";
        case CommandCategory::View:     return "View";
        case CommandCategory::Build:    return "Build";
        case CommandCategory::Navigate: return "Navigate";
        case CommandCategory::Debug:    return "Debug";
        case CommandCategory::Plugin:   return "Plugin";
        default:                        return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// CommandEntry
// ═════════════════════════════════════════════════════════════════

struct CommandEntry {
    using Handler = std::function<bool()>;

    std::string     id;
    std::string     label;
    CommandCategory category    = CommandCategory::General;
    std::string     shortcut;       // e.g. "Ctrl+Shift+P"
    std::string     description;
    bool            enabled     = true;
    Handler         handler;

    [[nodiscard]] bool isValid() const { return !id.empty() && !label.empty() && handler != nullptr; }

    bool operator==(const CommandEntry& o) const { return id == o.id; }
    bool operator!=(const CommandEntry& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// CommandHistory
// ═════════════════════════════════════════════════════════════════

class CommandHistory {
public:
    static constexpr int MAX_HISTORY = 64;

    void push(const std::string& commandId) {
        if (commandId.empty()) return;
        // Remove existing occurrence so it bubbles to top
        auto it = std::find(m_history.begin(), m_history.end(), commandId);
        if (it != m_history.end()) m_history.erase(it);
        if (static_cast<int>(m_history.size()) >= MAX_HISTORY) {
            m_history.pop_back();
        }
        m_history.insert(m_history.begin(), commandId);
    }

    [[nodiscard]] bool contains(const std::string& commandId) const {
        return std::find(m_history.begin(), m_history.end(), commandId) != m_history.end();
    }

    [[nodiscard]] const std::string* mostRecent() const {
        return m_history.empty() ? nullptr : &m_history.front();
    }

    [[nodiscard]] int count() const { return static_cast<int>(m_history.size()); }
    [[nodiscard]] bool empty() const { return m_history.empty(); }
    [[nodiscard]] const std::vector<std::string>& entries() const { return m_history; }

    void clear() { m_history.clear(); }

private:
    std::vector<std::string> m_history; // most-recent first
};

// ═════════════════════════════════════════════════════════════════
// CommandPalette
// ═════════════════════════════════════════════════════════════════

class CommandPalette {
public:
    using ExecuteObserver = std::function<void(const std::string& commandId, bool succeeded)>;

    static constexpr int MAX_COMMANDS  = 512;
    static constexpr int MAX_OBSERVERS = 16;

    // Command registry ─────────────────────────────────────────

    bool registerCommand(CommandEntry entry) {
        if (!entry.isValid()) return false;
        if (find(entry.id)) return false;
        if (static_cast<int>(m_commands.size()) >= MAX_COMMANDS) return false;
        m_commands.push_back(std::move(entry));
        return true;
    }

    bool unregisterCommand(const std::string& commandId) {
        auto it = findIt(commandId);
        if (it == m_commands.end()) return false;
        m_commands.erase(it);
        return true;
    }

    const CommandEntry* find(const std::string& commandId) const {
        auto it = std::find_if(m_commands.begin(), m_commands.end(),
                               [&](const CommandEntry& c) { return c.id == commandId; });
        return it != m_commands.end() ? &(*it) : nullptr;
    }

    CommandEntry* findMut(const std::string& commandId) {
        auto it = findIt(commandId);
        return it != m_commands.end() ? &(*it) : nullptr;
    }

    bool hasCommand(const std::string& commandId) const { return find(commandId) != nullptr; }

    bool setEnabled(const std::string& commandId, bool enabled) {
        auto* cmd = findMut(commandId);
        if (!cmd) return false;
        cmd->enabled = enabled;
        return true;
    }

    [[nodiscard]] int commandCount() const { return static_cast<int>(m_commands.size()); }

    [[nodiscard]] const std::vector<CommandEntry>& commands() const { return m_commands; }

    // Execution ────────────────────────────────────────────────

    bool execute(const std::string& commandId) {
        const auto* cmd = find(commandId);
        if (!cmd) return false;
        if (!cmd->enabled) return false;
        bool ok = cmd->handler ? cmd->handler() : false;
        m_history.push(commandId);
        notifyObservers(commandId, ok);
        return ok;
    }

    // Search & filter ──────────────────────────────────────────

    [[nodiscard]] std::vector<const CommandEntry*> searchByLabel(const std::string& query) const {
        std::vector<const CommandEntry*> results;
        if (query.empty()) return results;
        std::string lq = toLower(query);
        for (const auto& cmd : m_commands) {
            if (toLower(cmd.label).find(lq) != std::string::npos
                || toLower(cmd.description).find(lq) != std::string::npos) {
                results.push_back(&cmd);
            }
        }
        return results;
    }

    [[nodiscard]] std::vector<const CommandEntry*> filterByCategory(CommandCategory cat) const {
        std::vector<const CommandEntry*> results;
        for (const auto& cmd : m_commands) {
            if (cmd.category == cat) results.push_back(&cmd);
        }
        return results;
    }

    [[nodiscard]] std::vector<const CommandEntry*> findByShortcut(const std::string& shortcut) const {
        std::vector<const CommandEntry*> results;
        for (const auto& cmd : m_commands) {
            if (cmd.shortcut == shortcut) results.push_back(&cmd);
        }
        return results;
    }

    // History ──────────────────────────────────────────────────

    [[nodiscard]] const CommandHistory& history() const { return m_history; }
    CommandHistory&                     history()       { return m_history; }

    // Observers ────────────────────────────────────────────────

    bool addObserver(ExecuteObserver cb) {
        if (!cb) return false;
        if (static_cast<int>(m_observers.size()) >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    // Serialization (registry metadata only, no handlers) ─────

    [[nodiscard]] std::string serialize() const {
        std::string out;
        for (const auto& cmd : m_commands) {
            out += esc(cmd.id) + "|"
                 + esc(cmd.label) + "|"
                 + std::to_string(static_cast<int>(cmd.category)) + "|"
                 + esc(cmd.shortcut) + "|"
                 + (cmd.enabled ? "1" : "0") + "|"
                 + esc(cmd.description) + "\n";
        }
        return out;
    }

    bool deserializeMetadata(const std::string& text) {
        // Updates enabled/shortcut/description for existing commands;
        // does NOT add new commands (handlers are runtime-only).
        if (text.empty()) return true;
        size_t pos = 0;
        while (pos < text.size()) {
            size_t eol = text.find('\n', pos);
            if (eol == std::string::npos) eol = text.size();
            std::string line = text.substr(pos, eol - pos);
            pos = eol + 1;
            if (line.empty()) continue;
            auto fields = splitPipe(line);
            if (fields.size() < 6) continue;
            std::string id = unesc(fields[0]);
            auto* cmd = findMut(id);
            if (!cmd) continue;
            cmd->label       = unesc(fields[1]);
            cmd->category    = static_cast<CommandCategory>(std::stoi(fields[2]));
            cmd->shortcut    = unesc(fields[3]);
            cmd->enabled     = (fields[4] == "1");
            cmd->description = unesc(fields[5]);
        }
        return true;
    }

    void clear() {
        m_commands.clear();
        m_history.clear();
        m_observers.clear();
    }

private:
    std::vector<CommandEntry>    m_commands;
    CommandHistory               m_history;
    std::vector<ExecuteObserver> m_observers;

    std::vector<CommandEntry>::iterator findIt(const std::string& id) {
        return std::find_if(m_commands.begin(), m_commands.end(),
                            [&](const CommandEntry& c) { return c.id == id; });
    }

    void notifyObservers(const std::string& commandId, bool succeeded) {
        for (auto& cb : m_observers) if (cb) cb(commandId, succeeded);
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
