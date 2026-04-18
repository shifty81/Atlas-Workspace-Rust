#pragma once
// NF::Editor — Live preview controller v1: preview session lifecycle and frame capture authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Lpcv1PreviewMode   : uint8_t { Realtime, Offline, Thumbnail, Streaming };
enum class Lpcv1SessionState  : uint8_t { Stopped, Starting, Running, Paused, Error };
enum class Lpcv1CaptureFormat : uint8_t { PNG, JPEG, EXR, WebP, RAW };

inline const char* lpcv1PreviewModeName(Lpcv1PreviewMode m) {
    switch (m) {
        case Lpcv1PreviewMode::Realtime:  return "Realtime";
        case Lpcv1PreviewMode::Offline:   return "Offline";
        case Lpcv1PreviewMode::Thumbnail: return "Thumbnail";
        case Lpcv1PreviewMode::Streaming: return "Streaming";
    }
    return "Unknown";
}

inline const char* lpcv1SessionStateName(Lpcv1SessionState s) {
    switch (s) {
        case Lpcv1SessionState::Stopped:  return "Stopped";
        case Lpcv1SessionState::Starting: return "Starting";
        case Lpcv1SessionState::Running:  return "Running";
        case Lpcv1SessionState::Paused:   return "Paused";
        case Lpcv1SessionState::Error:    return "Error";
    }
    return "Unknown";
}

inline const char* lpcv1CaptureFormatName(Lpcv1CaptureFormat f) {
    switch (f) {
        case Lpcv1CaptureFormat::PNG:  return "PNG";
        case Lpcv1CaptureFormat::JPEG: return "JPEG";
        case Lpcv1CaptureFormat::EXR:  return "EXR";
        case Lpcv1CaptureFormat::WebP: return "WebP";
        case Lpcv1CaptureFormat::RAW:  return "RAW";
    }
    return "Unknown";
}

struct Lpcv1PreviewSession {
    uint64_t             id             = 0;
    std::string          name;
    std::string          sourceAsset;
    Lpcv1PreviewMode     mode           = Lpcv1PreviewMode::Realtime;
    Lpcv1SessionState    state          = Lpcv1SessionState::Stopped;
    Lpcv1CaptureFormat   captureFormat  = Lpcv1CaptureFormat::PNG;
    uint32_t             width          = 1280;
    uint32_t             height         = 720;
    float                fps            = 60.f;
    uint32_t             framesCaptured = 0;
    bool                 loop           = false;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty() && width > 0 && height > 0 && fps > 0.f; }
    [[nodiscard]] bool isRunning() const { return state == Lpcv1SessionState::Running; }
    [[nodiscard]] bool isPaused()  const { return state == Lpcv1SessionState::Paused; }
    [[nodiscard]] bool hasError()  const { return state == Lpcv1SessionState::Error; }
};

using Lpcv1ChangeCallback = std::function<void(uint64_t)>;

class LivePreviewControllerV1 {
public:
    static constexpr size_t MAX_SESSIONS = 16;

    bool addSession(const Lpcv1PreviewSession& session) {
        if (!session.isValid()) return false;
        for (const auto& s : m_sessions) if (s.id == session.id) return false;
        if (m_sessions.size() >= MAX_SESSIONS) return false;
        m_sessions.push_back(session);
        return true;
    }

    bool removeSession(uint64_t id) {
        for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
            if (it->id == id) {
                if (m_activeId == id) m_activeId = 0;
                m_sessions.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Lpcv1PreviewSession* findSession(uint64_t id) {
        for (auto& s : m_sessions) if (s.id == id) return &s;
        return nullptr;
    }

    bool startSession(uint64_t id) {
        auto* s = findSession(id);
        if (!s) return false;
        s->state    = Lpcv1SessionState::Running;
        m_activeId  = id;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool pauseSession(uint64_t id) {
        auto* s = findSession(id);
        if (!s || !s->isRunning()) return false;
        s->state = Lpcv1SessionState::Paused;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool stopSession(uint64_t id) {
        auto* s = findSession(id);
        if (!s) return false;
        s->state = Lpcv1SessionState::Stopped;
        if (m_activeId == id) m_activeId = 0;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setState(uint64_t id, Lpcv1SessionState state) {
        auto* s = findSession(id);
        if (!s) return false;
        s->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool incrementFrames(uint64_t id, uint32_t count = 1) {
        auto* s = findSession(id);
        if (!s) return false;
        s->framesCaptured += count;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setCaptureFormat(uint64_t id, Lpcv1CaptureFormat fmt) {
        auto* s = findSession(id);
        if (!s) return false;
        s->captureFormat = fmt;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] uint64_t activeId()      const { return m_activeId; }
    [[nodiscard]] size_t   sessionCount()  const { return m_sessions.size(); }
    [[nodiscard]] size_t   runningCount()  const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.isRunning()) ++c; return c;
    }
    [[nodiscard]] size_t   pausedCount()   const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.isPaused())  ++c; return c;
    }
    [[nodiscard]] size_t   errorCount()    const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.hasError())  ++c; return c;
    }
    [[nodiscard]] size_t   countByMode(Lpcv1PreviewMode mode) const {
        size_t c = 0; for (const auto& s : m_sessions) if (s.mode == mode) ++c; return c;
    }

    void setOnChange(Lpcv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Lpcv1PreviewSession> m_sessions;
    uint64_t                         m_activeId = 0;
    Lpcv1ChangeCallback              m_onChange;
};

} // namespace NF
