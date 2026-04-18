#pragma once
#include "UICommandBus.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::ui {

enum class GUIRecordState : uint8_t {
    Idle,
    Recording,
    Playing
};

struct GUIInputEvent {
    UICommandType type = UICommandType::Custom;
    uint32_t targetWidgetId = 0;
    uint64_t tick = 0;
    float valueFloat = 0.0f;
    std::string valueString;
};

struct GUIInputLog {
    uint32_t version = 1;
    uint64_t startTick = 0;
    uint64_t endTick = 0;
    std::vector<GUIInputEvent> events;
};

class GUIInputRecorder {
public:
    void StartRecording(uint64_t startTick);
    void StopRecording();
    void RecordEvent(const UICommand& cmd);

    void StartPlayback(UICommandBus* bus, uint64_t startTick);
    void StopPlayback();
    void AdvancePlayback(uint64_t currentTick);

    bool SaveLog(const std::string& path) const;
    bool LoadLog(const std::string& path);

    GUIRecordState State() const;
    const GUIInputLog& Log() const;
    size_t EventCount() const;
    size_t PlaybackPosition() const;

private:
    GUIRecordState m_state = GUIRecordState::Idle;
    GUIInputLog m_log;
    UICommandBus* m_playbackBus = nullptr;
    uint64_t m_playbackStartTick = 0;
    size_t m_playbackPos = 0;
};

} // namespace atlas::ui
