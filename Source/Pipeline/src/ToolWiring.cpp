// NF::Pipeline — S1 Tool Wiring
//
// Implements ToolAdapter, five concrete tool adapters, and ToolRegistry.
// All tools communicate exclusively through .change.json files in the
// shared .atlas/pipeline/changes/ directory.

#include "NF/Pipeline/Pipeline.h"

#include <chrono>

namespace NF {

namespace {

static int64_t nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(
               system_clock::now().time_since_epoch()).count();
}

} // anonymous namespace

// ── ToolAdapter (base) ────────────────────────────────────────────────────

bool ToolAdapter::emitEvent(ChangeEventType type,
                            const std::string& path,
                            const std::string& metadata,
                            const PipelineDirectories& dirs) {
    ChangeEvent ev;
    ev.tool      = name();
    ev.eventType = type;
    ev.path      = path;
    ev.timestamp = nowMs();
    ev.metadata  = metadata;
    return ev.writeToFile(dirs.changes);
}

// ── BlenderGenAdapter ─────────────────────────────────────────────────────

const char* BlenderGenAdapter::name() const noexcept {
    return "BlenderGenerator";
}

bool BlenderGenAdapter::acceptsEvent(ChangeEventType type) const noexcept {
    return type == ChangeEventType::AssetImported;
}

bool BlenderGenAdapter::handleEvent(const ChangeEvent& event,
                                    const PipelineDirectories& dirs) {
    if (!acceptsEvent(event.eventType)) return false;

    // Process the imported asset: acknowledge receipt and emit an
    // AnimationExported event if the asset contains animation data.
    std::string meta = "source_tool=" + event.tool + ";asset=" + event.path;
    emitEvent(ChangeEventType::AnimationExported,
              event.path, meta, dirs);
    ++m_handledCount;
    return true;
}

// ── ContractScannerAdapter ────────────────────────────────────────────────

const char* ContractScannerAdapter::name() const noexcept {
    return "ContractScanner";
}

bool ContractScannerAdapter::acceptsEvent(ChangeEventType type) const noexcept {
    return type == ChangeEventType::ScriptUpdated;
}

bool ContractScannerAdapter::handleEvent(const ChangeEvent& event,
                                         const PipelineDirectories& dirs) {
    if (!acceptsEvent(event.eventType)) return false;

    // Scan the updated script for contract violations and emit an issue
    // event if any are found.  (Actual static analysis is deferred to a
    // later milestone — the adapter wiring is the deliverable for S1.)
    std::string meta = "scanned=" + event.path + ";violations=0";
    emitEvent(ChangeEventType::ContractIssue,
              event.path, meta, dirs);
    ++m_handledCount;
    return true;
}

// ── ReplayMinimizerAdapter ────────────────────────────────────────────────

const char* ReplayMinimizerAdapter::name() const noexcept {
    return "ReplayMinimizer";
}

bool ReplayMinimizerAdapter::acceptsEvent(ChangeEventType type) const noexcept {
    return type == ChangeEventType::ReplayExported;
}

bool ReplayMinimizerAdapter::handleEvent(const ChangeEvent& event,
                                         const PipelineDirectories& dirs) {
    if (!acceptsEvent(event.eventType)) return false;

    // Minimize the replay and emit the result.  (Actual minimization
    // logic is deferred — the wiring is the S1 deliverable.)
    std::string minPath = event.path;
    // Replace .replay.json with .min.replay.json when possible.
    const std::string ext = ".replay.json";
    auto pos = minPath.rfind(ext);
    if (pos != std::string::npos)
        minPath.replace(pos, ext.size(), ".min.replay.json");

    std::string meta = "original=" + event.path;
    emitEvent(ChangeEventType::ReplayExported,
              minPath, meta, dirs);
    ++m_handledCount;
    return true;
}

// ── AtlasAIAdapter ────────────────────────────────────────────────────────
// Unified AI broker: absorbs the former SwissAgent broad-observer role.
// Accepts all non-Unknown event types and emits an AIAnalysis response for
// each, with specialised reasoning metadata for ContractIssue and WorldChanged.

const char* AtlasAIAdapter::name() const noexcept {
    return "AtlasAI";
}

bool AtlasAIAdapter::acceptsEvent(ChangeEventType type) const noexcept {
    return type != ChangeEventType::Unknown;
}

bool AtlasAIAdapter::handleEvent(const ChangeEvent& event,
                                 const PipelineDirectories& dirs) {
    if (!acceptsEvent(event.eventType)) return false;

    std::string meta;
    if (event.eventType == ChangeEventType::ContractIssue ||
        event.eventType == ChangeEventType::WorldChanged) {
        // Specialised reasoning for contract/world events.
        meta = "reasoning=" +
               std::string(changeEventTypeName(event.eventType)) +
               ";subject=" + event.path;
    } else {
        // Broad-spectrum analysis for all other event types.
        meta = "analyzed_event=" +
               std::string(changeEventTypeName(event.eventType)) +
               ";source_tool=" + event.tool;
    }
    emitEvent(ChangeEventType::AIAnalysis,
              event.path, meta, dirs);
    ++m_handledCount;
    return true;
}

// ── ToolRegistry ──────────────────────────────────────────────────────────

void ToolRegistry::registerTool(std::unique_ptr<ToolAdapter> tool) {
    if (tool)
        m_tools.push_back(std::move(tool));
}

void ToolRegistry::attach(PipelineWatcher& watcher,
                           const PipelineDirectories& dirs) {
    // Capture a raw pointer to `this` and the dirs by value.  The
    // registry must outlive the watcher.
    watcher.subscribe([this, dirs](const ChangeEvent& event) {
        dispatch(event, dirs);
    });
}

int ToolRegistry::dispatch(const ChangeEvent& event,
                            const PipelineDirectories& dirs) {
    int count = 0;
    for (auto& tool : m_tools) {
        if (tool->acceptsEvent(event.eventType)) {
            if (tool->handleEvent(event, dirs))
                ++count;
        }
    }
    return count;
}

const ToolAdapter* ToolRegistry::tool(size_t index) const {
    if (index >= m_tools.size()) return nullptr;
    return m_tools[index].get();
}

} // namespace NF
