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

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::{Arc, Mutex};

    fn make_event(kind: &str) -> Event {
        Event::new(kind)
    }

    #[test]
    fn subscribe_and_publish() {
        let mut bus = EventBus::new();
        let received: Arc<Mutex<Vec<String>>> = Arc::new(Mutex::new(Vec::new()));
        let r = received.clone();
        bus.subscribe("damage", move |e| {
            r.lock().unwrap().push(e.event_type.clone());
        });
        bus.publish(make_event("damage"));
        assert_eq!(received.lock().unwrap().as_slice(), &["damage"]);
    }

    #[test]
    fn wildcard_subscription_receives_all() {
        let mut bus = EventBus::new();
        let count = Arc::new(Mutex::new(0u32));
        let c = count.clone();
        bus.subscribe("*", move |_| { *c.lock().unwrap() += 1; });
        bus.publish(make_event("foo"));
        bus.publish(make_event("bar"));
        assert_eq!(*count.lock().unwrap(), 2);
    }

    #[test]
    fn unsubscribe_stops_delivery() {
        let mut bus = EventBus::new();
        let count = Arc::new(Mutex::new(0u32));
        let c = count.clone();
        let id = bus.subscribe("evt", move |_| { *c.lock().unwrap() += 1; });
        bus.publish(make_event("evt")); // delivered
        bus.unsubscribe(id);
        bus.publish(make_event("evt")); // not delivered
        assert_eq!(*count.lock().unwrap(), 1);
    }

    #[test]
    fn enqueue_and_flush() {
        let mut bus = EventBus::new();
        let count = Arc::new(Mutex::new(0u32));
        let c = count.clone();
        bus.subscribe("e", move |_| { *c.lock().unwrap() += 1; });
        bus.enqueue(make_event("e"));
        bus.enqueue(make_event("e"));
        assert_eq!(bus.queue_size(), 2);
        assert_eq!(*count.lock().unwrap(), 0); // not yet dispatched
        bus.flush();
        assert_eq!(bus.queue_size(), 0);
        assert_eq!(*count.lock().unwrap(), 2);
    }

    #[test]
    fn subscription_count() {
        let mut bus = EventBus::new();
        assert_eq!(bus.subscription_count(), 0);
        let id1 = bus.subscribe("a", |_| {});
        let _id2 = bus.subscribe("b", |_| {});
        assert_eq!(bus.subscription_count(), 2);
        bus.unsubscribe(id1);
        assert_eq!(bus.subscription_count(), 1);
    }

    #[test]
    fn total_published_tracks_count() {
        let mut bus = EventBus::new();
        bus.publish(make_event("x"));
        bus.publish(make_event("y"));
        assert_eq!(bus.total_published(), 2);
    }

    #[test]
    fn reset_clears_all() {
        let mut bus = EventBus::new();
        bus.subscribe("a", |_| {});
        bus.enqueue(make_event("a"));
        bus.reset();
        assert_eq!(bus.subscription_count(), 0);
        assert_eq!(bus.queue_size(), 0);
        assert_eq!(bus.total_published(), 0);
    }

    #[test]
    fn event_fields_propagated() {
        let mut bus = EventBus::new();
        let ev = Arc::new(Mutex::new(None::<Event>));
        let ev2 = ev.clone();
        bus.subscribe("hit", move |e| { *ev2.lock().unwrap() = Some(e.clone()); });
        let mut e = make_event("hit");
        e.int_param = 99;
        e.float_param = 3.14;
        e.str_param = "headshot".into();
        e.sender_id = 7;
        bus.publish(e);
        let captured = ev.lock().unwrap().clone().unwrap();
        assert_eq!(captured.int_param, 99);
        assert!((captured.float_param - 3.14).abs() < 1e-10);
        assert_eq!(captured.str_param, "headshot");
        assert_eq!(captured.sender_id, 7);
    }
}
