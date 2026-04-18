#include "../engine/interaction/InteractionSystem.h"
#include "../engine/interaction/RuleIntentResolver.h"
#include "../engine/interaction/VoiceAdapter.h"
#include <iostream>
#include <cassert>
#include <regex>

using namespace atlas::interaction;

// Simple router for testing
class TestRouter : public InteractionRouter {
public:
    Utterance Route(const Intent& intent, const Utterance&) override {
        return { "Handled: " + intent.name };
    }
};

void test_utterance_creation() {
    Utterance u;
    u.text = "Hello world";
    u.speaker = 1;
    u.listener = 2;
    u.ctx = { InteractionMode::Conversation, false };

    assert(u.text == "Hello world");
    assert(u.speaker == 1);
    assert(u.listener == 2);
    assert(u.ctx.mode == InteractionMode::Conversation);
    assert(!u.ctx.authoritative);
    std::cout << "[PASS] test_utterance_creation" << std::endl;
}

void test_intent_creation() {
    Intent i;
    i.name = "RequestResource";
    i.confidence = 0.95f;
    i.slots["resource"] = std::string("minerals");
    i.slots["amount"] = int64_t(500);

    assert(i.name == "RequestResource");
    assert(i.confidence > 0.9f);
    assert(std::get<std::string>(i.slots["resource"]) == "minerals");
    assert(std::get<int64_t>(i.slots["amount"]) == 500);
    std::cout << "[PASS] test_intent_creation" << std::endl;
}

void test_rule_intent_resolver() {
    RuleIntentResolver resolver;
    resolver.AddRule({ "ExplainGraphNode", std::regex("what does this node do") });
    resolver.AddRule({ "RequestResource", std::regex("send.*resources") });
    resolver.AddRule({ "Threaten", std::regex("or else|threat") });

    Utterance u1{ "what does this node do?", 0, 0, { InteractionMode::Command, true } };
    auto intent1 = resolver.Resolve(u1);
    assert(intent1.has_value());
    assert(intent1->name == "ExplainGraphNode");

    Utterance u2{ "please send me resources", 0, 0, { InteractionMode::Conversation, false } };
    auto intent2 = resolver.Resolve(u2);
    assert(intent2.has_value());
    assert(intent2->name == "RequestResource");

    Utterance u3{ "this is a threat", 0, 0, { InteractionMode::Conversation, false } };
    auto intent3 = resolver.Resolve(u3);
    assert(intent3.has_value());
    assert(intent3->name == "Threaten");

    Utterance u4{ "random gibberish", 0, 0, { InteractionMode::Command, true } };
    auto intent4 = resolver.Resolve(u4);
    assert(!intent4.has_value());

    std::cout << "[PASS] test_rule_intent_resolver" << std::endl;
}

void test_interaction_system_uninitialized() {
    InteractionSystem sys;
    Utterance u{ "hello", 0, 0, { InteractionMode::Command, true } };
    auto result = sys.Handle(u);
    assert(result.text == "Interaction system not initialized.");
    std::cout << "[PASS] test_interaction_system_uninitialized" << std::endl;
}

void test_interaction_system_no_match() {
    InteractionSystem sys;
    auto resolver = std::make_unique<RuleIntentResolver>();
    resolver->AddRule({ "Greet", std::regex("hello") });
    sys.SetResolver(std::move(resolver));
    sys.SetRouter(std::make_unique<TestRouter>());

    Utterance u{ "unknown input xyz", 0, 0, { InteractionMode::Command, true } };
    auto result = sys.Handle(u);
    assert(result.text == "I didn't understand that.");
    std::cout << "[PASS] test_interaction_system_no_match" << std::endl;
}

void test_interaction_system_full_pipeline() {
    InteractionSystem sys;
    auto resolver = std::make_unique<RuleIntentResolver>();
    resolver->AddRule({ "Greet", std::regex("hello") });
    sys.SetResolver(std::move(resolver));
    sys.SetRouter(std::make_unique<TestRouter>());

    Utterance u{ "hello world", 0, 0, { InteractionMode::Command, true } };
    auto result = sys.Handle(u);
    assert(result.text == "Handled: Greet");
    std::cout << "[PASS] test_interaction_system_full_pipeline" << std::endl;
}
