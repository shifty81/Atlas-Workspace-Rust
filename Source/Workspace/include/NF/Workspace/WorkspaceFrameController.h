#pragma once
// NF::WorkspaceFrameController — Frame pacing, delta-time smoothing, and budget tracking.
//
// The frame controller sits between the OS/platform message loop and the
// WorkspaceShell::update() + render calls. It encapsulates:
//
//   - Target FPS and minimum frame time
//   - Delta-time clamping (prevents spiral-of-death on lag spikes)
//   - Running average smoothing (Exponential Moving Average over last N frames)
//   - Per-frame budget tracking (update time, render time, total time)
//   - Frame statistics (total frames rendered, fps, min/max dt)
//   - Frame skip detection (warns when frame exceeds budget)
//
// Usage:
//
//   WorkspaceFrameController fc;
//   fc.setTargetFPS(60.f);
//
//   while (running) {
//       auto [dt, skip] = fc.beginFrame();        // compute dt + stats
//       shell.update(dt);
//       fc.markUpdateDone();
//       shell.renderAll(w, h);
//       fc.markRenderDone();
//       fc.endFrame();                            // update statistics
//       if (fc.shouldSleep()) Sleep(fc.sleepMs()); // optional pacing
//   }
//
// Thread-safety: single-threaded only.

#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <string>

namespace NF {

// ── Frame Budget ───────────────────────────────────────────────────
// Time budget allocations for a single frame (in milliseconds).

struct FrameBudget {
    float totalMs  = 16.67f; // 60 FPS default (1000 / targetFPS)
    float updateMs = 4.0f;   // target: ≤25% for update
    float renderMs = 12.0f;  // target: ≤75% for render

    [[nodiscard]] bool isValid() const {
        return totalMs > 0.f && updateMs > 0.f && renderMs > 0.f
            && (updateMs + renderMs) <= totalMs * 1.5f; // allow 50% overrun
    }
};

// ── Frame Result ───────────────────────────────────────────────────
// Per-frame result from beginFrame().

struct FrameResult {
    float dt         = 0.f;    // smoothed delta time in seconds
    float rawDt      = 0.f;    // raw (clamped) delta time in seconds
    bool  wasSkipped = false;  // true if previous frame exceeded budget
    uint64_t frameNumber = 0;  // monotonically increasing frame index
};

// ── Frame Statistics ──────────────────────────────────────────────
// Accumulated statistics across all rendered frames.

struct FrameStatistics {
    uint64_t totalFrames  = 0;
    float    fps          = 0.f;   // rolling FPS (from EMA dt)
    float    avgDtMs      = 0.f;   // exponential moving average dt in ms
    float    minDtMs      = 1e9f;  // minimum observed dt in ms
    float    maxDtMs      = 0.f;   // maximum observed dt in ms
    float    lastUpdateMs = 0.f;   // last measured update time (ms)
    float    lastRenderMs = 0.f;   // last measured render time (ms)
    uint32_t skippedFrames = 0;    // frames that exceeded budget

    void reset() { *this = FrameStatistics{}; }

    [[nodiscard]] float budgetUtilization(float totalBudgetMs) const {
        if (totalBudgetMs <= 0.f) return 0.f;
        return (lastUpdateMs + lastRenderMs) / totalBudgetMs;
    }
};

// ── WorkspaceFrameController ──────────────────────────────────────

class WorkspaceFrameController {
public:
    // Default target: 60 FPS, EMA alpha 0.1, max dt 100ms.
    WorkspaceFrameController()
        : m_targetFPS(60.f)
        , m_maxDtSec(0.1f)
        , m_emaAlpha(0.1f)
    {
        m_budget.totalMs  = 1000.f / m_targetFPS;
        m_budget.updateMs = m_budget.totalMs * 0.25f;
        m_budget.renderMs = m_budget.totalMs * 0.75f;
    }

    // ── Configuration ─────────────────────────────────────────────

    void setTargetFPS(float fps) {
        if (fps <= 0.f) return;
        m_targetFPS = fps;
        m_budget.totalMs  = 1000.f / fps;
        m_budget.updateMs = m_budget.totalMs * 0.25f;
        m_budget.renderMs = m_budget.totalMs * 0.75f;
    }

    void setMaxDeltaTime(float maxDtSeconds) {
        if (maxDtSeconds > 0.f) m_maxDtSec = maxDtSeconds;
    }

    void setEMAAlpha(float alpha) {
        if (alpha > 0.f && alpha <= 1.f) m_emaAlpha = alpha;
    }

    void setBudget(FrameBudget budget) {
        if (budget.isValid()) m_budget = budget;
    }

    // ── Frame lifecycle ────────────────────────────────────────────

    // Call at the start of each frame.
    // Returns a FrameResult with smoothed dt and frame number.
    FrameResult beginFrame(float rawDtSeconds) {
        FrameResult fr;
        fr.frameNumber = ++m_frameNumber;

        // Clamp the raw dt
        float clamped = rawDtSeconds;
        if (clamped <= 0.f) clamped = 0.0001f;
        if (clamped > m_maxDtSec) clamped = m_maxDtSec;
        fr.rawDt = clamped;

        // Exponential moving average
        if (m_frameNumber == 1) {
            m_emaDtSec = clamped; // seed on very first frame
        } else {
            m_emaDtSec = m_emaAlpha * clamped + (1.f - m_emaAlpha) * m_emaDtSec;
        }
        fr.dt = m_emaDtSec;

        // Check if previous frame was skipped (over budget)
        fr.wasSkipped = m_prevFrameOverBudget;
        m_prevFrameOverBudget = false;

        m_frameStartSec = rawDtSeconds; // record for timing
        m_updateStarted = false;
        m_renderStarted = false;
        m_updateDoneSec = 0.f;
        m_renderDoneSec = 0.f;

        return fr;
    }

    // Mark that the update phase has finished.
    void markUpdateDone(float elapsedMs = 0.f) {
        m_updateStarted = true;
        m_updateDoneSec = elapsedMs;
    }

    // Mark that the render phase has finished.
    void markRenderDone(float elapsedMs = 0.f) {
        m_renderStarted = true;
        m_renderDoneSec = elapsedMs;
    }

    // Call at the end of each frame to commit statistics.
    void endFrame(float totalFrameMs = 0.f) {
        float dtMs = m_emaDtSec * 1000.f;
        m_stats.totalFrames++;
        m_stats.avgDtMs  = dtMs;
        m_stats.fps      = dtMs > 0.f ? 1000.f / dtMs : 0.f;
        m_stats.minDtMs  = std::min(m_stats.minDtMs, dtMs);
        m_stats.maxDtMs  = std::max(m_stats.maxDtMs, dtMs);

        if (m_updateStarted) m_stats.lastUpdateMs = m_updateDoneSec;
        if (m_renderStarted) m_stats.lastRenderMs = m_renderDoneSec;

        float totalMs = totalFrameMs > 0.f ? totalFrameMs :
                        (m_stats.lastUpdateMs + m_stats.lastRenderMs);
        if (totalMs > m_budget.totalMs) {
            m_prevFrameOverBudget = true;
            m_stats.skippedFrames++;
        }
    }

    // ── Pacing ────────────────────────────────────────────────────

    // Returns true if the frame completed faster than the target budget.
    [[nodiscard]] bool shouldSleep(float actualFrameMs) const {
        return actualFrameMs < m_budget.totalMs;
    }

    // Returns how many milliseconds to sleep to hit the target frame time.
    [[nodiscard]] float sleepMs(float actualFrameMs) const {
        float slack = m_budget.totalMs - actualFrameMs;
        return slack > 0.f ? slack : 0.f;
    }

    // ── Accessors ─────────────────────────────────────────────────

    [[nodiscard]] float                 targetFPS()   const { return m_targetFPS;    }
    [[nodiscard]] float                 maxDtSec()    const { return m_maxDtSec;     }
    [[nodiscard]] float                 emaAlpha()    const { return m_emaAlpha;     }
    [[nodiscard]] float                 emaDtSec()    const { return m_emaDtSec;     }
    [[nodiscard]] const FrameBudget&    budget()      const { return m_budget;       }
    [[nodiscard]] const FrameStatistics& stats()      const { return m_stats;        }
    [[nodiscard]] uint64_t              frameNumber() const { return m_frameNumber;  }

    void resetStats() {
        m_stats.reset();
        m_frameNumber        = 0;
        m_emaDtSec           = 0.f;
        m_prevFrameOverBudget = false;
    }

private:
    float          m_targetFPS  = 60.f;
    float          m_maxDtSec   = 0.1f;
    float          m_emaAlpha   = 0.1f;
    float          m_emaDtSec   = 0.f;
    float          m_frameStartSec = 0.f;
    float          m_updateDoneSec = 0.f;
    float          m_renderDoneSec = 0.f;
    bool           m_updateStarted = false;
    bool           m_renderStarted = false;
    bool           m_prevFrameOverBudget = false;
    uint64_t       m_frameNumber   = 0;
    FrameBudget    m_budget;
    FrameStatistics m_stats;
};

} // namespace NF
