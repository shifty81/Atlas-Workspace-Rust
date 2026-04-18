use parking_lot::Mutex;
use std::sync::Arc;

pub type SubscriptionId = u64;

#[derive(Debug, Clone, Default)]
pub struct Event {
    pub event_type: String,
    pub sender_id: u32,
    pub int_param: i64,
    pub float_param: f64,
    pub str_param: String,
}

impl Event {
    pub fn new(event_type: impl Into<String>) -> Self {
        Self { event_type: event_type.into(), ..Default::default() }
    }
}

type Callback = Box<dyn Fn(&Event) + Send + Sync>;

struct Subscription {
    id: SubscriptionId,
    event_type: String,
    callback: Callback,
}

pub struct EventBus {
    subscriptions: Arc<Mutex<Vec<Subscription>>>,
    queue: Vec<Event>,
    next_id: SubscriptionId,
    total_published: u64,
}

impl Default for EventBus {
    fn default() -> Self { Self::new() }
}

impl EventBus {
    pub fn new() -> Self {
        Self {
            subscriptions: Arc::new(Mutex::new(Vec::new())),
            queue: Vec::new(),
            next_id: 1,
            total_published: 0,
        }
    }

    pub fn subscribe<F: Fn(&Event) + Send + Sync + 'static>(
        &mut self,
        event_type: &str,
        callback: F,
    ) -> SubscriptionId {
        let id = self.next_id;
        self.next_id += 1;
        self.subscriptions.lock().push(Subscription {
            id,
            event_type: event_type.to_owned(),
            callback: Box::new(callback),
        });
        id
    }

    pub fn unsubscribe(&mut self, id: SubscriptionId) {
        self.subscriptions.lock().retain(|s| s.id != id);
    }

    pub fn publish(&mut self, event: Event) {
        self.total_published += 1;
        let subs = self.subscriptions.lock();
        for sub in subs.iter() {
            if sub.event_type == "*" || sub.event_type == event.event_type {
                (sub.callback)(&event);
            }
        }
    }

    pub fn enqueue(&mut self, event: Event) {
        self.queue.push(event);
    }

    pub fn flush(&mut self) {
        let events: Vec<Event> = self.queue.drain(..).collect();
        for event in events {
            self.publish(event);
        }
    }

    pub fn subscription_count(&self) -> usize {
        self.subscriptions.lock().len()
    }

    pub fn queue_size(&self) -> usize {
        self.queue.len()
    }

    pub fn total_published(&self) -> u64 {
        self.total_published
    }

    pub fn reset(&mut self) {
        self.subscriptions.lock().clear();
        self.queue.clear();
        self.total_published = 0;
        self.next_id = 1;
    }
}
