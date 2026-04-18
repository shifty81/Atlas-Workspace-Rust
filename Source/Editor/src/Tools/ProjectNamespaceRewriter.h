#pragma once
// ============================================================
// Atlas Project Namespace Rewriter
// ============================================================
//
// Provides the C++ counterpart to tools/atlas_init.py.
// Scans a directory tree for Atlas template markers and
// rewrites namespaces, project names, and module identifiers
// so that a fork can have its own isolated identity without
// touching the upstream engine sources.
//
// Only files matching registered extensions are rewritten.
// All changes are written to a separate output directory so
// the source tree remains intact.
//
// See: docs/12_CI_AND_BUILD.md
//      docs/ATLAS_CORE_CONTRACT.md

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace atlas::tools {

/// A single token substitution rule: replace `from` with `to`.
struct NamespaceRule {
    std::string from;
    std::string to;
};

/// Result of rewriting a single file.
struct RewriteResult {
    std::string sourcePath;
    std::string outputPath;
    bool        success = false;
    uint32_t    substitutionCount = 0;
    std::string errorMessage;
};

/// Overall result of a rewrite run.
struct RewriteRunResult {
    uint32_t                  filesScanned = 0;
    uint32_t                  filesModified = 0;
    uint32_t                  filesSkipped = 0;
    uint32_t                  totalSubstitutions = 0;
    bool                      hasErrors = false;
    std::vector<RewriteResult> fileResults;
};

/// Atlas project namespace rewriter.
class ProjectNamespaceRewriter {
public:
    /// Add an extension to the set of files that will be rewritten (e.g. ".cpp", ".h").
    void RegisterExtension(const std::string& ext);

    /// Number of registered extensions.
    size_t ExtensionCount() const;

    /// Whether the given filename should be rewritten.
    bool ShouldRewrite(const std::string& filename) const;

    /// Add a substitution rule.
    void AddRule(const NamespaceRule& rule);
    void AddRule(const std::string& from, const std::string& to);

    /// Number of substitution rules.
    size_t RuleCount() const;

    /// Remove all rules.
    void ClearRules();

    /// Apply all substitution rules to `content` and return the result.
    std::string ApplyRules(const std::string& content, uint32_t* substitutionCount = nullptr) const;

    /// Rewrite a single file from `sourcePath` into `outputPath`.
    RewriteResult RewriteFile(const std::string& sourcePath,
                              const std::string& outputPath) const;

    /// Recursively rewrite all eligible files under `sourceDir` into `outputDir`.
    RewriteRunResult RewriteDirectory(const std::string& sourceDir,
                                      const std::string& outputDir) const;

    /// Set a callback invoked after each file is processed.
    using ProgressCallback = std::function<void(const RewriteResult&)>;
    void SetProgressCallback(ProgressCallback cb);

    /// Validate that all rules have non-empty from/to strings and
    /// that `from != to`.  Returns a list of invalid rule descriptions.
    std::vector<std::string> ValidateRules() const;

private:
    std::vector<std::string>   m_extensions;
    std::vector<NamespaceRule> m_rules;
    ProgressCallback           m_progressCallback;
};

} // namespace atlas::tools
