#include "../engine/ai/FactionRouter.h"
#include <iostream>
#include <cassert>

using namespace atlas::ai;
using namespace atlas::interaction;

void test_faction_request_neutral() {
    FactionRouter router;
    // Default trust is 0.5, hostility is 0.0
    Intent intent{ "RequestResource", 1.0f };
    Utterance source{ "give me resources", 1, 2, { InteractionMode::Conversation, false } };

    auto result = router.Route(intent, source);
    assert(result.text == "What do we receive in return?");
    std::cout << "[PASS] test_faction_request_neutral" << std::endl;
}

void test_faction_request_trusted() {
    FactionRouter router;
    router.GetDiplomacy().trust = 0.9f;
    Intent intent{ "RequestResource", 1.0f };
    Utterance source{ "give me resources", 1, 2, { InteractionMode::Conversation, false } };

    auto result = router.Route(intent, source);
    assert(result.text == "We will send the resources.");
    std::cout << "[PASS] test_faction_request_trusted" << std::endl;
}

void test_faction_request_hostile() {
    FactionRouter router;
    router.GetDiplomacy().hostility = 0.8f;
    Intent intent{ "RequestResource", 1.0f };
    Utterance source{ "give me resources", 1, 2, { InteractionMode::Conversation, false } };

    auto result = router.Route(intent, source);
    assert(result.text == "Request denied. Do not ask again.");
    std::cout << "[PASS] test_faction_request_hostile" << std::endl;
}

void test_faction_threaten_increases_hostility() {
    FactionRouter router;
    float initialHostility = router.GetDiplomacy().hostility;
    Intent intent{ "Threaten", 1.0f };
    Utterance source{ "or else", 1, 2, { InteractionMode::Conversation, false } };

    auto result = router.Route(intent, source);
    assert(result.text == "This will not be forgotten.");
    assert(router.GetDiplomacy().hostility > initialHostility);
    std::cout << "[PASS] test_faction_threaten_increases_hostility" << std::endl;
}

void test_faction_unknown_intent() {
    FactionRouter router;
    Intent intent{ "UnknownAction", 1.0f };
    Utterance source{ "???", 1, 2, { InteractionMode::Conversation, false } };

    auto result = router.Route(intent, source);
    assert(result.text == "We have no response.");
    std::cout << "[PASS] test_faction_unknown_intent" << std::endl;
}

void test_faction_personality_affects_hostility() {
    FactionRouter router;
    router.GetPersonality().aggression = 1.0f;
    router.GetPersonality().paranoia = 1.0f;

    Intent intent{ "Threaten", 1.0f };
    Utterance source{ "threat", 1, 2, { InteractionMode::Conversation, false } };

    router.Route(intent, source);
    // aggression * 0.3 + paranoia * 0.2 = 0.5
    assert(router.GetDiplomacy().hostility > 0.4f);
    std::cout << "[PASS] test_faction_personality_affects_hostility" << std::endl;
}
