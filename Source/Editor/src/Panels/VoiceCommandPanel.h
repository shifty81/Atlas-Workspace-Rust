#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/voice/VoiceCommand.h"
#include "../../engine/interaction/Interaction.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>

namespace atlas::editor {

class VoiceCommandPanel : public EditorPanel {
public:
    VoiceCommandPanel(voice::VoiceCommandRegistry& voiceRegistry,
                      interaction::IntentRegistry& intentRegistry)
        : m_voiceRegistry(voiceRegistry), m_intentRegistry(intentRegistry) {}

    const char* Name() const override { return "Voice Commands"; }
    void Draw() override;

    void SetListening(bool listening) { m_listening = listening; }
    bool IsListening() const { return m_listening; }

    void ProcessPhrase(const std::string& phrase);
    const std::string& LastPhrase() const { return m_lastPhrase; }
    const std::string& LastResolvedIntent() const { return m_lastIntent; }

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    voice::VoiceCommandRegistry& m_voiceRegistry;
    interaction::IntentRegistry& m_intentRegistry;
    bool m_listening = false;
    std::string m_lastPhrase;
    std::string m_lastIntent;
    atlas::ui::UIDrawList m_drawList;
};

}
