#pragma once
// NF::Editor — camera shake preset management editor
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

enum class CamShakeProfile : uint8_t { Explosion, Earthquake, Impact, Rumble, Custom };
inline const char* camShakeProfileName(CamShakeProfile v) {
    switch (v) {
        case CamShakeProfile::Explosion:  return "Explosion";
        case CamShakeProfile::Earthquake: return "Earthquake";
        case CamShakeProfile::Impact:     return "Impact";
        case CamShakeProfile::Rumble:     return "Rumble";
        case CamShakeProfile::Custom:     return "Custom";
    }
    return "Unknown";
}

enum class CamShakeAxis : uint8_t { X, Y, Z, XY, XYZ };
inline const char* camShakeAxisName(CamShakeAxis v) {
    switch (v) {
        case CamShakeAxis::X:   return "X";
        case CamShakeAxis::Y:   return "Y";
        case CamShakeAxis::Z:   return "Z";
        case CamShakeAxis::XY:  return "XY";
        case CamShakeAxis::XYZ: return "XYZ";
    }
    return "Unknown";
}

class CameraShake {
public:
    explicit CameraShake(uint32_t id, const std::string& name,
                          CamShakeProfile profile, CamShakeAxis axis)
        : m_id(id), m_name(name), m_profile(profile), m_axis(axis) {}

    void setAmplitude(float v)  { m_amplitude = v; }
    void setFrequency(float v)  { m_frequency = v; }
    void setDecayTime(float v)  { m_decayTime = v; }
    void setIsEnabled(bool v)   { m_isEnabled = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] CamShakeProfile    profile()   const { return m_profile;   }
    [[nodiscard]] CamShakeAxis       axis()      const { return m_axis;      }
    [[nodiscard]] float              amplitude() const { return m_amplitude; }
    [[nodiscard]] float              frequency() const { return m_frequency; }
    [[nodiscard]] float              decayTime() const { return m_decayTime; }
    [[nodiscard]] bool               isEnabled() const { return m_isEnabled; }

private:
    uint32_t       m_id;
    std::string    m_name;
    CamShakeProfile m_profile;
    CamShakeAxis   m_axis;
    float          m_amplitude = 1.0f;
    float          m_frequency = 10.0f;
    float          m_decayTime = 0.5f;
    bool           m_isEnabled = true;
};

class CameraShakeEditor {
public:
    void setIsShowDisabled(bool v)    { m_isShowDisabled    = v; }
    void setIsGroupByProfile(bool v)  { m_isGroupByProfile  = v; }
    void setDefaultAmplitude(float v) { m_defaultAmplitude  = v; }

    bool addShake(const CameraShake& s) {
        for (auto& x : m_shakes) if (x.id() == s.id()) return false;
        m_shakes.push_back(s); return true;
    }
    bool removeShake(uint32_t id) {
        auto it = std::find_if(m_shakes.begin(), m_shakes.end(),
            [&](const CameraShake& s){ return s.id() == id; });
        if (it == m_shakes.end()) return false;
        m_shakes.erase(it); return true;
    }
    [[nodiscard]] CameraShake* findShake(uint32_t id) {
        for (auto& s : m_shakes) if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] bool  isShowDisabled()   const { return m_isShowDisabled;   }
    [[nodiscard]] bool  isGroupByProfile() const { return m_isGroupByProfile; }
    [[nodiscard]] float defaultAmplitude() const { return m_defaultAmplitude; }
    [[nodiscard]] size_t shakeCount()      const { return m_shakes.size();    }

    [[nodiscard]] size_t countByProfile(CamShakeProfile p) const {
        size_t n = 0; for (auto& s : m_shakes) if (s.profile() == p) ++n; return n;
    }
    [[nodiscard]] size_t countByAxis(CamShakeAxis a) const {
        size_t n = 0; for (auto& s : m_shakes) if (s.axis() == a) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& s : m_shakes) if (s.isEnabled()) ++n; return n;
    }

private:
    std::vector<CameraShake> m_shakes;
    bool  m_isShowDisabled   = false;
    bool  m_isGroupByProfile = true;
    float m_defaultAmplitude = 0.5f;
};

} // namespace NF
