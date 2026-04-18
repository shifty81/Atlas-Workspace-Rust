#pragma once
// NF::Editor — AnimationDocument: animation clip document model.
//
// Phase G.4 — Animation Editor full tool wiring.
//
// An AnimationDocument stores the authoring state for a single animation clip:
//   - A set of channels (one per bone/property)
//   - Per-channel keyframe list (time + value)
//   - Clip metadata (name, duration, fps, looping)
//   - Dirty tracking + save/load contract
//
// AnimationEditorTool owns one AnimationDocument at a time.

#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Identifiers ────────────────────────────────────────────────────────────────

using AnimChannelId  = uint32_t;
using AnimKeyframeId = uint32_t;

static constexpr AnimChannelId  kInvalidAnimChannelId  = 0u;
static constexpr AnimKeyframeId kInvalidAnimKeyframeId = 0u;

// ── Interpolation mode ─────────────────────────────────────────────────────────

enum class AnimInterpolation : uint8_t {
    Step,   ///< hold value until next keyframe
    Linear, ///< linear interpolation
    Bezier, ///< smooth bezier curve
};

inline const char* animInterpolationName(AnimInterpolation i) {
    switch (i) {
    case AnimInterpolation::Step:   return "Step";
    case AnimInterpolation::Linear: return "Linear";
    case AnimInterpolation::Bezier: return "Bezier";
    }
    return "Unknown";
}

// ── Channel type ───────────────────────────────────────────────────────────────

enum class AnimChannelType : uint8_t {
    Translation, ///< XYZ translation
    Rotation,    ///< XYZ euler rotation
    Scale,       ///< XYZ scale
    Float,       ///< generic float property
    Bool,        ///< boolean property (trigger)
    Event,       ///< animation event marker
};

inline const char* animChannelTypeName(AnimChannelType t) {
    switch (t) {
    case AnimChannelType::Translation: return "Translation";
    case AnimChannelType::Rotation:    return "Rotation";
    case AnimChannelType::Scale:       return "Scale";
    case AnimChannelType::Float:       return "Float";
    case AnimChannelType::Bool:        return "Bool";
    case AnimChannelType::Event:       return "Event";
    }
    return "Unknown";
}

// ── AnimKeyframe ───────────────────────────────────────────────────────────────

struct AnimKeyframe {
    AnimKeyframeId    id            = kInvalidAnimKeyframeId;
    AnimChannelId     channelId     = kInvalidAnimChannelId;
    float             timeMs        = 0.f; ///< time in milliseconds
    float             value[4]      = {};  ///< up to 4 floats (for translation/rotation/scale)
    AnimInterpolation interpolation = AnimInterpolation::Linear;
    float             tanIn[4]      = {};  ///< bezier tangent in
    float             tanOut[4]     = {};  ///< bezier tangent out
};

// ── AnimChannel ────────────────────────────────────────────────────────────────

struct AnimChannel {
    AnimChannelId           id       = kInvalidAnimChannelId;
    std::string             boneName; ///< bone/object/property path
    AnimChannelType         type     = AnimChannelType::Float;
    std::vector<AnimKeyframe> keyframes;
};

// ── AnimationDocument ──────────────────────────────────────────────────────────

class AnimationDocument {
public:
    AnimationDocument() = default;
    explicit AnimationDocument(const std::string& clipName) : m_clipName(clipName) {}

    // ── Identity ───────────────────────────────────────────────────────────────

    [[nodiscard]] const std::string& clipName()  const { return m_clipName; }
    [[nodiscard]] const std::string& assetPath() const { return m_assetPath; }
    void setClipName(const std::string& n)  { m_clipName  = n; markDirty(); }
    void setAssetPath(const std::string& p) { m_assetPath = p; }

    // ── Clip metadata ──────────────────────────────────────────────────────────

    [[nodiscard]] float    durationMs() const { return m_durationMs; }
    [[nodiscard]] float    fps()        const { return m_fps; }
    [[nodiscard]] bool     isLooping()  const { return m_looping; }

    void setDurationMs(float ms) { m_durationMs = ms; markDirty(); }
    void setFps(float fps)       { m_fps        = fps; markDirty(); }
    void setLooping(bool loop)   { m_looping    = loop; markDirty(); }

    // ── Dirty tracking ─────────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void markDirty()  { m_dirty = true; }
    void clearDirty() { m_dirty = false; }

    // ── Channel management ─────────────────────────────────────────────────────

    AnimChannelId addChannel(const std::string& boneName, AnimChannelType type) {
        AnimChannelId id = ++m_nextChannelId;
        AnimChannel ch;
        ch.id       = id;
        ch.boneName = boneName;
        ch.type     = type;
        m_channels.push_back(std::move(ch));
        markDirty();
        return id;
    }

    bool removeChannel(AnimChannelId id) {
        for (auto it = m_channels.begin(); it != m_channels.end(); ++it) {
            if (it->id == id) {
                m_channels.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t channelCount() const {
        return static_cast<uint32_t>(m_channels.size());
    }

    [[nodiscard]] const AnimChannel* findChannel(AnimChannelId id) const {
        for (const auto& c : m_channels) if (c.id == id) return &c;
        return nullptr;
    }

    [[nodiscard]] AnimChannel* findChannel(AnimChannelId id) {
        for (auto& c : m_channels) if (c.id == id) return &c;
        return nullptr;
    }

    [[nodiscard]] AnimChannelId findChannelByBone(const std::string& boneName) const {
        for (const auto& c : m_channels) if (c.boneName == boneName) return c.id;
        return kInvalidAnimChannelId;
    }

    [[nodiscard]] const std::vector<AnimChannel>& channels() const { return m_channels; }

    // ── Keyframe management ────────────────────────────────────────────────────

    AnimKeyframeId addKeyframe(AnimChannelId channelId, float timeMs,
                                const float value[4],
                                AnimInterpolation interp = AnimInterpolation::Linear) {
        auto* ch = findChannel(channelId);
        if (!ch) return kInvalidAnimKeyframeId;

        AnimKeyframeId kid = ++m_nextKeyframeId;
        AnimKeyframe kf;
        kf.id            = kid;
        kf.channelId     = channelId;
        kf.timeMs        = timeMs;
        kf.interpolation = interp;
        for (int i = 0; i < 4; ++i) kf.value[i] = value[i];
        ch->keyframes.push_back(kf);

        // Keep keyframes sorted by time
        std::sort(ch->keyframes.begin(), ch->keyframes.end(),
                  [](const AnimKeyframe& a, const AnimKeyframe& b) {
                      return a.timeMs < b.timeMs;
                  });

        markDirty();
        return kid;
    }

    bool removeKeyframe(AnimChannelId channelId, AnimKeyframeId keyframeId) {
        auto* ch = findChannel(channelId);
        if (!ch) return false;
        auto& kfs = ch->keyframes;
        for (auto it = kfs.begin(); it != kfs.end(); ++it) {
            if (it->id == keyframeId) {
                kfs.erase(it);
                markDirty();
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t keyframeCount(AnimChannelId channelId) const {
        if (const auto* ch = findChannel(channelId)) {
            return static_cast<uint32_t>(ch->keyframes.size());
        }
        return 0u;
    }

    [[nodiscard]] uint32_t totalKeyframeCount() const {
        uint32_t total = 0;
        for (const auto& ch : m_channels) {
            total += static_cast<uint32_t>(ch.keyframes.size());
        }
        return total;
    }

    // ── Save / load ────────────────────────────────────────────────────────────

    bool save(const std::string& path = "") {
        if (!path.empty()) m_assetPath = path;
        if (m_assetPath.empty()) return false;
        clearDirty();
        return true;
    }

    bool load(const std::string& /*json*/) {
        clearDirty();
        return true;
    }

    [[nodiscard]] std::string serialize() const {
        std::string out = "{\"clip\":\"" + m_clipName + "\",";
        out += "\"durationMs\":" + std::to_string(m_durationMs) + ",";
        out += "\"channels\":" + std::to_string(m_channels.size()) + "}";
        return out;
    }

private:
    std::string  m_clipName;
    std::string  m_assetPath;
    float        m_durationMs = 0.f;
    float        m_fps        = 30.f;
    bool         m_looping    = false;
    bool         m_dirty      = false;

    AnimChannelId  m_nextChannelId  = 0u;
    AnimKeyframeId m_nextKeyframeId = 0u;

    std::vector<AnimChannel> m_channels;
};

} // namespace NF
