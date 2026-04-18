#pragma once
// NF::Editor — Load test editor v1: load test scenario and result tracking
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ldtv1ScenarioState : uint8_t { Draft, Ready, Running, Done, Aborted };
enum class Ldtv1LoadProfile   : uint8_t { Steady, Ramp, Spike, Soak, Step };

inline const char* ldtv1ScenarioStateName(Ldtv1ScenarioState s) {
    switch (s) {
        case Ldtv1ScenarioState::Draft:   return "Draft";
        case Ldtv1ScenarioState::Ready:   return "Ready";
        case Ldtv1ScenarioState::Running: return "Running";
        case Ldtv1ScenarioState::Done:    return "Done";
        case Ldtv1ScenarioState::Aborted: return "Aborted";
    }
    return "Unknown";
}

inline const char* ldtv1LoadProfileName(Ldtv1LoadProfile p) {
    switch (p) {
        case Ldtv1LoadProfile::Steady: return "Steady";
        case Ldtv1LoadProfile::Ramp:   return "Ramp";
        case Ldtv1LoadProfile::Spike:  return "Spike";
        case Ldtv1LoadProfile::Soak:   return "Soak";
        case Ldtv1LoadProfile::Step:   return "Step";
    }
    return "Unknown";
}

struct Ldtv1Result {
    uint64_t    id         = 0;
    uint64_t    scenarioId = 0;
    float       durationMs = 0.f;
    bool        passed     = false;

    [[nodiscard]] bool isValid()  const { return id != 0 && scenarioId != 0; }
    [[nodiscard]] bool isPassed() const { return passed; }
};

struct Ldtv1Scenario {
    uint64_t            id      = 0;
    std::string         name;
    Ldtv1ScenarioState  state   = Ldtv1ScenarioState::Draft;
    Ldtv1LoadProfile    profile = Ldtv1LoadProfile::Steady;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isDone()    const { return state == Ldtv1ScenarioState::Done; }
    [[nodiscard]] bool isAborted() const { return state == Ldtv1ScenarioState::Aborted; }
    [[nodiscard]] bool isRunning() const { return state == Ldtv1ScenarioState::Running; }
};

using Ldtv1ChangeCallback = std::function<void(uint64_t)>;

class LoadTestEditorV1 {
public:
    static constexpr size_t MAX_SCENARIOS = 1024;
    static constexpr size_t MAX_RESULTS   = 65536;

    bool addScenario(const Ldtv1Scenario& scenario) {
        if (!scenario.isValid()) return false;
        for (const auto& s : m_scenarios) if (s.id == scenario.id) return false;
        if (m_scenarios.size() >= MAX_SCENARIOS) return false;
        m_scenarios.push_back(scenario);
        return true;
    }

    bool removeScenario(uint64_t id) {
        for (auto it = m_scenarios.begin(); it != m_scenarios.end(); ++it) {
            if (it->id == id) { m_scenarios.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ldtv1Scenario* findScenario(uint64_t id) {
        for (auto& s : m_scenarios) if (s.id == id) return &s;
        return nullptr;
    }

    bool addResult(const Ldtv1Result& result) {
        if (!result.isValid()) return false;
        for (const auto& r : m_results) if (r.id == result.id) return false;
        if (m_results.size() >= MAX_RESULTS) return false;
        m_results.push_back(result);
        if (m_onChange) m_onChange(result.scenarioId);
        return true;
    }

    bool setState(uint64_t id, Ldtv1ScenarioState state) {
        auto* s = findScenario(id);
        if (!s) return false;
        s->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t scenarioCount() const { return m_scenarios.size(); }

    [[nodiscard]] size_t doneCount() const {
        size_t c = 0; for (const auto& s : m_scenarios) if (s.isDone()) ++c; return c;
    }
    [[nodiscard]] size_t failedCount() const {
        size_t c = 0; for (const auto& s : m_scenarios) if (s.isAborted()) ++c; return c;
    }
    [[nodiscard]] size_t countByProfile(Ldtv1LoadProfile profile) const {
        size_t c = 0; for (const auto& s : m_scenarios) if (s.profile == profile) ++c; return c;
    }

    void setOnChange(Ldtv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ldtv1Scenario> m_scenarios;
    std::vector<Ldtv1Result>   m_results;
    Ldtv1ChangeCallback        m_onChange;
};

} // namespace NF
