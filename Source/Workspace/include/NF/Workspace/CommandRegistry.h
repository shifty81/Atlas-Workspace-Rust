#pragma once
// NF::Workspace — CommandRegistry: register, find, and execute workspace commands.
//
// CommandRegistry is the authoritative store of all WorkspaceCommands in the
// workspace session. It supports:
//   - Register / unregister by command id
//   - Find by id, category, or shortcut
//   - Enable / disable / show / hide individual commands
//   - Execute with optional pre/post hooks and structured result
//   - Query (all, by category)
//
// CommandExecuteResult carries the outcome of an execution attempt so callers
// can distinguish "not found", "disabled", "no handler", and "handler returned
// false" from a clean success.
//
// Pre/post hooks (one each, globally) run before/after every successful dispatch.
// They receive the command id so they can do logging, telemetry, etc.

#include "NF/Workspace/WorkspaceCommand.h"
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Execute Result ─────────────────────────────────────────────────

enum class ExecuteStatus : uint8_t {
    Success,        // handler returned true
    NotFound,       // no command with that id
    Disabled,       // command exists but is not enabled
    NoHandler,      // command has no execute handler
    HandlerFailed,  // handler returned false
};

inline const char* executeStatusName(ExecuteStatus s) {
    switch (s) {
    case ExecuteStatus::Success:       return "Success";
    case ExecuteStatus::NotFound:      return "NotFound";
    case ExecuteStatus::Disabled:      return "Disabled";
    case ExecuteStatus::NoHandler:     return "NoHandler";
    case ExecuteStatus::HandlerFailed: return "HandlerFailed";
    }
    return "Unknown";
}

struct CommandExecuteResult {
    ExecuteStatus status      = ExecuteStatus::NotFound;
    std::string   commandId;

    [[nodiscard]] bool succeeded() const { return status == ExecuteStatus::Success; }
    [[nodiscard]] bool failed()    const { return !succeeded(); }

    static CommandExecuteResult ok(const std::string& id) {
        return {ExecuteStatus::Success, id};
    }
    static CommandExecuteResult notFound(const std::string& id) {
        return {ExecuteStatus::NotFound, id};
    }
    static CommandExecuteResult disabled(const std::string& id) {
        return {ExecuteStatus::Disabled, id};
    }
    static CommandExecuteResult noHandler(const std::string& id) {
        return {ExecuteStatus::NoHandler, id};
    }
    static CommandExecuteResult handlerFailed(const std::string& id) {
        return {ExecuteStatus::HandlerFailed, id};
    }
};

// ── Hook types ─────────────────────────────────────────────────────

using CommandPreHook  = std::function<void(const std::string& commandId)>;
using CommandPostHook = std::function<void(const std::string& commandId,
                                           ExecuteStatus status)>;

// ── Command Registry ───────────────────────────────────────────────

class CommandRegistry {
public:
    static constexpr size_t MAX_COMMANDS = 1024;

    // ── Registration ──────────────────────────────────────────────

    // Returns false if id is empty, already registered, or registry is full.
    bool registerCommand(WorkspaceCommand cmd) {
        if (!cmd.isValid()) return false;
        if (m_commands.size() >= MAX_COMMANDS) return false;
        if (findById(cmd.id()) != nullptr) return false;
        m_commands.push_back(std::move(cmd));
        return true;
    }

    bool unregisterCommand(const std::string& id) {
        for (auto it = m_commands.begin(); it != m_commands.end(); ++it) {
            if (it->id() == id) { m_commands.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] bool isRegistered(const std::string& id) const {
        return findById(id) != nullptr;
    }

    [[nodiscard]] size_t count() const { return m_commands.size(); }

    // ── Lookup ────────────────────────────────────────────────────

    [[nodiscard]] WorkspaceCommand* findById(const std::string& id) {
        for (auto& c : m_commands)
            if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] const WorkspaceCommand* findById(const std::string& id) const {
        for (const auto& c : m_commands)
            if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] const WorkspaceCommand* findByShortcut(const std::string& shortcut) const {
        if (shortcut.empty()) return nullptr;
        for (const auto& c : m_commands)
            if (c.shortcut() == shortcut && c.isEnabled() && c.isVisible()) return &c;
        return nullptr;
    }

    [[nodiscard]] std::vector<const WorkspaceCommand*> findByCategory(CommandCategory cat) const {
        std::vector<const WorkspaceCommand*> result;
        for (const auto& c : m_commands)
            if (c.category() == cat) result.push_back(&c);
        return result;
    }

    [[nodiscard]] const std::vector<WorkspaceCommand>& all() const { return m_commands; }

    // ── State mutation ────────────────────────────────────────────

    bool setEnabled(const std::string& id, bool enabled) {
        auto* cmd = findById(id);
        if (!cmd) return false;
        cmd->setEnabled(enabled);
        return true;
    }

    bool setVisible(const std::string& id, bool visible) {
        auto* cmd = findById(id);
        if (!cmd) return false;
        cmd->setVisible(visible);
        return true;
    }

    bool setChecked(const std::string& id, bool checked) {
        auto* cmd = findById(id);
        if (!cmd) return false;
        cmd->setChecked(checked);
        return true;
    }

    // ── Execution ─────────────────────────────────────────────────

    CommandExecuteResult execute(const std::string& id) {
        auto* cmd = findById(id);
        if (!cmd) return CommandExecuteResult::notFound(id);
        if (!cmd->isEnabled()) return CommandExecuteResult::disabled(id);
        if (!cmd->hasHandler()) return CommandExecuteResult::noHandler(id);

        if (m_preHook) m_preHook(id);

        bool ok = cmd->execute();
        auto status = ok ? ExecuteStatus::Success : ExecuteStatus::HandlerFailed;

        if (m_postHook) m_postHook(id, status);

        return ok ? CommandExecuteResult::ok(id)
                  : CommandExecuteResult::handlerFailed(id);
    }

    // ── Hooks ─────────────────────────────────────────────────────

    void setPreHook(CommandPreHook h)   { m_preHook  = std::move(h); }
    void setPostHook(CommandPostHook h) { m_postHook = std::move(h); }
    void clearHooks() { m_preHook = {}; m_postHook = {}; }

    // ── Batch operations ──────────────────────────────────────────

    void enableAll()  { for (auto& c : m_commands) c.setEnabled(true); }
    void disableAll() { for (auto& c : m_commands) c.setEnabled(false); }

    void clear() { m_commands.clear(); }

private:
    std::vector<WorkspaceCommand> m_commands;
    CommandPreHook  m_preHook;
    CommandPostHook m_postHook;
};

} // namespace NF
