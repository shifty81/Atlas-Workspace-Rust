#pragma once
// NovaForge::PCGGeneratorService — stateless PCG generator service.
//
// Takes a PCGRuleSet + PCGDeterministicSeedContext and produces a
// PCGGenerationResult — a snapshot of the generated content for a given domain.
//
// The service is stateless: the same inputs always produce the same outputs.
// Results are renderer-agnostic: meshTags/materialTags/placements are symbolic
// so they can be consumed by the preview world or the game runtime equally.
//
// Phase E.1 — Shared NovaForge PCG Core

#include "NovaForge/EditorAdapter/PCGRuleSet.h"
#include "NovaForge/EditorAdapter/PCGDeterministicSeedContext.h"
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>

namespace NovaForge {

// ── PCGPlacement ─────────────────────────────────────────────────────────────
// A single generated placement — symbolic name + transform parameters.

struct PCGPlacement {
    std::string  assetTag;
    float        x = 0.f, y = 0.f, z = 0.f;
    float        yaw = 0.f;  ///< rotation around Y axis (degrees)
    float        scale = 1.f;
    std::string  materialTag;
    std::string  pcgTag;     ///< source placement tag that generated this entry
};

// ── PCGGenerationResult ───────────────────────────────────────────────────────

struct PCGGenerationResult {
    bool                      success       = false;
    uint32_t                  seed          = 0;
    std::string               ruleSetId;
    std::string               domain;
    std::vector<PCGPlacement> placements;
    uint32_t                  generatedCount = 0;  ///< total entries before deduplication
    uint32_t                  culledCount    = 0;  ///< entries removed by exclusion / density
    std::string               errorMessage;
};

// ── PCGGeneratorService ───────────────────────────────────────────────────────

class PCGGeneratorService {
public:
    PCGGeneratorService()  = default;
    ~PCGGeneratorService() = default;

    // ── Generation ────────────────────────────────────────────────────────
    // Generates placement output from a rule set + seed context.
    // The domain used is taken from ruleSet.domain(), or overridden via domainOverride.

    PCGGenerationResult generate(const PCGRuleSet&                ruleSet,
                                 const PCGDeterministicSeedContext& seedCtx,
                                 const std::string&                domainOverride = "") const {
        PCGGenerationResult result;
        result.ruleSetId = ruleSet.id();
        result.domain    = domainOverride.empty() ? ruleSet.domain() : domainOverride;
        result.seed      = static_cast<uint32_t>(seedCtx.seedForDomain(result.domain) & 0xFFFFFFFFu);

        if (ruleSet.ruleCount() == 0) {
            result.success      = true; // empty ruleset → empty output, not an error
            return result;
        }

        // Deterministic placement generation from density + placementTag rules
        const std::string densityStr  = ruleSet.getValue("density", "1.0");
        const std::string countStr    = ruleSet.getValue("count", "10");
        const std::string placementTag = ruleSet.getValue("placementTag", "default");
        const std::string materialTag  = ruleSet.getValue("materialTag", "mat/default");

        const float density = densityStr.empty() ? 1.f : static_cast<float>(std::atof(densityStr.c_str()));
        const int   count   = countStr.empty()   ? 10  : std::atoi(countStr.c_str());
        const int   actual  = static_cast<int>(static_cast<float>(count) * density);

        // Simple xorshift PRNG seeded from result.seed for reproducible layout
        uint64_t rng = static_cast<uint64_t>(result.seed) | 1;
        auto xnext = [&]() -> uint32_t {
            rng ^= rng << 13;
            rng ^= rng >> 7;
            rng ^= rng << 17;
            return static_cast<uint32_t>(rng & 0xFFFFFFFFu);
        };

        const std::string exclusion = ruleSet.getValue("exclusionGroup", "");

        result.generatedCount = static_cast<uint32_t>(actual < 0 ? 0 : actual);
        for (int i = 0; i < actual && i >= 0; ++i) {
            PCGPlacement p;
            p.assetTag   = placementTag;
            p.materialTag = materialTag;
            p.pcgTag      = placementTag;

            // Generate position in a normalized [−50, 50] area
            auto toF = [](uint32_t v, float scale) -> float {
                return (static_cast<float>(v % 10000) / 10000.f - 0.5f) * scale;
            };
            p.x   = toF(xnext(), 100.f);
            p.y   = 0.f;
            p.z   = toF(xnext(), 100.f);
            p.yaw = static_cast<float>(xnext() % 360);

            const std::string minScaleStr = ruleSet.getValue("minScale", "0.8");
            const std::string maxScaleStr = ruleSet.getValue("maxScale", "1.2");
            const float minS = minScaleStr.empty() ? 0.8f : static_cast<float>(std::atof(minScaleStr.c_str()));
            const float maxS = maxScaleStr.empty() ? 1.2f : static_cast<float>(std::atof(maxScaleStr.c_str()));
            const float t = static_cast<float>(xnext() % 1000) / 1000.f;
            p.scale = minS + t * (maxS - minS);

            result.placements.push_back(p);
        }
        result.success = true;
        return result;
    }

    // ── Validation ────────────────────────────────────────────────────────
    // Validates that a rule set has the required rules for generation.

    struct ValidationResult {
        bool                     valid = true;
        std::vector<std::string> missingRules;
        std::vector<std::string> warnings;
    };

    static ValidationResult validate(const PCGRuleSet& ruleSet) {
        ValidationResult r;

        // Check recommended rules
        static const char* recommended[] = { "density", "count", "placementTag" };
        for (const char* key : recommended) {
            if (!ruleSet.hasRule(key))
                r.warnings.push_back(std::string("Missing recommended rule: ") + key);
        }

        if (ruleSet.id().empty()) {
            r.valid = false;
            r.missingRules.push_back("id");
        }

        return r;
    }
};

} // namespace NovaForge
