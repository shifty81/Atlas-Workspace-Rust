#pragma once
#include "../../engine/interaction/Utterance.h"
#include "../../engine/interaction/Intent.h"
#include <vector>

namespace atlas::editor {

struct DebuggerLogEntry {
    interaction::Utterance utterance;
    interaction::Intent intent;
};

class InteractionDebugger {
public:
    void Record(const DebuggerLogEntry& entry);
    void Clear();
    size_t EntryCount() const;
    const DebuggerLogEntry& GetEntry(size_t index) const;

private:
    std::vector<DebuggerLogEntry> log;
};

} // namespace atlas::editor
