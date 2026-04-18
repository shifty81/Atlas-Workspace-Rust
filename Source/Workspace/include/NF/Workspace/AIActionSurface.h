#pragma once
// NF::Editor — AI action surface: apply/reject/diff surface for AI-proposed changes
#include "NF/Workspace/AIPanelSession.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── AI Action Type ────────────────────────────────────────────────

enum class AIActionType : uint8_t {
    ApplyDiff,    // apply a code/file change diff
    RejectDiff,   // dismiss a proposed diff
    RunCommand,   // run a suggested command
    OpenFile,     // open a file suggested by AI
    InsertSnippet,// insert a code snippet
    ShowExplanation, // expand an explanation block
};

inline const char* aiActionTypeName(AIActionType t) {
    switch (t) {
        case AIActionType::ApplyDiff:        return "ApplyDiff";
        case AIActionType::RejectDiff:       return "RejectDiff";
        case AIActionType::RunCommand:       return "RunCommand";
        case AIActionType::OpenFile:         return "OpenFile";
        case AIActionType::InsertSnippet:    return "InsertSnippet";
        case AIActionType::ShowExplanation:  return "ShowExplanation";
    }
    return "Unknown";
}

// ── AI Action Status ──────────────────────────────────────────────

enum class AIActionStatus : uint8_t {
    Pending,
    Applied,
    Rejected,
    Failed,
};

inline const char* aiActionStatusName(AIActionStatus s) {
    switch (s) {
        case AIActionStatus::Pending:  return "Pending";
        case AIActionStatus::Applied:  return "Applied";
        case AIActionStatus::Rejected: return "Rejected";
        case AIActionStatus::Failed:   return "Failed";
    }
    return "Unknown";
}

// ── AI Action Item ────────────────────────────────────────────────

struct AIActionItem {
    uint32_t      id          = 0;
    AIActionType  type        = AIActionType::ApplyDiff;
    AIActionStatus status     = AIActionStatus::Pending;
    std::string   label;
    std::string   payload;     // diff text, command string, snippet, etc.
    std::string   targetPath;  // file path for diffs / open-file actions
    std::string   errorMsg;
    bool          confirmed   = false;  // user confirmed action

    [[nodiscard]] bool isPending()  const { return status == AIActionStatus::Pending; }
    [[nodiscard]] bool isApplied()  const { return status == AIActionStatus::Applied; }
    [[nodiscard]] bool isRejected() const { return status == AIActionStatus::Rejected;}
    [[nodiscard]] bool isValid()    const { return id != 0 && !payload.empty(); }
};

// ── AI Action Surface ─────────────────────────────────────────────
// Manages the list of pending AI-proposed actions and tracks apply/reject.

using AIActionCallback = std::function<bool(AIActionItem&)>;

class AIActionSurface {
public:
    static constexpr size_t MAX_ACTIONS = 64;

    uint32_t propose(AIActionType type, const std::string& label,
                     const std::string& payload, const std::string& targetPath = "") {
        if (m_actions.size() >= MAX_ACTIONS) return 0;

        AIActionItem item;
        item.id         = ++m_nextId;
        item.type       = type;
        item.status     = AIActionStatus::Pending;
        item.label      = label;
        item.payload    = payload;
        item.targetPath = targetPath;
        m_actions.push_back(std::move(item));
        ++m_totalProposed;
        return m_nextId;
    }

    bool apply(uint32_t id) {
        auto* item = findMut(id);
        if (!item || !item->isPending()) return false;

        if (m_applyHandler) {
            if (!m_applyHandler(*item)) {
                item->status = AIActionStatus::Failed;
                ++m_totalFailed;
                return false;
            }
        }
        item->status = AIActionStatus::Applied;
        ++m_totalApplied;
        return true;
    }

    bool reject(uint32_t id) {
        auto* item = findMut(id);
        if (!item || !item->isPending()) return false;
        item->status = AIActionStatus::Rejected;
        ++m_totalRejected;
        return true;
    }

    bool confirm(uint32_t id) {
        auto* item = findMut(id);
        if (!item) return false;
        item->confirmed = true;
        return true;
    }

    void setApplyHandler(AIActionCallback cb) { m_applyHandler = std::move(cb); }

    void clearFinished() {
        m_actions.erase(std::remove_if(m_actions.begin(), m_actions.end(),
            [](const AIActionItem& a) {
                return a.status != AIActionStatus::Pending;
            }), m_actions.end());
    }

    [[nodiscard]] const AIActionItem* find(uint32_t id) const {
        for (const auto& a : m_actions) if (a.id == id) return &a;
        return nullptr;
    }

    [[nodiscard]] size_t pendingCount()      const {
        size_t n = 0;
        for (const auto& a : m_actions) if (a.isPending()) ++n;
        return n;
    }

    [[nodiscard]] size_t totalCount()        const { return m_actions.size();    }
    [[nodiscard]] size_t totalProposed()     const { return m_totalProposed;     }
    [[nodiscard]] size_t totalApplied()      const { return m_totalApplied;      }
    [[nodiscard]] size_t totalRejected()     const { return m_totalRejected;     }
    [[nodiscard]] size_t totalFailed()       const { return m_totalFailed;       }

    [[nodiscard]] const std::vector<AIActionItem>& actions() const { return m_actions; }

private:
    AIActionItem* findMut(uint32_t id) {
        for (auto& a : m_actions) if (a.id == id) return &a;
        return nullptr;
    }

    std::vector<AIActionItem> m_actions;
    AIActionCallback          m_applyHandler;
    uint32_t m_nextId        = 0;
    size_t   m_totalProposed = 0;
    size_t   m_totalApplied  = 0;
    size_t   m_totalRejected = 0;
    size_t   m_totalFailed   = 0;
};

} // namespace NF
