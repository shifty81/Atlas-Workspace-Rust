#pragma once
// NF::Workspace — Console command bus and command palette backend
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class ConsoleCmdScope : uint8_t {
    Global, Editor, Game, Server, Client, Plugin
};

inline const char* consoleCmdScopeName(ConsoleCmdScope s) {
    switch (s) {
        case ConsoleCmdScope::Global: return "Global";
        case ConsoleCmdScope::Editor: return "Editor";
        case ConsoleCmdScope::Game:   return "Game";
        case ConsoleCmdScope::Server: return "Server";
        case ConsoleCmdScope::Client: return "Client";
        case ConsoleCmdScope::Plugin: return "Plugin";
    }
    return "Unknown";
}

enum class ConsoleCmdArgType : uint8_t {
    None, Bool, Int, Float, String, Enum
};

inline const char* consoleCmdArgTypeName(ConsoleCmdArgType t) {
    switch (t) {
        case ConsoleCmdArgType::None:   return "None";
        case ConsoleCmdArgType::Bool:   return "Bool";
        case ConsoleCmdArgType::Int:    return "Int";
        case ConsoleCmdArgType::Float:  return "Float";
        case ConsoleCmdArgType::String: return "String";
        case ConsoleCmdArgType::Enum:   return "Enum";
    }
    return "Unknown";
}

enum class ConsoleCmdExecResult : uint8_t {
    Ok, NotFound, InvalidArgs, PermissionDenied, Error
};

inline const char* consoleCmdExecResultName(ConsoleCmdExecResult r) {
    switch (r) {
        case ConsoleCmdExecResult::Ok:               return "Ok";
        case ConsoleCmdExecResult::NotFound:         return "NotFound";
        case ConsoleCmdExecResult::InvalidArgs:      return "InvalidArgs";
        case ConsoleCmdExecResult::PermissionDenied: return "PermissionDenied";
        case ConsoleCmdExecResult::Error:            return "Error";
    }
    return "Unknown";
}

class ConsoleCommand {
public:
    explicit ConsoleCommand(const std::string& name,
                             ConsoleCmdScope scope,
                             ConsoleCmdArgType argType)
        : m_name(name), m_scope(scope), m_argType(argType) {}

    void setDescription(const std::string& d) { m_description = d; }
    void setEnabled(bool v)                   { m_enabled     = v; }
    void setHidden(bool v)                    { m_hidden      = v; }

    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] ConsoleCmdScope    scope()        const { return m_scope;       }
    [[nodiscard]] ConsoleCmdArgType  argType()      const { return m_argType;     }
    [[nodiscard]] const std::string& description()  const { return m_description; }
    [[nodiscard]] bool               isEnabled()    const { return m_enabled;     }
    [[nodiscard]] bool               isHidden()     const { return m_hidden;      }

private:
    std::string       m_name;
    ConsoleCmdScope   m_scope;
    ConsoleCmdArgType m_argType;
    std::string       m_description;
    bool              m_enabled = true;
    bool              m_hidden  = false;
};

// ── Handler type ──────────────────────────────────────────────────
// Callable stored alongside a command.  When the command is executed the
// handler is invoked and its result is returned to the caller.
// Commands registered without a handler still execute cleanly (returning Ok)
// so that existing code and tests are not affected.
using ConsoleCommandHandler = std::function<ConsoleCmdExecResult()>;

// ── ConsoleCommandEntry ───────────────────────────────────────────
// Internal storage pairing a command descriptor with its optional handler.
struct ConsoleCommandEntry {
    ConsoleCommand       command;
    ConsoleCommandHandler handler;
};

class ConsoleCommandBus {
public:
    static constexpr size_t MAX_COMMANDS = 1024;

    // Register a command without a handler (backward-compatible overload).
    // Executing such a command succeeds silently — useful for testing and for
    // commands whose behaviour will be filled in later.
    [[nodiscard]] bool registerCommand(const ConsoleCommand& cmd) {
        return registerCommand(cmd, {});
    }

    // Register a command with a live handler.  The handler is invoked by
    // execute() and its return value is propagated to the caller.
    [[nodiscard]] bool registerCommand(const ConsoleCommand& cmd,
                                       ConsoleCommandHandler handler) {
        for (auto& e : m_entries) if (e.command.name() == cmd.name()) return false;
        if (m_entries.size() >= MAX_COMMANDS) return false;
        m_entries.push_back({cmd, std::move(handler)});
        return true;
    }

    [[nodiscard]] bool unregisterCommand(const std::string& name) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->command.name() == name) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ConsoleCommand* findCommand(const std::string& name) {
        for (auto& e : m_entries) if (e.command.name() == name) return &e.command;
        return nullptr;
    }

    [[nodiscard]] ConsoleCommandEntry* findEntry(const std::string& name) {
        for (auto& e : m_entries) if (e.command.name() == name) return &e;
        return nullptr;
    }

    // Execute a command by name.
    //   NotFound         — no command with this name is registered
    //   PermissionDenied — the command is disabled
    //   Ok               — command executed (handler returned Ok, or no handler)
    //   Error            — handler returned Error
    [[nodiscard]] ConsoleCmdExecResult execute(const std::string& name) {
        auto* entry = findEntry(name);
        if (!entry)                         return ConsoleCmdExecResult::NotFound;
        if (!entry->command.isEnabled())    return ConsoleCmdExecResult::PermissionDenied;
        m_lastExec = name;
        if (entry->handler)                 return entry->handler();
        return ConsoleCmdExecResult::Ok;    // no handler → silent success
    }

    [[nodiscard]] size_t commandCount()  const { return m_entries.size(); }
    [[nodiscard]] const std::string& lastExec() const { return m_lastExec; }

    [[nodiscard]] size_t countByScope(ConsoleCmdScope s) const {
        size_t c = 0;
        for (auto& e : m_entries) if (e.command.scope() == s) ++c;
        return c;
    }
    [[nodiscard]] size_t hiddenCount() const {
        size_t c = 0;
        for (auto& e : m_entries) if (e.command.isHidden()) ++c;
        return c;
    }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0;
        for (auto& e : m_entries) if (e.command.isEnabled()) ++c;
        return c;
    }

private:
    std::vector<ConsoleCommandEntry> m_entries;
    std::string                      m_lastExec;
};

} // namespace NF
