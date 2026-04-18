#include "systems/chatter_interrupt_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ChatterInterruptSystem::ChatterInterruptSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ChatterInterruptSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::ChatterInterruptState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (!comp.is_speaking) return;

    comp.speak_timer += delta_time;
    if (comp.speak_timer >= comp.active_line.duration) {
        ++comp.total_lines_spoken;
        comp.is_speaking = false;
        comp.speak_timer = 0.0f;
        comp.was_interrupted = false;
        startNextLine(comp);
    }
}

void ChatterInterruptSystem::startNextLine(
        components::ChatterInterruptState& comp) {
    if (comp.queue.empty()) return;
    comp.active_line = comp.queue.front();
    comp.queue.erase(comp.queue.begin());
    comp.is_speaking  = true;
    comp.speak_timer  = 0.0f;
    comp.was_interrupted = false;
}

bool ChatterInterruptSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ChatterInterruptState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ChatterInterruptSystem::queueLine(const std::string& entity_id,
                                       const std::string& line_id,
                                       const std::string& text,
                                       float priority,
                                       float duration,
                                       bool interruptible,
                                       const std::string& activity_tag) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (line_id.empty()) return false;
    if (duration <= 0.0f) return false;

    ++comp->total_lines_queued;

    components::ChatterInterruptState::PendingLine line;
    line.line_id       = line_id;
    line.text          = text;
    line.priority      = priority;
    line.duration      = duration;
    line.interruptible = interruptible;
    line.activity_tag  = activity_tag;

    if (!comp->is_speaking) {
        comp->active_line     = line;
        comp->is_speaking     = true;
        comp->speak_timer     = 0.0f;
        comp->was_interrupted = false;
    } else {
        // Insert sorted by priority (highest first)
        auto it = comp->queue.begin();
        while (it != comp->queue.end() && it->priority >= priority) {
            ++it;
        }
        comp->queue.insert(it, line);

        // Trim queue to max_queue_size (drop lowest-priority tail)
        while (static_cast<int>(comp->queue.size()) > comp->max_queue_size) {
            comp->queue.pop_back();
        }
    }
    return true;
}

bool ChatterInterruptSystem::interruptWith(const std::string& entity_id,
                                           const std::string& line_id,
                                           const std::string& text,
                                           float priority,
                                           float duration,
                                           const std::string& activity_tag) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (line_id.empty()) return false;
    if (duration <= 0.0f) return false;
    if (!comp->is_speaking) return false;
    if (priority <= comp->active_line.priority) return false;
    if (!comp->active_line.interruptible) return false;

    comp->was_interrupted = true;
    ++comp->total_interrupts;
    ++comp->total_lines_queued;

    comp->active_line.line_id       = line_id;
    comp->active_line.text          = text;
    comp->active_line.priority      = priority;
    comp->active_line.duration      = duration;
    comp->active_line.interruptible = true;
    comp->active_line.activity_tag  = activity_tag;
    comp->speak_timer = 0.0f;
    return true;
}

bool ChatterInterruptSystem::clearQueue(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->queue.clear();
    return true;
}

bool ChatterInterruptSystem::removeLine(const std::string& entity_id,
                                        const std::string& line_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->queue.begin(), comp->queue.end(),
        [&](const components::ChatterInterruptState::PendingLine& l) {
            return l.line_id == line_id;
        });
    if (it == comp->queue.end()) return false;
    comp->queue.erase(it);
    return true;
}

bool ChatterInterruptSystem::finishCurrentLine(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->is_speaking) return false;
    ++comp->total_lines_spoken;
    comp->is_speaking     = false;
    comp->speak_timer     = 0.0f;
    comp->was_interrupted = false;
    startNextLine(*comp);
    return true;
}

bool ChatterInterruptSystem::setMaxQueueSize(const std::string& entity_id,
                                             int max_size) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_size < 1) return false;
    comp->max_queue_size = max_size;
    return true;
}

bool ChatterInterruptSystem::isSpeaking(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->is_speaking;
}

std::string ChatterInterruptSystem::getActiveLineText(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->active_line.text;
}

std::string ChatterInterruptSystem::getActiveLineId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->active_line.line_id;
}

float ChatterInterruptSystem::getActivePriority(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->active_line.priority;
}

bool ChatterInterruptSystem::isActiveInterruptible(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->active_line.interruptible;
}

bool ChatterInterruptSystem::wasInterrupted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->was_interrupted;
}

int ChatterInterruptSystem::getQueueDepth(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->queue.size());
}

bool ChatterInterruptSystem::hasLineInQueue(const std::string& entity_id,
                                            const std::string& line_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& l : comp->queue) {
        if (l.line_id == line_id) return true;
    }
    return false;
}

int ChatterInterruptSystem::getTotalLinesQueued(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_lines_queued;
}

int ChatterInterruptSystem::getTotalLinesSpoken(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_lines_spoken;
}

int ChatterInterruptSystem::getTotalInterrupts(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_interrupts;
}

int ChatterInterruptSystem::getMaxQueueSize(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_queue_size;
}

} // namespace systems
} // namespace atlas
