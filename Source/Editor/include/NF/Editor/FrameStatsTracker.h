#pragma once
// NF::Editor — Frame stats tracker
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

struct FrameStats {
    float fps          = 0.f;
    float frameTimeMs  = 0.f;
    float updateTimeMs = 0.f;
    float renderTimeMs = 0.f;
    uint64_t frameCount = 0;
};

class FrameStatsTracker {
public:
    void beginFrame(float dtSeconds) {
        m_frameTimeMs = dtSeconds * 1000.f;
        // Exponential moving average for FPS
        float newFps = dtSeconds > 0.f ? 1.f / dtSeconds : 0.f;
        m_stats.fps = m_stats.fps * 0.9f + newFps * 0.1f;
        m_stats.frameTimeMs = m_frameTimeMs;
        ++m_stats.frameCount;
        m_updateStart = m_stats.frameCount;  // reuse as a simple "step" marker
    }

    void recordUpdateTime(float ms) { m_stats.updateTimeMs = ms; }
    void recordRenderTime(float ms) { m_stats.renderTimeMs = ms; }

    const FrameStats& stats() const { return m_stats; }

private:
    FrameStats m_stats;
    float m_frameTimeMs = 0.f;
    uint64_t m_updateStart = 0;
};

// ── M2/S1: Dev World Editing ─────────────────────────────────────

// Integer 3D vector for voxel coordinates.

} // namespace NF
