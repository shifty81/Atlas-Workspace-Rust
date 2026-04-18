#pragma once
// NF::Editor — Frame profiler, memory profiler, timeline, performance profiler
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

enum class ProfileMetricType : uint8_t {
    FrameTime,
    CpuUsage,
    GpuUsage,
    MemoryAlloc,
    DrawCalls,
    TriangleCount,
    ScriptTime,
    NetworkLatency
};

inline const char* profileMetricTypeName(ProfileMetricType t) noexcept {
    switch (t) {
        case ProfileMetricType::FrameTime:       return "FrameTime";
        case ProfileMetricType::CpuUsage:        return "CpuUsage";
        case ProfileMetricType::GpuUsage:        return "GpuUsage";
        case ProfileMetricType::MemoryAlloc:     return "MemoryAlloc";
        case ProfileMetricType::DrawCalls:        return "DrawCalls";
        case ProfileMetricType::TriangleCount:   return "TriangleCount";
        case ProfileMetricType::ScriptTime:      return "ScriptTime";
        case ProfileMetricType::NetworkLatency:  return "NetworkLatency";
        default:                                  return "Unknown";
    }
}

struct ProfileSample {
    ProfileMetricType type = ProfileMetricType::FrameTime;
    float value = 0.f;
    double timestamp = 0.0;
    std::string tag;

    [[nodiscard]] bool hasTag() const { return !tag.empty(); }
};

struct ProfileSession {
    std::string name;
    double startTime = 0.0;
    double endTime = 0.0;
    bool active = false;
    size_t sampleCount = 0;

    void start(double time) { startTime = time; active = true; sampleCount = 0; }
    void stop(double time) { endTime = time; active = false; }
    [[nodiscard]] double duration() const { return active ? 0.0 : endTime - startTime; }
    [[nodiscard]] bool isActive() const { return active; }
};

class FrameProfiler {
public:
    void beginFrame(double timestamp) {
        m_frameStart = timestamp;
        m_inFrame = true;
    }

    void endFrame(double timestamp) {
        if (!m_inFrame) return;
        float frameDuration = static_cast<float>(timestamp - m_frameStart);
        ProfileSample sample;
        sample.type = ProfileMetricType::FrameTime;
        sample.value = frameDuration;
        sample.timestamp = timestamp;
        if (m_samples.size() < kMaxSamples) {
            m_samples.push_back(sample);
        }
        m_totalFrames++;
        m_totalFrameTime += frameDuration;
        if (frameDuration > m_peakFrameTime) m_peakFrameTime = frameDuration;
        m_inFrame = false;
    }

    void recordMetric(ProfileMetricType type, float value, double timestamp,
                      const std::string& tag = "") {
        ProfileSample sample;
        sample.type = type;
        sample.value = value;
        sample.timestamp = timestamp;
        sample.tag = tag;
        if (m_samples.size() < kMaxSamples) {
            m_samples.push_back(sample);
        }
    }

    [[nodiscard]] const std::vector<ProfileSample>& samples() const { return m_samples; }
    [[nodiscard]] size_t sampleCount() const { return m_samples.size(); }
    [[nodiscard]] size_t totalFrames() const { return m_totalFrames; }

    [[nodiscard]] float averageFrameTime() const {
        return m_totalFrames > 0 ? m_totalFrameTime / static_cast<float>(m_totalFrames) : 0.f;
    }

    [[nodiscard]] float peakFrameTime() const { return m_peakFrameTime; }

    [[nodiscard]] std::vector<ProfileSample> samplesByType(ProfileMetricType type) const {
        std::vector<ProfileSample> result;
        for (const auto& s : m_samples) {
            if (s.type == type) result.push_back(s);
        }
        return result;
    }

    void clear() {
        m_samples.clear();
        m_totalFrames = 0;
        m_totalFrameTime = 0.f;
        m_peakFrameTime = 0.f;
        m_inFrame = false;
    }

    static constexpr size_t kMaxSamples = 4096;

private:
    std::vector<ProfileSample> m_samples;
    size_t m_totalFrames = 0;
    float m_totalFrameTime = 0.f;
    float m_peakFrameTime = 0.f;
    double m_frameStart = 0.0;
    bool m_inFrame = false;
};

class MemoryProfiler {
public:
    void trackAllocation(size_t bytes, const std::string& tag = "") {
        m_currentUsage += bytes;
        m_allocationCount++;
        m_totalAllocated += bytes;
        if (m_currentUsage > m_peakUsage) m_peakUsage = m_currentUsage;
        if (!tag.empty()) m_taggedUsage[tag] += bytes;
    }

    void trackFree(size_t bytes, const std::string& tag = "") {
        m_currentUsage = (bytes > m_currentUsage) ? 0 : m_currentUsage - bytes;
        m_freeCount++;
        if (!tag.empty()) {
            auto it = m_taggedUsage.find(tag);
            if (it != m_taggedUsage.end()) {
                it->second = (bytes > it->second) ? 0 : it->second - bytes;
            }
        }
    }

    [[nodiscard]] size_t currentUsage() const { return m_currentUsage; }
    [[nodiscard]] size_t peakUsage() const { return m_peakUsage; }
    [[nodiscard]] size_t allocationCount() const { return m_allocationCount; }
    [[nodiscard]] size_t freeCount() const { return m_freeCount; }
    [[nodiscard]] size_t totalAllocated() const { return m_totalAllocated; }

    [[nodiscard]] size_t taggedUsage(const std::string& tag) const {
        auto it = m_taggedUsage.find(tag);
        return it != m_taggedUsage.end() ? it->second : 0;
    }

    void reset() {
        m_currentUsage = 0;
        m_peakUsage = 0;
        m_allocationCount = 0;
        m_freeCount = 0;
        m_totalAllocated = 0;
        m_taggedUsage.clear();
    }

private:
    size_t m_currentUsage = 0;
    size_t m_peakUsage = 0;
    size_t m_allocationCount = 0;
    size_t m_freeCount = 0;
    size_t m_totalAllocated = 0;
    std::map<std::string, size_t> m_taggedUsage;
};

struct ProfileMarker {
    std::string label;
    double timestamp = 0.0;
    float duration = 0.f;
    std::string category;

    [[nodiscard]] double endTime() const { return timestamp + static_cast<double>(duration); }
};

class ProfilerTimeline {
public:
    void addMarker(const std::string& label, double timestamp, float duration,
                   const std::string& category = "") {
        if (m_markers.size() >= kMaxMarkers) return;
        ProfileMarker m;
        m.label = label;
        m.timestamp = timestamp;
        m.duration = duration;
        m.category = category;
        m_markers.push_back(m);
    }

    [[nodiscard]] std::vector<ProfileMarker> markersInRange(double start, double end) const {
        std::vector<ProfileMarker> result;
        for (const auto& m : m_markers) {
            if (m.timestamp >= start && m.timestamp <= end) result.push_back(m);
        }
        return result;
    }

    [[nodiscard]] std::vector<ProfileMarker> markersByCategory(const std::string& category) const {
        std::vector<ProfileMarker> result;
        for (const auto& m : m_markers) {
            if (m.category == category) result.push_back(m);
        }
        return result;
    }

    [[nodiscard]] size_t markerCount() const { return m_markers.size(); }
    [[nodiscard]] const std::vector<ProfileMarker>& markers() const { return m_markers; }

    void clear() { m_markers.clear(); }

    static constexpr size_t kMaxMarkers = 2048;

private:
    std::vector<ProfileMarker> m_markers;
};

struct PerformanceProfilerConfig {
    bool autoCapture = true;
    size_t maxFrameSamples = 4096;
    size_t maxTimelineMarkers = 2048;
    float warningFrameTimeMs = 33.33f;
    float criticalFrameTimeMs = 50.f;
};

class PerformanceProfiler {
public:
    void init(const PerformanceProfilerConfig& config = {}) {
        m_config = config;
        m_initialized = true;
    }

    void shutdown() {
        m_frameProfiler.clear();
        m_memoryProfiler.reset();
        m_timeline.clear();
        m_initialized = false;
        m_sessionCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    void startSession(const std::string& name, double time) {
        if (!m_initialized) return;
        m_session.name = name;
        m_session.start(time);
        m_sessionCount++;
    }

    void stopSession(double time) {
        if (!m_initialized || !m_session.isActive()) return;
        m_session.stop(time);
    }

    void beginFrame(double timestamp) {
        if (!m_initialized) return;
        m_frameProfiler.beginFrame(timestamp);
    }

    void endFrame(double timestamp) {
        if (!m_initialized) return;
        m_frameProfiler.endFrame(timestamp);
    }

    void recordMetric(ProfileMetricType type, float value, double timestamp,
                      const std::string& tag = "") {
        if (!m_initialized) return;
        m_frameProfiler.recordMetric(type, value, timestamp, tag);
    }

    void trackAllocation(size_t bytes, const std::string& tag = "") {
        if (!m_initialized) return;
        m_memoryProfiler.trackAllocation(bytes, tag);
    }

    void trackFree(size_t bytes, const std::string& tag = "") {
        if (!m_initialized) return;
        m_memoryProfiler.trackFree(bytes, tag);
    }

    void addTimelineMarker(const std::string& label, double timestamp, float duration,
                           const std::string& category = "") {
        if (!m_initialized) return;
        m_timeline.addMarker(label, timestamp, duration, category);
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] const FrameProfiler& frameProfiler() const { return m_frameProfiler; }
    [[nodiscard]] const MemoryProfiler& memoryProfiler() const { return m_memoryProfiler; }
    [[nodiscard]] const ProfilerTimeline& timeline() const { return m_timeline; }
    [[nodiscard]] const ProfileSession& session() const { return m_session; }
    [[nodiscard]] const PerformanceProfilerConfig& config() const { return m_config; }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] size_t sessionCount() const { return m_sessionCount; }
    [[nodiscard]] size_t frameSampleCount() const { return m_frameProfiler.sampleCount(); }
    [[nodiscard]] size_t memoryPeakBytes() const { return m_memoryProfiler.peakUsage(); }
    [[nodiscard]] size_t timelineMarkerCount() const { return m_timeline.markerCount(); }

private:
    PerformanceProfilerConfig m_config;
    FrameProfiler m_frameProfiler;
    MemoryProfiler m_memoryProfiler;
    ProfilerTimeline m_timeline;
    ProfileSession m_session;
    bool m_initialized = false;
    size_t m_tickCount = 0;
    size_t m_sessionCount = 0;
};

// ── S11 — Live Collaboration System ─────────────────────────────


} // namespace NF
