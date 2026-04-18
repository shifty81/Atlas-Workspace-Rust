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
