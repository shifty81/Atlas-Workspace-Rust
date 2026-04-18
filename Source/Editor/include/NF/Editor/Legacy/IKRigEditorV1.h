#pragma once
// NF::Editor — IK rig editor v1: IK chain and effector management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ikev1ChainType  : uint8_t { TwoBone, FABRIK, CCD, Spline, Aim, LookAt };
enum class Ikev1ChainState : uint8_t { Inactive, Active, Blending, Disabled };

inline const char* ikev1ChainTypeName(Ikev1ChainType t) {
    switch (t) {
        case Ikev1ChainType::TwoBone: return "TwoBone";
        case Ikev1ChainType::FABRIK:  return "FABRIK";
        case Ikev1ChainType::CCD:     return "CCD";
        case Ikev1ChainType::Spline:  return "Spline";
        case Ikev1ChainType::Aim:     return "Aim";
        case Ikev1ChainType::LookAt:  return "LookAt";
    }
    return "Unknown";
}

inline const char* ikev1ChainStateName(Ikev1ChainState s) {
    switch (s) {
        case Ikev1ChainState::Inactive:  return "Inactive";
        case Ikev1ChainState::Active:    return "Active";
        case Ikev1ChainState::Blending:  return "Blending";
        case Ikev1ChainState::Disabled:  return "Disabled";
    }
    return "Unknown";
}

struct Ikev1Chain {
    uint64_t        id        = 0;
    std::string     name;
    Ikev1ChainType  chainType = Ikev1ChainType::TwoBone;
    Ikev1ChainState state     = Ikev1ChainState::Inactive;
    float           weight    = 1.0f;
    uint32_t        iterations = 10;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()   const { return state == Ikev1ChainState::Active; }
    [[nodiscard]] bool isBlending() const { return state == Ikev1ChainState::Blending; }
    [[nodiscard]] bool isDisabled() const { return state == Ikev1ChainState::Disabled; }
};

struct Ikev1Effector {
    uint64_t    id      = 0;
    uint64_t    chainId = 0;
    std::string name;
    float       weight  = 1.0f;

    [[nodiscard]] bool isValid() const { return id != 0 && chainId != 0 && !name.empty(); }
};

using Ikev1ChangeCallback = std::function<void(uint64_t)>;

class IKRigEditorV1 {
public:
    static constexpr size_t MAX_CHAINS    = 128;
    static constexpr size_t MAX_EFFECTORS = 256;

    bool addChain(const Ikev1Chain& chain) {
        if (!chain.isValid()) return false;
        for (const auto& c : m_chains) if (c.id == chain.id) return false;
        if (m_chains.size() >= MAX_CHAINS) return false;
        m_chains.push_back(chain);
        if (m_onChange) m_onChange(chain.id);
        return true;
    }

    bool removeChain(uint64_t id) {
        for (auto it = m_chains.begin(); it != m_chains.end(); ++it) {
            if (it->id == id) { m_chains.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ikev1Chain* findChain(uint64_t id) {
        for (auto& c : m_chains) if (c.id == id) return &c;
        return nullptr;
    }

    bool addEffector(const Ikev1Effector& effector) {
        if (!effector.isValid()) return false;
        for (const auto& e : m_effectors) if (e.id == effector.id) return false;
        if (m_effectors.size() >= MAX_EFFECTORS) return false;
        m_effectors.push_back(effector);
        if (m_onChange) m_onChange(effector.chainId);
        return true;
    }

    bool removeEffector(uint64_t id) {
        for (auto it = m_effectors.begin(); it != m_effectors.end(); ++it) {
            if (it->id == id) { m_effectors.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t chainCount()    const { return m_chains.size(); }
    [[nodiscard]] size_t effectorCount() const { return m_effectors.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& ch : m_chains) if (ch.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t disabledCount() const {
        size_t c = 0; for (const auto& ch : m_chains) if (ch.isDisabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Ikev1ChainType type) const {
        size_t c = 0; for (const auto& ch : m_chains) if (ch.chainType == type) ++c; return c;
    }
    [[nodiscard]] size_t effectorsForChain(uint64_t chainId) const {
        size_t c = 0; for (const auto& e : m_effectors) if (e.chainId == chainId) ++c; return c;
    }

    void setOnChange(Ikev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ikev1Chain>    m_chains;
    std::vector<Ikev1Effector> m_effectors;
    Ikev1ChangeCallback        m_onChange;
};

} // namespace NF
