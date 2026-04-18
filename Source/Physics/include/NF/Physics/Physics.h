#pragma once
// NF::Physics — Rigid bodies, collision detection, character controller
#include "NF/Core/Core.h"
#include <optional>

namespace NF {

// ── AABB ─────────────────────────────────────────────────────────

struct AABB {
    Vec3 min, max;

    bool intersects(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }

    bool contains(const Vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    Vec3 center() const {
        return {(min.x + max.x) * 0.5f,
                (min.y + max.y) * 0.5f,
                (min.z + max.z) * 0.5f};
    }

    Vec3 extents() const {
        return {(max.x - min.x) * 0.5f,
                (max.y - min.y) * 0.5f,
                (max.z - min.z) * 0.5f};
    }

    AABB merged(const AABB& other) const {
        return {
            {std::min(min.x, other.min.x), std::min(min.y, other.min.y), std::min(min.z, other.min.z)},
            {std::max(max.x, other.max.x), std::max(max.y, other.max.y), std::max(max.z, other.max.z)}
        };
    }
};

// ── Ray ──────────────────────────────────────────────────────────

struct Ray {
    Vec3 origin;
    Vec3 direction; // should be normalized

    Vec3 pointAt(float t) const {
        return origin + direction * t;
    }
};

// ── Ray-AABB intersection ────────────────────────────────────────

struct RayHit {
    float distance = 0.f;
    Vec3 point;
    Vec3 normal;
};

inline std::optional<RayHit> rayIntersectAABB(const Ray& ray, const AABB& box) {
    float tmin = -1e30f, tmax = 1e30f;
    Vec3 normal;

    auto slabTest = [&](float origin, float dir, float bmin, float bmax,
                         const Vec3& nLow, const Vec3& nHigh) -> bool {
        if (std::abs(dir) < 1e-8f) {
            return origin >= bmin && origin <= bmax;
        }
        float t1 = (bmin - origin) / dir;
        float t2 = (bmax - origin) / dir;
        Vec3 n1 = nLow, n2 = nHigh;
        if (t1 > t2) { std::swap(t1, t2); std::swap(n1, n2); }
        if (t1 > tmin) { tmin = t1; normal = n1; }
        if (t2 < tmax) { tmax = t2; }
        return tmin <= tmax;
    };

    if (!slabTest(ray.origin.x, ray.direction.x, box.min.x, box.max.x,
                   {-1,0,0}, {1,0,0})) return std::nullopt;
    if (!slabTest(ray.origin.y, ray.direction.y, box.min.y, box.max.y,
                   {0,-1,0}, {0,1,0})) return std::nullopt;
    if (!slabTest(ray.origin.z, ray.direction.z, box.min.z, box.max.z,
                   {0,0,-1}, {0,0,1})) return std::nullopt;

    if (tmax < 0.f) return std::nullopt;
    float t = (tmin >= 0.f) ? tmin : tmax;
    return RayHit{t, ray.pointAt(t), normal};
}

// ── Sphere ───────────────────────────────────────────────────────

struct Sphere {
    Vec3 center;
    float radius = 1.f;

    bool contains(const Vec3& point) const {
        return (point - center).lengthSq() <= radius * radius;
    }

    bool intersects(const Sphere& other) const {
        float dist = (center - other.center).length();
        return dist <= radius + other.radius;
    }
};

// ── Rigid Body ───────────────────────────────────────────────────

enum class BodyType : uint8_t {
    Static,
    Dynamic,
    Kinematic
};

struct RigidBody {
    BodyType type = BodyType::Dynamic;
    float mass = 1.f;
    float restitution = 0.3f;
    float friction = 0.5f;
    Vec3 velocity;
    Vec3 acceleration;
    Vec3 force;
    bool useGravity = true;

    [[nodiscard]] float inverseMass() const {
        return (type == BodyType::Static || mass <= 0.f) ? 0.f : 1.f / mass;
    }

    void applyForce(const Vec3& f) { force = force + f; }

    void integrate(float dt) {
        if (type == BodyType::Static) return;
        acceleration = force * inverseMass();
        velocity = velocity + acceleration * dt;
        force = {0.f, 0.f, 0.f};
    }
};

// ── Collision Shapes (separate from render mesh) ─────────────────

enum class CollisionShapeType : uint8_t {
    Box,
    Sphere,
    Mesh,
    Compound
};

class CollisionShape {
public:
    virtual ~CollisionShape() = default;
    [[nodiscard]] virtual CollisionShapeType shapeType() const = 0;
    [[nodiscard]] virtual AABB boundingBox() const = 0;
};

class BoxShape : public CollisionShape {
public:
    explicit BoxShape(const Vec3& halfExtents) : m_halfExtents(halfExtents) {}

    [[nodiscard]] CollisionShapeType shapeType() const override { return CollisionShapeType::Box; }
    [[nodiscard]] const Vec3& halfExtents() const { return m_halfExtents; }

    [[nodiscard]] AABB boundingBox() const override {
        return { {-m_halfExtents.x, -m_halfExtents.y, -m_halfExtents.z},
                 { m_halfExtents.x,  m_halfExtents.y,  m_halfExtents.z} };
    }

private:
    Vec3 m_halfExtents;
};

class SphereShape : public CollisionShape {
public:
    explicit SphereShape(float radius) : m_radius(radius) {}

    [[nodiscard]] CollisionShapeType shapeType() const override { return CollisionShapeType::Sphere; }
    [[nodiscard]] float radius() const { return m_radius; }

    [[nodiscard]] AABB boundingBox() const override {
        return { {-m_radius, -m_radius, -m_radius},
                 { m_radius,  m_radius,  m_radius} };
    }

private:
    float m_radius;
};

class MeshCollisionShape : public CollisionShape {
public:
    struct Triangle {
        Vec3 a, b, c;
    };

    void addTriangle(const Vec3& a, const Vec3& b, const Vec3& c) {
        m_triangles.push_back({a, b, c});
        m_dirty = true;
    }

    void clear() { m_triangles.clear(); m_dirty = true; }

    [[nodiscard]] CollisionShapeType shapeType() const override { return CollisionShapeType::Mesh; }
    [[nodiscard]] const std::vector<Triangle>& triangles() const { return m_triangles; }
    [[nodiscard]] size_t triangleCount() const { return m_triangles.size(); }
    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

    [[nodiscard]] AABB boundingBox() const override {
        if (m_triangles.empty()) return {{}, {}};
        Vec3 lo = m_triangles[0].a, hi = m_triangles[0].a;
        for (auto& tri : m_triangles) {
            for (auto* v : {&tri.a, &tri.b, &tri.c}) {
                lo.x = std::min(lo.x, v->x); lo.y = std::min(lo.y, v->y); lo.z = std::min(lo.z, v->z);
                hi.x = std::max(hi.x, v->x); hi.y = std::max(hi.y, v->y); hi.z = std::max(hi.z, v->z);
            }
        }
        return {lo, hi};
    }

private:
    std::vector<Triangle> m_triangles;
    bool m_dirty = true;
};

// ── Collision Contact ────────────────────────────────────────────

struct CollisionContact {
    Vec3 point;
    Vec3 normal;
    float penetration = 0.f;
};

// ── Physics World ────────────────────────────────────────────────

class PhysicsWorld {
public:
    void init() {
        NF_LOG_INFO("Physics", "Physics world initialized");
    }

    void shutdown() {
        m_bodies.clear();
        NF_LOG_INFO("Physics", "Physics world shutdown");
    }

    void setGravity(const Vec3& g) { m_gravity = g; }
    [[nodiscard]] const Vec3& gravity() const { return m_gravity; }

    size_t addBody(RigidBody body) {
        size_t id = m_bodies.size();
        m_bodies.push_back(std::move(body));
        m_positions.push_back({});
        return id;
    }

    RigidBody* getBody(size_t id) {
        return (id < m_bodies.size()) ? &m_bodies[id] : nullptr;
    }

    void setPosition(size_t id, const Vec3& pos) {
        if (id < m_positions.size()) m_positions[id] = pos;
    }

    [[nodiscard]] Vec3 getPosition(size_t id) const {
        return (id < m_positions.size()) ? m_positions[id] : Vec3{};
    }

    void step(float dt) {
        for (size_t i = 0; i < m_bodies.size(); ++i) {
            auto& body = m_bodies[i];
            if (body.type == BodyType::Static) continue;

            if (body.useGravity) {
                body.applyForce(m_gravity * body.mass);
            }

            body.integrate(dt);
            m_positions[i] = m_positions[i] + body.velocity * dt;
        }
    }

    [[nodiscard]] size_t bodyCount() const { return m_bodies.size(); }

private:
    Vec3 m_gravity{0.f, -9.81f, 0.f};
    std::vector<RigidBody> m_bodies;
    std::vector<Vec3> m_positions;
};

} // namespace NF
