#pragma once
// NF::Editor — Gamepad mapper v1: button mapping and profile management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Gpmv1ButtonType  : uint8_t { FaceA, FaceB, FaceX, FaceY, LB, RB, LT, RT, Start, Select, LS, RS, DPadUp, DPadDown, DPadLeft, DPadRight };
enum class Gpmv1BindingMode : uint8_t { None, Press, Hold, Release, Toggle };

inline const char* gpmv1ButtonTypeName(Gpmv1ButtonType t) {
    switch (t) {
        case Gpmv1ButtonType::FaceA:     return "FaceA";
        case Gpmv1ButtonType::FaceB:     return "FaceB";
        case Gpmv1ButtonType::FaceX:     return "FaceX";
        case Gpmv1ButtonType::FaceY:     return "FaceY";
        case Gpmv1ButtonType::LB:        return "LB";
        case Gpmv1ButtonType::RB:        return "RB";
        case Gpmv1ButtonType::LT:        return "LT";
        case Gpmv1ButtonType::RT:        return "RT";
        case Gpmv1ButtonType::Start:     return "Start";
        case Gpmv1ButtonType::Select:    return "Select";
        case Gpmv1ButtonType::LS:        return "LS";
        case Gpmv1ButtonType::RS:        return "RS";
        case Gpmv1ButtonType::DPadUp:    return "DPadUp";
        case Gpmv1ButtonType::DPadDown:  return "DPadDown";
        case Gpmv1ButtonType::DPadLeft:  return "DPadLeft";
        case Gpmv1ButtonType::DPadRight: return "DPadRight";
    }
    return "Unknown";
}

inline const char* gpmv1BindingModeName(Gpmv1BindingMode m) {
    switch (m) {
        case Gpmv1BindingMode::None:    return "None";
        case Gpmv1BindingMode::Press:   return "Press";
        case Gpmv1BindingMode::Hold:    return "Hold";
        case Gpmv1BindingMode::Release: return "Release";
        case Gpmv1BindingMode::Toggle:  return "Toggle";
    }
    return "Unknown";
}

struct Gpmv1Binding {
    uint64_t          id          = 0;
    std::string       action;
    Gpmv1ButtonType   button      = Gpmv1ButtonType::FaceA;
    Gpmv1BindingMode  mode        = Gpmv1BindingMode::Press;

    [[nodiscard]] bool isValid()  const { return id != 0 && !action.empty(); }
    [[nodiscard]] bool isBound()  const { return mode != Gpmv1BindingMode::None; }
};

struct Gpmv1Profile {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Gpmv1ChangeCallback = std::function<void(uint64_t)>;

class GamepadMapperV1 {
public:
    static constexpr size_t MAX_BINDINGS = 256;
    static constexpr size_t MAX_PROFILES = 32;

    bool addBinding(const Gpmv1Binding& binding) {
        if (!binding.isValid()) return false;
        for (const auto& b : m_bindings) if (b.id == binding.id) return false;
        if (m_bindings.size() >= MAX_BINDINGS) return false;
        m_bindings.push_back(binding);
        if (m_onChange) m_onChange(binding.id);
        return true;
    }

    bool removeBinding(uint64_t id) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->id == id) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Gpmv1Binding* findBinding(uint64_t id) {
        for (auto& b : m_bindings) if (b.id == id) return &b;
        return nullptr;
    }

    bool addProfile(const Gpmv1Profile& profile) {
        if (!profile.isValid()) return false;
        for (const auto& p : m_profiles) if (p.id == profile.id) return false;
        if (m_profiles.size() >= MAX_PROFILES) return false;
        m_profiles.push_back(profile);
        return true;
    }

    bool removeProfile(uint64_t id) {
        for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
            if (it->id == id) { m_profiles.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t bindingCount() const { return m_bindings.size(); }
    [[nodiscard]] size_t profileCount() const { return m_profiles.size(); }

    [[nodiscard]] size_t boundCount() const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.isBound()) ++c; return c;
    }
    [[nodiscard]] size_t countByButton(Gpmv1ButtonType button) const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.button == button) ++c; return c;
    }
    [[nodiscard]] size_t countByMode(Gpmv1BindingMode mode) const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.mode == mode) ++c; return c;
    }

    void setOnChange(Gpmv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Gpmv1Binding> m_bindings;
    std::vector<Gpmv1Profile> m_profiles;
    Gpmv1ChangeCallback       m_onChange;
};

} // namespace NF
