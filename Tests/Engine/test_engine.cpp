#include <catch2/catch_test_macros.hpp>
#include "NF/Engine/Engine.h"

// ── EntityManager ────────────────────────────────────────────────

TEST_CASE("EntityManager creates unique IDs", "[Engine][ECS]") {
    NF::EntityManager em;

    auto e1 = em.createEntity();
    auto e2 = em.createEntity();
    auto e3 = em.createEntity();

    REQUIRE(e1 != e2);
    REQUIRE(e2 != e3);
    REQUIRE(e1 != NF::INVALID_ENTITY);
    REQUIRE(em.count() == 3);
}

TEST_CASE("EntityManager destroys entities", "[Engine][ECS]") {
    NF::EntityManager em;

    auto e1 = em.createEntity();
    auto e2 = em.createEntity();
    REQUIRE(em.count() == 2);

    em.destroyEntity(e1);
    REQUIRE(em.count() == 1);

    const auto& alive = em.alive();
    REQUIRE(alive.size() == 1);
    REQUIRE(alive[0] == e2);
}

TEST_CASE("EntityManager handles empty state", "[Engine][ECS]") {
    NF::EntityManager em;
    REQUIRE(em.count() == 0);
    REQUIRE(em.alive().empty());

    // Destroying non-existent entity should not crash
    em.destroyEntity(999);
    REQUIRE(em.count() == 0);
}

TEST_CASE("EntityManager isAlive check", "[Engine][ECS]") {
    NF::EntityManager em;
    auto e = em.createEntity();
    REQUIRE(em.isAlive(e));

    em.destroyEntity(e);
    REQUIRE_FALSE(em.isAlive(e));
    REQUIRE_FALSE(em.isAlive(999));
}

TEST_CASE("EntityManager recycles IDs via free list", "[Engine][ECS]") {
    NF::EntityManager em;
    auto e1 = em.createEntity();
    auto e2 = em.createEntity();

    em.destroyEntity(e1);
    REQUIRE(em.count() == 1);

    auto e3 = em.createEntity();
    REQUIRE(em.count() == 2);
    // e3 should reuse e1's ID from the free list
    REQUIRE(e3 == e1);
}

TEST_CASE("EntityManager clear resets state", "[Engine][ECS]") {
    NF::EntityManager em;
    em.createEntity();
    em.createEntity();
    em.createEntity();
    REQUIRE(em.count() == 3);

    em.clear();
    REQUIRE(em.count() == 0);
    REQUIRE(em.alive().empty());
}

// ── ComponentStore ───────────────────────────────────────────────

struct Position { float x = 0.f, y = 0.f, z = 0.f; };
struct Velocity { float dx = 0.f, dy = 0.f, dz = 0.f; };
struct Health   { int hp = 100; };

TEST_CASE("ComponentStore add and get", "[Engine][ECS]") {
    NF::ComponentStore cs;

    cs.add<Position>(1, {10.f, 20.f, 30.f});
    cs.add<Health>(1, {75});

    auto* pos = cs.get<Position>(1);
    REQUIRE(pos != nullptr);
    REQUIRE(pos->x == 10.f);
    REQUIRE(pos->y == 20.f);

    auto* hp = cs.get<Health>(1);
    REQUIRE(hp != nullptr);
    REQUIRE(hp->hp == 75);

    auto* vel = cs.get<Velocity>(1);
    REQUIRE(vel == nullptr);
}

TEST_CASE("ComponentStore has check", "[Engine][ECS]") {
    NF::ComponentStore cs;

    cs.add<Position>(1, {1.f, 2.f, 3.f});
    REQUIRE(cs.has<Position>(1));
    REQUIRE_FALSE(cs.has<Velocity>(1));
    REQUIRE_FALSE(cs.has<Position>(999));
}

TEST_CASE("ComponentStore remove", "[Engine][ECS]") {
    NF::ComponentStore cs;

    cs.add<Position>(1, {1.f, 2.f, 3.f});
    REQUIRE(cs.has<Position>(1));

    cs.remove<Position>(1);
    REQUIRE_FALSE(cs.has<Position>(1));
    REQUIRE(cs.get<Position>(1) == nullptr);
}

TEST_CASE("ComponentStore removeAll for entity", "[Engine][ECS]") {
    NF::ComponentStore cs;

    cs.add<Position>(1, {1.f, 2.f, 3.f});
    cs.add<Health>(1, {50});
    cs.add<Position>(2, {5.f, 6.f, 7.f});

    cs.removeAll(1);

    REQUIRE_FALSE(cs.has<Position>(1));
    REQUIRE_FALSE(cs.has<Health>(1));
    REQUIRE(cs.has<Position>(2)); // entity 2 unaffected
}

TEST_CASE("ComponentStore getAll returns typed map", "[Engine][ECS]") {
    NF::ComponentStore cs;

    cs.add<Position>(1, {1.f, 0.f, 0.f});
    cs.add<Position>(2, {2.f, 0.f, 0.f});
    cs.add<Position>(3, {3.f, 0.f, 0.f});

    auto& all = cs.getAll<Position>();
    REQUIRE(all.size() == 3);
    REQUIRE(all[2].x == 2.f);
}

// ── Level ────────────────────────────────────────────────────────

TEST_CASE("Level spawn and despawn entities", "[Engine][Level]") {
    NF::Level level("TestLevel");
    REQUIRE(level.name() == "TestLevel");

    auto e1 = level.spawnEntity();
    auto e2 = level.spawnEntity();
    REQUIRE(level.entities().count() == 2);

    level.addComponent<Position>(e1, {1.f, 2.f, 3.f});
    REQUIRE(level.getComponent<Position>(e1) != nullptr);

    level.despawnEntity(e1);
    REQUIRE(level.entities().count() == 1);
    REQUIRE_FALSE(level.entities().isAlive(e1));
}

// ── Behavior Tree ────────────────────────────────────────────────

TEST_CASE("BTAction returns status from function", "[Engine][BT]") {
    int callCount = 0;
    NF::BTAction action([&](float) -> NF::BTStatus {
        callCount++;
        return NF::BTStatus::Success;
    });

    auto status = action.tick(0.016f);
    REQUIRE(status == NF::BTStatus::Success);
    REQUIRE(callCount == 1);
}

TEST_CASE("BTSequence runs children in order", "[Engine][BT]") {
    std::vector<int> order;

    NF::BTSequence seq;
    seq.addChild(std::make_unique<NF::BTAction>([&](float) -> NF::BTStatus {
        order.push_back(1);
        return NF::BTStatus::Success;
    }));
    seq.addChild(std::make_unique<NF::BTAction>([&](float) -> NF::BTStatus {
        order.push_back(2);
        return NF::BTStatus::Success;
    }));
    seq.addChild(std::make_unique<NF::BTAction>([&](float) -> NF::BTStatus {
        order.push_back(3);
        return NF::BTStatus::Success;
    }));

    auto status = seq.tick(0.016f);
    REQUIRE(status == NF::BTStatus::Success);
    REQUIRE(order == std::vector<int>{1, 2, 3});
}

TEST_CASE("BTSequence fails on first failure", "[Engine][BT]") {
    int reached = 0;

    NF::BTSequence seq;
    seq.addChild(std::make_unique<NF::BTAction>([&](float) -> NF::BTStatus {
        reached = 1;
        return NF::BTStatus::Success;
    }));
    seq.addChild(std::make_unique<NF::BTAction>([&](float) -> NF::BTStatus {
        reached = 2;
        return NF::BTStatus::Failure;
    }));
    seq.addChild(std::make_unique<NF::BTAction>([&](float) -> NF::BTStatus {
        reached = 3;
        return NF::BTStatus::Success;
    }));

    auto status = seq.tick(0.016f);
    REQUIRE(status == NF::BTStatus::Failure);
    REQUIRE(reached == 2);
}

TEST_CASE("BTSelector succeeds on first success", "[Engine][BT]") {
    int reached = 0;

    NF::BTSelector sel;
    sel.addChild(std::make_unique<NF::BTAction>([&](float) -> NF::BTStatus {
        reached = 1;
        return NF::BTStatus::Failure;
    }));
    sel.addChild(std::make_unique<NF::BTAction>([&](float) -> NF::BTStatus {
        reached = 2;
        return NF::BTStatus::Success;
    }));
    sel.addChild(std::make_unique<NF::BTAction>([&](float) -> NF::BTStatus {
        reached = 3;
        return NF::BTStatus::Success;
    }));

    auto status = sel.tick(0.016f);
    REQUIRE(status == NF::BTStatus::Success);
    REQUIRE(reached == 2);
}

TEST_CASE("BTSelector fails when all children fail", "[Engine][BT]") {
    NF::BTSelector sel;
    sel.addChild(std::make_unique<NF::BTAction>([](float) { return NF::BTStatus::Failure; }));
    sel.addChild(std::make_unique<NF::BTAction>([](float) { return NF::BTStatus::Failure; }));

    auto status = sel.tick(0.016f);
    REQUIRE(status == NF::BTStatus::Failure);
}

// ── AssetHandle ──────────────────────────────────────────────────

TEST_CASE("AssetHandle default state", "[Engine][Asset]") {
    NF::AssetHandle<int> handle(NF::StringID("test_asset"));
    REQUIRE(handle.id() == NF::StringID("test_asset"));
    REQUIRE(handle.state() == NF::AssetState::Unloaded);
    REQUIRE_FALSE(handle.isLoaded());
    REQUIRE(handle.get() == nullptr);
}

TEST_CASE("AssetHandle load and access data", "[Engine][Asset]") {
    NF::AssetHandle<std::string> handle(NF::StringID("greeting"));
    handle.setData(std::make_unique<std::string>("Hello, Atlas!"));

    REQUIRE(handle.isLoaded());
    REQUIRE(handle.state() == NF::AssetState::Loaded);
    REQUIRE(*handle.get() == "Hello, Atlas!");
}

// ── Scene Graph ──────────────────────────────────────────────────

TEST_CASE("SceneGraph parent-child relationships", "[Engine][SceneGraph]") {
    NF::SceneGraph graph;

    graph.setParent(2, 1); // entity 2 is child of entity 1
    graph.setParent(3, 1); // entity 3 is child of entity 1

    REQUIRE(graph.parent(2) == 1);
    REQUIRE(graph.parent(3) == 1);
    REQUIRE(graph.parent(1) == NF::INVALID_ENTITY);
    REQUIRE(graph.isRoot(1));
    REQUIRE_FALSE(graph.isRoot(2));
    REQUIRE(graph.childCount(1) == 2);
}

TEST_CASE("SceneGraph removeFromParent", "[Engine][SceneGraph]") {
    NF::SceneGraph graph;

    graph.setParent(2, 1);
    graph.setParent(3, 1);
    REQUIRE(graph.childCount(1) == 2);

    graph.removeFromParent(2);
    REQUIRE(graph.childCount(1) == 1);
    REQUIRE(graph.isRoot(2));
}

TEST_CASE("SceneGraph reparent", "[Engine][SceneGraph]") {
    NF::SceneGraph graph;

    graph.setParent(2, 1);
    REQUIRE(graph.parent(2) == 1);

    graph.setParent(2, 3); // reparent to entity 3
    REQUIRE(graph.parent(2) == 3);
    REQUIRE(graph.childCount(1) == 0);
    REQUIRE(graph.childCount(3) == 1);
}

TEST_CASE("SceneGraph isDescendantOf", "[Engine][SceneGraph]") {
    NF::SceneGraph graph;

    graph.setParent(2, 1);
    graph.setParent(3, 2);
    graph.setParent(4, 3);

    REQUIRE(graph.isDescendantOf(4, 1));
    REQUIRE(graph.isDescendantOf(3, 1));
    REQUIRE(graph.isDescendantOf(4, 2));
    REQUIRE_FALSE(graph.isDescendantOf(1, 4));
    REQUIRE_FALSE(graph.isDescendantOf(1, 2));
}

TEST_CASE("SceneGraph removeEntity reparents children", "[Engine][SceneGraph]") {
    NF::SceneGraph graph;

    graph.setParent(2, 1);
    graph.setParent(3, 2);  // 1 -> 2 -> 3
    graph.setParent(4, 2);  // 1 -> 2 -> 4

    graph.removeEntity(2);
    // Children 3 and 4 should now be children of 1 (grandparent)
    REQUIRE(graph.parent(3) == 1);
    REQUIRE(graph.parent(4) == 1);
    REQUIRE(graph.childCount(1) == 2);
}

// ── SystemRegistry ───────────────────────────────────────────────

class TestSystem : public NF::SystemBase {
public:
    int updateCount = 0;
    bool initialized = false;
    bool shut = false;

    void init(NF::EntityManager& em, NF::ComponentStore& cs) override {
        SystemBase::init(em, cs);
        initialized = true;
    }
    void update(float dt) override { updateCount++; (void)dt; }
    void shutdown() override { shut = true; }
    std::string_view name() const override { return "TestSystem"; }
};

TEST_CASE("SystemRegistry add and update systems", "[Engine][Systems]") {
    NF::SystemRegistry registry;
    NF::EntityManager em;
    NF::ComponentStore cs;

    auto& sys = registry.addSystem<TestSystem>();
    REQUIRE(registry.count() == 1);

    registry.initAll(em, cs);
    REQUIRE(sys.initialized);

    registry.updateAll(0.016f);
    registry.updateAll(0.016f);
    REQUIRE(sys.updateCount == 2);

    registry.shutdownAll();
    REQUIRE(sys.shut);
}
