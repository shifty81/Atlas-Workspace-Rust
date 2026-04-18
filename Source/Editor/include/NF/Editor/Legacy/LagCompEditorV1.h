#pragma once
// NF::Editor — Lag compensation editor v1: rewind buffer and hit-validation authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Lcv1RewindMode  : uint8_t { Disabled, Snapshot, Interpolated };
enum class Lcv1HitValMethod: uint8_t { ServerSide, ClientSide, Hybrid };

struct Lcv1RewindConfig {
    float        bufferMs    = 200.f;
    float        maxLatencyMs= 300.f;
    uint32_t     snapshotRate= 20;
    Lcv1RewindMode mode      = Lcv1RewindMode::Interpolated;
    [[nodiscard]] bool isValid() const { return bufferMs > 0.f && snapshotRate > 0; }
};

struct Lcv1HitRecord {
    uint64_t id          = 0;
    uint64_t attackerId  = 0;
    uint64_t targetId    = 0;
    uint64_t timestampMs = 0;
    float    latencyMs   = 0.f;
    bool     validated   = false;
    [[nodiscard]] bool isValid() const { return id != 0 && attackerId != 0 && targetId != 0; }
};

using Lcv1HitCallback = std::function<void(const Lcv1HitRecord&)>;

class LagCompEditorV1 {
public:
    bool setRewindConfig(const Lcv1RewindConfig& cfg) {
        if (!cfg.isValid()) return false;
        m_config = cfg;
        return true;
    }

    void setHitValidMethod(Lcv1HitValMethod m) { m_hitMethod = m; }

    bool addHitRecord(const Lcv1HitRecord& rec) {
        if (!rec.isValid()) return false;
        for (const auto& er : m_hits) if (er.id == rec.id) return false;
        m_hits.push_back(rec);
        if (m_onHit) m_onHit(rec);
        return true;
    }

    bool validateHit(uint64_t id, bool valid) {
        for (auto& h : m_hits) {
            if (h.id == id) { h.validated = valid; return true; }
        }
        return false;
    }

    bool clearHits() {
        if (m_hits.empty()) return false;
        m_hits.clear();
        return true;
    }

    [[nodiscard]] Lcv1RewindConfig   rewindConfig()   const { return m_config;     }
    [[nodiscard]] Lcv1HitValMethod   hitValMethod()   const { return m_hitMethod;  }
    [[nodiscard]] size_t             hitCount()       const { return m_hits.size(); }

    void setOnHit(Lcv1HitCallback cb) { m_onHit = std::move(cb); }

private:
    Lcv1RewindConfig            m_config;
    Lcv1HitValMethod            m_hitMethod = Lcv1HitValMethod::Hybrid;
    std::vector<Lcv1HitRecord>  m_hits;
    Lcv1HitCallback             m_onHit;
};

} // namespace NF
