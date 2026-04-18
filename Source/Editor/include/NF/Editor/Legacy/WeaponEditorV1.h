#pragma once
// NF::Editor — Weapon editor v1: weapon definition and attachment management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Weev1WeaponType  : uint8_t { Melee, Ranged, Thrown, Magic, Hybrid };
enum class Weev1WeaponState : uint8_t { Draft, Ready, Deprecated, Locked };

inline const char* weev1WeaponTypeName(Weev1WeaponType t) {
    switch (t) {
        case Weev1WeaponType::Melee:   return "Melee";
        case Weev1WeaponType::Ranged:  return "Ranged";
        case Weev1WeaponType::Thrown:  return "Thrown";
        case Weev1WeaponType::Magic:   return "Magic";
        case Weev1WeaponType::Hybrid:  return "Hybrid";
    }
    return "Unknown";
}

inline const char* weev1WeaponStateName(Weev1WeaponState s) {
    switch (s) {
        case Weev1WeaponState::Draft:      return "Draft";
        case Weev1WeaponState::Ready:      return "Ready";
        case Weev1WeaponState::Deprecated: return "Deprecated";
        case Weev1WeaponState::Locked:     return "Locked";
    }
    return "Unknown";
}

struct Weev1Weapon {
    uint64_t         id         = 0;
    std::string      name;
    Weev1WeaponType  weaponType = Weev1WeaponType::Melee;
    Weev1WeaponState state      = Weev1WeaponState::Draft;
    float            baseDamage = 0.0f;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isReady()      const { return state == Weev1WeaponState::Ready; }
    [[nodiscard]] bool isDeprecated() const { return state == Weev1WeaponState::Deprecated; }
    [[nodiscard]] bool isLocked()     const { return state == Weev1WeaponState::Locked; }
};

struct Weev1Attachment {
    uint64_t    id       = 0;
    uint64_t    weaponId = 0;
    std::string name;
    std::string slot;

    [[nodiscard]] bool isValid() const { return id != 0 && weaponId != 0 && !name.empty(); }
};

using Weev1ChangeCallback = std::function<void(uint64_t)>;

class WeaponEditorV1 {
public:
    static constexpr size_t MAX_WEAPONS     = 1024;
    static constexpr size_t MAX_ATTACHMENTS = 4096;

    bool addWeapon(const Weev1Weapon& weapon) {
        if (!weapon.isValid()) return false;
        for (const auto& w : m_weapons) if (w.id == weapon.id) return false;
        if (m_weapons.size() >= MAX_WEAPONS) return false;
        m_weapons.push_back(weapon);
        if (m_onChange) m_onChange(weapon.id);
        return true;
    }

    bool removeWeapon(uint64_t id) {
        for (auto it = m_weapons.begin(); it != m_weapons.end(); ++it) {
            if (it->id == id) { m_weapons.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Weev1Weapon* findWeapon(uint64_t id) {
        for (auto& w : m_weapons) if (w.id == id) return &w;
        return nullptr;
    }

    bool addAttachment(const Weev1Attachment& attach) {
        if (!attach.isValid()) return false;
        for (const auto& a : m_attachments) if (a.id == attach.id) return false;
        if (m_attachments.size() >= MAX_ATTACHMENTS) return false;
        m_attachments.push_back(attach);
        if (m_onChange) m_onChange(attach.id);
        return true;
    }

    bool removeAttachment(uint64_t id) {
        for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it) {
            if (it->id == id) { m_attachments.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t weaponCount()     const { return m_weapons.size(); }
    [[nodiscard]] size_t attachmentCount() const { return m_attachments.size(); }

    [[nodiscard]] size_t readyCount() const {
        size_t c = 0; for (const auto& w : m_weapons) if (w.isReady()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& w : m_weapons) if (w.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Weev1WeaponType type) const {
        size_t c = 0; for (const auto& w : m_weapons) if (w.weaponType == type) ++c; return c;
    }
    [[nodiscard]] size_t attachmentsForWeapon(uint64_t weaponId) const {
        size_t c = 0; for (const auto& a : m_attachments) if (a.weaponId == weaponId) ++c; return c;
    }
    [[nodiscard]] float totalBaseDamage() const {
        float total = 0.0f; for (const auto& w : m_weapons) total += w.baseDamage; return total;
    }

    void setOnChange(Weev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Weev1Weapon>     m_weapons;
    std::vector<Weev1Attachment> m_attachments;
    Weev1ChangeCallback          m_onChange;
};

} // namespace NF
