#include "JobTracer.h"
#include <chrono>
#include <functional>

namespace atlas::sim {

void JobTracer::BeginTick(uint64_t tick) {
    m_current = TickTrace{};
    m_current.tick = tick;
    m_currentOrder = 0;
    m_inTick = true;
    m_inSystem = false;
}

void JobTracer::RecordSystemStart(const std::string& systemName) {
    if (!m_inTick) return;
    m_inSystem = true;

    auto now = std::chrono::steady_clock::now();
    m_systemStartTime = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()).count());

    JobTraceEntry entry;
    entry.systemName = systemName;
    entry.tick = m_current.tick;
    entry.orderIndex = m_currentOrder;
    entry.startTimeUs = m_systemStartTime;
    m_current.entries.push_back(entry);
}

void JobTracer::RecordSystemEnd() {
    if (!m_inTick || !m_inSystem || m_current.entries.empty()) return;
    m_inSystem = false;

    auto now = std::chrono::steady_clock::now();
    double endTime = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            now.time_since_epoch()).count());

    m_current.entries.back().durationUs = endTime - m_systemStartTime;
    m_currentOrder++;
}

void JobTracer::EndTick() {
    if (!m_inTick) return;
    m_inTick = false;

    // Compute order hash: FNV-1a over system names in execution order
    constexpr uint64_t FNV1A_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr uint64_t FNV1A_PRIME = 1099511628211ULL;
    uint64_t hash = FNV1A_OFFSET_BASIS;
    for (const auto& entry : m_current.entries) {
        for (char c : entry.systemName) {
            hash ^= static_cast<uint64_t>(static_cast<uint8_t>(c));
            hash *= FNV1A_PRIME;
        }
        // Mix in the order index
        hash ^= static_cast<uint64_t>(entry.orderIndex);
        hash *= FNV1A_PRIME;
    }
    m_current.orderHash = hash;

    m_history.push_back(std::move(m_current));
    m_current = TickTrace{};

    // Prune old history
    while (m_history.size() > m_maxHistory) {
        m_history.erase(m_history.begin());
    }
}

const TickTrace* JobTracer::LatestTrace() const {
    if (m_history.empty()) return nullptr;
    return &m_history.back();
}

const TickTrace* JobTracer::TraceAtTick(uint64_t tick) const {
    for (const auto& trace : m_history) {
        if (trace.tick == tick) return &trace;
    }
    return nullptr;
}

const std::vector<TickTrace>& JobTracer::History() const {
    return m_history;
}

int JobTracer::CompareOrder(const TickTrace& a, const TickTrace& b) {
    size_t count = a.entries.size();
    if (b.entries.size() < count) count = b.entries.size();

    for (size_t i = 0; i < count; ++i) {
        if (a.entries[i].systemName != b.entries[i].systemName) {
            return static_cast<int>(i);
        }
    }

    if (a.entries.size() != b.entries.size()) {
        return static_cast<int>(count);
    }

    return -1;
}

void JobTracer::SetMaxHistory(size_t max) {
    m_maxHistory = max;
    while (m_history.size() > m_maxHistory) {
        m_history.erase(m_history.begin());
    }
}

size_t JobTracer::MaxHistory() const {
    return m_maxHistory;
}

void JobTracer::Clear() {
    m_history.clear();
    m_current = TickTrace{};
    m_currentOrder = 0;
    m_inTick = false;
    m_inSystem = false;
}

}  // namespace atlas::sim
