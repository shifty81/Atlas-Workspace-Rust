#pragma once
// NovaForge::DocumentSavePipeline — orchestrates document save/apply with validation
// and notification bus integration.
//
// Responsibilities:
//   - Validate the document panel before saving
//   - Write the document to its file path in the project data directory
//   - Clear dirty state on success
//   - Push validation failure messages to an optional notification callback
//   - Support undo/redo stack per document
//
// Phase C.4 — Panel Save/Apply Pipeline

#include "NovaForge/EditorAdapter/IDocumentPanel.h"
#include "NovaForge/EditorAdapter/NovaForgeDocument.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

namespace NovaForge {

// ── SaveResult ───────────────────────────────────────────────────────────

enum class SaveResultStatus : uint8_t {
    Ok              = 0,
    NoDocument      = 1,  // panel has no bound document
    ValidationFailed = 2,  // one or more validation errors blocked save
    WriteError      = 3,  // file system write failed
};

struct SaveResult {
    SaveResultStatus                         status  = SaveResultStatus::Ok;
    std::vector<DocumentPanelValidationMessage> messages;

    [[nodiscard]] bool ok() const { return status == SaveResultStatus::Ok; }
    [[nodiscard]] bool hasErrors() const {
        for (const auto& m : messages)
            if (m.severity == DocumentPanelValidationSeverity::Error) return true;
        return false;
    }
};

// ── DocumentSavePipeline ─────────────────────────────────────────────────

class DocumentSavePipeline {
public:
    // Optional notification sink: called with severity + message text when
    // a validation error or save error occurs.
    using NotificationCallback = std::function<void(const std::string& severity,
                                                    const std::string& message)>;

    void setNotificationCallback(NotificationCallback cb) {
        m_notify = std::move(cb);
    }

    // ── Save ─────────────────────────────────────────────────────────
    // Validate → write → clear dirty.
    // Returns SaveResult describing outcome.
    SaveResult save(IDocumentPanel& panel) {
        SaveResult result;

        // 1. Document must be bound
        if (!panel.hasDocument()) {
            result.status = SaveResultStatus::NoDocument;
            notify("error", "[DocumentSavePipeline] No document bound to panel '" +
                            panel.panelId() + "'");
            return result;
        }

        // 2. Validate
        result.messages = panel.validate();
        if (result.hasErrors()) {
            result.status = SaveResultStatus::ValidationFailed;
            for (const auto& m : result.messages) {
                if (m.severity == DocumentPanelValidationSeverity::Error) {
                    notify("error", "[Validation] " + m.field + ": " + m.message);
                } else if (m.severity == DocumentPanelValidationSeverity::Warning) {
                    notify("warning", "[Validation] " + m.field + ": " + m.message);
                }
            }
            return result;
        }

        // 3. Write document
        NovaForgeDocument* doc = panel.boundDocument();
        if (!writeDocument(*doc)) {
            result.status = SaveResultStatus::WriteError;
            notify("error", "[DocumentSavePipeline] Write failed for '" +
                            doc->filePath() + "'");
            return result;
        }

        // 4. Clear dirty + undo stack
        doc->clearDirty();

        result.status = SaveResultStatus::Ok;
        notify("info", "[DocumentSavePipeline] Saved '" + doc->filePath() + "'");
        return result;
    }

    // ── Revert ───────────────────────────────────────────────────────
    // Reload document from disk and notify the panel.
    bool revert(IDocumentPanel& panel) {
        if (!panel.hasDocument()) return false;
        bool ok = panel.revert();
        if (ok) {
            notify("info", "[DocumentSavePipeline] Reverted '" +
                           panel.boundDocument()->filePath() + "'");
        } else {
            notify("error", "[DocumentSavePipeline] Revert failed for panel '" +
                            panel.panelId() + "'");
        }
        return ok;
    }

    // ── Validate only ─────────────────────────────────────────────────
    std::vector<DocumentPanelValidationMessage> validateOnly(const IDocumentPanel& panel) {
        auto msgs = panel.validate();
        for (const auto& m : msgs) {
            if (m.severity == DocumentPanelValidationSeverity::Error) {
                notify("error", "[Validation] " + m.field + ": " + m.message);
            } else if (m.severity == DocumentPanelValidationSeverity::Warning) {
                notify("warning", "[Validation] " + m.field + ": " + m.message);
            }
        }
        return msgs;
    }

private:
    // Write the document's current state to its filePath as a minimal JSON stub.
    // Phase D will implement full JSON serialization; this creates the file and
    // ensures the directory exists so the pipeline is exercisable in tests.
    bool writeDocument(const NovaForgeDocument& doc) {
        namespace fs = std::filesystem;
        try {
            fs::path filePath(doc.filePath());
            if (filePath.empty()) return false;

            // Ensure parent directory exists
            auto parent = filePath.parent_path();
            if (!parent.empty() && !fs::exists(parent)) {
                fs::create_directories(parent);
            }

            // Write a minimal JSON envelope
            std::ofstream out(filePath);
            if (!out.is_open()) return false;
            out << "{\n"
                << "  \"schema\": \"novaforge.document.v1\",\n"
                << "  \"type\": \""
                << documentTypeName(doc.type())
                << "\",\n"
                << "  \"displayName\": \""
                << doc.displayName()
                << "\"\n"
                << "}\n";
            return out.good();
        } catch (...) {
            return false;
        }
    }

    void notify(const std::string& severity, const std::string& message) {
        if (m_notify) m_notify(severity, message);
    }

    NotificationCallback m_notify;
};

} // namespace NovaForge
