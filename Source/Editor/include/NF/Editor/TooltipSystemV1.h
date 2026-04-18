#pragma once
// NF::Editor — Tooltip system v1: hover target registration, deferred show/hide, content providers
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Tooltip Position ──────────────────────────────────────────────

enum class TooltipPosition : uint8_t {
    Auto,   // automatic (above or below, avoiding viewport edges)
    Above,
    Below,
    Left,
    Right,
};

inline const char* tooltipPositionName(TooltipPosition p) {
    switch (p) {
        case TooltipPosition::Auto:  return "Auto";
        case TooltipPosition::Above: return "Above";
        case TooltipPosition::Below: return "Below";
        case TooltipPosition::Left:  return "Left";
        case TooltipPosition::Right: return "Right";
    }
    return "Unknown";
}

// ── Tooltip Content ───────────────────────────────────────────────

struct TooltipContent {
    std::string title;
    std::string body;
    std::string iconId;
    std::string keyboardShortcut;
    bool        rich = false;  // markdown/rich-text body

    [[nodiscard]] bool isEmpty() const { return title.empty() && body.empty(); }
};

// ── Tooltip Content Provider ──────────────────────────────────────
// Callback that returns content on demand (lazy evaluation).

using TooltipContentProvider = std::function<TooltipContent()>;

// ── Tooltip Target ────────────────────────────────────────────────
// A hover region that can show a tooltip.

struct TooltipTarget {
    uint32_t              id          = 0;
    std::string           ownerId;    // panel or widget id
    TooltipPosition       position    = TooltipPosition::Auto;
    float                 delayMs     = 400.f;  // show delay
    float                 maxWidthPx  = 300.f;
    bool                  enabled     = true;
    TooltipContent        staticContent;
    TooltipContentProvider provider;   // optional; overrides staticContent if set

    [[nodiscard]] bool isValid() const { return id != 0 && !ownerId.empty(); }
    [[nodiscard]] bool hasDynamicContent() const { return provider != nullptr; }

    [[nodiscard]] TooltipContent resolveContent() const {
        if (provider) return provider();
        return staticContent;
    }
};

// ── Tooltip Show State ────────────────────────────────────────────

enum class TooltipShowState : uint8_t {
    Hidden,
    Pending,   // waiting for delay
    Visible,
    Dismissed,
};

inline const char* tooltipShowStateName(TooltipShowState s) {
    switch (s) {
        case TooltipShowState::Hidden:    return "Hidden";
        case TooltipShowState::Pending:   return "Pending";
        case TooltipShowState::Visible:   return "Visible";
        case TooltipShowState::Dismissed: return "Dismissed";
    }
    return "Unknown";
}

// ── Tooltip System V1 ─────────────────────────────────────────────

class TooltipSystemV1 {
public:
    static constexpr size_t MAX_TARGETS = 1024;

    bool registerTarget(const TooltipTarget& target) {
        if (!target.isValid()) return false;
        if (m_targets.size() >= MAX_TARGETS) return false;
        for (const auto& t : m_targets) if (t.id == target.id) return false;
        m_targets.push_back(target);
        return true;
    }

    bool unregisterTarget(uint32_t id) {
        for (auto it = m_targets.begin(); it != m_targets.end(); ++it) {
            if (it->id == id) {
                if (m_hoveredTargetId == id) cancelHover();
                m_targets.erase(it);
                return true;
            }
        }
        return false;
    }

    bool unregisterByOwner(const std::string& ownerId) {
        size_t removed = 0;
        m_targets.erase(std::remove_if(m_targets.begin(), m_targets.end(),
            [&](const TooltipTarget& t) {
                if (t.ownerId == ownerId) { ++removed; return true; }
                return false;
            }), m_targets.end());
        return removed > 0;
    }

    [[nodiscard]] const TooltipTarget* findTarget(uint32_t id) const {
        for (const auto& t : m_targets) if (t.id == id) return &t;
        return nullptr;
    }

    // Hover events
    void onHoverEnter(uint32_t targetId) {
        auto* t = findTargetMut(targetId);
        if (!t || !t->enabled) return;
        m_hoveredTargetId = targetId;
        m_showState       = TooltipShowState::Pending;
        m_pendingMs       = 0.f;
        m_pendingDelay    = t->delayMs;
    }

    void onHoverExit(uint32_t targetId) {
        if (m_hoveredTargetId == targetId) cancelHover();
    }

    void update(float deltaMs) {
        if (m_showState != TooltipShowState::Pending) return;
        m_pendingMs += deltaMs;
        if (m_pendingMs >= m_pendingDelay) {
            m_showState = TooltipShowState::Visible;
            ++m_showCount;
        }
    }

    void cancelHover() {
        m_hoveredTargetId = 0;
        m_showState       = TooltipShowState::Hidden;
        m_pendingMs       = 0.f;
    }

    void dismiss() {
        m_showState = TooltipShowState::Dismissed;
    }

    [[nodiscard]] bool isVisible()   const { return m_showState == TooltipShowState::Visible;  }
    [[nodiscard]] bool isPending()   const { return m_showState == TooltipShowState::Pending;  }

    [[nodiscard]] TooltipContent currentContent() const {
        auto* t = findTarget(m_hoveredTargetId);
        if (!t) return {};
        return t->resolveContent();
    }

    bool setTargetEnabled(uint32_t id, bool enabled) {
        auto* t = findTargetMut(id);
        if (!t) return false;
        t->enabled = enabled;
        if (!enabled && m_hoveredTargetId == id) cancelHover();
        return true;
    }

    bool updateStaticContent(uint32_t id, const TooltipContent& content) {
        auto* t = findTargetMut(id);
        if (!t) return false;
        t->staticContent = content;
        return true;
    }

    [[nodiscard]] size_t           targetCount()      const { return m_targets.size();     }
    [[nodiscard]] size_t           showCount()        const { return m_showCount;          }
    [[nodiscard]] uint32_t         hoveredTargetId()  const { return m_hoveredTargetId;    }
    [[nodiscard]] TooltipShowState showState()        const { return m_showState;          }
    [[nodiscard]] float            pendingMs()        const { return m_pendingMs;          }

private:
    TooltipTarget* findTargetMut(uint32_t id) {
        for (auto& t : m_targets) if (t.id == id) return &t;
        return nullptr;
    }

    std::vector<TooltipTarget> m_targets;
    uint32_t         m_hoveredTargetId = 0;
    TooltipShowState m_showState       = TooltipShowState::Hidden;
    float            m_pendingMs       = 0.f;
    float            m_pendingDelay    = 0.f;
    size_t           m_showCount       = 0;
};

} // namespace NF
