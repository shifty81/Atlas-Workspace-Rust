#include "systems/galactic_news_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

GalacticNewsSystem::GalacticNewsSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void GalacticNewsSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::GalacticNewsState& comp,
        float delta_time) {
    if (!comp.active) return;
    for (auto& entry : comp.news) {
        if (!entry.is_expired) {
            entry.age_seconds += comp.news_decay_rate * delta_time;
            if (entry.age_seconds >= comp.expiry_threshold) {
                entry.is_expired = true;
                ++comp.total_expired;
            }
        }
    }
    comp.elapsed += delta_time;
}

bool GalacticNewsSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::GalacticNewsState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool GalacticNewsSystem::publishNews(
        const std::string& entity_id,
        const std::string& entry_id,
        const std::string& headline,
        components::GalacticNewsState::NewsCategory category,
        const std::string& source_system) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (entry_id.empty()) return false;
    if (headline.empty()) return false;
    for (const auto& e : comp->news) {
        if (e.entry_id == entry_id) return false;
    }
    if (static_cast<int>(comp->news.size()) >= comp->max_entries) {
        // Purge oldest expired first
        auto it = std::find_if(comp->news.begin(), comp->news.end(),
            [](const components::GalacticNewsState::NewsEntry& e) { return e.is_expired; });
        if (it != comp->news.end()) {
            comp->news.erase(it);
        } else {
            comp->news.erase(comp->news.begin());
        }
    }
    components::GalacticNewsState::NewsEntry entry;
    entry.entry_id = entry_id;
    entry.headline = headline;
    entry.source_system = source_system;
    entry.category = category;
    entry.age_seconds = 0.0f;
    entry.is_expired = false;
    comp->news.push_back(entry);
    ++comp->total_published;
    return true;
}

bool GalacticNewsSystem::removeNews(
        const std::string& entity_id, const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->news.begin(); it != comp->news.end(); ++it) {
        if (it->entry_id == entry_id) {
            comp->news.erase(it);
            return true;
        }
    }
    return false;
}

bool GalacticNewsSystem::clearNews(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->news.clear();
    return true;
}

bool GalacticNewsSystem::setSystemId(
        const std::string& entity_id, const std::string& system_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (system_id.empty()) return false;
    comp->system_id = system_id;
    return true;
}

bool GalacticNewsSystem::setDecayRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->news_decay_rate = rate;
    return true;
}

bool GalacticNewsSystem::setMaxEntries(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_entries = max;
    return true;
}

bool GalacticNewsSystem::setExpiryThreshold(const std::string& entity_id, float threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threshold <= 0.0f) return false;
    comp->expiry_threshold = threshold;
    return true;
}

int GalacticNewsSystem::getNewsCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->news.size());
}

bool GalacticNewsSystem::hasNews(
        const std::string& entity_id, const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->news) {
        if (e.entry_id == entry_id) return true;
    }
    return false;
}

std::string GalacticNewsSystem::getHeadline(
        const std::string& entity_id, const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& e : comp->news) {
        if (e.entry_id == entry_id) return e.headline;
    }
    return "";
}

float GalacticNewsSystem::getNewsAge(
        const std::string& entity_id, const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->news) {
        if (e.entry_id == entry_id) return e.age_seconds;
    }
    return 0.0f;
}

bool GalacticNewsSystem::isExpired(
        const std::string& entity_id, const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->news) {
        if (e.entry_id == entry_id) return e.is_expired;
    }
    return false;
}

int GalacticNewsSystem::getActiveCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->news) {
        if (!e.is_expired) ++count;
    }
    return count;
}

int GalacticNewsSystem::getCountByCategory(
        const std::string& entity_id,
        components::GalacticNewsState::NewsCategory cat) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->news) {
        if (e.category == cat) ++count;
    }
    return count;
}

int GalacticNewsSystem::getTotalPublished(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_published;
}

int GalacticNewsSystem::getTotalExpired(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_expired;
}

std::string GalacticNewsSystem::getSystemId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->system_id;
}

float GalacticNewsSystem::getDecayRate(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->news_decay_rate;
}

std::string GalacticNewsSystem::getSourceSystem(
        const std::string& entity_id, const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& e : comp->news) {
        if (e.entry_id == entry_id) return e.source_system;
    }
    return "";
}

components::GalacticNewsState::NewsCategory GalacticNewsSystem::getNewsCategory(
        const std::string& entity_id, const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::GalacticNewsState::NewsCategory::Conflict;
    for (const auto& e : comp->news) {
        if (e.entry_id == entry_id) return e.category;
    }
    return components::GalacticNewsState::NewsCategory::Conflict;
}

} // namespace systems
} // namespace atlas
