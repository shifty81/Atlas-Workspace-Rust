#pragma once
#include "UIGraph.h"

namespace atlas::ui {

class PanelNode : public UINode {
public:
    float width = 100.0f;
    float height = 100.0f;

    const char* GetName() const override { return "Panel"; }
    const char* GetCategory() const override { return "Container"; }
    std::vector<UIPort> Inputs() const override;
    std::vector<UIPort> Outputs() const override;
    void Evaluate(const UIContext& ctx, const std::vector<UIValue>& inputs, std::vector<UIValue>& outputs) const override;
};

class ButtonNode : public UINode {
public:
    const char* GetName() const override { return "Button"; }
    const char* GetCategory() const override { return "Interactive"; }
    std::vector<UIPort> Inputs() const override;
    std::vector<UIPort> Outputs() const override;
    void Evaluate(const UIContext& ctx, const std::vector<UIValue>& inputs, std::vector<UIValue>& outputs) const override;
};

class TextNode : public UINode {
public:
    std::string content = "Text";

    const char* GetName() const override { return "Text"; }
    const char* GetCategory() const override { return "Display"; }
    std::vector<UIPort> Inputs() const override;
    std::vector<UIPort> Outputs() const override;
    void Evaluate(const UIContext& ctx, const std::vector<UIValue>& inputs, std::vector<UIValue>& outputs) const override;
};

class ListNode : public UINode {
public:
    const char* GetName() const override { return "List"; }
    const char* GetCategory() const override { return "Container"; }
    std::vector<UIPort> Inputs() const override;
    std::vector<UIPort> Outputs() const override;
    void Evaluate(const UIContext& ctx, const std::vector<UIValue>& inputs, std::vector<UIValue>& outputs) const override;
};

class SlotGridNode : public UINode {
public:
    int columns = 4;
    int rows = 4;
    float slotSize = 48.0f;

    const char* GetName() const override { return "SlotGrid"; }
    const char* GetCategory() const override { return "Container"; }
    std::vector<UIPort> Inputs() const override;
    std::vector<UIPort> Outputs() const override;
    void Evaluate(const UIContext& ctx, const std::vector<UIValue>& inputs, std::vector<UIValue>& outputs) const override;
};

class InputFieldNode : public UINode {
public:
    std::string placeholder = "";

    const char* GetName() const override { return "InputField"; }
    const char* GetCategory() const override { return "Interactive"; }
    std::vector<UIPort> Inputs() const override;
    std::vector<UIPort> Outputs() const override;
    void Evaluate(const UIContext& ctx, const std::vector<UIValue>& inputs, std::vector<UIValue>& outputs) const override;
};

}
