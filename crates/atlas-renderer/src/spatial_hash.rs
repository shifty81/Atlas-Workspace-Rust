use std::collections::HashMap;

#[derive(Debug, Clone)]
pub struct SpatialEntity {
    pub id: u32,
    pub x: f32,
    pub y: f32,
    pub z: f32,
    pub radius: f32,
}

#[derive(Debug)]
pub struct SpatialHash {
    cell_size: f32,
    grid: HashMap<(i32, i32, i32), Vec<u32>>,
    entities: HashMap<u32, SpatialEntity>,
}

impl SpatialHash {
    pub fn new(cell_size: f32) -> Self {
        Self { cell_size, grid: HashMap::new(), entities: HashMap::new() }
    }

    fn cell_coords(&self, x: f32, y: f32, z: f32) -> (i32, i32, i32) {
        ((x / self.cell_size).floor() as i32,
         (y / self.cell_size).floor() as i32,
         (z / self.cell_size).floor() as i32)
    }

    pub fn insert(&mut self, entity: SpatialEntity) {
        let cell = self.cell_coords(entity.x, entity.y, entity.z);
        self.grid.entry(cell).or_default().push(entity.id);
        self.entities.insert(entity.id, entity);
    }

    pub fn remove(&mut self, entity_id: u32) {
        if let Some(entity) = self.entities.remove(&entity_id) {
            let cell = self.cell_coords(entity.x, entity.y, entity.z);
            if let Some(ids) = self.grid.get_mut(&cell) {
                ids.retain(|&id| id != entity_id);
                if ids.is_empty() { self.grid.remove(&cell); }
            }
        }
    }

    pub fn update(&mut self, entity_id: u32, x: f32, y: f32, z: f32) {
        if let Some(entity) = self.entities.get(&entity_id).cloned() {
            let old_cell = self.cell_coords(entity.x, entity.y, entity.z);
            let new_cell = self.cell_coords(x, y, z);
            if old_cell != new_cell {
                if let Some(ids) = self.grid.get_mut(&old_cell) {
                    ids.retain(|&id| id != entity_id);
                    if ids.is_empty() { self.grid.remove(&old_cell); }
                }
                self.grid.entry(new_cell).or_default().push(entity_id);
            }
            if let Some(e) = self.entities.get_mut(&entity_id) {
                e.x = x; e.y = y; e.z = z;
            }
        }
    }

    pub fn query_radius(&self, x: f32, y: f32, z: f32, radius: f32) -> Vec<u32> {
        let min_cell = self.cell_coords(x - radius, y - radius, z - radius);
        let max_cell = self.cell_coords(x + radius, y + radius, z + radius);
        let mut result = Vec::new();
        let r2 = radius * radius;
        for cx in min_cell.0..=max_cell.0 {
            for cy in min_cell.1..=max_cell.1 {
                for cz in min_cell.2..=max_cell.2 {
                    if let Some(ids) = self.grid.get(&(cx, cy, cz)) {
                        for &id in ids {
                            if let Some(e) = self.entities.get(&id) {
                                let dx = e.x - x; let dy = e.y - y; let dz = e.z - z;
                                if dx*dx + dy*dy + dz*dz <= r2 { result.push(id); }
                            }
                        }
                    }
                }
            }
        }
        result
    }

    pub fn query_aabb(&self, min_x: f32, min_y: f32, min_z: f32, max_x: f32, max_y: f32, max_z: f32) -> Vec<u32> {
        let min_cell = self.cell_coords(min_x, min_y, min_z);
        let max_cell = self.cell_coords(max_x, max_y, max_z);
        let mut result = Vec::new();
        for cx in min_cell.0..=max_cell.0 {
            for cy in min_cell.1..=max_cell.1 {
                for cz in min_cell.2..=max_cell.2 {
                    if let Some(ids) = self.grid.get(&(cx, cy, cz)) {
                        for &id in ids {
                            if let Some(e) = self.entities.get(&id) {
                                if e.x >= min_x && e.x <= max_x
                                    && e.y >= min_y && e.y <= max_y
                                    && e.z >= min_z && e.z <= max_z {
                                    result.push(id);
                                }
                            }
                        }
                    }
                }
            }
        }
        result
    }

    pub fn get_nearest_neighbors(&self, x: f32, y: f32, z: f32, count: u32) -> Vec<u32> {
        let big_radius = self.cell_size * 10.0;
        let mut candidates: Vec<(u32, f32)> = self.query_radius(x, y, z, big_radius)
            .into_iter()
            .filter_map(|id| {
                self.entities.get(&id).map(|e| {
                    let dx = e.x - x; let dy = e.y - y; let dz = e.z - z;
                    (id, dx*dx + dy*dy + dz*dz)
                })
            })
            .collect();
        candidates.sort_by(|a, b| a.1.partial_cmp(&b.1).unwrap_or(std::cmp::Ordering::Equal));
        candidates.into_iter().take(count as usize).map(|(id, _)| id).collect()
    }

    pub fn clear(&mut self) { self.grid.clear(); self.entities.clear(); }
    pub fn entity_count(&self) -> u32 { self.entities.len() as u32 }
    pub fn cell_size(&self) -> f32 { self.cell_size }
    pub fn occupied_cell_count(&self) -> u32 { self.grid.len() as u32 }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn entity(id: u32, x: f32, y: f32, z: f32) -> SpatialEntity {
        SpatialEntity { id, x, y, z, radius: 0.5 }
    }

    #[test]
    fn insert_and_count() {
        let mut sh = SpatialHash::new(10.0);
        sh.insert(entity(1, 0.0, 0.0, 0.0));
        sh.insert(entity(2, 5.0, 0.0, 0.0));
        assert_eq!(sh.entity_count(), 2);
    }

    #[test]
    fn remove_decrements_count() {
        let mut sh = SpatialHash::new(10.0);
        sh.insert(entity(1, 0.0, 0.0, 0.0));
        sh.remove(1);
        assert_eq!(sh.entity_count(), 0);
    }

    #[test]
    fn remove_nonexistent_is_noop() {
        let mut sh = SpatialHash::new(10.0);
        sh.remove(999); // should not panic
    }

    #[test]
    fn query_radius_finds_nearby_entities() {
        let mut sh = SpatialHash::new(10.0);
        sh.insert(entity(1, 0.0, 0.0, 0.0));
        sh.insert(entity(2, 1.0, 0.0, 0.0));
        sh.insert(entity(3, 100.0, 0.0, 0.0)); // far away
        let result = sh.query_radius(0.0, 0.0, 0.0, 5.0);
        assert!(result.contains(&1));
        assert!(result.contains(&2));
        assert!(!result.contains(&3));
    }

    #[test]
    fn query_aabb_finds_entities_inside_box() {
        let mut sh = SpatialHash::new(10.0);
        sh.insert(entity(1, 1.0, 1.0, 1.0));
        sh.insert(entity(2, 5.0, 5.0, 5.0));
        sh.insert(entity(3, 20.0, 20.0, 20.0)); // outside
        let result = sh.query_aabb(0.0, 0.0, 0.0, 10.0, 10.0, 10.0);
        assert!(result.contains(&1));
        assert!(result.contains(&2));
        assert!(!result.contains(&3));
    }

    #[test]
    fn update_changes_position() {
        let mut sh = SpatialHash::new(1.0);
        sh.insert(entity(1, 0.0, 0.0, 0.0));
        sh.update(1, 50.0, 50.0, 50.0);
        let near_origin = sh.query_radius(0.0, 0.0, 0.0, 1.0);
        assert!(!near_origin.contains(&1));
        let near_new = sh.query_radius(50.0, 50.0, 50.0, 1.0);
        assert!(near_new.contains(&1));
    }

    #[test]
    fn nearest_neighbors_returns_closest() {
        let mut sh = SpatialHash::new(5.0);
        sh.insert(entity(1, 1.0, 0.0, 0.0));
        sh.insert(entity(2, 2.0, 0.0, 0.0));
        sh.insert(entity(3, 3.0, 0.0, 0.0));
        let nearest = sh.get_nearest_neighbors(0.0, 0.0, 0.0, 2);
        assert_eq!(nearest.len(), 2);
        assert!(nearest.contains(&1));
        assert!(nearest.contains(&2));
    }

    #[test]
    fn clear_empties_all() {
        let mut sh = SpatialHash::new(10.0);
        sh.insert(entity(1, 0.0, 0.0, 0.0));
        sh.insert(entity(2, 1.0, 0.0, 0.0));
        sh.clear();
        assert_eq!(sh.entity_count(), 0);
        assert_eq!(sh.occupied_cell_count(), 0);
    }

    #[test]
    fn cell_size_accessor() {
        let sh = SpatialHash::new(7.5);
        assert_eq!(sh.cell_size(), 7.5);
    }
}
