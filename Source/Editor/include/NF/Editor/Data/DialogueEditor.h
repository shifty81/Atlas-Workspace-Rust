#pragma once
// NF::Editor — Dialogue editor panel
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

enum class DialogueNodeType : uint8_t {
    Start, Speech, Choice, Branch, Event, End
};

inline const char* dialogueNodeTypeName(DialogueNodeType t) {
    switch (t) {
        case DialogueNodeType::Start:  return "Start";
        case DialogueNodeType::Speech: return "Speech";
        case DialogueNodeType::Choice: return "Choice";
        case DialogueNodeType::Branch: return "Branch";
        case DialogueNodeType::Event:  return "Event";
        case DialogueNodeType::End:    return "End";
    }
    return "Unknown";
}

enum class DialogueSpeakerRole : uint8_t {
    Player, NPC, Narrator, System
};

inline const char* dialogueSpeakerRoleName(DialogueSpeakerRole r) {
    switch (r) {
        case DialogueSpeakerRole::Player:   return "Player";
        case DialogueSpeakerRole::NPC:      return "NPC";
        case DialogueSpeakerRole::Narrator: return "Narrator";
        case DialogueSpeakerRole::System:   return "System";
    }
    return "Unknown";
}

enum class DialogueConditionType : uint8_t {
    None, HasItem, HasFlag, QuestActive, QuestDone, StatCheck, Custom
};

inline const char* dialogueConditionTypeName(DialogueConditionType t) {
    switch (t) {
        case DialogueConditionType::None:       return "None";
        case DialogueConditionType::HasItem:    return "HasItem";
        case DialogueConditionType::HasFlag:    return "HasFlag";
        case DialogueConditionType::QuestActive:return "QuestActive";
        case DialogueConditionType::QuestDone:  return "QuestDone";
        case DialogueConditionType::StatCheck:  return "StatCheck";
        case DialogueConditionType::Custom:     return "Custom";
    }
    return "Unknown";
}

class DialogueNode {
public:
    explicit DialogueNode(uint32_t id, DialogueNodeType type)
        : m_id(id), m_type(type) {}

    void setSpeakerRole(DialogueSpeakerRole r) { m_speakerRole = r; }
    void setText(const std::string& t)         { m_text        = t; }
    void setCondition(DialogueConditionType c) { m_condition   = c; }
    void setEnabled(bool v)                    { m_enabled     = v; }

    [[nodiscard]] uint32_t             id()         const { return m_id;         }
    [[nodiscard]] DialogueNodeType     type()       const { return m_type;       }
    [[nodiscard]] DialogueSpeakerRole  speakerRole()const { return m_speakerRole;}
    [[nodiscard]] const std::string&   text()       const { return m_text;       }
    [[nodiscard]] DialogueConditionType condition() const { return m_condition;  }
    [[nodiscard]] bool                 isEnabled()  const { return m_enabled;    }

private:
    uint32_t              m_id;
    DialogueNodeType      m_type;
    DialogueSpeakerRole   m_speakerRole = DialogueSpeakerRole::NPC;
    DialogueConditionType m_condition   = DialogueConditionType::None;
    std::string           m_text;
    bool                  m_enabled     = true;
};

class DialogueGraph {
public:
    explicit DialogueGraph(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addNode(const DialogueNode& node) {
        for (auto& n : m_nodes) if (n.id() == node.id()) return false;
        m_nodes.push_back(node); return true;
    }

    [[nodiscard]] bool removeNode(uint32_t id) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            if (it->id() == id) { m_nodes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] DialogueNode* findNode(uint32_t id) {
        for (auto& n : m_nodes) if (n.id() == id) return &n;
        return nullptr;
    }

    [[nodiscard]] const std::string& name()      const { return m_name;          }
    [[nodiscard]] size_t             nodeCount() const { return m_nodes.size();  }
    [[nodiscard]] size_t countByType(DialogueNodeType t) const {
        size_t c = 0; for (auto& n : m_nodes) if (n.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countBySpeaker(DialogueSpeakerRole r) const {
        size_t c = 0; for (auto& n : m_nodes) if (n.speakerRole() == r) ++c; return c;
    }

private:
    std::string               m_name;
    std::vector<DialogueNode> m_nodes;
};

class DialogueEditor {
public:
    static constexpr size_t MAX_GRAPHS = 256;

    [[nodiscard]] bool addGraph(const DialogueGraph& graph) {
        for (auto& g : m_graphs) if (g.name() == graph.name()) return false;
        if (m_graphs.size() >= MAX_GRAPHS) return false;
        m_graphs.push_back(graph);
        return true;
    }

    [[nodiscard]] bool removeGraph(const std::string& name) {
        for (auto it = m_graphs.begin(); it != m_graphs.end(); ++it) {
            if (it->name() == name) { m_graphs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] DialogueGraph* findGraph(const std::string& name) {
        for (auto& g : m_graphs) if (g.name() == name) return &g;
        return nullptr;
    }

    [[nodiscard]] size_t graphCount() const { return m_graphs.size(); }
    [[nodiscard]] size_t totalNodeCount() const {
        size_t c = 0; for (auto& g : m_graphs) c += g.nodeCount(); return c;
    }

private:
    std::vector<DialogueGraph> m_graphs;
};

} // namespace NF
