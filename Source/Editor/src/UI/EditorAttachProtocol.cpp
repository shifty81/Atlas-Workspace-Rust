#include "EditorAttachProtocol.h"

namespace atlas::editor {

void EditorAttachProtocol::Init() {
    m_state = AttachState::Disconnected;
    m_config = AttachConfig{};
}

bool EditorAttachProtocol::Connect(const AttachConfig& config) {
    m_config = config;

    switch (config.mode) {
        case AttachMode::Standalone:
            m_state = AttachState::Connected;
            return true;

        case AttachMode::LiveClient:
            if (config.host.empty() || config.port == 0) {
                m_state = AttachState::Error;
                return false;
            }
            // All permission tiers can connect; operation-level enforcement
            // is handled by RequestOperation() / IsOperationAllowed().
            m_state = AttachState::Connected;
            return true;

        case AttachMode::HeadlessServer:
            if (config.host.empty() || config.port == 0) {
                m_state = AttachState::Error;
                return false;
            }
            // All permission tiers can connect; operation-level enforcement
            // is handled by RequestOperation() / IsOperationAllowed().
            m_state = AttachState::Connected;
            return true;

        case AttachMode::Replay:
            if (config.replayPath.empty()) {
                m_state = AttachState::Error;
                return false;
            }
            m_state = AttachState::Connected;
            return true;
    }

    m_state = AttachState::Error;
    return false;
}

void EditorAttachProtocol::Disconnect() {
    m_state = AttachState::Disconnected;
    m_config = AttachConfig{};
}

AttachMode EditorAttachProtocol::GetMode() const {
    return m_config.mode;
}

AttachState EditorAttachProtocol::GetState() const {
    return m_state;
}

std::string EditorAttachProtocol::TargetDescription() const {
    switch (m_config.mode) {
        case AttachMode::Standalone:
            return "Standalone";
        case AttachMode::LiveClient:
            return "LiveClient " + m_config.host + ":" + std::to_string(m_config.port);
        case AttachMode::HeadlessServer:
            return "HeadlessServer " + m_config.host + ":" + std::to_string(m_config.port);
        case AttachMode::Replay:
            return "Replay " + m_config.replayPath;
    }
    return "Unknown";
}

bool EditorAttachProtocol::IsConnected() const {
    return m_state == AttachState::Connected;
}

bool EditorAttachProtocol::RequestOperation(EditorOperation op) const {
    if (!IsConnected()) return false;

    // Check permission tier
    if (!IsOperationAllowed(op)) return false;

    // Mode-specific restrictions
    switch (m_config.mode) {
        case AttachMode::Replay:
            // Replay mode is read-only — no modification or input injection
            if (op == EditorOperation::ModifyState ||
                op == EditorOperation::InjectInput ||
                op == EditorOperation::EditAssets ||
                op == EditorOperation::RunCI) {
                return false;
            }
            break;

        case AttachMode::HeadlessServer:
            // HeadlessServer doesn't support stepping simulation locally
            if (op == EditorOperation::StepSimulation) {
                return false;
            }
            break;

        default:
            break;
    }

    return true;
}

void EditorAttachProtocol::SetPermissionTier(atlas::PermissionTier tier) {
    m_permissionTier = tier;
}

atlas::PermissionTier EditorAttachProtocol::GetPermissionTier() const {
    return m_permissionTier;
}

bool EditorAttachProtocol::IsOperationAllowed(EditorOperation op) const {
    switch (m_permissionTier) {
        case atlas::PermissionTier::ViewOnly:
            return op == EditorOperation::ViewState ||
                   op == EditorOperation::InspectEntities;

        case atlas::PermissionTier::QA:
            return op == EditorOperation::ViewState ||
                   op == EditorOperation::InspectEntities ||
                   op == EditorOperation::RecordReplay;

        case atlas::PermissionTier::Developer:
            return op != EditorOperation::RunCI;

        case atlas::PermissionTier::CI:
            return op == EditorOperation::ViewState ||
                   op == EditorOperation::InspectEntities ||
                   op == EditorOperation::RunCI;

        case atlas::PermissionTier::Admin:
            return true;
    }
    return false;
}

std::vector<EditorOperation> EditorAttachProtocol::AllowedOperations() const {
    std::vector<EditorOperation> ops;
    static const EditorOperation all[] = {
        EditorOperation::ViewState,
        EditorOperation::InspectEntities,
        EditorOperation::ModifyState,
        EditorOperation::InjectInput,
        EditorOperation::StepSimulation,
        EditorOperation::RecordReplay,
        EditorOperation::EditAssets,
        EditorOperation::RunCI
    };
    for (auto op : all) {
        if (IsOperationAllowed(op)) {
            ops.push_back(op);
        }
    }
    return ops;
}

std::string EditorAttachProtocol::PermissionDescription() const {
    switch (m_permissionTier) {
        case atlas::PermissionTier::ViewOnly:
            return "ViewOnly: can view state and inspect entities";
        case atlas::PermissionTier::QA:
            return "QA: can view, inspect, and record replays";
        case atlas::PermissionTier::Developer:
            return "Developer: all operations except CI";
        case atlas::PermissionTier::CI:
            return "CI: can view, inspect, and run CI";
        case atlas::PermissionTier::Admin:
            return "Admin: full access to all operations";
    }
    return "Unknown";
}

} // namespace atlas::editor
