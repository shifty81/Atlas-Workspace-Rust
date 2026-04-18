#include "ProfilerPanel.h"
#include <algorithm>
#include <numeric>

namespace atlas::editor {

void ProfilerPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 200, 20}, "Profiler", {220, 220, 220, 255});

    // Summary line
    std::string summary = "Avg: " + std::to_string(AverageFrameTime()).substr(0, 6)
        + " ms  Peak: " + std::to_string(PeakFrameTime()).substr(0, 6)
        + " ms  Frames: " + std::to_string(FrameCount());
    m_drawList.DrawText({4, 28, 592, 16}, summary, {180, 220, 180, 255});

    // Frame time bars
    int32_t barY = 48;
    size_t start = m_history.size() > 60 ? m_history.size() - 60 : 0;
    for (size_t i = start; i < m_history.size(); ++i) {
        int32_t barH = static_cast<int32_t>(m_history[i].frameDurationMs * 2.0);
        if (barH < 1) barH = 1;
        if (barH > 100) barH = 100;
        int32_t x = static_cast<int32_t>((i - start) * 9 + 4);
        atlas::ui::UIColor barColor = {100, 200, 100, 255};
        if (m_history[i].frameDurationMs > 33.3) barColor = {255, 100, 100, 255};
        else if (m_history[i].frameDurationMs > 16.6) barColor = {255, 200, 100, 255};
        m_drawList.DrawRect({x, barY + 100 - barH, 7, barH}, barColor);
    }

    // System metrics
    int32_t metricY = 160;
    for (const auto& metric : m_currentMetrics) {
        std::string line = metric.systemName + ": "
            + std::to_string(metric.durationMs).substr(0, 6) + " ms";
        m_drawList.DrawText({4, metricY, 400, 16}, line, {200, 200, 200, 255});
        metricY += 18;
    }
}

void ProfilerPanel::RecordFrame(const FrameTiming& timing) {
    if (m_paused) return;
    m_history.push_back(timing);
    if (m_history.size() > m_maxHistory) {
        m_history.erase(m_history.begin());
    }
    m_currentMetrics.clear();
}

void ProfilerPanel::RecordSystemMetric(const SystemMetric& metric) {
    if (m_paused) return;
    m_currentMetrics.push_back(metric);
}

const std::vector<FrameTiming>& ProfilerPanel::History() const {
    return m_history;
}

const std::vector<SystemMetric>& ProfilerPanel::CurrentMetrics() const {
    return m_currentMetrics;
}

double ProfilerPanel::AverageFrameTime() const {
    if (m_history.empty()) return 0.0;
    double total = 0.0;
    for (const auto& f : m_history) {
        total += f.frameDurationMs;
    }
    return total / static_cast<double>(m_history.size());
}

double ProfilerPanel::PeakFrameTime() const {
    if (m_history.empty()) return 0.0;
    double peak = 0.0;
    for (const auto& f : m_history) {
        if (f.frameDurationMs > peak) peak = f.frameDurationMs;
    }
    return peak;
}

uint64_t ProfilerPanel::FrameCount() const {
    return m_history.size();
}

void ProfilerPanel::SetMaxHistory(size_t maxFrames) {
    m_maxHistory = maxFrames;
    if (m_history.size() > m_maxHistory) {
        m_history.erase(m_history.begin(),
                        m_history.begin() + static_cast<std::ptrdiff_t>(m_history.size() - m_maxHistory));
    }
}

size_t ProfilerPanel::MaxHistory() const {
    return m_maxHistory;
}

void ProfilerPanel::ClearHistory() {
    m_history.clear();
    m_currentMetrics.clear();
}

void ProfilerPanel::SetPaused(bool paused) {
    m_paused = paused;
}

bool ProfilerPanel::IsPaused() const {
    return m_paused;
}

}
