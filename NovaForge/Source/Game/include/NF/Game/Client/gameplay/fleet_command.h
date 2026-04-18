#pragma once
/**
 * @file fleet_command.h
 * @brief Fleet Command intent system — 6 high-level orders for the RTS layer.
 *
 * Orders are *intent*, not direct control.  Fleet AI translates each intent
 * into ship-level behavior.  The system is designed for use inside the
 * FLEET_COMMAND PlayerMode where time scale is 0.1–0.25×.
 *
 * Design rules:
 *  - Time scale: 0.15×
 *  - Direct weapon / thruster control is disabled
 *  - Camera is tactical-only
 *  - Exiting Fleet Command restores the exact prior flight state
 */

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace atlas::gameplay {

// ── Fleet intent vocabulary ──────────────────────────────────────────────────

enum class FleetIntent : uint8_t {
    ENGAGE,    ///< Seek and fight enemies
    DEFEND,    ///< Hold position, intercept threats
    ESCORT,    ///< Protect target entity
    RETREAT,   ///< Disengage intelligently
    HOLD,      ///< Freeze formation
    REGROUP    ///< Reform fleet to default doctrine
};

inline const char* FleetIntentName(FleetIntent intent) {
    switch (intent) {
        case FleetIntent::ENGAGE:  return "Engage";
        case FleetIntent::DEFEND:  return "Defend";
        case FleetIntent::ESCORT:  return "Escort";
        case FleetIntent::RETREAT: return "Retreat";
        case FleetIntent::HOLD:    return "Hold";
        case FleetIntent::REGROUP: return "Regroup";
    }
    return "Unknown";
}

// ── FleetOrder ───────────────────────────────────────────────────────────────

struct FleetOrder {
    FleetIntent  intent       = FleetIntent::HOLD;
    std::string  targetID;            ///< Entity to engage / escort / defend (empty = none)
    float        aggression   = 0.5f; ///< 0.0 (passive) – 1.0 (all-in)
    std::string  fleetID;             ///< Which fleet the order applies to
    std::string  wingID;              ///< Narrow scope to one wing (empty = all)
};

// ── FleetCommandSystem ───────────────────────────────────────────────────────

/**
 * Manages pending fleet orders and broadcasts them to AI layers.
 *
 * This is an in-memory client-side abstraction; the authoritative server
 * FleetSystem (cpp_server) receives orders via the network.
 */
class FleetCommandSystem {
public:
    using OrderCallback = std::function<void(const FleetOrder&)>;

    FleetCommandSystem() = default;

    // ── Order issuing ─────────────────────────────────────────────────

    /** Issue an order to a fleet (or specific wing). */
    void IssueOrder(const FleetOrder& order);

    /** Convenience overload — fills in fleet and wing from context. */
    void IssueOrder(FleetIntent intent,
                    const std::string& fleetID,
                    const std::string& targetID = "",
                    float aggression = 0.5f);

    // ── Order queries ──────────────────────────────────────────────────

    /** Get the last issued order (for UI display). */
    const FleetOrder& LastOrder() const { return m_lastOrder; }

    /** True if any order has been issued this session. */
    bool HasOrder() const { return m_hasOrder; }

    /** Number of orders issued since construction. */
    int OrderCount() const { return m_orderCount; }

    // ── Callbacks ──────────────────────────────────────────────────────

    /** Register a callback invoked whenever an order is issued. */
    void SetOrderCallback(OrderCallback cb) { m_callback = std::move(cb); }

    // ── Fleet Command Mode restrictions ───────────────────────────────

    /**
     * Returns true when Fleet Command mode is active (weapons & thrusters
     * disabled; time scale 0.15×).
     */
    bool IsFleetCommandModeActive() const { return m_modeActive; }
    void SetFleetCommandModeActive(bool active) { m_modeActive = active; }

    /** Fleet Command time scale (typically 0.15). */
    static constexpr float FLEET_COMMAND_TIME_SCALE = 0.15f;

private:
    FleetOrder   m_lastOrder;
    bool         m_hasOrder   = false;
    int          m_orderCount = 0;
    bool         m_modeActive = false;
    OrderCallback m_callback;
};

} // namespace atlas::gameplay
