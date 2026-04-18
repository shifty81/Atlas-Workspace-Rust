/**
 * @file fleet_command.cpp
 * @brief FleetCommandSystem implementation.
 */

#include "gameplay/fleet_command.h"

namespace atlas::gameplay {

void FleetCommandSystem::IssueOrder(const FleetOrder& order) {
    m_lastOrder  = order;
    m_hasOrder   = true;
    ++m_orderCount;
    if (m_callback) m_callback(order);
}

void FleetCommandSystem::IssueOrder(FleetIntent intent,
                                    const std::string& fleetID,
                                    const std::string& targetID,
                                    float aggression) {
    FleetOrder order;
    order.intent     = intent;
    order.fleetID    = fleetID;
    order.targetID   = targetID;
    order.aggression = aggression;
    IssueOrder(order);
}

} // namespace atlas::gameplay
