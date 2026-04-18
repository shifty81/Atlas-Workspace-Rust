#include "MeshViewerToolModule.h"

namespace atlas::editor {

std::string MeshViewerToolModule::Name() const { return "Mesh Viewer"; }
void MeshViewerToolModule::OnRegister() {}
void MeshViewerToolModule::OnUnregister() {}
void MeshViewerToolModule::RegisterPanels() {}
void MeshViewerToolModule::RegisterMenus() {}
void MeshViewerToolModule::RegisterModes() {}
bool MeshViewerToolModule::HandleInput(uint32_t /*keyCode*/, bool /*pressed*/) { return false; }
void MeshViewerToolModule::Update(float /*deltaTime*/) {}
void MeshViewerToolModule::Render() {}

} // namespace atlas::editor
