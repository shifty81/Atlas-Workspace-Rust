#pragma once
// NF::Editor — State graph v1: lightweight FSM authoring with transitions
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Sgv1NodeKind : uint8_t { Entry, State, AnyState, Exit };
enum class Sgv1CondOp   : uint8_t { Equal, NotEqual, Greater, Less, Always };

struct Sgv1Condition {
    std::string param;
    Sgv1CondOp  op    = Sgv1CondOp::Always;
    float       value = 0.f;
};

struct Sgv1Transition {
    uint64_t          id       = 0;
    uint64_t          fromNode = 0;
    uint64_t          toNode   = 0;
    float             duration = 0.f;
    std::vector<Sgv1Condition> conditions;
    [[nodiscard]] bool isValid() const { return id != 0 && fromNode != 0 && toNode != 0; }
};

struct Sgv1Node {
    uint64_t      id   = 0;
    std::string   name;
    Sgv1NodeKind  kind = Sgv1NodeKind::State;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Sgv1StateCallback = std::function<void(uint64_t)>;

class StateGraphV1 {
public:
    bool addNode(const Sgv1Node& node) {
        if (!node.isValid()) return false;
        for (const auto& n : m_nodes) if (n.id == node.id) return false;
        m_nodes.push_back(node);
        return true;
    }

    bool removeNode(uint64_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id == id) { m_nodes.erase(it); return true; }
        }
        return false;
    }

    bool addTransition(const Sgv1Transition& t) {
        if (!t.isValid()) return false;
        for (const auto& tr : m_transitions) if (tr.id == t.id) return false;
        m_transitions.push_back(t);
        return true;
    }

    bool removeTransition(uint64_t id) {
        for (auto it = m_transitions.begin(); it != m_transitions.end(); ++it) {
            if (it->id == id) { m_transitions.erase(it); return true; }
        }
        return false;
    }

    bool setActiveNode(uint64_t id) {
        for (const auto& n : m_nodes) {
            if (n.id == id) {
                m_activeNode = id;
                if (m_onEnter) m_onEnter(id);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint64_t activeNode()      const { return m_activeNode;       }
    [[nodiscard]] size_t   nodeCount()       const { return m_nodes.size();      }
    [[nodiscard]] size_t   transitionCount() const { return m_transitions.size(); }

    void setOnEnter(Sgv1StateCallback cb) { m_onEnter = std::move(cb); }

private:
    std::vector<Sgv1Node>       m_nodes;
    std::vector<Sgv1Transition> m_transitions;
    uint64_t                    m_activeNode = 0;
    Sgv1StateCallback           m_onEnter;
};

} // namespace NF
