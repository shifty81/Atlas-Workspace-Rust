#pragma once
// NF::Editor — Video clip asset + editor
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

enum class VideoClipCodec : uint8_t {
    H264, H265, VP8, VP9, AV1
};
inline const char* videoClipCodecName(VideoClipCodec c) {
    switch (c) {
        case VideoClipCodec::H264: return "H264";
        case VideoClipCodec::H265: return "H265";
        case VideoClipCodec::VP8:  return "VP8";
        case VideoClipCodec::VP9:  return "VP9";
        case VideoClipCodec::AV1:  return "AV1";
    }
    return "Unknown";
}

enum class VideoClipState : uint8_t {
    Idle, Playing, Paused, Stopped, Finished
};
inline const char* videoClipStateName(VideoClipState s) {
    switch (s) {
        case VideoClipState::Idle:     return "Idle";
        case VideoClipState::Playing:  return "Playing";
        case VideoClipState::Paused:   return "Paused";
        case VideoClipState::Stopped:  return "Stopped";
        case VideoClipState::Finished: return "Finished";
    }
    return "Unknown";
}

enum class VideoAspectRatio : uint8_t {
    Ratio4x3, Ratio16x9, Ratio21x9, Ratio1x1, Custom
};
inline const char* videoAspectRatioName(VideoAspectRatio r) {
    switch (r) {
        case VideoAspectRatio::Ratio4x3:  return "4x3";
        case VideoAspectRatio::Ratio16x9: return "16x9";
        case VideoAspectRatio::Ratio21x9: return "21x9";
        case VideoAspectRatio::Ratio1x1:  return "1x1";
        case VideoAspectRatio::Custom:    return "Custom";
    }
    return "Unknown";
}

class VideoClipAsset {
public:
    explicit VideoClipAsset(const std::string& name,
                             float durationSec = 1.0f,
                             uint32_t fps = 30)
        : m_name(name), m_durationSec(durationSec), m_fps(fps) {}

    void setCodec(VideoClipCodec c)         { m_codec       = c; }
    void setState(VideoClipState s)         { m_state       = s; }
    void setAspectRatio(VideoAspectRatio r) { m_aspectRatio = r; }
    void setWidth(uint32_t w)               { m_width       = w; }
    void setHeight(uint32_t h)              { m_height      = h; }
    void setLooping(bool v)                 { m_looping     = v; }
    void setStreaming(bool v)               { m_streaming   = v; }
    void setDirty(bool v)                   { m_dirty       = v; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] VideoClipCodec     codec()      const { return m_codec;      }
    [[nodiscard]] VideoClipState     state()      const { return m_state;      }
    [[nodiscard]] VideoAspectRatio   aspectRatio()const { return m_aspectRatio;}
    [[nodiscard]] float              durationSec()const { return m_durationSec;}
    [[nodiscard]] uint32_t           fps()        const { return m_fps;        }
    [[nodiscard]] uint32_t           width()      const { return m_width;      }
    [[nodiscard]] uint32_t           height()     const { return m_height;     }
    [[nodiscard]] bool               isLooping()  const { return m_looping;    }
    [[nodiscard]] bool               isStreaming()const { return m_streaming;  }
    [[nodiscard]] bool               isDirty()    const { return m_dirty;      }

    [[nodiscard]] bool isPlaying()  const { return m_state == VideoClipState::Playing;  }
    [[nodiscard]] bool isPaused()   const { return m_state == VideoClipState::Paused;   }
    [[nodiscard]] bool isFinished() const { return m_state == VideoClipState::Finished; }
    [[nodiscard]] bool isHD()       const { return m_width >= 1280 && m_height >= 720;  }

private:
    std::string      m_name;
    VideoClipCodec   m_codec       = VideoClipCodec::H264;
    VideoClipState   m_state       = VideoClipState::Idle;
    VideoAspectRatio m_aspectRatio = VideoAspectRatio::Ratio16x9;
    float            m_durationSec;
    uint32_t         m_fps;
    uint32_t         m_width       = 1920;
    uint32_t         m_height      = 1080;
    bool             m_looping     = false;
    bool             m_streaming   = false;
    bool             m_dirty       = false;
};

class VideoClipEditor {
public:
    static constexpr size_t MAX_CLIPS = 256;

    [[nodiscard]] bool addClip(const VideoClipAsset& clip) {
        if (m_clips.size() >= MAX_CLIPS) return false;
        for (auto& c : m_clips) if (c.name() == clip.name()) return false;
        m_clips.push_back(clip);
        return true;
    }

    [[nodiscard]] bool removeClip(const std::string& name) {
        for (auto it = m_clips.begin(); it != m_clips.end(); ++it) {
            if (it->name() == name) {
                if (m_activeClip == name) m_activeClip.clear();
                m_clips.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] VideoClipAsset* findClip(const std::string& name) {
        for (auto& c : m_clips) if (c.name() == name) return &c;
        return nullptr;
    }

    [[nodiscard]] bool setActiveClip(const std::string& name) {
        for (auto& c : m_clips)
            if (c.name() == name) { m_activeClip = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeClip()   const { return m_activeClip;  }
    [[nodiscard]] size_t clipCount()                const { return m_clips.size(); }
    [[nodiscard]] size_t dirtyCount()               const {
        size_t n = 0; for (auto& c : m_clips) if (c.isDirty())     ++n; return n;
    }
    [[nodiscard]] size_t playingCount()             const {
        size_t n = 0; for (auto& c : m_clips) if (c.isPlaying())   ++n; return n;
    }
    [[nodiscard]] size_t streamingCount()           const {
        size_t n = 0; for (auto& c : m_clips) if (c.isStreaming())  ++n; return n;
    }
    [[nodiscard]] size_t loopingCount()             const {
        size_t n = 0; for (auto& c : m_clips) if (c.isLooping())   ++n; return n;
    }
    [[nodiscard]] size_t hdCount()                  const {
        size_t n = 0; for (auto& c : m_clips) if (c.isHD())        ++n; return n;
    }
    [[nodiscard]] size_t countByCodec(VideoClipCodec codec) const {
        size_t n = 0; for (auto& c : m_clips) if (c.codec() == codec) ++n; return n;
    }
    [[nodiscard]] size_t countByState(VideoClipState s) const {
        size_t n = 0; for (auto& c : m_clips) if (c.state()  == s) ++n; return n;
    }

private:
    std::vector<VideoClipAsset> m_clips;
    std::string                 m_activeClip;
};

// ── S42 — Scene Prefab Editor ──────────────────────────────────


} // namespace NF
