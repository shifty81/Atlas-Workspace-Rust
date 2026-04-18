#pragma once
// NF::Editor — Palette command group + command palette
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

enum class CommandPaletteCategory : uint8_t { File, Edit, View, Navigate, Debug, Build, Tools, Help };

inline const char* commandPaletteCategoryName(CommandPaletteCategory c) {
    switch (c) {
        case CommandPaletteCategory::File:     return "File";
        case CommandPaletteCategory::Edit:     return "Edit";
        case CommandPaletteCategory::View:     return "View";
        case CommandPaletteCategory::Navigate: return "Navigate";
        case CommandPaletteCategory::Debug:    return "Debug";
        case CommandPaletteCategory::Build:    return "Build";
        case CommandPaletteCategory::Tools:    return "Tools";
        case CommandPaletteCategory::Help:     return "Help";
        default:                               return "Unknown";
    }
}

enum class CommandPaletteState : uint8_t { Idle, Open, Searching, Executing };

inline const char* commandPaletteStateName(CommandPaletteState s) {
    switch (s) {
        case CommandPaletteState::Idle:      return "Idle";
        case CommandPaletteState::Open:      return "Open";
        case CommandPaletteState::Searching: return "Searching";
        case CommandPaletteState::Executing: return "Executing";
        default:                             return "Unknown";
    }
}

struct PaletteCommand {
    std::string            id;
    std::string            label;
    CommandPaletteCategory category     = CommandPaletteCategory::File;
    bool                   enabled      = true;
    size_t                 executeCount = 0;

    void execute()  { if (enabled) executeCount++; }
    void disable()  { enabled = false; }
    void enable()   { enabled = true;  }

    [[nodiscard]] bool   hasBeenExecuted() const { return executeCount > 0; }
    [[nodiscard]] bool   isEnabled()       const { return enabled;          }
    [[nodiscard]] size_t timesExecuted()   const { return executeCount;     }
};

class PaletteCommandGroup {
public:
    explicit PaletteCommandGroup(const std::string& name) : m_name(name) {}

    [[nodiscard]] const std::string& name() const { return m_name; }

    bool addCommand(PaletteCommand cmd) {
        for (auto& existing : m_commands) if (existing.id == cmd.id) return false;
        m_commands.push_back(std::move(cmd));
        return true;
    }

    bool removeCommand(const std::string& id) {
        for (auto it = m_commands.begin(); it != m_commands.end(); ++it) {
            if (it->id == id) { m_commands.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] PaletteCommand* find(const std::string& id) {
        for (auto& c : m_commands) if (c.id == id) return &c;
        return nullptr;
    }

    void enableAll()  { for (auto& c : m_commands) c.enable();  }
    void disableAll() { for (auto& c : m_commands) c.disable(); }

    [[nodiscard]] size_t commandCount() const { return m_commands.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& c : m_commands) if (c.isEnabled()) n++;
        return n;
    }

private:
    std::string                  m_name;
    std::vector<PaletteCommand>  m_commands;
};

class CommandPalette {
public:
    static constexpr size_t MAX_COMMANDS = 128;

    bool registerCommand(PaletteCommand cmd) {
        if (m_commands.size() >= MAX_COMMANDS) return false;
        for (auto& existing : m_commands) if (existing.id == cmd.id) return false;
        m_commands.push_back(std::move(cmd));
        return true;
    }

    bool unregisterCommand(const std::string& id) {
        for (auto it = m_commands.begin(); it != m_commands.end(); ++it) {
            if (it->id == id) { m_commands.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] PaletteCommand* find(const std::string& id) {
        for (auto& c : m_commands) if (c.id == id) return &c;
        return nullptr;
    }

    bool execute(const std::string& id) {
        auto* cmd = find(id);
        if (!cmd) return false;
        cmd->execute();
        return true;
    }

    [[nodiscard]] std::vector<PaletteCommand*> search(const std::string& query) {
        std::vector<PaletteCommand*> results;
        for (auto& c : m_commands)
            if (c.label.find(query) != std::string::npos) results.push_back(&c);
        return results;
    }

    [[nodiscard]] size_t commandCount() const { return m_commands.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t n = 0;
        for (auto& c : m_commands) if (c.isEnabled()) n++;
        return n;
    }

    void setState(CommandPaletteState s) { m_state = s; }
    [[nodiscard]] CommandPaletteState state() const { return m_state; }

    void open()  { m_state = CommandPaletteState::Open; }
    void close() { m_state = CommandPaletteState::Idle; }

private:
    std::vector<PaletteCommand> m_commands;
    CommandPaletteState         m_state = CommandPaletteState::Idle;
};

// ============================================================
// S27 — Theme Manager
// ============================================================


} // namespace NF
