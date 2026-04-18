#include "../engine/interaction/Interaction.h"
#include <iostream>
#include <cassert>

void test_intent_registry_dispatch() {
    atlas::interaction::IntentRegistry registry;
    bool called = false;
    registry.Register("test_intent", [&](const atlas::interaction::Intent&) {
        called = true;
    });

    atlas::interaction::Intent intent;
    intent.name = "test_intent";
    assert(registry.Dispatch(intent));
    assert(called);
    std::cout << "[PASS] test_intent_registry_dispatch" << std::endl;
}

void test_intent_registry_missing() {
    atlas::interaction::IntentRegistry registry;
    atlas::interaction::Intent intent;
    intent.name = "missing";
    assert(!registry.Dispatch(intent));
    std::cout << "[PASS] test_intent_registry_missing" << std::endl;
}

void test_intent_registry_has_handler() {
    atlas::interaction::IntentRegistry registry;
    registry.Register("exists", [](const atlas::interaction::Intent&) {});
    assert(registry.HasHandler("exists"));
    assert(!registry.HasHandler("nope"));
    assert(registry.HandlerCount() == 1);
    std::cout << "[PASS] test_intent_registry_has_handler" << std::endl;
}

void test_pattern_resolver() {
    atlas::interaction::PatternIntentResolver resolver;
    resolver.AddPattern({"select_units", {"select all fighters", "select ships"}});
    resolver.AddPattern({"pause_game", {"pause", "stop simulation"}});

    atlas::interaction::Utterance u;
    u.text = "select all fighters";
    auto result = resolver.Resolve(u);
    assert(result.has_value());
    assert(result->name == "select_units");
    std::cout << "[PASS] test_pattern_resolver" << std::endl;
}

void test_pattern_resolver_no_match() {
    atlas::interaction::PatternIntentResolver resolver;
    resolver.AddPattern({"attack", {"attack target"}});

    atlas::interaction::Utterance u;
    u.text = "retreat now";
    auto result = resolver.Resolve(u);
    assert(!result.has_value());
    std::cout << "[PASS] test_pattern_resolver_no_match" << std::endl;
}

void test_pattern_resolver_case_insensitive() {
    atlas::interaction::PatternIntentResolver resolver;
    resolver.AddPattern({"zoom", {"zoom out"}});

    atlas::interaction::Utterance u;
    u.text = "ZOOM OUT";
    auto result = resolver.Resolve(u);
    assert(result.has_value());
    assert(result->name == "zoom");
    std::cout << "[PASS] test_pattern_resolver_case_insensitive" << std::endl;
}
