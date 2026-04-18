#pragma once
// NF::Editor — Destruction editor v1: breakable mesh and fracture authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Dev1FractureMode : uint8_t { Voronoi, Uniform, Radial, Custom };
enum class Dev1ChunkState   : uint8_t { Intact, Fractured, Detached, Destroyed };

inline const char* dev1FractureModeName(Dev1FractureMode m) {
    switch (m) {
        case Dev1FractureMode::Voronoi: return "Voronoi";
        case Dev1FractureMode::Uniform: return "Uniform";
        case Dev1FractureMode::Radial:  return "Radial";
        case Dev1FractureMode::Custom:  return "Custom";
    }
    return "Unknown";
}

inline const char* dev1ChunkStateName(Dev1ChunkState s) {
    switch (s) {
        case Dev1ChunkState::Intact:    return "Intact";
        case Dev1ChunkState::Fractured: return "Fractured";
        case Dev1ChunkState::Detached:  return "Detached";
        case Dev1ChunkState::Destroyed: return "Destroyed";
    }
    return "Unknown";
}

struct Dev1Chunk {
    uint64_t       id      = 0;
    std::string    name;
    Dev1ChunkState state   = Dev1ChunkState::Intact;
    float          mass    = 1.f;
    float          health  = 100.f;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isIntact()    const { return state == Dev1ChunkState::Intact; }
    [[nodiscard]] bool isDestroyed() const { return state == Dev1ChunkState::Destroyed; }
};

struct Dev1FractureConfig {
    uint64_t          id         = 0;
    std::string       name;
    Dev1FractureMode  mode       = Dev1FractureMode::Voronoi;
    uint32_t          chunkCount = 8;
    float             seed       = 42.f;
    float             impulseThreshold = 100.f;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty() && chunkCount > 0; }
};

using Dev1ChangeCallback = std::function<void(uint64_t)>;

class DestructionEditorV1 {
public:
    static constexpr size_t MAX_CONFIGS = 64;
    static constexpr size_t MAX_CHUNKS  = 512;

    bool addConfig(const Dev1FractureConfig& config) {
        if (!config.isValid()) return false;
        for (const auto& c : m_configs) if (c.id == config.id) return false;
        if (m_configs.size() >= MAX_CONFIGS) return false;
        m_configs.push_back(config);
        return true;
    }

    bool removeConfig(uint64_t id) {
        for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
            if (it->id == id) { m_configs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Dev1FractureConfig* findConfig(uint64_t id) {
        for (auto& c : m_configs) if (c.id == id) return &c;
        return nullptr;
    }

    bool addChunk(const Dev1Chunk& chunk) {
        if (!chunk.isValid()) return false;
        for (const auto& c : m_chunks) if (c.id == chunk.id) return false;
        if (m_chunks.size() >= MAX_CHUNKS) return false;
        m_chunks.push_back(chunk);
        if (m_onChange) m_onChange(chunk.id);
        return true;
    }

    bool removeChunk(uint64_t id) {
        for (auto it = m_chunks.begin(); it != m_chunks.end(); ++it) {
            if (it->id == id) { m_chunks.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Dev1Chunk* findChunk(uint64_t id) {
        for (auto& c : m_chunks) if (c.id == id) return &c;
        return nullptr;
    }

    bool setChunkState(uint64_t id, Dev1ChunkState state) {
        auto* c = findChunk(id);
        if (!c) return false;
        c->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t configCount() const { return m_configs.size(); }
    [[nodiscard]] size_t chunkCount()  const { return m_chunks.size(); }

    [[nodiscard]] size_t intactCount() const {
        size_t c = 0;
        for (const auto& ch : m_chunks) if (ch.isIntact()) ++c;
        return c;
    }

    [[nodiscard]] size_t destroyedCount() const {
        size_t c = 0;
        for (const auto& ch : m_chunks) if (ch.isDestroyed()) ++c;
        return c;
    }

    void setOnChange(Dev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Dev1FractureConfig> m_configs;
    std::vector<Dev1Chunk>          m_chunks;
    Dev1ChangeCallback              m_onChange;
};

} // namespace NF
