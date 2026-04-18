#include "systems/captain_mentorship_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CaptainMentorshipSystem::CaptainMentorshipSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CaptainMentorshipSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CaptainMentorshipState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    // Advance session_duration for the active session
    if (comp.has_active_student) {
        for (auto& s : comp.sessions) {
            if (s.is_active) {
                s.session_duration += delta_time;
            }
        }
    }
}

bool CaptainMentorshipSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CaptainMentorshipState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CaptainMentorshipSystem::startMentorship(const std::string& entity_id,
                                               const std::string& session_id,
                                               const std::string& student_captain_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (session_id.empty() || student_captain_id.empty()) return false;
    if (comp->has_active_student) return false; // already mentoring someone
    // Check duplicate session_id
    for (const auto& s : comp->sessions) {
        if (s.session_id == session_id) return false;
    }
    // Enforce capacity cap (trim oldest inactive)
    if (static_cast<int>(comp->sessions.size()) >= comp->max_sessions) {
        auto it = std::find_if(comp->sessions.begin(), comp->sessions.end(),
                               [](const components::MentorSession& s){ return !s.is_active; });
        if (it != comp->sessions.end()) comp->sessions.erase(it);
        else return false;
    }
    components::MentorSession session;
    session.session_id          = session_id;
    session.student_captain_id  = student_captain_id;
    session.is_active           = true;
    session.is_graduated        = false;
    comp->sessions.push_back(session);
    comp->active_student_id  = student_captain_id;
    comp->has_active_student = true;
    ++comp->total_students_mentored;
    return true;
}

bool CaptainMentorshipSystem::endMentorship(const std::string& entity_id,
                                             const std::string& session_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& s : comp->sessions) {
        if (s.session_id == session_id && s.is_active) {
            s.is_active = false;
            if (comp->active_student_id == s.student_captain_id) {
                comp->has_active_student = false;
                comp->active_student_id  = "";
            }
            return true;
        }
    }
    return false;
}

bool CaptainMentorshipSystem::graduateStudent(const std::string& entity_id,
                                               const std::string& session_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& s : comp->sessions) {
        if (s.session_id == session_id) {
            if (s.is_graduated) return false; // already graduated
            if (s.bond_strength < comp->graduation_threshold) return false;
            s.is_graduated = true;
            s.is_active    = false;
            if (comp->active_student_id == s.student_captain_id) {
                comp->has_active_student = false;
                comp->active_student_id  = "";
            }
            ++comp->total_graduations;
            return true;
        }
    }
    return false;
}

bool CaptainMentorshipSystem::recordSharedEngagement(
        const std::string& entity_id,
        const std::string& session_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& s : comp->sessions) {
        if (s.session_id == session_id && s.is_active) {
            ++s.engagements_shared;
            s.bond_strength = std::min(1.0f,
                s.bond_strength + comp->bond_growth_rate);
            return true;
        }
    }
    return false;
}

bool CaptainMentorshipSystem::applySkillTransfer(const std::string& entity_id,
                                                  const std::string& session_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& s : comp->sessions) {
        if (s.session_id == session_id && s.is_active) {
            if (s.bond_strength < comp->skill_transfer_threshold) return false;
            ++s.skill_transfers;
            ++comp->total_skill_transfers;
            return true;
        }
    }
    return false;
}

bool CaptainMentorshipSystem::setBondGrowthRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->bond_growth_rate = rate;
    return true;
}

bool CaptainMentorshipSystem::setSkillTransferThreshold(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (val < 0.0f || val > 1.0f) return false;
    comp->skill_transfer_threshold = val;
    return true;
}

bool CaptainMentorshipSystem::setGraduationThreshold(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (val < 0.0f || val > 1.0f) return false;
    comp->graduation_threshold = val;
    return true;
}

bool CaptainMentorshipSystem::setMentorCaptainId(const std::string& entity_id,
                                                   const std::string& mentor_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (mentor_id.empty()) return false;
    comp->mentor_captain_id = mentor_id;
    return true;
}

bool CaptainMentorshipSystem::setMaxSessions(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_sessions = max;
    return true;
}

// ── Queries ──────────────────────────────────────────────────────────────────

bool CaptainMentorshipSystem::hasActiveStudent(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->has_active_student : false;
}

std::string CaptainMentorshipSystem::getActiveStudentId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active_student_id : "";
}

bool CaptainMentorshipSystem::hasSession(const std::string& entity_id,
                                          const std::string& session_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->sessions) {
        if (s.session_id == session_id) return true;
    }
    return false;
}

float CaptainMentorshipSystem::getBondStrength(const std::string& entity_id,
                                                const std::string& session_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& s : comp->sessions) {
        if (s.session_id == session_id) return s.bond_strength;
    }
    return 0.0f;
}

int CaptainMentorshipSystem::getEngagementsShared(const std::string& entity_id,
                                                   const std::string& session_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& s : comp->sessions) {
        if (s.session_id == session_id) return s.engagements_shared;
    }
    return 0;
}

int CaptainMentorshipSystem::getSkillTransfers(const std::string& entity_id,
                                                const std::string& session_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& s : comp->sessions) {
        if (s.session_id == session_id) return s.skill_transfers;
    }
    return 0;
}

bool CaptainMentorshipSystem::isSessionActive(const std::string& entity_id,
                                               const std::string& session_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->sessions) {
        if (s.session_id == session_id) return s.is_active;
    }
    return false;
}

bool CaptainMentorshipSystem::isGraduated(const std::string& entity_id,
                                           const std::string& session_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->sessions) {
        if (s.session_id == session_id) return s.is_graduated;
    }
    return false;
}

int CaptainMentorshipSystem::getSessionCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->sessions.size()) : 0;
}

int CaptainMentorshipSystem::getTotalStudentsMentored(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_students_mentored : 0;
}

int CaptainMentorshipSystem::getTotalGraduations(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_graduations : 0;
}

int CaptainMentorshipSystem::getTotalSkillTransfers(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_skill_transfers : 0;
}

std::string CaptainMentorshipSystem::getMentorCaptainId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->mentor_captain_id : "";
}

float CaptainMentorshipSystem::getBondGrowthRate(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->bond_growth_rate : 0.0f;
}

float CaptainMentorshipSystem::getSkillTransferThreshold(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->skill_transfer_threshold : 0.0f;
}

float CaptainMentorshipSystem::getGraduationThreshold(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->graduation_threshold : 0.0f;
}

int CaptainMentorshipSystem::getMaxSessions(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->max_sessions : 0;
}

} // namespace systems
} // namespace atlas
