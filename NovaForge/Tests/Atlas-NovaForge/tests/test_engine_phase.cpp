#include "../engine/core/EnginePhase.h"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace atlas;

void test_engine_phase_to_string() {
    assert(std::strcmp(EnginePhaseToString(EnginePhase::Boot), "Boot") == 0);
    assert(std::strcmp(EnginePhaseToString(EnginePhase::Launcher), "Launcher") == 0);
    assert(std::strcmp(EnginePhaseToString(EnginePhase::ProjectLoad), "ProjectLoad") == 0);
    assert(std::strcmp(EnginePhaseToString(EnginePhase::Editor), "Editor") == 0);
    assert(std::strcmp(EnginePhaseToString(EnginePhase::Client), "Client") == 0);
    assert(std::strcmp(EnginePhaseToString(EnginePhase::Server), "Server") == 0);
    assert(std::strcmp(EnginePhaseToString(EnginePhase::Shutdown), "Shutdown") == 0);
    std::cout << "[PASS] test_engine_phase_to_string" << std::endl;
}

void test_engine_phase_values() {
    // Phases should have distinct underlying values
    assert(static_cast<uint8_t>(EnginePhase::Boot) != static_cast<uint8_t>(EnginePhase::Launcher));
    assert(static_cast<uint8_t>(EnginePhase::Editor) != static_cast<uint8_t>(EnginePhase::Client));
    std::cout << "[PASS] test_engine_phase_values" << std::endl;
}
