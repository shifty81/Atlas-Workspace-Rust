//! Physics system — synchronises the ECS [`World`] with a [`PhysicsWorld`].
//!
//! ## Usage
//!
//! ```ignore
//! let mut phys_world = PhysicsWorld::new();
//! phys_world.init();
//!
//! // Spawn ECS entity with PhysicsBody + Transform
//! let id = ecs_world.spawn();
//! let body_id = phys_world.create_body(1.0, false);
//! ecs_world.components.add(id, PhysicsBody::dynamic(body_id, 1.0));
//! ecs_world.components.add(id, Transform::default());
//!
//! // Each tick:
//! PhysicsSystem::step(&mut ecs_world, &mut phys_world, dt);
//! ```

use atlas_ecs::{PhysicsBody, World};
use atlas_math::Transform;

use crate::PhysicsWorld;

/// Bridges ECS entity transforms with a [`PhysicsWorld`].
pub struct PhysicsSystem;

impl PhysicsSystem {
    /// Advance the physics simulation by `dt` seconds.
    ///
    /// **Step order:**
    /// 1. Copy each entity's `Transform.position` into the corresponding body
    ///    in `phys_world` (only static bodies or bodies whose transform was
    ///    externally moved — the normal case).
    /// 2. Advance the `phys_world` by `dt`.
    /// 3. Copy each simulated body's new position back into the entity's
    ///    `Transform`.
    pub fn step(world: &mut World, phys: &mut PhysicsWorld, dt: f32) {
        // Collect (entity, body_id, is_static) pairs to avoid borrow conflicts
        let body_entries: Vec<(atlas_ecs::EntityId, u32, bool)> = world
            .components
            .get_all::<PhysicsBody>()
            .iter()
            .map(|(&entity, body)| (entity, body.body_id, body.is_static))
            .collect();

        // 1. Push static body positions into physics (kinematic override)
        for &(entity, body_id, is_static) in &body_entries {
            if is_static {
                if let Some(t) = world.components.get::<Transform>(entity) {
                    phys.set_position(body_id, t.position.x, t.position.y, t.position.z);
                }
            }
        }

        // 2. Advance simulation
        phys.step(dt);

        // 3. Pull simulated positions back into ECS transforms
        for &(entity, body_id, is_static) in &body_entries {
            if !is_static {
                if let Some(phys_body) = phys.get_body(body_id) {
                    let pos = phys_body.position;
                    if let Some(t) = world.components.get_mut::<Transform>(entity) {
                        t.position = pos;
                    }
                }
            }
        }

        log::trace!("[PhysicsSystem] stepped dt={dt:.4}  bodies={}  collisions={}",
            body_entries.len(), phys.collisions().len());
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use atlas_ecs::World;
    use atlas_math::Transform;

    #[test]
    fn dynamic_body_falls_under_gravity() {
        let mut w = World::new();
        let mut phys = PhysicsWorld::new();
        phys.init();

        let entity = w.spawn();
        let body_id = phys.create_body(1.0, false);
        w.components.add(entity, PhysicsBody::dynamic(body_id, 1.0));
        let mut t = Transform::default();
        t.position.y = 10.0;
        w.components.add(entity, t);

        // Step once
        PhysicsSystem::step(&mut w, &mut phys, 0.1);

        let t = w.components.get::<Transform>(entity).unwrap();
        // Should have moved downward
        assert!(t.position.y < 10.0, "Expected entity to fall, y={}", t.position.y);
    }

    #[test]
    fn static_body_does_not_move() {
        let mut w = World::new();
        let mut phys = PhysicsWorld::new();
        phys.init();

        let entity = w.spawn();
        let body_id = phys.create_body(0.0, true);
        phys.set_position(body_id, 5.0, 5.0, 5.0);
        w.components.add(entity, PhysicsBody::r#static(body_id));
        let mut t = Transform::default();
        t.position = atlas_math::Vec3::new(5.0, 5.0, 5.0);
        w.components.add(entity, t);

        PhysicsSystem::step(&mut w, &mut phys, 0.1);

        let t = w.components.get::<Transform>(entity).unwrap();
        assert!((t.position.y - 5.0).abs() < 0.01);
    }

    #[test]
    fn multi_body_scene() {
        let mut w = World::new();
        let mut phys = PhysicsWorld::new();
        phys.set_gravity(0.0, 0.0, 0.0); // no gravity
        phys.init();

        for i in 0..3u32 {
            let e = w.spawn();
            let bid = phys.create_body(1.0, false);
            phys.set_velocity(bid, 1.0, 0.0, 0.0);
            w.components.add(e, PhysicsBody::dynamic(bid, 1.0));
            w.components.add(e, Transform::default());
        }

        PhysicsSystem::step(&mut w, &mut phys, 1.0);

        // All three should have moved +1 on X
        let all = w.components.get_all::<Transform>();
        assert_eq!(all.len(), 3);
        for (_, t) in all {
            assert!((t.position.x - 1.0).abs() < 0.01, "x={}", t.position.x);
        }
    }
}
