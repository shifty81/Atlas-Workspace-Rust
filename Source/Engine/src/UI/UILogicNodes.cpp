#include "UILogicNodes.h"
#include <algorithm>

namespace atlas::ui {

// --- ConditionNode ---

std::vector<UILogicPort> ConditionNode::Inputs() const {
    return {{"Value", UILogicPinType::Bool}};
}

std::vector<UILogicPort> ConditionNode::Outputs() const {
    return {{"Result", UILogicPinType::Bool}};
}

void ConditionNode::Evaluate(const UILogicContext& /*ctx*/, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const {
    outputs.resize(1);
    outputs[0].type = UILogicPinType::Bool;
    bool val = false;
    if (!inputs.empty() && !inputs[0].data.empty()) {
        val = inputs[0].data[0] != 0.0f;
    }
    outputs[0].data = {val ? 1.0f : 0.0f};
}

// --- ActionNode ---

std::vector<UILogicPort> ActionNode::Inputs() const {
    return {{"Trigger", UILogicPinType::Signal}};
}

std::vector<UILogicPort> ActionNode::Outputs() const {
    return {{"Done", UILogicPinType::Signal}};
}

void ActionNode::Evaluate(const UILogicContext& /*ctx*/, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const {
    outputs.resize(1);
    outputs[0].type = UILogicPinType::Signal;
    float active = 0.0f;
    if (!inputs.empty() && !inputs[0].data.empty() && inputs[0].data[0] != 0.0f) {
        active = 1.0f;
    }
    outputs[0].data = {active};
    outputs[0].text = actionName;
}

// --- DataBindNode ---

std::vector<UILogicPort> DataBindNode::Inputs() const {
    return {{"Key", UILogicPinType::String}};
}

std::vector<UILogicPort> DataBindNode::Outputs() const {
    return {
        {"FloatValue", UILogicPinType::Float},
        {"IntValue", UILogicPinType::Int},
        {"StringValue", UILogicPinType::String},
        {"BoolValue", UILogicPinType::Bool}
    };
}

void DataBindNode::Evaluate(const UILogicContext& /*ctx*/, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const {
    std::string key = boundKey;
    if (!inputs.empty() && !inputs[0].text.empty()) {
        key = inputs[0].text;
    }

    outputs.resize(4);

    outputs[0].type = UILogicPinType::Float;
    outputs[0].data = {boundFloat};

    outputs[1].type = UILogicPinType::Int;
    outputs[1].data = {static_cast<float>(boundInt)};

    outputs[2].type = UILogicPinType::String;
    outputs[2].text = boundString;

    outputs[3].type = UILogicPinType::Bool;
    outputs[3].data = {boundBool ? 1.0f : 0.0f};
}

// --- TransitionNode ---

std::vector<UILogicPort> TransitionNode::Inputs() const {
    return {{"Trigger", UILogicPinType::Signal}};
}

std::vector<UILogicPort> TransitionNode::Outputs() const {
    return {{"Value", UILogicPinType::Float}};
}

void TransitionNode::Evaluate(const UILogicContext& ctx, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const {
    outputs.resize(1);
    outputs[0].type = UILogicPinType::Float;

    bool triggered = false;
    if (!inputs.empty() && !inputs[0].data.empty() && inputs[0].data[0] != 0.0f) {
        triggered = true;
    }

    if (!triggered || durationTicks == 0) {
        outputs[0].data = {startValue};
        return;
    }

    float t = static_cast<float>(ctx.tick % (durationTicks + 1)) / static_cast<float>(durationTicks);
    t = std::min(t, 1.0f);
    float value = startValue + (endValue - startValue) * t;
    outputs[0].data = {value};
}

// --- TimerNode ---

std::vector<UILogicPort> TimerNode::Inputs() const {
    return {{"Start", UILogicPinType::Signal}};
}

std::vector<UILogicPort> TimerNode::Outputs() const {
    return {{"Fire", UILogicPinType::Signal}};
}

void TimerNode::Evaluate(const UILogicContext& ctx, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const {
    outputs.resize(1);
    outputs[0].type = UILogicPinType::Signal;

    // If no start signal is connected (empty data), treat as always started
    bool started = true;
    if (!inputs.empty() && !inputs[0].data.empty()) {
        started = inputs[0].data[0] != 0.0f;
    }

    float fire = 0.0f;
    if (started && ctx.tick >= delayTicks) {
        fire = 1.0f;
    }
    outputs[0].data = {fire};
}

// --- GateNode ---

std::vector<UILogicPort> GateNode::Inputs() const {
    return {
        {"Signal", UILogicPinType::Signal},
        {"Condition", UILogicPinType::Bool}
    };
}

std::vector<UILogicPort> GateNode::Outputs() const {
    return {{"Out", UILogicPinType::Signal}};
}

void GateNode::Evaluate(const UILogicContext& /*ctx*/, const std::vector<UILogicValue>& inputs, std::vector<UILogicValue>& outputs) const {
    outputs.resize(1);
    outputs[0].type = UILogicPinType::Signal;

    bool hasSignal = false;
    if (!inputs.empty() && !inputs[0].data.empty() && inputs[0].data[0] != 0.0f) {
        hasSignal = true;
    }

    bool conditionMet = false;
    if (inputs.size() > 1 && !inputs[1].data.empty() && inputs[1].data[0] != 0.0f) {
        conditionMet = true;
    }

    float out = (hasSignal && conditionMet) ? 1.0f : 0.0f;
    outputs[0].data = {out};
}

}
