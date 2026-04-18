#pragma once
// NF::Editor — Memory tracker v1: allocation tracking with tag-based reporting
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class MtvAllocType : uint8_t { Heap, Pool, Stack, Mapped, GPU };

struct MtvAlloc {
    uint64_t    id           = 0;
    MtvAllocType type        = MtvAllocType::Heap;
    std::string  tag;
    size_t       bytes       = 0;
    uint64_t     timestampMs = 0;
    bool         freed       = false;
    [[nodiscard]] bool isValid() const { return id != 0 && bytes > 0; }
};

using MtvLeakCallback = std::function<void(const MtvAlloc&)>;

class MemoryTrackerV1 {
public:
    bool trackAlloc(const MtvAlloc& alloc) {
        if (!alloc.isValid()) return false;
        for (const auto& a : m_allocs) if (a.id == alloc.id) return false;
        m_allocs.push_back(alloc);
        if (totalLiveBytes() > m_peakBytes) m_peakBytes = totalLiveBytes();
        return true;
    }

    bool trackFree(uint64_t id) {
        for (auto& a : m_allocs) {
            if (a.id == id && !a.freed) {
                a.freed = true;
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] size_t allocCount() const { return m_allocs.size(); }

    [[nodiscard]] size_t liveAllocCount() const {
        size_t n = 0;
        for (const auto& a : m_allocs) if (!a.freed) ++n;
        return n;
    }

    [[nodiscard]] size_t totalLiveBytes() const {
        size_t n = 0;
        for (const auto& a : m_allocs) if (!a.freed) n += a.bytes;
        return n;
    }

    [[nodiscard]] size_t peakBytes() const { return m_peakBytes; }

    [[nodiscard]] size_t byTag(const std::string& tag) const {
        size_t n = 0;
        for (const auto& a : m_allocs) if (!a.freed && a.tag == tag) n += a.bytes;
        return n;
    }

    void clearFreed() {
        m_allocs.erase(
            std::remove_if(m_allocs.begin(), m_allocs.end(),
                           [](const MtvAlloc& a){ return a.freed; }),
            m_allocs.end());
    }

    void checkLeaks() {
        if (!m_onLeak) return;
        for (const auto& a : m_allocs)
            if (!a.freed) m_onLeak(a);
    }

    void setOnLeak(MtvLeakCallback cb) { m_onLeak = std::move(cb); }

private:
    std::vector<MtvAlloc> m_allocs;
    MtvLeakCallback       m_onLeak;
    size_t                m_peakBytes = 0;
};

} // namespace NF
