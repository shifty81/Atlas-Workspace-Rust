#pragma once
// NF::Editor — Curve library v1: named curve asset management with categories
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Clv1Category : uint8_t { Animation, Material, Gameplay, Audio, UI, Custom };
enum class Clv1Interp   : uint8_t { Linear, Bezier, Hermite, Step };

struct Clv1Keyframe {
    float time  = 0.f;
    float value = 0.f;
    float inTan = 0.f;
    float outTan= 0.f;
};

struct Clv1Curve {
    uint64_t              id   = 0;
    std::string           name;
    Clv1Category          category = Clv1Category::Animation;
    Clv1Interp            interp   = Clv1Interp::Linear;
    std::vector<Clv1Keyframe> keys;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Clv1ChangeCallback = std::function<void(uint64_t)>;

class CurveLibraryV1 {
public:
    bool addCurve(const Clv1Curve& c) {
        if (!c.isValid()) return false;
        for (const auto& ec : m_curves) if (ec.id == c.id) return false;
        m_curves.push_back(c);
        return true;
    }

    bool removeCurve(uint64_t id) {
        for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
            if (it->id == id) { m_curves.erase(it); return true; }
        }
        return false;
    }

    bool renameCurve(uint64_t id, const std::string& name) {
        if (name.empty()) return false;
        for (auto& c : m_curves) {
            if (c.id == id) { c.name = name; if (m_onChange) m_onChange(id); return true; }
        }
        return false;
    }

    bool addKeyframe(uint64_t curveId, const Clv1Keyframe& kf) {
        for (auto& c : m_curves) {
            if (c.id == curveId) { c.keys.push_back(kf); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t curveCount() const { return m_curves.size(); }

    [[nodiscard]] const Clv1Curve* findCurve(uint64_t id) const {
        for (const auto& c : m_curves) if (c.id == id) return &c;
        return nullptr;
    }

    void setOnChange(Clv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Clv1Curve>  m_curves;
    Clv1ChangeCallback      m_onChange;
};

} // namespace NF
