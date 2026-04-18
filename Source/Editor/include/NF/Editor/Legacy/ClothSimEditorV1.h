#pragma once
// NF::Editor — Cloth simulation editor v1: cloth layer and constraint management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Csev1ClothType  : uint8_t { Rigid, Soft, Hybrid, Tearable, Wind };
enum class Csev1ClothState : uint8_t { Inactive, Simulating, Paused, Baked, Error };

inline const char* csev1ClothTypeName(Csev1ClothType t) {
    switch (t) {
        case Csev1ClothType::Rigid:    return "Rigid";
        case Csev1ClothType::Soft:     return "Soft";
        case Csev1ClothType::Hybrid:   return "Hybrid";
        case Csev1ClothType::Tearable: return "Tearable";
        case Csev1ClothType::Wind:     return "Wind";
    }
    return "Unknown";
}

inline const char* csev1ClothStateName(Csev1ClothState s) {
    switch (s) {
        case Csev1ClothState::Inactive:   return "Inactive";
        case Csev1ClothState::Simulating: return "Simulating";
        case Csev1ClothState::Paused:     return "Paused";
        case Csev1ClothState::Baked:      return "Baked";
        case Csev1ClothState::Error:      return "Error";
    }
    return "Unknown";
}

struct Csev1ClothLayer {
    uint64_t        id          = 0;
    std::string     name;
    Csev1ClothType  clothType   = Csev1ClothType::Soft;
    Csev1ClothState state       = Csev1ClothState::Inactive;
    float           stiffness   = 1.0f;
    float           damping     = 0.1f;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isSimulating() const { return state == Csev1ClothState::Simulating; }
    [[nodiscard]] bool isBaked()      const { return state == Csev1ClothState::Baked; }
    [[nodiscard]] bool isError()      const { return state == Csev1ClothState::Error; }
};

struct Csev1Constraint {
    uint64_t    id      = 0;
    uint64_t    layerId = 0;
    std::string name;
    float       strength = 1.0f;

    [[nodiscard]] bool isValid() const { return id != 0 && layerId != 0 && !name.empty(); }
};

using Csev1ChangeCallback = std::function<void(uint64_t)>;

class ClothSimEditorV1 {
public:
    static constexpr size_t MAX_LAYERS      = 128;
    static constexpr size_t MAX_CONSTRAINTS = 512;

    bool addLayer(const Csev1ClothLayer& layer) {
        if (!layer.isValid()) return false;
        for (const auto& l : m_layers) if (l.id == layer.id) return false;
        if (m_layers.size() >= MAX_LAYERS) return false;
        m_layers.push_back(layer);
        if (m_onChange) m_onChange(layer.id);
        return true;
    }

    bool removeLayer(uint64_t id) {
        for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
            if (it->id == id) { m_layers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Csev1ClothLayer* findLayer(uint64_t id) {
        for (auto& l : m_layers) if (l.id == id) return &l;
        return nullptr;
    }

    bool addConstraint(const Csev1Constraint& constraint) {
        if (!constraint.isValid()) return false;
        for (const auto& c : m_constraints) if (c.id == constraint.id) return false;
        if (m_constraints.size() >= MAX_CONSTRAINTS) return false;
        m_constraints.push_back(constraint);
        if (m_onChange) m_onChange(constraint.layerId);
        return true;
    }

    bool removeConstraint(uint64_t id) {
        for (auto it = m_constraints.begin(); it != m_constraints.end(); ++it) {
            if (it->id == id) { m_constraints.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t layerCount()      const { return m_layers.size(); }
    [[nodiscard]] size_t constraintCount() const { return m_constraints.size(); }

    [[nodiscard]] size_t simulatingCount() const {
        size_t c = 0; for (const auto& l : m_layers) if (l.isSimulating()) ++c; return c;
    }
    [[nodiscard]] size_t bakedCount() const {
        size_t c = 0; for (const auto& l : m_layers) if (l.isBaked()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Csev1ClothType type) const {
        size_t c = 0; for (const auto& l : m_layers) if (l.clothType == type) ++c; return c;
    }
    [[nodiscard]] size_t constraintsForLayer(uint64_t layerId) const {
        size_t c = 0; for (const auto& con : m_constraints) if (con.layerId == layerId) ++c; return c;
    }

    void setOnChange(Csev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Csev1ClothLayer>  m_layers;
    std::vector<Csev1Constraint>  m_constraints;
    Csev1ChangeCallback           m_onChange;
};

} // namespace NF
