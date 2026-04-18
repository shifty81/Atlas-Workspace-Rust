#ifndef NOVAFORGE_COMPONENTS_SOCIAL_COMPONENTS_H
#define NOVAFORGE_COMPONENTS_SOCIAL_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>
#include <cmath>

namespace atlas {
namespace components {

class Corporation : public ecs::Component {
public:
    std::string corp_id;
    std::string corp_name;
    std::string ticker;
    std::string ceo_id;
    float tax_rate = 0.05f;
    std::vector<std::string> member_ids;
    double corp_wallet = 0.0;

    struct CorpHangarItem {
        std::string item_id;
        std::string name;
        std::string type;
        int quantity = 1;
        float volume = 1.0f;
    };

    std::vector<CorpHangarItem> hangar_items;

    COMPONENT_TYPE(Corporation)
};
/**
 * @brief Chat channel for persistent messaging
 *
 * Supports multiple channel types (local, corp, fleet, private)
 * with message history, moderation, and member tracking.
 */
class ChatChannel : public ecs::Component {
public:
    std::string channel_id;
    std::string channel_name;
    std::string channel_type = "local";  // "local", "corp", "fleet", "alliance", "private"
    std::string owner_id;                // channel creator/owner
    std::string motd;                    // message of the day
    int max_members = 0;                 // 0 = unlimited
    bool is_moderated = false;

    struct ChatMessage {
        std::string message_id;
        std::string sender_id;
        std::string sender_name;
        std::string content;
        std::string timestamp;
        bool is_system_message = false;
    };

    struct ChannelMember {
        std::string player_id;
        std::string player_name;
        std::string role = "member";     // "member", "moderator", "operator", "owner"
        bool is_muted = false;
    };

    std::vector<ChatMessage> messages;
    std::vector<ChannelMember> members;
    int max_history = 200;               // max messages to keep

    int memberCount() const { return static_cast<int>(members.size()); }

    COMPONENT_TYPE(ChatChannel)
};

/**
 * @brief Character sheet for player identity and attributes
 *
 * Tracks race, bloodline, ancestry, clone, implants, and
 * character attributes for creation and progression.
 */
class CharacterSheet : public ecs::Component {
public:
    std::string character_id;
    std::string character_name;
    std::string race = "Caldari";           // "Caldari", "Amarr", "Gallente", "Minmatar"
    std::string bloodline;                   // race-specific bloodline
    std::string ancestry;                    // background/origin
    std::string gender = "male";             // "male", "female"
    float date_of_birth = 0.0f;              // simulation time of creation

    // Attributes (base values, modified by implants)
    int intelligence = 20;
    int perception = 20;
    int charisma = 19;
    int willpower = 20;
    int memory = 20;

    // Clone
    std::string clone_grade = "foundry";       // "foundry", "apex"
    std::string clone_location;              // station ID for medical clone
    int clone_jump_cooldown = 0;             // seconds remaining

    // Implants (slots 1-10)
    struct Implant {
        std::string implant_id;
        std::string implant_name;
        int slot = 0;                        // 1-10
        std::string attribute_bonus;         // attribute boosted
        int bonus_amount = 0;
    };
    std::vector<Implant> implants;

    // Security status
    float security_status = 0.0f;            // -10.0 to 10.0

    // Employment history
    struct EmploymentRecord {
        std::string corp_id;
        std::string corp_name;
        float join_date = 0.0f;
        float leave_date = 0.0f;
    };
    std::vector<EmploymentRecord> employment_history;

    int getEffectiveAttribute(const std::string& attr) const {
        int base = 0;
        if (attr == "intelligence") base = intelligence;
        else if (attr == "perception") base = perception;
        else if (attr == "charisma") base = charisma;
        else if (attr == "willpower") base = willpower;
        else if (attr == "memory") base = memory;

        for (const auto& imp : implants) {
            if (imp.attribute_bonus == attr) base += imp.bonus_amount;
        }
        return base;
    }

    COMPONENT_TYPE(CharacterSheet)
};

/**
 * @brief Tournament bracket for competitive PvE events
 *
 * Tracks tournament lifecycle: registration, active rounds,
 * participant scoring, and final results with rewards.
 */
class Tournament : public ecs::Component {
public:
    std::string tournament_id;
    std::string name;
    std::string status = "registration";  // "registration", "active", "completed", "cancelled"

    int max_participants = 16;
    int current_round = 0;
    int total_rounds = 0;
    float round_duration = 600.0f;        // seconds per round
    float round_timer = 0.0f;             // countdown for current round

    double entry_fee = 0.0;
    double prize_pool = 0.0;

    struct Participant {
        std::string player_id;
        std::string player_name;
        int score = 0;
        int kills = 0;
        bool eliminated = false;
    };

    std::vector<Participant> participants;

    struct RoundResult {
        int round_number = 0;
        std::string winner_id;
        int winner_score = 0;
        int participant_count = 0;
    };

    std::vector<RoundResult> round_results;

    COMPONENT_TYPE(Tournament)
};

/**
 * @brief Leaderboard for tracking player rankings and achievements
 *
 * Aggregates player stats across categories (kills, Credits earned,
 * missions completed, etc.) and tracks unlocked achievements.
 */
class Leaderboard : public ecs::Component {
public:
    struct PlayerEntry {
        std::string player_id;
        std::string player_name;
        int total_kills = 0;
        double total_isc_earned = 0.0;
        int missions_completed = 0;
        int tournaments_won = 0;
        double total_bounty = 0.0;
        int ships_destroyed = 0;
        int ships_lost = 0;
        double total_damage_dealt = 0.0;
    };

    struct Achievement {
        std::string achievement_id;
        std::string name;
        std::string description;
        std::string category;             // "combat", "industry", "exploration", "social"
        int requirement = 1;              // threshold to unlock
        std::string stat_key;             // which stat to check against requirement
    };

    struct UnlockedAchievement {
        std::string achievement_id;
        std::string player_id;
        float unlock_time = 0.0f;
    };

    std::string board_id;
    std::vector<PlayerEntry> entries;
    std::vector<Achievement> achievements;
    std::vector<UnlockedAchievement> unlocked;

    COMPONENT_TYPE(Leaderboard)
};
class Alliance : public ecs::Component {
public:
    std::string alliance_id;
    std::string alliance_name;
    std::string ticker;
    std::string executor_corp_id;  // The corp that leads the alliance
    std::vector<std::string> member_corp_ids;
    double alliance_wallet = 0.0;
    float tax_rate = 0.0f;
    std::string founded_date;
    bool is_open = false;  // Open for new applications
    int max_corps = 100;
    COMPONENT_TYPE(Alliance)
};

class Sovereignty : public ecs::Component {
public:
    std::string system_id;        // Star system this sovereignty covers
    std::string owner_id;         // Alliance or corp entity that owns it
    std::string system_name;
    float control_level = 0.0f;   // 0.0 to 1.0 (contested to full control)
    float vulnerability_timer = 0.0f; // hours until vulnerable
    bool is_contested = false;
    float strategic_index = 0.0f; // 0.0 to 5.0 based on activity
    float military_index = 0.0f;  // 0.0 to 5.0 based on NPC kills
    float industrial_index = 0.0f;// 0.0 to 5.0 based on mining/production
    std::string infrastructure_hub_id;
    int upgrade_level = 0;        // 0-5
    COMPONENT_TYPE(Sovereignty)
};

class WarDeclaration : public ecs::Component {
public:
    std::string war_id;
    std::string aggressor_id;     // Corp or alliance that declared war
    std::string defender_id;      // Corp or alliance being attacked
    std::string status = "pending"; // "pending", "active", "mutual", "surrendered", "retracted", "finished"
    float duration_hours = 168.0f; // Default 1 week
    float elapsed_hours = 0.0f;
    double war_cost = 100000000.0; // Credits cost to declare war (100M)
    int aggressor_kills = 0;
    int defender_kills = 0;
    double aggressor_isc_destroyed = 0.0;
    double defender_isc_destroyed = 0.0;
    bool is_mutual = false;
    COMPONENT_TYPE(WarDeclaration)
};

/**
 * @brief Player faction standing and reputation aggregation
 */
class PlayerStanding : public ecs::Component {
public:
    struct FactionStanding {
        std::string faction_id;
        std::string faction_name;
        double standing = 0.0;     // -10.0 to 10.0
        int rank = 0;              // 0=Neutral, 1-5=Positive ranks, -1 to -5=Negative ranks
    };

    struct StandingNotification {
        std::string faction_id;
        int old_rank = 0;
        int new_rank = 0;
        float timestamp = 0.0f;
    };

    float elapsed = 0.0f;
    bool active = true;
    int max_notifications = 20;
    std::vector<FactionStanding> factions;
    std::vector<StandingNotification> notifications;

    COMPONENT_TYPE(PlayerStanding)
};


// ---------------------------------------------------------------------------
// CitadelState — player-owned Upwell structure management
// ---------------------------------------------------------------------------
/**
 * @brief Player-owned citadel / Upwell structure state.
 *
 * Models three sizes (Astrahus / Fortizar / Keepstar) with services,
 * fuel consumption, vulnerability windows, and reinforcement timers.
 * Fuel is consumed per tick based on active services.  When fuel runs
 * out, all services go offline.  Vulnerability windows allow attacks
 * that may trigger reinforcement timers.
 */
class CitadelState : public ecs::Component {
public:
    enum class CitadelType { Astrahus, Fortizar, Keepstar };
    enum class StructureState { Online, Vulnerable, Reinforced, Destroyed };

    struct Service {
        std::string service_id;
        std::string service_name;
        float       fuel_per_hour = 5.0f;
        bool        online        = true;
    };

    CitadelType    type              = CitadelType::Astrahus;
    StructureState state             = StructureState::Online;
    std::string    owner_corp_id;
    std::string    structure_name;
    float          shield_hp         = 10000.0f;
    float          shield_hp_max     = 10000.0f;
    float          armor_hp          = 10000.0f;
    float          armor_hp_max      = 10000.0f;
    float          hull_hp           = 10000.0f;
    float          hull_hp_max       = 10000.0f;
    std::vector<Service> services;
    int            max_services      = 5;
    float          fuel_remaining    = 1000.0f;  // fuel units
    float          fuel_capacity     = 5000.0f;
    float          vulnerability_hours = 3.0f;   // hours per week
    float          reinforcement_timer = 0.0f;   // seconds remaining
    float          reinforcement_duration = 86400.0f; // default 24h
    int            total_reinforcements = 0;
    int            total_services_installed = 0;
    float          elapsed           = 0.0f;
    bool           active            = true;

    COMPONENT_TYPE(CitadelState)
};

// ---------------------------------------------------------------------------
// AccessListState — access control list management for structures/containers
// ---------------------------------------------------------------------------
/**
 * @brief Manages access control lists for player-owned structures and
 *        containers.  Each ACL entry grants or blocks a specific member.
 *        Entries may have a TTL that counts down per-tick; expired entries
 *        are auto-removed.  A default_policy controls access for members
 *        not on any list.  max_entries caps the total number of entries
 *        (default 50).
 */
class AccessListState : public ecs::Component {
public:
    enum class Permission { Allow, Block };

    struct AclEntry {
        std::string entry_id;
        std::string member_id;
        Permission  permission = Permission::Allow;
        float       ttl        = 0.0f;   // 0 = permanent
    };

    std::string owner_id;
    std::string structure_id;
    Permission  default_policy = Permission::Block;
    std::vector<AclEntry> entries;
    int   max_entries          = 50;
    int   total_entries_added  = 0;
    int   total_entries_expired = 0;
    int   total_access_checks  = 0;
    float elapsed              = 0.0f;
    bool  active               = true;

    COMPONENT_TYPE(AccessListState)
};

// ---------------------------------------------------------------------------
// FleetAdvertisementState — fleet finder / advertisement management
// ---------------------------------------------------------------------------
/**
 * @brief Manages fleet advertisements for public fleet recruitment.
 *        Each ad has a title, description, fleet type, and requirements.
 *        Players can apply to join; the fleet boss accepts or rejects.
 *        Ads have a configurable TTL that counts down per-tick; expired
 *        ads auto-deactivate.  Applications are capped at max_applications
 *        (default 20).  Lifetime counters track total ads posted, total
 *        applications received, and total accepted.
 */
class FleetAdvertisementState : public ecs::Component {
public:
    enum class FleetType { PvE, Mining, Exploration, Incursion, Hauling };
    enum class AppStatus { Pending, Accepted, Rejected };

    struct Application {
        std::string app_id;
        std::string pilot_name;
        std::string ship_type;
        AppStatus   status = AppStatus::Pending;
    };

    std::string fleet_id;
    std::string title;
    std::string description;
    FleetType   fleet_type       = FleetType::PvE;
    std::string boss_name;
    bool        is_listed        = false;
    float       ttl              = 3600.0f;  // seconds before ad expires
    float       time_remaining   = 3600.0f;
    int         min_members      = 1;
    int         max_members      = 50;
    int         current_members  = 0;
    std::vector<Application> applications;
    int   max_applications           = 20;
    int   total_ads_posted           = 0;
    int   total_applications_received = 0;
    int   total_accepted             = 0;
    int   total_rejected             = 0;
    float elapsed                    = 0.0f;
    bool  active                     = true;

    COMPONENT_TYPE(FleetAdvertisementState)
};

/**
 * @brief In-game mailbox for player messaging
 *
 * Manages received mails with read/unread tracking, flagging,
 * labels, and capacity limits with auto-purge of oldest on overflow.
 */
class MailboxState : public ecs::Component {
public:
    struct Mail {
        std::string mail_id;
        std::string sender_id;
        std::string sender_name;
        std::string subject;
        std::string body;
        float       timestamp   = 0.0f;
        bool        is_read     = false;
        bool        is_flagged  = false;
        std::vector<std::string> labels;
    };

    std::string owner_id;
    std::vector<Mail> mails;
    int   max_mails          = 200;
    int   total_received     = 0;
    int   total_deleted      = 0;
    float elapsed            = 0.0f;
    bool  active             = true;

    COMPONENT_TYPE(MailboxState)
};

/**
 * @brief Corporation medal and decoration management
 *
 * Tracks medal definitions created by a corporation and individual
 * awards given to pilots. Supports creating, awarding, revoking,
 * and querying medals with capacity limits.
 */
class MedalCollectionState : public ecs::Component {
public:
    struct Medal {
        std::string medal_id;
        std::string name;
        std::string description;
        std::string creator_id;
    };

    struct AwardedMedal {
        std::string award_id;
        std::string medal_id;
        std::string recipient_id;
        std::string reason;
        float       timestamp = 0.0f;
    };

    std::string corp_id;
    std::vector<Medal>        medals;
    std::vector<AwardedMedal> awards;
    int   max_medals             = 50;
    int   max_awards             = 200;
    int   total_medals_created   = 0;
    int   total_awards_given     = 0;
    int   total_awards_revoked   = 0;
    float elapsed                = 0.0f;
    bool  active                 = true;

    COMPONENT_TYPE(MedalCollectionState)
};

class CaptainSocialGraphState : public ecs::Component {
public:
    enum class RelationshipType {
        Friendship,
        Grudge,
        Rivalry,
        Mentorship,
        Neutral
    };

    enum class EventType {
        SharedVictory,
        Betrayal,
        Rescue,
        Argument,
        TradeCompleted,
        MissionSuccess,
        MissionFailure,
        GiftGiven
    };

    struct Relationship {
        std::string target_captain_id;
        RelationshipType type = RelationshipType::Neutral;
        float trust = 0.5f;      // 0..1
        float affinity = 0.0f;   // -1..1
        int interactions = 0;
    };

    struct SocialEvent {
        std::string event_id;
        std::string captain_a;
        std::string captain_b;
        EventType event_type;
        float impact = 0.0f;     // -1..1 (negative = harmful)
    };

    std::string owner_captain_id;
    std::vector<Relationship> relationships;
    std::vector<SocialEvent> events;
    int max_relationships = 50;
    int max_events = 100;
    int total_relationships_formed = 0;
    int total_events_recorded = 0;
    int total_grudges = 0;
    int total_friendships = 0;
    float elapsed = 0.0f;
    bool active = true;

    COMPONENT_TYPE(CaptainSocialGraphState)
};

// EmotionalArcState
// Per-captain emotional arc: confidence, trust, fatigue, hope — updated by
// discrete events, not per-frame. Arc label is derived from current values.
class EmotionalArcState : public ecs::Component {
public:
    float confidence      = 0.5f;  // 0–1
    float trust_in_player = 0.5f;  // 0–1
    float fatigue         = 0.0f;  // 0–1
    float hope            = 0.5f;  // 0–1

    int wins   = 0;
    int losses = 0;
    int near_deaths = 0;
    int saves_by_player = 0;

    int total_arc_updates = 0;

    std::string captain_id;
    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(EmotionalArcState)
};

// ─── CaptainMoodState ────────────────────────────────────────────────────────
// Short-term per-captain mood driven by discrete events (victory, setback,
// comradeship, insult). Mood decays to Neutral over time. Unlike EmotionalArcState
// (long-term arcs) this captures the immediate psychological state that drives
// chatter tone and reaction speed.
enum class CaptainMood {
    Neutral,      // baseline
    Confident,    // recent success, high morale
    Elated,       // exceptional victory or rare positive event
    Focused,      // combat-alert, intense concentration
    Tense,        // threat nearby, pre-combat anxiety
    Frustrated,   // repeated setback, failed orders
    Anxious,      // near-death recently, critical stress overlap
};

struct MoodEvent {
    std::string event_id;
    CaptainMood mood_result = CaptainMood::Neutral;
    float       intensity   = 1.0f;  // 0–1 magnitude at time of event
    float       age_seconds = 0.0f;  // time since event was recorded
};

class CaptainMoodState : public ecs::Component {
public:
    CaptainMood current_mood       = CaptainMood::Neutral;
    float       mood_intensity     = 0.0f;  // 0–1, decays per tick
    float       decay_rate         = 0.05f; // pts/s decay toward 0
    float       mood_threshold     = 0.1f;  // below this → revert to Neutral

    std::vector<MoodEvent> mood_history;
    int   max_history          = 10;
    int   total_events_logged  = 0;

    std::string captain_id;
    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(CaptainMoodState)
};

// ─── CaptainMentorshipState ──────────────────────────────────────────────────
// Tracks an active mentor-student relationship between two captains.
// The mentor entity holds this component. A mentor can have at most one active
// student at a time. Sessions record shared engagements, skill-transfer
// milestones, and emotional bond strength. Completing mentorship "graduates"
// the student (bonus applied externally) and marks is_graduated.
struct MentorSession {
    std::string session_id;
    std::string student_captain_id;
    float       bond_strength   = 0.0f;  // 0–1, grows with shared events
    int         engagements_shared = 0;  // battles/warps experienced together
    int         skill_transfers = 0;     // number of skill transfer milestones
    float       session_duration = 0.0f; // total time in this session (seconds)
    bool        is_active       = true;
    bool        is_graduated    = false;
};

class CaptainMentorshipState : public ecs::Component {
public:
    std::string mentor_captain_id;
    std::string active_student_id;         // empty if no active student
    bool        has_active_student = false;

    std::vector<MentorSession> sessions;
    int   max_sessions             = 10;   // history cap

    float bond_growth_rate         = 0.05f; // bond gain per shared engagement
    float skill_transfer_threshold = 0.5f;  // bond level needed for a transfer
    float graduation_threshold     = 0.9f;  // bond needed to graduate

    int   total_students_mentored  = 0;
    int   total_graduations        = 0;
    int   total_skill_transfers    = 0;

    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(CaptainMentorshipState)
};

// ─── FleetCultureState ───────────────────────────────────────────────────────
// Tracks emergent cultural elements of a fleet: mottos, traditions, and taboos.
// Culture forms through repeated patterns; breaking it causes tension while
// reinforcing it builds cohesion. Elements can be player-named or auto-generated.
enum class CultureElementType {
    Tradition,   // positive reinforced habit ("We always salvage")
    Taboo,       // avoided behaviour ("We never retreat without orders")
    Motto,       // fleet-wide rallying phrase
    Ritual,      // recurring ceremony or practice
};

struct CultureElement {
    std::string       element_id;
    std::string       name;
    CultureElementType type         = CultureElementType::Tradition;
    std::string       description;
    int               reinforcement_count = 0;  // times the element was upheld
    int               violation_count     = 0;  // times the element was broken
    float             strength            = 0.0f; // 0–1 how ingrained it is
    bool              is_active           = true;
};

class FleetCultureState : public ecs::Component {
public:
    std::string fleet_id;
    std::vector<CultureElement> elements;
    int   max_elements              = 15;

    float cohesion_bonus            = 0.0f; // total cohesion bonus from culture
    float tension_level             = 0.0f; // 0–1 tension from recent violations
    float tension_decay_rate        = 0.01f; // per second natural decay

    int   total_elements_formed     = 0;
    int   total_reinforcements      = 0;
    int   total_violations          = 0;

    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(FleetCultureState)
};

// ─── CaptainAmbitionState ────────────────────────────────────────────────────
// Per-captain long-term goals. Ambition drives behaviour: when goals are blocked
// departure risk rises; when achieved loyalty and morale get a meaningful boost.
// A captain can hold multiple ambitions simultaneously (up to max_ambitions).
enum class AmbitionType {
    CommandCapital,    // pilot a capital-class ship
    LeadWing,          // become a wing commander
    FleetCommander,    // ascend to fleet commander role
    RetireWithHonor,   // survive N years of service with honor intact
    BecomeAce,         // achieve N confirmed kills
    EarnMedal,         // receive a specific medal/decoration
    MentorJunior,      // successfully mentor a younger captain
    BuildReputation,   // reach high standing with a faction
};

struct CaptainAmbition {
    std::string  ambition_id;
    AmbitionType type              = AmbitionType::LeadWing;
    std::string  description;
    float        progress          = 0.0f; // 0–1 toward completion
    float        target_value      = 1.0f; // the goal value
    float        current_value     = 0.0f; // the running counter
    bool         is_achieved       = false;
    bool         is_blocked        = false; // blocked = rising departure risk
    float        frustration_level = 0.0f; // 0–1, rises when blocked
};

class CaptainAmbitionState : public ecs::Component {
public:
    std::string captain_id;
    std::vector<CaptainAmbition> ambitions;
    int   max_ambitions            = 5;

    float frustration_decay_rate   = 0.002f; // per second natural decay
    float departure_risk_contrib   = 0.0f;   // sum of frustration levels

    int   total_ambitions_set      = 0;
    int   total_achieved           = 0;
    int   total_blocked            = 0;       // how many were ever blocked

    float elapsed = 0.0f;
    bool  active  = true;

    COMPONENT_TYPE(CaptainAmbitionState)
};

} // namespace components
} // namespace atlas

#endif // NOVAFORGE_COMPONENTS_SOCIAL_COMPONENTS_H
