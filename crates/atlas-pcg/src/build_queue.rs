//! Timed build / upgrade queue.
//!
//! Rust port of the C++ `atlas::procedural::BuildQueue`.

/// The kind of build operation.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u8)]
pub enum BuildOrderType {
    Construct = 0,
    Upgrade   = 1,
    Repair    = 2,
    Dismantle = 3,
}

/// A single build/upgrade/repair/dismantle order.
#[derive(Clone, Debug)]
pub struct BuildOrder {
    pub id:                 u32,
    pub order_type:         BuildOrderType,
    pub module_type:        u8,
    pub target_slot:        u32,
    pub target_tier:        u8,
    pub total_time_seconds: f32,
    pub elapsed_seconds:    f32,
    pub paused:             bool,
    pub priority:           u8,
}

impl BuildOrder {
    /// Progress in `[0, 1]`.
    pub fn progress(&self) -> f32 {
        if self.total_time_seconds <= 0.0 {
            return 1.0;
        }
        (self.elapsed_seconds / self.total_time_seconds).clamp(0.0, 1.0)
    }

    /// True when the order has completed.
    pub fn is_complete(&self) -> bool {
        self.elapsed_seconds >= self.total_time_seconds
    }

    /// Remaining seconds until completion.
    pub fn remaining_seconds(&self) -> f32 {
        (self.total_time_seconds - self.elapsed_seconds).max(0.0)
    }
}

/// Ordered queue of build operations.
pub struct BuildQueue {
    active:    Vec<BuildOrder>,
    completed: Vec<BuildOrder>,
    next_id:   u32,
}

impl BuildQueue {
    pub fn new() -> Self {
        Self { active: Vec::new(), completed: Vec::new(), next_id: 1 }
    }

    /// Enqueue a build order.  Returns the assigned order ID.
    pub fn add_order(&mut self, mut order: BuildOrder) -> u32 {
        let id = self.next_id;
        self.next_id += 1;
        order.id = id;
        self.active.push(order);
        id
    }

    /// Cancel (remove) an active order.
    pub fn remove_order(&mut self, order_id: u32) {
        self.active.retain(|o| o.id != order_id);
    }

    /// Pause an active order.
    pub fn pause_order(&mut self, order_id: u32) {
        if let Some(o) = self.active.iter_mut().find(|o| o.id == order_id) {
            o.paused = true;
        }
    }

    /// Resume a paused order.
    pub fn resume_order(&mut self, order_id: u32) {
        if let Some(o) = self.active.iter_mut().find(|o| o.id == order_id) {
            o.paused = false;
        }
    }

    /// Advance all non-paused orders by `delta_seconds`.  Completed orders
    /// are moved to the completed list.
    pub fn tick(&mut self, delta_seconds: f32) {
        let mut newly_done = Vec::new();
        for order in &mut self.active {
            if !order.paused {
                order.elapsed_seconds += delta_seconds;
                if order.is_complete() {
                    newly_done.push(order.id);
                }
            }
        }
        for id in newly_done {
            if let Some(pos) = self.active.iter().position(|o| o.id == id) {
                let done = self.active.remove(pos);
                self.completed.push(done);
            }
        }
    }

    /// Look up an active order by ID.
    pub fn get_order(&self, order_id: u32) -> Option<&BuildOrder> {
        self.active.iter().find(|o| o.id == order_id)
    }

    pub fn queue_size(&self) -> usize { self.active.len() }
    pub fn completed_count(&self) -> usize { self.completed.len() }
    pub fn completed_orders(&self) -> &[BuildOrder] { &self.completed }
    pub fn clear_completed(&mut self) { self.completed.clear(); }
    pub fn is_empty(&self) -> bool { self.active.is_empty() }

    /// Active orders sorted by priority (highest first).
    pub fn orders_by_priority(&self) -> Vec<BuildOrder> {
        let mut sorted = self.active.clone();
        sorted.sort_by(|a, b| b.priority.cmp(&a.priority));
        sorted
    }

    /// Sum of remaining time across all active orders.
    pub fn total_remaining_time(&self) -> f32 {
        self.active.iter().map(|o| o.remaining_seconds()).sum()
    }
}

impl Default for BuildQueue {
    fn default() -> Self { Self::new() }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn make_order(total: f32) -> BuildOrder {
        BuildOrder {
            id: 0,
            order_type: BuildOrderType::Construct,
            module_type: 1,
            target_slot: 0,
            target_tier: 1,
            total_time_seconds: total,
            elapsed_seconds: 0.0,
            paused: false,
            priority: 128,
        }
    }

    #[test]
    fn tick_completes_order() {
        let mut q = BuildQueue::new();
        q.add_order(make_order(1.0));
        q.tick(0.5);
        assert_eq!(q.queue_size(), 1);
        assert_eq!(q.completed_count(), 0);
        q.tick(0.6);
        assert_eq!(q.queue_size(), 0);
        assert_eq!(q.completed_count(), 1);
    }

    #[test]
    fn pause_prevents_progress() {
        let mut q = BuildQueue::new();
        let id = q.add_order(make_order(5.0));
        q.pause_order(id);
        q.tick(3.0);
        let o = q.get_order(id).unwrap();
        assert_eq!(o.elapsed_seconds, 0.0);
        q.resume_order(id);
        q.tick(3.0);
        let o = q.get_order(id).unwrap();
        assert_eq!(o.elapsed_seconds, 3.0);
    }

    #[test]
    fn priority_sort() {
        let mut q = BuildQueue::new();
        let mut o1 = make_order(10.0); o1.priority = 50;
        let mut o2 = make_order(10.0); o2.priority = 200;
        let mut o3 = make_order(10.0); o3.priority = 100;
        q.add_order(o1);
        q.add_order(o2);
        q.add_order(o3);
        let sorted = q.orders_by_priority();
        assert_eq!(sorted[0].priority, 200);
        assert_eq!(sorted[1].priority, 100);
        assert_eq!(sorted[2].priority, 50);
    }

    #[test]
    fn remove_order() {
        let mut q = BuildQueue::new();
        let id = q.add_order(make_order(10.0));
        assert_eq!(q.queue_size(), 1);
        q.remove_order(id);
        assert!(q.is_empty());
    }
}
