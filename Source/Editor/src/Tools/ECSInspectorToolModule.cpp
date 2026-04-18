#include "ECSInspectorToolModule.h"

namespace atlas::editor {

std::string ECSInspectorToolModule::Name() const { return "ECS Inspector"; }
void ECSInspectorToolModule::OnRegister() {}
void ECSInspectorToolModule::OnUnregister() {}
void ECSInspectorToolModule::RegisterPanels() {}
void ECSInspectorToolModule::RegisterMenus() {}
void ECSInspectorToolModule::RegisterModes() {}
bool ECSInspectorToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void ECSInspectorToolModule::Update(float /*deltaTime*/) {}
void ECSInspectorToolModule::Render() {}

} // namespace atlas::editor
