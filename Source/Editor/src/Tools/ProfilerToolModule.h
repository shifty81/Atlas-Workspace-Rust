#pragma once
#include "IEditorToolModule.h"
#include <string>

namespace atlas::editor {

class ProfilerPanel;

class ProfilerToolModule : public IEditorToolModule {
public:
    std::string Name() const override;
    void OnRegister() override;
    void OnUnregister() override;
    void RegisterPanels() override;
    void RegisterMenus() override;
    void RegisterModes() override;
    bool HandleInput(uint32_t keyCode, bool pressed) override;
    void Update(float deltaTime) override;
    void Render() override;
};

} // namespace atlas::editor
