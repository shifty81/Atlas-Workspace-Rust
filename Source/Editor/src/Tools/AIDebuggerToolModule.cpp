#include "AIDebuggerToolModule.h"

namespace atlas::editor {

std::string AIDebuggerToolModule::Name() const { return "AI Debugger"; }
void AIDebuggerToolModule::OnRegister() {}
void AIDebuggerToolModule::OnUnregister() {}
void AIDebuggerToolModule::RegisterPanels() {}
void AIDebuggerToolModule::RegisterMenus() {}
void AIDebuggerToolModule::RegisterModes() {}
bool AIDebuggerToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void AIDebuggerToolModule::Update(float /*deltaTime*/) {}
void AIDebuggerToolModule::Render() {}

} // namespace atlas::editor
