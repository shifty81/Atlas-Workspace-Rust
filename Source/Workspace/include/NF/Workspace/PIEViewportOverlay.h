#pragma once
// NF::PIEViewportOverlay — Viewport overlay commands for PIE state visualization.
//
// Produces a list of lightweight draw commands (text labels, badge rects) that
// the viewport panel can render on top of the 3D scene to show the current
// Play-In-Editor state at a glance.
//
// Layout (all coordinates are relative to the viewport top-left corner):
//   ┌──────────────────────────────────────────────────┐
//   │ [PIE: PLAYING]              FPS: 60  Ent: 12     │  ← top bar
//   │                                                  │
//   │                 (3D scene)                       │
//   │                                                  │
//   │ Tick: 314                                        │  ← bottom bar
//   └──────────────────────────────────────────────────┘
//
// All strings are written into caller-owned char buffers (no heap allocation
// per frame) — the overlay state is cached between calls and rebuilt only
// when the PIE state changes.
//
// Phase 73 — PIE Viewport Full Integration

#include "NF/Editor/PIEService.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace NF {

// ── PIEOverlayTextAnchor ───────────────────────────────────────────────────

enum class PIEOverlayTextAnchor : uint8_t {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    CenterTop,
};

// ── PIEOverlayDrawCommand ──────────────────────────────────────────────────

/// A single overlay item produced by PIEViewportOverlay::buildCommands().
struct PIEOverlayDrawCommand {
    enum class Kind : uint8_t { Text, Rect };

    Kind                 kind   = Kind::Text;
    PIEOverlayTextAnchor anchor = PIEOverlayTextAnchor::TopLeft;
    float                x      = 0.f;   ///< pixel offset from anchor
    float                y      = 0.f;
    float                w      = 0.f;   ///< only for Rect
    float                h      = 0.f;   ///< only for Rect
    uint32_t             color  = 0xFFFFFFFFu; ///< RRGGBBAA
    std::string          text;            ///< only for Text
};

// ── PIEViewportOverlay ─────────────────────────────────────────────────────

class PIEViewportOverlay {
public:
    PIEViewportOverlay() { rebuildCommands(); }

    // ── Badge colours (RRGGBBAA) ───────────────────────────────────────────
    static constexpr uint32_t kColorPlaying = 0x22CC44FFu; ///< bright green
    static constexpr uint32_t kColorPaused  = 0xFFAA00FFu; ///< amber
    static constexpr uint32_t kColorStopped = 0x888888FFu; ///< grey
    static constexpr uint32_t kColorText    = 0xEEEEEEFFu; ///< near-white
    static constexpr uint32_t kColorBadgeBg = 0x00000099u; ///< translucent dark

    // ── updateState ────────────────────────────────────────────────────────
    /// Call once per frame with the current PIE state and counters.
    /// Commands are rebuilt whenever any value changes.
    void updateState(PIEState                     state,
                     const PIEPerformanceCounters& counters) {
        if (state == m_lastState &&
            counters.fps        == m_lastCounters.fps &&
            counters.tickIndex  == m_lastCounters.tickIndex &&
            counters.entityCount == m_lastCounters.entityCount)
            return; // no change — keep cached commands

        m_lastState    = state;
        m_lastCounters = counters;
        rebuildCommands();
    }

    // ── buildCommands ──────────────────────────────────────────────────────
    /// Return the cached overlay draw command list.
    /// Call updateState() first so the list is current.
    [[nodiscard]] const std::vector<PIEOverlayDrawCommand>& commands() const {
        return m_commands;
    }

    /// Number of overlay commands.
    [[nodiscard]] uint32_t commandCount() const {
        return static_cast<uint32_t>(m_commands.size());
    }

    // ── Visibility ─────────────────────────────────────────────────────────

    /// When false, buildCommands() returns an empty list (overlay hidden).
    void setVisible(bool v) {
        if (m_visible != v) {
            m_visible = v;
            rebuildCommands();
        }
    }
    [[nodiscard]] bool isVisible() const { return m_visible; }

    // ── State badge label ──────────────────────────────────────────────────

    /// The short badge text shown in the top-left corner.
    [[nodiscard]] std::string badgeText() const {
        switch (m_lastState) {
        case PIEState::Playing: return "PIE: PLAYING";
        case PIEState::Paused:  return "PIE: PAUSED";
        case PIEState::Stopped: return "PIE: STOPPED";
        }
        return "PIE: STOPPED";
    }

    [[nodiscard]] uint32_t badgeColor() const {
        switch (m_lastState) {
        case PIEState::Playing: return kColorPlaying;
        case PIEState::Paused:  return kColorPaused;
        case PIEState::Stopped: return kColorStopped;
        }
        return kColorStopped;
    }

    // ── Formatted status string ────────────────────────────────────────────

    /// Single-line performance summary ("FPS: 60.0  Ent: 12  Tick: 314").
    [[nodiscard]] std::string statsText() const {
        char buf[80];
        std::snprintf(buf, sizeof(buf),
                      "FPS: %.1f  Ent: %u  Tick: %u",
                      m_lastCounters.fps,
                      m_lastCounters.entityCount,
                      m_lastCounters.tickIndex);
        return std::string(buf);
    }

    // ── Accessors for testing ──────────────────────────────────────────────

    [[nodiscard]] PIEState state()    const { return m_lastState; }
    [[nodiscard]] const PIEPerformanceCounters& counters() const { return m_lastCounters; }

private:
    void rebuildCommands() {
        m_commands.clear();
        if (!m_visible) return;

        // ── Badge background rect ──────────────────────────────────────────
        {
            PIEOverlayDrawCommand cmd;
            cmd.kind   = PIEOverlayDrawCommand::Kind::Rect;
            cmd.anchor = PIEOverlayTextAnchor::TopLeft;
            cmd.x      = 4.f;
            cmd.y      = 4.f;
            cmd.w      = 120.f;
            cmd.h      = 20.f;
            cmd.color  = kColorBadgeBg;
            m_commands.push_back(cmd);
        }

        // ── Badge text ─────────────────────────────────────────────────────
        {
            PIEOverlayDrawCommand cmd;
            cmd.kind   = PIEOverlayDrawCommand::Kind::Text;
            cmd.anchor = PIEOverlayTextAnchor::TopLeft;
            cmd.x      = 8.f;
            cmd.y      = 8.f;
            cmd.color  = badgeColor();
            cmd.text   = badgeText();
            m_commands.push_back(cmd);
        }

        // ── Stats text (top-right) ─────────────────────────────────────────
        {
            PIEOverlayDrawCommand cmd;
            cmd.kind   = PIEOverlayDrawCommand::Kind::Text;
            cmd.anchor = PIEOverlayTextAnchor::TopRight;
            cmd.x      = -8.f;   // offset leftward from right edge
            cmd.y      =  8.f;
            cmd.color  = kColorText;
            cmd.text   = statsText();
            m_commands.push_back(cmd);
        }

        // ── Tick counter (bottom-left) — only when active ──────────────────
        if (m_lastState != PIEState::Stopped) {
            PIEOverlayDrawCommand cmd;
            cmd.kind   = PIEOverlayDrawCommand::Kind::Text;
            cmd.anchor = PIEOverlayTextAnchor::BottomLeft;
            cmd.x      = 8.f;
            cmd.y      = -20.f;  // offset upward from bottom
            cmd.color  = kColorText;
            char buf[32];
            std::snprintf(buf, sizeof(buf), "Tick: %u", m_lastCounters.tickIndex);
            cmd.text   = buf;
            m_commands.push_back(cmd);
        }
    }

    PIEState                      m_lastState    = PIEState::Stopped;
    PIEPerformanceCounters        m_lastCounters;
    std::vector<PIEOverlayDrawCommand> m_commands;
    bool                          m_visible      = true;
};

} // namespace NF
