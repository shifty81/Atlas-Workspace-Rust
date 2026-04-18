#pragma once
// NF::Editor — Data binding system v1: property binding, source registration, and sync lifecycle
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Dbsv1BindingMode  : uint8_t { OneWay, TwoWay, OneTime, OneWayToSource };
enum class Dbsv1BindingState : uint8_t { Unbound, Bound, Stale, Error, Suspended };
enum class Dbsv1ValueType    : uint8_t { Bool, Int, Float, String, Vector2, Vector3, Color, Object };

inline const char* dbsv1BindingModeName(Dbsv1BindingMode m) {
    switch (m) {
        case Dbsv1BindingMode::OneWay:         return "OneWay";
        case Dbsv1BindingMode::TwoWay:         return "TwoWay";
        case Dbsv1BindingMode::OneTime:        return "OneTime";
        case Dbsv1BindingMode::OneWayToSource: return "OneWayToSource";
    }
    return "Unknown";
}

inline const char* dbsv1BindingStateName(Dbsv1BindingState s) {
    switch (s) {
        case Dbsv1BindingState::Unbound:   return "Unbound";
        case Dbsv1BindingState::Bound:     return "Bound";
        case Dbsv1BindingState::Stale:     return "Stale";
        case Dbsv1BindingState::Error:     return "Error";
        case Dbsv1BindingState::Suspended: return "Suspended";
    }
    return "Unknown";
}

inline const char* dbsv1ValueTypeName(Dbsv1ValueType t) {
    switch (t) {
        case Dbsv1ValueType::Bool:    return "Bool";
        case Dbsv1ValueType::Int:     return "Int";
        case Dbsv1ValueType::Float:   return "Float";
        case Dbsv1ValueType::String:  return "String";
        case Dbsv1ValueType::Vector2: return "Vector2";
        case Dbsv1ValueType::Vector3: return "Vector3";
        case Dbsv1ValueType::Color:   return "Color";
        case Dbsv1ValueType::Object:  return "Object";
    }
    return "Unknown";
}

struct Dbsv1Binding {
    uint64_t           id          = 0;
    std::string        sourceId;
    std::string        sourcePath;
    std::string        targetId;
    std::string        targetPath;
    Dbsv1BindingMode   mode        = Dbsv1BindingMode::OneWay;
    Dbsv1BindingState  state       = Dbsv1BindingState::Unbound;
    Dbsv1ValueType     valueType   = Dbsv1ValueType::String;
    bool               isEnabled   = true;

    [[nodiscard]] bool isValid()  const { return id != 0 && !sourceId.empty() && !targetId.empty(); }
    [[nodiscard]] bool isBound()  const { return state == Dbsv1BindingState::Bound; }
    [[nodiscard]] bool hasError() const { return state == Dbsv1BindingState::Error; }
    [[nodiscard]] bool isStale()  const { return state == Dbsv1BindingState::Stale; }
};

using Dbsv1ChangeCallback = std::function<void(uint64_t)>;

class DataBindingSystemV1 {
public:
    static constexpr size_t MAX_BINDINGS = 1024;

    bool addBinding(const Dbsv1Binding& binding) {
        if (!binding.isValid()) return false;
        for (const auto& b : m_bindings) if (b.id == binding.id) return false;
        if (m_bindings.size() >= MAX_BINDINGS) return false;
        m_bindings.push_back(binding);
        return true;
    }

    bool removeBinding(uint64_t id) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->id == id) { m_bindings.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Dbsv1Binding* findBinding(uint64_t id) {
        for (auto& b : m_bindings) if (b.id == id) return &b;
        return nullptr;
    }

    bool bind(uint64_t id) {
        auto* b = findBinding(id);
        if (!b) return false;
        b->state = Dbsv1BindingState::Bound;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool unbind(uint64_t id) {
        auto* b = findBinding(id);
        if (!b) return false;
        b->state = Dbsv1BindingState::Unbound;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setState(uint64_t id, Dbsv1BindingState state) {
        auto* b = findBinding(id);
        if (!b) return false;
        b->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setEnabled(uint64_t id, bool enabled) {
        auto* b = findBinding(id);
        if (!b) return false;
        b->isEnabled = enabled;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t bindingCount()  const { return m_bindings.size(); }
    [[nodiscard]] size_t boundCount()    const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.isBound())  ++c; return c;
    }
    [[nodiscard]] size_t errorCount()    const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.hasError()) ++c; return c;
    }
    [[nodiscard]] size_t staleCount()    const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.isStale())  ++c; return c;
    }
    [[nodiscard]] size_t enabledCount()  const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.isEnabled)  ++c; return c;
    }
    [[nodiscard]] size_t countByMode(Dbsv1BindingMode mode) const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.mode == mode) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Dbsv1ValueType type) const {
        size_t c = 0; for (const auto& b : m_bindings) if (b.valueType == type) ++c; return c;
    }

    void setOnChange(Dbsv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Dbsv1Binding> m_bindings;
    Dbsv1ChangeCallback       m_onChange;
};

} // namespace NF
