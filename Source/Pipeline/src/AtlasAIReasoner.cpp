// NF::Pipeline — S4 AtlasAI Integration (AtlasAIReasoner)
//
// Implements AtlasAIReasoner: the deterministic rule-based reasoning
// engine that evaluates declarative rules against pipeline events,
// detects violations, and emits findings back into the pipeline.

#include "NF/Pipeline/Pipeline.h"

#include <algorithm>
#include <chrono>
#include <sstream>

namespace NF {

namespace {

static int64_t nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
               system_clock::now().time_since_epoch()).count();
}

// Simple glob-like pattern matching: supports '*' as wildcard.
static bool globMatch(const std::string& pattern, const std::string& text) {
    if (pattern.empty()) return true;  // empty pattern matches everything

    size_t pi = 0, ti = 0;
    size_t starP = std::string::npos, starT = 0;

    while (ti < text.size()) {
        if (pi < pattern.size() && (pattern[pi] == text[ti] || pattern[pi] == '?')) {
            ++pi;
            ++ti;
        } else if (pi < pattern.size() && pattern[pi] == '*') {
            starP = pi++;
            starT = ti;
        } else if (starP != std::string::npos) {
            pi = starP + 1;
            ti = ++starT;
        } else {
            return false;
        }
    }
    while (pi < pattern.size() && pattern[pi] == '*') ++pi;
    return pi == pattern.size();
}

} // anonymous namespace

// ── AtlasAIReasoner ───────────────────────────────────────────────────────

AtlasAIReasoner::AtlasAIReasoner() = default;

// ── AB-1: Rule management ─────────────────────────────────────────────────

void AtlasAIReasoner::addRule(AtlasAIRule rule) {
    m_rules.push_back(std::move(rule));
}

const AtlasAIRule* AtlasAIReasoner::findRule(const std::string& id) const {
    for (const auto& r : m_rules) {
        if (r.id == id) return &r;
    }
    return nullptr;
}

size_t AtlasAIReasoner::loadRulesFromJson(const std::string& json) {
    // Simple parser for array of rule objects.
    // Format: [ { "id": "R001", "description": "...", ... }, ... ]
    size_t count = 0;
    size_t pos = 0;

    while (pos < json.size()) {
        auto objStart = json.find('{', pos);
        if (objStart == std::string::npos) break;

        // Find matching closing brace (non-nested for simple objects).
        int depth = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < json.size() && depth > 0) {
            if (json[objEnd] == '{') ++depth;
            if (json[objEnd] == '}') --depth;
            ++objEnd;
        }
        if (depth != 0) break;

        std::string obj = json.substr(objStart, objEnd - objStart);

        // Extract fields using simple string search.
        auto getStr = [&obj](const std::string& key) -> std::string {
            std::string pat = "\"" + key + "\":";
            auto p = obj.find(pat);
            if (p == std::string::npos) {
                pat = "\"" + key + "\": ";
                p = obj.find(pat);
            }
            if (p == std::string::npos) return {};
            p += pat.size();
            while (p < obj.size() && (obj[p] == ' ' || obj[p] == '\t')) ++p;
            if (p >= obj.size() || obj[p] != '"') return {};
            ++p;
            auto e = obj.find('"', p);
            if (e == std::string::npos) return {};
            return obj.substr(p, e - p);
        };

        AtlasAIRule rule;
        rule.id          = getStr("id");
        rule.description = getStr("description");
        rule.pathPattern = getStr("path_pattern");
        rule.condition   = getStr("condition");
        rule.suggestion  = getStr("suggestion");

        std::string sevStr = getStr("severity");
        if      (sevStr == "Info")     rule.severity = RuleSeverity::Info;
        else if (sevStr == "Warning")  rule.severity = RuleSeverity::Warning;
        else if (sevStr == "Error")    rule.severity = RuleSeverity::Error;
        else if (sevStr == "Critical") rule.severity = RuleSeverity::Critical;

        std::string evtStr = getStr("event_type");
        rule.eventType = changeEventTypeFromString(evtStr);

        if (!rule.id.empty()) {
            m_rules.push_back(std::move(rule));
            ++count;
        }
        pos = objEnd;
    }
    return count;
}

// ── AB-2: Rule evaluation ─────────────────────────────────────────────────

bool AtlasAIReasoner::matchesPath(const std::string& pattern,
                                   const std::string& path) const {
    return globMatch(pattern, path);
}

std::vector<RuleViolation> AtlasAIReasoner::evaluate(const ChangeEvent& event) const {
    std::vector<RuleViolation> results;

    for (const auto& rule : m_rules) {
        // Check event type match.
        if (rule.eventType != ChangeEventType::Unknown &&
            rule.eventType != event.eventType) {
            continue;
        }

        // Check path pattern match.
        if (!rule.pathPattern.empty() &&
            !matchesPath(rule.pathPattern, event.path)) {
            continue;
        }

        // Rule matches — create a violation.
        RuleViolation v;
        v.ruleId      = rule.id;
        v.description = rule.description;
        v.severity    = rule.severity;
        v.path        = event.path;
        v.suggestion  = rule.suggestion;
        v.timestamp   = nowMs();
        results.push_back(std::move(v));
    }

    return results;
}

// ── AB-3: Default game balance rules ──────────────────────────────────────

void AtlasAIReasoner::loadDefaultRules() {
    // Contract quality rules.
    addRule({"R001", "Contract violation detected in source file",
             RuleSeverity::Error, ChangeEventType::ContractIssue,
             "*.cpp", "", "Review and fix the contract violation."});

    addRule({"R002", "Contract violation in header file",
             RuleSeverity::Error, ChangeEventType::ContractIssue,
             "*.h", "", "Ensure header contracts are satisfied."});

    addRule({"R003", "Script requires validation after update",
             RuleSeverity::Warning, ChangeEventType::ScriptUpdated,
             "*.graph", "", "Run ContractScanner on the updated script."});

    // World state rules.
    addRule({"R010", "World state changed — verify entity bounds",
             RuleSeverity::Info, ChangeEventType::WorldChanged,
             "worlds/*", "", "Check entity placement is within map bounds."});

    addRule({"R011", "World sector snapshot requires review",
             RuleSeverity::Warning, ChangeEventType::WorldChanged,
             "worlds/sector*", "", "Review sector diff for unintended changes."});

    // Asset pipeline rules.
    addRule({"R020", "Imported asset should have LOD variants",
             RuleSeverity::Warning, ChangeEventType::AssetImported,
             "*.glb", "", "Generate LOD levels for mesh assets."});

    addRule({"R021", "Animation export should include bone mapping validation",
             RuleSeverity::Info, ChangeEventType::AnimationExported,
             "*.glb", "", "Validate bone mapping against target skeleton."});

    // Replay rules.
    addRule({"R030", "Replay file should be minimized before archival",
             RuleSeverity::Info, ChangeEventType::ReplayExported,
             "*.replay.json", "", "Run ReplayMinimizer to reduce file size."});
}

// ── AB-4: Violation tracking & processing ─────────────────────────────────

size_t AtlasAIReasoner::processEvent(const ChangeEvent& event,
                                      const PipelineDirectories& dirs) {
    auto violations = evaluate(event);
    ++m_eventsProcessed;

    if (violations.empty()) return 0;

    // Record violations.
    for (auto& v : violations) {
        m_violations.push_back(v);
    }

    // Emit an AIAnalysis event summarizing the findings.
    ChangeEvent response;
    response.tool      = "AtlasAI";
    response.eventType = ChangeEventType::AIAnalysis;
    response.path      = event.path;
    response.timestamp = nowMs();

    std::string meta = "violations=" + std::to_string(violations.size());
    for (const auto& v : violations) {
        meta += ";rule=" + v.ruleId + "(" + ruleSeverityName(v.severity) + ")";
    }
    response.metadata = meta;
    response.writeToFile(dirs.changes);

    return violations.size();
}

std::vector<RuleViolation> AtlasAIReasoner::violationsForPath(
    const std::string& path) const {
    std::vector<RuleViolation> result;
    for (const auto& v : m_violations) {
        if (v.path == path) result.push_back(v);
    }
    return result;
}

// ── AB-5: CI gate ─────────────────────────────────────────────────────────

bool AtlasAIReasoner::passesGate() const {
    for (const auto& v : m_violations) {
        if (v.severity == RuleSeverity::Error ||
            v.severity == RuleSeverity::Critical) {
            return false;
        }
    }
    return true;
}

std::string AtlasAIReasoner::summary() const {
    size_t info = 0, warn = 0, err = 0, crit = 0;
    for (const auto& v : m_violations) {
        switch (v.severity) {
            case RuleSeverity::Info:     ++info; break;
            case RuleSeverity::Warning:  ++warn; break;
            case RuleSeverity::Error:    ++err;  break;
            case RuleSeverity::Critical: ++crit; break;
        }
    }

    std::ostringstream oss;
    oss << "AtlasAI: " << m_violations.size() << " violation(s) — "
        << crit << " critical, " << err << " error, "
        << warn << " warning, " << info << " info. "
        << "Gate: " << (passesGate() ? "PASS" : "FAIL") << ". "
        << "Events processed: " << m_eventsProcessed << ".";
    return oss.str();
}

// ── Pipeline integration ──────────────────────────────────────────────────

void AtlasAIReasoner::attachToWatcher(PipelineWatcher& watcher,
                                       const PipelineDirectories& dirs) {
    watcher.subscribe([this, dirs](const ChangeEvent& event) {
        // Skip events emitted by AtlasAI itself to avoid feedback loops.
        if (event.tool == "AtlasAI") return;
        processEvent(event, dirs);
    });
}

} // namespace NF
