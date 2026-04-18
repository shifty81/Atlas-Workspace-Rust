#pragma once
// NF::Workspace — ContentRouter: file-type to tool routing rules.
//
// The ContentRouter answers one question: "Which workspace tool should handle
// this asset type?" It maps (AssetTypeTag, optional source) → tool id strings,
// using a priority-ordered list of RoutingRule entries.
//
// Design:
//   RoutingRule          — source/type filter → target tool id
//   ContentRouterPolicy  — what to do when no rule matches (Reject/UseDefault/Prompt)
//   RouteResult          — outcome of routing (toolId + rule name + success)
//   ContentRouter        — holds rules; routes AssetDescriptor or (type, source) pairs

#include "NF/Workspace/AssetCatalog.h"
#include "NF/Workspace/FileIntakePipeline.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Content Router Policy ──────────────────────────────────────────

enum class ContentRouterPolicy : uint8_t {
    Reject,      // no match → return failure
    UseDefault,  // no match → route to the registered default tool
    Prompt,      // no match → signal caller to ask the user
};

inline const char* contentRouterPolicyName(ContentRouterPolicy p) {
    switch (p) {
    case ContentRouterPolicy::Reject:     return "Reject";
    case ContentRouterPolicy::UseDefault: return "UseDefault";
    case ContentRouterPolicy::Prompt:     return "Prompt";
    }
    return "Unknown";
}

// ── Route Result ───────────────────────────────────────────────────

struct RouteResult {
    bool        matched     = false;
    std::string toolId;       // target tool id (empty when matched == false)
    std::string ruleName;     // name of the rule that matched (or "default")
    bool        needsPrompt  = false;  // set when policy == Prompt and no match

    [[nodiscard]] bool succeeded() const { return matched; }
};

// ── Routing Rule ───────────────────────────────────────────────────
// A rule maps a (type, optional source filter) → tool id.
// priority: higher wins; evaluated descending by priority.

struct RoutingRule {
    std::string     name;
    std::string     toolId;
    AssetTypeTag    typeTag     = AssetTypeTag::Unknown;  // Unknown = matches all types
    IntakeSource    sourceFilter = IntakeSource::FileDrop; // used only when filterBySource==true
    bool            filterBySource = false;
    int             priority    = 0;
    bool            enabled     = true;

    [[nodiscard]] bool isValid() const {
        return !name.empty() && !toolId.empty();
    }

    [[nodiscard]] bool matches(AssetTypeTag tag, IntakeSource source) const {
        if (!enabled) return false;
        // Type check: Unknown tag = wildcard
        if (typeTag != AssetTypeTag::Unknown && typeTag != tag) return false;
        // Source check (optional)
        if (filterBySource && sourceFilter != source) return false;
        return true;
    }
};

// ── Content Router ─────────────────────────────────────────────────

class ContentRouter {
public:
    static constexpr size_t MAX_RULES = 128;

    // ── Configuration ────────────────────────────────────────────

    void setPolicy(ContentRouterPolicy policy) { m_policy = policy; }
    void setDefaultToolId(const std::string& id) { m_defaultToolId = id; }

    [[nodiscard]] ContentRouterPolicy policy()        const { return m_policy;        }
    [[nodiscard]] const std::string&  defaultToolId() const { return m_defaultToolId; }

    // ── Rule management ───────────────────────────────────────────

    bool addRule(RoutingRule rule) {
        if (!rule.isValid()) return false;
        if (m_rules.size() >= MAX_RULES) return false;
        for (const auto& r : m_rules) if (r.name == rule.name) return false;
        m_rules.push_back(std::move(rule));
        // Keep sorted by priority descending for fast matching
        std::stable_sort(m_rules.begin(), m_rules.end(),
            [](const RoutingRule& a, const RoutingRule& b) {
                return a.priority > b.priority;
            });
        return true;
    }

    bool removeRule(const std::string& name) {
        for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
            if (it->name == name) { m_rules.erase(it); return true; }
        }
        return false;
    }

    bool enableRule(const std::string& name, bool enabled) {
        for (auto& r : m_rules) {
            if (r.name == name) { r.enabled = enabled; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t ruleCount() const { return m_rules.size(); }
    [[nodiscard]] bool   hasRule(const std::string& name) const {
        for (const auto& r : m_rules) if (r.name == name) return true;
        return false;
    }

    // ── Routing ───────────────────────────────────────────────────

    // Route by asset type tag and intake source
    [[nodiscard]] RouteResult route(AssetTypeTag tag,
                                    IntakeSource source = IntakeSource::FileDrop) const {
        RouteResult result;
        for (const auto& rule : m_rules) {
            if (rule.matches(tag, source)) {
                result.matched  = true;
                result.toolId   = rule.toolId;
                result.ruleName = rule.name;
                ++m_routeCount;
                return result;
            }
        }

        // No match — apply policy
        switch (m_policy) {
        case ContentRouterPolicy::UseDefault:
            if (!m_defaultToolId.empty()) {
                result.matched  = true;
                result.toolId   = m_defaultToolId;
                result.ruleName = "default";
                ++m_routeCount;
                return result;
            }
            break;
        case ContentRouterPolicy::Prompt:
            result.needsPrompt = true;
            break;
        case ContentRouterPolicy::Reject:
        default:
            break;
        }
        ++m_missCount;
        return result; // matched = false
    }

    // Route using an AssetDescriptor
    [[nodiscard]] RouteResult route(const AssetDescriptor& desc) const {
        return route(desc.typeTag);
    }

    // Route using an IntakeItem
    [[nodiscard]] RouteResult route(const IntakeItem& item) const {
        // Map IntakeFileType → AssetTypeTag
        AssetTypeTag tag = intakeTypeToAssetTag(item.fileType);
        return route(tag, item.source);
    }

    [[nodiscard]] size_t routeCount() const { return m_routeCount; }
    [[nodiscard]] size_t missCount()  const { return m_missCount;  }

    [[nodiscard]] const std::vector<RoutingRule>& rules() const { return m_rules; }

    void clearRules() { m_rules.clear(); }

private:
    static AssetTypeTag intakeTypeToAssetTag(IntakeFileType ft) {
        switch (ft) {
        case IntakeFileType::Texture:  return AssetTypeTag::Texture;
        case IntakeFileType::Mesh:     return AssetTypeTag::Mesh;
        case IntakeFileType::Audio:    return AssetTypeTag::Audio;
        case IntakeFileType::Script:   return AssetTypeTag::Script;
        case IntakeFileType::Shader:   return AssetTypeTag::Shader;
        case IntakeFileType::Scene:    return AssetTypeTag::Scene;
        case IntakeFileType::Font:     return AssetTypeTag::Font;
        case IntakeFileType::Video:    return AssetTypeTag::Video;
        case IntakeFileType::Archive:  return AssetTypeTag::Archive;
        case IntakeFileType::Project:  return AssetTypeTag::Project;
        default:                       return AssetTypeTag::Unknown;
        }
    }

    std::vector<RoutingRule> m_rules;
    std::string              m_defaultToolId;
    ContentRouterPolicy      m_policy   = ContentRouterPolicy::Reject;
    mutable size_t           m_routeCount = 0;
    mutable size_t           m_missCount  = 0;
};

} // namespace NF
