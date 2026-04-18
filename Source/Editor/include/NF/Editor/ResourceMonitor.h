#pragma once
// NF::Editor — Resource monitor channel + system
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

enum class ResourceMonitorMetric : uint8_t {
    CPU        = 0,
    GPU        = 1,
    Memory     = 2,
    DiskIO     = 3,
    NetworkIO  = 4,
    FrameTime  = 5,
    DrawCalls  = 6,
    ThreadLoad = 7,
};

inline const char* resourceMonitorMetricName(ResourceMonitorMetric m) {
    switch (m) {
        case ResourceMonitorMetric::CPU:        return "CPU";
        case ResourceMonitorMetric::GPU:        return "GPU";
        case ResourceMonitorMetric::Memory:     return "Memory";
        case ResourceMonitorMetric::DiskIO:     return "DiskIO";
        case ResourceMonitorMetric::NetworkIO:  return "NetworkIO";
        case ResourceMonitorMetric::FrameTime:  return "FrameTime";
        case ResourceMonitorMetric::DrawCalls:  return "DrawCalls";
        case ResourceMonitorMetric::ThreadLoad: return "ThreadLoad";
        default:                                return "Unknown";
    }
}

enum class ResourceMonitorLevel : uint8_t {
    Normal   = 0,
    Warning  = 1,
    Critical = 2,
    Overflow = 3,
};

struct ResourceMonitorSample {
    ResourceMonitorMetric metric    = ResourceMonitorMetric::CPU;
    ResourceMonitorLevel  level     = ResourceMonitorLevel::Normal;
    float                 value     = 0.0f;
    uint64_t              timestamp = 0;

    [[nodiscard]] bool isWarning()  const { return level == ResourceMonitorLevel::Warning;  }
    [[nodiscard]] bool isCritical() const { return level == ResourceMonitorLevel::Critical; }
    [[nodiscard]] bool isOverflow() const { return level == ResourceMonitorLevel::Overflow; }
    [[nodiscard]] bool isHealthy()  const { return level == ResourceMonitorLevel::Normal;   }

    [[nodiscard]] ResourceMonitorLevel computeLevel(float warnThreshold, float critThreshold) const {
        if (value >= critThreshold) return ResourceMonitorLevel::Critical;
        if (value >= warnThreshold) return ResourceMonitorLevel::Warning;
        return ResourceMonitorLevel::Normal;
    }
};

class ResourceMonitorChannel {
public:
    static constexpr size_t MAX_SAMPLES = 256;

    explicit ResourceMonitorChannel(ResourceMonitorMetric metric) : m_metric(metric) {}

    [[nodiscard]] ResourceMonitorMetric metric() const { return m_metric; }

    bool push(const ResourceMonitorSample& s) {
        if (s.metric != m_metric) return false;
        if (m_samples.size() >= MAX_SAMPLES) m_samples.erase(m_samples.begin());
        m_samples.push_back(s);
        return true;
    }

    [[nodiscard]] const ResourceMonitorSample* latest() const {
        if (m_samples.empty()) return nullptr;
        return &m_samples.back();
    }

    [[nodiscard]] size_t sampleCount() const { return m_samples.size(); }
    [[nodiscard]] bool   empty()       const { return m_samples.empty(); }

    [[nodiscard]] float average() const {
        if (m_samples.empty()) return 0.0f;
        float sum = 0.0f;
        for (auto& s : m_samples) sum += s.value;
        return sum / static_cast<float>(m_samples.size());
    }

    [[nodiscard]] float peak() const {
        float p = 0.0f;
        for (auto& s : m_samples) if (s.value > p) p = s.value;
        return p;
    }

    [[nodiscard]] size_t warningCount() const {
        size_t c = 0;
        for (auto& s : m_samples) if (s.isWarning() || s.isCritical()) c++;
        return c;
    }

    void clear() { m_samples.clear(); }

    [[nodiscard]] const std::vector<ResourceMonitorSample>& samples() const { return m_samples; }

private:
    ResourceMonitorMetric              m_metric;
    std::vector<ResourceMonitorSample> m_samples;
};

class ResourceMonitorSystem {
public:
    void init() {
        m_initialized = true;
        m_channels.clear();
        for (uint8_t i = 0; i <= static_cast<uint8_t>(ResourceMonitorMetric::ThreadLoad); ++i) {
            m_channels.emplace_back(static_cast<ResourceMonitorMetric>(i));
        }
    }

    void shutdown() {
        m_initialized = false;
        m_channels.clear();
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    bool record(const ResourceMonitorSample& s) {
        if (!m_initialized) return false;
        auto* ch = channelFor(s.metric);
        if (!ch) return false;
        return ch->push(s);
    }

    [[nodiscard]] ResourceMonitorChannel* channelFor(ResourceMonitorMetric m) {
        for (auto& ch : m_channels) if (ch.metric() == m) return &ch;
        return nullptr;
    }

    [[nodiscard]] const ResourceMonitorChannel* channelFor(ResourceMonitorMetric m) const {
        for (auto& ch : m_channels) if (ch.metric() == m) return &ch;
        return nullptr;
    }

    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }

    [[nodiscard]] size_t totalSamples() const {
        size_t t = 0;
        for (auto& ch : m_channels) t += ch.sampleCount();
        return t;
    }

    [[nodiscard]] size_t totalWarnings() const {
        size_t t = 0;
        for (auto& ch : m_channels) t += ch.warningCount();
        return t;
    }

    void clearAll() {
        for (auto& ch : m_channels) ch.clear();
    }

private:
    std::vector<ResourceMonitorChannel> m_channels;
    bool                                m_initialized = false;
};

// ============================================================
// S21 — Event Bus System
// ============================================================


} // namespace NF
