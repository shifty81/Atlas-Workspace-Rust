#pragma once
// NF::Editor — Camera path editor for cinematics
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

enum class CameraPathInterp : uint8_t {
    Linear, Bezier, CatmullRom, Hermite, Step
};

inline const char* cameraPathInterpName(CameraPathInterp i) {
    switch (i) {
        case CameraPathInterp::Linear:    return "Linear";
        case CameraPathInterp::Bezier:    return "Bezier";
        case CameraPathInterp::CatmullRom:return "CatmullRom";
        case CameraPathInterp::Hermite:   return "Hermite";
        case CameraPathInterp::Step:      return "Step";
    }
    return "Unknown";
}

enum class CameraLookAtMode : uint8_t {
    Free, Target, Path, Orbit, Fixed
};

inline const char* cameraLookAtModeName(CameraLookAtMode m) {
    switch (m) {
        case CameraLookAtMode::Free:   return "Free";
        case CameraLookAtMode::Target: return "Target";
        case CameraLookAtMode::Path:   return "Path";
        case CameraLookAtMode::Orbit:  return "Orbit";
        case CameraLookAtMode::Fixed:  return "Fixed";
    }
    return "Unknown";
}

enum class CameraFOVCurve : uint8_t {
    Constant, Linear, EaseIn, EaseOut, Custom
};

inline const char* cameraFOVCurveName(CameraFOVCurve c) {
    switch (c) {
        case CameraFOVCurve::Constant: return "Constant";
        case CameraFOVCurve::Linear:   return "Linear";
        case CameraFOVCurve::EaseIn:   return "EaseIn";
        case CameraFOVCurve::EaseOut:  return "EaseOut";
        case CameraFOVCurve::Custom:   return "Custom";
    }
    return "Unknown";
}

class CameraKeyframe {
public:
    explicit CameraKeyframe(float time) : m_time(time) {}

    void setInterp(CameraPathInterp i) { m_interp = i;    }
    void setFOV(float fov)             { m_fov    = fov;  }
    void setRoll(float roll)           { m_roll   = roll; }

    [[nodiscard]] float              time()   const { return m_time;   }
    [[nodiscard]] CameraPathInterp   interp() const { return m_interp; }
    [[nodiscard]] float              fov()    const { return m_fov;    }
    [[nodiscard]] float              roll()   const { return m_roll;   }

private:
    float            m_time;
    CameraPathInterp m_interp = CameraPathInterp::CatmullRom;
    float            m_fov    = 60.0f;
    float            m_roll   = 0.0f;
};

class CameraPathEditor {
public:
    static constexpr size_t MAX_KEYFRAMES = 512;

    [[nodiscard]] bool addKeyframe(const CameraKeyframe& kf) {
        if (m_keyframes.size() >= MAX_KEYFRAMES) return false;
        for (auto& k : m_keyframes) if (k.time() == kf.time()) return false;
        m_keyframes.push_back(kf);
        return true;
    }

    [[nodiscard]] bool removeKeyframe(float time) {
        for (auto it = m_keyframes.begin(); it != m_keyframes.end(); ++it) {
            if (it->time() == time) { m_keyframes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] CameraKeyframe* findKeyframe(float time) {
        for (auto& k : m_keyframes) if (k.time() == time) return &k;
        return nullptr;
    }

    void setLookAtMode(CameraLookAtMode m)  { m_lookAtMode = m; }
    void setFOVCurve(CameraFOVCurve c)      { m_fovCurve   = c; }
    void setLooped(bool v)                  { m_looped     = v; }
    void setPreviewEnabled(bool v)          { m_preview    = v; }

    [[nodiscard]] CameraLookAtMode lookAtMode()       const { return m_lookAtMode; }
    [[nodiscard]] CameraFOVCurve   fovCurve()         const { return m_fovCurve;   }
    [[nodiscard]] bool             isLooped()         const { return m_looped;     }
    [[nodiscard]] bool             isPreviewEnabled() const { return m_preview;    }
    [[nodiscard]] size_t           keyframeCount()    const { return m_keyframes.size(); }

    [[nodiscard]] size_t countByInterp(CameraPathInterp i) const {
        size_t c = 0; for (auto& k : m_keyframes) if (k.interp() == i) ++c; return c;
    }

private:
    std::vector<CameraKeyframe> m_keyframes;
    CameraLookAtMode            m_lookAtMode = CameraLookAtMode::Free;
    CameraFOVCurve              m_fovCurve   = CameraFOVCurve::Constant;
    bool                        m_looped     = false;
    bool                        m_preview    = false;
};

} // namespace NF
