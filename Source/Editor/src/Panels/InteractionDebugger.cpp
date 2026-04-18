#include "InteractionDebugger.h"

namespace atlas::editor {

void InteractionDebugger::Record(const DebuggerLogEntry& entry)
{
    log.push_back(entry);
}

void InteractionDebugger::Clear()
{
    log.clear();
}

size_t InteractionDebugger::EntryCount() const
{
    return log.size();
}

const DebuggerLogEntry& InteractionDebugger::GetEntry(size_t index) const
{
    return log.at(index);
}

} // namespace atlas::editor
