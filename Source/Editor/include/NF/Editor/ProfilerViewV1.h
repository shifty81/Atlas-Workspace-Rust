#pragma once
// NF::Editor — Profiler view v1: frame-based profiling with sample collection
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class PvvSampleType : uint8_t { CPU, GPU, Memory, IO, Network };

struct PvvSample {
    uint64_t      id          = 0;
    PvvSampleType type        = PvvSampleType::CPU;
    std::string   label;
    float         valueMs     = 0.f;
    float         budgetMs    = 0.f;
    uint64_t      timestampUs = 0;
    [[nodiscard]] bool isValid()     const { return id != 0 && !label.empty(); }
    [[nodiscard]] bool overBudget()  const { return budgetMs > 0.f && valueMs > budgetMs; }
};

struct PvvFrame {
    uint32_t              frameId    = 0;
    std::vector<PvvSample> samples;
    uint64_t              durationUs = 0;
    [[nodiscard]] bool isValid() const { return frameId != 0; }
    [[nodiscard]] float totalMs() const {
        float sum = 0.f;
        for (const auto& s : samples) sum += s.valueMs;
        return sum;
    }
};

using PvvFrameCallback = std::function<void(const PvvFrame&)>;

class ProfilerViewV1 {
public:
    static constexpr size_t maxSamplesPerFrame = 64;
    static constexpr size_t MAX_FRAMES         = 256;

    bool beginFrame(uint32_t frameId) {
        if (m_inFrame) return false;
        m_currentFrame         = PvvFrame{};
        m_currentFrame.frameId = frameId;
        m_inFrame              = true;
        return true;
    }

    bool endFrame() {
        if (!m_inFrame) return false;
        m_inFrame = false;
        if (m_frames.size() >= MAX_FRAMES) m_frames.erase(m_frames.begin());
        m_frames.push_back(m_currentFrame);
        if (m_onFrame) m_onFrame(m_frames.back());
        return true;
    }

    bool addSample(const PvvSample& sample) {
        if (!m_inFrame || !sample.isValid()) return false;
        if (m_currentFrame.samples.size() >= maxSamplesPerFrame) return false;
        m_currentFrame.samples.push_back(sample);
        return true;
    }

    const PvvFrame* getFrame(uint32_t frameId) const {
        for (const auto& f : m_frames) if (f.frameId == frameId) return &f;
        return nullptr;
    }

    void clearHistory() { m_frames.clear(); }

    [[nodiscard]] float averageFrameMs() const {
        if (m_frames.empty()) return 0.f;
        float sum = 0.f;
        for (const auto& f : m_frames) sum += f.totalMs();
        return sum / static_cast<float>(m_frames.size());
    }

    void setOnFrame(PvvFrameCallback cb) { m_onFrame = std::move(cb); }

    [[nodiscard]] size_t frameCount() const { return m_frames.size(); }

private:
    std::vector<PvvFrame> m_frames;
    PvvFrame              m_currentFrame;
    PvvFrameCallback      m_onFrame;
    bool                  m_inFrame = false;
};

} // namespace NF
