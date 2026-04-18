// S162 editor tests: NetworkSyncEditorV1, RpcInspectorV1, LagCompEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LagCompEditorV1.h"
#include "NF/Editor/RpcInspectorV1.h"
#include "NF/Editor/NetworkSyncEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── NetworkSyncEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("Nsv1SyncProperty validity", "[Editor][S162]") {
    Nsv1SyncProperty p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "Position"; p.sendRate = 20.f;
    REQUIRE(p.isValid());
}

TEST_CASE("Nsv1SyncProperty zero sendRate invalid", "[Editor][S162]") {
    Nsv1SyncProperty p;
    p.id = 1; p.name = "X"; p.sendRate = 0.f;
    REQUIRE(!p.isValid());
}

TEST_CASE("Nsv1SyncGroup validity", "[Editor][S162]") {
    Nsv1SyncGroup g;
    REQUIRE(!g.isValid());
    g.id = 1; g.name = "Transform";
    REQUIRE(g.isValid());
}

TEST_CASE("NetworkSyncEditorV1 addGroup and groupCount", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    REQUIRE(ns.groupCount() == 0);
    Nsv1SyncGroup g; g.id = 1; g.name = "Physics";
    REQUIRE(ns.addGroup(g));
    REQUIRE(ns.groupCount() == 1);
}

TEST_CASE("NetworkSyncEditorV1 addGroup invalid fails", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    Nsv1SyncGroup g;
    REQUIRE(!ns.addGroup(g));
}

TEST_CASE("NetworkSyncEditorV1 addGroup duplicate id fails", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    Nsv1SyncGroup g; g.id = 1; g.name = "A";
    ns.addGroup(g);
    REQUIRE(!ns.addGroup(g));
}

TEST_CASE("NetworkSyncEditorV1 removeGroup", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    Nsv1SyncGroup g; g.id = 2; g.name = "Anim";
    ns.addGroup(g);
    REQUIRE(ns.removeGroup(2));
    REQUIRE(ns.groupCount() == 0);
    REQUIRE(!ns.removeGroup(2));
}

TEST_CASE("NetworkSyncEditorV1 addProperty to group", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    Nsv1SyncGroup g; g.id = 1; g.name = "G";
    ns.addGroup(g);
    Nsv1SyncProperty p; p.id = 1; p.name = "Pos"; p.sendRate = 30.f;
    REQUIRE(ns.addProperty(1, p));
    auto* found = ns.findGroup(1);
    REQUIRE(found != nullptr);
    REQUIRE(found->props.size() == 1);
}

TEST_CASE("NetworkSyncEditorV1 addProperty invalid fails", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    Nsv1SyncGroup g; g.id = 1; g.name = "G";
    ns.addGroup(g);
    Nsv1SyncProperty p;
    REQUIRE(!ns.addProperty(1, p));
}

TEST_CASE("NetworkSyncEditorV1 addProperty unknown group fails", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    Nsv1SyncProperty p; p.id = 1; p.name = "X"; p.sendRate = 10.f;
    REQUIRE(!ns.addProperty(99, p));
}

TEST_CASE("NetworkSyncEditorV1 removeProperty", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    Nsv1SyncGroup g; g.id = 1; g.name = "G";
    ns.addGroup(g);
    Nsv1SyncProperty p; p.id = 2; p.name = "Rot"; p.sendRate = 20.f;
    ns.addProperty(1, p);
    REQUIRE(ns.removeProperty(1, 2));
    auto* found = ns.findGroup(1);
    REQUIRE(found->props.empty());
    REQUIRE(!ns.removeProperty(1, 2));
}

TEST_CASE("NetworkSyncEditorV1 setOwnership", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    Nsv1SyncGroup g; g.id = 1; g.name = "G";
    ns.addGroup(g);
    REQUIRE(ns.setOwnership(1, Nsv1Ownership::Client));
    auto* found = ns.findGroup(1);
    REQUIRE(found->owner == Nsv1Ownership::Client);
}

TEST_CASE("NetworkSyncEditorV1 setOwnership unknown group fails", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    REQUIRE(!ns.setOwnership(99, Nsv1Ownership::Server));
}

TEST_CASE("NetworkSyncEditorV1 onChange fires on addProperty", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    Nsv1SyncGroup g; g.id = 1; g.name = "G";
    ns.addGroup(g);
    uint64_t changedId = 0;
    ns.setOnChange([&](uint64_t id){ changedId = id; });
    Nsv1SyncProperty p; p.id = 1; p.name = "P"; p.sendRate = 20.f;
    ns.addProperty(1, p);
    REQUIRE(changedId == 1);
}

TEST_CASE("NetworkSyncEditorV1 findGroup unknown returns null", "[Editor][S162]") {
    NetworkSyncEditorV1 ns;
    REQUIRE(ns.findGroup(999) == nullptr);
}

// ── RpcInspectorV1 ────────────────────────────────────────────────────────────

TEST_CASE("Riv1RpcDefinition validity", "[Editor][S162]") {
    Riv1RpcDefinition r;
    REQUIRE(!r.isValid());
    r.id = 1; r.name = "DealDamage";
    REQUIRE(r.isValid());
}

TEST_CASE("Riv1CallRecord validity", "[Editor][S162]") {
    Riv1CallRecord rec;
    REQUIRE(!rec.isValid());
    rec.id = 1; rec.rpcId = 2;
    REQUIRE(rec.isValid());
}

TEST_CASE("RpcInspectorV1 addRpc and rpcCount", "[Editor][S162]") {
    RpcInspectorV1 ri;
    REQUIRE(ri.rpcCount() == 0);
    Riv1RpcDefinition r; r.id = 1; r.name = "Heal";
    REQUIRE(ri.addRpc(r));
    REQUIRE(ri.rpcCount() == 1);
}

TEST_CASE("RpcInspectorV1 addRpc invalid fails", "[Editor][S162]") {
    RpcInspectorV1 ri;
    Riv1RpcDefinition r;
    REQUIRE(!ri.addRpc(r));
}

TEST_CASE("RpcInspectorV1 addRpc duplicate id fails", "[Editor][S162]") {
    RpcInspectorV1 ri;
    Riv1RpcDefinition r; r.id = 1; r.name = "A";
    ri.addRpc(r);
    REQUIRE(!ri.addRpc(r));
}

TEST_CASE("RpcInspectorV1 removeRpc", "[Editor][S162]") {
    RpcInspectorV1 ri;
    Riv1RpcDefinition r; r.id = 2; r.name = "Spawn";
    ri.addRpc(r);
    REQUIRE(ri.removeRpc(2));
    REQUIRE(ri.rpcCount() == 0);
    REQUIRE(!ri.removeRpc(2));
}

TEST_CASE("RpcInspectorV1 logCall and callCount", "[Editor][S162]") {
    RpcInspectorV1 ri;
    Riv1CallRecord rec; rec.id = 1; rec.rpcId = 1;
    REQUIRE(ri.logCall(rec));
    REQUIRE(ri.callCount() == 1);
}

TEST_CASE("RpcInspectorV1 logCall invalid fails", "[Editor][S162]") {
    RpcInspectorV1 ri;
    Riv1CallRecord rec;
    REQUIRE(!ri.logCall(rec));
}

TEST_CASE("RpcInspectorV1 logCall fires callback", "[Editor][S162]") {
    RpcInspectorV1 ri;
    uint64_t cbRpcId = 0;
    ri.setOnCall([&](const Riv1CallRecord& r){ cbRpcId = r.rpcId; });
    Riv1CallRecord rec; rec.id = 2; rec.rpcId = 5;
    ri.logCall(rec);
    REQUIRE(cbRpcId == 5);
}

TEST_CASE("RpcInspectorV1 clearLog", "[Editor][S162]") {
    RpcInspectorV1 ri;
    Riv1CallRecord rec; rec.id = 1; rec.rpcId = 1;
    ri.logCall(rec);
    REQUIRE(ri.clearLog());
    REQUIRE(ri.callCount() == 0);
    REQUIRE(!ri.clearLog());
}

TEST_CASE("RpcInspectorV1 findRpc", "[Editor][S162]") {
    RpcInspectorV1 ri;
    Riv1RpcDefinition r; r.id = 3; r.name = "Kill";
    ri.addRpc(r);
    auto* found = ri.findRpc(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->name == "Kill");
}

TEST_CASE("RpcInspectorV1 findRpc unknown returns null", "[Editor][S162]") {
    RpcInspectorV1 ri;
    REQUIRE(ri.findRpc(999) == nullptr);
}

// ── LagCompEditorV1 ───────────────────────────────────────────────────────────

TEST_CASE("Lcv1RewindConfig validity", "[Editor][S162]") {
    Lcv1RewindConfig cfg;
    REQUIRE(cfg.isValid());
    cfg.bufferMs = 0.f;
    REQUIRE(!cfg.isValid());
}

TEST_CASE("Lcv1RewindConfig zero snapshotRate invalid", "[Editor][S162]") {
    Lcv1RewindConfig cfg;
    cfg.snapshotRate = 0;
    REQUIRE(!cfg.isValid());
}

TEST_CASE("Lcv1HitRecord validity", "[Editor][S162]") {
    Lcv1HitRecord r;
    REQUIRE(!r.isValid());
    r.id = 1; r.attackerId = 2; r.targetId = 3;
    REQUIRE(r.isValid());
}

TEST_CASE("LagCompEditorV1 setRewindConfig valid", "[Editor][S162]") {
    LagCompEditorV1 lc;
    Lcv1RewindConfig cfg; cfg.bufferMs = 300.f; cfg.snapshotRate = 30;
    REQUIRE(lc.setRewindConfig(cfg));
    REQUIRE(lc.rewindConfig().bufferMs == Approx(300.f));
}

TEST_CASE("LagCompEditorV1 setRewindConfig invalid fails", "[Editor][S162]") {
    LagCompEditorV1 lc;
    Lcv1RewindConfig cfg; cfg.bufferMs = 0.f;
    REQUIRE(!lc.setRewindConfig(cfg));
}

TEST_CASE("LagCompEditorV1 setHitValidMethod", "[Editor][S162]") {
    LagCompEditorV1 lc;
    lc.setHitValidMethod(Lcv1HitValMethod::ServerSide);
    REQUIRE(lc.hitValMethod() == Lcv1HitValMethod::ServerSide);
}

TEST_CASE("LagCompEditorV1 addHitRecord and hitCount", "[Editor][S162]") {
    LagCompEditorV1 lc;
    Lcv1HitRecord r; r.id = 1; r.attackerId = 2; r.targetId = 3;
    REQUIRE(lc.addHitRecord(r));
    REQUIRE(lc.hitCount() == 1);
}

TEST_CASE("LagCompEditorV1 addHitRecord invalid fails", "[Editor][S162]") {
    LagCompEditorV1 lc;
    Lcv1HitRecord r;
    REQUIRE(!lc.addHitRecord(r));
}

TEST_CASE("LagCompEditorV1 addHitRecord duplicate id fails", "[Editor][S162]") {
    LagCompEditorV1 lc;
    Lcv1HitRecord r; r.id = 1; r.attackerId = 2; r.targetId = 3;
    lc.addHitRecord(r);
    REQUIRE(!lc.addHitRecord(r));
}

TEST_CASE("LagCompEditorV1 addHitRecord fires callback", "[Editor][S162]") {
    LagCompEditorV1 lc;
    uint64_t cbId = 0;
    lc.setOnHit([&](const Lcv1HitRecord& h){ cbId = h.id; });
    Lcv1HitRecord r; r.id = 7; r.attackerId = 1; r.targetId = 2;
    lc.addHitRecord(r);
    REQUIRE(cbId == 7);
}

TEST_CASE("LagCompEditorV1 validateHit", "[Editor][S162]") {
    LagCompEditorV1 lc;
    Lcv1HitRecord r; r.id = 2; r.attackerId = 1; r.targetId = 3;
    lc.addHitRecord(r);
    REQUIRE(lc.validateHit(2, true));
}

TEST_CASE("LagCompEditorV1 validateHit unknown id fails", "[Editor][S162]") {
    LagCompEditorV1 lc;
    REQUIRE(!lc.validateHit(99, true));
}

TEST_CASE("LagCompEditorV1 clearHits", "[Editor][S162]") {
    LagCompEditorV1 lc;
    Lcv1HitRecord r; r.id = 1; r.attackerId = 2; r.targetId = 3;
    lc.addHitRecord(r);
    REQUIRE(lc.clearHits());
    REQUIRE(lc.hitCount() == 0);
    REQUIRE(!lc.clearHits());
}
