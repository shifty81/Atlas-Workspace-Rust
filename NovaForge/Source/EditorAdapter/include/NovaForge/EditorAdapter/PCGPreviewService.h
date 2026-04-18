#pragma once
// NovaForge::PCGPreviewService — editor-side PCG preview wrapper.
//
// Wraps PCGGeneratorService and PCGRuleSet to provide:
//   - Result caching (avoid regenerating every frame)
//   - Manual regeneration trigger (forceRegenerate())
//   - Automatic regeneration on rule edit (if autoRegenerate is enabled)
//   - Seed management via PCGDeterministicSeedContext
//   - Population of a NovaForgePreviewWorld from generated placements
//
// Phase E.2 — Editor PCG Preview Service
// Phase E.3 — PCG Rule Editing
// Phase E.4 — Asset PCG Metadata

#include "NovaForge/EditorAdapter/PCGGeneratorService.h"
#include "NovaForge/EditorAdapter/PCGRuleSet.h"
#include "NovaForge/EditorAdapter/PCGDeterministicSeedContext.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewWorld.h"
#include <functional>
#include <memory>
#include <string>

namespace NovaForge {

// ── PCGPreviewStats ───────────────────────────────────────────────────────────

struct PCGPreviewStats {
    uint32_t generationCount  = 0; ///< how many times generate() was called
    uint32_t lastPlacementCount = 0;
    float    lastGenerationMs = 0.f;
    bool     hasResult        = false;
};

// ── PCGPreviewService ─────────────────────────────────────────────────────────

class PCGPreviewService {
public:
    PCGPreviewService()  = default;
    ~PCGPreviewService() = default;

    // ── RuleSet binding (E.3) ─────────────────────────────────────────────

    /// Bind a PCGRuleSet document for preview.
    /// Triggers regeneration if autoRegenerate is enabled.
    void bindRuleSet(PCGRuleSet* ruleSet) {
        m_ruleSet    = ruleSet;
        m_resultValid = false;
        if (m_autoRegenerate && m_ruleSet) regenerate();
    }

    void clearRuleSet() { m_ruleSet = nullptr; m_resultValid = false; }

    [[nodiscard]] bool hasRuleSet()   const { return m_ruleSet != nullptr; }
    [[nodiscard]] PCGRuleSet* ruleSet() const { return m_ruleSet; }

    // ── Rule editing → auto-regen (E.3) ──────────────────────────────────

    /// Update a rule value. If autoRegenerate is enabled, triggers regen.
    bool setRuleValue(const std::string& key, const std::string& value) {
        if (!m_ruleSet) return false;
        if (!m_ruleSet->setValue(key, value)) return false;
        if (m_autoRegenerate) regenerate();
        return true;
    }

    /// Reset all rules to defaults. If autoRegenerate is enabled, triggers regen.
    bool resetRules() {
        if (!m_ruleSet) return false;
        m_ruleSet->resetToDefaults();
        if (m_autoRegenerate) regenerate();
        return true;
    }

    // ── Seed management ───────────────────────────────────────────────────

    void setSeed(uint64_t seed) {
        m_seedCtx.setUniverseSeed(seed);
        m_resultValid = false;
    }

    [[nodiscard]] uint64_t currentSeed() const { return m_seedCtx.universeSeed(); }

    [[nodiscard]] const PCGDeterministicSeedContext& seedContext() const { return m_seedCtx; }
    [[nodiscard]]       PCGDeterministicSeedContext& seedContext()       { return m_seedCtx; }

    // ── Auto-regeneration ─────────────────────────────────────────────────

    void setAutoRegenerate(bool enabled) { m_autoRegenerate = enabled; }
    [[nodiscard]] bool autoRegenerate()  const { return m_autoRegenerate; }

    // ── Manual regeneration trigger ───────────────────────────────────────

    bool forceRegenerate() {
        m_resultValid = false;
        return regenerate();
    }

    // ── Result access ─────────────────────────────────────────────────────

    [[nodiscard]] bool hasResult() const { return m_resultValid; }

    [[nodiscard]] const PCGGenerationResult& lastResult() const { return m_lastResult; }

    [[nodiscard]] const PCGPreviewStats& stats() const { return m_stats; }

    // ── Preview world population (E.2) ────────────────────────────────────
    // Writes generated placements into a NovaForgePreviewWorld.

    uint32_t populatePreviewWorld(NovaForgePreviewWorld& world) const {
        if (!m_resultValid) return 0;
        world.clearEntities();
        uint32_t created = 0;
        for (const auto& p : m_lastResult.placements) {
            EntityId id = world.createEntity(p.assetTag.empty() ? "PCGEntity" : p.assetTag);
            if (id == kInvalidEntityId) break;
            PreviewTransform t;
            t.position = {p.x, p.y, p.z};
            t.rotation = {0.f, p.yaw, 0.f};
            t.scale    = {p.scale, p.scale, p.scale};
            world.setTransform(id, t);
            if (!p.assetTag.empty())   world.setMesh(id, p.assetTag);
            if (!p.materialTag.empty()) world.setMaterial(id, p.materialTag);
            ++created;
        }
        world.clearDirty();
        return created;
    }

    // ── Change notification ───────────────────────────────────────────────
    // Optional callback invoked after each successful regeneration.

    using RegenerateCallback = std::function<void(const PCGGenerationResult&)>;

    void setOnRegenerateCallback(RegenerateCallback cb) {
        m_onRegenerate = std::move(cb);
    }

    // ── Domain override ───────────────────────────────────────────────────

    void setDomainOverride(const std::string& domain) { m_domainOverride = domain; }
    [[nodiscard]] const std::string& domainOverride() const { return m_domainOverride; }

private:
    PCGRuleSet*                 m_ruleSet         = nullptr;
    PCGDeterministicSeedContext m_seedCtx;
    PCGGeneratorService         m_generator;
    PCGGenerationResult         m_lastResult;
    PCGPreviewStats             m_stats;
    bool                        m_resultValid     = false;
    bool                        m_autoRegenerate  = true;
    std::string                 m_domainOverride;
    RegenerateCallback          m_onRegenerate;

    bool regenerate() {
        if (!m_ruleSet) return false;
        m_lastResult  = m_generator.generate(*m_ruleSet, m_seedCtx, m_domainOverride);
        m_resultValid = m_lastResult.success;
        m_stats.generationCount++;
        m_stats.lastPlacementCount = static_cast<uint32_t>(m_lastResult.placements.size());
        m_stats.hasResult          = m_resultValid;
        if (m_resultValid && m_onRegenerate) m_onRegenerate(m_lastResult);
        return m_resultValid;
    }
};

} // namespace NovaForge
