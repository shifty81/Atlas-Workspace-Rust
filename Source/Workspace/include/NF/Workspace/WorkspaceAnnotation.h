#pragma once
// NF::Workspace — Phase 29: Workspace Annotation System
//
// Workspace-level annotation anchoring and lifecycle:
//   AnnotationKind   — Note / Warning / Todo / Bookmark / Review
//   AnnotationAnchor — where on the workspace the annotation is pinned
//   Annotation       — id + kind + author + body + anchor + resolved; isValid()
//   AnnotationManager— registry with resolve/reopen; filter by target, author, kind; observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// AnnotationKind — semantic category
// ═════════════════════════════════════════════════════════════════

enum class AnnotationKind : uint8_t {
    Note,
    Warning,
    Todo,
    Bookmark,
    Review,
};

inline const char* annotationKindName(AnnotationKind k) {
    switch (k) {
        case AnnotationKind::Note:     return "Note";
        case AnnotationKind::Warning:  return "Warning";
        case AnnotationKind::Todo:     return "Todo";
        case AnnotationKind::Bookmark: return "Bookmark";
        case AnnotationKind::Review:   return "Review";
        default:                       return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// AnnotationAnchor — location in the workspace
// ═════════════════════════════════════════════════════════════════

struct AnnotationAnchor {
    std::string targetId;   // panel id, tool id, asset id, etc.
    std::string contextKey; // optional sub-location (e.g. node id, line number)
    float       x = 0.0f;  // optional 2-D position within the target
    float       y = 0.0f;

    bool isValid() const { return !targetId.empty(); }
};

// ═════════════════════════════════════════════════════════════════
// Annotation — a single annotated comment
// ═════════════════════════════════════════════════════════════════

struct Annotation {
    std::string      id;
    AnnotationKind   kind     = AnnotationKind::Note;
    std::string      author;
    std::string      body;
    AnnotationAnchor anchor;
    bool             resolved = false;
    uint64_t         timestamp = 0; // monotonic counter

    bool isValid() const {
        return !id.empty() && !body.empty() && anchor.isValid();
    }

    bool operator==(const Annotation& o) const { return id == o.id; }
    bool operator!=(const Annotation& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// AnnotationManager — registry with resolve/reopen and observers
// ═════════════════════════════════════════════════════════════════

class AnnotationManager {
public:
    using Observer = std::function<void(const Annotation&, bool added)>;

    static constexpr int MAX_ANNOTATIONS = 1024;
    static constexpr int MAX_OBSERVERS   = 16;

    // Registry ─────────────────────────────────────────────────

    bool add(const Annotation& annotation) {
        if (!annotation.isValid()) return false;
        if (findById(annotation.id)) return false;
        if ((int)m_annotations.size() >= MAX_ANNOTATIONS) return false;
        Annotation a = annotation;
        a.timestamp = ++m_clock;
        m_annotations.push_back(a);
        notify(m_annotations.back(), true);
        return true;
    }

    bool remove(const std::string& id) {
        auto it = findIt(id);
        if (it == m_annotations.end()) return false;
        Annotation copy = *it;
        m_annotations.erase(it);
        notify(copy, false);
        return true;
    }

    bool update(const Annotation& annotation) {
        auto it = findIt(annotation.id);
        if (it == m_annotations.end()) return false;
        *it = annotation;
        notify(*it, true);
        return true;
    }

    bool resolve(const std::string& id) {
        Annotation* a = findById(id);
        if (!a || a->resolved) return false;
        a->resolved = true;
        notify(*a, true);
        return true;
    }

    bool reopen(const std::string& id) {
        Annotation* a = findById(id);
        if (!a || !a->resolved) return false;
        a->resolved = false;
        notify(*a, true);
        return true;
    }

    bool isRegistered(const std::string& id) const { return findById(id) != nullptr; }

    const Annotation* findById(const std::string& id) const {
        for (auto& a : m_annotations)
            if (a.id == id) return &a;
        return nullptr;
    }

    Annotation* findById(const std::string& id) {
        for (auto& a : m_annotations)
            if (a.id == id) return &a;
        return nullptr;
    }

    // Filtering ────────────────────────────────────────────────

    std::vector<const Annotation*> findByTarget(const std::string& targetId) const {
        std::vector<const Annotation*> result;
        for (auto& a : m_annotations)
            if (a.anchor.targetId == targetId) result.push_back(&a);
        return result;
    }

    std::vector<const Annotation*> findByAuthor(const std::string& author) const {
        std::vector<const Annotation*> result;
        for (auto& a : m_annotations)
            if (a.author == author) result.push_back(&a);
        return result;
    }

    std::vector<const Annotation*> findByKind(AnnotationKind kind) const {
        std::vector<const Annotation*> result;
        for (auto& a : m_annotations)
            if (a.kind == kind) result.push_back(&a);
        return result;
    }

    std::vector<const Annotation*> unresolved() const {
        std::vector<const Annotation*> result;
        for (auto& a : m_annotations)
            if (!a.resolved) result.push_back(&a);
        return result;
    }

    std::vector<const Annotation*> resolved() const {
        std::vector<const Annotation*> result;
        for (auto& a : m_annotations)
            if (a.resolved) result.push_back(&a);
        return result;
    }

    std::vector<std::string> allIds() const {
        std::vector<std::string> ids;
        ids.reserve(m_annotations.size());
        for (auto& a : m_annotations) ids.push_back(a.id);
        return ids;
    }

    int  count() const { return (int)m_annotations.size(); }
    bool empty() const { return m_annotations.empty(); }

    void clear() { m_annotations.clear(); }

    // Observers ────────────────────────────────────────────────

    uint32_t addObserver(Observer cb) {
        if (!cb || (int)m_observers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_observers.push_back({id, std::move(cb)});
        return id;
    }

    void removeObserver(uint32_t id) {
        m_observers.erase(
            std::remove_if(m_observers.begin(), m_observers.end(),
                [id](const ObserverEntry& e) { return e.id == id; }),
            m_observers.end());
    }

    void clearObservers() { m_observers.clear(); }

private:
    struct ObserverEntry { uint32_t id; Observer cb; };

    using AnnIt = std::vector<Annotation>::iterator;

    AnnIt findIt(const std::string& id) {
        return std::find_if(m_annotations.begin(), m_annotations.end(),
            [&](const Annotation& a) { return a.id == id; });
    }

    void notify(const Annotation& a, bool added) {
        for (auto& e : m_observers) e.cb(a, added);
    }

    std::vector<Annotation>    m_annotations;
    uint64_t                   m_clock            = 0;
    uint32_t                   m_nextObserverId   = 0;
    std::vector<ObserverEntry> m_observers;
};

} // namespace NF
