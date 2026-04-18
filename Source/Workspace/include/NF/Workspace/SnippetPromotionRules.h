#pragma once
// NF::Workspace — SnippetPromotionRules: rules for promoting snippets
// from local → shared → codex, with validation and dedup.
//
// Phase 4 component that defines when and how CodexSnippets get
// promoted through the lifecycle stages. Also provides a validation
// and deduplication layer on top of CodexSnippetMirror.
//
// Promotion path:
//   Local → (validate) → Pending → (review/auto) → Synced (codex)
//
// Deduplication:
//   Content hashing (FNV-1a) to detect duplicate snippet bodies.

#include "NF/Workspace/CodexSnippetMirror.h"
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

// ── Snippet Validation Result ─────────────────────────────────────

enum class SnippetValidationStatus : uint8_t {
    Valid,
    EmptyBody,
    BodyTooLong,
    TitleTooLong,
    InvalidLanguage,
    DuplicateContent,
    TooManyTags,
};

inline const char* snippetValidationStatusName(SnippetValidationStatus s) {
    switch (s) {
        case SnippetValidationStatus::Valid:            return "Valid";
        case SnippetValidationStatus::EmptyBody:        return "EmptyBody";
        case SnippetValidationStatus::BodyTooLong:      return "BodyTooLong";
        case SnippetValidationStatus::TitleTooLong:     return "TitleTooLong";
        case SnippetValidationStatus::InvalidLanguage:  return "InvalidLanguage";
        case SnippetValidationStatus::DuplicateContent: return "DuplicateContent";
        case SnippetValidationStatus::TooManyTags:      return "TooManyTags";
    }
    return "Unknown";
}

struct SnippetValidationResult {
    SnippetValidationStatus status = SnippetValidationStatus::Valid;
    std::string             detail;
    uint32_t                duplicateOfId = 0; // if DuplicateContent, the id of existing snippet

    [[nodiscard]] bool isValid() const { return status == SnippetValidationStatus::Valid; }
};

// ── Promotion Rule ────────────────────────────────────────────────

enum class PromotionTrigger : uint8_t {
    Manual,        // user explicitly promotes
    AutoOnSave,    // promote when snippet is saved
    AutoOnUse,     // promote after N uses
    AutoOnReview,  // promote after code review approval
};

inline const char* promotionTriggerName(PromotionTrigger t) {
    switch (t) {
        case PromotionTrigger::Manual:       return "Manual";
        case PromotionTrigger::AutoOnSave:   return "AutoOnSave";
        case PromotionTrigger::AutoOnUse:    return "AutoOnUse";
        case PromotionTrigger::AutoOnReview: return "AutoOnReview";
    }
    return "Unknown";
}

struct PromotionRule {
    uint32_t         id        = 0;
    std::string      name;
    PromotionTrigger trigger   = PromotionTrigger::Manual;
    SnippetLanguage  language  = SnippetLanguage::Any;  // Any = all languages
    uint32_t         minUses   = 0;   // for AutoOnUse trigger
    bool             requirePinned = false; // only promote pinned snippets
    bool             enabled   = true;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

// ── Promotion Log Entry ───────────────────────────────────────────

struct PromotionLogEntry {
    uint32_t snippetId   = 0;
    uint32_t ruleId      = 0;
    SnippetSyncState fromState = SnippetSyncState::Local;
    SnippetSyncState toState   = SnippetSyncState::Pending;
    bool             success   = false;
    std::string      detail;
};

// ── Content Hash Utility ──────────────────────────────────────────
// FNV-1a 64-bit hash for snippet body deduplication.

// FNV-1a 64-bit hash for snippet body deduplication.
// Offset basis: 14695981039346656037 (FNV-1a 64-bit standard value)
// Prime: 1099511628211 (FNV-1a 64-bit standard value)

inline uint64_t fnv1aHash(const std::string& data) {
    constexpr uint64_t kFnv1aOffsetBasis = 14695981039346656037ULL;
    constexpr uint64_t kFnv1aPrime       = 1099511628211ULL;
    uint64_t hash = kFnv1aOffsetBasis;
    for (char c : data) {
        hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        hash *= kFnv1aPrime;
    }
    return hash;
}

// ── Snippet Validation Limits ─────────────────────────────────────

struct SnippetValidationLimits {
    size_t maxBodyLength  = 16384;
    size_t maxTitleLength = 256;
    size_t maxTags        = 16;
};

// ── Snippet Promotion Rules Engine ────────────────────────────────

class SnippetPromotionRules {
public:
    void setLimits(const SnippetValidationLimits& limits) { m_limits = limits; }
    [[nodiscard]] const SnippetValidationLimits& limits() const { return m_limits; }

    // ── Validation ────────────────────────────────────────────────

    [[nodiscard]] SnippetValidationResult validate(const CodexSnippet& snippet) const {
        SnippetValidationResult result;

        if (snippet.body.empty()) {
            result.status = SnippetValidationStatus::EmptyBody;
            result.detail = "Snippet body is empty";
            return result;
        }
        if (snippet.body.size() > m_limits.maxBodyLength) {
            result.status = SnippetValidationStatus::BodyTooLong;
            result.detail = "Body exceeds " + std::to_string(m_limits.maxBodyLength) + " chars";
            return result;
        }
        if (snippet.title.size() > m_limits.maxTitleLength) {
            result.status = SnippetValidationStatus::TitleTooLong;
            result.detail = "Title exceeds " + std::to_string(m_limits.maxTitleLength) + " chars";
            return result;
        }
        if (snippet.tags.size() > m_limits.maxTags) {
            result.status = SnippetValidationStatus::TooManyTags;
            result.detail = "Exceeds max " + std::to_string(m_limits.maxTags) + " tags";
            return result;
        }

        // Check for duplicate content
        uint64_t hash = fnv1aHash(snippet.body);
        auto it = m_contentHashes.find(hash);
        if (it != m_contentHashes.end() && it->second != snippet.id) {
            result.status = SnippetValidationStatus::DuplicateContent;
            result.detail = "Duplicate of snippet " + std::to_string(it->second);
            result.duplicateOfId = it->second;
            return result;
        }

        return result; // Valid
    }

    // ── Deduplication ─────────────────────────────────────────────

    // Register a snippet's content hash for dedup tracking.
    void trackSnippet(uint32_t id, const std::string& body) {
        uint64_t hash = fnv1aHash(body);
        m_contentHashes[hash] = id;
    }

    // Untrack a snippet (e.g. when deleted).
    void untrackSnippet(uint32_t id, const std::string& body) {
        uint64_t hash = fnv1aHash(body);
        auto it = m_contentHashes.find(hash);
        if (it != m_contentHashes.end() && it->second == id)
            m_contentHashes.erase(it);
    }

    // Check if a body already exists (returns snippet id or 0).
    [[nodiscard]] uint32_t findDuplicate(const std::string& body) const {
        uint64_t hash = fnv1aHash(body);
        auto it = m_contentHashes.find(hash);
        return (it != m_contentHashes.end()) ? it->second : 0;
    }

    [[nodiscard]] size_t trackedCount() const { return m_contentHashes.size(); }

    // ── Promotion Rules ───────────────────────────────────────────

    bool addRule(const PromotionRule& rule) {
        if (!rule.isValid()) return false;
        for (const auto& r : m_rules) if (r.id == rule.id) return false;
        m_rules.push_back(rule);
        return true;
    }

    bool removeRule(uint32_t id) {
        for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
            if (it->id == id) { m_rules.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] const PromotionRule* findRule(uint32_t id) const {
        for (const auto& r : m_rules) if (r.id == id) return &r;
        return nullptr;
    }

    [[nodiscard]] const std::vector<PromotionRule>& rules() const { return m_rules; }
    [[nodiscard]] size_t ruleCount() const { return m_rules.size(); }

    // Load a set of sensible default promotion rules.
    void loadDefaults() {
        addRule({1, "Manual promote (all languages)", PromotionTrigger::Manual,
                 SnippetLanguage::Any, 0, false, true});
        addRule({2, "Auto-promote C++ on save", PromotionTrigger::AutoOnSave,
                 SnippetLanguage::Cpp, 0, false, true});
        addRule({3, "Auto-promote after 3 uses", PromotionTrigger::AutoOnUse,
                 SnippetLanguage::Any, 3, false, true});
        addRule({4, "Promote pinned on review", PromotionTrigger::AutoOnReview,
                 SnippetLanguage::Any, 0, true, true});
    }

    // ── Evaluate promotion ────────────────────────────────────────
    // Check which rules apply to a snippet and return applicable rule ids.

    [[nodiscard]] std::vector<uint32_t> evaluatePromotion(
            const CodexSnippet& snippet,
            PromotionTrigger trigger,
            uint32_t useCount = 0) const {
        std::vector<uint32_t> matched;
        for (const auto& rule : m_rules) {
            if (!rule.enabled) continue;
            if (rule.trigger != trigger) continue;
            if (rule.language != SnippetLanguage::Any &&
                rule.language != snippet.language) continue;
            if (rule.requirePinned && !snippet.pinned) continue;
            if (rule.trigger == PromotionTrigger::AutoOnUse &&
                useCount < rule.minUses) continue;
            matched.push_back(rule.id);
        }
        return matched;
    }

    // Attempt to promote a snippet. Returns a log entry for the attempt.
    PromotionLogEntry promote(CodexSnippetMirror& mirror,
                              uint32_t snippetId,
                              uint32_t ruleId = 0) {
        PromotionLogEntry entry;
        entry.snippetId = snippetId;
        entry.ruleId    = ruleId;

        auto* snippet = mirror.findSnippetMut(snippetId);
        if (!snippet) {
            entry.detail = "Snippet not found";
            return entry;
        }

        entry.fromState = snippet->syncState;

        // Validate before promotion
        auto validation = validate(*snippet);
        if (!validation.isValid()) {
            entry.detail = "Validation failed: " + validation.detail;
            return entry;
        }

        // Transition: Local/Modified → Pending
        if (snippet->syncState == SnippetSyncState::Local ||
            snippet->syncState == SnippetSyncState::Modified) {
            snippet->syncState = SnippetSyncState::Pending;
            entry.toState = SnippetSyncState::Pending;
            entry.success = true;
            entry.detail  = "Promoted to Pending";
            ++m_promotionCount;
            m_promotionLog.push_back(entry);
            return entry;
        }

        entry.detail = "Cannot promote from state: " +
                       std::string(snippetSyncStateName(snippet->syncState));
        return entry;
    }

    // ── Promotion log ─────────────────────────────────────────────

    [[nodiscard]] const std::vector<PromotionLogEntry>& promotionLog() const { return m_promotionLog; }
    [[nodiscard]] size_t promotionCount() const { return m_promotionCount; }

    void clearLog() { m_promotionLog.clear(); }

private:
    SnippetValidationLimits                    m_limits;
    std::vector<PromotionRule>                 m_rules;
    std::unordered_map<uint64_t, uint32_t>     m_contentHashes; // hash → snippet id
    std::vector<PromotionLogEntry>             m_promotionLog;
    size_t                                     m_promotionCount = 0;
};

} // namespace NF
