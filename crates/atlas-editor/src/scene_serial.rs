//! Scene serialization — JSON save / load of the ECS world.
//!
//! Only components that implement [`serde::Serialize`] / [`Deserialize`] are
//! saved.  Currently that means:
//! - [`atlas_math::Transform`]
//! - [`atlas_ecs::Name`]
//!
//! ## File format
//!
//! ```json
//! {
//!   "version": 1,
//!   "entities": [
//!     { "id": 1, "name": "Cube", "transform": { ... } },
//!     { "id": 2, "transform": { ... } }
//!   ]
//! }
//! ```

use std::path::Path;

use atlas_ecs::{EntityId, World, Name};
use atlas_math::Transform;
use serde::{Deserialize, Serialize};

const SCENE_FORMAT_VERSION: u32 = 1;

// ── Data types ───────────────────────────────────────────────────────────────

#[derive(Debug, Serialize, Deserialize)]
struct EntityRecord {
    id:        EntityId,
    #[serde(skip_serializing_if = "Option::is_none")]
    name:      Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    transform: Option<TransformRecord>,
}

/// Flattened transform so the JSON is human-readable without glam internals.
#[derive(Debug, Clone, Serialize, Deserialize)]
struct TransformRecord {
    position: [f32; 3],
    rotation: [f32; 4], // xyzw quaternion
    scale:    [f32; 3],
}

impl From<&Transform> for TransformRecord {
    fn from(t: &Transform) -> Self {
        Self {
            position: [t.position.x, t.position.y, t.position.z],
            rotation: [t.rotation.x, t.rotation.y, t.rotation.z, t.rotation.w],
            scale:    [t.scale.x, t.scale.y, t.scale.z],
        }
    }
}

impl From<&TransformRecord> for Transform {
    fn from(r: &TransformRecord) -> Self {
        use atlas_math::{Vec3, Quat};
        Transform {
            position: Vec3::new(r.position[0], r.position[1], r.position[2]),
            rotation: Quat::from_xyzw(r.rotation[0], r.rotation[1], r.rotation[2], r.rotation[3]),
            scale:    Vec3::new(r.scale[0], r.scale[1], r.scale[2]),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
struct SceneFile {
    version:  u32,
    entities: Vec<EntityRecord>,
}

// ── Public API ───────────────────────────────────────────────────────────────

/// Serialize the entire ECS world to a JSON string.
pub fn serialize_scene(world: &World) -> String {
    let entities = world.entities.alive().iter().map(|&id| {
        let name      = world.components.get::<Name>(id).map(|n| n.0.clone());
        let transform = world.components.get::<Transform>(id).map(TransformRecord::from);
        EntityRecord { id, name, transform }
    }).collect();

    let scene = SceneFile { version: SCENE_FORMAT_VERSION, entities };
    serde_json::to_string_pretty(&scene).unwrap_or_else(|e| {
        log::error!("[Scene] Serialize error: {e}");
        String::new()
    })
}

/// Deserialize a JSON string into the given world.
///
/// The world is **NOT** cleared first; entities are added on top of existing
/// state.  Call `World::new()` and pass the fresh world if you want a clean
/// load.
pub fn deserialize_scene(world: &mut World, json: &str) -> Result<(), String> {
    let scene: SceneFile = serde_json::from_str(json)
        .map_err(|e| format!("Scene parse error: {e}"))?;

    if scene.version != SCENE_FORMAT_VERSION {
        return Err(format!(
            "Unsupported scene version {} (expected {})",
            scene.version, SCENE_FORMAT_VERSION
        ));
    }

    for rec in &scene.entities {
        let id = world.spawn();
        if let Some(name) = &rec.name {
            world.components.add(id, Name::new(name.as_str()));
        }
        if let Some(tr) = &rec.transform {
            world.components.add(id, Transform::from(tr));
        }
    }

    log::info!("[Scene] Loaded {} entities", scene.entities.len());
    Ok(())
}

/// Save the world to a file.
pub fn save_scene(world: &World, path: &Path) -> Result<(), String> {
    let json = serialize_scene(world);
    std::fs::write(path, json)
        .map_err(|e| format!("Failed to write scene file: {e}"))?;
    log::info!("[Scene] Saved to {:?}", path);
    Ok(())
}

/// Load a world from a file.  The world is **cleared** before loading.
pub fn load_scene(world: &mut World, path: &Path) -> Result<(), String> {
    let json = std::fs::read_to_string(path)
        .map_err(|e| format!("Failed to read scene file: {e}"))?;
    *world = World::new();
    deserialize_scene(world, &json)
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use atlas_ecs::World;
    use atlas_math::Transform;

    #[test]
    fn roundtrip_empty_world() {
        let world = World::new();
        let json  = serialize_scene(&world);
        let mut w2 = World::new();
        deserialize_scene(&mut w2, &json).unwrap();
        assert_eq!(w2.entities.count(), 0);
    }

    #[test]
    fn roundtrip_entities() {
        let mut world = World::new();
        let e1 = world.spawn();
        world.components.add(e1, Name::new("Cube"));
        world.components.add(e1, Transform::IDENTITY);
        let e2 = world.spawn();
        world.components.add(e2, Transform::from_position(atlas_math::Vec3::new(1.0, 2.0, 3.0)));

        let json = serialize_scene(&world);
        let mut w2 = World::new();
        deserialize_scene(&mut w2, &json).unwrap();

        assert_eq!(w2.entities.count(), 2);
        // Both entities should have a Transform
        let all = w2.components.get_all::<Transform>();
        assert_eq!(all.len(), 2);
    }
}
