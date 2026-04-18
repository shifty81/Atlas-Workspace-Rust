/**
 * @file legend_system.cpp
 * @brief LegendSystem implementation.
 */

#include "gameplay/legend_system.h"
#include <algorithm>
#include <cmath>

namespace atlas::gameplay {

// ── PlayerReputation ─────────────────────────────────────────────────────────

void PlayerReputation::Clamp() {
    auto clamp01 = [](float v) { return std::max(0.0f, std::min(1.0f, v)); };
    aggression = clamp01(aggression);
    stealth    = clamp01(stealth);
    precision  = clamp01(precision);
    authority  = clamp01(authority);
    chaos      = clamp01(chaos);
}

// ── LegendSystem ─────────────────────────────────────────────────────────────

void LegendSystem::RecordAction(float aggression, float stealth,
                                float precision, float authority, float chaos) {
    const float alpha = 0.1f;  // EWMA smoothing

    auto ewma = [alpha](float cur, float sample) {
        return cur + alpha * (sample - cur);
    };

    m_reputation.aggression = ewma(m_reputation.aggression, aggression);
    m_reputation.stealth    = ewma(m_reputation.stealth,    stealth);
    m_reputation.precision  = ewma(m_reputation.precision,  precision);
    m_reputation.authority  = ewma(m_reputation.authority,  authority);
    m_reputation.chaos      = ewma(m_reputation.chaos,      chaos);
    m_reputation.Clamp();
}

void LegendSystem::RecordMemory(const std::string& key,
                                MemoryTier tier,
                                float strength,
                                const std::string& biomeID) {
    // Find existing memory or create new
    LegendMemory* existing = nullptr;
    for (auto& m : m_memories) {
        if (m.key == key) { existing = &m; break; }
    }

    if (existing) {
        existing->strength = std::max(existing->strength, strength);
        ++existing->runCount;
        if (!biomeID.empty()) {
            m_biomesSeen[key + "|" + biomeID] = 1;
        }
    } else {
        LegendMemory mem;
        mem.key      = key;
        mem.tier     = tier;
        mem.strength = strength;
        mem.runCount = 1;
        if (!biomeID.empty()) {
            m_biomesSeen[key + "|" + biomeID] = 1;
        }
        m_memories.push_back(mem);
    }
}

void LegendSystem::PromoteMemories(int runsCompleted) {
    for (auto& mem : m_memories) {
        // Count distinct biomes
        int biomeCount = 0;
        for (const auto& kv : m_biomesSeen) {
            if (kv.first.find(mem.key + "|") == 0) ++biomeCount;
        }
        mem.biomeCount = biomeCount;

        // RUN → CAMPAIGN
        if (mem.tier == MemoryTier::RUN &&
            mem.runCount >= MIN_RUNS_FOR_CAMPAIGN) {
            mem.tier = MemoryTier::CAMPAIGN;
        }

        // CAMPAIGN → LEGEND
        if (mem.tier == MemoryTier::CAMPAIGN &&
            runsCompleted >= MIN_RUNS_FOR_LEGEND &&
            mem.biomeCount >= MIN_BIOMES_FOR_LEGEND &&
            mem.strength >= LEGEND_STRENGTH_THRESHOLD) {
            mem.tier = MemoryTier::LEGEND;
        }
    }
}

void LegendSystem::ClearRunMemories() {
    m_memories.erase(
        std::remove_if(m_memories.begin(), m_memories.end(),
            [](const LegendMemory& m) {
                return m.tier == MemoryTier::RUN;
            }),
        m_memories.end());
}

int LegendSystem::MemoryCount(MemoryTier tier) const {
    int n = 0;
    for (const auto& m : m_memories)
        if (m.tier == tier) ++n;
    return n;
}

const LegendMemory* LegendSystem::FindMemory(const std::string& key) const {
    for (const auto& m : m_memories)
        if (m.key == key) return &m;
    return nullptr;
}

std::vector<BossMutation> LegendSystem::DeriveBossMutations() const {
    std::vector<BossMutation> mutations;

    auto add = [&](BossMutation mut) {
        if (static_cast<int>(mutations.size()) < MAX_BOSS_MUTATIONS)
            mutations.push_back(mut);
    };

    // Order mutations by highest reputation score
    struct Candidate { float score; BossMutation mutation; };
    std::vector<Candidate> candidates = {
        { m_reputation.aggression,  BossMutation::COUNTER_RUSH },
        { m_reputation.precision,   BossMutation::MOVING_WEAKPOINTS },
        { m_reputation.stealth,     BossMutation::AREA_SCANS },
        // explosives approximated by chaos + aggression
        { (m_reputation.chaos + m_reputation.aggression) * 0.5f,
          BossMutation::SHIELD_PHASING }
    };

    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) {
                  return a.score > b.score;
              });

    for (const auto& c : candidates) {
        if (c.score >= MUTATION_THRESHOLD)
            add(c.mutation);
    }

    return mutations;
}

} // namespace atlas::gameplay
