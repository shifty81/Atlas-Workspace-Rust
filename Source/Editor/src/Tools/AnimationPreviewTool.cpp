#include "AnimationPreviewTool.h"
#include <algorithm>

namespace atlas::tools {

void AnimationPreviewTool::Activate() {
    m_active = true;
    Reset();
}

void AnimationPreviewTool::Deactivate() {
    m_active = false;
    m_playing = false;
}

void AnimationPreviewTool::Update(float deltaTime) {
    if (!m_active || !m_playing) return;
    m_currentTime += deltaTime * m_speed;
}

void AnimationPreviewTool::SetTarget(uint32_t entityId) {
    m_target = entityId;
    Reset();
}

void AnimationPreviewTool::SetClip(const std::string& clipName) {
    m_clip = clipName;
    m_currentTime = 0.0f;
}

void AnimationPreviewTool::SetPlaying(bool playing) {
    m_playing = playing;
}

void AnimationPreviewTool::SetPlaybackSpeed(float speed) {
    m_speed = speed;
}

void AnimationPreviewTool::Scrub(float time) {
    m_currentTime = std::max(0.0f, time);
}

void AnimationPreviewTool::SetBlendWeight(float weight) {
    m_blendWeight = std::max(0.0f, std::min(1.0f, weight));
}

void AnimationPreviewTool::Reset() {
    m_clip.clear();
    m_playing = false;
    m_speed = 1.0f;
    m_currentTime = 0.0f;
    m_blendWeight = 1.0f;
}

} // namespace atlas::tools
