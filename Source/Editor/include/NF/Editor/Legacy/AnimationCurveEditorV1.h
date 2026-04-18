#pragma once
// NF::Editor — Animation curve editor v1: keyframe curves with tangent interpolation
#include "NF/Core/Core.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class AcvTangentType : uint8_t { Linear, Stepped, Bezier, Auto };

struct AcvKey {
    uint32_t       id          = 0;
    float          time        = 0.f;
    float          value       = 0.f;
    AcvTangentType tangent     = AcvTangentType::Linear;
    float          inTangent   = 0.f;
    float          outTangent  = 0.f;
    [[nodiscard]] bool isValid() const { return id != 0; }
};

struct AcvCurve {
    uint32_t            id      = 0;
    std::string         name;
    std::vector<AcvKey> keys;
    bool                looping = false;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }

    AcvKey* findKeyAtTime(float t, float tol = 0.001f) {
        for (auto& k : keys)
            if (std::fabs(k.time - t) <= tol) return &k;
        return nullptr;
    }

    float evaluate(float t) const {
        if (keys.empty()) return 0.f;
        if (keys.size() == 1) return keys[0].value;
        for (size_t i = 0; i + 1 < keys.size(); ++i) {
            const auto& a = keys[i];
            const auto& b = keys[i + 1];
            if (t >= a.time && t <= b.time) {
                float dt = b.time - a.time;
                if (dt < 1e-9f) return a.value;
                float u = (t - a.time) / dt;
                if (a.tangent == AcvTangentType::Stepped) return a.value;
                return a.value + u * (b.value - a.value);
            }
        }
        return keys.back().value;
    }

    [[nodiscard]] size_t keyCount() const { return keys.size(); }
};

using AcvChangeCallback = std::function<void(uint32_t)>;

class AnimationCurveEditorV1 {
public:
    bool addCurve(const AcvCurve& curve) {
        if (!curve.isValid()) return false;
        for (const auto& c : m_curves) if (c.id == curve.id) return false;
        m_curves.push_back(curve);
        return true;
    }

    bool removeCurve(uint32_t id) {
        for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
            if (it->id == id) { m_curves.erase(it); return true; }
        }
        return false;
    }

    bool addKey(uint32_t curveId, const AcvKey& key) {
        AcvCurve* c = findCurve_(curveId);
        if (!c || !key.isValid()) return false;
        for (const auto& k : c->keys) if (k.id == key.id) return false;
        c->keys.push_back(key);
        std::sort(c->keys.begin(), c->keys.end(),
                  [](const AcvKey& a, const AcvKey& b){ return a.time < b.time; });
        if (m_onChange) m_onChange(curveId);
        return true;
    }

    bool removeKey(uint32_t curveId, uint32_t keyId) {
        AcvCurve* c = findCurve_(curveId);
        if (!c) return false;
        for (auto it = c->keys.begin(); it != c->keys.end(); ++it) {
            if (it->id == keyId) {
                c->keys.erase(it);
                if (m_onChange) m_onChange(curveId);
                return true;
            }
        }
        return false;
    }

    float evaluate(uint32_t curveId, float t) const {
        const AcvCurve* c = findCurve_(curveId);
        return c ? c->evaluate(t) : 0.f;
    }

    void setOnChange(AcvChangeCallback cb) { m_onChange = std::move(cb); }

    [[nodiscard]] size_t curveCount() const { return m_curves.size(); }

private:
    AcvCurve* findCurve_(uint32_t id) {
        for (auto& c : m_curves) if (c.id == id) return &c;
        return nullptr;
    }
    const AcvCurve* findCurve_(uint32_t id) const {
        for (const auto& c : m_curves) if (c.id == id) return &c;
        return nullptr;
    }

    std::vector<AcvCurve> m_curves;
    AcvChangeCallback     m_onChange;
};

} // namespace NF
