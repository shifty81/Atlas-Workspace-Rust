#pragma once
// NF::Animation — Skeleton, blend tree, state machine, IK
#include "NF/Core/Core.h"
#include <map>

namespace NF {

// ── Bone / Joint ─────────────────────────────────────────────────

struct Bone {
    std::string name;
    int32_t parentIndex = -1; // -1 = root
    Transform localBindPose;
};

// ── Skeleton ─────────────────────────────────────────────────────

class Skeleton {
public:
    Skeleton() = default;
    explicit Skeleton(std::string name) : m_name(std::move(name)) {}

    int32_t addBone(const std::string& boneName, int32_t parentIndex,
                     const Transform& bindPose = {}) {
        int32_t index = static_cast<int32_t>(m_bones.size());
        m_bones.push_back({boneName, parentIndex, bindPose});
        m_boneNames[boneName] = index;
        return index;
    }

    [[nodiscard]] int32_t findBone(const std::string& boneName) const {
        auto it = m_boneNames.find(boneName);
        return (it != m_boneNames.end()) ? it->second : -1;
    }

    [[nodiscard]] const Bone& bone(int32_t index) const { return m_bones[index]; }
    [[nodiscard]] size_t boneCount() const { return m_bones.size(); }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::vector<Bone>& bones() const { return m_bones; }

private:
    std::string m_name;
    std::vector<Bone> m_bones;
    std::unordered_map<std::string, int32_t> m_boneNames;
};

// ── Animation Keyframe ───────────────────────────────────────────

struct TransformKey {
    float time = 0.f;
    Transform value;
};

struct AnimationChannel {
    int32_t boneIndex = -1;
    std::vector<TransformKey> keys;

    Transform sample(float time) const {
        if (keys.empty()) return {};
        if (keys.size() == 1 || time <= keys.front().time) return keys.front().value;
        if (time >= keys.back().time) return keys.back().value;

        // Find surrounding keyframes
        for (size_t i = 0; i + 1 < keys.size(); ++i) {
            if (time >= keys[i].time && time < keys[i + 1].time) {
                float t = (time - keys[i].time) / (keys[i + 1].time - keys[i].time);
                Transform result;
                result.position = Vec3::lerp(keys[i].value.position,
                                              keys[i + 1].value.position, t);
                result.rotation = Quat::slerp(keys[i].value.rotation,
                                               keys[i + 1].value.rotation, t);
                result.scale = Vec3::lerp(keys[i].value.scale,
                                           keys[i + 1].value.scale, t);
                return result;
            }
        }
        return keys.back().value;
    }
};

// ── Animation Clip ───────────────────────────────────────────────

class AnimationClip {
public:
    AnimationClip() = default;
    AnimationClip(std::string name, float duration)
        : m_name(std::move(name)), m_duration(duration) {}

    AnimationChannel& addChannel(int32_t boneIndex) {
        m_channels.push_back({boneIndex, {}});
        return m_channels.back();
    }

    void sample(float time, std::vector<Transform>& outPose) const {
        for (auto& channel : m_channels) {
            if (channel.boneIndex >= 0 &&
                channel.boneIndex < static_cast<int32_t>(outPose.size())) {
                outPose[channel.boneIndex] = channel.sample(time);
            }
        }
    }

    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] float duration() const { return m_duration; }
    [[nodiscard]] const std::vector<AnimationChannel>& channels() const { return m_channels; }
    [[nodiscard]] size_t channelCount() const { return m_channels.size(); }

private:
    std::string m_name;
    float m_duration = 0.f;
    std::vector<AnimationChannel> m_channels;
};

// ── Animation State Machine ──────────────────────────────────────

struct AnimationTransition {
    std::string targetState;
    std::function<bool()> condition;
    float blendDuration = 0.2f;
};

struct AnimationStateNode {
    std::string name;
    const AnimationClip* clip = nullptr;
    bool looping = true;
    float speed = 1.f;
    std::vector<AnimationTransition> transitions;
};

class AnimationStateMachine {
public:
    void addState(const std::string& name, const AnimationClip* clip,
                   bool looping = true, float speed = 1.f) {
        m_states[name] = {name, clip, looping, speed, {}};
        if (m_currentState.empty()) m_currentState = name;
    }

    void addTransition(const std::string& from, const std::string& to,
                        std::function<bool()> condition, float blendDuration = 0.2f) {
        auto it = m_states.find(from);
        if (it != m_states.end()) {
            it->second.transitions.push_back({to, std::move(condition), blendDuration});
        }
    }

    void update(float dt) {
        auto it = m_states.find(m_currentState);
        if (it == m_states.end()) return;

        auto& state = it->second;
        m_stateTime += dt * state.speed;

        if (state.clip && m_stateTime >= state.clip->duration()) {
            if (state.looping) {
                m_stateTime = 0.f;
            }
        }

        // Check transitions
        for (auto& transition : state.transitions) {
            if (transition.condition && transition.condition()) {
                m_currentState = transition.targetState;
                m_stateTime = 0.f;
                break;
            }
        }
    }

    [[nodiscard]] const std::string& currentState() const { return m_currentState; }
    [[nodiscard]] float stateTime() const { return m_stateTime; }
    [[nodiscard]] size_t stateCount() const { return m_states.size(); }

    [[nodiscard]] const AnimationClip* currentClip() const {
        auto it = m_states.find(m_currentState);
        return (it != m_states.end()) ? it->second.clip : nullptr;
    }

    void sampleCurrentPose(std::vector<Transform>& outPose) const {
        auto it = m_states.find(m_currentState);
        if (it != m_states.end() && it->second.clip) {
            it->second.clip->sample(m_stateTime, outPose);
        }
    }

private:
    std::unordered_map<std::string, AnimationStateNode> m_states;
    std::string m_currentState;
    float m_stateTime = 0.f;
};

// ── Animation System ─────────────────────────────────────────────

class AnimationSystem {
public:
    void init() { NF_LOG_INFO("Animation", "Animation system initialized"); }

    void shutdown() {
        NF_LOG_INFO("Animation", "Animation system shutdown");
    }

    void update(float dt) {
        (void)dt;
        // In a real implementation, this would iterate registered
        // state machines and update animations for all entities
    }
};

// ── S5 Character & Animation Suite ────────────────────────────────

/// A chain of three bones for IK solving (e.g., shoulder → elbow → hand).
struct BoneChain {
    int32_t rootBone = -1;   // e.g., upper arm / thigh
    int32_t midBone  = -1;   // e.g., elbow / knee
    int32_t endBone  = -1;   // e.g., hand / foot

    [[nodiscard]] bool isValid() const {
        return rootBone >= 0 && midBone >= 0 && endBone >= 0;
    }
};

/// Analytical Two-Joint IK solver.
/// Given a 3-bone chain and a target position, computes the mid-joint bend
/// using the law of cosines and an optional pole vector to orient the plane.
class TwoJointIK {
public:
    /// Solve IK for a two-joint chain. Updates outPose transforms in-place.
    /// Returns true if a valid solution was found.
    bool solve(const BoneChain& chain, const Vec3& target, const Vec3& poleTarget,
               std::vector<Transform>& outPose) const {
        if (!chain.isValid()) return false;
        if (chain.rootBone >= static_cast<int32_t>(outPose.size()) ||
            chain.midBone  >= static_cast<int32_t>(outPose.size()) ||
            chain.endBone  >= static_cast<int32_t>(outPose.size()))
            return false;

        Vec3 rootPos = outPose[chain.rootBone].position;
        Vec3 midPos  = outPose[chain.midBone].position;
        Vec3 endPos  = outPose[chain.endBone].position;

        float upperLen = (midPos - rootPos).length();
        float lowerLen = (endPos - midPos).length();
        float targetDist = (target - rootPos).length();

        // Clamp target distance to reachable range
        float maxReach = upperLen + lowerLen;
        float minReach = std::abs(upperLen - lowerLen);
        targetDist = std::clamp(targetDist, minReach + 0.001f, maxReach - 0.001f);

        // Law of cosines to find the angle at the mid joint
        float cosAngle = (upperLen * upperLen + lowerLen * lowerLen - targetDist * targetDist)
                         / (2.f * upperLen * lowerLen);
        cosAngle = std::clamp(cosAngle, -1.f, 1.f);

        // Direction from root to target
        Vec3 toTarget = target - rootPos;
        float toTargetLen = toTarget.length();
        if (toTargetLen < 0.0001f) return false;
        Vec3 toTargetDir = toTarget * (1.f / toTargetLen);

        // Use pole vector to determine the bend plane
        Vec3 toPole = poleTarget - rootPos;
        // Project pole onto plane perpendicular to toTargetDir
        Vec3 poleOnPlane = toPole - toTargetDir * toPole.dot(toTargetDir);
        float poleLen = poleOnPlane.length();
        Vec3 bendDir = (poleLen > 0.0001f)
            ? poleOnPlane * (1.f / poleLen)
            : Vec3{0.f, 1.f, 0.f};

        // Position the mid bone
        float cosRoot = (upperLen * upperLen + targetDist * targetDist - lowerLen * lowerLen)
                        / (2.f * upperLen * targetDist);
        cosRoot = std::clamp(cosRoot, -1.f, 1.f);
        float sinRoot = std::sqrt(1.f - cosRoot * cosRoot);

        Vec3 newMidPos = rootPos + toTargetDir * (upperLen * cosRoot)
                                 + bendDir * (upperLen * sinRoot);

        // Update poses
        outPose[chain.midBone].position = newMidPos;
        outPose[chain.endBone].position = target;

        ++m_solveCount;
        return true;
    }

    [[nodiscard]] size_t solveCount() const { return m_solveCount; }
    void resetStats() { m_solveCount = 0; }

private:
    mutable size_t m_solveCount = 0;
};

/// Hand anchor for FPS view — which arm/hand to drive.
enum class HandSide : uint8_t { Left = 0, Right = 1 };

/// FPS hand rig — manages first-person hand/arm bone setup
/// and drives posing via IK targets.
class FPSHandRig {
public:
    void setSkeleton(const Skeleton* skel) { m_skeleton = skel; }
    [[nodiscard]] const Skeleton* skeleton() const { return m_skeleton; }

    /// Configure the arm chain for a given hand side.
    void setArmChain(HandSide side, const BoneChain& chain) {
        if (side == HandSide::Left)  m_leftArm = chain;
        else                          m_rightArm = chain;
    }

    [[nodiscard]] const BoneChain& armChain(HandSide side) const {
        return (side == HandSide::Left) ? m_leftArm : m_rightArm;
    }

    /// Set the IK target for a hand.
    void setHandTarget(HandSide side, const Vec3& target) {
        if (side == HandSide::Left)  m_leftTarget = target;
        else                          m_rightTarget = target;
    }

    [[nodiscard]] Vec3 handTarget(HandSide side) const {
        return (side == HandSide::Left) ? m_leftTarget : m_rightTarget;
    }

    /// Set pole target for IK bend direction.
    void setPoleTarget(HandSide side, const Vec3& pole) {
        if (side == HandSide::Left)  m_leftPole = pole;
        else                          m_rightPole = pole;
    }

    /// Apply IK to update the given pose. Returns number of hands solved.
    int applyIK(std::vector<Transform>& pose) const {
        int solved = 0;
        if (m_leftArm.isValid()) {
            if (m_ik.solve(m_leftArm, m_leftTarget, m_leftPole, pose))
                ++solved;
        }
        if (m_rightArm.isValid()) {
            if (m_ik.solve(m_rightArm, m_rightTarget, m_rightPole, pose))
                ++solved;
        }
        return solved;
    }

    /// Weapon sway offset (camera-relative).
    void setSwayOffset(const Vec3& sway) { m_swayOffset = sway; }
    [[nodiscard]] Vec3 swayOffset() const { return m_swayOffset; }

    /// Apply sway to both hand targets (call before applyIK).
    void applySwayToTargets() {
        m_leftTarget  = m_leftTarget + m_swayOffset;
        m_rightTarget = m_rightTarget + m_swayOffset;
    }

    [[nodiscard]] const TwoJointIK& ik() const { return m_ik; }

private:
    const Skeleton* m_skeleton = nullptr;
    BoneChain m_leftArm;
    BoneChain m_rightArm;
    Vec3 m_leftTarget{0.f, 0.f, 0.f};
    Vec3 m_rightTarget{0.f, 0.f, 0.f};
    Vec3 m_leftPole{0.f, 1.f, 0.f};
    Vec3 m_rightPole{0.f, 1.f, 0.f};
    Vec3 m_swayOffset{0.f, 0.f, 0.f};
    TwoJointIK m_ik;
};

/// A node in an animation blend graph.
struct BlendNode {
    std::string name;
    const AnimationClip* clip = nullptr;
    float weight = 1.f;       // blend weight (0–1)
    float timeScale = 1.f;    // playback speed multiplier
    float localTime = 0.f;    // current playback position
    bool  looping = true;
};

/// Weighted pose blending across multiple animation clips.
class AnimationBlendGraph {
public:
    static constexpr int kMaxNodes = 16;

    /// Add a blend node. Returns index, or -1 if at capacity.
    int addNode(const std::string& name, const AnimationClip* clip,
                float weight = 1.f, bool looping = true) {
        if (static_cast<int>(m_nodes.size()) >= kMaxNodes) return -1;
        BlendNode node;
        node.name = name;
        node.clip = clip;
        node.weight = weight;
        node.looping = looping;
        m_nodes.push_back(node);
        return static_cast<int>(m_nodes.size()) - 1;
    }

    /// Set blend weight for a node by index.
    void setWeight(int index, float weight) {
        if (index >= 0 && index < static_cast<int>(m_nodes.size()))
            m_nodes[static_cast<size_t>(index)].weight = std::clamp(weight, 0.f, 1.f);
    }

    /// Update all nodes by dt seconds.
    void update(float dt) {
        for (auto& node : m_nodes) {
            if (!node.clip) continue;
            node.localTime += dt * node.timeScale;
            if (node.looping && node.clip->duration() > 0.f) {
                while (node.localTime >= node.clip->duration())
                    node.localTime -= node.clip->duration();
            } else {
                node.localTime = std::min(node.localTime, node.clip->duration());
            }
        }
    }

    /// Sample and blend all nodes into a single output pose.
    /// Weights are normalized before blending.
    void sample(std::vector<Transform>& outPose) const {
        if (m_nodes.empty() || outPose.empty()) return;

        float totalWeight = 0.f;
        for (auto& n : m_nodes) totalWeight += n.weight;
        if (totalWeight < 0.0001f) return;

        // Zero out pose
        for (auto& t : outPose) {
            t.position = {0.f, 0.f, 0.f};
            t.scale    = {0.f, 0.f, 0.f};
        }

        // Accumulate weighted contributions
        std::vector<Transform> tempPose(outPose.size());
        for (auto& node : m_nodes) {
            if (!node.clip || node.weight <= 0.f) continue;
            float w = node.weight / totalWeight;

            // Reset temp
            for (auto& t : tempPose) t = Transform{};
            node.clip->sample(node.localTime, tempPose);

            for (size_t i = 0; i < outPose.size(); ++i) {
                outPose[i].position = outPose[i].position + tempPose[i].position * w;
                outPose[i].scale    = outPose[i].scale + tempPose[i].scale * w;
                // For rotation, simple weighted accumulation (approximate)
                outPose[i].rotation = Quat::slerp(outPose[i].rotation,
                                                   tempPose[i].rotation, w);
            }
        }
    }

    [[nodiscard]] BlendNode* node(int index) {
        if (index < 0 || index >= static_cast<int>(m_nodes.size())) return nullptr;
        return &m_nodes[static_cast<size_t>(index)];
    }

    [[nodiscard]] const BlendNode* node(int index) const {
        if (index < 0 || index >= static_cast<int>(m_nodes.size())) return nullptr;
        return &m_nodes[static_cast<size_t>(index)];
    }

    [[nodiscard]] int findNode(const std::string& name) const {
        for (size_t i = 0; i < m_nodes.size(); ++i)
            if (m_nodes[i].name == name) return static_cast<int>(i);
        return -1;
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    void clear() { m_nodes.clear(); }

private:
    std::vector<BlendNode> m_nodes;
};

/// Character grounding — adjusts foot placement to match terrain.
class CharacterGroundingSystem {
public:
    /// Set the ground height at the character's position.
    void setGroundHeight(float height) { m_groundHeight = height; }
    [[nodiscard]] float groundHeight() const { return m_groundHeight; }

    /// Set the maximum foot adjustment distance.
    void setMaxAdjustment(float maxAdj) { m_maxAdjustment = std::max(0.f, maxAdj); }
    [[nodiscard]] float maxAdjustment() const { return m_maxAdjustment; }

    /// Set the foot bone indices.
    void setFootBones(int32_t leftFoot, int32_t rightFoot) {
        m_leftFoot = leftFoot;
        m_rightFoot = rightFoot;
    }

    /// Apply grounding offset to the character pose.
    /// Adjusts the vertical position of foot bones to sit at ground height.
    void apply(std::vector<Transform>& pose, float characterY) const {
        float offset = m_groundHeight - characterY;
        offset = std::clamp(offset, -m_maxAdjustment, m_maxAdjustment);

        if (m_leftFoot >= 0 && m_leftFoot < static_cast<int32_t>(pose.size())) {
            pose[static_cast<size_t>(m_leftFoot)].position.y += offset;
        }
        if (m_rightFoot >= 0 && m_rightFoot < static_cast<int32_t>(pose.size())) {
            pose[static_cast<size_t>(m_rightFoot)].position.y += offset;
        }
    }

    /// Check if grounding is needed (character vs ground delta exceeds threshold).
    [[nodiscard]] bool needsGrounding(float characterY, float threshold = 0.01f) const {
        return std::abs(m_groundHeight - characterY) > threshold;
    }

    [[nodiscard]] int32_t leftFootBone() const { return m_leftFoot; }
    [[nodiscard]] int32_t rightFootBone() const { return m_rightFoot; }

private:
    float m_groundHeight   = 0.f;
    float m_maxAdjustment  = 0.5f;
    int32_t m_leftFoot     = -1;
    int32_t m_rightFoot    = -1;
};

} // namespace NF
