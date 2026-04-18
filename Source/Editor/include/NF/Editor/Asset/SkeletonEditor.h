#pragma once
// NF::Editor — Skeleton editor
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

enum class BoneRotationOrder : uint8_t {
    XYZ, XZY, YXZ, YZX, ZXY, ZYX
};

inline const char* boneRotationOrderName(BoneRotationOrder o) {
    switch (o) {
        case BoneRotationOrder::XYZ: return "XYZ";
        case BoneRotationOrder::XZY: return "XZY";
        case BoneRotationOrder::YXZ: return "YXZ";
        case BoneRotationOrder::YZX: return "YZX";
        case BoneRotationOrder::ZXY: return "ZXY";
        case BoneRotationOrder::ZYX: return "ZYX";
    }
    return "Unknown";
}

enum class BoneConstraintType : uint8_t {
    None, PointAt, Orient, Position, Scale, IK, LookAt
};

inline const char* boneConstraintTypeName(BoneConstraintType t) {
    switch (t) {
        case BoneConstraintType::None:     return "None";
        case BoneConstraintType::PointAt:  return "PointAt";
        case BoneConstraintType::Orient:   return "Orient";
        case BoneConstraintType::Position: return "Position";
        case BoneConstraintType::Scale:    return "Scale";
        case BoneConstraintType::IK:       return "IK";
        case BoneConstraintType::LookAt:   return "LookAt";
    }
    return "Unknown";
}

enum class SkeletonRetargetMode : uint8_t {
    None, Humanoid, Generic, Custom
};

inline const char* skeletonRetargetModeName(SkeletonRetargetMode m) {
    switch (m) {
        case SkeletonRetargetMode::None:     return "None";
        case SkeletonRetargetMode::Humanoid: return "Humanoid";
        case SkeletonRetargetMode::Generic:  return "Generic";
        case SkeletonRetargetMode::Custom:   return "Custom";
    }
    return "Unknown";
}

class BoneDefinition {
public:
    explicit BoneDefinition(uint32_t id, const std::string& name)
        : m_id(id), m_name(name) {}

    void setParentId(uint32_t pid)             { m_parentId   = pid; m_hasParent = true; }
    void setRotationOrder(BoneRotationOrder o) { m_rotOrder   = o; }
    void setConstraint(BoneConstraintType c)   { m_constraint = c; }
    void setLength(float l)                    { m_length     = l; }
    void setSelected(bool v)                   { m_selected   = v; }

    [[nodiscard]] uint32_t             id()            const { return m_id;         }
    [[nodiscard]] const std::string&   name()          const { return m_name;       }
    [[nodiscard]] uint32_t             parentId()      const { return m_parentId;   }
    [[nodiscard]] bool                 hasParent()     const { return m_hasParent;  }
    [[nodiscard]] BoneRotationOrder    rotationOrder() const { return m_rotOrder;   }
    [[nodiscard]] BoneConstraintType   constraint()    const { return m_constraint; }
    [[nodiscard]] float                length()        const { return m_length;     }
    [[nodiscard]] bool                 isSelected()    const { return m_selected;   }

private:
    uint32_t            m_id;
    std::string         m_name;
    uint32_t            m_parentId   = 0;
    BoneRotationOrder   m_rotOrder   = BoneRotationOrder::XYZ;
    BoneConstraintType  m_constraint = BoneConstraintType::None;
    float               m_length     = 1.0f;
    bool                m_hasParent  = false;
    bool                m_selected   = false;
};

class SkeletonEditor {
public:
    void setRetargetMode(SkeletonRetargetMode m) { m_retargetMode = m; }
    void setName(const std::string& n)           { m_name = n; }

    [[nodiscard]] bool addBone(const BoneDefinition& bone) {
        for (auto& b : m_bones) if (b.id() == bone.id()) return false;
        m_bones.push_back(bone);
        return true;
    }

    [[nodiscard]] bool removeBone(uint32_t id) {
        for (auto it = m_bones.begin(); it != m_bones.end(); ++it) {
            if (it->id() == id) { m_bones.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] BoneDefinition* findBone(uint32_t id) {
        for (auto& b : m_bones) if (b.id() == id) return &b;
        return nullptr;
    }

    [[nodiscard]] const std::string&     name()          const { return m_name;         }
    [[nodiscard]] SkeletonRetargetMode   retargetMode()  const { return m_retargetMode; }
    [[nodiscard]] size_t                 boneCount()     const { return m_bones.size(); }

    [[nodiscard]] size_t rootBoneCount() const {
        size_t c = 0; for (auto& b : m_bones) if (!b.hasParent()) ++c; return c;
    }
    [[nodiscard]] size_t selectedBoneCount() const {
        size_t c = 0; for (auto& b : m_bones) if (b.isSelected()) ++c; return c;
    }
    [[nodiscard]] size_t constrainedBoneCount() const {
        size_t c = 0; for (auto& b : m_bones) if (b.constraint() != BoneConstraintType::None) ++c; return c;
    }
    [[nodiscard]] size_t countByConstraint(BoneConstraintType t) const {
        size_t c = 0; for (auto& b : m_bones) if (b.constraint() == t) ++c; return c;
    }

private:
    std::string              m_name;
    SkeletonRetargetMode     m_retargetMode = SkeletonRetargetMode::None;
    std::vector<BoneDefinition> m_bones;
};

} // namespace NF
