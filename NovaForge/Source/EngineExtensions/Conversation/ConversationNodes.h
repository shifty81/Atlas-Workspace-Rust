#pragma once
#include "ConversationGraph.h"

namespace atlas::conversation {

// DialogueNode: Emits dialogue text from a speaker
class DialogueNode : public ConversationNode {
public:
    std::string dialogueText;

    const char* GetName() const override { return "Dialogue"; }
    const char* GetCategory() const override { return "Conversation"; }

    std::vector<ConversationPort> Inputs() const override {
        return {};
    }

    std::vector<ConversationPort> Outputs() const override {
        return {{ "dialogue", ConversationPinType::Dialogue }};
    }

    void Evaluate(
        const ConversationContext& /*ctx*/,
        const std::vector<ConversationValue>& /*inputs*/,
        std::vector<ConversationValue>& outputs
    ) const override {
        ConversationValue val;
        val.type = ConversationPinType::Dialogue;
        val.text = dialogueText;
        outputs[0] = std::move(val);
    }
};

// PlayerChoiceNode: Presents choices and emits the selected one
class PlayerChoiceNode : public ConversationNode {
public:
    std::vector<std::string> choices;
    int selectedIndex = 0;

    const char* GetName() const override { return "PlayerChoice"; }
    const char* GetCategory() const override { return "Conversation"; }

    std::vector<ConversationPort> Inputs() const override {
        return {{ "prompt", ConversationPinType::Dialogue }};
    }

    std::vector<ConversationPort> Outputs() const override {
        return {{ "choice", ConversationPinType::Choice }};
    }

    void Evaluate(
        const ConversationContext& /*ctx*/,
        const std::vector<ConversationValue>& /*inputs*/,
        std::vector<ConversationValue>& outputs
    ) const override {
        ConversationValue val;
        val.type = ConversationPinType::Choice;
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(choices.size())) {
            val.text = choices[selectedIndex];
        }
        val.numericValue = static_cast<float>(selectedIndex);
        outputs[0] = std::move(val);
    }
};

// MemoryWriteNode: Writes a key-value pair to AI memory
class MemoryWriteNode : public ConversationNode {
public:
    std::string memoryKey;
    std::string memoryValue;

    const char* GetName() const override { return "MemoryWrite"; }
    const char* GetCategory() const override { return "Conversation"; }

    std::vector<ConversationPort> Inputs() const override {
        return {{ "trigger", ConversationPinType::Dialogue }};
    }

    std::vector<ConversationPort> Outputs() const override {
        return {{ "memory", ConversationPinType::Memory }};
    }

    void Evaluate(
        const ConversationContext& /*ctx*/,
        const std::vector<ConversationValue>& /*inputs*/,
        std::vector<ConversationValue>& outputs
    ) const override {
        ConversationValue val;
        val.type = ConversationPinType::Memory;
        val.metadata[memoryKey] = memoryValue;
        outputs[0] = std::move(val);
    }
};

// RelationshipDeltaNode: Modifies relationship value between entities
class RelationshipDeltaNode : public ConversationNode {
public:
    float delta = 0.0f;

    const char* GetName() const override { return "RelationshipDelta"; }
    const char* GetCategory() const override { return "Conversation"; }

    std::vector<ConversationPort> Inputs() const override {
        return {{ "trigger", ConversationPinType::Dialogue }};
    }

    std::vector<ConversationPort> Outputs() const override {
        return {{ "relationship", ConversationPinType::Relationship }};
    }

    void Evaluate(
        const ConversationContext& ctx,
        const std::vector<ConversationValue>& /*inputs*/,
        std::vector<ConversationValue>& outputs
    ) const override {
        ConversationValue val;
        val.type = ConversationPinType::Relationship;
        val.numericValue = delta;
        val.metadata["speaker"] = std::to_string(ctx.speakerID);
        val.metadata["listener"] = std::to_string(ctx.listenerID);
        outputs[0] = std::move(val);
    }
};

}
