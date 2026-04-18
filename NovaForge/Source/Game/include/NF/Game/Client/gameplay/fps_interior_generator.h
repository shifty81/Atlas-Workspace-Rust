#pragma once
/**
 * @file fps_interior_generator.h
 * @brief Grammar-based procedural FPS interior generation.
 *
 * Every boarding / FPS mission interior is generated from a compact room
 * grammar ensuring correct FPS pacing:
 *
 *   AIRLOCK → CHOKE → PRIMARY_COMBAT → OPTIONAL_SIDE_ROOM → OBJECTIVE
 *
 * Combat room rules (non-negotiable):
 *   - 2–3 entry angles max
 *   - Cover every 4–6 metres
 *   - One elevation change only
 *   - Clear sightline priority (not maze-like)
 *
 * Low-poly geometry budget per room type:
 *   Corridor  12–24 faces
 *   Room      30–60 faces
 *   Door       6–12 faces
 */

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace atlas::gameplay {

// ── Room types ───────────────────────────────────────────────────────────────

enum class InteriorRoomType : uint8_t {
    AIRLOCK,
    CHOKE,
    PRIMARY_COMBAT,
    SIDE_ROOM,
    OBJECTIVE
};

inline const char* InteriorRoomTypeName(InteriorRoomType t) {
    switch (t) {
        case InteriorRoomType::AIRLOCK:         return "Airlock";
        case InteriorRoomType::CHOKE:           return "Choke";
        case InteriorRoomType::PRIMARY_COMBAT:  return "Primary Combat";
        case InteriorRoomType::SIDE_ROOM:       return "Side Room";
        case InteriorRoomType::OBJECTIVE:       return "Objective";
    }
    return "Unknown";
}

// ── Interior room descriptor ─────────────────────────────────────────────────

struct InteriorRoom {
    std::string       id;
    InteriorRoomType  type            = InteriorRoomType::CHOKE;
    int               entryAngles     = 2;    ///< 2–3
    float             coverSpacingM   = 4.0f; ///< metres
    bool              hasElevation    = false;
    int               polyFaceCount   = 30;
    std::vector<std::string> connections; ///< IDs of adjacent rooms
};

// ── Interior layout ──────────────────────────────────────────────────────────

struct FPSInteriorLayout {
    std::string              missionID;
    std::vector<InteriorRoom> rooms;

    const InteriorRoom* StartRoom()     const;
    const InteriorRoom* ObjectiveRoom() const;
};

// ── Generation options ───────────────────────────────────────────────────────

struct InteriorGenOptions {
    int   seed            = 0;
    int   difficulty      = 1;  ///< 1–5; affects enemy density and layout complexity
    bool  includeSideRoom = true;
    int   maxRooms        = 5;
};

// ── FPSInteriorGenerator ─────────────────────────────────────────────────────

/**
 * Generates boarding / FPS interior layouts from a seed.
 *
 * The grammar always produces layouts that satisfy the FPS pacing rules:
 *   AIRLOCK → CHOKE → PRIMARY_COMBAT [→ SIDE_ROOM] → OBJECTIVE
 *
 * Poly budgets and room constraints are validated at generation time.
 */
class FPSInteriorGenerator {
public:
    FPSInteriorGenerator() = default;

    /**
     * Generate an interior layout for the given mission.
     * @param missionID Unique identifier for this layout.
     * @param opts      Generation parameters.
     * @return Validated interior layout.
     */
    FPSInteriorLayout Generate(const std::string& missionID,
                               const InteriorGenOptions& opts = {}) const;

    /**
     * Validate that a layout satisfies all FPS pacing rules.
     * @return Empty string on success, error description on failure.
     */
    std::string Validate(const FPSInteriorLayout& layout) const;

    // ── Poly budget constants ─────────────────────────────────────────

    static constexpr int CORRIDOR_MAX_FACES = 24;
    static constexpr int ROOM_MAX_FACES     = 60;
    static constexpr int DOOR_MAX_FACES     = 12;

private:
    InteriorRoom MakeRoom(const std::string& id,
                          InteriorRoomType type,
                          int seed,
                          int difficulty) const;
};

} // namespace atlas::gameplay
