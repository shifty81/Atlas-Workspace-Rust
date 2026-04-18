#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

struct FrameTiming {
    uint64_t frameNumber = 0;
    double frameDurationMs = 0.0;
    double tickDurationMs = 0.0;
    double renderDurationMs = 0.0;
    double idleMs = 0.0;
};

struct SystemMetric {
    std::string systemName;
    double durationMs = 0.0;
    uint32_t entityCount = 0;
};

class ProfilerPanel : public EditorPanel {
public:
    const char* Name() const override { return "Profiler"; }
    void Draw() override;

    // Record a frame timing
    void RecordFrame(const FrameTiming& timing);

    // Record a system metric for the current frame
    void RecordSystemMetric(const SystemMetric& metric);

    // Query
    const std::vector<FrameTiming>& History() const;
    const std::vector<SystemMetric>& CurrentMetrics() const;
    double AverageFrameTime() const;
    double PeakFrameTime() const;
    uint64_t FrameCount() const;

    // Control
    void SetMaxHistory(size_t maxFrames);
    size_t MaxHistory() const;
    void ClearHistory();
    void SetPaused(bool paused);
    bool IsPaused() const;

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::vector<FrameTiming> m_history;
    std::vector<SystemMetric> m_currentMetrics;
    size_t m_maxHistory = 300;
    bool m_paused = false;
    atlas::ui::UIDrawList m_drawList;
};

}
