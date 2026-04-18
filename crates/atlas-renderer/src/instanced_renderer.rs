#[derive(Debug, Clone, Default)]
pub struct InstanceData {
    pub transform: [f32; 16],
    pub color: [f32; 4],
    pub entity_id: u32,
}

#[derive(Debug, Clone)]
pub struct InstanceBatch {
    pub mesh_id: u32,
    pub material_id: u32,
    pub instances: Vec<InstanceData>,
    pub id: u32,
}

#[derive(Debug, Default)]
pub struct InstancedRenderer {
    batches: Vec<InstanceBatch>,
    next_batch_id: u32,
    max_instances_per_batch: u32,
    initialized: bool,
    draw_calls: u32,
}

impl InstancedRenderer {
    pub fn new() -> Self { Self::default() }

    pub fn init(&mut self, max_instances_per_batch: u32) {
        self.max_instances_per_batch = max_instances_per_batch;
        self.initialized = true;
    }

    pub fn shutdown(&mut self) {
        self.batches.clear();
        self.initialized = false;
    }

    pub fn create_batch(&mut self, mesh_id: u32, material_id: u32) -> u32 {
        let id = self.next_batch_id;
        self.next_batch_id += 1;
        self.batches.push(InstanceBatch { mesh_id, material_id, instances: Vec::new(), id });
        id
    }

    pub fn remove_batch(&mut self, id: u32) {
        self.batches.retain(|b| b.id != id);
    }

    pub fn add_instance(&mut self, batch_id: u32, data: InstanceData) -> u32 {
        if let Some(batch) = self.batches.iter_mut().find(|b| b.id == batch_id) {
            let idx = batch.instances.len() as u32;
            batch.instances.push(data);
            idx
        } else { u32::MAX }
    }

    pub fn remove_instance(&mut self, batch_id: u32, instance_index: u32) {
        if let Some(batch) = self.batches.iter_mut().find(|b| b.id == batch_id) {
            let idx = instance_index as usize;
            if idx < batch.instances.len() { batch.instances.remove(idx); }
        }
    }

    pub fn update_instance(&mut self, batch_id: u32, instance_index: u32, data: InstanceData) {
        if let Some(batch) = self.batches.iter_mut().find(|b| b.id == batch_id) {
            let idx = instance_index as usize;
            if idx < batch.instances.len() { batch.instances[idx] = data; }
        }
    }

    pub fn clear_batch(&mut self, batch_id: u32) {
        if let Some(batch) = self.batches.iter_mut().find(|b| b.id == batch_id) {
            batch.instances.clear();
        }
    }

    pub fn get_batch(&self, id: u32) -> Option<&InstanceBatch> {
        self.batches.iter().find(|b| b.id == id)
    }

    pub fn batch_count(&self) -> u32 { self.batches.len() as u32 }
    pub fn total_instance_count(&self) -> u32 { self.batches.iter().map(|b| b.instances.len() as u32).sum() }

    pub fn submit_all(&mut self) {
        self.draw_calls += self.batches.len() as u32;
    }

    pub fn draw_call_count(&self) -> u32 { self.draw_calls }
    pub fn max_instances_per_batch(&self) -> u32 { self.max_instances_per_batch }
    pub fn is_initialized(&self) -> bool { self.initialized }
}
