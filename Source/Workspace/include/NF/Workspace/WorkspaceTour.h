#pragma once
// NF::Workspace — Phase 32: Workspace Tour / Onboarding System
//
// Guided onboarding tour with typed steps, sequences, and a controller:
//   TourStepKind     — Highlight / Tooltip / Modal / Action / Pause (5 kinds)
//   TourStep         — id + kind + targetId + title + body + optional actionLabel
//   TourSequence     — ordered step list with name + id; isValid()
//   TourState        — Idle / Running / Paused / Completed / Cancelled
//   TourProgress     — current sequence + step index + total
//   TourController   — load/start/next/prev/pause/resume/cancel/complete; observer

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// TourStepKind — presentation style of a single tour step
// ═════════════════════════════════════════════════════════════════

enum class TourStepKind : uint8_t {
    Highlight,  // highlight a UI element
    Tooltip,    // show a tooltip near an element
    Modal,      // full blocking modal
    Action,     // wait for user to perform an action
    Pause,      // timed or manual pause
};

inline const char* tourStepKindName(TourStepKind k) {
    switch (k) {
        case TourStepKind::Highlight: return "Highlight";
        case TourStepKind::Tooltip:   return "Tooltip";
        case TourStepKind::Modal:     return "Modal";
        case TourStepKind::Action:    return "Action";
        case TourStepKind::Pause:     return "Pause";
        default:                      return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// TourState — lifecycle state of the TourController
// ═════════════════════════════════════════════════════════════════

enum class TourState : uint8_t {
    Idle,
    Running,
    Paused,
    Completed,
    Cancelled,
};

inline const char* tourStateName(TourState s) {
    switch (s) {
        case TourState::Idle:      return "Idle";
        case TourState::Running:   return "Running";
        case TourState::Paused:    return "Paused";
        case TourState::Completed: return "Completed";
        case TourState::Cancelled: return "Cancelled";
        default:                   return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// TourStep — one step in a guided tour
// ═════════════════════════════════════════════════════════════════

struct TourStep {
    std::string  id;
    TourStepKind kind        = TourStepKind::Tooltip;
    std::string  targetId;   // UI element or panel id to highlight
    std::string  title;
    std::string  body;
    std::string  actionLabel; // optional CTA label (e.g. "Click here")

    bool isValid() const { return !id.empty() && !title.empty(); }
};

// ═════════════════════════════════════════════════════════════════
// TourSequence — ordered list of tour steps
// ═════════════════════════════════════════════════════════════════

struct TourSequence {
    static constexpr int kMaxSteps = 128;

    std::string           id;
    std::string           name;
    std::vector<TourStep> steps;

    bool isValid() const { return !id.empty() && !name.empty() && !steps.empty(); }

    bool addStep(const TourStep& step) {
        if (!step.isValid()) return false;
        if (static_cast<int>(steps.size()) >= kMaxSteps) return false;
        steps.push_back(step);
        return true;
    }

    int stepCount() const { return static_cast<int>(steps.size()); }

    const TourStep* stepAt(int index) const {
        if (index < 0 || index >= stepCount()) return nullptr;
        return &steps[static_cast<std::size_t>(index)];
    }
};

// ═════════════════════════════════════════════════════════════════
// TourProgress — snapshot of current tour position
// ═════════════════════════════════════════════════════════════════

struct TourProgress {
    std::string sequenceId;
    int         stepIndex  = -1;
    int         totalSteps = 0;

    bool isActive() const { return stepIndex >= 0 && stepIndex < totalSteps; }
    float fraction() const {
        if (totalSteps <= 0) return 0.0f;
        return static_cast<float>(stepIndex + 1) / static_cast<float>(totalSteps);
    }
};

// ═════════════════════════════════════════════════════════════════
// TourController — manages tour lifecycle
// ═════════════════════════════════════════════════════════════════

struct TourController {
    using ObserverFn = std::function<void(TourState, const TourProgress&)>;

    TourState state() const { return state_; }

    const TourProgress& progress() const { return progress_; }

    bool load(const TourSequence& seq) {
        if (!seq.isValid()) return false;
        sequence_ = seq;
        return true;
    }

    bool start() {
        if (state_ == TourState::Running || state_ == TourState::Paused) return false;
        if (!sequence_.isValid()) return false;
        progress_.sequenceId = sequence_.id;
        progress_.totalSteps = sequence_.stepCount();
        progress_.stepIndex  = 0;
        setState(TourState::Running);
        return true;
    }

    bool next() {
        if (state_ != TourState::Running) return false;
        if (progress_.stepIndex + 1 >= progress_.totalSteps) {
            complete();
            return true;
        }
        ++progress_.stepIndex;
        notify();
        return true;
    }

    bool prev() {
        if (state_ != TourState::Running) return false;
        if (progress_.stepIndex <= 0) return false;
        --progress_.stepIndex;
        notify();
        return true;
    }

    bool pause() {
        if (state_ != TourState::Running) return false;
        setState(TourState::Paused);
        return true;
    }

    bool resume() {
        if (state_ != TourState::Paused) return false;
        setState(TourState::Running);
        return true;
    }

    bool cancel() {
        if (state_ == TourState::Idle || state_ == TourState::Completed ||
            state_ == TourState::Cancelled) return false;
        setState(TourState::Cancelled);
        return true;
    }

    void complete() {
        if (state_ == TourState::Completed) return;
        progress_.stepIndex = progress_.totalSteps; // past-end sentinel
        setState(TourState::Completed);
    }

    bool reset() {
        state_    = TourState::Idle;
        progress_ = TourProgress{};
        return true;
    }

    const TourStep* currentStep() const {
        if (state_ != TourState::Running && state_ != TourState::Paused) return nullptr;
        return sequence_.stepAt(progress_.stepIndex);
    }

    bool addObserver(ObserverFn fn) {
        if (static_cast<int>(observers_.size()) >= kMaxObservers) return false;
        observers_.push_back(std::move(fn));
        return true;
    }

    void clearObservers() { observers_.clear(); }

private:
    static constexpr int kMaxObservers = 16;

    TourState       state_    = TourState::Idle;
    TourSequence    sequence_;
    TourProgress    progress_;
    std::vector<ObserverFn> observers_;

    void setState(TourState s) {
        state_ = s;
        notify();
    }

    void notify() {
        for (auto& obs : observers_)
            if (obs) obs(state_, progress_);
    }
};

} // namespace NF
