#pragma once
// NF::Editor — Narrative branch editor v1: story branch and decision point management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Nbrv1BranchType  : uint8_t { Linear, Choice, Random, Conditional, Event };
enum class Nbrv1BranchState : uint8_t { Inactive, Active, Complete, Locked, Hidden };

inline const char* nbrv1BranchTypeName(Nbrv1BranchType t) {
    switch (t) {
        case Nbrv1BranchType::Linear:      return "Linear";
        case Nbrv1BranchType::Choice:      return "Choice";
        case Nbrv1BranchType::Random:      return "Random";
        case Nbrv1BranchType::Conditional: return "Conditional";
        case Nbrv1BranchType::Event:       return "Event";
    }
    return "Unknown";
}

inline const char* nbrv1BranchStateName(Nbrv1BranchState s) {
    switch (s) {
        case Nbrv1BranchState::Inactive: return "Inactive";
        case Nbrv1BranchState::Active:   return "Active";
        case Nbrv1BranchState::Complete: return "Complete";
        case Nbrv1BranchState::Locked:   return "Locked";
        case Nbrv1BranchState::Hidden:   return "Hidden";
    }
    return "Unknown";
}

struct Nbrv1Branch {
    uint64_t          id    = 0;
    std::string       name;
    Nbrv1BranchType   type  = Nbrv1BranchType::Linear;
    Nbrv1BranchState  state = Nbrv1BranchState::Inactive;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()   const { return state == Nbrv1BranchState::Active; }
    [[nodiscard]] bool isComplete() const { return state == Nbrv1BranchState::Complete; }
};

struct Nbrv1Decision {
    uint64_t    id       = 0;
    uint64_t    branchId = 0;
    std::string text;

    [[nodiscard]] bool isValid() const { return id != 0 && branchId != 0 && !text.empty(); }
};

using Nbrv1ChangeCallback = std::function<void(uint64_t)>;

class NarrativeBranchEditorV1 {
public:
    static constexpr size_t MAX_BRANCHES   = 1024;
    static constexpr size_t MAX_DECISIONS  = 8192;

    bool addBranch(const Nbrv1Branch& branch) {
        if (!branch.isValid()) return false;
        for (const auto& b : m_branches) if (b.id == branch.id) return false;
        if (m_branches.size() >= MAX_BRANCHES) return false;
        m_branches.push_back(branch);
        if (m_onChange) m_onChange(branch.id);
        return true;
    }

    bool removeBranch(uint64_t id) {
        for (auto it = m_branches.begin(); it != m_branches.end(); ++it) {
            if (it->id == id) { m_branches.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Nbrv1Branch* findBranch(uint64_t id) {
        for (auto& b : m_branches) if (b.id == id) return &b;
        return nullptr;
    }

    bool addDecision(const Nbrv1Decision& decision) {
        if (!decision.isValid()) return false;
        for (const auto& d : m_decisions) if (d.id == decision.id) return false;
        if (m_decisions.size() >= MAX_DECISIONS) return false;
        m_decisions.push_back(decision);
        if (m_onChange) m_onChange(decision.branchId);
        return true;
    }

    bool removeDecision(uint64_t id) {
        for (auto it = m_decisions.begin(); it != m_decisions.end(); ++it) {
            if (it->id == id) { m_decisions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t branchCount()   const { return m_branches.size(); }
    [[nodiscard]] size_t decisionCount() const { return m_decisions.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& b : m_branches) if (b.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t completeCount() const {
        size_t c = 0; for (const auto& b : m_branches) if (b.isComplete()) ++c; return c;
    }
    [[nodiscard]] size_t countByBranchType(Nbrv1BranchType type) const {
        size_t c = 0; for (const auto& b : m_branches) if (b.type == type) ++c; return c;
    }

    void setOnChange(Nbrv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Nbrv1Branch>   m_branches;
    std::vector<Nbrv1Decision> m_decisions;
    Nbrv1ChangeCallback        m_onChange;
};

} // namespace NF
