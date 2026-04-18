#include "WorldGraphToolModule.h"

namespace atlas::editor {

std::string WorldGraphToolModule::Name() const { return "World Graph"; }
void WorldGraphToolModule::OnRegister() {}
void WorldGraphToolModule::OnUnregister() {}
void WorldGraphToolModule::RegisterPanels() {}
void WorldGraphToolModule::RegisterMenus() {}
void WorldGraphToolModule::RegisterModes() {}
bool WorldGraphToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void WorldGraphToolModule::Update(float /*deltaTime*/) {}
void WorldGraphToolModule::Render() {}

} // namespace atlas::editor
