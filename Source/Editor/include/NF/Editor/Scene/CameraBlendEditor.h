#pragma once
// NF::Editor — camera transition/blend management editor
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

enum class CamBlendCurve : uint8_t { Linear, EaseIn, EaseOut, EaseInOut, Custom };
inline const char* camBlendCurveName(CamBlendCurve v) {
    switch (v) {
        case CamBlendCurve::Linear:    return "Linear";
        case CamBlendCurve::EaseIn:    return "EaseIn";
        case CamBlendCurve::EaseOut:   return "EaseOut";
        case CamBlendCurve::EaseInOut: return "EaseInOut";
        case CamBlendCurve::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class CamBlendTrigger : uint8_t { Immediate, OnEnter, OnExit, OnDistance, Manual };
inline const char* camBlendTriggerName(CamBlendTrigger v) {
    switch (v) {
        case CamBlendTrigger::Immediate:  return "Immediate";
        case CamBlendTrigger::OnEnter:    return "OnEnter";
        case CamBlendTrigger::OnExit:     return "OnExit";
        case CamBlendTrigger::OnDistance: return "OnDistance";
        case CamBlendTrigger::Manual:     return "Manual";
    }
    return "Unknown";
}

class CameraBlend {
public:
    explicit CameraBlend(uint32_t id, const std::string& name,
                          CamBlendCurve curve, CamBlendTrigger trigger)
        : m_id(id), m_name(name), m_curve(curve), m_trigger(trigger) {}

    void setDuration(float v)     { m_duration    = v; }
    void setBlendWeight(float v)  { m_blendWeight = v; }
    void setIsEnabled(bool v)     { m_isEnabled   = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] CamBlendCurve      curve()       const { return m_curve;       }
    [[nodiscard]] CamBlendTrigger    trigger()     const { return m_trigger;     }
    [[nodiscard]] float              duration()    const { return m_duration;    }
    [[nodiscard]] float              blendWeight() const { return m_blendWeight; }
    [[nodiscard]] bool               isEnabled()  const { return m_isEnabled;   }

private:
    uint32_t        m_id;
    std::string     m_name;
    CamBlendCurve   m_curve;
    CamBlendTrigger m_trigger;
    float           m_duration    = 1.0f;
    float           m_blendWeight = 1.0f;
    bool            m_isEnabled   = true;
};

class CameraBlendEditor {
public:
    void setIsShowDisabled(bool v)  { m_isShowDisabled  = v; }
    void setIsGroupByCurve(bool v)  { m_isGroupByCurve  = v; }
    void setDefaultDuration(float v){ m_defaultDuration = v; }

    bool addBlend(const CameraBlend& b) {
        for (auto& x : m_blends) if (x.id() == b.id()) return false;
        m_blends.push_back(b); return true;
    }
    bool removeBlend(uint32_t id) {
        auto it = std::find_if(m_blends.begin(), m_blends.end(),
            [&](const CameraBlend& b){ return b.id() == id; });
        if (it == m_blends.end()) return false;
        m_blends.erase(it); return true;
    }
    [[nodiscard]] CameraBlend* findBlend(uint32_t id) {
        for (auto& b : m_blends) if (b.id() == id) return &b;
        return nullptr;
    }

    [[nodiscard]] bool  isShowDisabled()  const { return m_isShowDisabled;  }
    [[nodiscard]] bool  isGroupByCurve()  const { return m_isGroupByCurve;  }
    [[nodiscard]] float defaultDuration() const { return m_defaultDuration; }
    [[nodiscard]] size_t blendCount()     const { return m_blends.size();   }

    [[nodiscard]] size_t countByCurve(CamBlendCurve c) const {
        size_t n = 0; for (auto& b : m_blends) if (b.curve() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByTrigger(CamBlendTrigger t) const {
        size_t n = 0; for (auto& b : m_blends) if (b.trigger() == t) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& b : m_blends) if (b.isEnabled()) ++n; return n;
    }

private:
    std::vector<CameraBlend> m_blends;
    bool  m_isShowDisabled  = false;
    bool  m_isGroupByCurve  = false;
    float m_defaultDuration = 0.5f;
};

} // namespace NF
