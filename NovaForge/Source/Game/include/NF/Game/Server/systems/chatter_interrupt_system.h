#ifndef NOVAFORGE_SYSTEMS_CHATTER_INTERRUPT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CHATTER_INTERRUPT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class ChatterInterruptSystem
    : public ecs::SingleComponentSystem<components::ChatterInterruptState> {
public:
    explicit ChatterInterruptSystem(ecs::World* world);
    ~ChatterInterruptSystem() override = default;

    std::string getName() const override { return "ChatterInterruptSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Queue management ---
    bool queueLine(const std::string& entity_id,
                   const std::string& line_id,
                   const std::string& text,
                   float priority,
                   float duration,
                   bool interruptible,
                   const std::string& activity_tag);

    bool interruptWith(const std::string& entity_id,
                       const std::string& line_id,
                       const std::string& text,
                       float priority,
                       float duration,
                       const std::string& activity_tag);

    bool clearQueue(const std::string& entity_id);
    bool removeLine(const std::string& entity_id, const std::string& line_id);
    bool finishCurrentLine(const std::string& entity_id);

    // --- Configuration ---
    bool setMaxQueueSize(const std::string& entity_id, int max_size);

    // --- Queries ---
    bool        isSpeaking(const std::string& entity_id) const;
    std::string getActiveLineText(const std::string& entity_id) const;
    std::string getActiveLineId(const std::string& entity_id) const;
    float       getActivePriority(const std::string& entity_id) const;
    bool        isActiveInterruptible(const std::string& entity_id) const;
    bool        wasInterrupted(const std::string& entity_id) const;
    int         getQueueDepth(const std::string& entity_id) const;
    bool        hasLineInQueue(const std::string& entity_id,
                               const std::string& line_id) const;
    int         getTotalLinesQueued(const std::string& entity_id) const;
    int         getTotalLinesSpoken(const std::string& entity_id) const;
    int         getTotalInterrupts(const std::string& entity_id) const;
    int         getMaxQueueSize(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::ChatterInterruptState& comp,
                         float delta_time) override;

private:
    void startNextLine(components::ChatterInterruptState& comp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CHATTER_INTERRUPT_SYSTEM_H
