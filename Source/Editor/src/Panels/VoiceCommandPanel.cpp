#include "VoiceCommandPanel.h"

namespace atlas::editor {

void VoiceCommandPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 400, 200}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 400, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 200, 20}, "Voice Commands", {220, 220, 220, 255});

    // Listening status
    std::string status = m_listening ? "LISTENING" : "IDLE";
    atlas::ui::UIColor statusColor = m_listening
        ? atlas::ui::UIColor{100, 255, 100, 255}
        : atlas::ui::UIColor{160, 160, 160, 255};
    m_drawList.DrawText({4, 28, 200, 16}, status, statusColor);

    // Last phrase
    if (!m_lastPhrase.empty()) {
        m_drawList.DrawText({4, 48, 392, 16}, "Phrase: " + m_lastPhrase, {200, 200, 200, 255});
    }

    // Last intent
    if (!m_lastIntent.empty()) {
        m_drawList.DrawText({4, 66, 392, 16}, "Intent: " + m_lastIntent, {180, 220, 180, 255});
    }
}

void VoiceCommandPanel::ProcessPhrase(const std::string& phrase) {
    m_lastPhrase = phrase;
    m_lastIntent.clear();

    auto* cmd = m_voiceRegistry.Match(phrase);
    if (cmd) {
        m_lastIntent = cmd->intentName;
        interaction::Intent intent;
        intent.name = cmd->intentName;
        intent.source = interaction::IntentSource::Voice;
        m_intentRegistry.Dispatch(intent);
    }
}

}
