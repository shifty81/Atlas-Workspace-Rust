#include "SceneGraphToolModule.h"

namespace atlas::editor {

std::string SceneGraphToolModule::Name() const { return "Scene Graph"; }
void SceneGraphToolModule::OnRegister() {}
void SceneGraphToolModule::OnUnregister() {}
void SceneGraphToolModule::RegisterPanels() {}
void SceneGraphToolModule::RegisterMenus() {}
void SceneGraphToolModule::RegisterModes() {}
bool SceneGraphToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void SceneGraphToolModule::Update(float /*deltaTime*/) {}
void SceneGraphToolModule::Render() {}

} // namespace atlas::editor
