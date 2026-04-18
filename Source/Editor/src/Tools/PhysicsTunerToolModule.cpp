#include "PhysicsTunerToolModule.h"

namespace atlas::editor {

std::string PhysicsTunerToolModule::Name() const { return "Physics Tuner"; }
void PhysicsTunerToolModule::OnRegister() {}
void PhysicsTunerToolModule::OnUnregister() {}
void PhysicsTunerToolModule::RegisterPanels() {}
void PhysicsTunerToolModule::RegisterMenus() {}
void PhysicsTunerToolModule::RegisterModes() {}
bool PhysicsTunerToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void PhysicsTunerToolModule::Update(float /*deltaTime*/) {}
void PhysicsTunerToolModule::Render() {}

} // namespace atlas::editor
