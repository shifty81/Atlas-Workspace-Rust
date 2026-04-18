#pragma once
// NF::Workspace — BuildGateController: project-level build gating.
//
// BuildGateController answers "can we build right now?" by inspecting the
// active project's load contract and running registered validation rules.
//
// Design:
//   - Reads a ProjectLoadContract (produced by ProjectRegistry::loadProject).
//   - Runs zero or more BuildGateRule callables registered by the caller.
//   - Produces a BuildGateResult that callers can interrogate.
//   - A gate is OPEN (build allowed) only when:
//       1. A project is loaded (state == Ready), AND
//       2. The contract has no Fatal/Error entries, AND
//       3. All registered BuildGateRules pass.
//
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase 5.

#include "NF/Workspace/ProjectLoadContract.h"
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── BuildGateStatus ───────────────────────────────────────────────────────

enum class BuildGateStatus : uint8_t {
    Open,             // build is permitted
    ClosedNoProject,  // no project loaded
    ClosedInvalid,    // project has blocking validation errors
    ClosedByRule,     // a custom BuildGateRule blocked the build
};

inline const char* buildGateStatusName(BuildGateStatus s) {
    switch (s) {
        case BuildGateStatus::Open:             return "Open";
        case BuildGateStatus::ClosedNoProject:  return "ClosedNoProject";
        case BuildGateStatus::ClosedInvalid:    return "ClosedInvalid";
        case BuildGateStatus::ClosedByRule:     return "ClosedByRule";
    }
    return "Unknown";
}

// ── BuildGateRuleResult ───────────────────────────────────────────────────

struct BuildGateRuleResult {
    bool        passed = true;
    std::string ruleId;    // identifies which rule this result belongs to
    std::string reason;    // human-readable reason for a failure
};

// ── BuildGateRule callable type ───────────────────────────────────────────
// Takes a const-ref to the current ProjectLoadContract and returns a result.

using BuildGateRule = std::function<BuildGateRuleResult(const ProjectLoadContract&)>;

// ── BuildGateResult ───────────────────────────────────────────────────────

struct BuildGateResult {
    BuildGateStatus                  status = BuildGateStatus::ClosedNoProject;
    std::vector<BuildGateRuleResult> ruleResults;   // one per registered rule
    std::vector<std::string>         blockingErrors; // from contract + rule failures

    [[nodiscard]] bool isOpen() const { return status == BuildGateStatus::Open; }

    [[nodiscard]] bool hasBlockingErrors() const {
        return !blockingErrors.empty();
    }
};

// ── BuildGateController ───────────────────────────────────────────────────

class BuildGateController {
public:
    // ── Rule registration ─────────────────────────────────────────

    /// Register a named gate rule. Rules are evaluated in registration order.
    void addRule(const std::string& ruleId, BuildGateRule rule) {
        if (!ruleId.empty() && rule)
            m_rules.push_back({ruleId, std::move(rule)});
    }

    void clearRules() { m_rules.clear(); }

    [[nodiscard]] uint32_t ruleCount() const {
        return static_cast<uint32_t>(m_rules.size());
    }

    // ── Evaluation ────────────────────────────────────────────────

    /// Evaluate the gate against a ProjectLoadContract.
    /// Pass an empty/default contract to represent "no project loaded".
    [[nodiscard]] BuildGateResult evaluate(const ProjectLoadContract& contract) const {
        BuildGateResult result;

        // Gate: no project
        if (contract.state == ProjectLoadState::Unloaded) {
            result.status = BuildGateStatus::ClosedNoProject;
            result.blockingErrors.push_back("No project is currently loaded.");
            return result;
        }

        // Gate: project failed to load
        if (contract.state == ProjectLoadState::Failed ||
            contract.state == ProjectLoadState::Loading) {
            result.status = BuildGateStatus::ClosedNoProject;
            result.blockingErrors.push_back(
                "Project '" + contract.projectId + "' is not in Ready state (state=" +
                std::string(projectLoadStateName(contract.state)) + ").");
            return result;
        }

        // Gate: contract-level validation errors / fatals
        for (const auto& entry : contract.validationEntries) {
            if (entry.severity == ProjectValidationSeverity::Fatal ||
                entry.severity == ProjectValidationSeverity::Error) {
                result.blockingErrors.push_back(
                    "[" + std::string(validationSeverityName(entry.severity)) +
                    "] " + entry.code + ": " + entry.message);
            }
        }

        if (!result.blockingErrors.empty()) {
            result.status = BuildGateStatus::ClosedInvalid;
            return result;
        }

        // Gate: custom rules
        bool allRulesPassed = true;
        for (const auto& [entryRuleId, rule] : m_rules) {
            auto rr = rule(contract);
            rr.ruleId = entryRuleId;
            if (!rr.passed) {
                allRulesPassed = false;
                result.blockingErrors.push_back(
                    "[Rule:" + entryRuleId + "] " + rr.reason);
            }
            result.ruleResults.push_back(std::move(rr));
        }

        if (!allRulesPassed) {
            result.status = BuildGateStatus::ClosedByRule;
            return result;
        }

        result.status = BuildGateStatus::Open;
        return result;
    }

    /// Convenience: evaluate against the active contract from a pointer.
    /// Passing nullptr is equivalent to an Unloaded contract.
    [[nodiscard]] BuildGateResult evaluateOptional(const ProjectLoadContract* contract) const {
        if (!contract) {
            ProjectLoadContract empty;
            return evaluate(empty);
        }
        return evaluate(*contract);
    }

private:
    struct RuleEntry {
        std::string    ruleId;
        BuildGateRule  rule;
    };

    std::vector<RuleEntry> m_rules;
};

} // namespace NF
