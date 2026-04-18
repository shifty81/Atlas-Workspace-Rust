#pragma once
// NF::Editor — Input profile editor v1: input profile and device preset management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ipev1DeviceType    : uint8_t { Keyboard, Mouse, Gamepad, Touch, Joystick, Custom };
enum class Ipev1ProfileState  : uint8_t { Inactive, Active, Locked, Deprecated };

inline const char* ipev1DeviceTypeName(Ipev1DeviceType t) {
    switch (t) {
        case Ipev1DeviceType::Keyboard: return "Keyboard";
        case Ipev1DeviceType::Mouse:    return "Mouse";
        case Ipev1DeviceType::Gamepad:  return "Gamepad";
        case Ipev1DeviceType::Touch:    return "Touch";
        case Ipev1DeviceType::Joystick: return "Joystick";
        case Ipev1DeviceType::Custom:   return "Custom";
    }
    return "Unknown";
}

inline const char* ipev1ProfileStateName(Ipev1ProfileState s) {
    switch (s) {
        case Ipev1ProfileState::Inactive:   return "Inactive";
        case Ipev1ProfileState::Active:     return "Active";
        case Ipev1ProfileState::Locked:     return "Locked";
        case Ipev1ProfileState::Deprecated: return "Deprecated";
    }
    return "Unknown";
}

struct Ipev1Profile {
    uint64_t            id         = 0;
    std::string         name;
    Ipev1DeviceType     deviceType = Ipev1DeviceType::Keyboard;
    Ipev1ProfileState   state      = Ipev1ProfileState::Inactive;
    uint32_t            version    = 1;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()     const { return state == Ipev1ProfileState::Active; }
    [[nodiscard]] bool isLocked()     const { return state == Ipev1ProfileState::Locked; }
    [[nodiscard]] bool isDeprecated() const { return state == Ipev1ProfileState::Deprecated; }
};

struct Ipev1Preset {
    uint64_t    id        = 0;
    std::string name;
    uint64_t    profileId = 0;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty() && profileId != 0; }
};

using Ipev1ChangeCallback = std::function<void(uint64_t)>;

class InputProfileEditorV1 {
public:
    static constexpr size_t MAX_PROFILES = 128;
    static constexpr size_t MAX_PRESETS  = 512;

    bool addProfile(const Ipev1Profile& profile) {
        if (!profile.isValid()) return false;
        for (const auto& p : m_profiles) if (p.id == profile.id) return false;
        if (m_profiles.size() >= MAX_PROFILES) return false;
        m_profiles.push_back(profile);
        if (m_onChange) m_onChange(profile.id);
        return true;
    }

    bool removeProfile(uint64_t id) {
        for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
            if (it->id == id) { m_profiles.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ipev1Profile* findProfile(uint64_t id) {
        for (auto& p : m_profiles) if (p.id == id) return &p;
        return nullptr;
    }

    bool addPreset(const Ipev1Preset& preset) {
        if (!preset.isValid()) return false;
        for (const auto& p : m_presets) if (p.id == preset.id) return false;
        if (m_presets.size() >= MAX_PRESETS) return false;
        m_presets.push_back(preset);
        if (m_onChange) m_onChange(preset.profileId);
        return true;
    }

    bool removePreset(uint64_t id) {
        for (auto it = m_presets.begin(); it != m_presets.end(); ++it) {
            if (it->id == id) { m_presets.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t profileCount() const { return m_profiles.size(); }
    [[nodiscard]] size_t presetCount()  const { return m_presets.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& p : m_profiles) if (p.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& p : m_profiles) if (p.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByDevice(Ipev1DeviceType device) const {
        size_t c = 0; for (const auto& p : m_profiles) if (p.deviceType == device) ++c; return c;
    }
    [[nodiscard]] size_t presetsForProfile(uint64_t profileId) const {
        size_t c = 0; for (const auto& p : m_presets) if (p.profileId == profileId) ++c; return c;
    }

    void setOnChange(Ipev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ipev1Profile> m_profiles;
    std::vector<Ipev1Preset>  m_presets;
    Ipev1ChangeCallback       m_onChange;
};

} // namespace NF
