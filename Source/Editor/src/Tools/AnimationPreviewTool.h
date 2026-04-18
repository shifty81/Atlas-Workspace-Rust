#pragma once
#include "ITool.h"
#include <cstdint>
#include <string>

namespace atlas::tools {

/// In-client overlay tool for previewing/scrubbing animations.
///
/// Select an entity and preview its animation clips — scrub the
/// timeline, adjust playback speed, or blend between clips — all
/// while the simulation continues.
class AnimationPreviewTool : public ITool {
public:
    std::string Name() const override { return "Animation Preview"; }
    void Activate() override;
    void Deactivate() override;
    void Update(float deltaTime) override;
    bool IsActive() const override { return m_active; }

    // Target entity
    void SetTarget(uint32_t entityId);
    uint32_t Target() const { return m_target; }

    // Clip selection
    void SetClip(const std::string& clipName);
    std::string CurrentClip() const { return m_clip; }

    // Playback control
    void SetPlaying(bool playing);
    bool IsPlaying() const { return m_playing; }

    void SetPlaybackSpeed(float speed);
    float PlaybackSpeed() const { return m_speed; }

    // Scrub to a specific time position
    void Scrub(float time);
    float CurrentTime() const { return m_currentTime; }

    // Blend weight (0–1) for the current clip
    void SetBlendWeight(float weight);
    float BlendWeight() const { return m_blendWeight; }

    // Reset to start
    void Reset();

private:
    bool m_active = false;
    uint32_t m_target = 0;
    std::string m_clip;
    bool m_playing = false;
    float m_speed = 1.0f;
    float m_currentTime = 0.0f;
    float m_blendWeight = 1.0f;
};

} // namespace atlas::tools
