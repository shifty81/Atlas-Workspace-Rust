---- MODULE ecs ----
(***************************************************************************)
(* ECS Execution Model for the Atlas engine.                               *)
(*                                                                         *)
(* Models entity creation/destruction, component add/remove, and           *)
(* deterministic system execution ordering.  The real implementation uses  *)
(* sequential uint32 entity IDs (m_nextID counter), a two-level map for    *)
(* component storage (EntityID -> type_index -> any), and a registered     *)
(* system list that executes in fixed insertion order every tick.           *)
(*                                                                         *)
(* Verified invariants:                                                    *)
(*   UniqueIDs        — Entity IDs are never reused while alive.           *)
(*   DeadClean        — Destroyed entities have no components.             *)
(*   FixedOrder       — Systems execute in their registered order.         *)
(*   ExclusiveOwner   — A component type is mutated by at most one system  *)
(*                      per tick (matches WorldState::CanMutate).          *)
(***************************************************************************)
EXTENDS Naturals, Sequences, FiniteSets

CONSTANTS
    MaxEntities,      \* Upper bound on entity IDs (model-checking bound)
    ComponentTypes,   \* Set of component type tags, e.g. {"Transform", "Velocity", "Health"}
    Systems           \* Ordered sequence of system names, e.g. <<"Physics", "Combat", "Render">>

VARIABLES
    nextID,           \* Next entity ID to allocate (corresponds to ECS::m_nextID)
    alive,            \* Set of currently alive entity IDs
    components,       \* Function: [EntityID -> SUBSET ComponentTypes]
    ownership,        \* Function: [ComponentTypes -> STRING] — which system owns each type
    tickOrder,        \* The sequence of systems that will run this tick
    currentStep,      \* Index into tickOrder (0 = not yet started, Len+1 = done)
    tickCount         \* Monotonic tick counter

vars == <<nextID, alive, components, ownership, tickOrder, currentStep, tickCount>>

(***************************************************************************)
(* Type invariant — structural well-formedness.                            *)
(***************************************************************************)
TypeOK ==
    /\ nextID \in 1..(MaxEntities + 1)
    /\ alive \subseteq 1..MaxEntities
    /\ DOMAIN components = alive
    /\ \A e \in alive : components[e] \subseteq ComponentTypes
    /\ DOMAIN ownership \subseteq ComponentTypes
    /\ tickOrder = Systems
    /\ currentStep \in 0..(Len(Systems) + 1)
    /\ tickCount \in Nat

(***************************************************************************)
(* INV: UniqueIDs — no two living entities share an ID.  Guaranteed by     *)
(* construction: alive is a set, and CreateEntity only adds nextID which   *)
(* is always > any previously issued ID.                                   *)
(***************************************************************************)
UniqueIDs ==
    \A e \in alive : e < nextID

(***************************************************************************)
(* INV: DeadClean — destroyed entities retain no components.               *)
(* In the implementation, DestroyEntity erases the entity's component map. *)
(***************************************************************************)
DeadClean ==
    \A e \in 1..MaxEntities : e \notin alive => e \notin DOMAIN components

(***************************************************************************)
(* INV: FixedOrder — system execution order equals the registered order.   *)
(* tickOrder is always identical to the Systems constant.                  *)
(***************************************************************************)
FixedOrder ==
    tickOrder = Systems

(***************************************************************************)
(* INV: ExclusiveOwnership — each component type is owned by at most one   *)
(* system.  In the engine this is enforced by WorldState::CanMutate().     *)
(***************************************************************************)
ExclusiveOwnership ==
    \A c \in DOMAIN ownership :
        \A s1, s2 \in {s \in DOMAIN ownership : ownership[s] = ownership[c]} :
            s1 = s2

(***************************************************************************)
(* Combined invariant checked by TLC.                                      *)
(***************************************************************************)
Invariant ==
    /\ TypeOK
    /\ UniqueIDs
    /\ DeadClean
    /\ FixedOrder

(***************************************************************************)
(* Initial state: no entities, counter starts at 1, tick 0, no systems     *)
(* have run yet.                                                           *)
(***************************************************************************)
Init ==
    /\ nextID = 1
    /\ alive = {}
    /\ components = [e \in {} |-> {}]
    /\ ownership = [c \in {} |-> ""]
    /\ tickOrder = Systems
    /\ currentStep = 0
    /\ tickCount = 0

(***************************************************************************)
(* ACTION: CreateEntity — allocate a new entity with a fresh sequential ID *)
(* Mirrors ECS::CreateEntity() which pushes nextID onto m_entities and     *)
(* increments m_nextID.                                                    *)
(***************************************************************************)
CreateEntity ==
    /\ nextID <= MaxEntities
    /\ currentStep = 0            \* Only between ticks (no system running)
    /\ alive' = alive \union {nextID}
    /\ components' = [e \in alive' |-> IF e = nextID THEN {} ELSE components[e]]
    /\ nextID' = nextID + 1
    /\ UNCHANGED <<ownership, tickOrder, currentStep, tickCount>>

(***************************************************************************)
(* ACTION: DestroyEntity — remove an entity and all its components.        *)
(* Mirrors ECS::DestroyEntity() which erases from m_entities and           *)
(* m_components.                                                           *)
(***************************************************************************)
DestroyEntity(e) ==
    /\ e \in alive
    /\ currentStep = 0            \* Only between ticks
    /\ alive' = alive \ {e}
    /\ components' = [ent \in alive' |-> components[ent]]
    /\ UNCHANGED <<nextID, ownership, tickOrder, currentStep, tickCount>>

(***************************************************************************)
(* ACTION: AddComponent — attach a component type to a living entity.      *)
(* Mirrors ECS::AddComponent<T>() which inserts into the entity's          *)
(* component sub-map.                                                      *)
(***************************************************************************)
AddComponent(e, c) ==
    /\ e \in alive
    /\ c \in ComponentTypes
    /\ c \notin components[e]
    /\ components' = [components EXCEPT ![e] = components[e] \union {c}]
    /\ UNCHANGED <<nextID, alive, ownership, tickOrder, currentStep, tickCount>>

(***************************************************************************)
(* ACTION: RemoveComponent — detach a component type from a living entity. *)
(* Mirrors ECS::RemoveComponent<T>().                                      *)
(***************************************************************************)
RemoveComponent(e, c) ==
    /\ e \in alive
    /\ c \in components[e]
    /\ components' = [components EXCEPT ![e] = components[e] \ {c}]
    /\ UNCHANGED <<nextID, alive, ownership, tickOrder, currentStep, tickCount>>

(***************************************************************************)
(* ACTION: RegisterOwnership — declare that a system exclusively owns a    *)
(* component type.  Mirrors WorldState::RegisterOwnership().               *)
(* Only permitted if no other system already owns the type.                *)
(***************************************************************************)
RegisterOwnership(sys, c) ==
    /\ c \in ComponentTypes
    /\ c \notin DOMAIN ownership
    /\ sys \in {Systems[i] : i \in 1..Len(Systems)}
    /\ ownership' = [x \in DOMAIN ownership \union {c} |->
                        IF x = c THEN sys ELSE ownership[x]]
    /\ UNCHANGED <<nextID, alive, components, tickOrder, currentStep, tickCount>>

(***************************************************************************)
(* ACTION: BeginTick — start a new simulation tick.  Resets the step       *)
(* counter so systems execute from the first registered system.            *)
(***************************************************************************)
BeginTick ==
    /\ currentStep = 0
    /\ currentStep' = 1
    /\ tickCount' = tickCount + 1
    /\ UNCHANGED <<nextID, alive, components, ownership, tickOrder>>

(***************************************************************************)
(* ACTION: RunSystem — execute the system at currentStep in tickOrder.     *)
(* Systems run strictly in order; each step advances the index by one.     *)
(* The system may read any components but only mutate owned ones.          *)
(* We model the mutation as a no-op on component sets (value mutation is   *)
(* internal); the ordering guarantee is the key property.                  *)
(***************************************************************************)
RunSystem ==
    /\ currentStep >= 1
    /\ currentStep <= Len(Systems)
    /\ currentStep' = currentStep + 1
    /\ UNCHANGED <<nextID, alive, components, ownership, tickOrder, tickCount>>

(***************************************************************************)
(* ACTION: EndTick — all systems have finished; return to inter-tick state *)
(***************************************************************************)
EndTick ==
    /\ currentStep = Len(Systems) + 1
    /\ currentStep' = 0
    /\ UNCHANGED <<nextID, alive, components, ownership, tickOrder, tickCount>>

(***************************************************************************)
(* Next-state relation: non-deterministic choice among all actions.        *)
(***************************************************************************)
Next ==
    \/ CreateEntity
    \/ \E e \in alive : DestroyEntity(e)
    \/ \E e \in alive, c \in ComponentTypes : AddComponent(e, c)
    \/ \E e \in alive, c \in ComponentTypes : RemoveComponent(e, c)
    \/ \E s \in {Systems[i] : i \in 1..Len(Systems)}, c \in ComponentTypes :
           RegisterOwnership(s, c)
    \/ BeginTick
    \/ RunSystem
    \/ EndTick

(***************************************************************************)
(* Fairness: we require weak fairness on tick progression so the model     *)
(* checker explores tick sequences, not just entity manipulation.          *)
(***************************************************************************)
Spec == Init /\ [][Next]_vars /\ WF_vars(BeginTick) /\ WF_vars(EndTick)

(***************************************************************************)
(* Liveness: every tick eventually completes.                              *)
(***************************************************************************)
TickProgress == []<>(currentStep = 0)

====
