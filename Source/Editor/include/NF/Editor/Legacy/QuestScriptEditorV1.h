#pragma once
// NF::Editor — Quest script editor v1: quest script step and condition management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Qscv1ScriptState : uint8_t { Draft, Active, Complete, Failed, Abandoned };
enum class Qscv1StepType    : uint8_t { Trigger, Action, Check, Wait, Branch, End };

inline const char* qscv1ScriptStateName(Qscv1ScriptState s) {
    switch (s) {
        case Qscv1ScriptState::Draft:     return "Draft";
        case Qscv1ScriptState::Active:    return "Active";
        case Qscv1ScriptState::Complete:  return "Complete";
        case Qscv1ScriptState::Failed:    return "Failed";
        case Qscv1ScriptState::Abandoned: return "Abandoned";
    }
    return "Unknown";
}

inline const char* qscv1StepTypeName(Qscv1StepType t) {
    switch (t) {
        case Qscv1StepType::Trigger: return "Trigger";
        case Qscv1StepType::Action:  return "Action";
        case Qscv1StepType::Check:   return "Check";
        case Qscv1StepType::Wait:    return "Wait";
        case Qscv1StepType::Branch:  return "Branch";
        case Qscv1StepType::End:     return "End";
    }
    return "Unknown";
}

struct Qscv1Script {
    uint64_t         id    = 0;
    std::string      name;
    Qscv1ScriptState state = Qscv1ScriptState::Draft;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()   const { return state == Qscv1ScriptState::Active; }
    [[nodiscard]] bool isComplete() const { return state == Qscv1ScriptState::Complete; }
};

struct Qscv1Step {
    uint64_t      id       = 0;
    uint64_t      scriptId = 0;
    std::string   description;
    Qscv1StepType type     = Qscv1StepType::Action;

    [[nodiscard]] bool isValid() const { return id != 0 && scriptId != 0 && !description.empty(); }
};

using Qscv1ChangeCallback = std::function<void(uint64_t)>;

class QuestScriptEditorV1 {
public:
    static constexpr size_t MAX_SCRIPTS = 512;
    static constexpr size_t MAX_STEPS   = 8192;

    bool addScript(const Qscv1Script& script) {
        if (!script.isValid()) return false;
        for (const auto& s : m_scripts) if (s.id == script.id) return false;
        if (m_scripts.size() >= MAX_SCRIPTS) return false;
        m_scripts.push_back(script);
        if (m_onChange) m_onChange(script.id);
        return true;
    }

    bool removeScript(uint64_t id) {
        for (auto it = m_scripts.begin(); it != m_scripts.end(); ++it) {
            if (it->id == id) { m_scripts.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Qscv1Script* findScript(uint64_t id) {
        for (auto& s : m_scripts) if (s.id == id) return &s;
        return nullptr;
    }

    bool addStep(const Qscv1Step& step) {
        if (!step.isValid()) return false;
        for (const auto& st : m_steps) if (st.id == step.id) return false;
        if (m_steps.size() >= MAX_STEPS) return false;
        m_steps.push_back(step);
        if (m_onChange) m_onChange(step.scriptId);
        return true;
    }

    bool removeStep(uint64_t id) {
        for (auto it = m_steps.begin(); it != m_steps.end(); ++it) {
            if (it->id == id) { m_steps.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t scriptCount() const { return m_scripts.size(); }
    [[nodiscard]] size_t stepCount()   const { return m_steps.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& s : m_scripts) if (s.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t completeCount() const {
        size_t c = 0; for (const auto& s : m_scripts) if (s.isComplete()) ++c; return c;
    }
    [[nodiscard]] size_t countByStepType(Qscv1StepType type) const {
        size_t c = 0; for (const auto& st : m_steps) if (st.type == type) ++c; return c;
    }

    void setOnChange(Qscv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Qscv1Script> m_scripts;
    std::vector<Qscv1Step>   m_steps;
    Qscv1ChangeCallback      m_onChange;
};

} // namespace NF
