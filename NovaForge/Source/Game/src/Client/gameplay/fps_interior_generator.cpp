/**
 * @file fps_interior_generator.cpp
 * @brief FPSInteriorGenerator implementation.
 */

#include "gameplay/fps_interior_generator.h"
#include <stdexcept>
#include <sstream>

namespace atlas::gameplay {

InteriorRoom FPSInteriorGenerator::MakeRoom(const std::string& id,
                                             InteriorRoomType type,
                                             int seed,
                                             int difficulty) const {
    InteriorRoom room;
    room.id   = id;
    room.type = type;

    // Entry angles: 2–3 (combat rooms get 3 on harder difficulties)
    switch (type) {
        case InteriorRoomType::AIRLOCK:
            room.entryAngles   = 1;
            room.polyFaceCount = 12 + (seed % 4);
            room.coverSpacingM = 0.0f;
            break;
        case InteriorRoomType::CHOKE:
            room.entryAngles   = 2;
            room.polyFaceCount = 16 + (seed % 8);
            room.coverSpacingM = 5.0f;
            break;
        case InteriorRoomType::PRIMARY_COMBAT:
            room.entryAngles   = (difficulty >= 3) ? 3 : 2;
            room.hasElevation  = (difficulty >= 2);
            room.polyFaceCount = 35 + (seed % 20);
            room.coverSpacingM = 4.0f + static_cast<float>(seed % 3);
            break;
        case InteriorRoomType::SIDE_ROOM:
            room.entryAngles   = 2;
            room.polyFaceCount = 25 + (seed % 15);
            room.coverSpacingM = 5.0f;
            break;
        case InteriorRoomType::OBJECTIVE:
            room.entryAngles   = 2;
            room.polyFaceCount = 30 + (seed % 20);
            room.coverSpacingM = 4.0f;
            break;
    }

    // Clamp face counts to budget
    if (room.polyFaceCount > ROOM_MAX_FACES) room.polyFaceCount = ROOM_MAX_FACES;

    return room;
}

FPSInteriorLayout FPSInteriorGenerator::Generate(
    const std::string& missionID,
    const InteriorGenOptions& opts) const
{
    FPSInteriorLayout layout;
    layout.missionID = missionID;

    int seed = opts.seed;

    // Fixed grammar: AIRLOCK → CHOKE → PRIMARY_COMBAT [→ SIDE_ROOM] → OBJECTIVE
    InteriorRoom airlock = MakeRoom(missionID + "_airlock",
                                    InteriorRoomType::AIRLOCK, seed++, opts.difficulty);
    InteriorRoom choke   = MakeRoom(missionID + "_choke",
                                    InteriorRoomType::CHOKE, seed++, opts.difficulty);
    InteriorRoom combat  = MakeRoom(missionID + "_combat",
                                    InteriorRoomType::PRIMARY_COMBAT, seed++, opts.difficulty);
    InteriorRoom objective = MakeRoom(missionID + "_objective",
                                      InteriorRoomType::OBJECTIVE, seed++, opts.difficulty);

    if (opts.includeSideRoom) {
        InteriorRoom side = MakeRoom(missionID + "_side",
                                     InteriorRoomType::SIDE_ROOM, seed++, opts.difficulty);

        // Wire connections before pushing to layout
        airlock.connections.push_back(choke.id);
        choke.connections.push_back(airlock.id);
        choke.connections.push_back(combat.id);
        combat.connections.push_back(choke.id);
        combat.connections.push_back(side.id);
        side.connections.push_back(combat.id);
        side.connections.push_back(objective.id);
        objective.connections.push_back(side.id);

        layout.rooms.push_back(airlock);
        layout.rooms.push_back(choke);
        layout.rooms.push_back(combat);
        layout.rooms.push_back(side);
    } else {
        // Wire connections before pushing to layout
        airlock.connections.push_back(choke.id);
        choke.connections.push_back(airlock.id);
        choke.connections.push_back(combat.id);
        combat.connections.push_back(choke.id);
        combat.connections.push_back(objective.id);
        objective.connections.push_back(combat.id);

        layout.rooms.push_back(airlock);
        layout.rooms.push_back(choke);
        layout.rooms.push_back(combat);
    }

    layout.rooms.push_back(objective);

    return layout;
}

std::string FPSInteriorGenerator::Validate(const FPSInteriorLayout& layout) const {
    if (layout.rooms.empty())
        return "Layout has no rooms";

    // Must start with AIRLOCK
    if (layout.rooms.front().type != InteriorRoomType::AIRLOCK)
        return "First room must be AIRLOCK";

    // Must end with OBJECTIVE
    if (layout.rooms.back().type != InteriorRoomType::OBJECTIVE)
        return "Last room must be OBJECTIVE";

    for (const auto& room : layout.rooms) {
        if (room.entryAngles < 1 || room.entryAngles > 3) {
            return std::string("Room ") + room.id +
                   " has invalid entry angle count: " +
                   std::to_string(room.entryAngles);
        }

        // Poly budget checks
        int maxFaces = (room.type == InteriorRoomType::CHOKE ||
                        room.type == InteriorRoomType::AIRLOCK)
                       ? CORRIDOR_MAX_FACES
                       : ROOM_MAX_FACES;

        if (room.polyFaceCount > maxFaces) {
            return std::string("Room ") + room.id +
                   " exceeds poly budget (" +
                   std::to_string(room.polyFaceCount) + " > " +
                   std::to_string(maxFaces) + ")";
        }
    }

    return "";  // valid
}

const InteriorRoom* FPSInteriorLayout::StartRoom() const {
    if (rooms.empty()) return nullptr;
    return &rooms.front();
}

const InteriorRoom* FPSInteriorLayout::ObjectiveRoom() const {
    if (rooms.empty()) return nullptr;
    return &rooms.back();
}

} // namespace atlas::gameplay
