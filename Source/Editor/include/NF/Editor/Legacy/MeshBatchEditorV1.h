#pragma once
// NF::Editor — Mesh batch editor v1: batch definition and draw call management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Mbev1BatchMode  : uint8_t { Static, Dynamic, Skinned, Instanced };
enum class Mbev1BatchState : uint8_t { Pending, Ready, Submitted, Failed };

inline const char* mbev1BatchModeName(Mbev1BatchMode m) {
    switch (m) {
        case Mbev1BatchMode::Static:    return "Static";
        case Mbev1BatchMode::Dynamic:   return "Dynamic";
        case Mbev1BatchMode::Skinned:   return "Skinned";
        case Mbev1BatchMode::Instanced: return "Instanced";
    }
    return "Unknown";
}

inline const char* mbev1BatchStateName(Mbev1BatchState s) {
    switch (s) {
        case Mbev1BatchState::Pending:   return "Pending";
        case Mbev1BatchState::Ready:     return "Ready";
        case Mbev1BatchState::Submitted: return "Submitted";
        case Mbev1BatchState::Failed:    return "Failed";
    }
    return "Unknown";
}

struct Mbev1Batch {
    uint64_t        id          = 0;
    std::string     name;
    Mbev1BatchMode  mode        = Mbev1BatchMode::Static;
    Mbev1BatchState state       = Mbev1BatchState::Pending;
    uint32_t        drawCalls   = 0;
    uint32_t        vertexCount = 0;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isReady()     const { return state == Mbev1BatchState::Ready; }
    [[nodiscard]] bool isSubmitted() const { return state == Mbev1BatchState::Submitted; }
    [[nodiscard]] bool isFailed()    const { return state == Mbev1BatchState::Failed; }
};

struct Mbev1RenderGroup {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Mbev1ChangeCallback = std::function<void(uint64_t)>;

class MeshBatchEditorV1 {
public:
    static constexpr size_t MAX_BATCHES = 512;
    static constexpr size_t MAX_GROUPS  = 128;

    bool addBatch(const Mbev1Batch& batch) {
        if (!batch.isValid()) return false;
        for (const auto& b : m_batches) if (b.id == batch.id) return false;
        if (m_batches.size() >= MAX_BATCHES) return false;
        m_batches.push_back(batch);
        if (m_onChange) m_onChange(batch.id);
        return true;
    }

    bool removeBatch(uint64_t id) {
        for (auto it = m_batches.begin(); it != m_batches.end(); ++it) {
            if (it->id == id) { m_batches.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Mbev1Batch* findBatch(uint64_t id) {
        for (auto& b : m_batches) if (b.id == id) return &b;
        return nullptr;
    }

    bool addGroup(const Mbev1RenderGroup& group) {
        if (!group.isValid()) return false;
        for (const auto& g : m_groups) if (g.id == group.id) return false;
        if (m_groups.size() >= MAX_GROUPS) return false;
        m_groups.push_back(group);
        return true;
    }

    bool removeGroup(uint64_t id) {
        for (auto it = m_groups.begin(); it != m_groups.end(); ++it) {
            if (it->id == id) { m_groups.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t batchCount() const { return m_batches.size(); }
    [[nodiscard]] size_t groupCount() const { return m_groups.size(); }

    [[nodiscard]] size_t readyCount() const {
        size_t c = 0; for (const auto& b : m_batches) if (b.isReady()) ++c; return c;
    }
    [[nodiscard]] size_t countByMode(Mbev1BatchMode mode) const {
        size_t c = 0; for (const auto& b : m_batches) if (b.mode == mode) ++c; return c;
    }
    [[nodiscard]] uint32_t totalDrawCalls() const {
        uint32_t sum = 0; for (const auto& b : m_batches) sum += b.drawCalls; return sum;
    }

    void setOnChange(Mbev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Mbev1Batch>       m_batches;
    std::vector<Mbev1RenderGroup> m_groups;
    Mbev1ChangeCallback           m_onChange;
};

} // namespace NF
