#pragma once
// NF::Editor — Mesh deformer editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class DeformerType : uint8_t {
    Blend, Lattice, FFD, Morph, SkinWrap, Wave, Twist, Bend, Noise
};

inline const char* deformerTypeName(DeformerType t) {
    switch (t) {
        case DeformerType::Blend:    return "Blend";
        case DeformerType::Lattice:  return "Lattice";
        case DeformerType::FFD:      return "FFD";
        case DeformerType::Morph:    return "Morph";
        case DeformerType::SkinWrap: return "SkinWrap";
        case DeformerType::Wave:     return "Wave";
        case DeformerType::Twist:    return "Twist";
        case DeformerType::Bend:     return "Bend";
        case DeformerType::Noise:    return "Noise";
    }
    return "Unknown";
}

enum class DeformerEvalOrder : uint8_t {
    Sequential, Parallel, LayeredBlend
};

inline const char* deformerEvalOrderName(DeformerEvalOrder o) {
    switch (o) {
        case DeformerEvalOrder::Sequential:   return "Sequential";
        case DeformerEvalOrder::Parallel:     return "Parallel";
        case DeformerEvalOrder::LayeredBlend: return "LayeredBlend";
    }
    return "Unknown";
}

class DeformerLayer {
public:
    explicit DeformerLayer(uint32_t id, DeformerType type) : m_id(id), m_type(type) {}

    void setWeight(float w)    { m_weight  = w; }
    void setEnabled(bool v)    { m_enabled = v; }
    void setName(const std::string& n) { m_name = n; }

    [[nodiscard]] uint32_t      id()      const { return m_id;      }
    [[nodiscard]] DeformerType  type()    const { return m_type;    }
    [[nodiscard]] float         weight()  const { return m_weight;  }
    [[nodiscard]] bool          isEnabled()const{ return m_enabled; }
    [[nodiscard]] const std::string& name() const { return m_name;  }

private:
    uint32_t     m_id;
    DeformerType m_type;
    std::string  m_name;
    float        m_weight  = 1.0f;
    bool         m_enabled = true;
};

class MeshDeformerEditor {
public:
    void setEvalOrder(DeformerEvalOrder o) { m_evalOrder = o; }
    void setTargetMesh(const std::string& n){ m_targetMesh = n; }

    [[nodiscard]] bool addLayer(const DeformerLayer& layer) {
        for (auto& l : m_layers) if (l.id() == layer.id()) return false;
        m_layers.push_back(layer);
        return true;
    }

    [[nodiscard]] bool removeLayer(uint32_t id) {
        for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
            if (it->id() == id) { m_layers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] DeformerLayer* findLayer(uint32_t id) {
        for (auto& l : m_layers) if (l.id() == id) return &l;
        return nullptr;
    }

    [[nodiscard]] DeformerEvalOrder    evalOrder()   const { return m_evalOrder;  }
    [[nodiscard]] const std::string&   targetMesh()  const { return m_targetMesh; }
    [[nodiscard]] size_t               layerCount()  const { return m_layers.size(); }

    [[nodiscard]] size_t enabledLayerCount() const {
        size_t c = 0; for (auto& l : m_layers) if (l.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(DeformerType t) const {
        size_t c = 0; for (auto& l : m_layers) if (l.type() == t) ++c; return c;
    }

private:
    std::string               m_targetMesh;
    DeformerEvalOrder         m_evalOrder = DeformerEvalOrder::Sequential;
    std::vector<DeformerLayer> m_layers;
};

} // namespace NF
