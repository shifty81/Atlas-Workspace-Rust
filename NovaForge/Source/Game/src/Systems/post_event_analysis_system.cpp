#include "systems/post_event_analysis_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PostEventAnalysisSystem::PostEventAnalysisSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PostEventAnalysisSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::PostEventAnalysisState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (!comp.analysis_in_progress) return;

    comp.analysis_timer += delta_time;
    if (comp.analysis_timer >= comp.analysis_duration) {
        // Auto-finalize with empty conclusion
        components::PostEventAnalysisState::EventAnalysis ea;
        ea.analysis_id        = comp.current_event_id;
        ea.event_description  = "";
        ea.conclusion         = "";
        ea.agreement_count    = 0;
        ea.dissent_count      = 0;
        ea.is_finalized       = true;
        ea.blame              = comp.pending_blame;

        if (static_cast<int>(comp.analyses.size()) >= comp.max_analyses) {
            comp.analyses.erase(comp.analyses.begin());
        }
        comp.analyses.push_back(ea);
        ++comp.total_analyses_completed;

        comp.analysis_in_progress = false;
        comp.analysis_timer       = 0.0f;
        comp.current_event_id.clear();
        comp.pending_blame.clear();
    }
}

bool PostEventAnalysisSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PostEventAnalysisState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool PostEventAnalysisSystem::startAnalysis(const std::string& entity_id,
                                             const std::string& event_id,
                                             const std::string& event_description,
                                             float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (event_id.empty()) return false;
    if (duration <= 0.0f) return false;
    if (comp->analysis_in_progress) return false;

    comp->current_event_id     = event_id;
    comp->analysis_in_progress = true;
    comp->analysis_timer       = 0.0f;
    comp->analysis_duration    = duration;
    comp->pending_blame.clear();
    (void)event_description; // stored in finalize
    return true;
}

bool PostEventAnalysisSystem::addBlame(const std::string& entity_id,
                                        const std::string& captain_id,
                                        const std::string& reason,
                                        float blame_weight) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->analysis_in_progress) return false;
    if (captain_id.empty()) return false;
    if (reason.empty()) return false;
    if (blame_weight < 0.0f || blame_weight > 1.0f) return false;

    components::PostEventAnalysisState::CaptainBlame blame;
    blame.captain_id   = captain_id;
    blame.reason       = reason;
    blame.blame_weight = blame_weight;
    comp->pending_blame.push_back(blame);
    return true;
}

bool PostEventAnalysisSystem::finalizeAnalysis(const std::string& entity_id,
                                                const std::string& conclusion,
                                                int agreement_count,
                                                int dissent_count) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->analysis_in_progress) return false;

    components::PostEventAnalysisState::EventAnalysis ea;
    ea.analysis_id        = comp->current_event_id;
    ea.event_description  = "";
    ea.conclusion         = conclusion;
    ea.agreement_count    = agreement_count;
    ea.dissent_count      = dissent_count;
    ea.is_finalized       = true;
    ea.blame              = comp->pending_blame;

    if (static_cast<int>(comp->analyses.size()) >= comp->max_analyses) {
        comp->analyses.erase(comp->analyses.begin());
    }
    comp->analyses.push_back(ea);
    ++comp->total_analyses_completed;

    comp->analysis_in_progress = false;
    comp->analysis_timer       = 0.0f;
    comp->current_event_id.clear();
    comp->pending_blame.clear();
    return true;
}

bool PostEventAnalysisSystem::dismissAnalysis(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->analysis_in_progress) return false;

    comp->analysis_in_progress = false;
    comp->analysis_timer       = 0.0f;
    comp->current_event_id.clear();
    comp->pending_blame.clear();
    return true;
}

bool PostEventAnalysisSystem::addLesson(const std::string& entity_id,
                                         const std::string& lesson) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (lesson.empty()) return false;

    if (static_cast<int>(comp->lessons.size()) >= comp->max_lessons) {
        comp->lessons.erase(comp->lessons.begin());
    }
    comp->lessons.push_back(lesson);
    ++comp->total_lessons_learned;
    return true;
}

bool PostEventAnalysisSystem::clearLessons(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->lessons.clear();
    return true;
}

bool PostEventAnalysisSystem::removeAnalysis(const std::string& entity_id,
                                              const std::string& analysis_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->analyses.begin(), comp->analyses.end(),
        [&](const components::PostEventAnalysisState::EventAnalysis& a) {
            return a.analysis_id == analysis_id;
        });
    if (it == comp->analyses.end()) return false;
    comp->analyses.erase(it);
    return true;
}

bool PostEventAnalysisSystem::clearAnalyses(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->analyses.clear();
    return true;
}

bool PostEventAnalysisSystem::setFleetId(const std::string& entity_id,
                                          const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fleet_id.empty()) return false;
    comp->fleet_id = fleet_id;
    return true;
}

bool PostEventAnalysisSystem::setMaxAnalyses(const std::string& entity_id,
                                              int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_analyses = max;
    return true;
}

bool PostEventAnalysisSystem::setAnalysisDuration(const std::string& entity_id,
                                                    float secs) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (secs <= 0.0f) return false;
    comp->analysis_duration = secs;
    return true;
}

bool PostEventAnalysisSystem::isAnalysisInProgress(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->analysis_in_progress;
}

float PostEventAnalysisSystem::getAnalysisProgress(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || !comp->analysis_in_progress || comp->analysis_duration <= 0.0f)
        return 0.0f;
    float p = comp->analysis_timer / comp->analysis_duration;
    if (p > 1.0f) p = 1.0f;
    return p;
}

std::string PostEventAnalysisSystem::getCurrentEventId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->current_event_id;
}

int PostEventAnalysisSystem::getAnalysisCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->analyses.size());
}

bool PostEventAnalysisSystem::hasAnalysis(
        const std::string& entity_id,
        const std::string& analysis_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& a : comp->analyses) {
        if (a.analysis_id == analysis_id) return true;
    }
    return false;
}

bool PostEventAnalysisSystem::isAnalysisFinalized(
        const std::string& entity_id,
        const std::string& analysis_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& a : comp->analyses) {
        if (a.analysis_id == analysis_id) return a.is_finalized;
    }
    return false;
}

std::string PostEventAnalysisSystem::getAnalysisConclusion(
        const std::string& entity_id,
        const std::string& analysis_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& a : comp->analyses) {
        if (a.analysis_id == analysis_id) return a.conclusion;
    }
    return "";
}

int PostEventAnalysisSystem::getAnalysisAgreementCount(
        const std::string& entity_id,
        const std::string& analysis_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& a : comp->analyses) {
        if (a.analysis_id == analysis_id) return a.agreement_count;
    }
    return 0;
}

int PostEventAnalysisSystem::getAnalysisDisssentCount(
        const std::string& entity_id,
        const std::string& analysis_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& a : comp->analyses) {
        if (a.analysis_id == analysis_id) return a.dissent_count;
    }
    return 0;
}

int PostEventAnalysisSystem::getCompletedBlameCount(
        const std::string& entity_id,
        const std::string& analysis_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& a : comp->analyses) {
        if (a.analysis_id == analysis_id)
            return static_cast<int>(a.blame.size());
    }
    return 0;
}

int PostEventAnalysisSystem::getTotalAnalysesCompleted(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_analyses_completed;
}

int PostEventAnalysisSystem::getPendingBlameCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->pending_blame.size());
}

int PostEventAnalysisSystem::getLessonCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->lessons.size());
}

int PostEventAnalysisSystem::getTotalLessonsLearned(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_lessons_learned;
}

std::string PostEventAnalysisSystem::getFleetId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->fleet_id;
}

float PostEventAnalysisSystem::getAnalysisDuration(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->analysis_duration;
}

} // namespace systems
} // namespace atlas
