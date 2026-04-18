#include "ProfilerToolModule.h"

namespace atlas::editor {

std::string ProfilerToolModule::Name() const { return "Profiler"; }
void ProfilerToolModule::OnRegister() {}
void ProfilerToolModule::OnUnregister() {}
void ProfilerToolModule::RegisterPanels() {}
void ProfilerToolModule::RegisterMenus() {}
void ProfilerToolModule::RegisterModes() {}
bool ProfilerToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void ProfilerToolModule::Update(float /*deltaTime*/) {}
void ProfilerToolModule::Render() {}

} // namespace atlas::editor
