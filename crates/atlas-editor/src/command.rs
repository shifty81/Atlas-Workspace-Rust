//! [`EditorCommand`] trait and [`CommandStack`] for undo/redo (M7).

use atlas_ecs::World;

/// An atomic, reversible editor operation.
pub trait EditorCommand: Send + Sync {
    /// Human-readable description shown in the undo history.
    fn description(&self) -> &str;
    /// Apply the command to the world.
    fn apply(&mut self, world: &mut World);
    /// Revert the command (undo).
    fn revert(&mut self, world: &mut World);
}

/// Fixed-capacity undo / redo stack.
pub struct CommandStack {
    undo_stack: Vec<Box<dyn EditorCommand>>,
    redo_stack: Vec<Box<dyn EditorCommand>>,
    capacity:   usize,
}

impl CommandStack {
    pub fn new(capacity: usize) -> Self {
        Self { undo_stack: Vec::new(), redo_stack: Vec::new(), capacity }
    }

    /// Execute a command, push onto undo stack, and clear redo stack.
    pub fn execute(&mut self, mut cmd: Box<dyn EditorCommand>, world: &mut World) {
        cmd.apply(world);
        self.redo_stack.clear();
        self.undo_stack.push(cmd);
        if self.undo_stack.len() > self.capacity {
            self.undo_stack.remove(0);
        }
    }

    /// Undo the last command.
    pub fn undo(&mut self, world: &mut World) {
        if let Some(mut cmd) = self.undo_stack.pop() {
            cmd.revert(world);
            self.redo_stack.push(cmd);
        }
    }

    /// Redo the last undone command.
    pub fn redo(&mut self, world: &mut World) {
        if let Some(mut cmd) = self.redo_stack.pop() {
            cmd.apply(world);
            self.undo_stack.push(cmd);
        }
    }

    pub fn can_undo(&self) -> bool { !self.undo_stack.is_empty() }
    pub fn can_redo(&self) -> bool { !self.redo_stack.is_empty() }

    pub fn undo_description(&self) -> Option<&str> {
        self.undo_stack.last().map(|c| c.description())
    }
    pub fn redo_description(&self) -> Option<&str> {
        self.redo_stack.last().map(|c| c.description())
    }

    pub fn clear(&mut self) {
        self.undo_stack.clear();
        self.redo_stack.clear();
    }
}

impl Default for CommandStack {
    fn default() -> Self { Self::new(64) }
}

#[cfg(test)]
mod tests {
    use super::*;
    use atlas_ecs::World;

    // A simple counter command that increments/decrements a shared value
    struct IncrCmd { amount: i32 }

    impl EditorCommand for IncrCmd {
        fn description(&self) -> &str { "Increment" }
        fn apply(&mut self, _world: &mut World) { /* we track externally */ }
        fn revert(&mut self, _world: &mut World) {}
    }

    struct TagCmd { tag: &'static str }
    impl EditorCommand for TagCmd {
        fn description(&self) -> &str { self.tag }
        fn apply(&mut self, _world: &mut World) {}
        fn revert(&mut self, _world: &mut World) {}
    }

    fn world() -> World { World::new() }

    #[test]
    fn empty_stack_cannot_undo_or_redo() {
        let mut stack = CommandStack::new(10);
        assert!(!stack.can_undo());
        assert!(!stack.can_redo());
        assert!(stack.undo_description().is_none());
        assert!(stack.redo_description().is_none());
    }

    #[test]
    fn execute_pushes_to_undo_stack() {
        let mut stack = CommandStack::new(10);
        let mut w = world();
        stack.execute(Box::new(TagCmd { tag: "First" }), &mut w);
        assert!(stack.can_undo());
        assert!(!stack.can_redo());
        assert_eq!(stack.undo_description(), Some("First"));
    }

    #[test]
    fn undo_moves_to_redo_stack() {
        let mut stack = CommandStack::new(10);
        let mut w = world();
        stack.execute(Box::new(TagCmd { tag: "A" }), &mut w);
        stack.undo(&mut w);
        assert!(!stack.can_undo());
        assert!(stack.can_redo());
        assert_eq!(stack.redo_description(), Some("A"));
    }

    #[test]
    fn redo_moves_back_to_undo() {
        let mut stack = CommandStack::new(10);
        let mut w = world();
        stack.execute(Box::new(TagCmd { tag: "B" }), &mut w);
        stack.undo(&mut w);
        stack.redo(&mut w);
        assert!(stack.can_undo());
        assert!(!stack.can_redo());
    }

    #[test]
    fn execute_clears_redo_stack() {
        let mut stack = CommandStack::new(10);
        let mut w = world();
        stack.execute(Box::new(TagCmd { tag: "A" }), &mut w);
        stack.undo(&mut w);
        assert!(stack.can_redo());
        stack.execute(Box::new(TagCmd { tag: "B" }), &mut w);
        assert!(!stack.can_redo());
    }

    #[test]
    fn capacity_is_enforced() {
        let mut stack = CommandStack::new(3);
        let mut w = world();
        for t in ["A", "B", "C", "D", "E"] {
            stack.execute(Box::new(TagCmd { tag: t }), &mut w);
        }
        // Only last 3 should be on the undo stack
        let mut descriptions = vec![];
        while stack.can_undo() {
            descriptions.push(stack.undo_description().unwrap().to_string());
            stack.undo(&mut w);
        }
        assert!(descriptions.len() <= 3);
    }

    #[test]
    fn clear_empties_both_stacks() {
        let mut stack = CommandStack::new(10);
        let mut w = world();
        stack.execute(Box::new(TagCmd { tag: "X" }), &mut w);
        stack.undo(&mut w);
        stack.clear();
        assert!(!stack.can_undo());
        assert!(!stack.can_redo());
    }

    #[test]
    fn undo_description_reflects_top_of_stack() {
        let mut stack = CommandStack::new(10);
        let mut w = world();
        stack.execute(Box::new(TagCmd { tag: "first" }), &mut w);
        stack.execute(Box::new(TagCmd { tag: "second" }), &mut w);
        assert_eq!(stack.undo_description(), Some("second"));
    }
}
