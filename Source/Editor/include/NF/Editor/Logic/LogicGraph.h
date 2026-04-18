#pragma once
// NF::Editor — Logic wire graph + template library
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

enum class LogicPinType : uint8_t {
    Flow, Bool, Int, Float, String, Vector, Event, Object
};

inline const char* logicPinTypeName(LogicPinType t) {
    switch (t) {
        case LogicPinType::Flow:   return "Flow";
        case LogicPinType::Bool:   return "Bool";
        case LogicPinType::Int:    return "Int";
        case LogicPinType::Float:  return "Float";
        case LogicPinType::String: return "String";
        case LogicPinType::Vector: return "Vector";
        case LogicPinType::Event:  return "Event";
        case LogicPinType::Object: return "Object";
    }
    return "Unknown";
}

struct LogicPin {
    std::string   id;
    std::string   name;
    LogicPinType  type      = LogicPinType::Flow;
    bool          isOutput  = false;
    bool          connected = false;
    float         value     = 0.f;
};

enum class LogicNodeType : uint8_t {
    AndGate, OrGate, NotGate, Latch, Delay, Switch, Compare, MathOp
};

inline const char* logicNodeTypeName(LogicNodeType t) {
    switch (t) {
        case LogicNodeType::AndGate: return "AND Gate";
        case LogicNodeType::OrGate:  return "OR Gate";
        case LogicNodeType::NotGate: return "NOT Gate";
        case LogicNodeType::Latch:   return "Latch";
        case LogicNodeType::Delay:   return "Delay";
        case LogicNodeType::Switch:  return "Switch";
        case LogicNodeType::Compare: return "Compare";
        case LogicNodeType::MathOp:  return "Math Op";
    }
    return "Unknown";
}

struct LogicNodeDef {
    std::string   name;
    LogicNodeType nodeType = LogicNodeType::AndGate;
    std::vector<LogicPin> inputs;
    std::vector<LogicPin> outputs;
    std::string   description;
};

class LogicWireNode {
public:
    void setId(int id) { m_id = id; }
    [[nodiscard]] int id() const { return m_id; }

    void setName(const std::string& name) { m_name = name; }
    [[nodiscard]] const std::string& name() const { return m_name; }

    void setNodeType(LogicNodeType type) { m_nodeType = type; }
    [[nodiscard]] LogicNodeType nodeType() const { return m_nodeType; }

    bool addInput(const LogicPin& pin) {
        if (m_inputs.size() >= kMaxPins) return false;
        m_inputs.push_back(pin);
        return true;
    }

    bool addOutput(const LogicPin& pin) {
        if (m_outputs.size() >= kMaxPins) return false;
        m_outputs.push_back(pin);
        return true;
    }

    [[nodiscard]] const std::vector<LogicPin>& inputs() const { return m_inputs; }
    [[nodiscard]] const std::vector<LogicPin>& outputs() const { return m_outputs; }

    inline LogicPin* findInput(const std::string& pinId) {
        for (auto& p : m_inputs) { if (p.id == pinId) return &p; }
        return nullptr;
    }
    inline LogicPin* findOutput(const std::string& pinId) {
        for (auto& p : m_outputs) { if (p.id == pinId) return &p; }
        return nullptr;
    }
    inline const LogicPin* findInput(const std::string& pinId) const {
        for (const auto& p : m_inputs) { if (p.id == pinId) return &p; }
        return nullptr;
    }
    inline const LogicPin* findOutput(const std::string& pinId) const {
        for (const auto& p : m_outputs) { if (p.id == pinId) return &p; }
        return nullptr;
    }

    [[nodiscard]] size_t inputCount() const { return m_inputs.size(); }
    [[nodiscard]] size_t outputCount() const { return m_outputs.size(); }

    inline void evaluate() {
        float result = 0.f;
        switch (m_nodeType) {
            case LogicNodeType::AndGate: {
                result = 1.f;
                for (const auto& in : m_inputs) {
                    if (in.value <= 0.5f) { result = 0.f; break; }
                }
                if (m_inputs.empty()) result = 0.f;
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::OrGate: {
                result = 0.f;
                for (const auto& in : m_inputs) {
                    if (in.value > 0.5f) { result = 1.f; break; }
                }
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::NotGate: {
                if (m_inputs.empty()) {
                    result = 1.f;
                } else {
                    result = (m_inputs[0].value <= 0.5f) ? 1.f : 0.f;
                }
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::Latch: {
                if (!m_inputs.empty()) {
                    result = m_inputs[0].value;
                    for (auto& out : m_outputs) out.value = result;
                }
                break;
            }
            case LogicNodeType::Compare: {
                if (m_inputs.size() >= 2) {
                    result = (m_inputs[0].value == m_inputs[1].value) ? 1.f : 0.f;
                } else {
                    result = 0.f;
                }
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::MathOp: {
                result = 0.f;
                for (const auto& in : m_inputs) result += in.value;
                for (auto& out : m_outputs) out.value = result;
                break;
            }
            case LogicNodeType::Delay:
            case LogicNodeType::Switch:
            default: {
                result = m_inputs.empty() ? 0.f : m_inputs[0].value;
                for (auto& out : m_outputs) out.value = result;
                break;
            }
        }
    }

    static constexpr size_t kMaxPins = 16;

private:
    int m_id = 0;
    std::string m_name;
    LogicNodeType m_nodeType = LogicNodeType::AndGate;
    std::vector<LogicPin> m_inputs;
    std::vector<LogicPin> m_outputs;
};

struct LogicWire {
    int sourceNodeId = -1;
    std::string sourcePin;
    int targetNodeId = -1;
    std::string targetPin;
};

class LogicWireGraph {
public:
    inline int addNode(LogicWireNode node) {
        if (m_nodes.size() >= kMaxNodes) return -1;
        int nid = m_nextId++;
        node.setId(nid);
        m_nodes.push_back(std::move(node));
        return nid;
    }

    inline bool removeNode(int id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [id](const LogicWireNode& n) { return n.id() == id; });
        if (it == m_nodes.end()) return false;
        // Remove wires referencing this node
        m_wires.erase(
            std::remove_if(m_wires.begin(), m_wires.end(),
                [id](const LogicWire& w) {
                    return w.sourceNodeId == id || w.targetNodeId == id;
                }),
            m_wires.end());
        m_nodes.erase(it);
        return true;
    }

    [[nodiscard]] inline const LogicWireNode* findNode(int id) const {
        for (const auto& n : m_nodes) { if (n.id() == id) return &n; }
        return nullptr;
    }

    inline LogicWireNode* findNode(int id) {
        for (auto& n : m_nodes) { if (n.id() == id) return &n; }
        return nullptr;
    }

    inline bool addWire(const LogicWire& wire) {
        if (m_wires.size() >= kMaxWires) return false;
        if (!findNode(wire.sourceNodeId) || !findNode(wire.targetNodeId)) return false;
        m_wires.push_back(wire);
        return true;
    }

    inline bool removeWire(size_t index) {
        if (index >= m_wires.size()) return false;
        m_wires.erase(m_wires.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
    }

    [[nodiscard]] size_t nodeCount() const { return m_nodes.size(); }
    [[nodiscard]] size_t wireCount() const { return m_wires.size(); }

    [[nodiscard]] const std::vector<LogicWireNode>& nodes() const { return m_nodes; }
    [[nodiscard]] const std::vector<LogicWire>& wires() const { return m_wires; }

    void clear() {
        m_nodes.clear();
        m_wires.clear();
        m_nextId = 1;
    }

    [[nodiscard]] inline bool isValid() const {
        for (const auto& w : m_wires) {
            if (!findNode(w.sourceNodeId) || !findNode(w.targetNodeId))
                return false;
        }
        return true;
    }

    inline void evaluate() {
        for (auto& n : m_nodes) n.evaluate();
    }

    static constexpr size_t kMaxNodes = 128;
    static constexpr size_t kMaxWires = 256;

private:
    std::vector<LogicWireNode> m_nodes;
    std::vector<LogicWire> m_wires;
    int m_nextId = 1;
};

struct LogicGraphTemplate {
    std::string name;
    std::string description;
    std::string category;
    std::vector<LogicNodeDef> nodeDefs;
};

class LogicTemplateLibrary {
public:
    inline bool addTemplate(const LogicGraphTemplate& tmpl) {
        if (m_templates.size() >= kMaxTemplates) return false;
        for (const auto& t : m_templates) {
            if (t.name == tmpl.name) return false;
        }
        m_templates.push_back(tmpl);
        return true;
    }

    inline bool removeTemplate(const std::string& name) {
        auto it = std::find_if(m_templates.begin(), m_templates.end(),
            [&name](const LogicGraphTemplate& t) { return t.name == name; });
        if (it == m_templates.end()) return false;
        m_templates.erase(it);
        return true;
    }

    [[nodiscard]] inline const LogicGraphTemplate* findTemplate(const std::string& name) const {
        for (const auto& t : m_templates) {
            if (t.name == name) return &t;
        }
        return nullptr;
    }

    [[nodiscard]] inline std::vector<const LogicGraphTemplate*>
    templatesInCategory(const std::string& category) const {
        std::vector<const LogicGraphTemplate*> result;
        for (const auto& t : m_templates) {
            if (t.category == category) result.push_back(&t);
        }
        return result;
    }

    [[nodiscard]] size_t templateCount() const { return m_templates.size(); }
    [[nodiscard]] const std::vector<LogicGraphTemplate>& templates() const { return m_templates; }

    void clear() { m_templates.clear(); }

    [[nodiscard]] inline size_t categoryCount() const {
        std::set<std::string> cats;
        for (const auto& t : m_templates) cats.insert(t.category);
        return cats.size();
    }

    static constexpr size_t kMaxTemplates = 64;

private:
    std::vector<LogicGraphTemplate> m_templates;
};

// ── S8 — Tool Ecosystem ──────────────────────────────────────────


} // namespace NF
