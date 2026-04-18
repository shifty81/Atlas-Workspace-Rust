#include <cassert>
#include <iostream>
#include "../engine/ai/AISignalRegistry.h"

using namespace atlas::ai;

void test_ai_signal_register() {
    AISignalRegistry& reg = AISignalRegistry::Get();
    reg.Clear();

    reg.Register("game", "threat_level", 0.5f);
    assert(reg.Has("game.threat_level"));
    assert(reg.SignalCount() == 1);
    std::cout << "[PASS] test_ai_signal_register" << std::endl;
}

void test_ai_signal_read_write() {
    AISignalRegistry& reg = AISignalRegistry::Get();
    reg.Clear();

    reg.Register("economy", "demand", 1.0f);
    assert(reg.Read("economy.demand") >= 0.99f);

    reg.Write("economy.demand", 3.5f, 100);
    assert(reg.Read("economy.demand") >= 3.49f && reg.Read("economy.demand") <= 3.51f);

    const AISignal* sig = reg.GetSignal("economy.demand");
    assert(sig != nullptr);
    assert(sig->lastUpdatedTick == 100);

    std::cout << "[PASS] test_ai_signal_read_write" << std::endl;
}

void test_ai_signal_missing() {
    AISignalRegistry& reg = AISignalRegistry::Get();
    reg.Clear();

    assert(!reg.Has("nonexistent.signal"));
    assert(reg.Read("nonexistent.signal") == 0.0f);
    assert(reg.GetSignal("nonexistent.signal") == nullptr);

    std::cout << "[PASS] test_ai_signal_missing" << std::endl;
}

void test_ai_signal_namespace() {
    AISignalRegistry& reg = AISignalRegistry::Get();
    reg.Clear();

    reg.Register("faction", "morale", 0.8f);
    reg.Register("faction", "aggression", 0.3f);
    reg.Register("world", "tension", 0.6f);

    assert(reg.SignalCount() == 3);
    assert(reg.Has("faction.morale"));
    assert(reg.Has("faction.aggression"));
    assert(reg.Has("world.tension"));
    assert(!reg.Has("faction.tension"));

    std::cout << "[PASS] test_ai_signal_namespace" << std::endl;
}

void test_ai_signal_clear() {
    AISignalRegistry& reg = AISignalRegistry::Get();
    reg.Clear();

    reg.Register("test", "val", 1.0f);
    assert(reg.SignalCount() == 1);
    reg.Clear();
    assert(reg.SignalCount() == 0);
    assert(!reg.Has("test.val"));

    std::cout << "[PASS] test_ai_signal_clear" << std::endl;
}
