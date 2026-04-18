#ifndef NOVAFORGE_SYSTEMS_POST_EVENT_ANALYSIS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_POST_EVENT_ANALYSIS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class PostEventAnalysisSystem
    : public ecs::SingleComponentSystem<components::PostEventAnalysisState> {
public:
    explicit PostEventAnalysisSystem(ecs::World* world);
    ~PostEventAnalysisSystem() override = default;

    std::string getName() const override { return "PostEventAnalysisSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Analysis workflow ---
    bool startAnalysis(const std::string& entity_id,
                       const std::string& event_id,
                       const std::string& event_description,
                       float duration);

    bool addBlame(const std::string& entity_id,
                  const std::string& captain_id,
                  const std::string& reason,
                  float blame_weight);

    bool finalizeAnalysis(const std::string& entity_id,
                          const std::string& conclusion,
                          int agreement_count,
                          int dissent_count);

    bool dismissAnalysis(const std::string& entity_id);

    // --- Lessons ---
    bool addLesson(const std::string& entity_id, const std::string& lesson);
    bool clearLessons(const std::string& entity_id);

    // --- Analysis management ---
    bool removeAnalysis(const std::string& entity_id,
                        const std::string& analysis_id);
    bool clearAnalyses(const std::string& entity_id);

    // --- Configuration ---
    bool setFleetId(const std::string& entity_id, const std::string& fleet_id);
    bool setMaxAnalyses(const std::string& entity_id, int max);
    bool setAnalysisDuration(const std::string& entity_id, float secs);

    // --- Queries ---
    bool        isAnalysisInProgress(const std::string& entity_id) const;
    float       getAnalysisProgress(const std::string& entity_id) const;
    std::string getCurrentEventId(const std::string& entity_id) const;
    int         getAnalysisCount(const std::string& entity_id) const;
    bool        hasAnalysis(const std::string& entity_id,
                            const std::string& analysis_id) const;
    bool        isAnalysisFinalized(const std::string& entity_id,
                                    const std::string& analysis_id) const;
    std::string getAnalysisConclusion(const std::string& entity_id,
                                      const std::string& analysis_id) const;
    int         getAnalysisAgreementCount(const std::string& entity_id,
                                          const std::string& analysis_id) const;
    int         getAnalysisDisssentCount(const std::string& entity_id,
                                          const std::string& analysis_id) const;
    int         getCompletedBlameCount(const std::string& entity_id,
                                       const std::string& analysis_id) const;
    int         getTotalAnalysesCompleted(const std::string& entity_id) const;
    int         getPendingBlameCount(const std::string& entity_id) const;
    int         getLessonCount(const std::string& entity_id) const;
    int         getTotalLessonsLearned(const std::string& entity_id) const;
    std::string getFleetId(const std::string& entity_id) const;
    float       getAnalysisDuration(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::PostEventAnalysisState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_POST_EVENT_ANALYSIS_SYSTEM_H
