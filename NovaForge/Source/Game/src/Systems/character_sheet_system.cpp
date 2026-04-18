#include "systems/character_sheet_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

CharacterSheetSystem::CharacterSheetSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CharacterSheetSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CharacterSheet& comp,
        float delta_time) {
    // Tick down clone jump cooldown
    if (comp.clone_jump_cooldown > 0) {
        int ticks = static_cast<int>(delta_time);
        if (ticks < 1) ticks = 1;
        comp.clone_jump_cooldown -= ticks;
        if (comp.clone_jump_cooldown < 0) comp.clone_jump_cooldown = 0;
    }
}

bool CharacterSheetSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CharacterSheet>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Identity configuration ---

bool CharacterSheetSystem::setCharacterName(const std::string& entity_id,
                                            const std::string& name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (name.empty()) return false;
    comp->character_name = name;
    return true;
}

bool CharacterSheetSystem::setRace(const std::string& entity_id,
                                   const std::string& race) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (race.empty()) return false;
    comp->race = race;
    return true;
}

bool CharacterSheetSystem::setBloodline(const std::string& entity_id,
                                        const std::string& bloodline) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->bloodline = bloodline;
    return true;
}

bool CharacterSheetSystem::setAncestry(const std::string& entity_id,
                                       const std::string& ancestry) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->ancestry = ancestry;
    return true;
}

bool CharacterSheetSystem::setGender(const std::string& entity_id,
                                     const std::string& gender) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (gender != "male" && gender != "female") return false;
    comp->gender = gender;
    return true;
}

bool CharacterSheetSystem::setDateOfBirth(const std::string& entity_id,
                                          float dob) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (dob < 0.0f) return false;
    comp->date_of_birth = dob;
    return true;
}

// --- Attributes ---

bool CharacterSheetSystem::setAttribute(const std::string& entity_id,
                                        const std::string& attribute,
                                        int value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0) return false;
    if (attribute == "intelligence") { comp->intelligence = value; return true; }
    if (attribute == "perception")   { comp->perception = value;   return true; }
    if (attribute == "charisma")     { comp->charisma = value;     return true; }
    if (attribute == "willpower")    { comp->willpower = value;    return true; }
    if (attribute == "memory")       { comp->memory = value;       return true; }
    return false; // unknown attribute
}

// --- Security status ---

bool CharacterSheetSystem::setSecurityStatus(const std::string& entity_id,
                                             float status) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (status < -10.0f) status = -10.0f;
    if (status > 10.0f) status = 10.0f;
    comp->security_status = status;
    return true;
}

bool CharacterSheetSystem::adjustSecurityStatus(const std::string& entity_id,
                                                float delta) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    float ns = comp->security_status + delta;
    if (ns < -10.0f) ns = -10.0f;
    if (ns > 10.0f) ns = 10.0f;
    comp->security_status = ns;
    return true;
}

// --- Clone ---

bool CharacterSheetSystem::setCloneGrade(const std::string& entity_id,
                                         const std::string& grade) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (grade.empty()) return false;
    comp->clone_grade = grade;
    return true;
}

bool CharacterSheetSystem::setCloneLocation(const std::string& entity_id,
                                            const std::string& station_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->clone_location = station_id;
    return true;
}

bool CharacterSheetSystem::setCloneJumpCooldown(const std::string& entity_id,
                                                int seconds) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (seconds < 0) return false;
    comp->clone_jump_cooldown = seconds;
    return true;
}

// --- Implants ---

bool CharacterSheetSystem::addImplant(const std::string& entity_id,
                                      const std::string& implant_id,
                                      const std::string& implant_name,
                                      int slot,
                                      const std::string& attribute_bonus,
                                      int bonus_amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (implant_id.empty()) return false;
    if (slot < 1 || slot > 10) return false;
    if (bonus_amount < 0) return false;

    // Duplicate implant_id prevention
    for (const auto& imp : comp->implants) {
        if (imp.implant_id == implant_id) return false;
    }
    // Slot occupancy check
    for (const auto& imp : comp->implants) {
        if (imp.slot == slot) return false;
    }

    components::CharacterSheet::Implant imp;
    imp.implant_id = implant_id;
    imp.implant_name = implant_name;
    imp.slot = slot;
    imp.attribute_bonus = attribute_bonus;
    imp.bonus_amount = bonus_amount;
    comp->implants.push_back(imp);
    return true;
}

bool CharacterSheetSystem::removeImplant(const std::string& entity_id,
                                         const std::string& implant_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->implants.begin(); it != comp->implants.end(); ++it) {
        if (it->implant_id == implant_id) {
            comp->implants.erase(it);
            return true;
        }
    }
    return false;
}

bool CharacterSheetSystem::clearImplants(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->implants.clear();
    return true;
}

// --- Employment history ---

bool CharacterSheetSystem::addEmploymentRecord(const std::string& entity_id,
                                               const std::string& corp_id,
                                               const std::string& corp_name,
                                               float join_date) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (corp_id.empty()) return false;
    if (corp_name.empty()) return false;

    components::CharacterSheet::EmploymentRecord rec;
    rec.corp_id = corp_id;
    rec.corp_name = corp_name;
    rec.join_date = join_date;
    rec.leave_date = 0.0f;
    comp->employment_history.push_back(rec);
    return true;
}

bool CharacterSheetSystem::endEmployment(const std::string& entity_id,
                                         const std::string& corp_id,
                                         float leave_date) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& rec : comp->employment_history) {
        if (rec.corp_id == corp_id && rec.leave_date == 0.0f) {
            rec.leave_date = leave_date;
            return true;
        }
    }
    return false;
}

bool CharacterSheetSystem::clearEmploymentHistory(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->employment_history.clear();
    return true;
}

// --- Queries ---

std::string CharacterSheetSystem::getCharacterName(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->character_name;
}

std::string CharacterSheetSystem::getRace(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->race;
}

std::string CharacterSheetSystem::getBloodline(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->bloodline;
}

std::string CharacterSheetSystem::getAncestry(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->ancestry;
}

std::string CharacterSheetSystem::getGender(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->gender;
}

float CharacterSheetSystem::getDateOfBirth(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->date_of_birth;
}

float CharacterSheetSystem::getSecurityStatus(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->security_status;
}

int CharacterSheetSystem::getBaseAttribute(
        const std::string& entity_id,
        const std::string& attribute) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    if (attribute == "intelligence") return comp->intelligence;
    if (attribute == "perception")   return comp->perception;
    if (attribute == "charisma")     return comp->charisma;
    if (attribute == "willpower")    return comp->willpower;
    if (attribute == "memory")       return comp->memory;
    return 0;
}

int CharacterSheetSystem::getEffectiveAttribute(
        const std::string& entity_id,
        const std::string& attribute) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->getEffectiveAttribute(attribute);
}

std::string CharacterSheetSystem::getCloneGrade(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->clone_grade;
}

std::string CharacterSheetSystem::getCloneLocation(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->clone_location;
}

int CharacterSheetSystem::getCloneJumpCooldown(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->clone_jump_cooldown;
}

bool CharacterSheetSystem::isOnCloneCooldown(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->clone_jump_cooldown > 0;
}

int CharacterSheetSystem::getImplantCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->implants.size());
}

bool CharacterSheetSystem::hasImplant(
        const std::string& entity_id,
        const std::string& implant_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& imp : comp->implants) {
        if (imp.implant_id == implant_id) return true;
    }
    return false;
}

int CharacterSheetSystem::getImplantSlot(
        const std::string& entity_id,
        const std::string& implant_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& imp : comp->implants) {
        if (imp.implant_id == implant_id) return imp.slot;
    }
    return 0;
}

int CharacterSheetSystem::getEmploymentCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->employment_history.size());
}

} // namespace systems
} // namespace atlas
