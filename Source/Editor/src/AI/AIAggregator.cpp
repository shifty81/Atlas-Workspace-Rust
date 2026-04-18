#include "AIAggregator.h"

namespace atlas::ai {

void AIAggregator::RegisterBackend(AIBackend* backend) {
    if (backend) {
        m_backends.push_back(backend);
    }
}

AggregatorResponse AIAggregator::Execute(
    AIRequestType type,
    const std::string& prompt,
    const AIContext& context) {
    if (m_backends.empty()) {
        return AggregatorResponse{};
    }

    const char* prefix = "";
    switch (type) {
        case AIRequestType::GraphGeneration: prefix = "[GraphGeneration] "; break;
        case AIRequestType::WorldGeneration: prefix = "[WorldGeneration] "; break;
        case AIRequestType::CodeAssist:      prefix = "[CodeAssist] ";      break;
        case AIRequestType::Analysis:        prefix = "[Analysis] ";        break;
    }

    std::string enhancedPrompt = std::string(prefix) + prompt;

    std::vector<AggregatorResponse> responses;
    for (auto* backend : m_backends) {
        AggregatorResponse resp = backend->Query(enhancedPrompt, context);
        if (!resp.content.empty()) {
            responses.push_back(std::move(resp));
        }
    }

    return SelectBest(responses);
}

AggregatorResponse AIAggregator::SelectBest(const std::vector<AggregatorResponse>& responses) const {
    if (responses.empty()) {
        return AggregatorResponse{};
    }

    const AggregatorResponse* best = &responses[0];
    for (size_t i = 1; i < responses.size(); ++i) {
        if (responses[i].confidence > best->confidence) {
            best = &responses[i];
        }
    }

    return *best;
}

size_t AIAggregator::BackendCount() const {
    return m_backends.size();
}

}
