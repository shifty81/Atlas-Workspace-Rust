pub mod physics_system;

pub use physics_system::PhysicsSystem;

use std::collections::HashMap;
use atlas_math::Vec3;

pub type BodyId = u32;

#[derive(Debug, Clone)]
pub struct RigidBody {
    pub id: BodyId,
    pub position: Vec3,
    pub velocity: Vec3,
    pub acceleration: Vec3,
    pub mass: f32,
    pub restitution: f32,
    pub is_static: bool,
    pub active: bool,
}

impl RigidBody {
    fn new(id: BodyId, mass: f32, is_static: bool) -> Self {
        Self {
            id,
            position: Vec3::ZERO,
            velocity: Vec3::ZERO,
            acceleration: Vec3::ZERO,
            mass,
            restitution: 0.5,
            is_static,
            active: true,
        }
    }
}

#[derive(Debug, Clone)]
pub struct CollisionPair {
    pub a: BodyId,
    pub b: BodyId,
}

#[derive(Default)]
pub struct PhysicsWorld {
    bodies: HashMap<BodyId, RigidBody>,
    next_id: BodyId,
    gravity: Vec3,
    collisions: Vec<CollisionPair>,
}

impl PhysicsWorld {
    pub fn new() -> Self {
        Self {
            gravity: Vec3::new(0.0, -9.81, 0.0),
            ..Default::default()
        }
    }

    pub fn init(&mut self) {
        log::info!("PhysicsWorld initialized");
    }

    pub fn shutdown(&mut self) {
        self.bodies.clear();
        log::info!("PhysicsWorld shutdown");
    }

    pub fn create_body(&mut self, mass: f32, is_static: bool) -> BodyId {
        let id = self.next_id;
        self.next_id += 1;
        self.bodies.insert(id, RigidBody::new(id, mass, is_static));
        id
    }

    pub fn destroy_body(&mut self, id: BodyId) {
        self.bodies.remove(&id);
    }

    pub fn get_body(&self, id: BodyId) -> Option<&RigidBody> {
        self.bodies.get(&id)
    }

    pub fn get_body_mut(&mut self, id: BodyId) -> Option<&mut RigidBody> {
        self.bodies.get_mut(&id)
    }

    pub fn body_count(&self) -> usize {
        self.bodies.len()
    }

    pub fn set_position(&mut self, id: BodyId, x: f32, y: f32, z: f32) {
        if let Some(b) = self.bodies.get_mut(&id) {
            b.position = Vec3::new(x, y, z);
        }
    }

    pub fn set_velocity(&mut self, id: BodyId, vx: f32, vy: f32, vz: f32) {
        if let Some(b) = self.bodies.get_mut(&id) {
            b.velocity = Vec3::new(vx, vy, vz);
        }
    }

    pub fn apply_force(&mut self, id: BodyId, fx: f32, fy: f32, fz: f32) {
        if let Some(b) = self.bodies.get_mut(&id) {
            if !b.is_static && b.mass > 0.0 {
                b.acceleration += Vec3::new(fx, fy, fz) / b.mass;
            }
        }
    }

    pub fn set_gravity(&mut self, x: f32, y: f32, z: f32) {
        self.gravity = Vec3::new(x, y, z);
    }

    pub fn gravity(&self) -> Vec3 {
        self.gravity
    }

    pub fn step(&mut self, dt: f32) {
        let ids: Vec<BodyId> = self.bodies.keys().copied().collect();
        for id in &ids {
            let body = self.bodies.get_mut(id).unwrap();
            if !body.is_static && body.active {
                // Gravity is added to accumulated forces each frame.
                // acceleration acts as a per-frame force accumulator; it is
                // reset to zero after integration so callers must re-apply
                // impulse forces every tick.
                body.acceleration += self.gravity;
                body.velocity     += body.acceleration * dt;
                body.position     += body.velocity     * dt;
                body.acceleration  = Vec3::ZERO;   // reset accumulator
            }
        }

        // AABB broad-phase: unit cube centered at position
        self.collisions.clear();
        let body_list: Vec<(BodyId, Vec3)> = self.bodies.values()
            .map(|b| (b.id, b.position))
            .collect();
        for i in 0..body_list.len() {
            for j in (i + 1)..body_list.len() {
                let (ai, ap) = body_list[i];
                let (bi, bp) = body_list[j];
                // AABB overlap check: half-extent = 0.5
                let diff = (ap - bp).abs();
                if diff.x < 1.0 && diff.y < 1.0 && diff.z < 1.0 {
                    self.collisions.push(CollisionPair { a: ai, b: bi });
                }
            }
        }
    }

    pub fn collisions(&self) -> &[CollisionPair] {
        &self.collisions
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn create_and_destroy_body() {
        let mut w = PhysicsWorld::new();
        let id = w.create_body(1.0, false);
        assert_eq!(w.body_count(), 1);
        w.destroy_body(id);
        assert_eq!(w.body_count(), 0);
    }

    #[test]
    fn gravity_integration() {
        let mut w = PhysicsWorld::new();
        let id = w.create_body(1.0, false);
        w.set_position(id, 0.0, 10.0, 0.0);
        let dt = 0.1_f32;
        w.step(dt);
        let body = w.get_body(id).unwrap();
        // After one step, velocity should be ~= gravity * dt = -0.981 in Y
        assert!((body.velocity.y - (-9.81 * dt)).abs() < 0.001);
        // Position moves down
        assert!(body.position.y < 10.0);
    }

    #[test]
    fn static_body_does_not_move() {
        let mut w = PhysicsWorld::new();
        let id = w.create_body(0.0, true);
        w.set_position(id, 5.0, 5.0, 5.0);
        w.step(0.1);
        let body = w.get_body(id).unwrap();
        assert!((body.position.x - 5.0).abs() < f32::EPSILON);
        assert!((body.position.y - 5.0).abs() < f32::EPSILON);
    }

    #[test]
    fn apply_force_changes_velocity() {
        let mut w = PhysicsWorld::new();
        let id = w.create_body(2.0, false);
        // Override gravity for this test
        w.set_gravity(0.0, 0.0, 0.0);
        w.apply_force(id, 10.0, 0.0, 0.0);
        w.step(1.0);
        let body = w.get_body(id).unwrap();
        // a = F/m = 10/2 = 5; v = a*dt = 5; x = v*dt = 5
        assert!((body.velocity.x - 5.0).abs() < 0.001);
        assert!((body.position.x - 5.0).abs() < 0.001);
    }

    #[test]
    fn collision_detection() {
        let mut w = PhysicsWorld::new();
        w.set_gravity(0.0, 0.0, 0.0);
        let a = w.create_body(1.0, false);
        let b = w.create_body(1.0, false);
        // Place them at the same position → should collide (AABB unit cubes overlap)
        w.set_position(a, 0.0, 0.0, 0.0);
        w.set_position(b, 0.5, 0.0, 0.0); // within 1.0 unit
        w.step(0.016);
        assert!(!w.collisions().is_empty());
    }

    #[test]
    fn no_collision_when_far() {
        let mut w = PhysicsWorld::new();
        w.set_gravity(0.0, 0.0, 0.0);
        let a = w.create_body(1.0, false);
        let b = w.create_body(1.0, false);
        w.set_position(a, 0.0, 0.0, 0.0);
        w.set_position(b, 100.0, 0.0, 0.0);
        w.step(0.016);
        assert!(w.collisions().is_empty());
    }

    #[test]
    fn set_and_get_gravity() {
        let mut w = PhysicsWorld::new();
        w.set_gravity(0.0, -20.0, 0.0);
        assert!((w.gravity().y - (-20.0)).abs() < f32::EPSILON);
    }

    #[test]
    fn set_velocity_directly() {
        let mut w = PhysicsWorld::new();
        w.set_gravity(0.0, 0.0, 0.0);
        let id = w.create_body(1.0, false);
        w.set_velocity(id, 3.0, 0.0, 0.0);
        w.step(1.0);
        let body = w.get_body(id).unwrap();
        assert!((body.position.x - 3.0).abs() < 0.01);
    }
}
