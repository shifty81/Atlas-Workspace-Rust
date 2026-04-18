#include "ConsoleToolModule.h"

namespace atlas::editor {

std::string ConsoleToolModule::Name() const { return "Console"; }
void ConsoleToolModule::OnRegister() {}
void ConsoleToolModule::OnUnregister() {}
void ConsoleToolModule::RegisterPanels() {}
void ConsoleToolModule::RegisterMenus() {}
void ConsoleToolModule::RegisterModes() {}
bool ConsoleToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void ConsoleToolModule::Update(float /*deltaTime*/) {}
void ConsoleToolModule::Render() {}

} // namespace atlas::editor
