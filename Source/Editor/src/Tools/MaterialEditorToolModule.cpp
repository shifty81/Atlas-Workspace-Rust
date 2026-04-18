#include "MaterialEditorToolModule.h"

namespace atlas::editor {

std::string MaterialEditorToolModule::Name() const { return "Material Editor"; }
void MaterialEditorToolModule::OnRegister() {}
void MaterialEditorToolModule::OnUnregister() {}
void MaterialEditorToolModule::RegisterPanels() {}
void MaterialEditorToolModule::RegisterMenus() {}
void MaterialEditorToolModule::RegisterModes() {}
bool MaterialEditorToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void MaterialEditorToolModule::Update(float /*deltaTime*/) {}
void MaterialEditorToolModule::Render() {}

} // namespace atlas::editor
