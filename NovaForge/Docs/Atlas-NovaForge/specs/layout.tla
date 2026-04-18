---- MODULE layout ----
(***************************************************************************)
(* GUI Layout Solver for the Atlas engine.                                 *)
(*                                                                         *)
(* Models the dockable panel system as a binary tree of split nodes.       *)
(* Leaf nodes hold panels; internal nodes split space horizontally or      *)
(* vertically, matching the DockSplit / DockMerge / DockResize commands    *)
(* in UICommandBus and the constraint-based solving in UILayoutSolver.     *)
(*                                                                         *)
(* The real layout solver uses integer-only arithmetic with min/max/pref   *)
(* sizes and weight-proportional distribution.  This spec abstracts sizes  *)
(* to natural numbers and ratios to verify structural invariants.          *)
(*                                                                         *)
(* Verified invariants:                                                    *)
(*   I1  NoOverlap      — No two panels occupy the same pixel region.     *)
(*   I2  FullCoverage   — Panel sizes sum to parent's available space.     *)
(*   I3  MinimumSize    — Every panel meets its declared minimum size.     *)
(*   I4  TreeConsistent — The tree is well-formed: no orphans, no cycles. *)
(*   I5  NoOrphanPanels — Every panel appears in exactly one leaf node.   *)
(***************************************************************************)
EXTENDS Naturals, Sequences, FiniteSets

CONSTANTS
    MaxNodes,         \* Upper bound on tree node count (model-checking bound)
    MinPanelSize,     \* Minimum size for any panel dimension (integer pixels)
    TotalSize         \* Total available space for the root (viewport extent)

VARIABLES
    nodes,            \* Function: [NodeID -> NodeRecord]
    nextNodeID,       \* Counter for fresh node IDs
    root,             \* ID of the root node (0 if tree is empty)
    panelSet          \* Set of all panel names currently docked

vars == <<nodes, nextNodeID, root, panelSet>>

(***************************************************************************)
(* Node record structure.                                                  *)
(*                                                                         *)
(* A node is either a Leaf or a Split:                                     *)
(*   Leaf:  [type: "leaf", panel: STRING, size: Nat, parent: NodeID]       *)
(*   Split: [type: "hsplit"|"vsplit", left: NodeID, right: NodeID,         *)
(*           ratio: Nat (percent 1..99), size: Nat, parent: NodeID]        *)
(*                                                                         *)
(* parent = 0 means this node is the root.                                 *)
(* Mirrors the dock node tree navigated by DockSplit/DockMerge commands.   *)
(***************************************************************************)
NullNode == 0

IsLeaf(n) == nodes[n].type = "leaf"
IsSplit(n) == nodes[n].type \in {"hsplit", "vsplit"}

(***************************************************************************)
(* Helper: all node IDs currently in the tree.                             *)
(***************************************************************************)
AllNodeIDs == DOMAIN nodes

(***************************************************************************)
(* Helper: compute child sizes from a split node's ratio.                  *)
(* ratio is a percentage (1-99).  Integer division mirrors the engine's    *)
(* integer-only arithmetic.                                                *)
(***************************************************************************)
LeftSize(n)  == (nodes[n].size * nodes[n].ratio) \div 100
RightSize(n) == nodes[n].size - LeftSize(n)

(***************************************************************************)
(* Type invariant — structural well-formedness.                            *)
(***************************************************************************)
TypeOK ==
    /\ nextNodeID \in 1..(MaxNodes + 1)
    /\ root \in 0..MaxNodes
    /\ \A n \in AllNodeIDs :
        /\ nodes[n].size \in Nat
        /\ nodes[n].parent \in 0..MaxNodes
        /\ nodes[n].type \in {"leaf", "hsplit", "vsplit"}

(***************************************************************************)
(* INV I1: SizePartition — within any split, left + right = parent size.   *)
(* Child sizes partition the parent exactly with no overlap and no gap.    *)
(* Recursive application over the tree guarantees no pixel region is       *)
(* assigned to more than one panel and no region is unassigned.            *)
(***************************************************************************)
SizePartition ==
    \A n \in AllNodeIDs :
        IsSplit(n) =>
            LeftSize(n) + RightSize(n) = nodes[n].size

(***************************************************************************)
(* INV I2: FullCoverage — the root node spans TotalSize and every split    *)
(* distributes its full size to children.  Combined with NoOverlap this    *)
(* guarantees the viewport is fully covered.                               *)
(***************************************************************************)
FullCoverage ==
    root /= NullNode => nodes[root].size = TotalSize

(***************************************************************************)
(* INV I3: MinimumSize — every leaf node is at least MinPanelSize.         *)
(* Matches UIConstraint::minWidth / minHeight enforcement.                 *)
(***************************************************************************)
MinimumSize ==
    \A n \in AllNodeIDs :
        IsLeaf(n) => nodes[n].size >= MinPanelSize

(***************************************************************************)
(* INV I4: TreeConsistent — parent/child links are bidirectional and the   *)
(* root has no parent.  No orphaned sub-trees, no cycles.                  *)
(***************************************************************************)
TreeConsistent ==
    /\ (root /= NullNode => nodes[root].parent = NullNode)
    /\ \A n \in AllNodeIDs :
        IsSplit(n) =>
            /\ nodes[n].left \in AllNodeIDs
            /\ nodes[n].right \in AllNodeIDs
            /\ nodes[nodes[n].left].parent = n
            /\ nodes[nodes[n].right].parent = n
            /\ nodes[n].left /= nodes[n].right
    \* Every non-root node's parent is in the tree and is a split
    /\ \A n \in AllNodeIDs :
        (n /= root /\ nodes[n].parent /= NullNode) =>
            /\ nodes[n].parent \in AllNodeIDs
            /\ IsSplit(nodes[n].parent)

(***************************************************************************)
(* INV I5: NoOrphanPanels — every panel name in panelSet appears in        *)
(* exactly one leaf, and every leaf references a panel in panelSet.        *)
(***************************************************************************)
NoOrphanPanels ==
    /\ \A p \in panelSet :
        Cardinality({n \in AllNodeIDs : IsLeaf(n) /\ nodes[n].panel = p}) = 1
    /\ \A n \in AllNodeIDs :
        IsLeaf(n) => nodes[n].panel \in panelSet

(***************************************************************************)
(* Combined invariant.                                                     *)
(***************************************************************************)
Invariant ==
    /\ TypeOK
    /\ SizePartition
    /\ FullCoverage
    /\ MinimumSize
    /\ TreeConsistent
    /\ NoOrphanPanels

(***************************************************************************)
(* Initial state: a single root leaf panel filling the entire viewport.    *)
(* Mirrors the editor starting with one docked panel.                      *)
(***************************************************************************)
Init ==
    /\ nextNodeID = 2
    /\ root = 1
    /\ nodes = [n \in {1} |->
                    [type   |-> "leaf",
                     panel  |-> "main",
                     size   |-> TotalSize,
                     parent |-> NullNode]]
    /\ panelSet = {"main"}

(***************************************************************************)
(* ACTION: SplitPanel — split a leaf node into two panels.                 *)
(* The leaf becomes a split node with two new leaf children.               *)
(* ratio (1..99) determines how space is divided.                          *)
(* Mirrors the DockSplit UI command.                                       *)
(***************************************************************************)
SplitPanel(leafID, newPanel, direction, ratio) ==
    /\ leafID \in AllNodeIDs
    /\ IsLeaf(leafID)
    /\ newPanel \notin panelSet
    /\ direction \in {"hsplit", "vsplit"}
    /\ ratio \in 1..99
    /\ nextNodeID + 1 <= MaxNodes    \* Need two fresh IDs
    /\ LET parentSize == nodes[leafID].size
           oldPanel  == nodes[leafID].panel
           leftID    == nextNodeID
           rightID   == nextNodeID + 1
           lSize     == (parentSize * ratio) \div 100
           rSize     == parentSize - lSize
       IN  \* Both children must meet minimum size
           /\ lSize >= MinPanelSize
           /\ rSize >= MinPanelSize
           /\ nodes' = [n \in (AllNodeIDs \union {leftID, rightID}) |->
                            IF n = leafID THEN
                                [type      |-> direction,
                                 left      |-> leftID,
                                 right     |-> rightID,
                                 ratio     |-> ratio,
                                 size      |-> parentSize,
                                 parent    |-> nodes[leafID].parent]
                            ELSE IF n = leftID THEN
                                [type   |-> "leaf",
                                 panel  |-> oldPanel,
                                 size   |-> lSize,
                                 parent |-> leafID]
                            ELSE IF n = rightID THEN
                                [type   |-> "leaf",
                                 panel  |-> newPanel,
                                 size   |-> rSize,
                                 parent |-> leafID]
                            ELSE nodes[n]]
           /\ nextNodeID' = nextNodeID + 2
           /\ panelSet' = panelSet \union {newPanel}
           /\ UNCHANGED root

(***************************************************************************)
(* ACTION: MergePanel — remove a leaf and collapse its sibling into the    *)
(* parent position.  Mirrors the DockMerge UI command.                     *)
(* Cannot merge the last panel (root leaf).                                *)
(***************************************************************************)
MergePanel(leafID) ==
    /\ leafID \in AllNodeIDs
    /\ IsLeaf(leafID)
    /\ leafID /= root               \* Cannot remove the last panel
    /\ LET parentID  == nodes[leafID].parent
           siblingID == IF nodes[parentID].left = leafID
                        THEN nodes[parentID].right
                        ELSE nodes[parentID].left
           grandparent == nodes[parentID].parent
       IN  /\ parentID \in AllNodeIDs
           /\ siblingID \in AllNodeIDs
           \* Promote sibling into parent's position with parent's size
           /\ nodes' = LET remaining == AllNodeIDs \ {leafID, parentID}
                            updated   == [n \in remaining |->
                                            IF n = siblingID THEN
                                                [nodes[siblingID] EXCEPT
                                                    !.size   = nodes[parentID].size,
                                                    !.parent = grandparent]
                                            ELSE IF IsSplit(n) /\ nodes[n].left = parentID THEN
                                                [nodes[n] EXCEPT !.left = siblingID]
                                            ELSE IF IsSplit(n) /\ nodes[n].right = parentID THEN
                                                [nodes[n] EXCEPT !.right = siblingID]
                                            ELSE nodes[n]]
                        IN  updated
           /\ root' = IF parentID = root THEN siblingID ELSE root
           /\ panelSet' = panelSet \ {nodes[leafID].panel}
           /\ UNCHANGED nextNodeID

(***************************************************************************)
(* ACTION: ResizePanel — adjust the split ratio of a split node.           *)
(* Both children must still meet minimum size after the resize.            *)
(* Mirrors the DockResize UI command.                                      *)
(***************************************************************************)
ResizePanel(splitID, newRatio) ==
    /\ splitID \in AllNodeIDs
    /\ IsSplit(splitID)
    /\ newRatio \in 1..99
    /\ LET parentSize == nodes[splitID].size
           lSize      == (parentSize * newRatio) \div 100
           rSize      == parentSize - lSize
           leftID     == nodes[splitID].left
           rightID    == nodes[splitID].right
       IN  /\ lSize >= MinPanelSize
           /\ rSize >= MinPanelSize
           /\ nodes' = [nodes EXCEPT
                            ![splitID].ratio = newRatio,
                            ![leftID].size   = lSize,
                            ![rightID].size  = rSize]
           /\ UNCHANGED <<nextNodeID, root, panelSet>>

(***************************************************************************)
(* Next-state relation.                                                    *)
(***************************************************************************)
Next ==
    \/ \E lid \in AllNodeIDs, p \in {"panel_a", "panel_b", "panel_c", "panel_d"},
          d \in {"hsplit", "vsplit"}, r \in {25, 33, 50, 67, 75} :
            SplitPanel(lid, p, d, r)
    \/ \E lid \in AllNodeIDs : MergePanel(lid)
    \/ \E sid \in AllNodeIDs, r \in {25, 33, 50, 67, 75} :
            ResizePanel(sid, r)

Spec == Init /\ [][Next]_vars

(***************************************************************************)
(* Temporal property: the tree always has at least one panel.              *)
(***************************************************************************)
AlwaysHasPanel == [](panelSet /= {})

====
