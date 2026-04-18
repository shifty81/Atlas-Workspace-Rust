#pragma once
// NF::Editor — Scripting console v1: script execution, command history, output buffering, console channels
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Scv1ChannelType    : uint8_t { Debug, Info, Warning, Error, Script };
enum class Scv1ExecutionState : uint8_t { Idle, Running, Paused, Error, Complete };

inline const char* scv1ChannelTypeName(Scv1ChannelType c) {
    switch (c) {
        case Scv1ChannelType::Debug:   return "Debug";
        case Scv1ChannelType::Info:    return "Info";
        case Scv1ChannelType::Warning: return "Warning";
        case Scv1ChannelType::Error:   return "Error";
        case Scv1ChannelType::Script:  return "Script";
    }
    return "Unknown";
}

inline const char* scv1ExecutionStateName(Scv1ExecutionState s) {
    switch (s) {
        case Scv1ExecutionState::Idle:     return "Idle";
        case Scv1ExecutionState::Running:  return "Running";
        case Scv1ExecutionState::Paused:   return "Paused";
        case Scv1ExecutionState::Error:    return "Error";
        case Scv1ExecutionState::Complete: return "Complete";
    }
    return "Unknown";
}

struct Scv1OutputLine {
    std::string      text;
    Scv1ChannelType  channel = Scv1ChannelType::Info;
    bool             isValid() const { return !text.empty(); }
};

struct Scv1ConsoleChannel {
    std::string     name;
    Scv1ChannelType type    = Scv1ChannelType::Info;
    bool            enabled = true;
    bool isValid() const { return !name.empty(); }
};

struct Scv1ScriptEntry {
    uint64_t             id      = 0;
    std::string          name;
    std::string          source;
    Scv1ExecutionState   state   = Scv1ExecutionState::Idle;
    Scv1ChannelType      channel = Scv1ChannelType::Script;
    uint32_t             executeCount = 0;
    std::vector<Scv1OutputLine> output;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isRunning()  const { return state == Scv1ExecutionState::Running; }
    [[nodiscard]] bool hasError()   const { return state == Scv1ExecutionState::Error; }
    [[nodiscard]] bool isComplete() const { return state == Scv1ExecutionState::Complete; }

    void addOutput(const Scv1OutputLine& line) { if (line.isValid()) output.push_back(line); }
};

using Scv1ExecuteCallback = std::function<void(uint64_t)>;

class ScriptingConsoleV1 {
public:
    static constexpr size_t MAX_ENTRIES = 512;

    bool addEntry(const Scv1ScriptEntry& entry) {
        if (!entry.isValid()) return false;
        for (const auto& e : m_entries) if (e.id == entry.id) return false;
        if (m_entries.size() >= MAX_ENTRIES) return false;
        m_entries.push_back(entry);
        return true;
    }

    bool removeEntry(uint64_t id) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->id == id) { m_entries.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Scv1ScriptEntry* findEntry(uint64_t id) {
        for (auto& e : m_entries) if (e.id == id) return &e;
        return nullptr;
    }

    bool executeScript(uint64_t id) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->state = Scv1ExecutionState::Running;
        ++e->executeCount;
        ++m_executeCount;
        if (m_onExecute) m_onExecute(id);
        return true;
    }

    bool clearHistory(uint64_t id) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->output.clear();
        e->executeCount = 0;
        return true;
    }

    bool setState(uint64_t id, Scv1ExecutionState state) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->state = state;
        if (m_onExecute) m_onExecute(id);
        return true;
    }

    [[nodiscard]] size_t entryCount()   const { return m_entries.size(); }
    [[nodiscard]] size_t executeCount() const { return m_executeCount; }

    [[nodiscard]] size_t countByChannel(Scv1ChannelType channel) const {
        size_t c = 0;
        for (const auto& e : m_entries) if (e.channel == channel) ++c;
        return c;
    }

    void setOnExecute(Scv1ExecuteCallback cb) { m_onExecute = std::move(cb); }

private:
    std::vector<Scv1ScriptEntry> m_entries;
    size_t                       m_executeCount = 0;
    Scv1ExecuteCallback          m_onExecute;
};

} // namespace NF
