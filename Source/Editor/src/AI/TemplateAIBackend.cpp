#include "TemplateAIBackend.h"
#include <algorithm>
#include <sstream>

namespace atlas::ai {

TemplateAIBackend::TemplateAIBackend() {
    RegisterDefaults();
}

void TemplateAIBackend::RegisterDefaults() {
    // Graph-related templates
    m_templates.push_back({
        "create graph",
        "To create a new graph in ${project}, open the Flow Graph panel and "
        "select File > New Graph. Choose a graph type (logic, animation, or "
        "material) and the editor will scaffold the default node layout.",
        AIRequestType::GraphGeneration});

    m_templates.push_back({
        "add node",
        "To add a node, right-click the graph canvas and select from the node "
        "palette. Nodes are categorised by system (math, logic, asset, event). "
        "Drag from the palette or use the search bar to find nodes by name.",
        AIRequestType::GraphGeneration});

    m_templates.push_back({
        "connect nodes",
        "To connect nodes, click an output port and drag to a compatible input "
        "port. The editor validates type compatibility automatically. Hover over "
        "a port to see its expected type.",
        AIRequestType::GraphGeneration});

    // World-related templates
    m_templates.push_back({
        "generate world",
        "World generation in ${project} uses the terrain pipeline. Configure "
        "biome parameters, heightmap resolution, and seed value in the World "
        "Settings panel, then click Generate. Results appear in the preview "
        "viewport before committing.",
        AIRequestType::WorldGeneration});

    m_templates.push_back({
        "terrain",
        "The terrain system supports layered heightmaps, erosion simulation, "
        "and biome-based material assignment. Edit terrain brushes in the "
        "Terrain Tools panel. Each layer can be exported independently.",
        AIRequestType::WorldGeneration});

    m_templates.push_back({
        "heightmap",
        "Heightmaps are stored as 16-bit grayscale images. Import via "
        "Assets > Import Heightmap or generate procedurally through the "
        "terrain pipeline. The resolution must be a power of two plus one "
        "(e.g. 1025x1025).",
        AIRequestType::WorldGeneration});

    // Code-related templates
    m_templates.push_back({
        "explain",
        "The selected asset '${asset}' is part of the ${project} project. "
        "Check the asset's schema definition for its properties and the "
        "flow graph panel for any attached logic. See the documentation "
        "panel for system-level explanations.",
        AIRequestType::CodeAssist});

    m_templates.push_back({
        "how does",
        "Atlas systems follow a modular architecture. Each system registers "
        "with the engine core and communicates through typed events and asset "
        "references. Consult the relevant doc in the docs/ folder for detailed "
        "architecture of each subsystem.",
        AIRequestType::CodeAssist});

    m_templates.push_back({
        "what is",
        "Atlas is a deterministic game engine with an editor-centric workflow. "
        "Assets, graphs, and schemas are the primary authoring primitives. "
        "Runtime replay is fully deterministic and independent of editor or "
        "AI state.",
        AIRequestType::CodeAssist});

    // Analysis templates
    m_templates.push_back({
        "analyze",
        "To analyze ${asset} in ${project}, open the Validation panel. "
        "It checks schema conformance, graph connectivity, and asset "
        "reference integrity. Issues are listed by severity with suggested "
        "fixes.",
        AIRequestType::Analysis});

    m_templates.push_back({
        "check",
        "Run a validation check from the Tools menu or press Ctrl+Shift+V. "
        "The checker verifies asset references, graph cycles, and schema "
        "compliance. Results are grouped by category in the output panel.",
        AIRequestType::Analysis});

    m_templates.push_back({
        "validate",
        "Validation in ${project} operates on three levels: schema validation "
        "(structural correctness), graph validation (connectivity and type "
        "safety), and asset validation (reference integrity and missing "
        "dependencies).",
        AIRequestType::Analysis});
}

AggregatorResponse TemplateAIBackend::Query(
    const std::string& prompt,
    const AIContext& context) {
    if (prompt.empty()) {
        return AggregatorResponse{};
    }

    // Strip type prefix if present (e.g. "[GraphGeneration] ")
    std::string cleanPrompt = prompt;
    if (!cleanPrompt.empty() && cleanPrompt[0] == '[') {
        auto closing = cleanPrompt.find("] ");
        if (closing != std::string::npos) {
            cleanPrompt = cleanPrompt.substr(closing + 2);
        }
    }

    return MatchTemplate(cleanPrompt, context);
}

AggregatorResponse TemplateAIBackend::MatchTemplate(
    const std::string& prompt,
    const AIContext& context) const {
    float bestScore = 0.0f;
    const TemplateEntry* bestEntry = nullptr;

    for (const auto& entry : m_templates) {
        float score = ComputeMatchScore(prompt, entry.pattern);
        if (score > bestScore) {
            bestScore = score;
            bestEntry = &entry;
        }
    }

    if (!bestEntry || bestScore <= 0.0f) {
        return AggregatorResponse{};
    }

    // Count additional matches for confidence boost
    int matchCount = 0;
    for (const auto& entry : m_templates) {
        if (&entry == bestEntry) continue;
        if (ComputeMatchScore(prompt, entry.pattern) > 0.0f) ++matchCount;
    }

    AggregatorResponse resp;
    resp.content = ExpandVariables(bestEntry->response, context);
    resp.confidence = std::min(1.0f, bestEntry->confidence * bestScore + matchCount * 0.05f);
    return resp;
}

std::string TemplateAIBackend::ExpandVariables(
    const std::string& tmpl,
    const AIContext& context) const {
    std::string result = tmpl;

    std::string project = context.projectName.empty()
        ? "the current project" : context.projectName;
    std::string asset = context.selectedAsset.empty()
        ? "the selected asset" : context.selectedAsset;
    std::string mode = context.networkMode.empty()
        ? "default" : context.networkMode;

    // Replace ${project}
    for (std::string::size_type pos = result.find("${project}");
         pos != std::string::npos;
         pos = result.find("${project}", pos)) {
        result.replace(pos, 10, project);
        pos += project.size();
    }

    // Replace ${asset}
    for (std::string::size_type pos = result.find("${asset}");
         pos != std::string::npos;
         pos = result.find("${asset}", pos)) {
        result.replace(pos, 8, asset);
        pos += asset.size();
    }

    // Replace ${mode}
    for (std::string::size_type pos = result.find("${mode}");
         pos != std::string::npos;
         pos = result.find("${mode}", pos)) {
        result.replace(pos, 7, mode);
        pos += mode.size();
    }

    return result;
}

float TemplateAIBackend::ComputeMatchScore(
    const std::string& prompt,
    const std::string& pattern) const {
    // Tokenize the pattern into words
    std::vector<std::string> patternWords;
    {
        std::istringstream stream(pattern);
        std::string word;
        while (stream >> word) {
            patternWords.push_back(word);
        }
    }

    if (patternWords.empty()) {
        return 0.0f;
    }

    // Convert prompt to lowercase for case-insensitive matching
    std::string lowerPrompt = prompt;
    std::transform(lowerPrompt.begin(), lowerPrompt.end(),
                   lowerPrompt.begin(), ::tolower);

    int matched = 0;
    for (const auto& w : patternWords) {
        std::string lowerWord = w;
        std::transform(lowerWord.begin(), lowerWord.end(),
                       lowerWord.begin(), ::tolower);
        if (lowerPrompt.find(lowerWord) != std::string::npos) {
            ++matched;
        }
    }

    return static_cast<float>(matched) / static_cast<float>(patternWords.size());
}

void TemplateAIBackend::AddTemplate(
    const std::string& pattern,
    const std::string& response) {
    m_templates.push_back({pattern, response, AIRequestType::GraphGeneration, 0.4f});
}

void TemplateAIBackend::AddTemplate(
    const std::string& keyword,
    const std::string& response,
    float confidence) {
    if (keyword.empty()) return;
    m_templates.push_back({keyword, response, AIRequestType::GraphGeneration, confidence});
}

bool TemplateAIBackend::RemoveTemplate(const std::string& keyword) {
    for (auto it = m_templates.begin(); it != m_templates.end(); ++it) {
        if (it->pattern == keyword) {
            m_templates.erase(it);
            return true;
        }
    }
    return false;
}

size_t TemplateAIBackend::TemplateCount() const {
    return m_templates.size();
}

void TemplateAIBackend::ClearTemplates() {
    m_templates.clear();
}

} // namespace atlas::ai
