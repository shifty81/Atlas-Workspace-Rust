#pragma once
// NF::Editor — Packet capture and inspector
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

enum class PacketDirection : uint8_t {
    Inbound, Outbound, Internal, Loopback, Dropped
};

inline const char* packetDirectionName(PacketDirection d) {
    switch (d) {
        case PacketDirection::Inbound:  return "Inbound";
        case PacketDirection::Outbound: return "Outbound";
        case PacketDirection::Internal: return "Internal";
        case PacketDirection::Loopback: return "Loopback";
        case PacketDirection::Dropped:  return "Dropped";
    }
    return "Unknown";
}

enum class PacketCategory : uint8_t {
    State, Input, RPC, Spawn, Despawn, Custom
};

inline const char* packetCategoryName(PacketCategory c) {
    switch (c) {
        case PacketCategory::State:   return "State";
        case PacketCategory::Input:   return "Input";
        case PacketCategory::RPC:     return "RPC";
        case PacketCategory::Spawn:   return "Spawn";
        case PacketCategory::Despawn: return "Despawn";
        case PacketCategory::Custom:  return "Custom";
    }
    return "Unknown";
}

enum class CaptureFilterMode : uint8_t {
    All, DirectionOnly, CategoryOnly, PeerOnly, Custom
};

inline const char* captureFilterModeName(CaptureFilterMode m) {
    switch (m) {
        case CaptureFilterMode::All:           return "All";
        case CaptureFilterMode::DirectionOnly: return "DirectionOnly";
        case CaptureFilterMode::CategoryOnly:  return "CategoryOnly";
        case CaptureFilterMode::PeerOnly:      return "PeerOnly";
        case CaptureFilterMode::Custom:        return "Custom";
    }
    return "Unknown";
}

class PacketRecord {
public:
    explicit PacketRecord(uint64_t id,
                          PacketDirection dir,
                          PacketCategory  cat,
                          uint32_t        sizeBytes)
        : m_id(id), m_direction(dir), m_category(cat), m_sizeBytes(sizeBytes) {}

    void setPeerId(const std::string& pid) { m_peerId    = pid; }
    void setChannel(uint8_t ch)            { m_channel   = ch;  }
    void setTimestamp(double ts)           { m_timestamp = ts;  }
    void setDropped(bool v)                { m_dropped   = v;   }

    [[nodiscard]] uint64_t         id()         const { return m_id;         }
    [[nodiscard]] PacketDirection  direction()  const { return m_direction;  }
    [[nodiscard]] PacketCategory   category()  const { return m_category;   }
    [[nodiscard]] uint32_t         sizeBytes() const { return m_sizeBytes;  }
    [[nodiscard]] const std::string& peerId()  const { return m_peerId;     }
    [[nodiscard]] uint8_t          channel()   const { return m_channel;    }
    [[nodiscard]] double           timestamp() const { return m_timestamp;  }
    [[nodiscard]] bool             isDropped() const { return m_dropped;    }

private:
    uint64_t        m_id;
    PacketDirection m_direction;
    PacketCategory  m_category;
    uint32_t        m_sizeBytes;
    std::string     m_peerId;
    uint8_t         m_channel   = 0;
    double          m_timestamp = 0.0;
    bool            m_dropped   = false;
};

class PacketCapturePanel {
public:
    static constexpr size_t MAX_RECORDS = 4096;

    void start()  { m_capturing = true;  }
    void stop()   { m_capturing = false; }
    void clear()  { m_records.clear(); m_nextId = 0; }

    [[nodiscard]] bool isCapturing() const { return m_capturing; }

    [[nodiscard]] bool record(PacketDirection dir, PacketCategory cat, uint32_t sizeBytes) {
        if (!m_capturing) return false;
        if (m_records.size() >= MAX_RECORDS) return false;
        m_records.emplace_back(m_nextId++, dir, cat, sizeBytes);
        return true;
    }

    [[nodiscard]] size_t recordCount() const { return m_records.size(); }

    [[nodiscard]] size_t countByDirection(PacketDirection d) const {
        size_t c = 0; for (auto& r : m_records) if (r.direction() == d) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(PacketCategory cat) const {
        size_t c = 0; for (auto& r : m_records) if (r.category() == cat) ++c; return c;
    }
    [[nodiscard]] size_t droppedCount() const {
        size_t c = 0; for (auto& r : m_records) if (r.isDropped()) ++c; return c;
    }
    [[nodiscard]] uint64_t totalBytesInbound() const {
        uint64_t b = 0;
        for (auto& r : m_records) if (r.direction() == PacketDirection::Inbound) b += r.sizeBytes();
        return b;
    }

    void setFilterMode(CaptureFilterMode m) { m_filterMode = m; }
    [[nodiscard]] CaptureFilterMode filterMode() const { return m_filterMode; }

private:
    std::vector<PacketRecord> m_records;
    CaptureFilterMode         m_filterMode = CaptureFilterMode::All;
    uint64_t                  m_nextId     = 0;
    bool                      m_capturing  = false;
};

} // namespace NF
