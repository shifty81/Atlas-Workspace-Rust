#pragma once
// NF::Editor — Room editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class RoomShape : uint8_t {
    Rectangular, LShaped, TShaped, Circular, Irregular, Hexagonal
};

inline const char* roomShapeName(RoomShape s) {
    switch (s) {
        case RoomShape::Rectangular: return "Rectangular";
        case RoomShape::LShaped:     return "LShaped";
        case RoomShape::TShaped:     return "TShaped";
        case RoomShape::Circular:    return "Circular";
        case RoomShape::Irregular:   return "Irregular";
        case RoomShape::Hexagonal:   return "Hexagonal";
    }
    return "Unknown";
}

enum class DoorType : uint8_t {
    Open, Door, LockedDoor, SecretDoor, Archway, Portcullis
};

inline const char* doorTypeName(DoorType t) {
    switch (t) {
        case DoorType::Open:        return "Open";
        case DoorType::Door:        return "Door";
        case DoorType::LockedDoor:  return "LockedDoor";
        case DoorType::SecretDoor:  return "SecretDoor";
        case DoorType::Archway:     return "Archway";
        case DoorType::Portcullis:  return "Portcullis";
    }
    return "Unknown";
}

enum class RoomTagType : uint8_t {
    Combat, Puzzle, Loot, Boss, Safe, Transition, Hub, Secret
};

inline const char* roomTagTypeName(RoomTagType t) {
    switch (t) {
        case RoomTagType::Combat:     return "Combat";
        case RoomTagType::Puzzle:     return "Puzzle";
        case RoomTagType::Loot:       return "Loot";
        case RoomTagType::Boss:       return "Boss";
        case RoomTagType::Safe:       return "Safe";
        case RoomTagType::Transition: return "Transition";
        case RoomTagType::Hub:        return "Hub";
        case RoomTagType::Secret:     return "Secret";
    }
    return "Unknown";
}

class RoomConnection {
public:
    explicit RoomConnection(uint32_t targetRoomId, DoorType doorType)
        : m_targetRoomId(targetRoomId), m_doorType(doorType) {}

    void setLocked(bool v)    { m_locked   = v; }
    void setKeyId(uint32_t v) { m_keyId    = v; }

    [[nodiscard]] uint32_t  targetRoomId() const { return m_targetRoomId; }
    [[nodiscard]] DoorType  doorType()     const { return m_doorType;     }
    [[nodiscard]] bool      isLocked()     const { return m_locked;       }
    [[nodiscard]] uint32_t  keyId()        const { return m_keyId;        }

private:
    uint32_t m_targetRoomId;
    DoorType m_doorType;
    bool     m_locked = false;
    uint32_t m_keyId  = 0;
};

class Room {
public:
    explicit Room(uint32_t id, const std::string& name, RoomShape shape)
        : m_id(id), m_name(name), m_shape(shape) {}

    void setWidth(float v)    { m_width  = v; }
    void setHeight(float v)   { m_height = v; }
    void addTag(RoomTagType t){ m_tags.insert(t); }
    void removeTag(RoomTagType t){ m_tags.erase(t); }
    void addConnection(const RoomConnection& c){ m_connections.push_back(c); }

    [[nodiscard]] uint32_t             id()              const { return m_id;     }
    [[nodiscard]] const std::string&   name()            const { return m_name;   }
    [[nodiscard]] RoomShape            shape()           const { return m_shape;  }
    [[nodiscard]] float                width()           const { return m_width;  }
    [[nodiscard]] float                height()          const { return m_height; }
    [[nodiscard]] bool                 hasTag(RoomTagType t) const { return m_tags.count(t) > 0; }
    [[nodiscard]] size_t               tagCount()        const { return m_tags.size();       }
    [[nodiscard]] size_t               connectionCount() const { return m_connections.size();}

private:
    uint32_t                    m_id;
    std::string                 m_name;
    RoomShape                   m_shape;
    float                       m_width    = 10.0f;
    float                       m_height   = 4.0f;
    std::set<RoomTagType>       m_tags;
    std::vector<RoomConnection> m_connections;
};

class RoomEditor {
public:
    bool addRoom(const Room& room) {
        for (auto& r : m_rooms) if (r.id() == room.id()) return false;
        m_rooms.push_back(room); return true;
    }
    bool removeRoom(uint32_t id) {
        auto it = std::find_if(m_rooms.begin(), m_rooms.end(),
            [&](const Room& r){ return r.id() == id; });
        if (it == m_rooms.end()) return false;
        m_rooms.erase(it); return true;
    }
    [[nodiscard]] Room* findRoom(uint32_t id) {
        for (auto& r : m_rooms) if (r.id() == id) return &r;
        return nullptr;
    }

    void setActiveRoomId(uint32_t id)  { m_activeRoomId  = id;  }
    void setShowGrid(bool v)           { m_showGrid      = v;   }
    void setShowConnections(bool v)    { m_showConnections = v; }

    [[nodiscard]] uint32_t activeRoomId()       const { return m_activeRoomId;   }
    [[nodiscard]] bool     isShowGrid()         const { return m_showGrid;       }
    [[nodiscard]] bool     isShowConnections()  const { return m_showConnections;}
    [[nodiscard]] size_t   roomCount()          const { return m_rooms.size();   }

    [[nodiscard]] size_t countByShape(RoomShape s) const {
        size_t c = 0; for (auto& r : m_rooms) if (r.shape() == s) ++c; return c;
    }
    [[nodiscard]] size_t countWithTag(RoomTagType t) const {
        size_t c = 0; for (auto& r : m_rooms) if (r.hasTag(t)) ++c; return c;
    }

private:
    std::vector<Room> m_rooms;
    uint32_t          m_activeRoomId     = 0;
    bool              m_showGrid         = true;
    bool              m_showConnections  = true;
};

} // namespace NF
