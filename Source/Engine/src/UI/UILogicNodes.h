#pragma once
#include "UILogicGraph.h"

namespace atlas::ui {

class ConditionNode : public UILogicNode {
public:
    const char* GetName() const override { return "Condition"; }
    const char* GetCategory() const override { return "Logic"; }
    std::vector<UILogicPort> Inputs() const override;
    std::vector<UILogicPort> Outputs() const override;
    void Evaluate(const UILogicContext& ctx, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const override;
};

class ActionNode : public UILogicNode {
public:
    std::string actionName = "action";

    const char* GetName() const override { return "Action"; }
    const char* GetCategory() const override { return "Logic"; }
    std::vector<UILogicPort> Inputs() const override;
    std::vector<UILogicPort> Outputs() const override;
    void Evaluate(const UILogicContext& ctx, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const override;
};

class DataBindNode : public UILogicNode {
public:
    std::string boundKey;
    float boundFloat = 0.0f;
    int boundInt = 0;
    std::string boundString;
    bool boundBool = false;

    const char* GetName() const override { return "DataBind"; }
    const char* GetCategory() const override { return "Data"; }
    std::vector<UILogicPort> Inputs() const override;
    std::vector<UILogicPort> Outputs() const override;
    void Evaluate(const UILogicContext& ctx, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const override;
};

class TransitionNode : public UILogicNode {
public:
    float startValue = 0.0f;
    float endValue = 1.0f;
    uint32_t durationTicks = 10;

    const char* GetName() const override { return "Transition"; }
    const char* GetCategory() const override { return "Animation"; }
    std::vector<UILogicPort> Inputs() const override;
    std::vector<UILogicPort> Outputs() const override;
    void Evaluate(const UILogicContext& ctx, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const override;
};

class TimerNode : public UILogicNode {
public:
    uint32_t delayTicks = 10;

    const char* GetName() const override { return "Timer"; }
    const char* GetCategory() const override { return "Timing"; }
    std::vector<UILogicPort> Inputs() const override;
    std::vector<UILogicPort> Outputs() const override;
    void Evaluate(const UILogicContext& ctx, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const override;
};

class GateNode : public UILogicNode {
public:
    const char* GetName() const override { return "Gate"; }
    const char* GetCategory() const override { return "Logic"; }
    std::vector<UILogicPort> Inputs() const override;
    std::vector<UILogicPort> Outputs() const override;
    void Evaluate(const UILogicContext& ctx, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const override;
};

}
