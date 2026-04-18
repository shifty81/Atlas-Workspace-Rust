#include "PrefabEditorToolModule.h"

namespace atlas::editor {

std::string PrefabEditorToolModule::Name() const { return "Prefab Editor"; }
void PrefabEditorToolModule::OnRegister() {}
void PrefabEditorToolModule::OnUnregister() {}
void PrefabEditorToolModule::RegisterPanels() {}
void PrefabEditorToolModule::RegisterMenus() {}
void PrefabEditorToolModule::RegisterModes() {}
bool PrefabEditorToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void PrefabEditorToolModule::Update(float /*deltaTime*/) {}
void PrefabEditorToolModule::Render() {}

} // namespace atlas::editor
