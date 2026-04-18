#include "ProjectNamespaceRewriter.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace atlas::tools {

// ============================================================
// Extension management
// ============================================================

void ProjectNamespaceRewriter::RegisterExtension(const std::string& ext) {
    for (const auto& e : m_extensions)
        if (e == ext) return;
    m_extensions.push_back(ext);
}

size_t ProjectNamespaceRewriter::ExtensionCount() const {
    return m_extensions.size();
}

bool ProjectNamespaceRewriter::ShouldRewrite(const std::string& filename) const {
    for (const auto& ext : m_extensions) {
        if (filename.size() >= ext.size() &&
            filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0)
            return true;
    }
    return false;
}

// ============================================================
// Rule management
// ============================================================

void ProjectNamespaceRewriter::AddRule(const NamespaceRule& rule) {
    m_rules.push_back(rule);
}

void ProjectNamespaceRewriter::AddRule(const std::string& from, const std::string& to) {
    m_rules.push_back({from, to});
}

size_t ProjectNamespaceRewriter::RuleCount() const {
    return m_rules.size();
}

void ProjectNamespaceRewriter::ClearRules() {
    m_rules.clear();
}

std::string ProjectNamespaceRewriter::ApplyRules(const std::string& content,
                                                  uint32_t* substitutionCount) const {
    if (m_rules.empty()) {
        if (substitutionCount) *substitutionCount = 0;
        return content;
    }

    std::string result = content;
    uint32_t total = 0;

    for (const auto& rule : m_rules) {
        if (rule.from.empty() || rule.from == rule.to) continue;
        size_t pos = 0;
        while ((pos = result.find(rule.from, pos)) != std::string::npos) {
            result.replace(pos, rule.from.size(), rule.to);
            pos += rule.to.size();
            ++total;
        }
    }

    if (substitutionCount) *substitutionCount = total;
    return result;
}

// ============================================================
// File rewriting
// ============================================================

RewriteResult ProjectNamespaceRewriter::RewriteFile(const std::string& sourcePath,
                                                     const std::string& outputPath) const {
    RewriteResult result;
    result.sourcePath = sourcePath;
    result.outputPath = outputPath;

    std::ifstream in(sourcePath);
    if (!in.is_open()) {
        result.success = false;
        result.errorMessage = "Cannot open source file: " + sourcePath;
        return result;
    }

    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    in.close();

    uint32_t subCount = 0;
    std::string rewritten = ApplyRules(content, &subCount);

    // Ensure output directory exists.
    std::filesystem::path outPath(outputPath);
    std::error_code ec;
    std::filesystem::create_directories(outPath.parent_path(), ec);

    std::ofstream out(outputPath);
    if (!out.is_open()) {
        result.success = false;
        result.errorMessage = "Cannot open output file: " + outputPath;
        return result;
    }
    out << rewritten;
    out.close();

    result.success = true;
    result.substitutionCount = subCount;
    return result;
}

RewriteRunResult ProjectNamespaceRewriter::RewriteDirectory(const std::string& sourceDir,
                                                             const std::string& outputDir) const {
    RewriteRunResult run;

    std::error_code ec;
    if (!std::filesystem::is_directory(sourceDir, ec)) {
        run.hasErrors = true;
        return run;
    }

    for (auto& entry : std::filesystem::recursive_directory_iterator(sourceDir, ec)) {
        if (!entry.is_regular_file()) continue;

        std::string srcPath = entry.path().string();
        std::string filename = entry.path().filename().string();

        ++run.filesScanned;

        if (!ShouldRewrite(filename)) {
            ++run.filesSkipped;
            continue;
        }

        // Compute relative path and mirror under outputDir.
        auto rel = std::filesystem::relative(entry.path(), sourceDir, ec);
        std::string outPath = (std::filesystem::path(outputDir) / rel).string();

        RewriteResult r = RewriteFile(srcPath, outPath);
        run.fileResults.push_back(r);

        if (r.success) {
            ++run.filesModified;
            run.totalSubstitutions += r.substitutionCount;
        } else {
            run.hasErrors = true;
        }

        if (m_progressCallback) m_progressCallback(r);
    }

    return run;
}

void ProjectNamespaceRewriter::SetProgressCallback(ProgressCallback cb) {
    m_progressCallback = std::move(cb);
}

std::vector<std::string> ProjectNamespaceRewriter::ValidateRules() const {
    std::vector<std::string> errors;
    for (size_t i = 0; i < m_rules.size(); ++i) {
        const auto& r = m_rules[i];
        if (r.from.empty())
            errors.push_back("Rule[" + std::to_string(i) + "]: 'from' is empty");
        else if (r.to.empty())
            errors.push_back("Rule[" + std::to_string(i) + "]: 'to' is empty for '" + r.from + "'");
        else if (r.from == r.to)
            errors.push_back("Rule[" + std::to_string(i) + "]: 'from' == 'to' ('" + r.from + "')");
    }
    return errors;
}

} // namespace atlas::tools
