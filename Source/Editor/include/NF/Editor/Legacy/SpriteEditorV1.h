#pragma once
// NF::Editor — Sprite editor v1: 2D sprite atlas and frame authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Sev1FrameState : uint8_t { Active, Hidden, Locked, Deprecated };

inline const char* sev1FrameStateName(Sev1FrameState s) {
    switch (s) {
        case Sev1FrameState::Active:     return "Active";
        case Sev1FrameState::Hidden:     return "Hidden";
        case Sev1FrameState::Locked:     return "Locked";
        case Sev1FrameState::Deprecated: return "Deprecated";
    }
    return "Unknown";
}

struct Sev1Frame {
    uint64_t    id       = 0;
    std::string name;
    float       x        = 0.f;
    float       y        = 0.f;
    float       width    = 0.f;
    float       height   = 0.f;
    float       pivotX   = 0.5f;
    float       pivotY   = 0.5f;
    float       duration = 0.1f;   // seconds per frame in animation
    Sev1FrameState state = Sev1FrameState::Active;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty() && width > 0.f && height > 0.f; }
    [[nodiscard]] float area()  const { return width * height; }
};

using Sev1ChangeCallback = std::function<void(uint64_t)>;

class SpriteEditorV1 {
public:
    static constexpr size_t MAX_FRAMES = 512;

    bool addFrame(const Sev1Frame& frame) {
        if (!frame.isValid()) return false;
        for (const auto& f : m_frames) if (f.id == frame.id) return false;
        if (m_frames.size() >= MAX_FRAMES) return false;
        m_frames.push_back(frame);
        if (m_onChange) m_onChange(frame.id);
        return true;
    }

    bool removeFrame(uint64_t id) {
        for (auto it = m_frames.begin(); it != m_frames.end(); ++it) {
            if (it->id == id) { m_frames.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Sev1Frame* findFrame(uint64_t id) {
        for (auto& f : m_frames) if (f.id == id) return &f;
        return nullptr;
    }

    [[nodiscard]] const Sev1Frame* findFrame(uint64_t id) const {
        for (const auto& f : m_frames) if (f.id == id) return &f;
        return nullptr;
    }

    bool setState(uint64_t id, Sev1FrameState state) {
        auto* f = findFrame(id);
        if (!f) return false;
        f->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setPivot(uint64_t id, float px, float py) {
        auto* f = findFrame(id);
        if (!f) return false;
        f->pivotX = px;
        f->pivotY = py;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t frameCount() const { return m_frames.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (const auto& f : m_frames) if (f.state == Sev1FrameState::Active) ++c;
        return c;
    }

    [[nodiscard]] float totalDuration() const {
        float d = 0.f;
        for (const auto& f : m_frames) if (f.state == Sev1FrameState::Active) d += f.duration;
        return d;
    }

    void setOnChange(Sev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Sev1Frame> m_frames;
    Sev1ChangeCallback     m_onChange;
};

} // namespace NF
