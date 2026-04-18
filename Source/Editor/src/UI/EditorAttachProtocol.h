#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../../engine/core/PermissionManager.h"

namespace atlas::editor {

/// Attach mode describing how the editor connects to a running session.
enum class AttachMode : uint8_t {
    Standalone,     ///< Editor launches its own runtime instance
    LiveClient,     ///< Editor attaches to a running client process
    HeadlessServer, ///< Editor attaches to a dedicated server over the network
    Replay          ///< Editor opens a replay file for offline inspection
};

/// Current state of the attach connection.
enum class AttachState : uint8_t {
    Disconnected,
    Connecting,
    Connected,
    Error
};

/// Configuration for an attach request.
struct AttachConfig {
    AttachMode mode = AttachMode::Standalone;
    std::string host;         ///< Network address (LiveClient / HeadlessServer)
    uint16_t   port = 0;     ///< Network port
    std::string replayPath;  ///< File path for Replay mode
};

/// Operations that the editor can perform on an attached target.
enum class EditorOperation : uint8_t {
    ViewState,
    InspectEntities,
    ModifyState,
    InjectInput,
    StepSimulation,
    RecordReplay,
    EditAssets,
    RunCI
};

/// Manages the editor's attachment to a target runtime process.
/// The attach protocol allows the editor to observe and (with sufficient
/// permissions) mutate the state of a live client, headless server, or
/// replay session.
class EditorAttachProtocol {
public:
    void Init();

    /// Begin an attach attempt with the given configuration.
    bool Connect(const AttachConfig& config);

    /// Disconnect from the current target.
    void Disconnect();

    AttachMode  GetMode() const;
    AttachState GetState() const;

    /// Human-readable description of the current attach target.
    std::string TargetDescription() const;

    /// Returns true when the editor has established a valid connection
    /// (or is in standalone mode, which is always "connected").
    bool IsConnected() const;

    /// Set the permission tier for this editor session
    void SetPermissionTier(atlas::PermissionTier tier);
    atlas::PermissionTier GetPermissionTier() const;

    /// Check if the current tier allows a specific editor operation
    bool IsOperationAllowed(EditorOperation op) const;

    /// Validate that an operation is allowed; returns true if permitted,
    /// false otherwise. Requires the editor to be connected.
    bool RequestOperation(EditorOperation op) const;

    /// Get list of allowed operations for the current tier
    std::vector<EditorOperation> AllowedOperations() const;

    /// Get a human-readable description of the current permissions
    std::string PermissionDescription() const;

private:
    AttachConfig m_config;
    AttachState  m_state = AttachState::Disconnected;
    atlas::PermissionTier m_permissionTier = atlas::PermissionTier::Developer;
};

} // namespace atlas::editor
