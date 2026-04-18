#pragma once
// NF::Editor — Skeleton editor v1: bone and joint management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Skev1BoneType  : uint8_t { Root, Spine, Limb, Finger, Toe, Head, Tail, Helper };
enum class Skev1BoneState : uint8_t { Normal, Selected, Hidden, Locked, Deforming };

inline const char* skev1BoneTypeName(Skev1BoneType t) {
    switch (t) {
        case Skev1BoneType::Root:      return "Root";
        case Skev1BoneType::Spine:     return "Spine";
        case Skev1BoneType::Limb:      return "Limb";
        case Skev1BoneType::Finger:    return "Finger";
        case Skev1BoneType::Toe:       return "Toe";
        case Skev1BoneType::Head:      return "Head";
        case Skev1BoneType::Tail:      return "Tail";
        case Skev1BoneType::Helper:    return "Helper";
    }
    return "Unknown";
}

inline const char* skev1BoneStateName(Skev1BoneState s) {
    switch (s) {
        case Skev1BoneState::Normal:    return "Normal";
        case Skev1BoneState::Selected:  return "Selected";
        case Skev1BoneState::Hidden:    return "Hidden";
        case Skev1BoneState::Locked:    return "Locked";
        case Skev1BoneState::Deforming: return "Deforming";
    }
    return "Unknown";
}

struct Skev1Bone {
    uint64_t        id       = 0;
    std::string     name;
    uint64_t        parentId = 0;
    Skev1BoneType   boneType = Skev1BoneType::Limb;
    Skev1BoneState  state    = Skev1BoneState::Normal;
    float           length   = 1.0f;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isSelected()  const { return state == Skev1BoneState::Selected; }
    [[nodiscard]] bool isHidden()    const { return state == Skev1BoneState::Hidden; }
    [[nodiscard]] bool isDeforming() const { return state == Skev1BoneState::Deforming; }
    [[nodiscard]] bool isRoot()      const { return parentId == 0; }
};

struct Skev1Joint {
    uint64_t    id     = 0;
    uint64_t    boneId = 0;
    std::string name;
    float       limit  = 180.0f;

    [[nodiscard]] bool isValid() const { return id != 0 && boneId != 0 && !name.empty(); }
};

using Skev1ChangeCallback = std::function<void(uint64_t)>;

class SkeletonEditorV1 {
public:
    static constexpr size_t MAX_BONES  = 512;
    static constexpr size_t MAX_JOINTS = 1024;

    bool addBone(const Skev1Bone& bone) {
        if (!bone.isValid()) return false;
        for (const auto& b : m_bones) if (b.id == bone.id) return false;
        if (m_bones.size() >= MAX_BONES) return false;
        m_bones.push_back(bone);
        if (m_onChange) m_onChange(bone.id);
        return true;
    }

    bool removeBone(uint64_t id) {
        for (auto it = m_bones.begin(); it != m_bones.end(); ++it) {
            if (it->id == id) { m_bones.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Skev1Bone* findBone(uint64_t id) {
        for (auto& b : m_bones) if (b.id == id) return &b;
        return nullptr;
    }

    bool addJoint(const Skev1Joint& joint) {
        if (!joint.isValid()) return false;
        for (const auto& j : m_joints) if (j.id == joint.id) return false;
        if (m_joints.size() >= MAX_JOINTS) return false;
        m_joints.push_back(joint);
        if (m_onChange) m_onChange(joint.boneId);
        return true;
    }

    bool removeJoint(uint64_t id) {
        for (auto it = m_joints.begin(); it != m_joints.end(); ++it) {
            if (it->id == id) { m_joints.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t boneCount()  const { return m_bones.size(); }
    [[nodiscard]] size_t jointCount() const { return m_joints.size(); }

    [[nodiscard]] size_t rootBoneCount() const {
        size_t c = 0; for (const auto& b : m_bones) if (b.isRoot()) ++c; return c;
    }
    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (const auto& b : m_bones) if (b.isSelected()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Skev1BoneType type) const {
        size_t c = 0; for (const auto& b : m_bones) if (b.boneType == type) ++c; return c;
    }
    [[nodiscard]] size_t jointsForBone(uint64_t boneId) const {
        size_t c = 0; for (const auto& j : m_joints) if (j.boneId == boneId) ++c; return c;
    }

    void setOnChange(Skev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Skev1Bone>  m_bones;
    std::vector<Skev1Joint> m_joints;
    Skev1ChangeCallback     m_onChange;
};

} // namespace NF
