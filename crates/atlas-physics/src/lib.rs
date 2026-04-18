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
                body.acceleration = self.gravity;
                body.velocity += body.acceleration * dt;
                body.position += body.velocity * dt;
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
