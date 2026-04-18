#include <iostream>
#include <cassert>
#include "graphvm/CollaborativeEditor.h"

using namespace atlas::graphvm;

void test_collab_add_peer() {
    CollaborativeEditor editor;
    editor.AddPeer(1, "Alice");
    editor.AddPeer(2, "Bob");
    assert(editor.PeerCount() == 2);
    auto* peer = editor.GetPeer(1);
    assert(peer != nullptr);
    assert(peer->name == "Alice");
    std::cout << "[PASS] test_collab_add_peer" << std::endl;
}

void test_collab_remove_peer() {
    CollaborativeEditor editor;
    editor.AddPeer(1, "Alice");
    editor.AddPeer(2, "Bob");
    editor.RemovePeer(1);
    assert(editor.PeerCount() == 1);
    assert(editor.GetPeer(1) == nullptr);
    assert(editor.GetPeer(2) != nullptr);
    std::cout << "[PASS] test_collab_remove_peer" << std::endl;
}

void test_collab_cursor_update() {
    CollaborativeEditor editor;
    editor.AddPeer(1, "Alice");
    CursorPosition pos;
    pos.nodeID = 42;
    pos.x = 100.0f;
    pos.y = 200.0f;
    pos.timestamp = 1000;
    editor.UpdateCursor(1, pos);
    auto cursor = editor.GetCursor(1);
    assert(cursor.nodeID == 42);
    assert(cursor.x == 100.0f);
    assert(cursor.y == 200.0f);
    std::cout << "[PASS] test_collab_cursor_update" << std::endl;
}

void test_collab_submit_operation() {
    CollaborativeEditor editor;
    EditOperation op;
    op.peerID = 1;
    op.type = EditOpType::AddNode;
    op.targetNodeID = 10;
    op.timestamp = 100;
    editor.SubmitOperation(op);
    assert(editor.OperationCount() == 1);
    assert(editor.OperationLog()[0].sequenceNumber == 1);
    std::cout << "[PASS] test_collab_submit_operation" << std::endl;
}

void test_collab_receive_remote() {
    CollaborativeEditor editor;
    EditOperation op;
    op.peerID = 2;
    op.type = EditOpType::MoveNode;
    op.targetNodeID = 5;
    op.timestamp = 200;
    editor.ReceiveRemoteOperation(op);
    assert(editor.OperationCount() == 1);
    std::cout << "[PASS] test_collab_receive_remote" << std::endl;
}

void test_collab_detect_conflicts() {
    CollaborativeEditor editor;
    EditOperation local;
    local.peerID = 1;
    local.type = EditOpType::ModifyProperty;
    local.targetNodeID = 10;
    local.timestamp = 100;
    editor.SubmitOperation(local);

    EditOperation remote;
    remote.peerID = 2;
    remote.type = EditOpType::ModifyProperty;
    remote.targetNodeID = 10;
    remote.timestamp = 110;
    editor.ReceiveRemoteOperation(remote);

    auto conflicts = editor.DetectConflicts();
    assert(conflicts.size() == 1);
    assert(conflicts[0].type == ConflictType::ConcurrentModify);
    std::cout << "[PASS] test_collab_detect_conflicts" << std::endl;
}

void test_collab_resolve_last_writer_wins() {
    CollaborativeEditor editor;
    editor.SetResolutionStrategy(ResolutionStrategy::LastWriterWins);

    EditOperation local;
    local.peerID = 1;
    local.type = EditOpType::ModifyProperty;
    local.targetNodeID = 10;
    local.timestamp = 100;
    local.propertyValue = "local_val";
    editor.SubmitOperation(local);

    EditOperation remote;
    remote.peerID = 2;
    remote.type = EditOpType::ModifyProperty;
    remote.targetNodeID = 10;
    remote.timestamp = 200;
    remote.propertyValue = "remote_val";
    editor.ReceiveRemoteOperation(remote);

    auto resolved = editor.ResolveConflicts();
    assert(resolved.size() == 1);
    assert(resolved[0].propertyValue == "remote_val");
    std::cout << "[PASS] test_collab_resolve_last_writer_wins" << std::endl;
}

void test_collab_resolve_first_writer_wins() {
    CollaborativeEditor editor;
    editor.SetResolutionStrategy(ResolutionStrategy::FirstWriterWins);

    EditOperation local;
    local.peerID = 1;
    local.type = EditOpType::ModifyProperty;
    local.targetNodeID = 10;
    local.timestamp = 100;
    local.propertyValue = "local_val";
    editor.SubmitOperation(local);

    EditOperation remote;
    remote.peerID = 2;
    remote.type = EditOpType::ModifyProperty;
    remote.targetNodeID = 10;
    remote.timestamp = 200;
    remote.propertyValue = "remote_val";
    editor.ReceiveRemoteOperation(remote);

    auto resolved = editor.ResolveConflicts();
    assert(resolved.size() == 1);
    assert(resolved[0].propertyValue == "local_val");
    std::cout << "[PASS] test_collab_resolve_first_writer_wins" << std::endl;
}

void test_collab_no_conflicts() {
    CollaborativeEditor editor;
    EditOperation local;
    local.peerID = 1;
    local.type = EditOpType::AddNode;
    local.targetNodeID = 10;
    editor.SubmitOperation(local);

    EditOperation remote;
    remote.peerID = 2;
    remote.type = EditOpType::AddNode;
    remote.targetNodeID = 20;
    editor.ReceiveRemoteOperation(remote);

    assert(editor.ConflictCount() == 0);
    std::cout << "[PASS] test_collab_no_conflicts" << std::endl;
}

void test_collab_clear() {
    CollaborativeEditor editor;
    editor.AddPeer(1, "Alice");
    EditOperation op;
    op.peerID = 1;
    op.type = EditOpType::AddNode;
    editor.SubmitOperation(op);
    editor.Clear();
    assert(editor.PeerCount() == 0);
    assert(editor.OperationCount() == 0);
    std::cout << "[PASS] test_collab_clear" << std::endl;
}
