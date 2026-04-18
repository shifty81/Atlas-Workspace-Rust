#ifndef NOVAFORGE_SYSTEMS_CHARACTER_SHEET_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CHARACTER_SHEET_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

class CharacterSheetSystem
    : public ecs::SingleComponentSystem<components::CharacterSheet> {
public:
    explicit CharacterSheetSystem(ecs::World* world);
    ~CharacterSheetSystem() override = default;

    std::string getName() const override { return "CharacterSheetSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Identity configuration ---
    bool setCharacterName(const std::string& entity_id,
                          const std::string& name);
    bool setRace(const std::string& entity_id,
                 const std::string& race);
    bool setBloodline(const std::string& entity_id,
                      const std::string& bloodline);
    bool setAncestry(const std::string& entity_id,
                     const std::string& ancestry);
    bool setGender(const std::string& entity_id,
                   const std::string& gender);
    bool setDateOfBirth(const std::string& entity_id, float dob);

    // --- Attributes ---
    bool setAttribute(const std::string& entity_id,
                      const std::string& attribute,
                      int value);

    // --- Security status ---
    bool setSecurityStatus(const std::string& entity_id, float status);
    bool adjustSecurityStatus(const std::string& entity_id, float delta);

    // --- Clone ---
    bool setCloneGrade(const std::string& entity_id,
                       const std::string& grade);
    bool setCloneLocation(const std::string& entity_id,
                          const std::string& station_id);
    bool setCloneJumpCooldown(const std::string& entity_id,
                              int seconds);

    // --- Implants ---
    bool addImplant(const std::string& entity_id,
                    const std::string& implant_id,
                    const std::string& implant_name,
                    int slot,
                    const std::string& attribute_bonus,
                    int bonus_amount);
    bool removeImplant(const std::string& entity_id,
                       const std::string& implant_id);
    bool clearImplants(const std::string& entity_id);

    // --- Employment history ---
    bool addEmploymentRecord(const std::string& entity_id,
                             const std::string& corp_id,
                             const std::string& corp_name,
                             float join_date);
    bool endEmployment(const std::string& entity_id,
                       const std::string& corp_id,
                       float leave_date);
    bool clearEmploymentHistory(const std::string& entity_id);

    // --- Queries ---
    std::string getCharacterName(const std::string& entity_id) const;
    std::string getRace(const std::string& entity_id) const;
    std::string getBloodline(const std::string& entity_id) const;
    std::string getAncestry(const std::string& entity_id) const;
    std::string getGender(const std::string& entity_id) const;
    float       getDateOfBirth(const std::string& entity_id) const;
    float       getSecurityStatus(const std::string& entity_id) const;
    int         getBaseAttribute(const std::string& entity_id,
                                 const std::string& attribute) const;
    int         getEffectiveAttribute(const std::string& entity_id,
                                      const std::string& attribute) const;
    std::string getCloneGrade(const std::string& entity_id) const;
    std::string getCloneLocation(const std::string& entity_id) const;
    int         getCloneJumpCooldown(const std::string& entity_id) const;
    bool        isOnCloneCooldown(const std::string& entity_id) const;
    int         getImplantCount(const std::string& entity_id) const;
    bool        hasImplant(const std::string& entity_id,
                           const std::string& implant_id) const;
    int         getImplantSlot(const std::string& entity_id,
                               const std::string& implant_id) const;
    int         getEmploymentCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CharacterSheet& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CHARACTER_SHEET_SYSTEM_H
