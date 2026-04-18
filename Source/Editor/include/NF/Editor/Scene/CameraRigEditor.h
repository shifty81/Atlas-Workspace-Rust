#pragma once
// NF::Editor — virtual camera rig management editor
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

enum class CamRigType : uint8_t { Fixed, Orbital, Follow, Dolly, FreeFly };
inline const char* camRigTypeName(CamRigType v) {
    switch (v) {
        case CamRigType::Fixed:   return "Fixed";
        case CamRigType::Orbital: return "Orbital";
        case CamRigType::Follow:  return "Follow";
        case CamRigType::Dolly:   return "Dolly";
        case CamRigType::FreeFly: return "FreeFly";
    }
    return "Unknown";
}

enum class CamRigConstraint : uint8_t { None, LookAt, Follow, Rail, Spline };
inline const char* camRigConstraintName(CamRigConstraint v) {
    switch (v) {
        case CamRigConstraint::None:   return "None";
        case CamRigConstraint::LookAt: return "LookAt";
        case CamRigConstraint::Follow: return "Follow";
        case CamRigConstraint::Rail:   return "Rail";
        case CamRigConstraint::Spline: return "Spline";
    }
    return "Unknown";
}

class CameraRig {
public:
    explicit CameraRig(uint32_t id, const std::string& name,
                        CamRigType type, CamRigConstraint constraint)
        : m_id(id), m_name(name), m_type(type), m_constraint(constraint) {}

    void setFov(float v)       { m_fov       = v; }
    void setNearClip(float v)  { m_nearClip  = v; }
    void setFarClip(float v)   { m_farClip   = v; }
    void setIsEnabled(bool v)  { m_isEnabled = v; }

    [[nodiscard]] uint32_t           id()         const { return m_id;         }
    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] CamRigType         type()       const { return m_type;       }
    [[nodiscard]] CamRigConstraint   constraint() const { return m_constraint; }
    [[nodiscard]] float              fov()        const { return m_fov;        }
    [[nodiscard]] float              nearClip()   const { return m_nearClip;   }
    [[nodiscard]] float              farClip()    const { return m_farClip;    }
    [[nodiscard]] bool               isEnabled()  const { return m_isEnabled;  }

private:
    uint32_t        m_id;
    std::string     m_name;
    CamRigType      m_type;
    CamRigConstraint m_constraint;
    float           m_fov       = 60.0f;
    float           m_nearClip  = 0.1f;
    float           m_farClip   = 1000.0f;
    bool            m_isEnabled = true;
};

class CameraRigEditor {
public:
    void setIsShowDisabled(bool v) { m_isShowDisabled = v; }
    void setIsGroupByType(bool v)  { m_isGroupByType  = v; }
    void setDefaultFov(float v)    { m_defaultFov     = v; }

    bool addRig(const CameraRig& r) {
        for (auto& x : m_rigs) if (x.id() == r.id()) return false;
        m_rigs.push_back(r); return true;
    }
    bool removeRig(uint32_t id) {
        auto it = std::find_if(m_rigs.begin(), m_rigs.end(),
            [&](const CameraRig& r){ return r.id() == id; });
        if (it == m_rigs.end()) return false;
        m_rigs.erase(it); return true;
    }
    [[nodiscard]] CameraRig* findRig(uint32_t id) {
        for (auto& r : m_rigs) if (r.id() == id) return &r;
        return nullptr;
    }

    [[nodiscard]] bool  isShowDisabled() const { return m_isShowDisabled; }
    [[nodiscard]] bool  isGroupByType()  const { return m_isGroupByType;  }
    [[nodiscard]] float defaultFov()     const { return m_defaultFov;     }
    [[nodiscard]] size_t rigCount()      const { return m_rigs.size();    }

    [[nodiscard]] size_t countByType(CamRigType t) const {
        size_t n = 0; for (auto& r : m_rigs) if (r.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByConstraint(CamRigConstraint c) const {
        size_t n = 0; for (auto& r : m_rigs) if (r.constraint() == c) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& r : m_rigs) if (r.isEnabled()) ++n; return n;
    }

private:
    std::vector<CameraRig> m_rigs;
    bool  m_isShowDisabled = false;
    bool  m_isGroupByType  = true;
    float m_defaultFov     = 75.0f;
};

} // namespace NF
