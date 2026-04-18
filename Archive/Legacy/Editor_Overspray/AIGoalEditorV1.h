#pragma once
// NF::Editor — AI goal editor v1: utility-AI goal and scorer authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Agv1GoalStatus  : uint8_t { Idle, Active, Completed, Failed, Interrupted };
enum class Agv1ScoreMethod : uint8_t { Fixed, Curve, Conditional, Random };

struct Agv1Scorer {
    uint64_t       id      = 0;
    std::string    name;
    Agv1ScoreMethod method = Agv1ScoreMethod::Fixed;
    float          weight  = 1.f;
    float          score   = 0.f;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

struct Agv1Goal {
    uint64_t              id       = 0;
    std::string           label;
    float                 priority = 0.f;
    Agv1GoalStatus        status   = Agv1GoalStatus::Idle;
    std::vector<Agv1Scorer> scorers;
    bool                  enabled  = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty(); }
};

using Agv1GoalCallback = std::function<void(uint64_t, Agv1GoalStatus)>;

class AIGoalEditorV1 {
public:
    bool addGoal(const Agv1Goal& g) {
        if (!g.isValid()) return false;
        for (const auto& eg : m_goals) if (eg.id == g.id) return false;
        m_goals.push_back(g);
        return true;
    }

    bool removeGoal(uint64_t id) {
        for (auto it = m_goals.begin(); it != m_goals.end(); ++it) {
            if (it->id == id) { m_goals.erase(it); return true; }
        }
        return false;
    }

    bool setGoalStatus(uint64_t id, Agv1GoalStatus status) {
        for (auto& g : m_goals) {
            if (g.id == id) {
                g.status = status;
                if (m_onStatus) m_onStatus(id, status);
                return true;
            }
        }
        return false;
    }

    bool addScorer(uint64_t goalId, const Agv1Scorer& scorer) {
        if (!scorer.isValid()) return false;
        for (auto& g : m_goals) {
            if (g.id == goalId) { g.scorers.push_back(scorer); return true; }
        }
        return false;
    }

    bool enableGoal(uint64_t id, bool enable) {
        for (auto& g : m_goals) {
            if (g.id == id) { g.enabled = enable; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t goalCount() const { return m_goals.size(); }

    [[nodiscard]] const Agv1Goal* findGoal(uint64_t id) const {
        for (const auto& g : m_goals) if (g.id == id) return &g;
        return nullptr;
    }

    void setOnStatus(Agv1GoalCallback cb) { m_onStatus = std::move(cb); }

private:
    std::vector<Agv1Goal> m_goals;
    Agv1GoalCallback      m_onStatus;
};

} // namespace NF
