#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_MENTORSHIP_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_MENTORSHIP_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

// CaptainMentorshipSystem — Phase B (Fleet Personality)
// Manages mentor-student relationships between captains. The mentor entity
// holds a CaptainMentorshipState component. Shared engagements (battles,
// warps, mining runs) grow bond_strength. When bond_strength reaches
// skill_transfer_threshold a skill-transfer milestone fires; at
// graduation_threshold the student is marked graduated and the session
// closes. A mentor can only mentor one student at a time.
class CaptainMentorshipSystem
    : public ecs::SingleComponentSystem<components::CaptainMentorshipState> {
public:
    explicit CaptainMentorshipSystem(ecs::World* world);
    ~CaptainMentorshipSystem() override = default;

    std::string getName() const override { return "CaptainMentorshipSystem"; }

    bool initialize(const std::string& entity_id);

    // Session lifecycle
    bool startMentorship(const std::string& entity_id,
                         const std::string& session_id,
                         const std::string& student_captain_id);
    bool endMentorship(const std::string& entity_id,
                       const std::string& session_id);
    bool graduateStudent(const std::string& entity_id,
                         const std::string& session_id);

    // Events that grow the bond
    bool recordSharedEngagement(const std::string& entity_id,
                                const std::string& session_id);
    bool applySkillTransfer(const std::string& entity_id,
                            const std::string& session_id);

    // Configuration
    bool setBondGrowthRate(const std::string& entity_id, float rate);
    bool setSkillTransferThreshold(const std::string& entity_id, float val);
    bool setGraduationThreshold(const std::string& entity_id, float val);
    bool setMentorCaptainId(const std::string& entity_id,
                            const std::string& mentor_id);
    bool setMaxSessions(const std::string& entity_id, int max);

    // Queries
    bool        hasActiveStudent(const std::string& entity_id) const;
    std::string getActiveStudentId(const std::string& entity_id) const;
    bool        hasSession(const std::string& entity_id,
                           const std::string& session_id) const;
    float       getBondStrength(const std::string& entity_id,
                                const std::string& session_id) const;
    int         getEngagementsShared(const std::string& entity_id,
                                     const std::string& session_id) const;
    int         getSkillTransfers(const std::string& entity_id,
                                  const std::string& session_id) const;
    bool        isSessionActive(const std::string& entity_id,
                                const std::string& session_id) const;
    bool        isGraduated(const std::string& entity_id,
                            const std::string& session_id) const;
    int         getSessionCount(const std::string& entity_id) const;
    int         getTotalStudentsMentored(const std::string& entity_id) const;
    int         getTotalGraduations(const std::string& entity_id) const;
    int         getTotalSkillTransfers(const std::string& entity_id) const;
    std::string getMentorCaptainId(const std::string& entity_id) const;
    float       getBondGrowthRate(const std::string& entity_id) const;
    float       getSkillTransferThreshold(const std::string& entity_id) const;
    float       getGraduationThreshold(const std::string& entity_id) const;
    int         getMaxSessions(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CaptainMentorshipState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_MENTORSHIP_SYSTEM_H
