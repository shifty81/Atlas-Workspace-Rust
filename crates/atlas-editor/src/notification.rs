//! [`NotificationCenter`] — workspace notification system (M14).
//!
//! Provides a severity-tagged notification queue that the workspace shell
//! can display, filter, and route to the AtlasAI debug path.

use std::time::{Duration, Instant};

// ── NotificationSeverity ──────────────────────────────────────────────────────

/// Priority / severity of a notification.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub enum NotificationSeverity {
    /// Routine information (lowest severity).
    Info,
    /// Potential issue that does not block work.
    Warning,
    /// An error that needs attention.
    Error,
    /// A blocking failure that requires immediate action (highest severity).
    Critical,
}

impl std::fmt::Display for NotificationSeverity {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Info     => write!(f, "INFO"),
            Self::Warning  => write!(f, "WARNING"),
            Self::Error    => write!(f, "ERROR"),
            Self::Critical => write!(f, "CRITICAL"),
        }
    }
}

// ── Notification ─────────────────────────────────────────────────────────────

/// A single notification entry.
#[derive(Clone, Debug)]
pub struct Notification {
    /// Unique sequence number assigned by the center.
    pub id:        u64,
    pub severity:  NotificationSeverity,
    /// Short one-line message.
    pub title:     String,
    /// Optional longer description.
    pub body:      Option<String>,
    /// Source module or subsystem that emitted the notification.
    pub source:    String,
    /// Whether the user has acknowledged / dismissed this notification.
    pub dismissed: bool,
    /// When the notification was created.
    pub created_at: Instant,
}

impl Notification {
    fn new(id: u64, severity: NotificationSeverity, title: &str, source: &str) -> Self {
        Self {
            id,
            severity,
            title:      title.to_string(),
            body:       None,
            source:     source.to_string(),
            dismissed:  false,
            created_at: Instant::now(),
        }
    }

    /// Age of this notification.
    pub fn age(&self) -> Duration {
        self.created_at.elapsed()
    }
}

// ── NotificationCenter ────────────────────────────────────────────────────────

/// Manages the notification queue for the workspace shell.
pub struct NotificationCenter {
    notifications: Vec<Notification>,
    next_id:       u64,
    /// Maximum number of notifications retained (oldest dropped when exceeded).
    capacity:      usize,
}

impl Default for NotificationCenter {
    fn default() -> Self { Self::new(512) }
}

impl NotificationCenter {
    /// Create a center with the given `capacity`.
    pub fn new(capacity: usize) -> Self {
        Self { notifications: Vec::new(), next_id: 1, capacity }
    }

    // ── Emit ─────────────────────────────────────────────────────────────────

    /// Post a notification.  Returns its assigned id.
    pub fn push(
        &mut self,
        severity: NotificationSeverity,
        title:    &str,
        source:   &str,
    ) -> u64 {
        let id = self.next_id;
        self.next_id += 1;
        let n = Notification::new(id, severity, title, source);
        self.notifications.push(n);
        if self.notifications.len() > self.capacity {
            self.notifications.remove(0);
        }
        id
    }

    /// Post a notification with a longer `body` string.
    pub fn push_with_body(
        &mut self,
        severity: NotificationSeverity,
        title:    &str,
        body:     &str,
        source:   &str,
    ) -> u64 {
        let id = self.push(severity, title, source);
        if let Some(n) = self.notifications.iter_mut().rev().find(|n| n.id == id) {
            n.body = Some(body.to_string());
        }
        id
    }

    // ── Query ─────────────────────────────────────────────────────────────────

    /// Look up a notification by id.
    pub fn get(&self, id: u64) -> Option<&Notification> {
        self.notifications.iter().find(|n| n.id == id)
    }

    /// All notifications in chronological order.
    pub fn all(&self) -> &[Notification] {
        &self.notifications
    }

    /// All notifications at or above `min_severity`.
    pub fn by_severity(&self, min_severity: NotificationSeverity) -> Vec<&Notification> {
        self.notifications.iter().filter(|n| n.severity >= min_severity).collect()
    }

    /// Undismissed notifications.
    pub fn active(&self) -> Vec<&Notification> {
        self.notifications.iter().filter(|n| !n.dismissed).collect()
    }

    /// Highest severity among undismissed notifications.  Returns `None` when
    /// the queue is empty or all are dismissed.
    pub fn peak_severity(&self) -> Option<NotificationSeverity> {
        self.active().iter().map(|n| n.severity).max()
    }

    /// Total number of notifications in the queue.
    pub fn count(&self) -> usize { self.notifications.len() }

    /// Number of undismissed notifications.
    pub fn active_count(&self) -> usize { self.active().len() }

    // ── Dismiss ───────────────────────────────────────────────────────────────

    /// Dismiss a notification by id.  Returns `false` if not found.
    pub fn dismiss(&mut self, id: u64) -> bool {
        if let Some(n) = self.notifications.iter_mut().find(|n| n.id == id) {
            n.dismissed = true;
            return true;
        }
        false
    }

    /// Dismiss all notifications below `severity`.
    pub fn dismiss_below(&mut self, severity: NotificationSeverity) {
        for n in &mut self.notifications {
            if n.severity < severity { n.dismissed = true; }
        }
    }

    /// Dismiss all notifications.
    pub fn dismiss_all(&mut self) {
        for n in &mut self.notifications { n.dismissed = true; }
    }

    /// Remove all dismissed notifications from the queue.
    pub fn purge_dismissed(&mut self) {
        self.notifications.retain(|n| !n.dismissed);
    }

    /// Clear the entire queue.
    pub fn clear(&mut self) {
        self.notifications.clear();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn center() -> NotificationCenter { NotificationCenter::new(64) }

    #[test]
    fn severity_ordering() {
        assert!(NotificationSeverity::Critical > NotificationSeverity::Error);
        assert!(NotificationSeverity::Error    > NotificationSeverity::Warning);
        assert!(NotificationSeverity::Warning  > NotificationSeverity::Info);
    }

    #[test]
    fn push_assigns_sequential_ids() {
        let mut nc = center();
        let id1 = nc.push(NotificationSeverity::Info, "a", "src");
        let id2 = nc.push(NotificationSeverity::Info, "b", "src");
        assert_eq!(id1 + 1, id2);
    }

    #[test]
    fn push_with_body() {
        let mut nc = center();
        let id = nc.push_with_body(NotificationSeverity::Error, "oops", "details here", "editor");
        let n = nc.get(id).unwrap();
        assert_eq!(n.body.as_deref(), Some("details here"));
    }

    #[test]
    fn get_by_id() {
        let mut nc = center();
        let id = nc.push(NotificationSeverity::Warning, "watch out", "renderer");
        let n = nc.get(id).unwrap();
        assert_eq!(n.title, "watch out");
        assert_eq!(n.source, "renderer");
        assert_eq!(n.severity, NotificationSeverity::Warning);
    }

    #[test]
    fn count_and_active_count() {
        let mut nc = center();
        nc.push(NotificationSeverity::Info, "a", "s");
        nc.push(NotificationSeverity::Info, "b", "s");
        assert_eq!(nc.count(), 2);
        assert_eq!(nc.active_count(), 2);
    }

    #[test]
    fn dismiss_by_id() {
        let mut nc = center();
        let id = nc.push(NotificationSeverity::Info, "hi", "s");
        assert!(nc.dismiss(id));
        assert_eq!(nc.active_count(), 0);
        assert!(!nc.dismiss(999)); // unknown
    }

    #[test]
    fn dismiss_all() {
        let mut nc = center();
        nc.push(NotificationSeverity::Error, "a", "s");
        nc.push(NotificationSeverity::Critical, "b", "s");
        nc.dismiss_all();
        assert_eq!(nc.active_count(), 0);
    }

    #[test]
    fn dismiss_below_severity() {
        let mut nc = center();
        nc.push(NotificationSeverity::Info, "info", "s");
        nc.push(NotificationSeverity::Warning, "warn", "s");
        nc.push(NotificationSeverity::Error, "err", "s");
        nc.dismiss_below(NotificationSeverity::Error);
        let active: Vec<_> = nc.active().iter().map(|n| n.severity).collect();
        assert_eq!(active, vec![NotificationSeverity::Error]);
    }

    #[test]
    fn by_severity_filter() {
        let mut nc = center();
        nc.push(NotificationSeverity::Info, "info", "s");
        nc.push(NotificationSeverity::Critical, "crit", "s");
        let high = nc.by_severity(NotificationSeverity::Error);
        assert_eq!(high.len(), 1);
        assert_eq!(high[0].severity, NotificationSeverity::Critical);
    }

    #[test]
    fn peak_severity() {
        let mut nc = center();
        assert_eq!(nc.peak_severity(), None);
        nc.push(NotificationSeverity::Info, "a", "s");
        nc.push(NotificationSeverity::Error, "b", "s");
        assert_eq!(nc.peak_severity(), Some(NotificationSeverity::Error));
    }

    #[test]
    fn purge_dismissed() {
        let mut nc = center();
        let id = nc.push(NotificationSeverity::Info, "a", "s");
        nc.push(NotificationSeverity::Warning, "b", "s");
        nc.dismiss(id);
        nc.purge_dismissed();
        assert_eq!(nc.count(), 1);
        assert_eq!(nc.all()[0].severity, NotificationSeverity::Warning);
    }

    #[test]
    fn capacity_drops_oldest() {
        let mut nc = NotificationCenter::new(3);
        for i in 0..5_u64 {
            nc.push(NotificationSeverity::Info, &format!("msg-{i}"), "s");
        }
        assert_eq!(nc.count(), 3);
        // oldest should be gone
        assert!(nc.get(1).is_none());
        assert!(nc.get(2).is_none());
    }

    #[test]
    fn clear_empties_queue() {
        let mut nc = center();
        nc.push(NotificationSeverity::Error, "a", "s");
        nc.clear();
        assert_eq!(nc.count(), 0);
    }

    #[test]
    fn severity_display() {
        assert_eq!(format!("{}", NotificationSeverity::Info),     "INFO");
        assert_eq!(format!("{}", NotificationSeverity::Critical), "CRITICAL");
    }
}
