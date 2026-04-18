#ifndef NOVAFORGE_COMPONENTS_FLEET_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_FLEET_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

/**
 * @brief Fleet membership for an entity (attached to each fleet member)
 *
 * Tracks which fleet a player belongs to, their role, and any
 * active fleet bonuses being applied.
 */
class FleetMembership : public ecs::Component {
public:
    std::string fleet_id;
    std::string role = "Member";  // "FleetCommander", "WingCommander", "SquadCommander", "Member"
    std::string squad_id;
    std::string wing_id;
    std::map<std::string, float> active_bonuses;  // e.g. "armor_hp_bonus" -> 0.10
    
    COMPONENT_TYPE(FleetMembership)
};
/**
 * @brief Personality axes for AI fleet captains
 *
 * Eight axes capturing both behavioural style and deeper psychology.
 * The original four (aggression, sociability, optimism, professionalism)
 * describe observable behaviour; the new four (loyalty, paranoia, ambition,
 * adaptability) drive long-term decision making and relationship dynamics.
 */
class CaptainPersonality : public ecs::Component {
public:
    // Behavioural axes
    float aggression = 0.5f;        // 0=cautious, 1=bold
    float sociability = 0.5f;       // 0=quiet, 1=talkative
    float optimism = 0.5f;          // 0=grim, 1=hopeful
    float professionalism = 0.5f;   // 0=casual, 1=formal

    // Psychological axes (Phase 1)
    float loyalty = 0.5f;           // 0=self-serving, 1=devoted to fleet
    float paranoia = 0.5f;          // 0=trusting, 1=suspicious/cautious
    float ambition = 0.5f;          // 0=content, 1=driven/power-seeking
    float adaptability = 0.5f;      // 0=rigid, 1=flexible

    std::string captain_name;
    std::string faction;            // Solari, Veyren, Aurelian, Keldari

    COMPONENT_TYPE(CaptainPersonality)
};

/**
 * @brief Tracks fleet morale for an entity
 */
class FleetMorale : public ecs::Component {
public:
    float morale_score = 0.0f;              // clamped -100 to +100
    int wins = 0;
    int losses = 0;
    int ships_lost = 0;
    int times_saved_by_player = 0;
    int times_player_saved = 0;
    int missions_together = 0;
    std::string morale_state = "Steady";    // Inspired/Steady/Doubtful/Disengaged

    void updateMoraleScore() {
        morale_score = static_cast<float>(wins) * 1.0f
                     - static_cast<float>(losses) * 1.5f
                     - static_cast<float>(ships_lost) * 2.0f
                     + static_cast<float>(times_saved_by_player) * 1.2f;
        morale_score = std::clamp(morale_score, -100.0f, 100.0f);
        if (morale_score >= 50.0f) {
            morale_state = "Inspired";
        } else if (morale_score >= 0.0f) {
            morale_state = "Steady";
        } else if (morale_score >= -50.0f) {
            morale_state = "Doubtful";
        } else {
            morale_state = "Disengaged";
        }
    }

    COMPONENT_TYPE(FleetMorale)
};

/**
 * @brief Social graph for fleet captains
 */
class CaptainRelationship : public ecs::Component {
public:
    struct Relationship {
        std::string other_captain_id;
        float affinity = 0.0f;  // -100 to +100
    };

    std::vector<Relationship> relationships;

    float getAffinityWith(const std::string& id) const {
        for (const auto& r : relationships) {
            if (r.other_captain_id == id) {
                return r.affinity;
            }
        }
        return 0.0f;
    }

    void modifyAffinity(const std::string& id, float change) {
        for (auto& r : relationships) {
            if (r.other_captain_id == id) {
                r.affinity = std::clamp(r.affinity + change, -100.0f, 100.0f);
                return;
            }
        }
        Relationship rel;
        rel.other_captain_id = id;
        rel.affinity = std::clamp(change, -100.0f, 100.0f);
        relationships.push_back(rel);
    }

    COMPONENT_TYPE(CaptainRelationship)
};

/**
 * @brief Long-term emotional arcs
 */
class EmotionalState : public ecs::Component {
public:
    float confidence = 50.0f;       // 0-100
    float trust_in_player = 50.0f;  // 0-100
    float fatigue = 0.0f;           // 0-100
    float hope = 50.0f;             // 0-100

    COMPONENT_TYPE(EmotionalState)
};
/**
 * @brief Chatter state for an entity
 */
class FleetChatterState : public ecs::Component {
public:
    float chatter_cooldown = 0.0f;
    bool is_speaking = false;
    float priority = 0.0f;
    std::string current_activity = "Idle";  // Warp/Mining/Combat/Travel/Idle
    std::string last_line_spoken;
    int lines_spoken_total = 0;

    // Interruptible chatter support (Phase 9)
    float speaking_priority = 0.0f;   // priority of the line currently being spoken
    bool was_interrupted = false;     // true if last speech was cut off

    COMPONENT_TYPE(FleetChatterState)
};

/**
 * @brief Rumors heard/witnessed by a captain
 */
class RumorLog : public ecs::Component {
public:
    struct Rumor {
        std::string rumor_id;
        std::string text;
        float belief_strength = 0.5f;
        bool personally_witnessed = false;
        int times_heard = 0;
    };

    std::vector<Rumor> rumors;

    bool hasRumor(const std::string& id) const {
        for (const auto& r : rumors) {
            if (r.rumor_id == id) {
                return true;
            }
        }
        return false;
    }

    void addRumor(const std::string& id, const std::string& text, bool witnessed) {
        for (auto& r : rumors) {
            if (r.rumor_id == id) {
                r.times_heard++;
                return;
            }
        }
        Rumor rumor;
        rumor.rumor_id = id;
        rumor.text = text;
        rumor.belief_strength = 0.5f;
        rumor.personally_witnessed = witnessed;
        rumor.times_heard = 1;
        rumors.push_back(rumor);
    }

    COMPONENT_TYPE(RumorLog)
};

/**
 * @brief Aggregated fleet cargo
 */
class FleetCargoPool : public ecs::Component {
public:
    uint64_t total_capacity = 0;
    uint64_t used_capacity = 0;
    std::map<std::string, uint64_t> pooled_items;   // item_type -> quantity
    std::vector<std::string> contributor_ship_ids;

    COMPONENT_TYPE(FleetCargoPool)
};
/**
 * @brief Fleet formation assignment for an entity
 *
 * Stores the formation type the fleet is using and this member's
 * computed offset relative to the fleet commander.
 */
class FleetFormation : public ecs::Component {
public:
    enum class FormationType { None, Arrow, Line, Wedge, Spread, Diamond };

    FormationType formation = FormationType::Arrow;
    int slot_index = 0;           // position within the formation (0 = leader)
    float offset_x = 0.0f;       // metres relative to commander
    float offset_y = 0.0f;
    float offset_z = 0.0f;

    // Relationship-driven spacing modifier (Phase 9)
    // 1.0 = normal spacing, <1.0 = closer (friends), >1.0 = wider (grudges)
    float spacing_modifier = 1.0f;

    COMPONENT_TYPE(FleetFormation)
};

/**
 * @brief Persistent memory for an AI fleet captain
 *
 * Records significant events so the captain can reference them in chatter
 * and factor them into personality-driven decisions.
 */
class CaptainMemory : public ecs::Component {
public:
    struct MemoryEntry {
        std::string event_type;     // "combat_win", "combat_loss", "ship_lost", "saved_by_player", "warp_anomaly"
        std::string context;        // free-form detail (e.g. enemy name)
        float timestamp = 0.0f;     // in-game seconds since session start
        float emotional_weight = 0.0f; // -1=traumatic, +1=uplifting
    };

    std::vector<MemoryEntry> memories;
    int max_memories = 50;          // cap to prevent unbounded growth

    void addMemory(const std::string& event, const std::string& ctx,
                   float time, float weight) {
        if (static_cast<int>(memories.size()) >= max_memories) {
            memories.erase(memories.begin()); // drop oldest
        }
        memories.push_back({event, ctx, time, weight});
    }

    int countByType(const std::string& event_type) const {
        int n = 0;
        for (const auto& m : memories) {
            if (m.event_type == event_type) ++n;
        }
        return n;
    }

    float averageWeight() const {
        if (memories.empty()) return 0.0f;
        float sum = 0.0f;
        for (const auto& m : memories) sum += m.emotional_weight;
        return sum / static_cast<float>(memories.size());
    }

    COMPONENT_TYPE(CaptainMemory)
};
// ==================== Phase 9: Rumor-to-Questline, Departure, Transfer ====================

class RumorQuestline : public ecs::Component {
public:
    std::string rumor_id;
    std::string questline_id;
    int required_confirmations = 3;
    bool graduated = false;
    std::string quest_description;

    COMPONENT_TYPE(RumorQuestline)
};

class CaptainDepartureState : public ecs::Component {
public:
    enum class DeparturePhase { None, Grumbling, FormalRequest, Departing };

    DeparturePhase phase = DeparturePhase::None;
    float disagreement_score = 0.0f;
    float grumble_threshold = 5.0f;
    float request_threshold = 10.0f;
    float departure_timer = 0.0f;
    float departure_delay = 120.0f;
    bool player_acknowledged = false;

    COMPONENT_TYPE(CaptainDepartureState)
};

class CaptainTransferRequest : public ecs::Component {
public:
    enum class TransferType { BiggerShip, EscortOnly, RoleChange };

    TransferType request_type = TransferType::BiggerShip;
    bool request_pending = false;
    std::string requested_ship_class;
    std::string requested_role;
    float morale_at_request = 0.0f;

    COMPONENT_TYPE(CaptainTransferRequest)
};
// ==================== Phase 11: Fleet History ====================

struct FleetHistoryEntry {
    std::string event_type;
    std::string description;
    float timestamp = 0.0f;
    std::string involved_entity_id;
};

class FleetHistory : public ecs::Component {
public:
    std::vector<FleetHistoryEntry> events;
    int max_events = 100;

    void addEvent(const std::string& type, const std::string& desc,
                  float ts, const std::string& entity_id) {
        FleetHistoryEntry entry;
        entry.event_type = type;
        entry.description = desc;
        entry.timestamp = ts;
        entry.involved_entity_id = entity_id;
        events.push_back(entry);
        while (static_cast<int>(events.size()) > max_events) {
            events.erase(events.begin());
        }
    }

    COMPONENT_TYPE(FleetHistory)
};
// ==================== Phase 11: Fleet-as-Civilization ====================

/**
 * @brief Fleet progression stage tracking
 *
 * Tracks fleet growth through 3 stages:
 *   Early (max 5 ships)  — Player + 4 captains, basic personalities + chatter
 *   Mid   (max 15 ships) — 3 wings × 5, wing commanders, role specialization
 *   End   (max 25 ships) — 5 wings × 5, full doctrine (mining, salvage, logistics, escort, construction)
 */
class FleetProgression : public ecs::Component {
public:
    enum class Stage { Early, Mid, End };

    Stage stage = Stage::Early;
    int max_ships = 5;
    int max_wings = 1;
    int current_ship_count = 1;       // starts with player ship
    int ships_per_wing = 5;

    // Stage thresholds (measured by cumulative fleet experience)
    float fleet_experience = 0.0f;    // accumulated from missions, combat, etc.
    float mid_threshold = 100.0f;     // XP required to unlock Mid stage
    float end_threshold = 500.0f;     // XP required to unlock End stage

    // Role specialization unlocks (Mid stage+)
    bool mining_wing_unlocked = false;
    bool combat_wing_unlocked = false;
    bool logistics_wing_unlocked = false;
    bool salvage_wing_unlocked = false;   // End stage
    bool construction_wing_unlocked = false; // End stage

    void updateStage() {
        if (fleet_experience >= end_threshold) {
            stage = Stage::End;
            max_ships = 25;
            max_wings = 5;
        } else if (fleet_experience >= mid_threshold) {
            stage = Stage::Mid;
            max_ships = 15;
            max_wings = 3;
        } else {
            stage = Stage::Early;
            max_ships = 5;
            max_wings = 1;
        }
    }

    COMPONENT_TYPE(FleetProgression)
};

/**
 * @brief Station deployment component
 *
 * Attached to ships capable of deploying into permanent stations.
 * Tracks deployment state, attached modules, and system upgrade effects.
 */
class StationDeployment : public ecs::Component {
public:
    enum class DeployState { Mobile, Deploying, Deployed };

    DeployState deploy_state = DeployState::Mobile;
    float deploy_timer = 0.0f;          // seconds remaining in deployment
    float deploy_duration = 300.0f;     // 5 minutes to deploy

    // Location (set on deployment)
    std::string system_id;
    float deploy_x = 0.0f;
    float deploy_y = 0.0f;
    float deploy_z = 0.0f;

    // Attached station modules (module_type -> count)
    std::map<std::string, int> attached_modules;
    int max_module_slots = 3;

    // System upgrading effects
    float security_bonus = 0.0f;        // +security to the system
    float economy_bonus = 0.0f;         // +economy to the system
    float resource_bonus = 0.0f;        // +resource availability

    int getTotalAttachedModules() const {
        int total = 0;
        for (const auto& pair : attached_modules)
            total += pair.second;
        return total;
    }

    bool canAttachModule() const {
        return getTotalAttachedModules() < max_module_slots;
    }

    COMPONENT_TYPE(StationDeployment)
};

/**
 * @brief Fleet warp formation state
 *
 * Extends FleetFormation with warp-specific behavior:
 * - Formation type selection based on ship class
 * - Breathing oscillation for organic feel (0.02–0.05 Hz)
 * - Visual interaction data (distortion bending, wake ripples)
 */
class FleetWarpState : public ecs::Component {
public:
    enum class WarpFormationType { TightEchelon, LooseDiamond, WideCapital };

    WarpFormationType warp_formation = WarpFormationType::TightEchelon;
    bool in_fleet_warp = false;

    // Breathing oscillation (organic feel)
    float breathing_frequency = 0.03f;  // Hz (0.02–0.05)
    float breathing_amplitude = 0.1f;   // fraction of spacing
    float breathing_phase = 0.0f;       // current phase (radians)

    // Visual interaction
    float distortion_bend = 0.0f;       // warp distortion bending factor (mass-based)
    float wake_ripple = 0.0f;           // wake ripple intensity

    // Ship class for formation selection
    std::string ship_class = "Frigate";

    float computeBreathingOffset(float elapsed_time) const {
        static constexpr float kTwoPi = 6.2831853f;
        float phase = breathing_phase + elapsed_time * breathing_frequency * kTwoPi;
        return breathing_amplitude * std::sin(phase);
    }

    COMPONENT_TYPE(FleetWarpState)
};

/**
 * @brief Fleet civilization-scale tracking
 *
 * Tracks the metrics required for a fleet to reach civilization status:
 * titan threshold, fleet stability, and persistent history.
 */
class FleetCivilization : public ecs::Component {
public:
    // Titan threshold requirements
    bool has_stable_logistics = false;     // sustained supply chain
    bool has_loyal_captains = false;       // average morale >= "Steady"
    bool has_fleet_history = false;        // >= 20 missions completed together
    bool has_fleet_industry = false;       // fleet-scale manufacturing active

    // Civilization metrics
    int total_missions_completed = 0;
    int total_stations_deployed = 0;
    float fleet_stability_score = 0.0f;   // 0-100 stability index
    float economic_output = 0.0f;         // cumulative fleet economic value

    // Persistent history
    int major_events_count = 0;
    int ships_ever_owned = 0;
    int captains_ever_served = 0;

    bool isCivilizationThresholdMet() const {
        return has_stable_logistics && has_loyal_captains &&
               has_fleet_history && has_fleet_industry;
    }

    COMPONENT_TYPE(FleetCivilization)
};

/**
 * @brief Fleet doctrine - composition template for organized fleet management
 *
 * Defines ship class requirements, role assignments, and minimum fleet
 * composition for coordinated operations.
 */
class FleetDoctrine : public ecs::Component {
public:
    struct DoctrineSlot {
        std::string role;           // "DPS", "Logistics", "Tackle", "EWAR", "Scout", "Command"
        std::string ship_class;     // "Frigate", "Cruiser", "Battleship", etc.
        int min_count = 0;
        int max_count = 1;
        int current_count = 0;
        bool required = false;      // must be filled for doctrine to be "ready"
    };

    std::string doctrine_name;
    std::string doctrine_id;
    std::vector<DoctrineSlot> slots;
    float readiness = 0.0f;         // 0.0-1.0, fraction of required slots filled
    bool is_locked = false;         // prevent modifications during operation
    int total_ship_target = 0;      // ideal fleet size
    int current_ship_count = 0;     // actual ships assigned

    COMPONENT_TYPE(FleetDoctrine)
};


/**
 * @brief Wing state tracking for fleet wing management
 */
class WingState : public ecs::Component {
public:
    struct Wing {
        std::string wing_id;
        std::string role;           // "Mining", "Combat", "Logistics", "Salvage", "Construction"
        std::string commander_id;   // entity id of wing commander
        std::vector<std::string> members;  // entity ids of wing members
        float morale = 50.0f;       // wing-level morale (0-100)
    };

    std::vector<Wing> wings;

    Wing* getWing(const std::string& wing_id) {
        for (auto& w : wings) {
            if (w.wing_id == wing_id) return &w;
        }
        return nullptr;
    }

    const Wing* getWing(const std::string& wing_id) const {
        for (const auto& w : wings) {
            if (w.wing_id == wing_id) return &w;
        }
        return nullptr;
    }

    COMPONENT_TYPE(WingState)
};

// ==================== Fleet Squad ====================

/**
 * @brief Group AI abstraction for fleet squads (Future Considerations)
 *
 * Represents a squad within a fleet with members, formation, role,
 * and computed cohesion/effectiveness metrics.
 */
class FleetSquad : public ecs::Component {
public:
    enum class SquadRole { Assault, Defense, Support, Scout, Reserve };
    enum class SquadFormation { Line, Wedge, Circle, Spread, Stack };

    std::string squad_id;
    std::string squad_leader_id;
    SquadRole role = SquadRole::Assault;
    std::vector<std::string> member_ids;
    int max_members = 5;
    SquadFormation formation = SquadFormation::Line;
    float cohesion = 1.0f;
    float effectiveness = 1.0f;
    bool is_active = true;

    static std::string roleToString(SquadRole r) {
        switch (r) {
            case SquadRole::Assault: return "assault";
            case SquadRole::Defense: return "defense";
            case SquadRole::Support: return "support";
            case SquadRole::Scout: return "scout";
            case SquadRole::Reserve: return "reserve";
            default: return "unknown";
        }
    }

    static std::string formationToString(SquadFormation f) {
        switch (f) {
            case SquadFormation::Line: return "line";
            case SquadFormation::Wedge: return "wedge";
            case SquadFormation::Circle: return "circle";
            case SquadFormation::Spread: return "spread";
            case SquadFormation::Stack: return "stack";
            default: return "unknown";
        }
    }

    COMPONENT_TYPE(FleetSquad)
};

// ==================== Commander Disagreement ====================

/**
 * @brief Tracks disagreements between wing commanders in a fleet
 *
 * When two wing commanders with conflicting personality traits face
 * a strategic decision, a disagreement arises that must be resolved.
 */
class CommanderDisagreement : public ecs::Component {
public:
    enum class Topic { Strategy, Target, Formation, Retreat, LootSplit };
    enum class Resolution { None, Vote, AuthorityOverride, Compromise, Escalated };
    enum class Severity { Minor, Moderate, Serious, Critical };

    struct Disagreement {
        std::string commander_a_id;
        std::string commander_b_id;
        Topic topic = Topic::Strategy;
        Severity severity = Severity::Minor;
        Resolution resolution = Resolution::None;
        float timer = 0.0f;           // seconds since raised
        float escalation_threshold = 30.0f;  // escalate after this many seconds
        bool resolved = false;
        float morale_impact = 0.0f;   // accumulated morale impact
    };

    std::vector<Disagreement> disagreements;
    int total_disagreements = 0;
    int total_resolved = 0;
    float fleet_tension = 0.0f;   // 0-100, accumulated tension

    static std::string topicToString(Topic t) {
        switch (t) {
            case Topic::Strategy: return "Strategy";
            case Topic::Target: return "Target";
            case Topic::Formation: return "Formation";
            case Topic::Retreat: return "Retreat";
            case Topic::LootSplit: return "LootSplit";
            default: return "Unknown";
        }
    }

    static std::string severityToString(Severity s) {
        switch (s) {
            case Severity::Minor: return "Minor";
            case Severity::Moderate: return "Moderate";
            case Severity::Serious: return "Serious";
            case Severity::Critical: return "Critical";
            default: return "Unknown";
        }
    }

    static std::string resolutionToString(Resolution r) {
        switch (r) {
            case Resolution::None: return "None";
            case Resolution::Vote: return "Vote";
            case Resolution::AuthorityOverride: return "AuthorityOverride";
            case Resolution::Compromise: return "Compromise";
            case Resolution::Escalated: return "Escalated";
            default: return "Unknown";
        }
    }

    COMPONENT_TYPE(CommanderDisagreement)
};

// ==================== Captain Background ====================

/**
 * @brief Captain backstory affecting stats, dialogue, and role preferences
 *
 * Each captain has a background archetype from their life before joining
 * the fleet. This affects their stat modifiers, preferred fleet roles,
 * personality weight adjustments, and dialogue flavor.
 */
class CaptainBackground : public ecs::Component {
public:
    enum class BackgroundType {
        FormerMiner, ExMilitary, Smuggler, Scientist,
        Noble, Colonist, BountyHunter, Trader
    };

    BackgroundType background = BackgroundType::Trader;
    std::string origin_system;       // where the captain came from
    int years_experience = 0;        // years of prior experience
    float aggression_modifier = 0.0f;    // added to personality aggression
    float loyalty_modifier = 0.0f;       // added to personality loyalty
    float professionalism_modifier = 0.0f;
    std::string preferred_role;      // preferred fleet role
    std::string dialogue_flavor;     // dialogue prefix/style marker
    float skill_bonus = 0.0f;       // bonus to relevant skill category
    std::string skill_category;      // which skill category gets bonus

    static std::string typeToString(BackgroundType t) {
        switch (t) {
            case BackgroundType::FormerMiner: return "FormerMiner";
            case BackgroundType::ExMilitary: return "ExMilitary";
            case BackgroundType::Smuggler: return "Smuggler";
            case BackgroundType::Scientist: return "Scientist";
            case BackgroundType::Noble: return "Noble";
            case BackgroundType::Colonist: return "Colonist";
            case BackgroundType::BountyHunter: return "BountyHunter";
            case BackgroundType::Trader: return "Trader";
            default: return "Unknown";
        }
    }

    static BackgroundType stringToType(const std::string& s) {
        if (s == "FormerMiner") return BackgroundType::FormerMiner;
        if (s == "ExMilitary") return BackgroundType::ExMilitary;
        if (s == "Smuggler") return BackgroundType::Smuggler;
        if (s == "Scientist") return BackgroundType::Scientist;
        if (s == "Noble") return BackgroundType::Noble;
        if (s == "Colonist") return BackgroundType::Colonist;
        if (s == "BountyHunter") return BackgroundType::BountyHunter;
        if (s == "Trader") return BackgroundType::Trader;
        return BackgroundType::Trader;
    }

    COMPONENT_TYPE(CaptainBackground)
};

// ==================== Phase 9: Positional Audio ====================

/**
 * @brief 3D positional audio source for fleet member voices
 *
 * Stores the computed 3D position (derived from formation offset),
 * attenuation parameters, and warp tunnel reverb settings.
 * FleetPositionalAudioSystem writes source_x/y/z, attenuation,
 * reverb_wet_mix, and reverb_decay each frame.
 */
class PositionalAudioSource : public ecs::Component {
public:
    // Listener (player/camera) position — set externally
    float listener_x = 0.0f;
    float listener_y = 0.0f;
    float listener_z = 0.0f;

    // Computed source position (from formation offset)
    float source_x = 0.0f;
    float source_y = 0.0f;
    float source_z = 0.0f;

    // Attenuation model
    float min_range = 5.0f;     // full volume within this distance (metres)
    float max_range = 200.0f;   // silent beyond this distance (metres)
    float attenuation = 1.0f;   // computed attenuation (0–1)

    // Warp tunnel reverb (set by system when in warp)
    float reverb_wet_mix = 0.0f;  // 0=dry, 1=fully wet
    float reverb_decay = 0.0f;    // decay time in seconds

    COMPONENT_TYPE(PositionalAudioSource)
};

/**
 * @brief Fleet morale crisis resolution through ideology alignment and fracture mechanics
 */
class FleetMoraleResolution : public ecs::Component {
public:
    std::string fleet_id;
    float fleet_morale = 50.0f;
    float fracture_threshold = 20.0f;
    float ideology_alignment = 0.5f;
    // Resolution methods: Compromise, AuthorityOverride, Vote, Mediation
    std::string current_resolution = "None";
    bool crisis_active = false;
    float recovery_rate = 1.0f;
    int fractures_triggered = 0;
    int resolutions_applied = 0;
    int departures = 0;
    float crisis_duration = 0.0f;
    float max_crisis_duration = 60.0f;
    bool active = true;

    COMPONENT_TYPE(FleetMoraleResolution)
};

/**
 * @brief Fleet logistics supply chain with fuel and ammo depots
 *
 * Manages supply depots that consume fuel and ammo over time.
 */
class FleetSupplyLine : public ecs::Component {
public:
    struct SupplyDepot {
        std::string depot_id;
        std::string system_id;
        float fuel_level = 100.0f;   // 0-100
        float ammo_level = 100.0f;   // 0-100
        float capacity = 100.0f;
    };

    std::vector<SupplyDepot> depots;
    int max_depots = 10;
    float consumption_rate = 1.0f; // per second
    int total_resupplies = 0;
    float total_fuel_consumed = 0.0f;
    float total_ammo_consumed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(FleetSupplyLine)
};

// ==================== Fleet Coordination ====================

/**
 * @brief Fleet-level tactical coordination with engagement orders
 *
 * Coordinates fleet-wide tactics including engagement orders (Hold/Engage/
 * FocusFire/Retreat/Regroup/FreeEngage), target priority assignments,
 * combat readiness tracking, and formation coherence degradation.
 */
class FleetCoordination : public ecs::Component {
public:
    enum class CoordinationOrder { Hold, Engage, FocusFire, Retreat, Regroup, FreeEngage };

    struct TargetAssignment {
        std::string target_id;
        int priority = 1;            // 1 = lowest, 5 = highest
        int assigned_ships = 0;
    };

    std::string fleet_id;
    CoordinationOrder current_order = CoordinationOrder::Hold;
    std::vector<TargetAssignment> target_assignments;
    std::vector<std::string> participating_ships;
    float formation_coherence = 1.0f;   // 0-1, degrades in combat
    float combat_readiness = 1.0f;      // coherence × morale
    float morale_factor = 1.0f;         // 0-1
    float order_duration = 0.0f;
    int max_targets = 10;
    int max_ships = 50;
    int total_orders_issued = 0;
    int total_targets_assigned = 0;
    float total_time_in_combat = 0.0f;
    float coherence_decay_rate = 0.01f; // per second during combat
    bool in_combat = false;
    bool active = true;

    COMPONENT_TYPE(FleetCoordination)
};

/**
 * @brief Fleet formation flight slot and cohesion tracking
 *
 * Each entity in a fleet formation holds a slot with a relative
 * position offset from the formation leader.  The system tracks
 * how well each member maintains their assigned position and
 * applies cohesion bonuses when the formation holds tight.
 */
class FormationFlight : public ecs::Component {
public:
    enum class FormationType { Line, Wedge, Sphere, Wall, Echelon };
    enum class SlotStatus { Holding, Drifting, Broken, Reforming };

    std::string fleet_id;
    std::string leader_id;
    FormationType formation = FormationType::Wedge;
    int slot_index = 0;
    double offset_x = 0.0, offset_y = 0.0, offset_z = 0.0;
    double actual_x = 0.0, actual_y = 0.0, actual_z = 0.0;
    float cohesion = 1.0f;             // 0.0–1.0, how well position is held
    float cohesion_bonus = 0.0f;       // performance bonus from cohesion
    float max_drift = 50.0f;           // max distance before slot is "Broken"
    SlotStatus status = SlotStatus::Holding;
    int formation_breaks = 0;
    int formation_reforms = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(FormationFlight)
};

// ==================== NPC Fleet Composition System ====================

/**
 * @brief Defines and spawns NPC fleet templates with mixed ship roles
 *
 * Each composition template specifies ship types, roles, and counts.
 * The system spawns fleets from templates, assigns roles (DPS, tank,
 * support, commander), and scales difficulty by security level.
 */
class NpcFleetComposition : public ecs::Component {
public:
    enum class ShipRole { DPS, Tank, Support, Commander, Scout };
    enum class DifficultyTier { Easy, Medium, Hard, Elite };

    struct ShipEntry {
        std::string ship_type;       // "frigate", "cruiser", "battleship"
        ShipRole role = ShipRole::DPS;
        float threat_value = 1.0f;   // contribution to fleet threat
        bool is_commander = false;
    };

    std::string template_id;
    std::string template_name;       // "Pirate Patrol", "Mining Escort"
    DifficultyTier difficulty = DifficultyTier::Easy;
    std::vector<ShipEntry> ship_roster;
    int max_ships = 5;
    float total_threat = 0.0f;       // sum of all ship threat values
    float security_level = 1.0f;     // system security 0.0-1.0
    float difficulty_scale = 1.0f;   // multiplier applied to threat
    int fleets_spawned = 0;
    int fleets_destroyed = 0;
    float spawn_cooldown = 300.0f;   // seconds between spawns
    float cooldown_timer = 0.0f;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(NpcFleetComposition)
};

// ---------------------------------------------------------------------------
// FleetAfterActionReport — post-combat fleet performance summary
// ---------------------------------------------------------------------------
/**
 * @brief Records and summarises fleet performance in a combat engagement.
 *
 * Opened via startReport(), updated with recordKill/recordLoss/
 * recordDamageDealt/recordDamageReceived/recordLootShared per fleet
 * member, then sealed with finalizeReport().  After finalisation the
 * data is immutable.  getMVP() returns the pilot_id with the highest
 * damage dealt.
 */
class FleetAfterActionReport : public ecs::Component {
public:
    enum class State { Idle, Recording, Finalized };

    struct MemberStats {
        std::string pilot_id;
        int   kills           = 0;
        int   losses          = 0;
        float damage_dealt    = 0.0f;
        float damage_received = 0.0f;
        float loot_shared     = 0.0f;
    };

    State                     state            = State::Idle;
    std::vector<MemberStats>  members;
    int                       total_kills      = 0;
    int                       total_losses     = 0;
    float                     total_damage     = 0.0f;  // fleet aggregate
    float                     total_loot       = 0.0f;
    int                       max_members      = 25;
    int                       total_reports    = 0;
    float                     duration         = 0.0f;  // engagement length (s)
    float                     elapsed          = 0.0f;
    bool                      active           = true;

    COMPONENT_TYPE(FleetAfterActionReport)
};

// ---------------------------------------------------------------------------
// FleetBroadcastState — fleet broadcast message management
// ---------------------------------------------------------------------------
/**
 * @brief Tracks fleet broadcast messages (target, align to, warp to,
 *        need reps, etc.) for coordination among fleet members.
 *
 * Each Broadcast has an id, type (Target/AlignTo/WarpTo/NeedShieldReps/
 * NeedArmorReps/NeedCapacitor/EnemySpotted/HoldPosition), sender_id,
 * target_label, and a ttl countdown.  Broadcasts expire when ttl reaches 0
 * and are removed on the next tick.  max_broadcasts caps the active list
 * (default 20).  total_sent / total_expired track lifetime statistics.
 */
class FleetBroadcastState : public ecs::Component {
public:
    enum class BroadcastType {
        Target,
        AlignTo,
        WarpTo,
        NeedShieldReps,
        NeedArmorReps,
        NeedCapacitor,
        EnemySpotted,
        HoldPosition
    };

    struct Broadcast {
        std::string   broadcast_id;
        BroadcastType type        = BroadcastType::Target;
        std::string   sender_id;
        std::string   target_label;  // human-readable description
        float         ttl         = 30.0f;  // seconds until expiry
    };

    std::string fleet_id;
    std::vector<Broadcast> broadcasts;
    int   max_broadcasts   = 20;
    int   total_sent       = 0;
    int   total_expired    = 0;
    float elapsed          = 0.0f;
    bool  active           = true;

    COMPONENT_TYPE(FleetBroadcastState)
};

// ---------------------------------------------------------------------------
// FleetNormState — emergent fleet habit tracking
// ---------------------------------------------------------------------------
/**
 * @brief Tracks emergent norms (habits, traditions, informal rules) that
 *        form within a fleet through repeated actions.
 *
 * When a trigger_action is recorded enough times (≥ activation_threshold),
 * the matching Norm becomes active.  strength (0-1) scales with how often
 * the norm has been reinforced, providing a natural "entrenchment" feel.
 */
class FleetNormState : public ecs::Component {
public:
    struct Norm {
        std::string norm_id;
        std::string name;
        std::string trigger_action;  // action string that increments this norm
        int         activation_count = 0;
        float       strength         = 0.0f;  // 0-1
        bool        active           = false;
    };

    std::string fleet_id;
    std::vector<Norm> norms;
    int   activation_threshold = 5;
    int   max_norms            = 10;
    int   total_norms_formed   = 0;
    float elapsed              = 0.0f;
    bool  active_flag          = true;

    COMPONENT_TYPE(FleetNormState)
};

// CaptainPsychologyState — captain personality axes for fleet behavior
// ---------------------------------------------------------------------------
/**
 * @brief Four personality axes (aggression, caution, loyalty, greed) that
 *        influence fleet behavior decisions such as engagement willingness,
 *        retreat thresholds, and loot distribution preferences.
 *
 * Each axis is a float in [0, 1]. Personality drift occurs over time via
 * the decay_rate toward each axis's baseline, allowing events to shift
 * personality temporarily.
 */
class CaptainPsychologyState : public ecs::Component {
public:
    struct PersonalityAxis {
        float current  = 0.5f;   // current value [0, 1]
        float baseline = 0.5f;   // resting value axes drift toward
    };

    enum class EventType {
        Victory,
        Defeat,
        LootGained,
        AllyLost,
        OrderGiven,
        LongIdle
    };

    std::string captain_id;
    PersonalityAxis aggression;    // willingness to engage in combat
    PersonalityAxis caution;       // tendency to retreat or avoid risk
    PersonalityAxis loyalty;       // faithfulness to fleet/commander
    PersonalityAxis greed;         // priority on loot/material gain

    float mood           = 0.5f;  // 0-1 composite mood indicator
    float stress          = 0.0f;  // 0-1 stress accumulator
    float stress_decay    = 0.05f; // stress reduction per second
    float drift_rate      = 0.01f; // personality drift toward baseline per second
    int   events_processed = 0;
    int   total_shifts     = 0;     // number of personality shifts applied
    float elapsed         = 0.0f;
    bool  active          = true;

    COMPONENT_TYPE(CaptainPsychologyState)
};

// FleetMoraleState — fleet memory and morale tracking
// ---------------------------------------------------------------------------
/**
 * @brief Tracks fleet-level morale, cohesion, and event memory.
 *
 * Morale is influenced by fleet events (victories, defeats, idle time).
 * The event_log stores recent events for fleet memory, which influences
 * chatter, captain behavior, and willingness to follow risky orders.
 * Cohesion reflects how well the fleet works together.
 */
class FleetMoraleState : public ecs::Component {
public:
    enum class MoraleEvent {
        Victory,
        Defeat,
        AllyDestroyed,
        SuccessfulRetreat,
        LootShared,
        LongIdle,
        OrderFollowed,
        OrderIgnored
    };

    struct EventEntry {
        MoraleEvent event;
        float       timestamp = 0.0f;
        float       impact    = 0.0f;
    };

    std::string fleet_id;
    float morale          = 0.5f;   // 0-1 overall morale level
    float cohesion        = 0.5f;   // 0-1 how unified the fleet is
    float morale_decay    = 0.01f;  // passive morale decay per second toward baseline
    float morale_baseline = 0.5f;   // resting morale level
    float cohesion_decay  = 0.005f; // passive cohesion decay per second toward baseline
    float cohesion_baseline = 0.5f;
    int   max_event_log   = 20;     // max events in memory
    int   total_events    = 0;
    int   victories       = 0;
    int   defeats         = 0;
    float elapsed         = 0.0f;
    bool  active          = true;
    std::vector<EventEntry> event_log;

    COMPONENT_TYPE(FleetMoraleState)
};

// ---------------------------------------------------------------------------
// ChatterInterruptState — priority-based interruptible chatter queue
// ---------------------------------------------------------------------------
/**
 * @brief Manages a priority-sorted queue of chatter lines for a captain.
 *
 * Higher-priority events (combat alerts, warp exits, anomalies) can interrupt
 * lower-priority lines that are marked interruptible. Each line has a speak
 * duration; when it expires the next queued line auto-starts.
 */
class ChatterInterruptState : public ecs::Component {
public:
    struct PendingLine {
        std::string line_id;
        std::string text;
        std::string activity_tag;   // "Combat", "Warp", "Idle", etc.
        float       priority     = 0.0f;
        float       duration     = 3.0f;  // seconds to speak this line
        bool        interruptible = true;
    };

    PendingLine active_line;
    bool        is_speaking         = false;
    float       speak_timer         = 0.0f;  // time elapsed speaking current line
    bool        was_interrupted     = false;
    std::vector<PendingLine> queue;           // pending lines not yet active

    int         max_queue_size      = 5;
    int         total_lines_queued  = 0;
    int         total_lines_spoken  = 0;
    int         total_interrupts    = 0;
    float       elapsed             = 0.0f;
    bool        active              = true;

    COMPONENT_TYPE(ChatterInterruptState)
};

// ---------------------------------------------------------------------------
// FactionBehaviorState — faction personality profile for a captain/entity
// ---------------------------------------------------------------------------
/**
 * @brief Encodes one of four faction personality profiles affecting morale,
 *        chatter rate, activity preferences, and departure thresholds.
 *
 * Profiles: Industrial (efficiency), Militaristic (victory), Nomadic
 * (exploration), Corporate (success metrics).  setProfile() applies the
 * canonical defaults for each profile; individual fields can then be
 * fine-tuned.
 */
class FactionBehaviorState : public ecs::Component {
public:
    enum class FactionProfile {
        Industrial,    // efficiency, mining/trade focus, low combat chatter
        Militaristic,  // victory, combat focus, high chatter
        Nomadic,       // exploration focus, superstitious, moderate chatter
        Corporate,     // success metrics, trade focus, formal low chatter
        Neutral        // balanced defaults
    };

    enum class MoraleDriver {
        Efficiency,
        Victory,
        Exploration,
        Success,
        None
    };

    FactionProfile profile            = FactionProfile::Neutral;
    MoraleDriver   morale_driver      = MoraleDriver::None;
    float          morale_bias        = 0.0f;    // additive morale modifier
    float          chatter_rate_mult  = 1.0f;    // multiplier for chatter frequency
    float          combat_preference  = 0.5f;    // 0-1
    float          mining_preference  = 0.5f;    // 0-1
    float          exploration_preference = 0.5f; // 0-1
    float          trade_preference   = 0.5f;    // 0-1
    float          departure_threshold = -50.0f; // morale below which departure risked
    std::string    faction_id;
    int            total_profile_changes = 0;
    float          elapsed            = 0.0f;
    bool           active             = true;

    COMPONENT_TYPE(FactionBehaviorState)
};

// ---------------------------------------------------------------------------
// OperationalWearState — long-deployment operational wear tracking
// ---------------------------------------------------------------------------
/**
 * @brief Tracks accumulated wear on a ship from extended deployments.
 *
 * Once a ship has been deployed beyond rotation_threshold, passive wear
 * accumulates each tick.  Field repairs reduce wear but leave hidden
 * penalties; only a full dock repair clears all wear and penalties.
 * Wear drives fuel_inefficiency, repair_delay_mult, and crew_stress
 * derived values that are recomputed on each tick.
 */
class OperationalWearState : public ecs::Component {
public:
    struct WearEvent {
        std::string event_id;
        std::string event_type;   // e.g. "Combat", "Mining", "Transit"
        float wear_amount = 0.0f;
    };

    float wear_level            = 0.0f;    // 0-100
    float fuel_inefficiency     = 0.0f;    // 0-0.5 additive fuel penalty
    float repair_delay_mult     = 1.0f;    // >= 1.0 repair time multiplier
    float crew_stress           = 0.0f;    // 0-100
    float deployment_duration   = 0.0f;    // total seconds deployed this session
    float rotation_threshold    = 86400.0f;// seconds before passive wear begins
    float passive_wear_rate     = 0.005f;  // wear/s accumulated past threshold
    float recovery_rate         = 5.0f;    // wear/s recovered while docked
    bool  is_docked             = false;
    bool  is_worn               = false;   // wear_level >= 50
    bool  is_critical           = false;   // wear_level >= 85
    bool  has_hidden_penalties  = false;   // set after any field repair
    int   total_field_repairs   = 0;
    int   total_dock_repairs    = 0;
    std::string ship_id;
    std::vector<WearEvent> event_log;
    int   max_events            = 20;
    float elapsed               = 0.0f;
    bool  active                = true;

    COMPONENT_TYPE(OperationalWearState)
};

// ---------------------------------------------------------------------------
// PostEventAnalysisState — post-event fleet debrief with blame attribution
// ---------------------------------------------------------------------------
/**
 * @brief Records post-event fleet analysis sessions where captains assign
 *        blame and derive lessons from major engagements or failures.
 *
 * An analysis is started with startAnalysis(), blame entries are added
 * via addBlame(), and finalized via finalizeAnalysis().  The system also
 * auto-finalizes when the analysis_timer reaches analysis_duration.
 * Completed analyses and extracted lessons are stored persistently.
 */
class PostEventAnalysisState : public ecs::Component {
public:
    struct CaptainBlame {
        std::string captain_id;
        std::string reason;
        float       blame_weight = 0.0f;   // 0-1
    };

    struct EventAnalysis {
        std::string analysis_id;
        std::string event_description;
        std::string conclusion;
        int         agreement_count = 0;
        int         dissent_count   = 0;
        bool        is_finalized    = false;
        std::vector<CaptainBlame> blame;
    };

    // In-progress analysis state
    std::string current_event_id;
    bool  analysis_in_progress  = false;
    float analysis_timer        = 0.0f;
    float analysis_duration     = 5.0f;   // seconds until auto-finalize
    std::vector<CaptainBlame> pending_blame;  // blame pending finalization

    // Completed analyses
    std::vector<EventAnalysis> analyses;
    int max_analyses            = 20;

    // Lessons learned
    std::vector<std::string> lessons;
    int max_lessons             = 50;

    int   total_analyses_completed = 0;
    int   total_lessons_learned    = 0;
    std::string fleet_id;
    float elapsed               = 0.0f;
    bool  active                = true;

    COMPONENT_TYPE(PostEventAnalysisState)
};

// FleetFractureRecoveryState
class FleetFractureRecoveryState : public ecs::Component {
public:
    enum class RecoveryPhase { Stable, Cracking, Fractured, Recovering, Rebuilt };

    struct FractureEvent {
        std::string event_id;
        std::string description;
        int ships_lost = 0;
        int captains_departed = 0;
        float morale_delta = 0.0f;
        float severity = 0.0f;
    };

    struct RecoveryMilestone {
        std::string milestone_id;
        std::string description;
        float required_value = 0.0f;
        float current_value = 0.0f;
        bool is_achieved = false;
    };

    RecoveryPhase phase = RecoveryPhase::Stable;
    float fracture_score = 0.0f;
    float recovery_momentum = 0.0f;
    float fragility = 0.3f;
    float fracture_threshold = 60.0f;
    float recovery_threshold = 20.0f;
    float cracking_threshold = 30.0f;

    std::vector<FractureEvent> fracture_log;
    int max_log = 20;
    std::vector<RecoveryMilestone> milestones;
    int max_milestones = 10;

    int total_fractures = 0;
    int total_recoveries = 0;
    int total_captains_lost = 0;
    int total_ships_lost = 0;

    std::string fleet_id;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(FleetFractureRecoveryState)
};

class FleetCargoAggregatorState : public ecs::Component {
public:
    enum class ResourceType {
        Ore,
        Ice,
        Gas,
        Mineral,
        Salvage,
        Fuel,
        Ammunition,
        Component
    };

    struct CargoContribution {
        std::string ship_id;
        std::string ship_name;
        ResourceType resource_type;
        float quantity = 0.0f;
        float capacity = 0.0f;
    };

    struct ResourcePool {
        std::string pool_id;
        ResourceType resource_type;
        float total_quantity = 0.0f;
        float total_capacity = 0.0f;
    };

    std::string fleet_id;
    std::vector<CargoContribution> contributions;
    std::vector<ResourcePool> pools;
    int max_contributions = 100;
    int max_pools = 20;
    int total_transfers = 0;
    float total_quantity_transferred = 0.0f;
    int total_contributions_added = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(FleetCargoAggregatorState)
};

class FleetEngagementRulesState : public ecs::Component {
public:
    enum class RoeProfile { Passive, Defensive, Aggressive, Skirmish, Doctrine };
    enum class TargetPriority { Structures, Capitals, Battleships, Cruisers, Frigates, Any };

    RoeProfile roe = RoeProfile::Defensive;
    TargetPriority primary_target = TargetPriority::Any;

    bool auto_engage_hostiles = true;
    bool auto_engage_neutrals = false;
    float range_limit = 0.0f;

    std::string broadcast_target;
    bool all_hands_fire = false;
    bool cease_fire = false;

    int total_engagements = 0;
    int total_disengages = 0;
    float time_since_last_engagement = 0.0f;
    bool in_combat = false;

    std::string fleet_id;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(FleetEngagementRulesState)
};

class CaptainMilestoneState : public ecs::Component {
public:
    enum class MilestoneType {
        FirstKill, SurvivedAmbush, LeadWing, ActingFleetCommander,
        FullCampaign, FirstSave, EscapeArtist, HeroicCharge,
        LoyaltyTest, SeniorCaptain
    };

    struct Milestone {
        std::string milestone_id;
        MilestoneType type = MilestoneType::FirstKill;
        std::string description;
        bool is_achieved = false;
        int career_points_awarded = 0;
    };

    std::vector<Milestone> milestones;
    int max_milestones = 20;
    int career_points = 0;
    int total_achieved = 0;
    std::string captain_id;
    std::string captain_rank;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CaptainMilestoneState)
};

// FleetDepartureState
// Tracks a captain's departure/transfer lifecycle: from Stable through
// Arguing → Requesting → Transferring → Departed.
class FleetDepartureState : public ecs::Component {
public:
    enum class DepartureStage {
        Stable, Arguing, Requesting, Transferring, Departed
    };

    DepartureStage stage        = DepartureStage::Stable;
    float departure_risk        = 0.0f;  // 0–1, computed
    float morale                = 0.0f;  // –100 to +100
    int   consecutive_near_deaths = 0;
    int   losing_streak           = 0;

    bool        has_departure_request = false;
    std::string departure_reason;
    float       time_in_stage = 0.0f;

    float departure_threshold = 0.75f;  // risk level that triggers Arguing

    int total_departures = 0;
    int total_transfers  = 0;

    std::string captain_id;
    std::string fleet_id;
    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(FleetDepartureState)
};

// FactionDoctrineState
// Pirate coalition AI doctrine state machine (Phase F — Pirate Titan Meta-Threat).
// Doctrine evolves through 5 phases driven by Titan completion, discovery risk,
// and resource scarcity. Shifts shape fleet tactics, chatter, and aggression.
class FactionDoctrineState : public ecs::Component {
public:
    enum class DoctrinePhase {
        Accumulate, Conceal, Disrupt, Defend, PrepareLaunch
    };

    DoctrinePhase doctrine_phase = DoctrinePhase::Accumulate;

    // Key driver floats (0–1)
    float titan_completion    = 0.0f;
    float discovery_risk      = 0.0f;
    float resource_scarcity   = 0.0f;
    float player_proximity    = 0.0f;

    // Per-phase thresholds
    float conceal_threshold  = 0.20f;
    float disrupt_threshold  = 0.40f;
    float defend_threshold   = 0.70f;
    float launch_threshold   = 0.90f;

    // Derived aggression profile (0–1)
    float aggression_mult    = 0.2f;
    float stealth_bias       = 0.8f;
    float raid_frequency     = 0.1f;

    int   total_phase_shifts = 0;

    std::string faction_id;
    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(FactionDoctrineState)
};

// CaptainPersonalityState
// Per-captain personality axes (Phase B — Fleet Personality).
// 4 floats on [0,1]; labels are inferred, not displayed to player.
// aggression: cautious(0) ↔ bold(1)
// sociability: quiet(0) ↔ talkative(1)
// optimism:    grim(0) ↔ hopeful(1)
// professionalism: casual(0) ↔ formal(1)
class CaptainPersonalityState : public ecs::Component {
public:
    float aggression      = 0.5f;
    float sociability     = 0.5f;
    float optimism        = 0.5f;
    float professionalism = 0.5f;

    // Derived modifiers (computed from personality)
    float combat_eagerness  = 0.5f;  // drives willingness to engage
    float chatter_rate_mult = 1.0f;  // scales chatter frequency
    float morale_sensitivity = 0.5f; // how quickly morale responds to events
    float departure_risk_mult = 1.0f;// scales departure threshold sensitivity

    int   personality_updates = 0;

    std::string captain_id;
    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(CaptainPersonalityState)
};

// CaptainStressState
// Per-captain accumulated stress from combat, near-deaths, long deployments.
// High stress triggers morale penalties and departure risk escalation.
// Per-tick: passive recovery when stress > 0; burst accumulation via recordStressor.
class CaptainStressState : public ecs::Component {
public:
    float stress_level     = 0.0f;  // 0–100
    float recovery_rate    = 1.0f;  // pts/s passive recovery when > 0
    float stress_threshold = 70.0f; // above this → high_stress flag
    float critical_level   = 90.0f; // above this → critical_stress flag

    int total_stressors_applied = 0;
    int total_relief_events     = 0;

    std::string captain_id;
    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(CaptainStressState)
};

// ─── FleetReconState ─────────────────────────────────────────────────────────
// Fleet recon/scouting operations. Scouts are dispatched to adjacent systems,
// gather intel (threat level, ship counts, anomaly presence) and return. Each
// mission tracks status, elapsed time, and the intel payload. Mission results
// persist until cleared so other systems can read them.
enum class ReconStatus {
    Idle,         // mission slot empty or returned
    Deployed,     // scout is in the field
    Returning,    // duration elapsed, scout returning
    Intel_Ready,  // intel payload available for consumption
    Scout_Lost,   // scout never returned (combat or timeout)
};

struct ReconMission {
    std::string  mission_id;
    std::string  target_system;
    std::string  scout_id;
    ReconStatus  status         = ReconStatus::Idle;
    float        duration       = 120.0f; // seconds until auto-return
    float        elapsed        = 0.0f;
    float        intel_threat   = 0.0f;  // 0–1 threat level observed
    int          intel_ships    = 0;     // estimated ship count
    int          intel_anomalies = 0;    // anomaly signatures found
    bool         intel_consumed = false; // true once caller has read it
};

class FleetReconState : public ecs::Component {
public:
    std::vector<ReconMission> missions;
    int   max_missions          = 5;
    float scout_loss_timeout    = 300.0f; // mark lost if exceeds duration×2
    int   total_missions_sent   = 0;
    int   total_missions_returned = 0;
    int   total_scouts_lost     = 0;

    std::string fleet_id;
    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(FleetReconState)
};

// ─── TitanCountermeasureState ────────────────────────────────────────────────
// Player-side operations to slow the Pirate Titan assembly. Each countermeasure
// op targets a specific titan assembly node, reduces titan_delay_score (how much
// assembly has been set back), and has an effectiveness rating. The system tracks
// pirate awareness of the player's interference — high awareness causes retaliation
// events. Pairs with FactionDoctrineSystem (pirate AI) and TitanAssemblySystem.
enum class CountermeasureType {
    HarassmentRaid,      // raids on assembly convoys
    ResourceDenial,      // destroy/capture resource nodes
    IntelGathering,      // scout to discover assembly sites
    SabotageOp,          // destroy assembly components
    PropagandaDisruption,// undermine pirate recruitment
    AllianceSupport,     // coordinate allied faction response
};

struct CountermeasureOp {
    std::string           op_id;
    CountermeasureType    type          = CountermeasureType::HarassmentRaid;
    std::string           target_node;      // assembly node targeted
    float                 effectiveness    = 1.0f;  // 0–1 how well it went
    float                 delay_contributed = 0.0f; // titan delay pts added
    bool                  is_complete     = false;
    bool                  triggered_awareness = false;
};

class TitanCountermeasureState : public ecs::Component {
public:
    std::vector<CountermeasureOp> operations;
    int   max_operations        = 20;

    float titan_delay_score     = 0.0f;  // accumulated delay (higher = titan slower)
    float pirate_awareness      = 0.0f;  // 0–1 how aware pirates are of player
    float awareness_decay_rate  = 0.005f;// per second natural awareness decay
    float awareness_threshold   = 0.7f; // above this: retaliation likely

    int   total_ops_executed    = 0;
    int   total_sabotage_ops    = 0;
    int   total_intel_ops       = 0;
    int   retaliation_events    = 0;

    std::string player_id;
    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(TitanCountermeasureState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_FLEET_COMPONENTS_H
