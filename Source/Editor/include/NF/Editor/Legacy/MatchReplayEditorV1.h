#pragma once
// NF::Editor — Match replay editor v1: competitive match replay bookmarks and analysis
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Mrpv1MatchOutcome  : uint8_t { Unknown, Win, Loss, Draw, Abandoned };
enum class Mrpv1BookmarkType  : uint8_t { Kill, Death, Objective, Highlight, Error };

inline const char* mrpv1MatchOutcomeName(Mrpv1MatchOutcome o) {
    switch (o) {
        case Mrpv1MatchOutcome::Unknown:   return "Unknown";
        case Mrpv1MatchOutcome::Win:       return "Win";
        case Mrpv1MatchOutcome::Loss:      return "Loss";
        case Mrpv1MatchOutcome::Draw:      return "Draw";
        case Mrpv1MatchOutcome::Abandoned: return "Abandoned";
    }
    return "Unknown";
}

inline const char* mrpv1BookmarkTypeName(Mrpv1BookmarkType t) {
    switch (t) {
        case Mrpv1BookmarkType::Kill:      return "Kill";
        case Mrpv1BookmarkType::Death:     return "Death";
        case Mrpv1BookmarkType::Objective: return "Objective";
        case Mrpv1BookmarkType::Highlight: return "Highlight";
        case Mrpv1BookmarkType::Error:     return "Error";
    }
    return "Unknown";
}

struct Mrpv1Bookmark {
    uint64_t          id        = 0;
    uint64_t          matchId   = 0;
    Mrpv1BookmarkType type      = Mrpv1BookmarkType::Highlight;
    float             timestampMs = 0.f;
    std::string       note;

    [[nodiscard]] bool isValid() const { return id != 0 && matchId != 0; }
};

struct Mrpv1Match {
    uint64_t           id      = 0;
    std::string        name;
    Mrpv1MatchOutcome  outcome = Mrpv1MatchOutcome::Unknown;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isWin()       const { return outcome == Mrpv1MatchOutcome::Win; }
    [[nodiscard]] bool isLoss()      const { return outcome == Mrpv1MatchOutcome::Loss; }
    [[nodiscard]] bool isAbandoned() const { return outcome == Mrpv1MatchOutcome::Abandoned; }
};

using Mrpv1ChangeCallback = std::function<void(uint64_t)>;

class MatchReplayEditorV1 {
public:
    static constexpr size_t MAX_MATCHES   = 1024;
    static constexpr size_t MAX_BOOKMARKS = 32768;

    bool addMatch(const Mrpv1Match& match) {
        if (!match.isValid()) return false;
        for (const auto& m : m_matches) if (m.id == match.id) return false;
        if (m_matches.size() >= MAX_MATCHES) return false;
        m_matches.push_back(match);
        return true;
    }

    bool removeMatch(uint64_t id) {
        for (auto it = m_matches.begin(); it != m_matches.end(); ++it) {
            if (it->id == id) { m_matches.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Mrpv1Match* findMatch(uint64_t id) {
        for (auto& m : m_matches) if (m.id == id) return &m;
        return nullptr;
    }

    bool addBookmark(const Mrpv1Bookmark& bookmark) {
        if (!bookmark.isValid()) return false;
        for (const auto& b : m_bookmarks) if (b.id == bookmark.id) return false;
        if (m_bookmarks.size() >= MAX_BOOKMARKS) return false;
        m_bookmarks.push_back(bookmark);
        if (m_onChange) m_onChange(bookmark.matchId);
        return true;
    }

    bool removeBookmark(uint64_t id) {
        for (auto it = m_bookmarks.begin(); it != m_bookmarks.end(); ++it) {
            if (it->id == id) { m_bookmarks.erase(it); return true; }
        }
        return false;
    }

    bool setOutcome(uint64_t matchId, Mrpv1MatchOutcome outcome) {
        auto* m = findMatch(matchId);
        if (!m) return false;
        m->outcome = outcome;
        if (m_onChange) m_onChange(matchId);
        return true;
    }

    [[nodiscard]] size_t matchCount()    const { return m_matches.size(); }
    [[nodiscard]] size_t bookmarkCount() const { return m_bookmarks.size(); }

    [[nodiscard]] size_t winCount() const {
        size_t c = 0; for (const auto& m : m_matches) if (m.isWin()) ++c; return c;
    }
    [[nodiscard]] size_t lossCount() const {
        size_t c = 0; for (const auto& m : m_matches) if (m.isLoss()) ++c; return c;
    }
    [[nodiscard]] size_t countByBookmarkType(Mrpv1BookmarkType type) const {
        size_t c = 0; for (const auto& b : m_bookmarks) if (b.type == type) ++c; return c;
    }

    void setOnChange(Mrpv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Mrpv1Match>    m_matches;
    std::vector<Mrpv1Bookmark> m_bookmarks;
    Mrpv1ChangeCallback        m_onChange;
};

} // namespace NF
