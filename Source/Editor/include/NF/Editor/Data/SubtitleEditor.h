#pragma once
// NF::Editor — subtitle editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class SubtitleDisplayMode : uint8_t {
    Standard, Karaoke, WordByWord, Narrative, SignLanguage
};

inline const char* subtitleDisplayModeName(SubtitleDisplayMode m) {
    switch (m) {
        case SubtitleDisplayMode::Standard:     return "Standard";
        case SubtitleDisplayMode::Karaoke:      return "Karaoke";
        case SubtitleDisplayMode::WordByWord:   return "WordByWord";
        case SubtitleDisplayMode::Narrative:    return "Narrative";
        case SubtitleDisplayMode::SignLanguage: return "SignLanguage";
    }
    return "Unknown";
}

enum class SubtitlePosition : uint8_t {
    Bottom, Top, Left, Right, Center, Custom
};

inline const char* subtitlePositionName(SubtitlePosition p) {
    switch (p) {
        case SubtitlePosition::Bottom: return "Bottom";
        case SubtitlePosition::Top:    return "Top";
        case SubtitlePosition::Left:   return "Left";
        case SubtitlePosition::Right:  return "Right";
        case SubtitlePosition::Center: return "Center";
        case SubtitlePosition::Custom: return "Custom";
    }
    return "Unknown";
}

enum class SubtitleSpeakerTag : uint8_t {
    None, Name, Color, Icon, Bracket
};

inline const char* subtitleSpeakerTagName(SubtitleSpeakerTag t) {
    switch (t) {
        case SubtitleSpeakerTag::None:    return "None";
        case SubtitleSpeakerTag::Name:    return "Name";
        case SubtitleSpeakerTag::Color:   return "Color";
        case SubtitleSpeakerTag::Icon:    return "Icon";
        case SubtitleSpeakerTag::Bracket: return "Bracket";
    }
    return "Unknown";
}

class SubtitleTrack {
public:
    explicit SubtitleTrack(uint32_t id, const std::string& name, const std::string& language)
        : m_id(id), m_name(name), m_language(language) {}

    void setDisplayMode(SubtitleDisplayMode v) { m_displayMode = v; }
    void setPosition(SubtitlePosition v)       { m_position    = v; }
    void setSpeakerTag(SubtitleSpeakerTag v)   { m_speakerTag  = v; }
    void setIsEnabled(bool v)                  { m_isEnabled   = v; }
    void setFontSize(float v)                  { m_fontSize    = v; }

    [[nodiscard]] uint32_t             id()          const { return m_id;          }
    [[nodiscard]] const std::string&   name()        const { return m_name;        }
    [[nodiscard]] const std::string&   language()    const { return m_language;    }
    [[nodiscard]] SubtitleDisplayMode  displayMode() const { return m_displayMode; }
    [[nodiscard]] SubtitlePosition     position()    const { return m_position;    }
    [[nodiscard]] SubtitleSpeakerTag   speakerTag()  const { return m_speakerTag;  }
    [[nodiscard]] bool                 isEnabled()   const { return m_isEnabled;   }
    [[nodiscard]] float                fontSize()    const { return m_fontSize;    }

private:
    uint32_t            m_id;
    std::string         m_name;
    std::string         m_language;
    SubtitleDisplayMode m_displayMode = SubtitleDisplayMode::Standard;
    SubtitlePosition    m_position    = SubtitlePosition::Bottom;
    SubtitleSpeakerTag  m_speakerTag  = SubtitleSpeakerTag::Name;
    bool                m_isEnabled   = true;
    float               m_fontSize    = 14.0f;
};

class SubtitleEditor {
public:
    void setShowPreview(bool v)           { m_showPreview  = v; }
    void setDefaultPosition(SubtitlePosition v) { m_defaultPosition = v; }
    void setDefaultFontSize(float v)      { m_defaultFontSize = v; }

    bool addTrack(const SubtitleTrack& t) {
        for (auto& x : m_tracks) if (x.id() == t.id()) return false;
        m_tracks.push_back(t); return true;
    }
    bool removeTrack(uint32_t id) {
        auto it = std::find_if(m_tracks.begin(), m_tracks.end(),
            [&](const SubtitleTrack& t){ return t.id() == id; });
        if (it == m_tracks.end()) return false;
        m_tracks.erase(it); return true;
    }
    [[nodiscard]] SubtitleTrack* findTrack(uint32_t id) {
        for (auto& t : m_tracks) if (t.id() == id) return &t;
        return nullptr;
    }

    [[nodiscard]] bool             isShowPreview()    const { return m_showPreview;     }
    [[nodiscard]] SubtitlePosition defaultPosition()  const { return m_defaultPosition; }
    [[nodiscard]] float            defaultFontSize()  const { return m_defaultFontSize; }
    [[nodiscard]] size_t           trackCount()       const { return m_tracks.size();   }

    [[nodiscard]] size_t countByDisplayMode(SubtitleDisplayMode m) const {
        size_t n = 0; for (auto& t : m_tracks) if (t.displayMode() == m) ++n; return n;
    }
    [[nodiscard]] size_t countByPosition(SubtitlePosition p) const {
        size_t n = 0; for (auto& t : m_tracks) if (t.position() == p) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& t : m_tracks) if (t.isEnabled()) ++n; return n;
    }

private:
    std::vector<SubtitleTrack> m_tracks;
    bool             m_showPreview     = true;
    SubtitlePosition m_defaultPosition = SubtitlePosition::Bottom;
    float            m_defaultFontSize = 14.0f;
};

} // namespace NF
