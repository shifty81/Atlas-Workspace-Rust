#pragma once
// NF::Workspace — AssetTransformer: import step chain for converting raw source
// files into workspace-ready assets.
//
// An AssetTransformer is a linear pipeline of TransformStep functions applied in
// order to a TransformContext. Each step can modify the context (e.g. decode,
// resize, compress, pack) and report progress. Any step returning an error aborts
// the chain and produces a TransformResult with ErrorStep set.
//
// Design:
//   TransformStepResult  — per-step outcome (Ok / Skip / Error + message)
//   TransformContext     — mutable state threaded through the chain
//   TransformStep        — callable: (TransformContext&) → TransformStepResult
//   TransformChain       — ordered list of named steps; supports add/remove/run
//   TransformResult      — final outcome of running a chain on one context
//   AssetTransformer     — manages multiple named chains; routes by AssetTypeTag

#include "NF/Workspace/AssetCatalog.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Transform Step Result ──────────────────────────────────────────

enum class TransformStepStatus : uint8_t {
    Ok,     // step completed successfully; continue chain
    Skip,   // step opted out (e.g. not applicable); continue chain
    Error,  // step failed; abort chain
};

inline const char* transformStepStatusName(TransformStepStatus s) {
    switch (s) {
    case TransformStepStatus::Ok:    return "Ok";
    case TransformStepStatus::Skip:  return "Skip";
    case TransformStepStatus::Error: return "Error";
    }
    return "Unknown";
}

struct TransformStepResult {
    TransformStepStatus status  = TransformStepStatus::Ok;
    std::string         message;  // optional detail for Ok/Skip; required for Error

    [[nodiscard]] bool succeeded() const { return status != TransformStepStatus::Error; }
    [[nodiscard]] bool isError()   const { return status == TransformStepStatus::Error; }
    [[nodiscard]] bool wasSkipped() const { return status == TransformStepStatus::Skip; }

    static TransformStepResult ok(const std::string& msg = {}) {
        return {TransformStepStatus::Ok, msg};
    }
    static TransformStepResult skip(const std::string& msg = {}) {
        return {TransformStepStatus::Skip, msg};
    }
    static TransformStepResult error(const std::string& msg) {
        return {TransformStepStatus::Error, msg};
    }
};

// ── Transform Context ──────────────────────────────────────────────
// Mutable bag of state threaded through the step chain.

struct TransformContext {
    AssetId          assetId     = INVALID_ASSET_ID;
    std::string      sourcePath;
    std::string      outputPath;
    AssetTypeTag     typeTag     = AssetTypeTag::Unknown;
    float            progress    = 0.f;    // 0..1, updated by steps
    AssetMetadata    metadata;             // steps may add/read metadata

    // Scratch area — steps may store intermediate results here as string values
    std::vector<std::pair<std::string, std::string>> scratchData;

    void setScratch(const std::string& key, const std::string& val) {
        for (auto& [k, v] : scratchData) {
            if (k == key) { v = val; return; }
        }
        scratchData.push_back({key, val});
    }

    [[nodiscard]] const std::string* getScratch(const std::string& key) const {
        for (const auto& [k, v] : scratchData)
            if (k == key) return &v;
        return nullptr;
    }

    [[nodiscard]] bool isValid() const {
        return assetId != INVALID_ASSET_ID
            && !sourcePath.empty()
            && typeTag != AssetTypeTag::Unknown;
    }
};

// ── Transform Step ─────────────────────────────────────────────────
using TransformStepFn = std::function<TransformStepResult(TransformContext&)>;

struct TransformStep {
    std::string     name;
    TransformStepFn fn;
    bool            enabled = true;

    [[nodiscard]] bool isValid() const { return !name.empty() && fn != nullptr; }
};

// ── Transform Result ───────────────────────────────────────────────

struct TransformResult {
    bool        succeeded      = false;
    std::string errorStep;          // name of the step that failed (empty on success)
    std::string errorMessage;       // error message from that step
    size_t      stepsRun       = 0;
    size_t      stepsSkipped   = 0;
    float       finalProgress  = 0.f;

    [[nodiscard]] bool failed() const { return !succeeded; }
};

// ── TransformChain ─────────────────────────────────────────────────
// Ordered sequence of named steps. Run all enabled steps in order.

class TransformChain {
public:
    static constexpr size_t MAX_STEPS = 64;

    bool addStep(TransformStep step) {
        if (!step.isValid()) return false;
        if (m_steps.size() >= MAX_STEPS) return false;
        for (const auto& s : m_steps) if (s.name == step.name) return false;
        m_steps.push_back(std::move(step));
        return true;
    }

    bool removeStep(const std::string& name) {
        for (auto it = m_steps.begin(); it != m_steps.end(); ++it) {
            if (it->name == name) { m_steps.erase(it); return true; }
        }
        return false;
    }

    bool enableStep(const std::string& name, bool enabled) {
        for (auto& s : m_steps) {
            if (s.name == name) { s.enabled = enabled; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t stepCount()        const { return m_steps.size(); }
    [[nodiscard]] bool   hasStep(const std::string& n) const {
        for (const auto& s : m_steps) if (s.name == n) return true;
        return false;
    }

    TransformResult run(TransformContext& ctx) const {
        TransformResult result;
        result.stepsRun     = 0;
        result.stepsSkipped = 0;

        for (const auto& step : m_steps) {
            if (!step.enabled) continue;
            auto sr = step.fn(ctx);
            ++result.stepsRun;

            if (sr.wasSkipped()) {
                ++result.stepsSkipped;
                continue;
            }
            if (sr.isError()) {
                result.succeeded    = false;
                result.errorStep    = step.name;
                result.errorMessage = sr.message;
                result.finalProgress = ctx.progress;
                return result;
            }
        }

        result.succeeded     = true;
        result.finalProgress = ctx.progress;
        return result;
    }

    [[nodiscard]] const std::vector<TransformStep>& steps() const { return m_steps; }

private:
    std::vector<TransformStep> m_steps;
};

// ── AssetTransformer ───────────────────────────────────────────────
// Manages one TransformChain per AssetTypeTag.
// Falls back to the "default" chain when no type-specific chain is found.

class AssetTransformer {
public:
    // Register a chain for a specific type tag. Overwrites any existing chain.
    void registerChain(AssetTypeTag tag, TransformChain chain) {
        m_chains[static_cast<uint8_t>(tag)] = std::move(chain);
    }

    // Register the fallback chain (used when no per-type chain is found)
    void setDefaultChain(TransformChain chain) {
        m_defaultChain = std::move(chain);
    }

    // Returns the chain for the given type (or the default chain pointer if not found)
    [[nodiscard]] const TransformChain* chainFor(AssetTypeTag tag) const {
        auto it = m_chains.find(static_cast<uint8_t>(tag));
        if (it != m_chains.end()) return &it->second;
        return m_defaultChain.stepCount() > 0 ? &m_defaultChain : nullptr;
    }

    [[nodiscard]] bool hasChainFor(AssetTypeTag tag) const {
        return m_chains.count(static_cast<uint8_t>(tag)) > 0;
    }

    // Run the appropriate chain for ctx.typeTag
    TransformResult transform(TransformContext& ctx) {
        if (!ctx.isValid()) {
            TransformResult r;
            r.succeeded    = false;
            r.errorStep    = "(validate)";
            r.errorMessage = "TransformContext is not valid";
            return r;
        }

        const TransformChain* chain = chainFor(ctx.typeTag);
        if (!chain) {
            TransformResult r;
            r.succeeded    = false;
            r.errorStep    = "(router)";
            r.errorMessage = "No transform chain for type: "
                           + std::string(assetTypeTagName(ctx.typeTag));
            return r;
        }

        ++m_totalTransforms;
        auto result = chain->run(ctx);
        if (result.succeeded) ++m_totalSucceeded;
        else                  ++m_totalFailed;
        return result;
    }

    [[nodiscard]] size_t chainCount()       const { return m_chains.size();      }
    [[nodiscard]] size_t totalTransforms()  const { return m_totalTransforms;    }
    [[nodiscard]] size_t totalSucceeded()   const { return m_totalSucceeded;     }
    [[nodiscard]] size_t totalFailed()      const { return m_totalFailed;        }

private:
    std::unordered_map<uint8_t, TransformChain> m_chains;
    TransformChain                              m_defaultChain;
    size_t m_totalTransforms = 0;
    size_t m_totalSucceeded  = 0;
    size_t m_totalFailed     = 0;
};

} // namespace NF
