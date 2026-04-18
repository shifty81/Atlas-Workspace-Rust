#pragma once
#include <string>
#include <vector>

namespace atlas::ai {

enum class AIRequestType {
    GraphGeneration,
    WorldGeneration,
    CodeAssist,
    Analysis,
};

struct AIContext {
    std::string projectName;
    std::vector<std::string> loadedAssets;
    std::string selectedAsset;
    std::string networkMode;
};

struct AggregatorResponse {
    std::string content;
    float confidence = 0.0f;
};

using AIResponse = AggregatorResponse;

class AIBackend {
public:
    virtual ~AIBackend() = default;
    virtual AggregatorResponse Query(
        const std::string& prompt,
        const AIContext& context) = 0;
};

class AIAggregator {
public:
    void RegisterBackend(AIBackend* backend);

    AggregatorResponse Execute(
        AIRequestType type,
        const std::string& prompt,
        const AIContext& context);

    size_t BackendCount() const;

private:
    std::vector<AIBackend*> m_backends;

    AggregatorResponse SelectBest(const std::vector<AggregatorResponse>& responses) const;
};

}
