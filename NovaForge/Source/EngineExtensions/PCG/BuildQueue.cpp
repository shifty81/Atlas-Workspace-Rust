#include "BuildQueue.h"
#include <algorithm>

namespace atlas::procedural {

// ---- BuildOrder ----

float BuildOrder::Progress() const {
    if (totalTimeSeconds <= 0.0f) return 1.0f;
    float p = elapsedSeconds / totalTimeSeconds;
    if (p < 0.0f) return 0.0f;
    if (p > 1.0f) return 1.0f;
    return p;
}

float BuildOrder::RemainingSeconds() const {
    float r = totalTimeSeconds - elapsedSeconds;
    return (r > 0.0f) ? r : 0.0f;
}

// ---- BuildQueue ----

uint32_t BuildQueue::AddOrder(BuildOrder order) {
    order.id = m_nextId++;
    m_active.push_back(order);
    return order.id;
}

void BuildQueue::RemoveOrder(uint32_t orderId) {
    m_active.erase(
        std::remove_if(m_active.begin(), m_active.end(),
                       [orderId](const BuildOrder& o) { return o.id == orderId; }),
        m_active.end());
}

void BuildQueue::PauseOrder(uint32_t orderId) {
    for (auto& o : m_active) {
        if (o.id == orderId) { o.paused = true; return; }
    }
}

void BuildQueue::ResumeOrder(uint32_t orderId) {
    for (auto& o : m_active) {
        if (o.id == orderId) { o.paused = false; return; }
    }
}

void BuildQueue::Tick(float deltaSeconds) {
    if (deltaSeconds <= 0.0f) return;

    for (auto& o : m_active) {
        if (!o.paused) {
            o.elapsedSeconds += deltaSeconds;
        }
    }

    // Move completed orders.
    auto it = std::stable_partition(m_active.begin(), m_active.end(),
                                    [](const BuildOrder& o) { return !o.IsComplete(); });
    for (auto comp = it; comp != m_active.end(); ++comp) {
        m_completed.push_back(*comp);
    }
    m_active.erase(it, m_active.end());
}

const BuildOrder* BuildQueue::GetOrder(uint32_t orderId) const {
    for (auto& o : m_active) {
        if (o.id == orderId) return &o;
    }
    return nullptr;
}

std::vector<BuildOrder> BuildQueue::GetOrdersByPriority() const {
    std::vector<BuildOrder> sorted = m_active;
    std::sort(sorted.begin(), sorted.end(),
              [](const BuildOrder& a, const BuildOrder& b) {
                  return a.priority > b.priority;
              });
    return sorted;
}

float BuildQueue::TotalRemainingTime() const {
    float total = 0.0f;
    for (auto& o : m_active) {
        total += o.RemainingSeconds();
    }
    return total;
}

} // namespace atlas::procedural
